/*
 * net/sched/sch_hi.c	A test QDisc, its basically pfifo with more logging.
 *
 *		This program is free software; you can redistribute it and/or
 *		modify it under the terms of the GNU General Public License
 *		as published by the Free Software Foundation; either version
 *		2 of the License, or (at your option) any later version.
 *
 * Authors:	Taran Lynn <taranlynn0@gmail.com>
 */

#include <linux/module.h>
#include <linux/slab.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/skbuff.h>
#include <net/pkt_sched.h>
#include <net/tcp.h>
#include <linux/version.h>

#include "../../mpc/control.h"
#include "sch_hi.h"

struct hi_sched_data {
    // The maximum rate to send in bytes per second.
    u64 max_rate;

    // The time we sent, or will send, the last packet at.
    u64 last_time_to_send;

    // Watchdog to set a timeout.
    struct qdisc_watchdog watchdog;

    u32 last_srtt;

    struct model *md;
};


struct hi_skb_cb {
    // When should this packet be sent?
    u64 time_to_send;
};

static inline struct hi_skb_cb *hi_skb_cb(struct sk_buff *skb)
{
    qdisc_cb_private_validate(skb, sizeof(struct hi_skb_cb));
    return (struct hi_skb_cb *)qdisc_skb_cb(skb)->data;
}


static int hi_enqueue(struct sk_buff *skb, struct Qdisc *sch,
        struct sk_buff **to_free)
{
    struct hi_sched_data *q = qdisc_priv(sch);
    u64 now = ktime_get_ns();

    hi_log("hi_enqueue\n");

    if(skb != NULL) {
        u64 min_delay;

        if(q->max_rate == 0)
            min_delay = 0;
        else
            min_delay = NSEC_PER_SEC * skb->len / q->max_rate;

        // Take the max here to ensure that we don't go past the max rate on a
        // burst.
        q->last_time_to_send = max_t(u64, now, q->last_time_to_send) + min_delay;
        hi_skb_cb(skb)->time_to_send = q->last_time_to_send;

        hi_log("enq, skb->len = %d, min_delay = %lld.%llds\n",
                skb->len, min_delay/NSEC_PER_SEC, min_delay%NSEC_PER_SEC);
    }

    if (likely(sch->q.qlen < sch->limit))
        return qdisc_enqueue_tail(skb, sch);

    return qdisc_drop(skb, sch, to_free);
}

static struct sk_buff* hi_dequeue(struct Qdisc *sch)
{
    struct hi_sched_data *q = qdisc_priv(sch);
    struct sk_buff *skb = NULL;
    u64 now = ktime_get_ns();
    u64 next_tts = 0;  // The next time to send a packet.

    hi_log("hi_dequeue\n");

    // skb corresponds to whatever packet is ready, NULL if none are.
    skb = qdisc_peek_head(sch);

    while(skb != NULL) {
        u64 tts = hi_skb_cb(skb)->time_to_send;
        s64 over = tts - now;

        // For diagnostics, ideally these values should be small.
        if(over > 0)
            hi_log("Time till send %lld.%llds\n", over/NSEC_PER_SEC, over%NSEC_PER_SEC);
        else {
            over = -over;
            hi_log("Time over send %lld.%llds\n", over/NSEC_PER_SEC, over%NSEC_PER_SEC);
        }

        // Only send out a packet if doing so doesn't go over the maximum
        // bit rate.
        if(hi_skb_cb(skb)->time_to_send <= now)
            goto next_packet;
        else {
            // Next send time should be for the closest packet.
            if(next_tts > 0)
                next_tts = min_t(u64, next_tts, tts);
            else
                next_tts = tts;

            skb = skb->next;
        }
    }

    // No packets are ready to be sent, they need to wait.
    skb = NULL;
    goto exit_dequeue;

next_packet:
    skb = qdisc_dequeue_head(sch);

    // Update rate using MPC.
    if(skb != NULL) {
        if(skb->sk != NULL && skb->sk->sk_protocol == IPPROTO_TCP) {
            // NOTE: There is a need to condsider this for different flows.
            struct tcp_sock *tp = tcp_sk(skb->sk);
            u32 rtt;

            // srtt' = (1 - a) * srtt + a * rtt
            // rtt = srtt + (srtt' - srtt)/a
            // Reading the sources tells us that a = 1/8.
            // We have to make sure we get a positive rtt, otherwise estimate as
            // srtt.
            //
            // TODO: Don't hardcode alpha.
            if(q->last_srtt + (tp->srtt_us<<3) > (q->last_srtt<<3))
                rtt = q->last_srtt + tp->srtt_us<<3 - q->last_srtt<<3;
            else
                rtt = tp->srtt_us;

            q->last_srtt = tp->srtt_us;

            q->max_rate = real_floor(control_process(q->md,
                  real_from_frac(rtt, USEC_PER_SEC)));

            hi_log("tp->srtt_us = %u, tp->mdev_us = %u, rtt = %u, q->max_rate = %llu\n",
                    tp->srtt_us, tp->mdev_us, rtt, q->max_rate);
        }

        hi_log("deq, skb->len = %d, cb->pkt_len = %d\n",
                skb->len, qdisc_skb_cb(skb)->pkt_len);
    }
exit_dequeue:
    hi_log("next_tts = %lld.%lld\n", next_tts/NSEC_PER_SEC, next_tts%NSEC_PER_SEC);
    if(next_tts > 0)
        qdisc_watchdog_schedule_ns(&q->watchdog, next_tts);
    else
        qdisc_watchdog_cancel(&q->watchdog);

    // Returns a packet to send out, NULL if we don't send out any.
    return skb;
}

static struct sk_buff* hi_peek(struct Qdisc *sch) {
    hi_log("hi_peek\n");

    return qdisc_peek_head(sch);
}


static const struct nla_policy hi_policy[TCA_HI_MAX + 1] = {
    [TCA_HI_LIMIT]			= { .type = NLA_U32 },
    [TCA_HI_MAXRATE]		= { .type = NLA_U32 },
};

#if LINUX_VERSION_CODE <= KERNEL_VERSION(4,16,0)
static int hi_change(struct Qdisc *sch, struct nlattr *opt)
#else
static int hi_change(struct Qdisc *sch, struct nlattr *opt,
        struct netlink_ext_ack *extack)
#endif
{
    struct hi_sched_data *q = qdisc_priv(sch);
    struct nlattr *tb[TCA_HI_MAX + 1];
    int err;

    hi_log("hi_change\n");

    if (opt == NULL)
        return -EINVAL;

    err = nla_parse_nested(tb, TCA_HI_MAX, opt, hi_policy, NULL);
    if (err < 0)
        return err;

    if (tb[TCA_HI_LIMIT]) {
        sch->limit = nla_get_u32(tb[TCA_HI_LIMIT]);
        hi_log("Set limit to %d\n", sch->limit);
    } else {
        hi_log("No set limit %p stays at %d\n", tb[TCA_HI_LIMIT], sch->limit);
    }

    if (tb[TCA_HI_MAXRATE]) {
        q->max_rate = nla_get_u32(tb[TCA_HI_MAXRATE]);

        hi_log("Set max_rate to %lld\n", q->max_rate);
    }

    return err;
}

// FIXME: This is a quick fix to make it compile on 4.15 and 4.17.
#if LINUX_VERSION_CODE <= KERNEL_VERSION(4,16,0)
static int hi_init(struct Qdisc *sch, struct nlattr *opt)
#else
static int hi_init(struct Qdisc *sch, struct nlattr *opt,
        struct netlink_ext_ack *extack)
#endif
{
    struct hi_sched_data *q = qdisc_priv(sch);

    hi_log("Initialized\n");

    q->max_rate = ~0U;
    q->last_time_to_send = 0;
    qdisc_watchdog_init(&q->watchdog, sch);

    q->last_srtt = 0;

    q->md = kmalloc(sizeof(struct model), GFP_KERNEL);
    model_init(q->md, real_from_frac(1, 1), real_from_frac(1, 10),
        real_from_frac(1, 2), real_from_frac(1, 2), 5, 1);

    sch->limit = qdisc_dev(sch)->tx_queue_len;

    if(opt) {
#if LINUX_VERSION_CODE <= KERNEL_VERSION(4,16,0)
        hi_change(sch, opt);
#else
        hi_change(sch, opt, extack);
#endif
    }

    return 0;
}

static int hi_dump(struct Qdisc *sch, struct sk_buff *skb)
{
    struct hi_sched_data *q = qdisc_priv(sch);
    struct nlattr *opts;

    opts = nla_nest_start(skb, TCA_OPTIONS);
    if (opts == NULL)
        goto nla_put_failure;

    if (nla_put_u32(skb, TCA_HI_LIMIT, sch->limit) ||
        nla_put_u32(skb, TCA_HI_MAXRATE, q->max_rate))
        goto nla_put_failure;

    return nla_nest_end(skb, opts);

nla_put_failure:
    return -1;
}

static void hi_reset(struct Qdisc *sch)
{
    struct hi_sched_data *q = qdisc_priv(sch);

    hi_log("hi_reset\n");

    qdisc_reset_queue(sch);
    qdisc_watchdog_cancel(&q->watchdog);
}

static void hi_destroy(struct Qdisc *sch)
{
    struct hi_sched_data *q = qdisc_priv(sch);
    hi_log("hi_destroy\n");

    model_release(q->md);
    kfree(q->md);
}

struct Qdisc_ops hi_qdisc_ops __read_mostly = {
    .id		=	"hi",
    .priv_size	=	0,
    .enqueue	=	hi_enqueue,
    .dequeue	=	hi_dequeue,
    .peek		=	hi_peek,
    .init		=	hi_init,
    .reset		=	hi_reset,
    .destroy  = hi_destroy,
    .change		=	hi_change,
    .dump		=	hi_dump,
    .owner		=	THIS_MODULE,
};
EXPORT_SYMBOL(hi_qdisc_ops);


MODULE_LICENSE("GPL");
MODULE_AUTHOR("Taran Lynn");
MODULE_DESCRIPTION("A simple example Qdisc.");
MODULE_VERSION("0.01");

static int __init hiqd_init(void) {
    register_qdisc(&hi_qdisc_ops);
    printk(KERN_INFO "Registered qdisc %s\n", hi_qdisc_ops.id);
    printk(KERN_INFO "Loaded module hiqd\n");

    return 0;
}

static void __exit hiqd_exit(void) {
    unregister_qdisc(&hi_qdisc_ops);
    printk(KERN_INFO "Unregistered qdisc %s\n", hi_qdisc_ops.id);
    printk(KERN_INFO "Removed module hiqd\n");
}

module_init(hiqd_init);
module_exit(hiqd_exit);

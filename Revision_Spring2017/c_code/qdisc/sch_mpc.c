/*
 * net/sched/sch_mpc.c	A test QDisc, its basically pfifo with more logging.
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

#include "../mpc/control.h"
#include "sch_mpc.h"

struct mpc_sched_data {
    // The maximum rate to send in bytes per second.
    u64 max_rate;

    // The time we sent, or will send, the last packet at.
    u64 last_time_to_send;

    // Watchdog to set a timeout.
    struct qdisc_watchdog watchdog;

    u32 last_srtt;

    struct model *md;
};


struct mpc_skb_cb {
    // When should this packet be sent?
    u64 time_to_send;
};

static inline struct mpc_skb_cb *mpc_skb_cb(struct sk_buff *skb)
{
    qdisc_cb_private_validate(skb, sizeof(struct mpc_skb_cb));
    return (struct mpc_skb_cb *)qdisc_skb_cb(skb)->data;
}


static int mpc_enqueue(struct sk_buff *skb, struct Qdisc *sch,
        struct sk_buff **to_free)
{
    struct mpc_sched_data *q = qdisc_priv(sch);
    u64 now = ktime_get_ns();

    if(skb != NULL) {
        u64 min_delay;

        if(q->max_rate == 0)
            min_delay = 0;
        else
            min_delay = NSEC_PER_SEC * skb->len / q->max_rate;

        // Take the max here to ensure that we don't go past the max rate on a
        // burst.
        q->last_time_to_send = max_t(u64, now, q->last_time_to_send) + min_delay;
        mpc_skb_cb(skb)->time_to_send = q->last_time_to_send;
    }

    if (likely(sch->q.qlen < sch->limit))
        return qdisc_enqueue_tail(skb, sch);

    return qdisc_drop(skb, sch, to_free);
}

static struct sk_buff* mpc_dequeue(struct Qdisc *sch)
{
    struct mpc_sched_data *q = qdisc_priv(sch);
    struct sk_buff *skb = NULL;
    u64 now = ktime_get_ns();
    u64 next_tts = 0;  // The next time to send a packet.

    // skb corresponds to whatever packet is ready, NULL if none are.
    skb = qdisc_peek_head(sch);

    while(skb != NULL) {
        u64 tts = mpc_skb_cb(skb)->time_to_send;
        s64 over = tts - now;

        // For diagnostics, ideally these values should be small.
        if(over > 0)
            mpc_qd_log("Time till send %lld.%llds\n", over/NSEC_PER_SEC, over%NSEC_PER_SEC);
        else {
            over = -over;
            mpc_qd_log("Time over send %lld.%llds\n", over/NSEC_PER_SEC, over%NSEC_PER_SEC);
        }

        // Only send out a packet if doing so doesn't go over the maximum
        // bit rate.
        if(mpc_skb_cb(skb)->time_to_send <= now)
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
                rtt = q->last_srtt + (tp->srtt_us<<3) - (q->last_srtt<<3);
            else
                rtt = tp->srtt_us;

            q->last_srtt = tp->srtt_us;

            q->max_rate = real_floor(control_process(q->md,
                  real_from_frac(rtt, USEC_PER_SEC), REAL_ZERO));
        }

        mpc_qd_log("deq, skb->len = %d, cb->pkt_len = %d\n",
                skb->len, qdisc_skb_cb(skb)->pkt_len);
    }
exit_dequeue:
    mpc_qd_log("next_tts = %lld.%lld\n", next_tts/NSEC_PER_SEC, next_tts%NSEC_PER_SEC);
    if(next_tts > 0)
        qdisc_watchdog_schedule_ns(&q->watchdog, next_tts);
    else
        qdisc_watchdog_cancel(&q->watchdog);

    // Returns a packet to send out, NULL if we don't send out any.
    return skb;
}

static struct sk_buff* mpc_peek(struct Qdisc *sch) {
    return qdisc_peek_head(sch);
}


static const struct nla_policy mpc_policy[TCA_MPC_MAX + 1] = {
    [TCA_MPC_LIMIT]			= { .type = NLA_U32 },
    [TCA_MPC_MAXRATE]		= { .type = NLA_U32 },
};

#if LINUX_VERSION_CODE <= KERNEL_VERSION(4,16,0)
static int mpc_change(struct Qdisc *sch, struct nlattr *opt)
#else
static int mpc_change(struct Qdisc *sch, struct nlattr *opt,
        struct netlink_ext_ack *extack)
#endif
{
    struct mpc_sched_data *q = qdisc_priv(sch);
    struct nlattr *tb[TCA_MPC_MAX + 1];
    int err;

    if (opt == NULL)
        return -EINVAL;

    err = nla_parse_nested(tb, TCA_MPC_MAX, opt, mpc_policy, NULL);
    if (err < 0)
        return err;

    if (tb[TCA_MPC_LIMIT])
        sch->limit = nla_get_u32(tb[TCA_MPC_LIMIT]);

    if (tb[TCA_MPC_MAXRATE])
        q->max_rate = nla_get_u32(tb[TCA_MPC_MAXRATE]);

    return err;
}

// FIXME: This is a quick fix to make it compile on 4.15 and 4.17.
#if LINUX_VERSION_CODE <= KERNEL_VERSION(4,16,0)
static int mpc_init(struct Qdisc *sch, struct nlattr *opt)
#else
static int mpc_init(struct Qdisc *sch, struct nlattr *opt,
        struct netlink_ext_ack *extack)
#endif
{
    struct mpc_sched_data *q = qdisc_priv(sch);

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
        mpc_change(sch, opt);
#else
        mpc_change(sch, opt, extack);
#endif
    }

    return 0;
}

static int mpc_dump(struct Qdisc *sch, struct sk_buff *skb)
{
    struct mpc_sched_data *q = qdisc_priv(sch);
    struct nlattr *opts;

    opts = nla_nest_start(skb, TCA_OPTIONS);
    if (opts == NULL)
        goto nla_put_failure;

    if (nla_put_u32(skb, TCA_MPC_LIMIT, sch->limit) ||
        nla_put_u32(skb, TCA_MPC_MAXRATE, q->max_rate))
        goto nla_put_failure;

    return nla_nest_end(skb, opts);

nla_put_failure:
    return -1;
}

static void mpc_reset(struct Qdisc *sch)
{
    struct mpc_sched_data *q = qdisc_priv(sch);

    qdisc_reset_queue(sch);
    qdisc_watchdog_cancel(&q->watchdog);
}

static void mpc_destroy(struct Qdisc *sch)
{
    struct mpc_sched_data *q = qdisc_priv(sch);

    model_release(q->md);
    kfree(q->md);
}

struct Qdisc_ops mpc_qdisc_ops __read_mostly = {
    .id		=	"mpc",
    .priv_size	=	0,
    .enqueue	=	mpc_enqueue,
    .dequeue	=	mpc_dequeue,
    .peek		=	mpc_peek,
    .init		=	mpc_init,
    .reset		=	mpc_reset,
    .destroy  = mpc_destroy,
    .change		=	mpc_change,
    .dump		=	mpc_dump,
    .owner		=	THIS_MODULE,
};
EXPORT_SYMBOL(mpc_qdisc_ops);


MODULE_LICENSE("GPL");
MODULE_AUTHOR("Taran Lynn");
MODULE_DESCRIPTION("A simple example Qdisc.");
MODULE_VERSION("0.01");

static int __init mpcqd_init(void) {
    register_qdisc(&mpc_qdisc_ops);
    printk(KERN_INFO "Loaded module mpcqd\n");

    return 0;
}

static void __exit mpcqd_exit(void) {
    unregister_qdisc(&mpc_qdisc_ops);
    printk(KERN_INFO "Removed module mpcqd\n");
}

module_init(mpcqd_init);
module_exit(mpcqd_exit);
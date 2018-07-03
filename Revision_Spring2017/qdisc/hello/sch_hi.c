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
#include <linux/version.h>

#include "sch_hi.h"

struct hi_sched_data {
    // The maximum rate to send in bytes per second.
    u64 max_rate;

    // The number of bits we are ready to send.
    int ready_bits;

    // The timestamp when the last packet was sent.
    u64 last_time;
};


static int hi_enqueue(struct sk_buff *skb, struct Qdisc *sch,
        struct sk_buff **to_free)
{
    hi_log("hi_enqueue\n");

    if(skb != NULL)
        hi_log("enq, skb->len = %d, skb->data_len = %d\n", skb->len, skb->data_len);

    if (likely(sch->q.qlen < sch->limit))
        return qdisc_enqueue_tail(skb, sch);

    return qdisc_drop(skb, sch, to_free);
}

static struct sk_buff* hi_dequeue(struct Qdisc *sch) {
    struct hi_sched_data *q = qdisc_priv(sch);
    struct sk_buff *skb = NULL;
    u64 now = ktime_get_ns();
    int new_bits = (now - q->last_time) * (8 * q->max_rate) / NSEC_PER_SEC;

    hi_log("hi_dequeue\n");

    hi_log("q->ready_bits = %d\n", q->ready_bits);

    // skb corresponds to whatever packet is ready, NULL is none are.
    skb = qdisc_peek_head(sch);

    if(skb != NULL) {
        q->ready_bits += new_bits;
    } else {
        // We are no longer sending a stream of packets, start decreasing
        // ready_bits. Can think of this as bits that would have been sent.
        q->ready_bits -= new_bits;

        if(q->ready_bits < 0)
            q->ready_bits = 0;
    }

    if(skb != NULL && skb->len <= q->ready_bits) {
        // Only send out a packet if doing so doesn't go over the maximum bit rate.
        skb = qdisc_dequeue_head(sch);
        q->ready_bits -= skb->len;
    } else {
        skb = NULL;
    }

    if(skb != NULL)
        hi_log("deq, skb->len = %d, skb->data_len = %d\n", skb->len, skb->data_len);

    q->last_time = now;

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
    q->ready_bits = 0;
    q->last_time = ktime_get_ns();

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
    hi_log("hi_reset\n");

    qdisc_reset_queue(sch);
}

static void hi_destroy(struct Qdisc *sch)
{
    hi_log("hi_destroy\n");
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

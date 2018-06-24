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
    struct sk_buff *skb;

    hi_log("hi_dequeue\n");

    // skb corresponds to whatever packet is ready, NULL is none are.
    skb = qdisc_dequeue_head(sch);

    if(skb != NULL)
        hi_log("deq, skb->len = %d, skb->data_len = %d\n", skb->len, skb->data_len);

    // Returns a packet to send out, NULL if we don't send out any.
    return skb;
}

static struct sk_buff* hi_peek(struct Qdisc *sch) {
    hi_log("hi_peek\n");

    return qdisc_peek_head(sch);
}


static const struct nla_policy hi_policy[TCA_HI_MAX + 1] = {
    [TCA_HI_LIMIT]			= { .type = NLA_U32 },
};

#if LINUX_VERSION_CODE <= KERNEL_VERSION(4,16,0)
static int hi_change(struct Qdisc *sch, struct nlattr *opt)
#else
static int hi_change(struct Qdisc *sch, struct nlattr *opt,
        struct netlink_ext_ack *extack)
#endif
{
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
        hi_log("No set limit %p\n", tb[TCA_HI_LIMIT]);
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
    hi_log("Initialized\n");

    if (opt == NULL) {
        u32 limit = qdisc_dev(sch)->tx_queue_len;

        sch->limit = limit;
    } else {
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
    struct nlattr *opts;

    opts = nla_nest_start(skb, TCA_OPTIONS);
    if (opts == NULL)
        goto nla_put_failure;

    if (nla_put_u32(skb, TCA_HI_LIMIT, sch->limit))
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

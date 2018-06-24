/*
 * net/sched/sch_hi.c	A test QDisc, its basically pfifo with more logging.
 *
 *		This program is free software; you can redistribute it and/or
 *		modify it under the terms of the GNU General Public License
 *		as published by the Free Software Foundation; either version
 *		2 of the License, or (at your option) any later version.
 *
 * Authors:	Alexey Kuznetsov, <kuznet@ms2.inr.ac.ru>
 *          Taran Lynn <taranlynn0@gmail.com>
 */

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
        struct tc_fifo_qopt *ctl = nla_data(opt);

        if (nla_len(opt) < sizeof(*ctl))
            return -EINVAL;

        sch->limit = ctl->limit;
    }

    return 0;
}

static int hi_dump(struct Qdisc *sch, struct sk_buff *skb)
{
    struct tc_fifo_qopt opt = { .limit = sch->limit };

    hi_log("hi_dump\n");

    if (nla_put(skb, TCA_OPTIONS, sizeof(opt), &opt))
        goto nla_put_failure;
    return skb->len;

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
    .change		=	hi_init,
    .dump		=	hi_dump,
    .owner		=	THIS_MODULE,
};
EXPORT_SYMBOL(hi_qdisc_ops);

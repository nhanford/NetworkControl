/*
 * net/sched/sch_mpc.c	A model predictive control QDisc.
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


const size_t NUM_FLOWS = 1024;
const u64 INTER_PROBE_TIME_NS = 10*NSEC_PER_SEC;
const u64 MAX_PROBE_TIME_NS = NSEC_PER_SEC;


struct mpc_flow {
	bool used;

	u32 addr;
	struct mpc_flow *next;

	struct sk_buff *head;
	struct sk_buff *tail;
	size_t qlen;

	// The pacing rate to send in bytes per second.
	u64 rate;

	// The time we sent, or will send, the last packet at.
	u64 last_time_to_send;

	// The time at which the next packet should be sent.
	u64 time_to_send;

	// Last smoothed rtt seen by the model.
	u32 last_srtt;

	bool probing;
	u64 target_lat;
	union {
		u64 probe_time_to_start;
		u64 probe_time_to_stop;
	};

	struct model md;
};


struct mpc_sched_data {
	// The flows. Currently index 0 is for unclassified flows, and is also the
	// head of the flow chain.
	struct mpc_flow *flows;
	struct mpc_flow *tail;
	struct mpc_flow *current_flow;

	// Watchdog to set a timeout.
	struct qdisc_watchdog watchdog;
};


static void flow_init(struct mpc_flow *flow)
{
	flow->used = true;

	flow->addr = 0;
	flow->next = NULL;

	flow->head = NULL;
	flow->tail = NULL;
	flow->qlen = 0;

	flow->last_time_to_send = 0;

	flow->last_srtt = 0;

	flow->probing = false;
	flow->probe_time_to_start = 0;
	flow->probe_time_to_stop = 0;

	model_init(&flow->md, 100, 10, 50, 50, 5, 1);
}

static void flow_release(struct mpc_flow *flow)
{
	flow->used = false;
	model_release(&flow->md);
}


static void flow_enqueue(struct mpc_flow *flow, struct sk_buff *skb)
{
	if (flow->head == NULL)
		flow->head = skb;
	else
		flow->tail->next = skb;

	flow->tail = skb;
	skb->next = NULL;

	flow->qlen++;
}

inline static struct sk_buff *flow_peek(struct mpc_flow *flow)
{
	return flow->head;
}

static struct sk_buff *flow_dequeue(struct mpc_flow *flow)
{
	struct sk_buff *skb = flow->head;

	if (skb != NULL) {
		flow->head = skb->next;
		skb->next = NULL;

		if (flow->head == NULL)
			flow->tail = NULL;

		flow->qlen--;
	}

	return skb;
}


static void flow_update_time_to_send(struct mpc_flow *flow, u64 time)
{
	u64 min_delay;

	if (flow->head != NULL) {
		if (flow->rate == 0)
			min_delay = 0;
		else
			min_delay = NSEC_PER_SEC * flow->head->len / flow->rate;

		// Take the max here to ensure that we don't go past the max rate on a
		// burst.
		flow->last_time_to_send = max_t(u64, time, flow->last_time_to_send) + min_delay;
		flow->time_to_send = flow->last_time_to_send;
	}
}

static void flow_update_rate(struct mpc_flow *flow, u64 srtt_us, u64 time)
{
	u64 rtt;

	// srtt' = (1 - a) * srtt + a * rtt
	// rtt = srtt + (srtt' - srtt)/a
	// Reading the sources tells us that a = 1/8.
	// We have to make sure we get a positive rtt, otherwise estimate as
	// srtt.
	//
	// TODO: Don't hardcode alpha.
	if (flow->last_srtt + (srtt_us<<3) > (flow->last_srtt<<3))
		rtt = flow->last_srtt + (srtt_us<<3) - (flow->last_srtt<<3);
	else
		rtt = srtt_us;

	flow->last_srtt = srtt_us;


	if(!flow->probing && time >= flow->probe_time_to_start) {
		flow->probing = true;
		flow->probe_time_to_stop = time + MAX_PROBE_TIME_NS;
		flow->target_lat = 2*rtt;
	}

	if(flow->probing) {
		if(rtt >= flow->target_lat || time >= flow->probe_time_to_stop) {
			flow->probing = false;
			flow->probe_time_to_start = time + INTER_PROBE_TIME_NS;
		} else {
			flow->rate = control_process(&flow->md, rtt,
						2*flow->rate);
		}
	} else {
		flow->rate = control_process(&flow->md, rtt, 0);
	}
}


// Returns the next flow that needs to send a packet at time. If not such flow
// exists NULL is returned.
static struct mpc_flow *mpc_get_next_flow(struct Qdisc *sch, u64 time)
{
	struct mpc_sched_data *q = qdisc_priv(sch);
	struct mpc_flow *flow = q->current_flow;

	while (flow != NULL) {
		struct sk_buff *skb = flow_peek(flow);

		if (flow->time_to_send <= time && skb != NULL) {
			q->current_flow = flow;
			return flow;
		}

		flow = flow->next;
	}

	flow = &q->flows[0];

	while (flow != NULL && flow != q->current_flow) {
		struct sk_buff *skb = flow_peek(flow);

		if (flow->time_to_send <= time && skb != NULL) {
			q->current_flow = flow;
			return flow;
		}

		flow = flow->next;
	}

	return NULL;
}


// Returns the flow that skb is a part of. Creates a new flow if necessary.
static struct mpc_flow *mpc_classify(struct Qdisc *sch, struct sk_buff *skb)
{
	// TODO: Reclaim flows. Also, I'm classifying by destination address. fq
	// classifies by socket, and fq_codel uses a classiful qdisc (even though
	// it isn't classful, *it's wierd*).
	struct mpc_sched_data *q = qdisc_priv(sch);
	struct mpc_flow *unused_flow = NULL;
	int i;

	if (skb->sk == NULL)
		goto class_default;

	for (i = 1; i < NUM_FLOWS; i++) {
		if (q->flows[i].used && q->flows[i].addr == skb->sk->sk_daddr)
			return &q->flows[i];
		else if (! q->flows[i].used)
			unused_flow = &q->flows[i];
	}

	if (unused_flow != NULL) {
		flow_init(unused_flow);
		unused_flow->addr = skb->sk->sk_daddr;
		q->tail->next = unused_flow;
		q->tail = unused_flow;
		return unused_flow;
	}

class_default:
	// No unused flows, default to 0.
	return &q->flows[0];
}


static int mpc_enqueue(struct sk_buff *skb, struct Qdisc *sch,
		struct sk_buff **to_free)
{
	struct mpc_sched_data *q = qdisc_priv(sch);
	struct mpc_flow *flow = mpc_classify(sch, skb);
	bool update_time = flow->head == NULL;

	if (flow->qlen == sch->limit) {
		return qdisc_drop(skb, sch, to_free);
	} else if (skb != NULL) {
		u64 now = ktime_get_ns();

		flow_enqueue(flow, skb);

		if (update_time)
			flow_update_time_to_send(flow, now);
	}

	return 0;
}


static struct sk_buff *mpc_dequeue(struct Qdisc *sch)
{
	struct mpc_sched_data *q = qdisc_priv(sch);
	struct mpc_flow *flow;
	struct sk_buff *skb;
	u64 now = ktime_get_ns();
	u64 next_tts = 0;
	size_t i;

	flow = mpc_get_next_flow(sch, now);

	if (flow == NULL) {
		skb = NULL;
		goto exit_dequeue;
	}

	skb = flow_dequeue(flow);

	// Update rate using MPC.
	if (skb != NULL && skb->sk != NULL
		&& skb->sk->sk_protocol == IPPROTO_TCP) {
		// NOTE: There is a need to condsider this for different flows.
		struct tcp_sock *tp = tcp_sk(skb->sk);

		flow_update_rate(flow, tp->srtt_us, now);
	}

exit_dequeue:
	for (i = 0; i < NUM_FLOWS; i++) {
		if (next_tts == 0)
			next_tts = q->flows[i].time_to_send;
		else
			next_tts = min_t(u64, next_tts, q->flows[i].time_to_send);
	}

	if (next_tts > 0)
		qdisc_watchdog_schedule_ns(&q->watchdog, next_tts);
	else
		qdisc_watchdog_cancel(&q->watchdog);

	// Returns a packet to send out, NULL if we don't send out any.
	return skb;
}


static struct sk_buff *mpc_peek(struct Qdisc *sch) {
	struct mpc_sched_data *q = qdisc_priv(sch);

	return flow_peek(q->current_flow);
}


static const struct nla_policy mpc_policy[TCA_MPC_MAX + 1] = {
	[TCA_MPC_LIMIT]			= { .type = NLA_U32 },
};

#if LINUX_VERSION_CODE <= KERNEL_VERSION(4,16,0)
static int mpc_change(struct Qdisc *sch, struct nlattr *opt)
#else
static int mpc_change(struct Qdisc *sch, struct nlattr *opt,
		struct netlink_ext_ack *extack)
#endif
{
	//struct mpc_sched_data *q = qdisc_priv(sch);
	struct nlattr *tb[TCA_MPC_MAX + 1];
	int err;

	if (opt == NULL)
		return -EINVAL;

	err = nla_parse_nested(tb, TCA_MPC_MAX, opt, mpc_policy, NULL);
	if (err < 0)
		return err;

	if (tb[TCA_MPC_LIMIT])
		sch->limit = nla_get_u32(tb[TCA_MPC_LIMIT]);

	return err;
}

// FIXME: This is a quick fix to make it compile on 4.15 and 4.17.
#if LINUX_VERSION_CODE <= KERNEL_VERSION(4,16,0)
static int mpc_init(struct Qdisc *sch, struct nlattr *opt)
#else
#endif
{
	struct mpc_sched_data *q = qdisc_priv(sch);
	size_t i;

	q->flows = kmalloc(NUM_FLOWS * sizeof(struct mpc_flow), GFP_KERNEL);

	for (i = 0; i < NUM_FLOWS; i++)
		q->flows[i].used = false;

	flow_init(&q->flows[0]);
	q->tail = &q->flows[0];
	q->current_flow = q->tail;

	qdisc_watchdog_init(&q->watchdog, sch);

	sch->limit = qdisc_dev(sch)->tx_queue_len;

	if (opt) {
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
	//struct mpc_sched_data *q = qdisc_priv(sch);
	struct nlattr *opts;

	opts = nla_nest_start(skb, TCA_OPTIONS);
	if (opts == NULL)
		goto nla_put_failure;

	if (nla_put_u32(skb, TCA_MPC_LIMIT, sch->limit))
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
	size_t i;

	for (i = 0; i < NUM_FLOWS; i++) {
		if (q->flows[i].used)
			flow_release(&q->flows[i]);
	}

	kfree(q->flows);
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

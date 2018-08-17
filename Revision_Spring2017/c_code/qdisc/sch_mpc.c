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
	struct list_head send_list;

	struct sk_buff *head;
	struct sk_buff *tail;
	size_t qlen;

	// The pacing rate to send in bytes per second.
	u64 rate;

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
	// The flows. This is a hash map from destination addresses to flows.
	struct mpc_flow *flows;

	// Flow to use when classification fails or we are at our limit.
	struct mpc_flow def_flow;

	// Watchdog to set a timeout.
	struct qdisc_watchdog watchdog;

	// A list of flows that need to send packets.
	struct list_head sending;
};


static void flow_init(struct mpc_flow *flow)
{
	flow->used = true;

	flow->addr = 0;
	INIT_LIST_HEAD(&flow->send_list);

	flow->head = NULL;
	flow->tail = NULL;
	flow->qlen = 0;

	flow->rate = 0;
	flow->time_to_send = 0;

	flow->last_srtt = 0;

	flow->probing = false;
	flow->target_lat = 0;
	flow->probe_time_to_start = 0;

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


static void flow_update_time_to_send(struct mpc_flow *flow, u64 now)
{
	u64 min_delay;

	if (flow->rate == 0)
		min_delay = 0;
	else
		min_delay = NSEC_PER_SEC * flow->head->len / flow->rate;

	// Take the max here to ensure that we don't go past the max rate on a
	// burst.
	flow->time_to_send = max_t(u64, now, flow->time_to_send + min_delay);
}

static void flow_update_rate(struct mpc_flow *flow, u64 srtt_us, u64 time)
{
	// srtt' = (1 - a) * srtt + a * rtt
	// rtt = srtt + (srtt' - srtt)/a
	// Reading the sources tells us that a = 1/8.
	// We have to make sure we get a positive rtt, otherwise estimate as
	// srtt.
	//
	// TODO: Don't hardcode alpha.
	u64 rtt;
	u64 t1 = flow->last_srtt + 8*srtt_us;
	u64 t2 = 8*flow->last_srtt;

	if (t1 > t2)
		rtt = t1 - t2;
	else
		rtt = 0;

	flow->last_srtt = srtt_us;


	if (!flow->probing && time >= flow->probe_time_to_start) {
		flow->probing = true;
		flow->probe_time_to_stop = time + MAX_PROBE_TIME_NS;
		flow->target_lat = 2*rtt;
	}

	if (flow->probing) {
		if (rtt >= flow->target_lat || time >= flow->probe_time_to_stop) {
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


// Returns the flow that skb is a part of. Creates a new flow if necessary.
static struct mpc_flow *mpc_classify(struct Qdisc *sch, struct sk_buff *skb)
{
	// NOTE: I'm classifying by destination address. fq classifies by
	// socket, and fq_codel uses a classiful qdisc (even though it isn't
	// classful, *it's wierd*).
	struct mpc_sched_data *q = qdisc_priv(sch);
	struct mpc_flow *flow = NULL;
	size_t cnt = 0;
	size_t i;

	if (skb->sk == NULL)
		goto class_default;

	i = skb->sk->sk_daddr % NUM_FLOWS;

	while (flow == NULL && cnt < NUM_FLOWS) {
		if (!q->flows[i].used || q->flows[i].addr == skb->sk->sk_daddr) {
			flow = &q->flows[i];
		} else if (q->flows[i].qlen == 0) {
			// Reclaim unused flows.
			flow_release(&q->flows[i]);
			flow = &q->flows[i];
		} else {
			i = (i + 1) % NUM_FLOWS;
		}

		cnt++;
	}

	if (flow != NULL) {
		if (!flow->used) {
			flow_init(flow);
			flow->addr = skb->sk->sk_daddr;
		}

		return flow;
	}

class_default:
	return &q->def_flow;
}


// Add a flow to the list of flows that have a packet to send.
static void mpc_add_flow(struct Qdisc *sch, struct mpc_flow *flow)
{
	struct mpc_sched_data *q = qdisc_priv(sch);
	struct mpc_flow *it;
	bool add = true;

	list_for_each_entry(it, &q->sending, send_list) {
		if (it == flow)
			add = false;
	}

	if (add)
		list_add_tail(&flow->send_list, &q->sending);
}


// Add a flow to the list of flows that have a packet to send.
static void mpc_del_flow(struct Qdisc *sch, struct mpc_flow *flow)
{
	list_del(&flow->send_list);
}


// Returns the next flow that needs to send a packet at time. If not such flow
// exists NULL is returned.
static struct mpc_flow *mpc_get_next_flow(struct Qdisc *sch)
{
	struct mpc_sched_data *q = qdisc_priv(sch);
	struct mpc_flow *flow = NULL;
	struct mpc_flow *it = NULL;
	u64 tts = U64_MAX;

	list_for_each_entry(it, &q->sending, send_list) {
		if (it->time_to_send < tts) {
			flow = it;
			tts = it->time_to_send;
		}
	}

	return flow;
}


// Update watchdog to dequeue when the next packet is ready.
static void mpc_update_watchdog(struct Qdisc *sch, u64 now)
{
	struct mpc_sched_data *q = qdisc_priv(sch);
	struct mpc_flow *flow = mpc_get_next_flow(sch);

	if (flow != NULL)
		qdisc_watchdog_schedule_ns(&q->watchdog, flow->time_to_send);
	else
		qdisc_watchdog_cancel(&q->watchdog);
}


static int mpc_enqueue(struct sk_buff *skb, struct Qdisc *sch,
		struct sk_buff **to_free)
{
	struct mpc_flow *flow = mpc_classify(sch, skb);

	if (flow->qlen == sch->limit) {
		return qdisc_drop(skb, sch, to_free);
	} else if (skb != NULL) {
		u64 now = ktime_get_ns();

		flow_enqueue(flow, skb);
		flow_update_time_to_send(flow, now);
		mpc_add_flow(sch, flow);
		mpc_update_watchdog(sch, now);
	}

	return 0;
}


static struct sk_buff *mpc_dequeue(struct Qdisc *sch)
{
	struct mpc_flow *flow;
	struct sk_buff *skb = NULL;
	u64 now = ktime_get_ns();

	flow = mpc_get_next_flow(sch);

	if (flow == NULL || flow->time_to_send > now) {
		skb = NULL;
		goto exit_dequeue;
	}

	skb = flow_dequeue(flow);

	if (flow->qlen == 0)
		mpc_del_flow(sch, flow);
	else
		flow_update_time_to_send(flow, now);

	// Update rate using MPC.
	if (skb != NULL && skb->sk != NULL
		&& skb->sk->sk_protocol == IPPROTO_TCP) {
		struct tcp_sock *tp = tcp_sk(skb->sk);

		flow_update_rate(flow, tp->srtt_us, now);
	}

exit_dequeue:
	mpc_update_watchdog(sch, now);

	// Returns a packet to send out, NULL if we don't send out any.
	return skb;
}


static struct sk_buff *mpc_peek(struct Qdisc *sch) {
	struct mpc_flow *flow = mpc_get_next_flow(sch);

	if (flow != NULL)
		return flow_peek(flow);
	else
		return NULL;
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

	flow_init(&q->def_flow);

	qdisc_watchdog_init(&q->watchdog, sch);
	INIT_LIST_HEAD(&q->sending);

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

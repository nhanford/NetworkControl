/*
 * net/sched/mpc_cc.c	A congestion control algorithm based on model predictive
 * control.
 *
 *		This program is free software; you can redistribute it and/or
 *		modify it under the terms of the GNU General Public License
 *		as published by the Free Software Foundation; either version
 *		2 of the License, or (at your option) any later version.
 *
 * Authors:	Taran Lynn <taranlynn0@gmail.com>
 */

#include <linux/module.h>
#include <net/tcp.h>

#include "../mpc/control.h"


static int weight = 10;
static int learn_rate = 10;
static int over = 1000;
static int c1 = 10;
static int c2 = 10;

module_param(weight, int, 0644);
MODULE_PARM_DESC(weight, "weight for moving averages (in %)");

module_param(learn_rate, int, 0644);
MODULE_PARM_DESC(learn_rate, "learning rate (in %)");

module_param(over, int, 0644);
MODULE_PARM_DESC(over, "how far over minimum RTT we should target (in us)");

module_param(c1, int, 0644);
MODULE_PARM_DESC(c1, "weight for reducing RTT variance (in %)");

module_param(c2, int, 0644);
MODULE_PARM_DESC(c2, "weight for reducing control action (in %)");


struct control {
	struct model *md;
	u32 rate;
	u64 losses;
	u64 mss;
	u64 start_time;
};

static struct mpc_dfs debugfs;


scaled get_loss_rate(struct control *ctl)
{
	u64 now = ktime_get_ns();

	scaled rate = SD(SM(SFI(ctl->losses, 0), SFI(ctl->mss, 0)),
		SD(SFI(now - ctl->start_time, 0), SFI(NSEC_PER_SEC, 0)));

	return rate;
}


// Set the pacing rate. rate is in bytes/sec.
inline void set_rate(struct sock *sk) {
	struct control *ctl = inet_csk_ca(sk);
	struct tcp_sock *tp = tcp_sk(sk);

	sk->sk_pacing_rate = ctl->rate;

	tp->snd_cwnd = 10000;//max_t(u32, 1, (ctl->rate / tp->mss_cache)
			//* tp->srtt_us / USEC_PER_SEC);
}

void mpc_cc_init(struct sock *sk)
{
	struct control *ctl = inet_csk_ca(sk);
	struct tcp_sock *tp = tcp_sk(sk);

	tp->snd_ssthresh = TCP_INFINITE_SSTHRESH;

	ctl->md = kmalloc(sizeof(struct model), GFP_KERNEL);
	ctl->losses = 0;
	ctl->mss = tp->mss_cache;
	ctl->start_time = ktime_get_ns();
	model_init(ctl->md,
		scaled_from_frac(weight, 100),
		scaled_from_frac(learn_rate, 100),
		scaled_from_int(over, 1),
		scaled_from_frac(c1, 100),
		scaled_from_frac(c2, 100));

	mpc_dfs_register(&debugfs, &ctl->md->stats);
}


void mpc_cc_release(struct sock *sk)
{
	struct control *ctl = inet_csk_ca(sk);

	if (ctl->md != NULL) {
		model_release(ctl->md);
		mpc_dfs_unregister(&debugfs, &ctl->md->stats);
		kfree(ctl->md);
	}
}


u32 mpc_cc_ssthresh(struct sock *sk)
{
	struct tcp_sock *tp = tcp_sk(sk);

	return tp->snd_ssthresh;
}


void mpc_cc_avoid(struct sock *sk, u32 ack, u32 acked)
{
}


u32 mpc_cc_undo_cwnd(struct sock *sk)
{
	struct control *ctl = inet_csk_ca(sk);
	struct tcp_sock *tp = tcp_sk(sk);

	return tp->snd_cwnd;
}


void mpc_cc_pkts_acked(struct sock *sk, const struct ack_sample *sample)
{
	// sample->rtt_us = RTT of acknowledged packet.
}


void mpc_cc_main(struct sock *sk, const struct rate_sample *rs)
{
	struct control *ctl = inet_csk_ca(sk);
	struct tcp_sock *tp = tcp_sk(sk);

	// rs->rtt_us = RTT of last packet to be acknowledged.
	// tp->srtt_us = WMA of RTT
	// tp->tp->mdev_us = Variance of WMA of RTT

	ctl->mss = tp->mss_cache;
	ctl->losses += rs->losses;

	if (ctl->md != NULL) {
		ctl->rate = STI(control_process(ctl->md, SFI(0, 0),
						SFI(ctl->rate, 0),
						get_loss_rate(ctl)));
		set_rate(sk);
	} else {
		mpc_cc_init(sk);
	}
}


static struct tcp_congestion_ops tcp_mpc_cc_cong_ops __read_mostly = {
	.flags          = TCP_CONG_NON_RESTRICTED,
	.name           = "mpc_cc",
	.owner          = THIS_MODULE,

	.init           = mpc_cc_init,
	.release        = mpc_cc_release,
	.ssthresh       = mpc_cc_ssthresh,
	.cong_avoid     = mpc_cc_avoid,
	.undo_cwnd      = mpc_cc_undo_cwnd,
	.pkts_acked     = mpc_cc_pkts_acked,
	.cong_control   = mpc_cc_main,
};

static int __init mpc_cc_mod_init(void)
{
	printk(KERN_INFO "mpc_cc: module init\n");
	tcp_register_congestion_control(&tcp_mpc_cc_cong_ops);

	mpc_dfs_init(&debugfs);

	return 0;
}

static void __exit mpc_cc_mod_exit(void)
{
	printk(KERN_INFO "mpc_cc: module exit\n");
	tcp_unregister_congestion_control(&tcp_mpc_cc_cong_ops);
	mpc_dfs_release(&debugfs);
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Taran Lynn");
MODULE_DESCRIPTION("Model Predictive Congestion Control");
MODULE_VERSION("0.01");

module_init(mpc_cc_mod_init);
module_exit(mpc_cc_mod_exit);

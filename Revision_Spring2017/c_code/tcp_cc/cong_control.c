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

#include <linux/kobject.h>
#include <linux/module.h>
#include <linux/sysctl.h>
#include <net/tcp.h>

#include "../mpc/control.h"
#include "../mpc/sysfs.h"

static struct kset *mpc_kset;
static struct mpc_dfs debugfs;

struct control {
	struct model md;
	struct mpc_settings settings;
	u32 rate;
};


inline struct control* get_control(struct sock *sk) {
	struct control **ctlptr = inet_csk_ca(sk);
	return *ctlptr;
}

// Set the pacing rate. rate is in bytes/sec.
inline void set_rate(struct sock *sk) {
	struct control *ctl = get_control(sk);
	struct tcp_sock *tp = tcp_sk(sk);

	sk->sk_pacing_rate = ctl->rate;

	tp->snd_cwnd = 10000;//max_t(u32, 1, (ctl->rate / tp->mss_cache)
			//* tp->srtt_us / USEC_PER_SEC);
}


void mpc_cc_init(struct sock *sk)
{
	struct control **ctlptr = inet_csk_ca(sk);
	struct tcp_sock *tp = tcp_sk(sk);
	struct control *ctl = kzalloc(sizeof(struct control), GFP_KERNEL);
	u32 addr = be32_to_cpu(sk->sk_daddr);
	int retval;

	if (ctl == NULL) {
		*ctlptr = NULL;
		return;
	}

	*ctlptr = ctl;

	tp->snd_ssthresh = TCP_INFINITE_SSTHRESH;


	mpc_sysfs_register(&ctl->settings, mpc_kset, addr, sk->sk_num,
		10, 10, 200, 1 << 10, 25 << 10, 400000, 10000);

	model_init(&ctl->md,
		scaled_from_frac(ctl->settings.weight, 100),
		5 << 3,
		5,
		scaled_from_frac(ctl->settings.learn_rate, 100),
		// Convert mbits to bytes.
		scaled_from_int(ctl->settings.min_rate, 20-3),
		scaled_from_int(ctl->settings.max_rate, 20-3),
		scaled_from_int(ctl->settings.over, 0),
		scaled_from_frac(ctl->settings.c1, 1000000),
		scaled_from_frac(ctl->settings.c2, 1000000));

	mpc_dfs_register(&debugfs, &ctl->md.stats);
}


void mpc_cc_release(struct sock *sk)
{
	struct control **ctlptr = inet_csk_ca(sk);
	struct control *ctl = *ctlptr;

	if (ctl != NULL) {
		mpc_dfs_unregister(&debugfs, &ctl->md.stats);
		model_release(&ctl->md);
		mpc_sysfs_unregister(&ctl->settings);
	}

	*ctlptr = NULL;
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
	struct tcp_sock *tp = tcp_sk(sk);
	return tp->snd_cwnd;
}


void mpc_cc_pkts_acked(struct sock *sk, const struct ack_sample *sample)
{
	// sample->rtt_us = RTT of acknowledged packet.
}


void mpc_cc_main(struct sock *sk, const struct rate_sample *rs)
{
	struct control *ctl = get_control(sk);
	u64 now = ktime_get_ns();

	// rs->rtt_us = RTT of last packet to be acknowledged.
	// tp->srtt_us = WMA of RTT
	// tp->tp->mdev_us = Variance of WMA of RTT

	if (ctl != NULL && rs->rtt_us > 0) {
		mpc_sysfs_set_model(&ctl->settings, &ctl->md);

		ctl->rate = STI(control_process(&ctl->md,
						SFI(now/NSEC_PER_USEC, 0),
						SFI(ctl->rate, 0),
						SFI(rs->rtt_us, 0)));
		set_rate(sk);
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
	printk(KERN_INFO "mpc: module init\n");

	mpc_kset = kset_create_and_add("mpccc", NULL, kernel_kobj);
	if (!mpc_kset)
		return -ENOMEM;

	mpc_dfs_init(&debugfs);
	tcp_register_congestion_control(&tcp_mpc_cc_cong_ops);

	return 0;
}

static void __exit mpc_cc_mod_exit(void)
{
	printk(KERN_INFO "mpc: module exit\n");
	tcp_unregister_congestion_control(&tcp_mpc_cc_cong_ops);
	mpc_dfs_release(&debugfs);
	kset_unregister(mpc_kset);
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Taran Lynn");
MODULE_DESCRIPTION("Model Predictive Congestion Control");
MODULE_VERSION("0.01");

module_init(mpc_cc_mod_init);
module_exit(mpc_cc_mod_exit);

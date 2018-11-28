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


struct control {
	struct model *md;
	u32 rate;
};


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
	model_init(ctl->md,
		scaled_from_int(100, 20),
		5 << 3,
		2,
		scaled_from_int(1, -8),
		scaled_from_int(500, 0),
		scaled_from_int(255, -8));
}


void mpc_cc_release(struct sock *sk)
{
	struct control *ctl = inet_csk_ca(sk);

	if (ctl->md != NULL) {
		model_release(ctl->md);
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

	if (ctl->md != NULL && rs->rtt_us > 0) {
		ctl->rate = STI(control_process(ctl->md, SFI(0, 0),
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
	tcp_register_congestion_control(&tcp_mpc_cc_cong_ops);

	return 0;
}

static void __exit mpc_cc_mod_exit(void)
{
	printk(KERN_INFO "mpc: module exit\n");
	tcp_unregister_congestion_control(&tcp_mpc_cc_cong_ops);
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Taran Lynn");
MODULE_DESCRIPTION("Model Predictive Congestion Control");
MODULE_VERSION("0.01");

module_init(mpc_cc_mod_init);
module_exit(mpc_cc_mod_exit);

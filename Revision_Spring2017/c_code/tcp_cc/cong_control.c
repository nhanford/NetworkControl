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

#define RATE (1 << 0)
#define mpc_cc_log(args, ...) printk(KERN_INFO "mpc_cc: " args, ##__VA_ARGS__)


struct control {
    struct model *md;
};


inline void set_rate(struct sock * sk, u32 rate) {
    struct tcp_sock *tp = tcp_sk(sk);

    sk->sk_pacing_rate = rate;
    sk->sk_max_pacing_rate = rate;
    tp->snd_cwnd = max_t(u32, 1, rate * tp->srtt_us / USEC_PER_SEC);
}

void mpc_cc_init(struct sock *sk)
{
    struct control *ctl = inet_csk_ca(sk);
    ctl->md = kmalloc(sizeof(struct model), GFP_KERNEL);
    model_init(ctl->md, 100, 10, 50, 50, 5, 1);

    mpc_cc_log("init\n");
}


void mpc_cc_release(struct sock *sk)
{
    struct control *ctl = inet_csk_ca(sk);
    model_release(ctl->md);
    kfree(ctl->md);

    mpc_cc_log("release\n");
}


u32 mpc_cc_ssthresh(struct sock *sk)
{
    u32 ret = tcp_reno_ssthresh(sk);

    mpc_cc_log("ssthresh\n");
    return ret;
}


void mpc_cc_avoid(struct sock *sk, u32 ack, u32 acked)
{
    mpc_cc_log("avoid\n");
    tcp_reno_cong_avoid(sk, ack, acked);
}


u32 mpc_cc_undo_cwnd(struct sock *sk)
{
    u32 ret = tcp_reno_undo_cwnd(sk);

    mpc_cc_log("undo\n");

    return ret;
}


void mpc_cc_main(struct sock *sk, const struct rate_sample *rs)
{
    struct control *ctl = inet_csk_ca(sk);
    struct tcp_sock *tp = tcp_sk(sk);

    if(ctl->md == NULL)
        mpc_cc_init(sk);

    mpc_cc_log("main: srtt_us = %u, rack->rtt_us = %u, rcv_rtt_est = %u\n"
            "rs->rtt_us = %lu, mdev_us = %u\n"
            "sk_pacing_rate = %u, sk_max_pacing_rate = %u\n",
            tp->srtt_us, tp->rack.rtt_us, tp->rcv_rtt_est.rtt_us,
            rs->rtt_us, tp->mdev_us,
            sk->sk_pacing_rate, sk->sk_max_pacing_rate);

    mpc_cc_log("snd_cwnd = %u, sk_pacing_status = %u\n", tp->snd_cwnd, sk->sk_pacing_status);

    //set_rate(sk, control_process(ctl->md, rs->rtt_us));
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
    .cong_control   = mpc_cc_main,
};

static int __init mpc_cc_mod_init(void)
{
    mpc_cc_log("mod_init\n");
    tcp_register_congestion_control(&tcp_mpc_cc_cong_ops);

    return 0;
}

static void __exit mpc_cc_mod_exit(void)
{
    mpc_cc_log("mod_exit\n");
    tcp_unregister_congestion_control(&tcp_mpc_cc_cong_ops);
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Taran Lynn");
MODULE_DESCRIPTION("Model Predictive Congestion Control");
MODULE_VERSION("0.01");

module_init(mpc_cc_mod_init);
module_exit(mpc_cc_mod_exit);

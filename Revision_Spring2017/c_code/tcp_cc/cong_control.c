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

#define RATE_GAIN (100 << 10)
#define PROBING_PERIOD 40
#define mpc_cc_log(args, ...) printk(KERN_INFO "mpc cc: " args, ##__VA_ARGS__)


struct control {
    struct model *md;

    u32 rate;
    bool probing;
    u32 count_down;
};


// Set the pacing rate. rate is in bytes/sec.
inline void set_rate(struct sock * sk) {
    struct control *ctl = inet_csk_ca(sk);
    struct tcp_sock *tp = tcp_sk(sk);

    sk->sk_pacing_rate = ctl->rate;

    tp->snd_cwnd = max_t(u32, 1, (ctl->rate / tp->mss_cache)
            * tp->srtt_us / USEC_PER_SEC);

    mpc_cc_log("rate = %u, srtt_us= %u, mss = %u, snd_cwnd = %u\n",
            ctl->rate, tp->srtt_us, tp->mss_cache, tp->snd_cwnd);
}

void mpc_cc_init(struct sock *sk)
{
    struct control *ctl = inet_csk_ca(sk);

    ctl->md = kmalloc(sizeof(struct model), GFP_KERNEL);
    model_init(ctl->md, real_from_frac(1, 1), real_from_frac(1, 10),
            real_from_frac(1, 2), real_from_frac(1, 2), 5, 1);

    ctl->probing = false;
    ctl->count_down = 0;
}


void mpc_cc_release(struct sock *sk)
{
    struct control *ctl = inet_csk_ca(sk);

    if(ctl->md != NULL) {
        model_release(ctl->md);
        kfree(ctl->md);
    }
}


u32 mpc_cc_ssthresh(struct sock *sk)
{
    u32 ret = tcp_reno_ssthresh(sk);
    set_rate(sk);

    return ret;
}


void mpc_cc_avoid(struct sock *sk, u32 ack, u32 acked)
{
    tcp_reno_cong_avoid(sk, ack, acked);
    set_rate(sk);
}


u32 mpc_cc_undo_cwnd(struct sock *sk)
{
    struct tcp_sock *tp = tcp_sk(sk);
    // Should probably not just override this return value.
    u32 ret = tcp_reno_undo_cwnd(sk);

    set_rate(sk);

    return tp->snd_cwnd;
}


void mpc_cc_pkts_acked(struct sock *sk, const struct ack_sample *sample)
{
    // sample->rtt_us = RTT of acknowledged packet.
    set_rate(sk);
}


void mpc_cc_main(struct sock *sk, const struct rate_sample *rs)
{
    struct control *ctl = inet_csk_ca(sk);
    struct tcp_sock *tp = tcp_sk(sk);

    // rs->rtt_us = RTT of last packet to be acknowledged.
    // tp->srtt_us = WMA of RTT
    // tp->tp->mdev_us = Variance of WMA of RTT

    mpc_cc_log("main, srtt_us = %u, rs->rtt_us = %lu, sk_pacing_rate = %u,"
            " snd_cwnd = %u, mss = %u\n",
            tp->srtt_us, rs->rtt_us, sk->sk_pacing_rate,
            tp->snd_cwnd, tp->mss_cache);

    if(ctl->md != NULL) {
        u32 rtt_us = rs->rtt_us;

        if(ctl->count_down == 0) {
            // Here we're probing. We increase the rate until packet losses are
            // detected

            ctl->rate = real_floor(control_process(ctl->md,
                        real_from_frac(rtt_us, USEC_PER_SEC),
                        real_from_int(RATE_GAIN)));

            if(ctl->probing && rs->losses > 0) {
                ctl->probing = false;
                ctl->count_down = PROBING_PERIOD;
            } else {
                ctl->probing = true;
            }
        } else {
            ctl->rate = real_floor(control_process(ctl->md,
                        real_from_frac(rtt_us, USEC_PER_SEC),
                        REAL_ZERO));
            ctl->count_down -= 1;
        }
    }

    set_rate(sk);
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

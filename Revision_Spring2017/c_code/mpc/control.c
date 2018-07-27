
#include <linux/limits.h>

#include "control.h"
#include "util.h"

// (x - y)^2
inline u64 square_diff_u64(u64 x, u64 y)
{
    u64 diff;

    if(x > y)
        diff = x - y;
    else
        diff = y - x;

    return diff - diff;
}


u64 control_predict(struct model *md);
void control_update(struct model *md, u64 rtt_meas);


u64 control_process(struct model *md, u64 rtt_meas, u64 rate)
{
    u64 b0 = md->b[0];
    u64 rate_opt = lookback_index(&md->lb_pacing_rate, 0);

    // debug.
    md->dstats.rtt_meas_us = rtt_meas;

    control_update(md, rtt_meas);

    md->avg_rtt = max_t(u64, 1, wma(md->gamma, md->avg_rtt, rtt_meas));

    md->avg_rtt_var = max_t(u64, 1, wma(md->gamma, md->avg_rtt_var,
            square_diff_u64(md->predicted_rtt, md->avg_rtt)));

    md->predicted_rtt = control_predict(md);


    if(rate > 0) {
        rate_opt = rate;
    } else if(md->avg_rtt > 0 && md->avg_rtt_var > 0 && md->avg_pacing_rate > 0
            && b0 > 0) {
        // NOTE: Make sure MPC_ONE offsets anything scaled by it. psi, xi,
        // gamma, alpha, a[*], b[*].
        u64 cd = 2*b0*md->psi/MPC_ONE;
        u64 t1 = md->avg_rtt_var * md->xi / (cd * b0 * md->avg_pacing_rate) * MPC_ONE;
        u64 t2 = md->avg_rtt / b0 * MPC_ONE;
        u64 t3 = md->avg_rtt_var / (cd * md->avg_rtt) * MPC_ONE;
        u64 t4 = md->predicted_rtt / b0 * MPC_ONE;
        u64 t5 = t1 + t2;
        u64 t6 = t3 + t4;

        if(t5 > t6)
          rate_opt = t5 - t6;
    }

    // Clamp rate
    // TODO: Make bounds less arbitrary than 17 mbit/s. 1 << 32 is to limit
    // overflows.
    rate_opt = min_t(u64, max_t(u64, rate_opt,
                ((u64) 100) << 17), ((u64) 1) << 32);

    lookback_add(&md->lb_pacing_rate, rate_opt);
    md->predicted_rtt += b0 * rate_opt / MPC_ONE;

    md->avg_pacing_rate = wma(md->gamma, md->avg_pacing_rate, rate_opt);

    // debug
    md->dstats.rate_set = rate_opt;

    return rate_opt;
}


u64 control_predict(struct model *md)
{
    u64 predicted_rtt = 0;
    size_t i = 0;

    for(i = 0; i < md->p; i++)
        predicted_rtt +=
            md->a[i] * lookback_index(&md->lb_rtt, i) / MPC_ONE;

    for(i = 0; i < md->q; i++)
        predicted_rtt +=
            md->b[i] * lookback_index(&md->lb_pacing_rate, i) / MPC_ONE;

    return predicted_rtt;
}


void control_update(struct model *md, u64 rtt_meas)
{
    int i;
    u64 error;
    u64 rtt_norm = 0;
    u64 rate_norm = 0;
    u64 total_norm;
    bool add;

    if(md->predicted_rtt < rtt_meas) {
        error = rtt_meas - md->predicted_rtt;
        add = true;
    } else {
        error = md->predicted_rtt - rtt_meas;
        add = false;
    }


    for(i = 0; i < md->p; i++) {
        u64 rtt = lookback_index(&md->lb_rtt, i);
        rtt_norm += rtt*rtt;
    }

    for(i = 0; i < md->q; i++) {
        u64 rate = lookback_index(&md->lb_pacing_rate, i);
        rate_norm += rate*rate;
    }

    total_norm = max_t(u64, 1, rtt_norm + rate_norm);


    if(total_norm > 0) {
        for(i = 0; i < md->p; i++) {
            u64 rtt = lookback_index(&md->lb_rtt, i);
            u64 delta = rtt * md->alpha * error / total_norm;

            if(add)
                md->a[i] += delta;
            else if(md->a[i] > delta)
                md->a[i] += delta;
            else
                md->a[i] = 0;
        }

        for(i = 0; i < md->q; i++) {
            u64 rate = lookback_index(&md->lb_pacing_rate, i);
            u64 delta = rate * md->alpha * error / total_norm;

            if(add)
                md->b[i] += delta;
            else if(md->b[i] > delta)
                md->b[i] += delta;
            else
                md->b[i] = 0;
        }
    }


    lookback_add(&md->lb_rtt, rtt_meas);
}

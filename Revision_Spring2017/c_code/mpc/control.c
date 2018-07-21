
#include "control.h"
#include "util.h"

// (x - y)^2
inline real square_diff_real(real x, real y)
{
    real diff;

    if(real_gt(x, y))
        diff = RS(x, y);
    else
        diff = RS(y, x);

    return RM(diff, diff);
}


real control_predict(struct model *md);
void control_update(struct model *md, real rtt_meas);


real_int control_process(struct model *md, real_int rtt_meas_us, real_int rate_gain_bs)
{
    // Normalize values for internal use. If we don't do this the values could
    // overflow.
    real rtt_meas = real_from_frac(rtt_meas_us, USEC_PER_SEC);
    real rate_gain = real_from_frac(rate_gain_bs, MB_PER_B);
    real_int ret_rate;

    real b0 = md->b[0];
    real rate_opt = REAL_ZERO;

    // debug.
    md->dstats.rtt_meas_us = rtt_meas_us;

    control_update(md, rtt_meas);

    md->avg_rtt = RA( RM(RS(REAL_ONE, md->gamma), rtt_meas),
        RM(md->gamma, md->avg_rtt) );

    md->avg_rtt_var =
        RA( RM(RS(REAL_ONE, md->gamma),
              square_diff_real(md->predicted_rtt, md->avg_rtt)),
            RM(md->gamma, md->avg_rtt_var));

    md->predicted_rtt = control_predict(md);


    if(real_gt(rate_gain, REAL_ZERO)) {
        rate_opt = RA(lookback_index(&md->lb_pacing_rate, 0), rate_gain);
    } else if(real_gt(md->avg_rtt, REAL_ZERO)
            && real_gt(md->avg_rtt_var, REAL_ZERO)
            && real_gt(md->avg_pacing_rate, REAL_ZERO)
            && real_gt(b0, REAL_ZERO)) {
        real cd = RM(real_ls(b0, 1), md->psi);
        real t1 = RD(RM(md->avg_rtt_var, md->xi), RM(RM(cd, b0), md->avg_pacing_rate));
        real t2 = RD(md->avg_rtt, b0);
        real t3 = RD(md->avg_rtt_var, RM(cd, md->avg_rtt));
        real t4 = RD(md->predicted_rtt, b0);
        rate_opt = RS(RA(t1, t2), RA(t3, t4));
    }

    // Clamp rate
    // TODO: Make bounds less arbitrary.
    if(real_lt(rate_opt, real_from_frac(100, 8)))
        rate_opt = real_from_frac(100, 8);

    lookback_add(&md->lb_pacing_rate, rate_opt);
    md->predicted_rtt = RA(md->predicted_rtt, RM(b0, rate_opt));

    md->avg_pacing_rate = RA(RM(RS(REAL_ONE, md->gamma), rate_opt),
        RM(md->gamma, md->avg_pacing_rate));

    ret_rate = real_floor(RM(rate_opt, real_from_int(MB_PER_B)));
    // debug
    md->dstats.rate_set = ret_rate;

    return ret_rate;
}


real control_predict(struct model *md)
{
    real predicted_rtt = REAL_ZERO;
    size_t i = 0;

    for(i = 0; i < md->p; i++)
        predicted_rtt = RA(predicted_rtt,
            RM(md->a[i], lookback_index(&md->lb_rtt, i)));

    for(i = 0; i < md->q; i++)
        predicted_rtt = RA(predicted_rtt,
            RM(md->b[i], lookback_index(&md->lb_pacing_rate, i)));

    return predicted_rtt;
}


void control_update(struct model *md, real rtt_meas)
{
    int i;
    real error;
    real rtt_norm = REAL_ZERO;
    real rate_norm = REAL_ZERO;
    real total_norm;
    bool add;

    if(real_lt(md->predicted_rtt, rtt_meas)) {
        error = RS(rtt_meas, md->predicted_rtt);
        add = true;
    } else {
        error = RS(md->predicted_rtt, rtt_meas);
        add = false;
    }


    for(i = 0; i < md->p; i++) {
        real rtt = lookback_index(&md->lb_rtt, i);
        rtt_norm = RA(rtt_norm, RM(rtt, rtt));
    }

    for(i = 0; i < md->q; i++) {
        real rate = lookback_index(&md->lb_pacing_rate, i);
        rate_norm = RA(rate_norm, RM(rate, rate));
    }

    total_norm = RA(rtt_norm, rate_norm);



    if(real_gt(total_norm, REAL_ZERO)) {
        for(i = 0; i < md->p; i++) {
            real rtt = lookback_index(&md->lb_rtt, i);
            real delta = RM(RM(RD(rtt, total_norm), md->alpha), error);

            if(add)
                md->a[i] = RA(md->a[i], delta);
            else if(real_gt(md->a[i], delta))
                md->a[i] = RS(md->a[i], delta);
            else
                md->a[i] = REAL_ZERO;
        }

        for(i = 0; i < md->q; i++) {
            real rate = lookback_index(&md->lb_pacing_rate, i);
            real delta = RM(RM(RD(rate, total_norm), md->alpha), error);

            if(add)
                md->b[i] = RA(md->b[i], delta);
            else if(real_gt(md->b[i], delta))
                md->b[i] = RS(md->b[i], delta);
            else
                md->b[i] = REAL_ZERO;
        }
    }


    lookback_add(&md->lb_rtt, rtt_meas);
}

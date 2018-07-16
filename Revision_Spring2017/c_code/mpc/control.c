
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

// Make a real at least 1 in its least place.
inline real atl1(real x)
{
  if(real_lt(x, (real) { 1 }))
    return (real) { 1 };
  else
    return x;
}


real control_predict(struct model *md);
void control_update(struct model *md, real rtt_meas);


real control_process(struct model *md, real rtt_meas)
{
    real psi, xi, b0, rate_opt;

    mpc_log("rtt_meas = %lluus\n", real_floor(RM(real_from_int(1000000), rtt_meas)));

    control_update(md, rtt_meas);

    md->avg_rtt = RA( RM(RS(REAL_ONE, md->gamma), rtt_meas),
        RM(md->gamma, md->avg_rtt) );

    md->avg_rtt_var =
        RA( RM(RS(REAL_ONE, md->gamma),
              square_diff_real(md->predicted_rtt, md->avg_rtt)),
            RM(md->gamma, md->avg_rtt_var));


    md->predicted_rtt = control_predict(md);


    psi = RD(md->psi, atl1(md->avg_rtt_var));
    xi = RD(md->xi, atl1(md->avg_pacing_rate));
    b0 = atl1(md->b[0]);

    // Tricky dealing with percetage.
    // The extra 2*'s are there to avoid 1/(psi*b0) potentially going to 0.5.
    rate_opt = RD(RA(RM(
            RS(xi, RD(b0, atl1(md->avg_rtt))),
            RD(REAL_ONE, RM(RM(real_from_int(1), atl1(psi)), b0))),
        RS(md->avg_rtt, md->predicted_rtt)), b0);

    // Clamp rate
    // TODO: Make bounds less arbitrary.
    if(real_lt(rate_opt, real_from_int((100<<20)>>3)))
        rate_opt = real_from_int((100<<20)>>3);

    lookback_add(&md->lb_pacing_rate, rate_opt);
    md->predicted_rtt = RA(md->predicted_rtt, RM(b0, rate_opt));

    md->avg_pacing_rate = RA(RM(RS(REAL_ONE, md->gamma), rate_opt),
        RM(md->gamma, md->avg_pacing_rate));

    mpc_log("rate_opt = %llu bytes/s\n", real_floor(rate_opt));

    return rate_opt;
}


real control_gain(struct model *md, real rtt_meas, real rate_gain)
{
    real rate_opt;

    control_update(md, rtt_meas);

    md->avg_rtt = RA( RM(RS(REAL_ONE, md->gamma), rtt_meas),
        RM(md->gamma, md->avg_rtt) );

    md->avg_rtt_var =
        RA( RM(RS(REAL_ONE, md->gamma),
              square_diff_real(md->predicted_rtt, md->avg_rtt)),
            RM(md->gamma, md->avg_rtt_var));


    md->predicted_rtt = control_predict(md);


    rate_opt = RA(lookback_index(&md->lb_pacing_rate, 0), rate_gain);


    lookback_add(&md->lb_pacing_rate, rate_opt);
    md->predicted_rtt = rate_opt;

    md->avg_pacing_rate = RA(RM(RS(REAL_ONE, md->gamma), rate_opt),
        RM(md->gamma, md->avg_pacing_rate));

    return rate_opt;
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
            real delta = RD(RM(md->alpha, RM(error, rtt)), total_norm);

            if(add)
              md->a[i] = RA(md->a[i], delta);
            else
              md->a[i] = RS(md->a[i], delta);
        }

        for(i = 0; i < md->q; i++) {
            real rate = lookback_index(&md->lb_pacing_rate, i);
            real delta = RD(RM(md->alpha, RM(error, rate)), total_norm);

            if(add)
                md->b[i] = RA(md->b[i], delta);
            else
                md->b[i] = RS(md->b[i], delta);
        }
    }


    lookback_add(&md->lb_rtt, rtt_meas);
}

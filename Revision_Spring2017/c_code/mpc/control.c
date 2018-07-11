
#include "control.h"

inline u32 square_diff_u32(u32 x, u32 y)
{
    u32 diff;

    if(x > y)
        diff = x - y;
    else
        diff = y - x;

    return diff;
}

// Make an integer at least 1.
inline u32 atl1(u32 x)
{
  if(x < 1)
    return 1;
  else
    return x;
}


u32 control_predict(struct model *md);
void control_update(struct model *md, u32 rtt_meas);


u32 control_process(struct model *md, u32 rtt_meas)
{
    u32 psi, xi, b0, rate_opt;

    control_update(md, rtt_meas);

    md->avg_rtt = LB_M(100 - md->gamma, rtt_meas) + LB_M(md->gamma, md->avg_rtt);

    md->avg_rtt_var =
        LB_M(100 - md->gamma, square_diff_u32(md->predicted_rtt, md->avg_rtt))
        + LB_M(md->gamma, md->avg_rtt_var);


    md->predicted_rtt = control_predict(md);


    psi = md->psi / atl1(md->avg_rtt_var);
    xi = md->xi / atl1(md->avg_pacing_rate);
    b0 = atl1(md->b[0]);

    // Tricky dealing with percetage.
    // The extra 2*'s are there to avoid 1/(psi*b0) potentially going to 0.5.
    rate_opt = LB_D(LB_M(xi - b0/atl1(md->avg_rtt), LB_D(LB_D(1, atl1(psi)), b0))
        + 2*md->avg_rtt - 2*md->predicted_rtt, 2*b0);

    lookback_add(&md->lb_pacing_rate, rate_opt);
    md->predicted_rtt += b0 * rate_opt;

    md->avg_pacing_rate = LB_M(100 - md->gamma, rate_opt)
        + LB_M(md->gamma, md->avg_pacing_rate);

    return rate_opt;
}


u32 control_predict(struct model *md)
{
    int predicted_rtt = 0;
    int i = 0;

    for(i = 0; i < md->p; i++)
        predicted_rtt += LB_M(md->a[i], lookback_index(&md->lb_rtt, i));

    for(i = 0; i < md->q; i++)
        predicted_rtt += LB_M(md->b[i], lookback_index(&md->lb_pacing_rate, i));

    return predicted_rtt;
}


void control_update(struct model *md, u32 rtt_meas)
{
    int i;
    u32 error;
    u32 rtt_norm = 0;
    u32 rate_norm = 0;
    u32 total_norm;
    bool add;

    if(md->predicted_rtt < rtt_meas) {
        error = rtt_meas - md->predicted_rtt;
        add = true;
    } else {
        error = md->predicted_rtt - rtt_meas;
        add = false;
    }


    for(i = 0; i < md->p; i++) {
        u32 rtt = lookback_index(&md->lb_rtt, i);
        rtt_norm += rtt*rtt;
    }

    for(i = 0; i < md->q; i++) {
        u32 rate = lookback_index(&md->lb_pacing_rate, i);
        rate_norm += rate*rate;
    }

    total_norm = rtt_norm + rate_norm;


    if(total_norm > 0) {
        for(i = 0; i < md->p; i++) {
            u32 rtt = lookback_index(&md->lb_rtt, i);
            u32 delta = LB_M(md->alpha, error * rtt / total_norm);

            if(add)
              md->a[i] += delta;
            else
              md->a[i] -= delta;
        }

        for(i = 0; i < md->q; i++) {
            u32 rate = lookback_index(&md->lb_pacing_rate, i);
            u32 delta = LB_M(md->alpha, error * rate / total_norm);

            if(add)
                md->b[i] += delta;
            else
                md->b[i] -= delta;
        }
    }


    lookback_add(&md->lb_rtt, rtt_meas);
}

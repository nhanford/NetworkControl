
#include "control.h"
#include "util.h"

// (x - y)^2
inline float square_diff(float x, float y)
{
    float diff = x - y;

    return diff * diff;
}


float control_predict(struct model *md);
void control_update(struct model *md, float rtt_meas);


float control_process(struct model *md, float rtt_meas, float rate_gain)
{
    float b0 = md->b[0];
    float rate_opt = 0;

    mpc_log("rtt_meas = %fs\n", 1000000 * rtt_meas);

    control_update(md, rtt_meas);

    md->avg_rtt = (1 - md->gamma) * rtt_meas + md->gamma * md->avg_rtt;

    md->avg_rtt_var =
      (1 - md->gamma) * square_diff(md->predicted_rtt, md->avg_rtt) +
      md->gamma * md->avg_rtt_var;

    md->predicted_rtt = control_predict(md);


    if(rate_gain > 0) {
        rate_opt = lookback_index(&md->lb_pacing_rate, 0) + rate_gain;
    } else if(md->avg_rtt > 0
            && md->avg_rtt_var > 0
            && md->avg_pacing_rate > 0
            && b0 > 0) {
        float cd = 2*b0*md->psi;
        float t1 = md->avg_rtt_var * md->xi / (cd * b0 * md->avg_pacing_rate);
        float t2 = md->avg_rtt / b0;
        float t3 = md->avg_rtt_var / (cd * md->avg_rtt);
        float t4 = md->predicted_rtt / b0;
        rate_opt = (t1 + t2) - (t3 + t4);
    }

    // Clamp rate
    // TODO: Make bounds less arbitrary.
    if(rate_opt > 100<<17)
        rate_opt = 100<<17;

    lookback_add(&md->lb_pacing_rate, rate_opt);
    md->predicted_rtt = md->predicted_rtt + b0 * rate_opt;

    md->avg_pacing_rate = (1 - md->gamma) * rate_opt +
        md->gamma * md->avg_pacing_rate;

    mpc_log("rate_opt = %f bytes/s\n", rate_opt);

    return rate_opt;
}


float control_predict(struct model *md)
{
    float predicted_rtt = 0;
    size_t i = 0;

    for(i = 0; i < md->p; i++)
        predicted_rtt += md->a[i] * lookback_index(&md->lb_rtt, i);

    for(i = 0; i < md->q; i++)
        predicted_rtt += md->b[i] * lookback_index(&md->lb_pacing_rate, i);

    return predicted_rtt;
}


void control_update(struct model *md, float rtt_meas)
{
    int i;
    float error = rtt_meas - md->predicted_rtt;
    float rtt_norm = 0;
    float rate_norm = 0;
    float total_norm;


    for(i = 0; i < md->p; i++) {
        float rtt = lookback_index(&md->lb_rtt, i);
        rtt_norm += rtt * rtt;
    }

    for(i = 0; i < md->q; i++) {
        float rate = lookback_index(&md->lb_pacing_rate, i);
        rate_norm += rate * rate;
    }

    total_norm = rtt_norm + rate_norm;


    if(total_norm > 0) {
        for(i = 0; i < md->p; i++) {
            float rtt = lookback_index(&md->lb_rtt, i);
            float delta = md->alpha * error * rtt / total_norm;

            md->a[i] += delta;
        }

        for(i = 0; i < md->q; i++) {
            float rate = lookback_index(&md->lb_pacing_rate, i);
            float delta = md->alpha * error * rate / total_norm;

            md->b[i] += delta;
        }
    }


    lookback_add(&md->lb_rtt, rtt_meas);
}

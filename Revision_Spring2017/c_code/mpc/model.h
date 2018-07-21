
#include "lookback.h"

#ifndef MODEL_H
#define MODEL_H

struct model {
    // These are all floats as defined above.
    float psi;
    float xi;
    float gamma;

    // s
    float avg_rtt;
    float avg_rtt_var;

    // bytes/s
    float avg_pacing_rate;

    // s
    float predicted_rtt;

    size_t p;
    size_t q;

    // float
    float alpha;
    float *a;
    float *b;

    // s
    struct lookback lb_rtt;

    // bytes/s
    struct lookback lb_pacing_rate;
};

void model_init(struct model *md, float psi, float xi, float gamma, float alpha,
        size_t p, size_t q);

void model_release(struct model *md);

#endif /* end of include guard: MODEL_H */

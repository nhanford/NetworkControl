
#include "lookback.h"

#ifndef MODEL_H
#define MODEL_H

struct model {
    // These are all floats as defined above.
    real psi;
    real xi;
    real gamma;

    // s
    real avg_rtt;
    real avg_rtt_var;

    // MB/s
    real avg_pacing_rate;

    // s
    real predicted_rtt;

    size_t p;
    size_t q;

    // float
    real alpha;
    real *a;
    real *b;

    // s
    struct lookback lb_rtt;

    // MB/s
    struct lookback lb_pacing_rate;
};

// psi, xi, gamma, and alpha are percentages.
void model_init(struct model *md, real_int psi, real_int xi, real_int gamma,
        real_int alpha, size_t p, size_t q);

void model_release(struct model *md);

#endif /* end of include guard: MODEL_H */

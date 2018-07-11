
#include "lookback.h"

#ifndef MODEL_H
#define MODEL_H

// No floats in the kernel, so they must be represented as integers.
// To convert a fraction x to an integer we use x' = floor(x * 2^prec).
#define LB_PRECISION 10

// Muliply and divide floats. For LB_M, at least one of x or y must be a float.
// For LB_D, y must be a float.
#define LB_M(x, y) (((x) * (y)) >> LB_PRECISION)
#define LB_D(x, y) (((x) << LB_PRECISION) / (y))
#define LB_FRAC_TO_INT(num, den) (((num) << LB_PRECISION) / den)
#define LB_PERC_TO_INT(perc) LB_FRAC_TO_INT(perc, 100)


struct model {
    // These are all floats as defined above.
    u32 psi;
    u32 xi;
    u32 gamma;

    // us
    u32 avg_rtt;
    u32 avg_rtt_var;

    // bytes/s
    u32 avg_pacing_rate;

    // us
    u32 predicted_rtt;

    size_t p;
    size_t q;

    // float
    u32 alpha;
    u32 *a;
    u32 *b;

    // us
    struct lookback lb_rtt;

    // bytes/s
    struct lookback lb_pacing_rate;
};

void model_init(struct model *md, u32 psi, u32 xi, u32 gamma, u32 alpha,
        size_t p, size_t q);

void model_release(struct model *md);

#endif /* end of include guard: MODEL_H */

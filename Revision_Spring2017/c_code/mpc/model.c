
#include <linux/slab.h>

#include "model.h"

void model_init(struct model *md, real_int psi, real_int xi, real_int gamma,
        real_int alpha, size_t p, size_t q)
{
    int i;

    md->psi = real_from_frac(psi, 100);
    md->xi = real_from_frac(xi, 100);
    md->gamma = real_from_frac(gamma, 100);

    md->avg_rtt = REAL_ZERO;
    md->avg_rtt_var = REAL_ZERO;
    md->avg_pacing_rate = REAL_ZERO;

    md->predicted_rtt = REAL_ZERO;

    md->p = p;
    md->q = q;

    md->alpha = real_from_frac(alpha, 100);
    md->a = kmalloc(p*sizeof(real), GFP_KERNEL);
    md->b = kmalloc(q*sizeof(real), GFP_KERNEL);

    for(i = 0; i < p; i++)
        md->a[i] = REAL_ZERO;
    for(i = 0; i < q; i++)
        md->b[i] = REAL_ZERO;

    lookback_init(&md->lb_rtt, p, REAL_ZERO);
    lookback_init(&md->lb_pacing_rate, q, REAL_ZERO);

    mpc_dfs_init(&md->dstats);
}

void model_release(struct model *md)
{
    kfree(md->a);
    kfree(md->b);

    lookback_release(&md->lb_rtt);
    lookback_release(&md->lb_pacing_rate);

    mpc_dfs_release(&md->dstats);
}

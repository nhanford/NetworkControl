
#include <linux/slab.h>

#include "model.h"

void model_init(struct model *md, float psi, float xi, float gamma, float alpha,
        size_t p, size_t q)
{
    int i;

    md->psi = psi;
    md->xi = xi;
    md->gamma = gamma;

    md->avg_rtt = 0;
    md->avg_rtt_var = 0;
    md->avg_pacing_rate = 0;

    md->predicted_rtt = 0;

    md->p = p;
    md->q = q;

    md->alpha = alpha;
    md->a = kmalloc(p*sizeof(float), GFP_KERNEL);
    md->b = kmalloc(q*sizeof(float), GFP_KERNEL);

    for(i = 0; i < p; i++)
        md->a[i] = 0;
    for(i = 0; i < q; i++)
        md->b[i] = 0;

    lookback_init(&md->lb_rtt, p, 0);
    lookback_init(&md->lb_pacing_rate, q, 0);
}

void model_release(struct model *md)
{
    kfree(md->a);
    kfree(md->b);

    lookback_release(&md->lb_rtt);
    lookback_release(&md->lb_pacing_rate);
}

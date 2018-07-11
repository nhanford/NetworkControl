
#include <linux/slab.h>

#include "model.h"

// psi, xi, gamma, and alpha are integer percentages (i.e. 32, 50, etc.).
void model_init(struct model *md, u32 psi, u32 xi, u32 gamma, u32 alpha,
        size_t p, size_t q)
{
    md->psi = LB_PERC_TO_INT(psi);
    md->xi = LB_PERC_TO_INT(xi);
    md->gamma = LB_PERC_TO_INT(gamma);

    md->avg_rtt = 0;
    md->avg_rtt_var = 0;
    md->avg_pacing_rate = 0;

    md->predicted_rtt = 0;

    md->p = p;
    md->q = q;

    md->alpha = LB_PERC_TO_INT(alpha);
    md->a = kmalloc(p*sizeof(u32), GFP_KERNEL);
    md->b = kmalloc(q*sizeof(u32), GFP_KERNEL);

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

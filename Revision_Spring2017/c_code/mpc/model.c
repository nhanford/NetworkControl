
#include <linux/slab.h>

#include "model.h"


int model_init(struct model *md, scaled rate_diff, u64 inc_period,
	u64 dec_period, scaled weight, scaled alpha, scaled c1, scaled c2)
{
	md->rate_diff = rate_diff;
	md->inc_period = inc_period;
	md->dec_period = dec_period;
	md->weight = weight;

	md->alpha_init = alpha;
	md->c1 = c1;
	md->c2 = c2;

	mpc_dfs_init(&md->stats);

	model_reset(md);

	return 0;
}

void model_release(struct model *md)
{
	mpc_dfs_release(&md->stats);
}

void model_reset(struct model *md)
{
	md->timer = md->inc_period;
	md->rate_set = ZERO;

	md->alpha = md->alpha_init;

	md->avg_rate = ZERO;
	md->avg_rtt = ZERO;

	md->x0 = ZERO;
	md->x1 = ZERO;

	md->rb = ZERO;
	md->lb = ZERO;
	md->lp = scaled_from_int(S64_MAX, 0);
}

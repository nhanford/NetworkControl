
#include <linux/slab.h>

#include "model.h"


int model_init(struct model *md, s64 rate_diff, s64 period, s64 weight,
	s64 alpha, s64 c1, s64 c2, s64 c3)
{
	md->rate_diff = rate_diff;
	md->period = period;
	md->weight = weight*MPC_ONE/100;

	md->alpha = alpha*MPC_ONE/100;
	md->c1 = c1;
	md->c2 = c2;
	md->c3 = c3;

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
	md->timer = md->period;
	md->rate_set = 0;

	md->avg_rate = 0;
	md->avg_rtt = 0;

	md->x0 = 0;
	md->x1 = 0;

	md->rb = 0;
	md->lb = 0;
	md->lp = S64_MAX;
}

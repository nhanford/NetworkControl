
#include <linux/slab.h>

#include "model.h"


int model_init(struct model *md, scaled rate_diff, u64 inc_period,
	u64 dec_period, scaled learn_rate, scaled over, scaled c)
{
	md->rate_diff = rate_diff;
	md->inc_period = inc_period;
	md->dec_period = dec_period;
	md->learn_rate = learn_rate;

	md->over = over;
	md->c = c;

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
	md->decreasing = false;
	md->rate_set = ZERO;

	md->avg_rate = ZERO;
	md->avg_rtt = ZERO;

	md->x0 = ZERO;
	md->x1 = ZERO;

	md->rb = ZERO;
	md->lb = ZERO;
	md->lp = scaled_from_int(S64_MAX, 0);
}

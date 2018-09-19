
#include <linux/slab.h>

#include "model.h"


int model_init(struct model *md, s64 rate_diff, s64 perc_rtt, s64 perc_max, s64
	weight, s64 num_obs)
{
	md->rate_diff = rate_diff;
	md->perc_rtt = perc_rtt*MPC_ONE/100;
	md->perc_max = perc_max*MPC_ONE/100;
	md->weight = weight*MPC_ONE/100;
	md->num_obs = num_obs;

	if (lookback_init(&md->rate, md->num_obs))
		goto fail_a;
	if (lookback_init(&md->rtt, md->num_obs))
		goto fail_b;
	if (lookback_init(&md->x, md->num_obs))
		goto fail_c;

	mpc_dfs_init(&md->stats);

	model_reset(md);

	return 0;
fail_c:
	lookback_release(&md->rtt);
fail_b:
	lookback_release(&md->rate);
fail_a:
	return 1;
}

void model_release(struct model *md)
{
	lookback_release(&md->rate);
	lookback_release(&md->rtt);
	lookback_release(&md->x);

	mpc_dfs_release(&md->stats);
}

void model_reset(struct model *md)
{
	size_t i;

	md->probe_time = 0;
	md->start_probe = 0;

	for (i = 0; i < md->num_obs; i++) {
		*lookback_index_ref(&md->rate, i) = 0;
		*lookback_index_ref(&md->rtt, i) = 0;
		*lookback_index_ref(&md->x, i) = 0;
	}

	md->avg_rb = 0;
	md->var_rb = 0;
	md->avg_x = 0;

	md->a = 0;
	md->rb = 0;
	md->lb = 0;
	md->lp = S64_MAX;
}

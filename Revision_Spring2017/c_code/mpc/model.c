
#include <linux/slab.h>

#include "model.h"


int model_init(struct model *md, s64 max_diff_perc, s64 weight)
{
	md->max_diff_perc = max_t(s64, 1, max_diff_perc*MPC_ONE/100);
	md->k1 = MPC_ONE/md->max_diff_perc;
	md->k2 = MPC_ONE;
	md->weight = weight*MPC_ONE/100;

	md->rtt_last = 0;
	md->rate_last = 0;
	md->rate_last2 = 0;
	md->a = 0;

	md->avg_rtt = 0;
	md->avg_rate = 0;
	md->pred_rtt = 0;

	mpc_dfs_init(&md->dstats);

	return 0;
}

void model_release(struct model *md)
{
	mpc_dfs_release(&md->dstats);
}

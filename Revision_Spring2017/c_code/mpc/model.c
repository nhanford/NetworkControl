
#include <linux/slab.h>

#include "model.h"


int model_init(struct model *md, s64 c1, s64 c2, s64 weight)
{
	md->c1 = c1*MPC_ONE/100;
	md->c2 = c2*MPC_ONE/100;
	md->weight = weight*MPC_ONE/100;

	md->rtt_last = 0;
	md->rate_last = 0;
	md->a = 0;

	md->avg_rtt = 0;
	md->avg_rate = 0;
	md->avg_rate_delta = 0;
	md->pred_rtt = 0;

	mpc_dfs_init(&md->dstats);

	return 0;
}

void model_release(struct model *md)
{
	mpc_dfs_release(&md->dstats);
}

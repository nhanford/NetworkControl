
#include <linux/slab.h>

#include "model.h"


int model_init(struct model *md, s64 time, size_t num_obs)
{
        md->m = 0;
        md->b = 0;

        md->start_time = time;
        md->last_time = time;

        md->integral = 0;

        md->num_obs = num_obs;
	if (lookback_init(&md->lb_rtt, num_obs, 0))
		goto exit_failure_a;

	if (lookback_init(&md->lb_rate, num_obs, 0))
		goto exit_failure_b;

	mpc_dfs_init(&md->dstats);

	return 0;

exit_failure_b:
	lookback_release(&md->lb_rtt);
exit_failure_a:
	return 1;
}

void model_release(struct model *md)
{
	lookback_release(&md->lb_rtt);
	lookback_release(&md->lb_rate);

	mpc_dfs_release(&md->dstats);
}


#include "dfs.h"
#include "lookback.h"
#include "util.h"

#ifndef MODEL_H
#define MODEL_H

struct model {
	// Out of MPC_ONE.
	s64 m;
	s64 b;

	// us
	s64 start_time;
	s64 last_time;

	// B (MB/s * us)
	s64 integral;

	size_t num_obs;

	// us
	struct lookback lb_rtt;

	// MB/s
	struct lookback lb_rate;

	s64 sum_rate;
	s64 sum_rate_sqr;
	s64 sum_rtt;
	s64 sum_rtt_rate;

	// For debugging.
	struct mpc_dfs_stats dstats;
};

// time is in us.
int model_init(struct model *md, s64 time, size_t num_obs);

void model_release(struct model *md);

#endif /* end of include guard: MODEL_H */

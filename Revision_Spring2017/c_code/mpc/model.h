
#include "dfs.h"
#include "lookback.h"
#include "util.h"

#ifndef MODEL_H
#define MODEL_H

struct model {
	// RTT is in microseconds, rate is in Mbytes/s, and everything else is a
	// fraction of MPC_ONE.
	s64 c1;
	s64 c2;
	s64 weight;

	s64 rtt_last;
	s64 rate_last;
	s64 a;

	s64 avg_rtt;
	s64 avg_rate;
	s64 avg_rate_delta;
	s64 pred_rtt;

	// For debugging.
	struct mpc_dfs_stats dstats;
};

// c1, c2, and weight are percentages (i.e. 50 = 50%).
// c1 is the weight to apply to minimizing the change in rate.
// c2 is the weight to apply to maximizing the rate.
// weight is the weight applied when averaging.
int model_init(struct model *md, s64 c1, s64 c2, s64 weight);

void model_release(struct model *md);

#endif /* end of include guard: MODEL_H */

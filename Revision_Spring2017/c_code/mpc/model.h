
#include "dfs.h"
#include "lookback.h"
#include "util.h"

#ifndef MODEL_H
#define MODEL_H

struct model {
	// Rates are in MB/s, and RTTs are in us. Percentages are out of
	// MPC_ONE.

	s64 rate_diff;
	s64 weight;
	s64 period;
	s64 rate_set;

	s64 alpha;
	s64 c1;
	s64 c2;
	s64 c3;

	s64 timer;

	s64 avg_rate;
	s64 avg_rtt;

	s64 x0;
	s64 x1;

	s64 rb;
	s64 lb;
	s64 lp;

	struct mpc_dfs_stats stats;
};

// alpha and weight are percentages.
int model_init(struct model *md, s64 rate_diff, s64 period, s64 weight,
	s64 alpha, s64 c1, s64 c2, s64 c3);

void model_release(struct model *md);

void model_reset(struct model *md);

#endif /* end of include guard: MODEL_H */

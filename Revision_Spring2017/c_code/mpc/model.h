
#include "dfs.h"
#include "lookback.h"
#include "util.h"

#ifndef MODEL_H
#define MODEL_H

struct model {
	// Rates are in MB/s, and RTTs are in us. a and percentages are out of
	// MPC_ONE.

	s64 rate_diff;
	s64 perc_rtt;
	s64 perc_max;
	s64 weight;
	s64 period;
	s64 num_obs;

	s64 probe_time;
	u64 start_probe;

	struct lookback rate;
	struct lookback rtt;
	struct lookback x;

	s64 avg_rb;
	s64 var_rb;
	s64 avg_x;

	s64 a;
	s64 rb;
	s64 lb;
	s64 lp;

	struct mpc_dfs_stats stats;
};

// perc_rtt, perc_max, and weight are percentages.
int model_init(struct model *md, s64 rate_diff, s64 perc_rtt, s64 perc_max, s64
	weight, s64 num_obs);

void model_release(struct model *md);

void model_reset(struct model *md);

#endif /* end of include guard: MODEL_H */

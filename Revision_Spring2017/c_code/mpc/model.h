
#include "dfs.h"
#include "scaled.h"
#include "util.h"

#ifndef MODEL_H
#define MODEL_H

struct model {
	// Rates are in B/s, and RTTs are in us.
	scaled rate_diff;
	scaled learn_rate;
	u64 inc_period;
	u64 dec_period;
	scaled rate_set;

	// How far over the minimum RTT should we be.
	scaled over;
	// RTT variance relative weight
	scaled c;

	u64 timer;
	bool decreasing;

	scaled avg_rate;
	scaled avg_rtt;

	scaled x0;
	scaled x1;

	scaled rb;
	scaled lb;
	scaled lp;

	struct mpc_dfs_stats stats;
};

int model_init(struct model *md, scaled rate_diff, u64 inc_period,
	u64 dec_period, scaled learn_rate, scaled over, scaled c);

void model_release(struct model *md);

void model_reset(struct model *md);

#endif /* end of include guard: MODEL_H */

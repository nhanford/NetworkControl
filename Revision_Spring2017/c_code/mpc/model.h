
#include "dfs.h"
#include "scaled.h"
#include "util.h"

#ifndef MODEL_H
#define MODEL_H

struct model {
	// Rates are in B/s, and RTTs are in us. Percentages are out of
	// MPC_ONE.

	scaled rate_diff;
	scaled weight;
	u64 period;
	scaled rate_set;

	scaled alpha;
	// RTT weight
	scaled c1;
	// Rate change weight
	scaled c2;
	// Rate weight
	scaled c3;

	u64 timer;

	scaled avg_rate;
	scaled avg_rtt;

	scaled x0;
	scaled x1;

	scaled rb;
	scaled lb;
	scaled lp;

	struct mpc_dfs_stats stats;
};

// alpha and weight are percentages.
int model_init(struct model *md, scaled rate_diff, u64 period, scaled weight,
	scaled alpha, scaled c1, scaled c2, scaled c3);

void model_release(struct model *md);

void model_reset(struct model *md);

#endif /* end of include guard: MODEL_H */

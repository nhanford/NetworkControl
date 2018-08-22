
#include "dfs.h"
#include "lookback.h"
#include "util.h"

#ifndef MODEL_H
#define MODEL_H

struct model {
	// These are all out of MPC_DIV. So, 0.5 = MPC_DIV/2
	s32 psi;
	s32 xi;
	s32 gamma;

	// us
	s64 avg_rtt;
	s64 avg_rtt_var;

	// MB/s
	s64 avg_pacing_rate;

	// us
	s64 predicted_rtt;

	size_t p;
	size_t q;

	// These are all out of MPC_DIV. So, 0.5 = MPC_DIV/2
	s32 alpha;
	s64 *a;
	s64 *b;

	// us
	struct lookback lb_rtt;

	// MB/s
	struct lookback lb_pacing_rate;

	// For debugging.
	struct mpc_dfs_stats dstats;
};

// psi, xi, gamma, and alpha are percentages.
int model_init(struct model *md, s32 psi, s32 xi, s32 gamma, s32 alpha,
		size_t p, size_t q);

void model_release(struct model *md);

#endif /* end of include guard: MODEL_H */

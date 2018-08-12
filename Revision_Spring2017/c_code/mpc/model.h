
#include <linux/debugfs.h>

#include "lookback.h"
#include "util.h"

#ifndef MODEL_H
#define MODEL_H

#define MPC_DFS_DIR "mpc"


struct mpc_dfs_stats {
	struct dentry *dir;

	s64 rtt_meas_us;
	s64 rtt_pred_us;
	s64 rate_set;
	bool probing;
};

struct model {
	// These are all out of MPC_DIV. So, 0.5 = MPC_DIV/2
	s32 gamma;

	// us
	s64 avg_rtt;
	s64 avg_rtt_var;

	// MB/s
	s64 avg_pacing_rate;

	// us
	s64 target_rtt;
	s64 predicted_rtt;

	size_t p;
	size_t q;

	// These are all out of MPC_DIV. So, 0.5 = MPC_DIV/2
	s32 alpha;
	s64 *a;
	s64 *b;

	// us
	s64 last_rtt;
	struct lookback lb_rtt_diff;

	// MB/s
	s64 last_rate;
	struct lookback lb_rate_diff;

	// For debugging.
	struct mpc_dfs_stats dstats;
};

// psi, xi, gamma, and alpha are percentages.
void model_init(struct model *md, s32 gamma, s32 alpha, size_t p, size_t q);

void model_release(struct model *md);

#endif /* end of include guard: MODEL_H */

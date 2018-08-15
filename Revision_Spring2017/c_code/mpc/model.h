
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
	// RTT is in microseconds, rate is in Mbytes/s, and everything else is a
	// fraction of MPC_ONE.

	s64 max_diff_perc;
	s64 k1;
	s64 k2;
	s64 weight;

	s64 rtt_last;
	s64 rate_last;
	s64 rate_last2;
	s64 a;

	s64 avg_rtt;
	s64 avg_rate;
	s64 pred_rtt;

	// For debugging.
	struct mpc_dfs_stats dstats;
};

// max_diff_perc and weight are percentages (i.e. 50 = 50%).
// max_diff_perc is maximum percentage change in rate.
// weight is the weight applied when averaging RTT and rate.
void model_init(struct model *md, s64 max_diff_perc, s64 weight);

void model_release(struct model *md);

#endif /* end of include guard: MODEL_H */

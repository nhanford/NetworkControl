
#include <linux/debugfs.h>

#ifndef DFS_H
#define DFS_H

#define MPC_DFS_DIR "mpc"

struct mpc_dfs_stats {
	struct dentry *dir;

	s64 rtt_meas_us;
	s64 rtt_pred_us;
	s64 rate_set;
	bool probing;
};


void mpc_dfs_init(struct mpc_dfs_stats *dstats);
void mpc_dfs_release(struct mpc_dfs_stats *dstats);

#endif /* end of include guard: DFS_H */

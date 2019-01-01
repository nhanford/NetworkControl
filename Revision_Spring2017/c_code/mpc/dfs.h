
#include <linux/debugfs.h>

#ifndef DFS_H
#define DFS_H

#define MPC_DFS_DIR "mpc"

struct mpc_dfs {
	struct dentry *root;
	unsigned long long alive;
	unsigned long long next_id;
};

struct mpc_dfs_stats {
	struct dentry *dir;

	s64 rtt_meas_us;
	s64 rtt_pred_us;
	s64 rate_meas;
	s64 rate_set;

	s64 lp;
	s64 rb;
	s64 x;
};


void mpc_dfs_init(struct mpc_dfs *dfs);
void mpc_dfs_release(struct mpc_dfs *dfs);

void mpc_dfs_register(struct mpc_dfs *dfs, struct mpc_dfs_stats *dstats);
void mpc_dfs_unregister(struct mpc_dfs *dfs, struct mpc_dfs_stats *dstats);

#endif /* end of include guard: DFS_H */

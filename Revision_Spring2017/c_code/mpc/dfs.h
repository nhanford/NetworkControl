
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

	s64 loss_meas;
	s64 loss_pred;
	s64 rate_meas;
	s64 rate_set;

	s64 rb;
};


void mpc_dfs_init(struct mpc_dfs *dfs);
void mpc_dfs_release(struct mpc_dfs *dfs);

void mpc_dfs_register(struct mpc_dfs *dfs, struct mpc_dfs_stats *dstats);
void mpc_dfs_unregister(struct mpc_dfs *dfs, struct mpc_dfs_stats *dstats);

#endif /* end of include guard: DFS_H */

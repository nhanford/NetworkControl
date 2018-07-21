
#include <linux/debugfs.h>
#include <linux/printk.h>

#ifndef MPC_UTIL_H
#define MPC_UTIL_H

#define mpc_log(args, ...) printk(KERN_INFO "mpc: " args, ##__VA_ARGS__)

#ifndef USEC_PER_SEC
#define USEC_PER_SEC 1000000
#endif

#define MB_PER_B (1 << 20)

#define MPC_DFS_DIR "mpc"

struct mpc_dfs_stats {
    struct dentry *root;
    u32 rtt_meas_us;
    u32 rate_set;
};

void mpc_dfs_init(struct mpc_dfs_stats *dstats);
void mpc_dfs_release(struct mpc_dfs_stats *dstats);

#endif /* end of include guard: MPC_UTIL_H */

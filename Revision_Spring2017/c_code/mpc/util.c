
#include "util.h"

void mpc_dfs_init(struct mpc_dfs_stats *dstats)
{
    // We need to create a unique name for each DFS since multiple instances may
    // be running.
    char uniq_name[32];

    sprintf(uniq_name, "%p", dstats);

    dstats->root = debugfs_lookup(MPC_DFS_DIR, NULL);

    if(dstats->root == NULL)
        dstats->root = debugfs_create_dir(MPC_DFS_DIR, NULL);

    dstats->root = debugfs_create_dir(uniq_name, dstats->root);

    if(dstats->root == NULL) {
        mpc_log("Failed to create dfs\n");
    } else {
        debugfs_create_u64("rtt_meas_us", 0644, dstats->root, &dstats->rtt_meas_us);
        debugfs_create_u64("rate_set", 0644, dstats->root, &dstats->rate_set);
        mpc_log("Created dfs\n");
    }
}

void mpc_dfs_release(struct mpc_dfs_stats *dstats)
{
    debugfs_remove_recursive(dstats->root);
}

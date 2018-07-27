
#include <linux/limits.h>
#include <linux/slab.h>

#include "model.h"


static unsigned long long dfs_id = 0;


void mpc_dfs_init(struct mpc_dfs_stats *dstats)
{
    // We need to create a unique name for each DFS since multiple instances may
    // be running.
    char uniq_name[32];

    dstats->rtt_meas_us = 0;
    dstats->rate_set = 0;

    sprintf(uniq_name, "%lld", dfs_id);

    dstats->root = debugfs_lookup(MPC_DFS_DIR, NULL);

    if(dstats->root == NULL)
        dstats->root = debugfs_create_dir(MPC_DFS_DIR, NULL);

    if(dstats->root != NULL)
        dstats->root = debugfs_create_dir(uniq_name, dstats->root);

    if(dstats->root == NULL) {
        mpc_log("Failed to create debugfs directory.\n");
    } else {
        debugfs_create_u64("rtt_meas_us", 0644, dstats->root, &dstats->rtt_meas_us);
        debugfs_create_u64("rate_set", 0644, dstats->root, &dstats->rate_set);
        dfs_id = (dfs_id + 1) % ULLONG_MAX;
    }
}

void mpc_dfs_release(struct mpc_dfs_stats *dstats)
{
    debugfs_remove_recursive(dstats->root);
}


void model_init(struct model *md, u8 psi, u8 xi, u8 gamma, u8 alpha,
        size_t p, size_t q)
{
    int i;

    md->psi = psi*MPC_ONE/100;
    md->xi = xi*MPC_ONE/100;
    md->gamma = gamma*MPC_ONE/100;

    md->avg_rtt = 0;
    md->avg_rtt_var = 0;
    md->avg_pacing_rate = 0;

    md->predicted_rtt = 0;

    md->p = p;
    md->q = q;

    md->alpha = alpha*MPC_ONE/100;
    md->a = kmalloc(p*sizeof(u8), GFP_KERNEL);
    md->b = kmalloc(q*sizeof(u8), GFP_KERNEL);

    for(i = 0; i < p; i++)
        md->a[i] = 0;
    for(i = 0; i < q; i++)
        md->b[i] = 0;

    lookback_init(&md->lb_rtt, p, 0);
    lookback_init(&md->lb_pacing_rate, q, 0);

    mpc_dfs_init(&md->dstats);
}

void model_release(struct model *md)
{
    kfree(md->a);
    kfree(md->b);

    lookback_release(&md->lb_rtt);
    lookback_release(&md->lb_pacing_rate);

    mpc_dfs_release(&md->dstats);
}

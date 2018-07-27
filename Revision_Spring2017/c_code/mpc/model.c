
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


void model_init(struct model *md, real_int psi, real_int xi, real_int gamma,
        real_int alpha, size_t p, size_t q)
{
    int i;

    md->psi = real_from_frac(psi, 100);
    md->xi = real_from_frac(xi, 100);
    md->gamma = real_from_frac(gamma, 100);

    md->avg_rtt = REAL_ZERO;
    md->avg_rtt_var = REAL_ZERO;
    md->avg_pacing_rate = REAL_ZERO;

    md->predicted_rtt = REAL_ZERO;

    md->p = p;
    md->q = q;

    md->alpha = real_from_frac(alpha, 100);
    md->a = kmalloc(p*sizeof(real), GFP_KERNEL);
    md->b = kmalloc(q*sizeof(real), GFP_KERNEL);

    for(i = 0; i < p; i++)
        md->a[i] = REAL_ZERO;
    for(i = 0; i < q; i++)
        md->b[i] = REAL_ZERO;

    lookback_init(&md->lb_rtt, p, REAL_ZERO);
    lookback_init(&md->lb_pacing_rate, q, REAL_ZERO);

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


#include <linux/debugfs.h>

#include "lookback.h"
#include "util.h"

#ifndef MODEL_H
#define MODEL_H

#define MPC_DFS_DIR "mpc"


struct mpc_dfs_stats {
    struct dentry *root;

    int id;
    u64 rtt_meas_us;
    u64 rate_set;
};

struct model {
    // These are all out of MPC_DIV. So, 0.5 = MPC_DIV/2
    u8 psi;
    u8 xi;
    u8 gamma;

    // us
    u64 avg_rtt;
    u64 avg_rtt_var;

    // B/s
    u64 avg_pacing_rate;

    // us
    u64 predicted_rtt;

    size_t p;
    size_t q;

    // These are all out of MPC_DIV. So, 0.5 = MPC_DIV/2
    u8 alpha;
    u8 *a;
    u8 *b;

    // us
    struct lookback lb_rtt;

    // B/s
    struct lookback lb_pacing_rate;

    // For debugging.
    struct mpc_dfs_stats dstats;
};

// psi, xi, gamma, and alpha are percentages.
void model_init(struct model *md, u8 psi, u8 xi, u8 gamma, u8 alpha,
        size_t p, size_t q);

void model_release(struct model *md);

#endif /* end of include guard: MODEL_H */

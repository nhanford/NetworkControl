
#include <linux/limits.h>
#include <linux/slab.h>
#include <linux/spinlock.h>

#include "model.h"


static struct dentry *root = NULL;
static DEFINE_SPINLOCK(dfs_lock);
static unsigned long long dfs_id = 0;


void mpc_dfs_init(struct mpc_dfs_stats *dstats)
{
	// We need to create a unique name for each DFS since multiple instances may
	// be running.
	char uniq_name[32];
	unsigned long flags;

	dstats->dir = NULL;
	dstats->rtt_meas_us = 0;
	dstats->rtt_pred_us = 0;
	dstats->rate_set = 0;
	dstats->probing = false;

	sprintf(uniq_name, "%lld", dfs_id);


	spin_lock_irqsave(&dfs_lock, flags);

	if (root == NULL)
		root = debugfs_create_dir(MPC_DFS_DIR, NULL);

	if (root != NULL)
		dstats->dir = debugfs_create_dir(uniq_name, root);

	spin_unlock_irqrestore(&dfs_lock, flags);


	if (dstats->dir == NULL) {
		mpc_log("Failed to create debugfs directory.\n");
	} else {
		debugfs_create_u64("rtt_meas_us", 0444, dstats->dir,
				&dstats->rtt_meas_us);
		debugfs_create_u64("rtt_pred_us", 0444, dstats->dir,
				&dstats->rtt_pred_us);
		debugfs_create_u64("rate_set", 0444, dstats->dir,
				&dstats->rate_set);
		debugfs_create_bool("probing", 0444, dstats->dir,
				&dstats->probing);
		dfs_id = (dfs_id + 1) % ULLONG_MAX;
	}
}

void mpc_dfs_release(struct mpc_dfs_stats *dstats)
{
	unsigned long flags;

	spin_lock_irqsave(&dfs_lock, flags);
	if (root != NULL)
		debugfs_remove_recursive(root);

	root = NULL;
	spin_unlock_irqrestore(&dfs_lock, flags);
}


void model_init(struct model *md, u8 psi, u8 xi, u8 gamma, u8 alpha,
		size_t p, size_t q)
{
	size_t i;

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
	md->a = kmalloc(p*sizeof(s64), GFP_KERNEL);
	md->b = kmalloc(q*sizeof(s64), GFP_KERNEL);

	for (i = 0; i < p; i++)
		md->a[i] = 0;
	for (i = 0; i < q; i++)
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

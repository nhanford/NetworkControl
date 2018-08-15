
#include <linux/limits.h>
#include <linux/slab.h>
#include <linux/spinlock.h>

#include "model.h"


static struct dentry *root = NULL;
static DEFINE_SPINLOCK(dfs_lock);
static unsigned long long dfs_id = 0;


static int debugfs_s64_set(void *data, u64 val)
{
	*(s64 *)data = val;
	return 0;
}
static int debugfs_s64_get(void *data, u64 *val)
{
	*val = *(s64 *)data;
	return 0;
}
DEFINE_DEBUGFS_ATTRIBUTE(fops_s64, debugfs_s64_get, debugfs_s64_set, "%lld\n");


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
		debugfs_create_file("rtt_meas_us", 0444, dstats->dir,
				&dstats->rtt_meas_us, &fops_s64);
		debugfs_create_file("rtt_pred_us", 0444, dstats->dir,
				&dstats->rtt_pred_us, &fops_s64);
		debugfs_create_file("rate_set", 0444, dstats->dir,
				&dstats->rate_set, &fops_s64);
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


void model_init(struct model *md, s64 max_diff_perc, s64 weight)
{
	md->max_diff_perc = max_t(s64, 1, max_diff_perc*MPC_ONE/100);
	md->k1 = MPC_ONE/md->max_diff_perc;
	md->k2 = MPC_ONE;
	md->weight = weight*MPC_ONE/100;

	md->rtt_last = 0;
	md->rate_last = 0;
	md->rate_last2 = 0;
	md->a = 0;

	md->avg_rtt = 0;
	md->avg_rate = 0;
	md->pred_rtt = 0;

	mpc_dfs_init(&md->dstats);
}

void model_release(struct model *md)
{
	mpc_dfs_release(&md->dstats);
}

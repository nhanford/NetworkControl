
#include <linux/limits.h>
#include <linux/spinlock.h>

#include "dfs.h"


static struct dentry *root = NULL;
static unsigned long long alive = 0;
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
	dstats->rate_meas = 0;
	dstats->rate_set = 0;
	dstats->lp = 0;
	dstats->rb = 0;
	dstats->x = 0;

	sprintf(uniq_name, "%lld", dfs_id);


	spin_lock_irqsave(&dfs_lock, flags);

	if (root == NULL)
		root = debugfs_create_dir(MPC_DFS_DIR, NULL);

	if (root != NULL)
		dstats->dir = debugfs_create_dir(uniq_name, root);

	if (dstats->dir == NULL) {
		printk(KERN_WARNING "MPC: Failed to create debugfs directory.\n");
	} else {
		alive++;

		debugfs_create_file("rtt_meas_us", 0444, dstats->dir,
				&dstats->rtt_meas_us, &fops_s64);
		debugfs_create_file("rtt_pred_us", 0444, dstats->dir,
				&dstats->rtt_pred_us, &fops_s64);
		debugfs_create_file("rate_meas", 0444, dstats->dir,
				&dstats->rate_meas, &fops_s64);
		debugfs_create_file("rate_set", 0444, dstats->dir,
				&dstats->rate_set, &fops_s64);

		debugfs_create_file("lp", 0444, dstats->dir,
				&dstats->lp, &fops_s64);
		debugfs_create_file("rb", 0444, dstats->dir,
				&dstats->rb, &fops_s64);
		debugfs_create_file("x", 0444, dstats->dir,
				&dstats->x, &fops_s64);

		dfs_id = (dfs_id + 1) % ULLONG_MAX;
	}

	spin_unlock_irqrestore(&dfs_lock, flags);
}

void mpc_dfs_release(struct mpc_dfs_stats *dstats)
{
	unsigned long flags;

	debugfs_remove_recursive(dstats->dir);

	spin_lock_irqsave(&dfs_lock, flags);

	alive--;

	if (alive == 0 && root != NULL) {
		debugfs_remove_recursive(root);
		root = NULL;
	}

	spin_unlock_irqrestore(&dfs_lock, flags);
}

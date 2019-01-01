
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


void mpc_dfs_init(struct mpc_dfs *dfs)
{
	dfs->root = debugfs_create_dir(MPC_DFS_DIR, NULL);
	dfs->alive = 0;
	dfs->next_id = 0;

	if (dfs->root == NULL)
		printk(KERN_WARNING "MPC: Failed to create root debugfs directory.\n");
}

void mpc_dfs_release(struct mpc_dfs *dfs)
{
	debugfs_remove_recursive(dfs->root);
}

void mpc_dfs_register(struct mpc_dfs *dfs, struct mpc_dfs_stats *dstats)
{
	// We need to create a unique name for each DFS since multiple instances may
	// be running.
	char uniq_name[32];
	unsigned long flags;

	dstats->dir = NULL;
	dstats->loss_meas = 0;
	dstats->loss_pred = 0;
	dstats->rate_meas = 0;
	dstats->rate_set = 0;
	dstats->rb = 0;

	sprintf(uniq_name, "%lld", dfs->next_id);


	spin_lock_irqsave(&dfs_lock, flags);

	if (dfs->root != NULL)
		dstats->dir = debugfs_create_dir(uniq_name, dfs->root);

	if (dstats->dir == NULL) {
		printk(KERN_WARNING "MPC: Failed to create debugfs directory.\n");
	} else {
		dfs->alive++;

		debugfs_create_file("loss_meas", 0444, dstats->dir,
				&dstats->loss_meas, &fops_s64);
		debugfs_create_file("loss_pred", 0444, dstats->dir,
				&dstats->loss_pred, &fops_s64);
		debugfs_create_file("rate_meas", 0444, dstats->dir,
				&dstats->rate_meas, &fops_s64);
		debugfs_create_file("rate_set", 0444, dstats->dir,
				&dstats->rate_set, &fops_s64);

		debugfs_create_file("rb", 0444, dstats->dir,
				&dstats->rb, &fops_s64);

		dfs->next_id = (dfs->next_id + 1) % ULLONG_MAX;
	}

	spin_unlock_irqrestore(&dfs_lock, flags);
}

void mpc_dfs_unregister(struct mpc_dfs *dfs, struct mpc_dfs_stats *dstats)
{
	unsigned long flags;

	debugfs_remove_recursive(dstats->dir);

	spin_lock_irqsave(&dfs_lock, flags);
	dfs->alive--;
	spin_unlock_irqrestore(&dfs_lock, flags);
}

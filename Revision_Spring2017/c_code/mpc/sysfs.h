
#include <linux/kobject.h>
#include <linux/sysctl.h>
#include <net/tcp.h>

#include "model.h"

#ifndef SYSFS_H
#define SYSFS_H

struct mpc_settings {
	bool has_kobj;
	struct kobject kobj;

	unsigned int daddr;
	unsigned int port;
	unsigned int weight;     // %
	unsigned int learn_rate; // %
	unsigned int over;       // us
	unsigned int min_rate;   // mbits/s
	unsigned int max_rate;   // mbits/s
	unsigned int c1;         // out of 1000000
	unsigned int c2;         // out of 1000000
};
#define to_mpc_settings(x) container_of(x, struct mpc_settings, kobj)


struct mpc_attribute {
	struct attribute attr;
	ssize_t (*show)(struct mpc_settings *settings, struct mpc_attribute *attr, char *buf);
	ssize_t (*store)(struct mpc_settings *settings, struct mpc_attribute *attr, const char *buf, size_t count);
};
#define to_mpc_attr(x) container_of(x, struct mpc_attribute, attr)


int mpc_sysfs_register(struct mpc_settings *settings, struct kset *set,
	unsigned int daddr, unsigned int port,
	unsigned int weight, unsigned int learn_rate, unsigned int over,
	unsigned int min_rate, unsigned int max_rate,
	unsigned int c1, unsigned int c2);

int mpc_sysfs_unregister(struct mpc_settings *settings);

void mpc_sysfs_set_model(struct mpc_settings *settings, struct model *md);

#endif /* end of include guard: SYSFS_H */

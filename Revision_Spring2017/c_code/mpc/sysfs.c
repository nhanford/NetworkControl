
#include "sysfs.h"


static unsigned long id_count = 0;


static ssize_t settings_attr_show(struct kobject *kobj,
	struct attribute *attr, char *buf)
{
	struct mpc_attribute *attribute;
	struct mpc_settings *settings;

	attribute = to_mpc_attr(attr);
	settings = to_mpc_settings(kobj);

	if (!attribute->show)
		return -EIO;

	return attribute->show(settings, attribute, buf);
}


static ssize_t settings_attr_store(struct kobject *kobj,
	struct attribute *attr, const char *buf, size_t len)
{
	struct mpc_attribute *attribute;
	struct mpc_settings *settings;

	attribute = to_mpc_attr(attr);
	settings = to_mpc_settings(kobj);

	if (!attribute->store)
		return -EIO;

	return attribute->store(settings, attribute, buf, len);
}


static const struct sysfs_ops sysfs_ops = {
	.show = settings_attr_show,
	.store = settings_attr_store,
};


static void release(struct kobject *kobj)
{
	//struct mpc_settings *settings = to_mpc_settings(kobj);
}


static ssize_t mpc_attr_show(struct mpc_settings *settings, struct mpc_attribute *attr,
	char *buf)
{
	int var;

	if (strcmp(attr->attr.name, "daddr") == 0)
		var = settings->daddr;
	if (strcmp(attr->attr.name, "port") == 0)
		var = settings->port;
	if (strcmp(attr->attr.name, "weight") == 0)
		var = settings->weight;
	if (strcmp(attr->attr.name, "learn_rate") == 0)
		var = settings->learn_rate;
	if (strcmp(attr->attr.name, "over") == 0)
		var = settings->over;
	if (strcmp(attr->attr.name, "min_rate") == 0)
		var = settings->min_rate;
	if (strcmp(attr->attr.name, "max_rate") == 0)
		var = settings->max_rate;
	if (strcmp(attr->attr.name, "c1") == 0)
		var = settings->c1;
	if (strcmp(attr->attr.name, "c2") == 0)
		var = settings->c2;

	return sprintf(buf, "%d\n", var);
}


static ssize_t mpc_attr_store(struct mpc_settings *settings, struct mpc_attribute *attr,
	const char *buf, size_t count)
{
	int var, ret;

	ret = kstrtoint(buf, 10, &var);
	if (ret < 0)
		return ret;

	// NOTE: Don't do the following, it should never be set.
	//if (strcmp(attr->attr.name, "port") == 0)
	//	settings->port = var;
	if (strcmp(attr->attr.name, "weight") == 0)
		settings->weight = var;
	if (strcmp(attr->attr.name, "learn_rate") == 0)
		settings->learn_rate = var;
	if (strcmp(attr->attr.name, "over") == 0)
		settings->over = var;
	if (strcmp(attr->attr.name, "min_rate") == 0)
		settings->min_rate = var;
	if (strcmp(attr->attr.name, "max_rate") == 0)
		settings->max_rate = var;
	if (strcmp(attr->attr.name, "c1") == 0)
		settings->c1 = var;
	if (strcmp(attr->attr.name, "c2") == 0)
		settings->c2 = var;

	return count;
}

static struct mpc_attribute daddr_attribute =
	__ATTR(daddr, 0444, mpc_attr_show, mpc_attr_store);
static struct mpc_attribute port_attribute =
	__ATTR(port, 0444, mpc_attr_show, mpc_attr_store);
static struct mpc_attribute weight_attribute =
	__ATTR(weight, 0664, mpc_attr_show, mpc_attr_store);
static struct mpc_attribute learn_rate_attribute =
	__ATTR(learn_rate, 0664, mpc_attr_show, mpc_attr_store);
static struct mpc_attribute over_attribute =
	__ATTR(over, 0664, mpc_attr_show, mpc_attr_store);
static struct mpc_attribute min_rate_attribute =
	__ATTR(min_rate, 0664, mpc_attr_show, mpc_attr_store);
static struct mpc_attribute max_rate_attribute =
	__ATTR(max_rate, 0664, mpc_attr_show, mpc_attr_store);
static struct mpc_attribute c1_attribute =
	__ATTR(c1, 0664, mpc_attr_show, mpc_attr_store);
static struct mpc_attribute c2_attribute =
	__ATTR(c2, 0664, mpc_attr_show, mpc_attr_store);


static struct attribute *default_attrs[] = {
	&daddr_attribute.attr,
	&port_attribute.attr,
	&weight_attribute.attr,
	&learn_rate_attribute.attr,
	&over_attribute.attr,
	&min_rate_attribute.attr,
	&max_rate_attribute.attr,
	&c1_attribute.attr,
	&c2_attribute.attr,
	NULL,
};


static struct kobj_type ktype = {
	.sysfs_ops = &sysfs_ops,
	.release = release,
	.default_attrs = default_attrs,
};


int mpc_sysfs_register(struct mpc_settings *settings, struct kset *set,
	unsigned int daddr, unsigned int port,
	unsigned int weight, unsigned int learn_rate, unsigned int over,
	unsigned int min_rate, unsigned int max_rate,
	unsigned int c1, unsigned int c2)
{
	int retval;

	memset(settings, 0, sizeof(*settings));
	settings->kobj.kset = set;

	retval = kobject_init_and_add(&settings->kobj, &ktype, NULL, "%lu", id_count);
	if (retval) {
		settings->has_kobj = false;
		kobject_put(&settings->kobj);
	} else {
		settings->has_kobj = true;
		kobject_uevent(&settings->kobj, KOBJ_ADD);
	}
	id_count++;

	settings->daddr = daddr;
	settings->port = port;
	settings->weight = weight;
	settings->learn_rate = learn_rate;
	settings->over = over;
	settings->min_rate = min_rate;
	settings->max_rate = max_rate;
	settings->c1 = c1;
	settings->c2 = c2;

	if (settings->has_kobj)
		return 0;
	else
		return 1;
}


int mpc_sysfs_unregister(struct mpc_settings *settings)
{
	if (settings->has_kobj) {
		kobject_put(&settings->kobj);
	}

	return 0;
}


void mpc_sysfs_set_model(struct mpc_settings *settings, struct model *md)
{
	model_change(md,
		scaled_from_frac(settings->weight, 100),
		5 << 3,
		5,
		scaled_from_frac(settings->learn_rate, 100),
		// Convert mbits to bytes.
		scaled_from_int(settings->min_rate, 20-3),
		scaled_from_int(settings->max_rate, 20-3),
		scaled_from_int(settings->over, 0),
		scaled_from_frac(settings->c1, 1000000),
		scaled_from_frac(settings->c2, 1000000));
}

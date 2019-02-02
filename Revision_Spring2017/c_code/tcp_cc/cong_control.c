/*
 * net/sched/mpc_cc.c	A congestion control algorithm based on model predictive
 * control.
 *
 *		This program is free software; you can redistribute it and/or
 *		modify it under the terms of the GNU General Public License
 *		as published by the Free Software Foundation; either version
 *		2 of the License, or (at your option) any later version.
 *
 * Authors:	Taran Lynn <taranlynn0@gmail.com>
 */

#include <linux/kobject.h>
#include <linux/module.h>
#include <linux/sysctl.h>
#include <net/tcp.h>

#include "../mpc/control.h"

static unsigned int id_counter = 0;
static struct kset *mpc_kset;
static struct mpc_dfs debugfs;

struct control {
	struct model *md;
	u32 rate;

	bool has_kobj;
	struct kobject kobj;

	struct ctl_table parent_sysctl_tbl;
	struct ctl_table sysctl_tbl[8];

	int weight;     // %
	int learn_rate; // %
	int over;       // us
	int min_rate;   // mbits/s
	int max_rate;   // mbits/s
	int c1;         // out of 1000000
	int c2;         // out of 1000000
};
#define to_control(x) container_of(x, struct control, kobj)


struct control_attribute {
	struct attribute attr;
	ssize_t (*show)(struct control *ctl, struct control_attribute *attr, char *buf);
	ssize_t (*store)(struct control *ctl, struct control_attribute *attr, const char *buf, size_t count);
};
#define to_control_attr(x) container_of(x, struct control_attribute, attr)


//
// sysfs control functions
//
static ssize_t control_attr_show(struct kobject *kobj,
	struct attribute *attr, char *buf)
{
	struct control_attribute *attribute;
	struct control *ctl;

	attribute = to_control_attr(attr);
	ctl = to_control(kobj);

	if (!attribute->show)
		return -EIO;

	return attribute->show(ctl, attribute, buf);
}


static ssize_t control_attr_store(struct kobject *kobj,
	struct attribute *attr, const char *buf, size_t len)
{
	struct control_attribute *attribute;
	struct control *ctl;

	attribute = to_control_attr(attr);
	ctl = to_control(kobj);

	if (!attribute->store)
		return -EIO;

	return attribute->store(ctl, attribute, buf, len);
}


static const struct sysfs_ops control_sysfs_ops = {
	.show = control_attr_show,
	.store = control_attr_store,
};


static void control_release(struct kobject *kobj)
{
	struct control *ctl;

	ctl = to_control(kobj);

	if (ctl->md != NULL) {
		mpc_dfs_unregister(&debugfs, &ctl->md->stats);
		model_release(ctl->md);
		kfree(ctl->md);
	}
}


static ssize_t attr_show(struct control *ctl, struct control_attribute *attr,
	char *buf)
{
	int var;

	if (strcmp(attr->attr.name, "weight") == 0)
		var = ctl->weight;
	if (strcmp(attr->attr.name, "learn_rate") == 0)
		var = ctl->learn_rate;
	if (strcmp(attr->attr.name, "over") == 0)
		var = ctl->over;
	if (strcmp(attr->attr.name, "min_rate") == 0)
		var = ctl->min_rate;
	if (strcmp(attr->attr.name, "max_rate") == 0)
		var = ctl->max_rate;
	if (strcmp(attr->attr.name, "c1") == 0)
		var = ctl->c1;
	if (strcmp(attr->attr.name, "c2") == 0)
		var = ctl->c2;

	return sprintf(buf, "%d\n", var);
}


static ssize_t attr_store(struct control *ctl, struct control_attribute *attr,
	const char *buf, size_t count)
{
	int var, ret;

	ret = kstrtoint(buf, 10, &var);
	if (ret < 0)
		return ret;

	if (strcmp(attr->attr.name, "weight") == 0)
		ctl->weight = var;
	if (strcmp(attr->attr.name, "learn_rate") == 0)
		ctl->learn_rate = var;
	if (strcmp(attr->attr.name, "over") == 0)
		ctl->over = var;
	if (strcmp(attr->attr.name, "min_rate") == 0)
		ctl->min_rate = var;
	if (strcmp(attr->attr.name, "max_rate") == 0)
		ctl->max_rate = var;
	if (strcmp(attr->attr.name, "c1") == 0)
		ctl->c1 = var;
	if (strcmp(attr->attr.name, "c2") == 0)
		ctl->c2 = var;

	return count;
}

static struct control_attribute weight_attribute =
	__ATTR(weight, 0664, attr_show, attr_store);
static struct control_attribute learn_rate_attribute =
	__ATTR(learn_rate, 0664, attr_show, attr_store);
static struct control_attribute over_attribute =
	__ATTR(over, 0664, attr_show, attr_store);
static struct control_attribute min_rate_attribute =
	__ATTR(min_rate, 0664, attr_show, attr_store);
static struct control_attribute max_rate_attribute =
	__ATTR(max_rate, 0664, attr_show, attr_store);
static struct control_attribute c1_attribute =
	__ATTR(c1, 0664, attr_show, attr_store);
static struct control_attribute c2_attribute =
	__ATTR(c2, 0664, attr_show, attr_store);


static struct attribute *control_default_attrs[] = {
	&weight_attribute.attr,
	&learn_rate_attribute.attr,
	&over_attribute.attr,
	&min_rate_attribute.attr,
	&max_rate_attribute.attr,
	&c1_attribute.attr,
	&c2_attribute.attr,
	NULL,
};


static struct kobj_type control_ktype = {
	.sysfs_ops = &control_sysfs_ops,
	.release = control_release,
	.default_attrs = control_default_attrs,
};

//
//
//


// Set the pacing rate. rate is in bytes/sec.
inline void set_rate(struct sock *sk) {
	struct control *ctl = inet_csk_ca(sk);
	struct tcp_sock *tp = tcp_sk(sk);

	sk->sk_pacing_rate = ctl->rate;
	printk(KERN_INFO "mpc: rate %u\n", ctl->rate);

	tp->snd_cwnd = 10000;//max_t(u32, 1, (ctl->rate / tp->mss_cache)
			//* tp->srtt_us / USEC_PER_SEC);
}


inline void set_model_params(struct control *ctl)
{
	model_change(ctl->md,
		scaled_from_frac(ctl->weight, 100),
		5 << 3,
		5,
		scaled_from_frac(ctl->learn_rate, 100),
		// Convert mbits to bytes.
		scaled_from_int(ctl->min_rate, 20-3),
		scaled_from_int(ctl->max_rate, 20-3),
		scaled_from_int(ctl->over, 0),
		scaled_from_frac(ctl->c1, 1000000),
		scaled_from_frac(ctl->c2, 1000000));
}


void mpc_cc_init(struct sock *sk)
{
	struct control *ctl = inet_csk_ca(sk);
	struct tcp_sock *tp = tcp_sk(sk);

	tp->snd_ssthresh = TCP_INFINITE_SSTHRESH;

	ctl->weight = 10;
	ctl->learn_rate = 10;
	ctl->over = 200;
	ctl->min_rate = 1 << 10;
	ctl->max_rate = 25 << 10;
	ctl->c1 = 400000;
	ctl->c2 = 10000;

	int retval;

	ctl->kobj.kset = mpc_kset;

	retval = kobject_init_and_add(&ctl->kobj, &control_ktype, NULL, "%d", id_counter);
	if (retval) {
		ctl->has_kobj = false;
		kobject_put(&ctl->kobj);
	} else {
		ctl->has_kobj = true;
	}
	id_counter++;

	kobject_uevent(&ctl->kobj, KOBJ_ADD);


	ctl->md = kmalloc(sizeof(struct model), GFP_KERNEL);
	model_init(ctl->md,
		scaled_from_frac(ctl->weight, 100),
		5 << 3,
		5,
		scaled_from_frac(ctl->learn_rate, 100),
		// Convert mbits to bytes.
		scaled_from_int(ctl->min_rate, 20-3),
		scaled_from_int(ctl->max_rate, 20-3),
		scaled_from_int(ctl->over, 0),
		scaled_from_frac(ctl->c1, 1000000),
		scaled_from_frac(ctl->c2, 1000000));

	mpc_dfs_register(&debugfs, &ctl->md->stats);
}


void mpc_cc_release(struct sock *sk)
{
	struct control *ctl = inet_csk_ca(sk);

	if (ctl->has_kobj) {
		kobject_put(&ctl->kobj);
		ctl->has_kobj = false;
	}
}


u32 mpc_cc_ssthresh(struct sock *sk)
{
	struct tcp_sock *tp = tcp_sk(sk);

	return tp->snd_ssthresh;
}


void mpc_cc_avoid(struct sock *sk, u32 ack, u32 acked)
{
}


u32 mpc_cc_undo_cwnd(struct sock *sk)
{
	struct control *ctl = inet_csk_ca(sk);
	struct tcp_sock *tp = tcp_sk(sk);

	return tp->snd_cwnd;
}


void mpc_cc_pkts_acked(struct sock *sk, const struct ack_sample *sample)
{
	// sample->rtt_us = RTT of acknowledged packet.
}


void mpc_cc_main(struct sock *sk, const struct rate_sample *rs)
{
	struct control *ctl = inet_csk_ca(sk);
	struct tcp_sock *tp = tcp_sk(sk);

	// rs->rtt_us = RTT of last packet to be acknowledged.
	// tp->srtt_us = WMA of RTT
	// tp->tp->mdev_us = Variance of WMA of RTT

	if (ctl->md != NULL && rs->rtt_us > 0) {
		set_model_params(ctl);

		ctl->rate = STI(control_process(ctl->md, SFI(0, 0),
						SFI(ctl->rate, 0),
						SFI(rs->rtt_us, 0)));
		set_rate(sk);
	}
}


static struct tcp_congestion_ops tcp_mpc_cc_cong_ops __read_mostly = {
	.flags          = TCP_CONG_NON_RESTRICTED,
	.name           = "mpc_cc",
	.owner          = THIS_MODULE,

	.init           = mpc_cc_init,
	.release        = mpc_cc_release,
	.ssthresh       = mpc_cc_ssthresh,
	.cong_avoid     = mpc_cc_avoid,
	.undo_cwnd      = mpc_cc_undo_cwnd,
	.pkts_acked     = mpc_cc_pkts_acked,
	.cong_control   = mpc_cc_main,
};

static int __init mpc_cc_mod_init(void)
{
	printk(KERN_INFO "mpc: module init\n");

	mpc_kset = kset_create_and_add("mpccc", NULL, kernel_kobj);
	if (!mpc_kset)
		return -ENOMEM;

	mpc_dfs_init(&debugfs);
	tcp_register_congestion_control(&tcp_mpc_cc_cong_ops);

	return 0;
}

static void __exit mpc_cc_mod_exit(void)
{
	printk(KERN_INFO "mpc: module exit\n");
	tcp_unregister_congestion_control(&tcp_mpc_cc_cong_ops);
	mpc_dfs_release(&debugfs);
	kset_unregister(mpc_kset);
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Taran Lynn");
MODULE_DESCRIPTION("Model Predictive Congestion Control");
MODULE_VERSION("0.01");

module_init(mpc_cc_mod_init);
module_exit(mpc_cc_mod_exit);

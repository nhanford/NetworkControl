
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/skbuff.h>
#include <net/pkt_sched.h>


#ifndef SCH_HI_H
#define SCH_HI_H

#define hi_log(args, ...) printk(KERN_INFO "qdisk hi: " args, ##__VA_ARGS__)

extern struct Qdisc_ops hi_qdisc_ops;

#endif /* end of include guard: SCH_HI_H */


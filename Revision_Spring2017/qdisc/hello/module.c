#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>

#include "sch_hi.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Taran Lynn");
MODULE_DESCRIPTION("A simple example Qdisc.");
MODULE_VERSION("0.01");

static int __init hiqd_init(void) {
  register_qdisc(&hi_qdisc_ops);
  printk(KERN_INFO "Registered qdisc %s\n", hi_qdisc_ops.id);
  printk(KERN_INFO "Loaded module hiqd\n");

  return 0;
}

static void __exit hiqd_exit(void) {
  unregister_qdisc(&hi_qdisc_ops);
  printk(KERN_INFO "Unregistered qdisc %s\n", hi_qdisc_ops.id);
  printk(KERN_INFO "Removed module hiqd\n");
}

module_init(hiqd_init);
module_exit(hiqd_exit);


#include <linux/printk.h>

#ifndef MPC_UTIL_H
#define MPC_UTIL_H

#define mpc_log(args, ...) printk(KERN_INFO "mpc: " args, ##__VA_ARGS__)

#endif /* end of include guard: MPC_UTIL_H */


#include <linux/printk.h>

#ifndef MPC_UTIL_H
#define MPC_UTIL_H

#define mpc_log(args, ...) printk(KERN_INFO "mpc: " args, ##__VA_ARGS__)

#ifndef USEC_PER_SEC
#define USEC_PER_SEC 1000000
#endif

#define MB_PER_B (1 << 20)

#endif /* end of include guard: MPC_UTIL_H */

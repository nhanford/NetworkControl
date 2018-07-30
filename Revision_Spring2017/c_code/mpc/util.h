
#include <linux/printk.h>

#ifndef MPC_UTIL_H
#define MPC_UTIL_H

#define mpc_log(args, ...) printk(KERN_INFO "mpc: " args, ##__VA_ARGS__)

#ifndef USEC_PER_SEC
#define USEC_PER_SEC 1000000
#endif

#define MPC_ONE 128

static inline u64 wma(u64 weight, u64 avg, u64 x)
{
	return (MPC_ONE - weight)*avg/MPC_ONE + weight*x/MPC_ONE;
}

#endif /* end of include guard: MPC_UTIL_H */

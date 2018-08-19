
#include <linux/printk.h>

#ifndef MPC_UTIL_H
#define MPC_UTIL_H

#define mpc_log(args, ...) printk(KERN_INFO "mpc: " args, ##__VA_ARGS__)

#ifndef USEC_PER_SEC
#define USEC_PER_SEC 1000000
#endif

// Bytes/s
#define MPC_MIN_RATE (1UL)
#define MPC_MAX_RATE (100UL << 10)

#define MPC_ONE 128

static inline s64 wma(s64 weight, s64 avg, s64 x)
{
	return (MPC_ONE - weight)*avg/MPC_ONE + weight*x/MPC_ONE;
}

#endif /* end of include guard: MPC_UTIL_H */

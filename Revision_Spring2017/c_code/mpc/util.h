
#include <linux/printk.h>

#include "scaled_shorthand.h"

#ifndef MPC_UTIL_H
#define MPC_UTIL_H

#ifndef USEC_PER_SEC
#define USEC_PER_SEC 1000000
#endif

// Bytes/s
#define MPC_MIN_RATE scaled_from_int(1, 27) // 1 Gbps
#define MPC_MAX_RATE scaled_from_int(U32_MAX, 0)

static inline scaled wma(scaled weight, scaled avg,
	scaled x)
{
	return SA(SM(SS(ONE, weight), avg), SM(weight, x));
}

#endif /* end of include guard: MPC_UTIL_H */

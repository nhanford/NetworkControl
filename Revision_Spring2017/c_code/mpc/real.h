
#include <linux/types.h>

#include "util.h"

#ifndef MPC_REAL_H
#define MPC_REAL_H


#define REAL_PREC 20
#define RA(x, y) real_add((x), (y))
#define RS(x, y) real_sub((x), (y))
#define RM(x, y) real_mul((x), (y))
#define RD(x, y) real_div((x), (y))


#define RI_MAX 18446744073709551615U
typedef u64 real_int;

typedef struct {
  real_int value;
} real;


static const real REAL_ZERO = { 0 };
static const real REAL_ONE = { 1 << REAL_PREC };
static const real REAL_MAX = { RI_MAX };


static inline real real_from_int(real_int x)
{
    if(x > RI_MAX >> REAL_PREC) {
        mpc_log("from_int overflow %llu\n", x);
        return REAL_MAX;
    }

    return (real) { x << REAL_PREC };
}

static inline real real_from_frac(real_int num, real_int den)
{
    if(num > RI_MAX >> REAL_PREC) {
        if(num/den > RI_MAX >> REAL_PREC) {
            mpc_log("from_frac overflow %llu/%llu\n", num, den);
            return REAL_MAX;
        } else {
            return (real) { (num/den) << REAL_PREC };
        }
    } else {
        return (real) { (num << REAL_PREC) / den };
    }
}

static inline real_int real_floor(real x)
{
  return x.value >> REAL_PREC;
}


static inline real real_add(real x, real y)
{
    if(x.value > RI_MAX - y.value) {
        mpc_log("add overflow %llu + %llu\n", x.value, y.value);
        return REAL_MAX;
    }

    return (real) { x.value + y.value };
}

static inline real real_sub(real x, real y)
{
    if(x.value < y.value) {
        mpc_log("sub underflow %llu - %llu\n", x.value, y.value);
        return REAL_ZERO;
    }

    return (real) { x.value - y.value };
}

static inline real real_mul(real x, real y)
{
    // Avoid overflow if possible.
    if(x.value != 0 && y.value != 0 && x.value > RI_MAX / y.value) {
        if(x.value > y.value) {
            if((x.value >> REAL_PREC) > RI_MAX / y.value) {
                mpc_log("mul overflow %llu * %llu\n", x.value, y.value);
                return REAL_MAX;
            }

            return (real) { (x.value >> REAL_PREC) * y.value };
        } else {
            if((y.value >> REAL_PREC) > RI_MAX / x.value) {
                mpc_log("mul overflow %llu * %llu\n", x.value, y.value);
                return REAL_MAX;
            }

            return (real) { x.value * (y.value >> REAL_PREC) };
        }
    } else {
        return (real) { (x.value * y.value) >> REAL_PREC };
    }
}

static inline real real_div(real x, real y)
{
    if(y.value == 0) {
        mpc_log("div by 0, %llu/0\n", x.value);
        // The logic here is that y is probably an extremely small value.
        return REAL_MAX;
    }

    return (real) { (x.value << REAL_PREC) / y.value };
}


static inline real real_ls(real x, real_int y)
{
    return (real) { x.value << y };
}

static inline real real_rs(real x, real_int y)
{
    return (real) { x.value >> y };
}


static inline bool real_lt(real x, real y)
{
    return x.value < y.value;
}

static inline bool real_gt(real x, real y)
{
    return x.value > y.value;
}


#endif /* end of include guard: MPC_REAL_H */


/*
 * FIXME: This is terrible, or at least I think it's terrible, implementation of
 * software floating point numbers. Ideally I'd use something from the kernel,
 * but I'm still looking for a solution.
 *
 * We need floating point because otherwise we would get overflows/underflows
 * galore.
 */

#include <linux/types.h>

#include "util.h"

#ifndef MPC_REAL_H
#define MPC_REAL_H


// The approximate location of the decimal point in terms of bits.
#define REAL_P 31
#define RA(x, y) real_add((x), (y))
#define RS(x, y) real_sub((x), (y))
#define RM(x, y) real_mul((x), (y))
#define RD(x, y) real_div((x), (y))

#define REAL_ZERO (real_from_int(0))
#define REAL_ONE (real_from_int(1))

#define RI_MAX 18446744073709551615U
typedef u64 real_int;
typedef s64 real_exp;

typedef struct {
  real_int sig;
  real_exp exp;
} real;


// Sets the decimal point so that we have only one digit greater then 1.
//
// For example, if REAL_P = 3, then 16*2^0 would go to 8*2^1, and 0.5 would go
// to 8*2^(-4).
static inline void real_set_point(real *x)
{
    real_int cntr = x->sig;
    real_exp bits = 0;
    real_exp shft;

    while(cntr > 0) {
        cntr >>= 1;
        bits++;
    }

    shft = REAL_P - bits;

    if(shft < 0)
        x->sig >>= (-shft);
    else
        x->sig <<= shft;

    x->exp -= shft;
}


static inline real real_from_int(real_int x)
{
    real res;
    res.sig = x;
    res.exp = 0;

    real_set_point(&res);

    return res;
}

static inline real real_from_frac(real_int num, real_int den)
{
    real res;
    res.sig = (num << REAL_P)/den;
    res.exp = -REAL_P;

    real_set_point(&res);

    return res;
}

static inline real_int real_floor(real x)
{
    if(x.exp < 0)
        return x.sig >> (-x.exp);
    else
        return x.sig << x.exp;
}


static inline real real_add(real x, real y)
{
    real res;
    real_int exp_diff;

    // Give both numbers the exponent of the larger of the two.
    if(x.exp > y.exp) {
        exp_diff = x.exp - y.exp;
        y.sig >>= exp_diff;
        y.exp += exp_diff;
    } else if(x.exp < y.exp) {
        exp_diff = y.exp - x.exp;
        x.sig >>= exp_diff;
        x.exp += exp_diff;
    }

    if(x.sig > RI_MAX - y.sig)
        mpc_log("real_add overflow\n");

    res.sig = x.sig + y.sig;
    res.exp = x.exp;

    real_set_point(&res);

    return res;
}

static inline real real_sub(real x, real y)
{
    real res;
    real_int exp_diff;

    // Give both numbers the exponent of the larger of the two.
    if(x.exp > y.exp) {
        exp_diff = x.exp - y.exp;
        y.sig >>= exp_diff;
        y.exp += exp_diff;
    } else if(x.exp < y.exp) {
        exp_diff = y.exp - x.exp;
        x.sig >>= exp_diff;
        x.exp += exp_diff;
    }

    if(x.sig < y.sig) {
        mpc_log("real_sub underflow, (%llu - %llu) * 2^%lld\n", x.sig, y.sig, y.exp);
        return REAL_ZERO;
    }

    res.sig = x.sig - y.sig;
    res.exp = x.exp;

    real_set_point(&res);

    return res;
}

static inline real real_mul(real x, real y)
{
    real res;

    if(y.sig != 0 && x.sig > RI_MAX/y.sig)
        mpc_log("real_mul overflow, %llu * 2^%lld * %llu * 2^%lld\n",
                x.sig, x.exp, y.sig, y.exp);

    res.sig = x.sig * y.sig;
    res.exp = x.exp + y.exp;

    real_set_point(&res);

    return res;
}

static inline real real_div(real x, real y)
{
    real res;

    if(y.sig == 0) {
        mpc_log("real_div got 0");
        return REAL_ZERO;
    }

    res.sig = (x.sig << REAL_P) / y.sig;
    res.exp = x.exp + REAL_P - y.exp;

    real_set_point(&res);

    return res;
}


static inline real real_ls(real x, real_int y)
{
    real res;
    res.sig = x.sig;
    res.exp = x.exp + y;

    real_set_point(&res);

    return res;
}

static inline real real_rs(real x, real_int y)
{
    real res;
    res.sig = x.sig;
    res.exp = x.exp - y;

    real_set_point(&res);

    return res;
}


static inline bool real_is_zero(real x)
{
    return x.sig == 0;
}

static inline bool real_lt(real x, real y)
{
    // FIXME: This is horribly inefficient, it's basically almost doing
    // subtraction.

    real_int exp_diff;

    // Give both numbers the exponent of the larger of the two.
    if(x.exp > y.exp) {
        exp_diff = x.exp - y.exp;
        y.sig >>= exp_diff;
        y.exp += exp_diff;
    } else if(x.exp < y.exp) {
        exp_diff = y.exp - x.exp;
        x.sig >>= exp_diff;
        x.exp += exp_diff;
    }

    return x.sig < y.sig;
}

static inline bool real_gt(real x, real y)
{
    return real_lt(y, x);
}


#endif /* end of include guard: MPC_REAL_H */

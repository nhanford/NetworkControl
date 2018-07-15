
#include <linux/types.h>

#define REAL_PREC 20
#define RA(x, y) real_add((x), (y))
#define RS(x, y) real_sub((x), (y))
#define RM(x, y) real_mul((x), (y))
#define RD(x, y) real_div((x), (y))


typedef u64 real_int;

typedef struct {
  real_int value;
} real;


static const real REAL_ZERO = { 0 };
static const real REAL_ONE = { 1 << REAL_PREC };


static inline real real_from_int(real_int x)
{
    return (real) { x << REAL_PREC };
}

static inline real real_from_frac(real_int num, real_int den)
{
    return (real) { (num << REAL_PREC) / den };
}

static inline real_int real_floor(real x)
{
  return x.value >> REAL_PREC;
}


static inline real real_add(real x, real y)
{
    return (real) { x.value + y.value };
}

static inline real real_sub(real x, real y)
{
    return (real) { x.value + y.value };
}

static inline real real_mul(real x, real y)
{
    return (real) { (x.value * y.value) >> REAL_PREC };
}

static inline real real_div(real x, real y)
{
    return (real) { (x.value << REAL_PREC) / y.value };
}


static inline bool real_eq(real x, real y)
{
    return x.value == y.value;
}

static inline bool real_lt(real x, real y)
{
    return x.value < y.value;
}

static inline bool real_gt(real x, real y)
{
    return x.value > y.value;
}

static inline bool real_lte(real x, real y)
{
    return x.value <= y.value;
}

static inline bool real_gte(real x, real y)
{
    return x.value >= y.value;
}


#include <linux/types.h>
//#include <stdbool.h>
//#include <stdlib.h>


#ifndef SCALED_H
#define SCALED_H


#define SCALED_PREC 16
typedef s64 scaled_int;
//typedef int64_t scaled_int;

typedef struct {
    scaled_int base;
    scaled_int exp;
} scaled;

#define ZERO ((scaled) {0, 0})
#define ONE scaled_from_int(1, 0)
#define TWO scaled_from_int(2, 0)


scaled scaled_from_int(scaled_int base, scaled_int exp);
scaled_int scaled_to_int(scaled x);

scaled negate(scaled x);

scaled scaled_add(scaled x, scaled y);
scaled scaled_sub(scaled x, scaled y);
scaled scaled_mul(scaled x, scaled y);
scaled scaled_div(scaled x, scaled y);
scaled scaled_sqr(scaled x);

bool scaled_lt(scaled x, scaled y);
bool scaled_eq(scaled x, scaled y);
bool scaled_lte(scaled x, scaled y);

scaled scaled_min(scaled x, scaled y);
scaled scaled_max(scaled x, scaled y);


#endif /* end of include guard: SCALED_H */
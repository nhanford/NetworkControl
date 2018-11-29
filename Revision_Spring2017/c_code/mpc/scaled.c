
#include "scaled.h"


static scaled_int log2i(scaled_int x)
{
	scaled_int res = 0;

	while (x > 1) {
		x >>= 1;
		res += 1;
	}

	return res;
}

static scaled_int shift(scaled_int x, scaled_int sft)
{
	bool neg = x < 0;

	if (neg)
		x = -x;

	if (sft < 0)
		x >>= -sft;
	else
		x <<= sft;

	if (neg)
		x = -x;

	return x;
}


void set_prec(scaled *x)
{
	scaled_int offset = log2i(x->base) - SCALED_PREC;
	x->base = shift(x->base, -offset);

	if (x->base == 0)
		x->exp = 0;
	else
		x->exp += offset;
}

scaled scaled_from_int(scaled_int base, scaled_int exp)
{
	scaled ret = {base, exp};
	set_prec(&ret);
	return ret;
}

scaled scaled_from_frac(scaled_int numer, scaled_int denom)
{
	return scaled_div(scaled_from_int(numer, 0), scaled_from_int(denom, 0));
}

scaled_int scaled_to_int(scaled x)
{
	return shift(x.base, x.exp);
}

scaled scaled_negate(scaled x)
{
	return (scaled){-x.base, x.exp};
}

scaled scaled_add(scaled x, scaled y)
{
	scaled res;
	scaled *a;
	scaled *b;

	if (x.exp >= y.exp) {
		a = &x;
		b = &y;
	} else {
		a = &y;
		b = &x;
	}

	res.base = a->base + shift(b->base, b->exp - a->exp);
	res.exp = a->exp;

	set_prec(&res);

	return res;
}

scaled scaled_sub(scaled x, scaled y)
{
	return scaled_add(x, scaled_negate(y));
}

scaled scaled_mul(scaled x, scaled y)
{
	scaled res;
	res.base = x.base*y.base;
	res.exp = x.exp + y.exp;

	set_prec(&res);

	return res;
}

scaled scaled_div(scaled x, scaled y)
{
	scaled res;
	scaled_int offset = log2i(y.base) - log2i(x.base) + SCALED_PREC;
	res.base = shift(x.base, offset)/y.base;
	res.exp = x.exp - y.exp - offset;

	set_prec(&res);

	return res;
}


scaled scaled_ipow(scaled x, scaled_int n)
{
	scaled res = ONE;
	scaled_int cnt = n;
	scaled_int i;

	if (n < 0)
		cnt = -cnt;

	for (i = 0; i < n; i++)
		res = scaled_mul(res, x);

	if (n < 0)
		res = scaled_div(ONE, res);

	return res;
}


bool scaled_lt(scaled x, scaled y)
{
	return scaled_sub(x, y).base < 0;
}

bool scaled_eq(scaled x, scaled y)
{
	return scaled_sub(x, y).base == 0;
}

bool scaled_lte(scaled x, scaled y)
{
	return scaled_sub(x, y).base <= 0;
}

scaled scaled_min(scaled x, scaled y)
{
	if (scaled_lt(x, y))
		return x;
	else
		return y;
}

scaled scaled_max(scaled x, scaled y)
{
	if (scaled_lt(x, y))
		return y;
	else
		return x;
}

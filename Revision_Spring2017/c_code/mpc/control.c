
#include <linux/limits.h>

#include "control.h"
#include "util.h"


inline s64 sqr(s64 x)
{
	return x*x;
}


static void control_update(struct model *md, s64 rate_meas, s64 rtt_meas);


size_t control_rollover(struct model *md)
{
	return md->num_obs;
}


s64 control_process(struct model *md, s64 time, s64 rate_meas, s64 rtt_meas)
{
	s64 opt;
	s64 xhat;

	if (rate_meas > 0 && rtt_meas > 0)
		control_update(md, rate_meas, rtt_meas);

	if (rtt_meas != 0) {
		opt = md->rb * (MPC_ONE - lookback_index(&md->x, 0)*MPC_ONE/rtt_meas);
		opt /= MPC_ONE;
		opt += md->rate_diff;
		md->stats.probing = false;
	} else {
		opt = md->rb;
		md->stats.probing = false;
	}

	opt = min_t(s64, max_t(s64, MPC_MIN_RATE, opt), MPC_MAX_RATE);

	xhat = lookback_index(&md->x, 0);
	if (md->rb > 0)
		xhat += (rate_meas - md->rb)/md->rb;
	xhat = min_t(s64, max_t(s64, 0, xhat), md->lb - md->lp);
	md->stats.rtt_pred_us = md->a/opt + md->lp + xhat;

	md->stats.rate_set = opt << 20;
	md->stats.rtt_meas_us = rtt_meas;
	md->stats.a = md->a;
	md->stats.lp = md->lp;
	md->stats.rb = md->rb << 20;
	md->stats.x = lookback_index(&md->x, 0);

	return opt;
}


static void control_update(struct model *md, s64 rate_meas, s64 rtt_meas)
{
	s64 k1 = 0;
	s64 k2 = 0;
	s64 k3 = 0;
	s64 k4 = 0;
	s64 T = 0;
	s64 a_new = 0;
	s64 denom = 0;
	size_t i;

	lookback_add(&md->rate, rate_meas);
	lookback_add(&md->rtt, rtt_meas);

	for (i = 0; i < md->num_obs; i++) {
		s64 rate = lookback_index(&md->rate, i);
		s64 rtt = lookback_index(&md->rate, i);

		if (rate != 0) {
			k1 += MPC_ONE/rate;
			k2 += MPC_ONE/sqr(rate);
			k3 += rtt*MPC_ONE/rate;
			k4 += MPC_ONE*rtt;
			T += 1;
		}
	}

	denom = (T*k2*MPC_ONE - sqr(k1));

	if (denom != 0)
		a_new = (T*k3*MPC_ONE - k1*k4)/denom;

	// TODO: make upper limit less arbitrary.
	if (a_new > 0 && a_new < 100*MPC_ONE)
		md->a = a_new;

	lookback_add(&md->x, min_t(s64, max_t(s64, 0,
		rtt_meas - md->a/rate_meas - md->lp), md->lb));

	// Estimate rB
	md->lb = 0;
	md->lp = S64_MAX;
	md->rb = 0;

	for (i = 1; i < md->num_obs; i++) {
		s64 rtt = lookback_index(&md->rtt, i);

		md->lb = max_t(s64, md->lb, rtt);
		md->lp = min_t(s64, md->lp, rtt);

		md->rb = max_t(s64, md->rb, (lookback_index(&md->x, i) + rate_meas)
			/ (lookback_index(&md->x, i - 1) + 1));
	}

	md->avg_rb = wma(md->weight, md->avg_rb, md->rb);
	md->var_rb = wma(md->weight, md->var_rb, sqr(md->rb - md->avg_rb));
	md->avg_x = wma(md->weight, md->avg_x, lookback_index(&md->x, 0));
}

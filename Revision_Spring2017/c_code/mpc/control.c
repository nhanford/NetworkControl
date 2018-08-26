
#include <linux/limits.h>

#include "control.h"
#include "util.h"


inline s64 sqr(s64 x)
{
	return x*x;
}


size_t control_rollover(struct model *md)
{
	return 2;
}


s64 control_process(struct model *md, s64 rtt_meas, s64 rate_meas)
{
	s64 rate_opt = md->rate_last;
	s64 diff = rate_meas - md->rate_last;

	if (diff != 0)
		md->a = (rtt_meas - md->rtt_last)*MPC_ONE/diff;

	if (md->avg_rate != 0 && md->avg_rate_delta != 0) {
		s64 k1 = md->c1*md->avg_rtt/sqr(md->avg_rate_delta);
		s64 k2 = md->c2*md->avg_rtt/md->avg_rate;

		// Be sure MPC_ONE offsets are balanced.
		s64 t1 = k2 - 2*md->a*md->rtt_last;
		s64 t2 = 2*k1 + 2*sqr(md->a)/MPC_ONE;

		if (t2 != 0)
			rate_opt += t1/t2;
		else
			rate_opt += 1;
	} else {
		rate_opt += 1;
	}

	// Clamp rate
	rate_opt = min_t(s64, max_t(s64, rate_opt, MPC_MIN_RATE), MPC_MAX_RATE);

	md->pred_rtt = md->a*(rate_meas - md->rate_last)/MPC_ONE + rtt_meas;
	md->rtt_last = rtt_meas;
	md->rate_last = rate_meas;

	md->avg_rtt = wma(md->weight, md->avg_rtt, rtt_meas);
	md->avg_rate = wma(md->weight, md->avg_rate, rate_meas);
	md->avg_rate_delta = wma(md->weight, md->avg_rate, diff);

	// Convert for external units.
	rate_opt <<= 20;

	// debug
	md->dstats.rtt_meas_us = rtt_meas;
	md->dstats.rtt_pred_us = md->pred_rtt;
	md->dstats.rate_set = rate_opt;

	return rate_opt;
}

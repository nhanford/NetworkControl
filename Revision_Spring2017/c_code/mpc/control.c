
#include <linux/limits.h>

#include "control.h"
#include "util.h"


inline s64 sqr(s64 x)
{
	return x*x;
}


s64 control_process(struct model *md, s64 rtt_meas, bool probe)
{
	s64 rate_opt = md->rate_last;
	s64 diff = md->rate_last - md->rate_last2;

	if (diff != 0)
		md->a = (rtt_meas - md->rtt_last)*MPC_ONE/diff;

	if (probe) {
		rate_opt = MPC_MAX_RATE;
		md->dstats.probing = true;
	} else {
		s64 k1 = md->k1;
		s64 k2 = md->k2;
		s64 a = md->a;
		s64 ml = md->avg_rtt;
		s64 mr = md->avg_rate;

		// Be sure MPC_ONE offsets are balanced.
		s64 t1 = k2*sqr(ml)/2 - a*rtt_meas*sqr(mr);
		s64 t2 = k1*sqr(ml) + sqr(a)*sqr(mr)/MPC_ONE;

		if (t2 != 0)
			rate_opt += t1/t2;
		else
			rate_opt += 1;

		md->dstats.probing = false;
	}

	// Clamp rate
	rate_opt = min_t(s64, max_t(s64, rate_opt, MPC_MIN_RATE), MPC_MAX_RATE);

	md->rtt_last = rtt_meas;
	md->rate_last2 = md->rate_last;
	md->rate_last = rate_opt;
	md->pred_rtt = md->a*(md->rate_last - md->rate_last2)/MPC_ONE + rtt_meas;

	md->avg_rtt = wma(md->weight, md->avg_rtt, rtt_meas);
	md->avg_rate = wma(md->weight, md->avg_rate, rate_opt);

	// Convert for external units.
	rate_opt <<= 20;

	// debug
	md->dstats.rtt_meas_us = rtt_meas;
	md->dstats.rtt_pred_us = md->pred_rtt;
	md->dstats.rate_set = rate_opt;

	return rate_opt;
}

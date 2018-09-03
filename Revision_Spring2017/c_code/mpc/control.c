
#include <linux/limits.h>

#include "control.h"
#include "util.h"


inline s64 sqr(s64 x)
{
	return x*x;
}


static void control_update(struct model *md, s64 rtt_meas, s64 rate_meas);


size_t control_rollover(struct model *md)
{
	return md->num_obs;
}


s64 control_process(struct model *md, s64 time, s64 rtt_meas, s64 rate_meas)
{
	s64 rate;
	s64 opt;

	// debug.
	md->dstats.rtt_meas_us = rtt_meas;

	if (rtt_meas > 0 && rate_meas > 0)
		control_update(md, rtt_meas, rate_meas);
	else
		goto increment;

	if (md->m == 0)
		goto increment;
	else
		opt = -md->b/md->m;

	opt = max_t(s64, MPC_MIN_RATE, min_t(s64, opt, MPC_MAX_RATE));

	md->integral += opt*(time - md->last_time);
	rate = md->integral/(time - md->start_time);
	md->last_time = time;

	rate = max_t(s64, MPC_MIN_RATE, min_t(s64, rate, MPC_MAX_RATE));

	// debug
	md->dstats.rate_set = rate;
	md->dstats.rtt_pred_us = md->m*rate/MPC_ONE + md->b/MPC_ONE;

	return rate;
increment:
	return *lookback_index(&md->lb_rate, 0) + MPC_MIN_RATE;
}


static void control_update(struct model *md, s64 rtt_meas, s64 rate_meas)
{
	s64 oldest_rate = *lookback_index_old(&md->lb_rate, 0);
	s64 oldest_rtt = *lookback_index_old(&md->lb_rtt, 0);
	s64 m_numer;
	s64 b_numer;
	s64 denom;

	/*
	md->sum_rate -= oldest_rate;
	md->sum_rate_sqr -= sqr(oldest_rate);
	md->sum_rtt -= oldest_rtt;
	md->sum_rtt_rate -= oldest_rtt * oldest_rate;

	md->sum_rate += rate_meas;
	md->sum_rate_sqr += sqr(rate_meas);
	md->sum_rtt += rtt_meas;
	md->sum_rtt_rate += rtt_meas * rate_meas;
	*/
	size_t i;
	md->sum_rate = 0;
	md->sum_rate_sqr = 0;
	md->sum_rtt = 0;
	md->sum_rtt_rate = 0;

	for (i = 0; i < md->num_obs; i++) {
		s64 rate = *lookback_index(&md->lb_rate, i);
		s64 rtt = *lookback_index(&md->lb_rtt, i);

		md->sum_rate += rate;
		md->sum_rate_sqr += sqr(rate);
		md->sum_rtt += rtt;
		md->sum_rtt_rate += rtt*rate;
	}

	m_numer = md->num_obs * md->sum_rtt_rate - md->sum_rtt * md->sum_rate;
	b_numer = md->sum_rtt * md->sum_rate_sqr - md->sum_rtt_rate * md->sum_rate;
	denom = md->num_obs * md->sum_rate_sqr - sqr(md->sum_rate);

	if (denom == 0) {
		md->m = 0;
		md->b = rtt_meas;
	} else {
		md->m = m_numer*MPC_ONE/denom;
		md->b = b_numer*MPC_ONE/denom;
	}

	lookback_add(&md->lb_rtt, rtt_meas);
	lookback_add(&md->lb_rate, rate_meas);
}

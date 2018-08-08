
#include <linux/limits.h>

#include "control.h"
#include "util.h"

// (x - y)^2
inline s64 sqr_diff_s64(s64 x, s64 y)
{
	s64 diff;

	if (x > y)
		diff = x - y;
	else
		diff = y - x;

	return diff * diff;
}


s64 control_predict(struct model *md);
void control_update(struct model *md, s64 rtt_meas);


s64 control_process(struct model *md, s64 rtt_meas, s64 rate)
{
	s64 b0;
	s64 rate_opt = *lookback_index(&md->lb_pacing_rate, 0);

	// debug.
	md->dstats.rtt_meas_us = rtt_meas;
	md->dstats.rtt_pred_us = md->predicted_rtt;

	control_update(md, rtt_meas);
	b0 = md->b[0];

	md->avg_rtt = max_t(s64, 1, wma(md->gamma, md->avg_rtt, rtt_meas));

	md->avg_rtt_var = max_t(s64, 1, wma(md->gamma, md->avg_rtt_var,
				sqr_diff_s64(md->predicted_rtt, md->avg_rtt)));


	// Predict RTT assuming current control is 0. This is l^(n + 1)|(r = 0).
	lookback_add(&md->lb_pacing_rate, 0);
	md->predicted_rtt = control_predict(md);

	if (rate > 0) {
		rate_opt = rate;
		md->dstats.probing = true;
	} else if (md->avg_rtt > 0 && md->avg_rtt_var > 0
			&& md->avg_pacing_rate > 0) {
		s64 t1 = (MPC_ONE - md->alpha)/md->alpha;
		s64 t2 = b0 / 2;
		s64 t3 = md->avg_pacing_rate / md->avg_rtt;

		rate_opt = t1*t2*t3;
		rate_opt *= rate_opt;

		md->dstats.probing = false;
	}

	// Clamp rate
	// TODO: Make bounds less arbitrary than 100 mbit/s. 1 << 32 is to limit
	// overflows.
	rate_opt = min_t(s64, max_t(s64, rate_opt,
			((s64) 100) << 17), ((s64) 1) << 32);

	// Now we set predicted RTT to include the control.
	*lookback_index(&md->lb_pacing_rate, 0) = rate_opt;
	md->predicted_rtt = control_predict(md);

	md->avg_pacing_rate = wma(md->gamma, md->avg_pacing_rate, rate_opt);

	// debug
	md->dstats.rate_set = rate_opt;

	return rate_opt;
}


s64 control_predict(struct model *md)
{
	s64 predicted_rtt = 0;
	size_t i = 0;

	for (i = 0; i < md->p; i++) {
		s64 rttRt = int_sqrt(*lookback_index(&md->lb_rtt, i));
		predicted_rtt += md->a[i] * rttRt / MPC_ONE;
	}

	for (i = 0; i < md->q; i++) {
		s64 rateRt = int_sqrt(*lookback_index(&md->lb_pacing_rate, i));
		predicted_rtt += md->b[i] * rateRt / MPC_ONE;
	}

	return max_t(s64, 0, predicted_rtt);
}


void control_update(struct model *md, s64 rtt_meas)
{
	size_t i;
	s64 error;
	s64 sum = 0;

	error = rtt_meas - md->predicted_rtt;


	for (i = 0; i < md->p; i++)
		sum += *lookback_index(&md->lb_rtt, i);

	for (i = 0; i < md->q; i++)
		sum += *lookback_index(&md->lb_pacing_rate, i);

	if (sum == 0)
		goto exit;


	for (i = 0; i < md->p; i++) {
		s64 rttRt = int_sqrt(*lookback_index(&md->lb_rtt, i));
		s64 delta = rttRt * error * MPC_ONE / sum;

		md->a[i] += delta;
	}

	for (i = 0; i < md->q; i++) {
		s64 rateRt = int_sqrt(*lookback_index(&md->lb_pacing_rate, i));
		s64 delta = rateRt * error * MPC_ONE / sum;

		md->b[i] += delta;
	}

exit:
	lookback_add(&md->lb_rtt, rtt_meas);
}


#include <linux/limits.h>

#include "control.h"
#include "util.h"

// (x - y)^2
inline s64 square_diff_s64(s64 x, s64 y)
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
	s64 b0 = md->b[0];
	s64 rate_opt = md->last_rtt;

	// debug.
	md->dstats.rtt_meas_us = rtt_meas;
	md->dstats.rtt_pred_us = md->predicted_rtt;

	control_update(md, rtt_meas);

	md->avg_rtt = max_t(s64, 1, wma(md->gamma, md->avg_rtt, rtt_meas));

	md->avg_rtt_var = max_t(s64, 1, wma(md->gamma, md->avg_rtt_var,
				square_diff_s64(md->predicted_rtt, md->avg_rtt)));


	// Predict RTT assuming current control is 0. This is l^(n + 1)|(r = 0).
	lookback_add(&md->lb_rate_diff, 0);
	md->predicted_rtt = control_predict(md);

	if (rate > 0) {
		rate_opt = rate;
		md->dstats.probing = true;
	} else if (md->avg_rtt > 0 && md->avg_rtt_var > 0
			&& md->avg_pacing_rate > 0 && b0 > 0) {
		s64 ml = md->avg_rtt;
		s64 mr = md->avg_pacing_rate;
		s64 lh = md->predicted_rtt;

		rate_opt += (lh*mr/(ml*ml - b0*b0*mr)) * b0*mr;

		md->dstats.probing = false;
	}

	// Clamp rate
	// TODO: Make bounds less arbitrary than 100 mbit/s. 1 << 32 is to limit
	// overflows.
	rate_opt = min_t(s64, max_t(s64, rate_opt,
			((s64) 100) << 17), ((s64) 1) << 32);

	// Now we set predicted RTT to include the control.
	*lookback_index(&md->lb_rate_diff, 0) = rate_opt - md->last_rate;
	md->last_rate = rate_opt;
	md->predicted_rtt = control_predict(md);

	md->avg_pacing_rate = wma(md->gamma, md->avg_pacing_rate, rate_opt);

	// debug
	md->dstats.rate_set = rate_opt;

	return rate_opt;
}


s64 control_predict(struct model *md)
{
	s64 predicted_rtt = md->target_rtt;
	size_t i = 0;

	for (i = 0; i < md->p; i++)
		predicted_rtt += md->a[i] * (*lookback_index(&md->lb_rtt_diff, i)) / MPC_ONE;

	for (i = 0; i < md->q; i++)
		predicted_rtt += md->b[i] * (*lookback_index(&md->lb_rate_diff, i)) / MPC_ONE;

	return max_t(s64, 0, min_t(s64, predicted_rtt, 4*md->avg_rtt));
}


void control_update(struct model *md, s64 rtt_meas)
{
	size_t i;
	s64 error = rtt_meas - md->predicted_rtt;
	s64 total_norm = 0;

	if (md->target_rtt <= 0 || (rtt_meas > 0 && rtt_meas < md->target_rtt))
		md->target_rtt = rtt_meas;

	for (i = 0; i < md->p; i++) {
		s64 rtt_diff = *lookback_index(&md->lb_rtt_diff, i);

		// Limit to prevent overflow.
		if (total_norm > S64_MAX - rtt_diff*rtt_diff)
			total_norm = S64_MAX;
		else
			total_norm += rtt_diff*rtt_diff;
	}

	for (i = 0; i < md->q; i++) {
		s64 rate_diff = *lookback_index(&md->lb_rate_diff, i);

		if (total_norm > S64_MAX - rate_diff*rate_diff)
			total_norm = S64_MAX;
		else
			total_norm += rate_diff*rate_diff;
	}

	total_norm = max_t(s64, 1, total_norm);


	if (total_norm == 0)
		goto exit;

	for (i = 0; i < md->p; i++) {
		s64 rtt_diff = *lookback_index(&md->lb_rtt_diff, i);
		s64 delta = rtt_diff * md->alpha * error / total_norm;

		md->a[i] += delta;
	}

	for (i = 0; i < md->q; i++) {
		s64 rate_diff = *lookback_index(&md->lb_rate_diff, i);
		s64 delta = rate_diff * md->alpha * error / total_norm;

		md->b[i] += delta;
	}

exit:
	lookback_add(&md->lb_rtt_diff, rtt_meas - md->last_rtt);
	md->last_rtt = rtt_meas;
}

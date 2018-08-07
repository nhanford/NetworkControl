
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
	s64 rate_opt = *lookback_index(&md->lb_pacing_rate, 0);

	// debug.
	md->dstats.rtt_meas_us = rtt_meas;
	md->dstats.rtt_pred_us = md->predicted_rtt;

	control_update(md, rtt_meas);

	md->avg_rtt = max_t(s64, 1, wma(md->gamma, md->avg_rtt, rtt_meas));

	md->avg_rtt_var = max_t(s64, 1, wma(md->gamma, md->avg_rtt_var,
				square_diff_s64(md->predicted_rtt, md->avg_rtt)));


	// Predict RTT assuming current control is 0. This is l^(n + 1)|(r = 0).
	lookback_add(&md->lb_pacing_rate, 0);
	md->predicted_rtt = control_predict(md);

	if (rate > 0) {
		rate_opt = rate;
		md->dstats.probing = true;
	} else if (md->avg_rtt > 0 && md->avg_rtt_var > 0
			&& md->avg_pacing_rate > 0 && b0 > 0) {
		// NOTE: Make sure MPC_ONE offsets anything scaled by it. psi, xi,
		// gamma, alpha, a[*], b[*].
		s64 cd = 2*b0*md->psi/MPC_ONE;
		s64 t1 = md->avg_rtt_var * md->xi
			/ (cd * b0 * md->avg_pacing_rate) * MPC_ONE;
		s64 t2 = md->avg_rtt / b0 * MPC_ONE;
		s64 t3 = md->avg_rtt_var / (cd * md->avg_rtt) * MPC_ONE;
		s64 t4 = md->predicted_rtt / b0 * MPC_ONE;
		s64 t5 = t1 + t2;
		s64 t6 = t3 + t4;

		if (t5 > t6)
			rate_opt = t5 - t6;

		md->dstats.probing = false;
	}

	// Clamp rate
	// TODO: Make bounds less arbitrary than 100 mbit/s. 1 << 32 is to limit
	// overflows.
	rate_opt = min_t(s64, max_t(s64, rate_opt,
			((s64) 100) << 17), ((s64) 1) << 32);

	// Now we set predicted RTT to include the control.
	*lookback_index(&md->lb_pacing_rate, 0) = rate_opt;
	md->predicted_rtt += b0 * rate_opt / MPC_ONE;

	md->avg_pacing_rate = wma(md->gamma, md->avg_pacing_rate, rate_opt);

	// debug
	md->dstats.rate_set = rate_opt;

	return rate_opt;
}


s64 control_predict(struct model *md)
{
	s64 predicted_rtt = 0;
	size_t i = 0;

	for (i = 0; i < md->p; i++)
		predicted_rtt += md->a[i] * (s64) *lookback_index(&md->lb_rtt, i) / MPC_ONE;

	for (i = 0; i < md->q; i++)
		predicted_rtt += md->b[i] * (s64) *lookback_index(&md->lb_pacing_rate, i) / MPC_ONE;

	return predicted_rtt;
}


void control_update(struct model *md, s64 rtt_meas)
{
	size_t i;
	s64 error;
	s64 total_norm = 0;

	error = rtt_meas - md->predicted_rtt;


	for (i = 0; i < md->p; i++) {
		s64 rtt = *lookback_index(&md->lb_rtt, i);
		total_norm += rtt*rtt;

		// Limit to prevent overflow.
		if (total_norm > S64_MAX - rtt*rtt)
			total_norm = S64_MAX;
		else
			total_norm += rtt*rtt;
	}

	for (i = 0; i < md->q; i++) {
		s64 rate = *lookback_index(&md->lb_pacing_rate, i);

		if (total_norm > S64_MAX - rate*rate)
			total_norm = S64_MAX;
		else
			total_norm += rate*rate;
	}

	total_norm = max_t(s64, 1, total_norm);


	if (total_norm == 0)
		goto exit;

	for (i = 0; i < md->p; i++) {
		// FIXME: This overflows.
		s64 rtt = *lookback_index(&md->lb_rtt, i);
		s64 delta = rtt * md->alpha * error / total_norm;

		md->a[i] += delta;
	}

	for (i = 0; i < md->q; i++) {
		s64 rate = *lookback_index(&md->lb_pacing_rate, i);
		s64 delta = rate * md->alpha * error / total_norm;

		md->b[i] += delta;
	}

exit:
	lookback_add(&md->lb_rtt, rtt_meas);
}

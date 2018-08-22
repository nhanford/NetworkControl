
#include <linux/limits.h>

#include "control.h"
#include "util.h"


inline s64 sqr(s64 x)
{
	return x*x;
}


static s64 control_predict(struct model *md);
static void control_update(struct model *md, s64 rtt_meas);


size_t control_rollover(struct model *md)
{
	return max_t(size_t, md->p, md->q);
}


s64 control_process(struct model *md, s64 rtt_meas, s64 rate_meas)
{
	s64 b0 = md->b[0];
	s64 rate_opt;

	// Convert to internal units.
	rate_meas >>= 20;

	// Now we set predicted RTT to include the control.
	*lookback_index(&md->lb_pacing_rate, 0) = rate_meas;
	md->predicted_rtt = control_predict(md);
	rate_opt = rate_meas;

	// debug.
	md->dstats.rtt_meas_us = rtt_meas;
	md->dstats.rtt_pred_us = md->predicted_rtt;


	control_update(md, rtt_meas);

	md->avg_rtt = max_t(s64, 1, wma(md->gamma, md->avg_rtt, rtt_meas));

	md->avg_rtt_var = max_t(s64, 1, wma(md->gamma, md->avg_rtt_var,
				sqr(md->predicted_rtt - md->avg_rtt)));

	md->avg_pacing_rate = wma(md->gamma, md->avg_pacing_rate, rate_meas);


	// Predict RTT assuming current control is 0. This is l^(n + 1)|(r = 0).
	lookback_add(&md->lb_pacing_rate, 0);
	md->predicted_rtt = control_predict(md);

	if (md->avg_rtt > 0 && md->avg_rtt_var > 0
			&& md->avg_pacing_rate > 0 && b0 > 0) {
		// NOTE: Make sure MPC_ONE offsets anything scaled by it. psi, xi,
		// gamma, alpha, a[*], b[*].
		s64 t1 = md->xi/md->avg_pacing_rate - b0/md->avg_rtt;
		s64 t2 = md->avg_rtt_var*sqr(MPC_ONE)/(2*md->psi*b0);

		rate_opt = (t1*t2/MPC_ONE + md->avg_rtt - md->predicted_rtt)
			* MPC_ONE/b0;
	}

	// Clamp rate
	rate_opt = min_t(s64, max_t(s64, rate_opt, MPC_MIN_RATE), MPC_MAX_RATE);

	// Convert for external units.
	rate_opt <<= 20;

	// debug
	md->dstats.rate_set = rate_opt;

	return rate_opt;
}


static s64 control_predict(struct model *md)
{
	s64 predicted_rtt = 0;
	size_t i = 0;

	for (i = 0; i < md->p; i++)
		predicted_rtt += md->a[i] * (*lookback_index(&md->lb_rtt, i)) / MPC_ONE;

	for (i = 0; i < md->q; i++)
		predicted_rtt += md->b[i] * (*lookback_index(&md->lb_pacing_rate, i)) / MPC_ONE;

	return max_t(s64, 0, min_t(s64, predicted_rtt, 4*md->avg_rtt));
}


static void control_update(struct model *md, s64 rtt_meas)
{
	size_t i;
	s64 error;
	s64 total_norm = 0;

	error = rtt_meas - md->predicted_rtt;


	for (i = 0; i < md->p; i++) {
		s64 rtt = *lookback_index(&md->lb_rtt, i);
		total_norm += rtt*rtt;
	}

	for (i = 0; i < md->q; i++) {
		s64 rate = *lookback_index(&md->lb_pacing_rate, i);
		total_norm += rate*rate;
	}

	total_norm = max_t(s64, 1, total_norm);


	if (total_norm == 0)
		goto exit;

	for (i = 0; i < md->p; i++) {
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

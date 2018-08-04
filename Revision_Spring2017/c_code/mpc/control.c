
#include <linux/limits.h>

#include "control.h"
#include "util.h"

// (x - y)^2
inline u64 square_diff_u64(u64 x, u64 y)
{
	u64 diff;

	if (x > y)
		diff = x - y;
	else
		diff = y - x;

	return diff * diff;
}


u64 control_predict(struct model *md);
void control_update(struct model *md, u64 rtt_meas);


u64 control_process(struct model *md, u64 rtt_meas, u64 rate)
{
	u64 b0 = md->b[0];
	u64 rate_opt = lookback_index(&md->lb_pacing_rate, 0);

	// debug.
	md->dstats.rtt_meas_us = rtt_meas;
	md->dstats.rtt_pred_us = md->predicted_rtt;

	control_update(md, rtt_meas);

	md->avg_rtt = max_t(u64, 1, wma(md->gamma, md->avg_rtt, rtt_meas));

	md->avg_rtt_var = max_t(u64, 1, wma(md->gamma, md->avg_rtt_var,
				square_diff_u64(md->predicted_rtt, md->avg_rtt)));

	md->predicted_rtt = control_predict(md);


	if (rate > 0) {
		rate_opt = rate;
		md->dstats.probing = true;
	} else if (md->avg_rtt > 0 && md->avg_rtt_var > 0
			&& md->avg_pacing_rate > 0) {
		rate_opt = U64_MAX;

		md->dstats.probing = false;
	}

	// Clamp rate
	// TODO: Make bounds less arbitrary than 100 mbit/s. 1 << 32 is to limit
	// overflows.
	rate_opt = min_t(u64, max_t(u64, rate_opt,
			((u64) 100) << 17), ((u64) 1) << 32);

	lookback_add(&md->lb_pacing_rate, rate_opt);
	md->predicted_rtt += rate_opt;

	md->avg_pacing_rate = wma(md->gamma, md->avg_pacing_rate, rate_opt);

	// debug
	md->dstats.rate_set = rate_opt;

	return rate_opt;
}


u64 control_predict(struct model *md)
{
	u64 predicted_rtt = 0;
	size_t i = 0;

	for (i = 0; i < md->p; i++) {
		u64 rtt = lookback_index(&md->lb_pacing_rate, i);
		predicted_rtt += md->a[i] * rtt;
	}

	for (i = 0; i < md->q; i++) {
		u64 rate = lookback_index(&md->lb_pacing_rate, i);
		predicted_rtt += md->b[i] * rate;
	}

	return predicted_rtt;
}


void control_update(struct model *md, u64 rtt_meas)
{
	int i;
	u64 error;
	u64 rtt_sum = 0;
	u64 rate_sum = 0;
	u64 sum;
	bool add;

	if (md->avg_rtt == 0 || md->avg_pacing_rate == 0)
		goto exit;


	if (md->predicted_rtt < rtt_meas) {
		error = rtt_meas - md->predicted_rtt;
		add = true;
	} else {
		error = md->predicted_rtt - rtt_meas;
		add = false;
	}


	for (i = 0; i < md->p; i++)
		rtt_sum += lookback_index(&md->lb_rtt, i);

	for (i = 0; i < md->q; i++)
		rate_sum += lookback_index(&md->lb_pacing_rate, i);

	if (rtt_sum == 0 || rate_sum == 0)
		goto exit;

	sum = rtt_sum + rate_sum;


	for (i = 0; i < md->p; i++) {
		u64 rtt = lookback_index(&md->lb_rtt, i);
		u64 delta = rtt * error / sum / rtt_sum;

		if (add)
			md->a[i] += delta;
		else if (md->a[i] > delta)
			md->a[i] -= delta;
		else
			md->a[i] = 0;
	}

	for (i = 0; i < md->q; i++) {
		u64 rate = lookback_index(&md->lb_pacing_rate, i);
		u64 delta = rate * error / sum / rate_sum;

		if (add)
			md->b[i] += delta;
		else if (md->b[i] > delta)
			md->b[i] -= delta;
		else
			md->b[i] = 0;
	}

exit:
	lookback_add(&md->lb_rtt, rtt_meas);
}

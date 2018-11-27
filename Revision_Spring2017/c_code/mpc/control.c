
#include <linux/limits.h>

#include "control.h"
#include "util.h"


static void control_update(struct model *md, scaled rate_meas, scaled rtt_meas);


size_t control_rollover(struct model *md)
{
	return md->timer;
}


scaled control_process(struct model *md, scaled time, scaled rate_meas,
	scaled rtt_meas)
{
	scaled target_rtt;
	scaled xhat;

	if (SLT(ZERO, md->rate_set) && SLT(ZERO, rtt_meas))
		// NOTE: We use rate_set because rate_mead is not accurate, and
		// rate_set can thus give a better prediction.
		control_update(md, md->rate_set, rtt_meas);

	md->timer--;
	if (md->timer <= 0) {
		md->rate_set = md->rb;

		if (!SLT(md->alpha, ZERO)) {
			md->alpha = scaled_negate(ONE);
			md->lp = rtt_meas;
			md->timer = md->dec_period;
		} else {
			md->alpha = md->alpha_init;
			md->lb = rtt_meas;
			md->timer = md->inc_period;
		}
	} else {
		target_rtt = SM(SA(ONE, md->alpha), md->lp);
		md->rate_set = SM(md->rb, SA(ONE, SM(SS(ONE, md->c),
				SS(target_rtt, rtt_meas))));
	}

	md->rate_set = scaled_min(scaled_max(MPC_MIN_RATE, md->rate_set), MPC_MAX_RATE);

	xhat = md->x0;
	if (SLT(ZERO, md->rb))
		xhat = SA(xhat, SD(SS(md->rate_set, md->rb), md->rb));
	xhat = scaled_min(scaled_max(ZERO, xhat), SS(md->lb, md->lp));
	md->stats.rtt_pred_us = STI(SA(md->lp, xhat));

	md->stats.rate_meas = STI(rate_meas);
	md->stats.rate_set = STI(md->rate_set);
	md->stats.rtt_meas_us = STI(rtt_meas);
	md->stats.lp = STI(md->lp);
	md->stats.rb = STI(md->rb);
	md->stats.x = STI(md->x0);

	return md->rate_set;
}


static void control_update(struct model *md, scaled rate_meas, scaled rtt_meas)
{
	scaled t1;
	scaled t2;

	md->lb = scaled_max(md->lb, rtt_meas);
	md->lp = scaled_min(md->lp, rtt_meas);

	md->x1 = md->x0;
	md->x0 = scaled_max(ZERO, SS(rtt_meas, md->lp));

	if (!SEQ(md->rb, ZERO)) {
		t1 = SA(SM(md->rb, SS(SS(md->x1, md->x0), ONE)), rate_meas);
		t2 = SD(SM(rate_meas, t1), SIP(md->rb, 3));
		md->rb = SA(md->rb, t2);
	}

	md->rb = scaled_max(MPC_MIN_RATE, md->rb);

	md->avg_rate = wma(md->weight, md->avg_rate, rate_meas);
	md->avg_rtt = wma(md->weight, md->avg_rtt, rtt_meas);
}

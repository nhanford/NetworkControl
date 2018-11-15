
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
	scaled xhat;
	scaled t1;
	scaled t2;
	scaled t3;

	if (SLT(ZERO, md->rate_set) && SLT(ZERO, rtt_meas))
		// NOTE: We use rate_set because rate_mead is not accurate, and
		// rate_set can thus give a better prediction.
		control_update(md, md->rate_set, rtt_meas);

	//md->timer--;
	if (md->timer <= 0) {
		scaled rb_save = md->rb;
		model_reset(md);
		md->rb = rb_save;
		md->rate_set = rb_save;
	} else {
		t1 = SM(SSQR(md->rb), SA(SM(md->c2, md->rate_set), md->c3));
		t2 = SM(SM(md->c1, md->rb), SS(ONE, SM(md->alpha, md->x0)));
		t3 = SA(SM(md->c2, SSQR(md->rb)), md->c1);

		if (!SEQ(t3, ZERO)) {
			md->rate_set = SD(SA(t1, t2), t3);
		} else {
			// Both c1 and rb must be 0 here. This means we are only
			// concerned about increasing the rate.
			md->rate_set = SA(md->rb, md->rate_diff);
		}
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

	t1 = SD(SS(rate_meas, md->rb), SSQR(rate_meas));
	t2 = SD(SA(SS(md->x0, rtt_meas), md->lp), rate_meas);
	md->rb = SA(md->rb, SA(t1, t2));
	md->rb = scaled_max(MPC_MIN_RATE, md->rb);

	md->avg_rate = wma(md->weight, md->avg_rate, rate_meas);
	md->avg_rtt = wma(md->weight, md->avg_rtt, rtt_meas);
}

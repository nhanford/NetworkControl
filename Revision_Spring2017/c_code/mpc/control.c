
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
		scaled rb_save = SM(SFI(7, -3), md->rb);
		model_reset(md);
		md->rb = rb_save;
		md->rate_set = rb_save;
	} else {
		t1 = SM(md->c1, SM(md->rb, SA(ONE, SS(SM(md->alpha, md->lp), md->x0))));
		t2 = SM(md->c2, md->rb);
		t3 = SA(md->c1, md->c2);

		if (!SEQ(t3, ZERO)) {
			md->rate_set = SD(SA(t1, t2), t3);
		} else {
			// Both c1 and c2 must be 0 here. This means we are only
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

	if (!SEQ(md->rb, ZERO)) {
		t1 = SA(SM(md->rb, SS(SS(md->x1, md->x0), ONE)), rate_meas);
		t2 = SD(SM(rate_meas, t1), SIP(md->rb, 3));
		md->rb = SA(md->rb, t2);
	}

	md->rb = scaled_max(MPC_MIN_RATE, md->rb);

	md->avg_rate = wma(md->weight, md->avg_rate, rate_meas);
	md->avg_rtt = wma(md->weight, md->avg_rtt, rtt_meas);
}

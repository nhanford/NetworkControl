
#include <linux/limits.h>

#include "control.h"
#include "util.h"


static void control_update(struct model *md, scaled rate_meas, scaled rtt_meas);


scaled control_process(struct model *md, scaled time, scaled rate_meas,
	scaled rtt_meas)
{
	scaled target_rtt;
	scaled xhat;
	scaled p, t1, t2, t3, t4;

	if (SLT(ZERO, md->rate_set) && SLT(ZERO, rtt_meas))
		// NOTE: We use rate_set because rate_mead is not accurate, and
		// rate_set can thus give a better prediction.
		control_update(md, md->rate_set, rtt_meas);

	md->timer--;
	if (md->dec_period > 0 && md->timer <= 0) {
		md->rate_set = md->rb;

		md->decreasing = !md->decreasing;
		if (md->decreasing) {
			md->lp = rtt_meas;
			md->timer = md->dec_period;
		} else {
			md->lb = rtt_meas;
			md->timer = md->inc_period;
		}
	}


	if (md->decreasing)
		target_rtt = ZERO;
	else
		target_rtt = SA(md->lp, md->over);

	p = SA(SS(SS(SM(md->weight, md->c1), md->c1), md->c2), ONE);
	t1 = SM(SM(p, md->rb), SA(ONE, SS(target_rtt, rtt_meas)));
	t2 = SM(SM(SM(md->weight, md->c1), md->rb), SS(md->avg_rtt, target_rtt));
	t3 = SM(SM(md->c2, SIP(md->lp, 2)), md->rate_set);
	t4 = SA(SM(md->c2, SIP(md->lp, 2)), p);

	if (!SEQ(t4, ZERO))
		md->rate_set = SD(SA(SA(t1, t2), t3), t4);

	md->rate_set = scaled_min(scaled_max(md->min_rate, md->rate_set), md->max_rate);


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
		md->rb = SA(md->rb, SM(md->learn_rate, t2));
	}

	md->rb = scaled_min(scaled_max(md->min_rate, md->rb), md->max_rate);

	md->avg_rtt = wma(md->weight, md->avg_rtt, rtt_meas);
}

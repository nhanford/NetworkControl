
#include <linux/limits.h>

#include "control.h"
#include "util.h"


static void control_update(struct model *md, scaled rate_meas, scaled loss_meas);


scaled control_process(struct model *md, scaled time, scaled rate_meas,
	scaled loss_meas)
{
	scaled target_loss;
	scaled xhat;
	scaled p, t1, t2, t3, t4;

	if (SLT(ZERO, md->rate_set) && SLT(ZERO, loss_meas))
		// NOTE: We use rate_set because rate_mead is not accurate, and
		// rate_set can thus give a better prediction.
		control_update(md, md->rate_set, loss_meas);


	target_loss = md->over;

	p = SA(SS(SM(md->weight, md->c1), md->c1), ONE);
	t1 = SM(SS(p, md->c2), md->rb);
	t2 = SM(md->c2, md->rate_set);
	t3 = SM(SS(SS(ONE, md->c1), md->c2), target_loss);
	t4 = SM(SM(md->weight, md->c1), md->avg_loss);

	if (!SEQ(t4, ZERO))
		md->rate_set = SD(SA(SA(SA(t1, t2), t3), t4), p);

	md->rate_set = scaled_min(scaled_max(MPC_MIN_RATE, md->rate_set), MPC_MAX_RATE);


	xhat = SS(md->rate_set, md->rb);
	md->stats.loss_pred = STI(xhat);

	md->stats.rate_meas = STI(rate_meas);
	md->stats.rate_set = STI(md->rate_set);
	md->stats.loss_meas = STI(loss_meas);
	md->stats.rb = STI(md->rb);

	return md->rate_set;
}


static void control_update(struct model *md, scaled rate_meas, scaled loss_meas)
{
	md->rb = wma(md->weight, md->rb, SS(rate_meas, loss_meas));
	md->rb = scaled_min(scaled_max(MPC_MIN_RATE, md->rb), MPC_MAX_RATE);
	md->avg_loss = wma(md->weight, md->avg_loss, loss_meas);
}

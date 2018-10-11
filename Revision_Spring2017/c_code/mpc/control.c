
#include <linux/limits.h>

#include "control.h"
#include "util.h"


inline s64 sqr(s64 x)
{
	return x*x;
}

s64 rdiv(s64 x, s64 y)
{
	s64 div = x/y;
	s64 rem = x%y;

	if (abs(rem) > 0) {
		if (x*y < 0)
			div -= 1;
		else
			div += 1;
	}


	return div;
}


static void control_update(struct model *md, s64 rate_meas, s64 rtt_meas);


size_t control_rollover(struct model *md)
{
	return md->timer;
}


s64 control_process(struct model *md, s64 time, s64 rate_meas, s64 rtt_meas)
{
	s64 xhat;
	s64 c1_adj;
	s64 t1;
	s64 t2;
	s64 t3;

	if (md->rate_set > 0 && rtt_meas > 0)
		// NOTE: We use rate_set because rate_mead is not accurate, and
		// rate_set can thus give a better prediction.
		control_update(md, md->rate_set, rtt_meas);

	if (md->avg_rtt != 0)
		c1_adj = md->c1*sqr(md->avg_rate)/md->avg_rtt;

	//md->timer--;
	if (md->timer <= 0) {
		s64 rb_save = md->rb;
		model_reset(md);
		md->rb = rb_save;
		md->rate_set = rb_save;
	} else {
		t1 = c1_adj*md->rb * (1 - md->alpha*md->x0/MPC_ONE);
		t2 = sqr(md->rb) * (md->c2*md->rate_set + md->c3/2);
		t3 = (md->c2*sqr(md->rb) + c1_adj);

		if (t3 != 0) {
			md->rate_set = (t1 + t2)/t3;
			printk(KERN_INFO "Rate set by alg: %lld, t1: %lld, t2: %lld, t3: %lld\n",
				md->rate_set, t1, t2, t3);
		} else {
			// Both c1 and rb must be 0 here. This means we are only
			// concerned about increasing the rate.
			md->rate_set = md->rb + md->rate_diff;
		}
	}

	printk(KERN_INFO "Rate set before adj: %lld\n",
		md->rate_set);

	md->rate_set = min_t(s64, max_t(s64, MPC_MIN_RATE, md->rate_set), MPC_MAX_RATE);

	xhat = md->x0;
	if (md->rb > 0)
		xhat += (md->rate_set - md->rb)/md->rb;
	xhat = min_t(s64, max_t(s64, 0, xhat), md->lb - md->lp);
	md->stats.rtt_pred_us = md->lp + xhat;

	md->stats.rate_meas = rate_meas << 20;
	md->stats.rate_set = md->rate_set << 20;
	md->stats.rtt_meas_us = rtt_meas;
	md->stats.lp = md->lp;
	md->stats.rb = md->rb << 20;
	md->stats.x = md->x0;

	return md->rate_set;
}


static void control_update(struct model *md, s64 rate_meas, s64 rtt_meas)
{
	s64 t1;
	s64 t2;

	md->lb = max_t(s64, md->lb, rtt_meas);
	md->lp = min_t(s64, md->lp, rtt_meas);

	md->x1 = md->x0;
	md->x0 = max_t(s64, 0, rtt_meas - md->lp);

	t1 = rdiv(rate_meas - md->rb, sqr(rate_meas));
	t2 = rdiv(md->x0 - rtt_meas + md->lp, rate_meas);
	md->rb += 2*(t1 + t2);
	md->rb = max_t(s64, MPC_MIN_RATE, md->rb);

	md->avg_rate = wma(md->weight, md->avg_rate, rate_meas);
	md->avg_rtt = wma(md->weight, md->avg_rtt, rtt_meas);
}

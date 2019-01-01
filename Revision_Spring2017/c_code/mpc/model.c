
#include <linux/slab.h>

#include "model.h"


int model_init(struct model *md, scaled weight, scaled learn_rate, scaled over,
	scaled c1, scaled c2)
{
	md->weight = weight;
	md->learn_rate = learn_rate;

	md->over = over;
	md->c1 = c1;
	md->c2 = c2;

	model_reset(md);

	return 0;
}

void model_release(struct model *md)
{
}

void model_reset(struct model *md)
{
	md->rate_set = ZERO;
	md->avg_loss = ZERO;
	md->rb = ZERO;
}


#include "dfs.h"
#include "scaled.h"
#include "util.h"

#ifndef MODEL_H
#define MODEL_H

struct model {
	// Rates are in B/s.
	scaled weight;
	scaled learn_rate;
	scaled rate_set;

	// What should the loss rate be?
	scaled over;
	// Loss rate variance and control action relative weights.
	scaled c1;
	scaled c2;

	scaled avg_loss;
	scaled rb;

	struct mpc_dfs_stats stats;
};

int model_init(struct model *md, scaled weight, scaled learn_rate, scaled over,
	scaled c1, scaled c2);

void model_release(struct model *md);

void model_reset(struct model *md);

#endif /* end of include guard: MODEL_H */

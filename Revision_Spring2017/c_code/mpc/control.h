
#include "model.h"

#ifndef CONTROL_H
#define CONTROL_H

// Update model and return the next pacing rate.
//
// If rate_gain is not 0, then forgo the normal MPC model and simply increment
// the rate by that amount. This is useful for implementing probing.
float control_process(struct model *md, float rtt_meas, float rate_gain);

#endif /* end of include guard: CONTROL_H */

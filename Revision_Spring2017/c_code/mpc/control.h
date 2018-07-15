
#include "model.h"

#ifndef CONTROL_H
#define CONTROL_H

// Update model and return the next pacing rate.
real control_process(struct model *md, real rtt_meas);

// Increase the rate by a certain amount, updates model.
real control_gain(struct model *md, real rtt_meas, real rate_gain);

#endif /* end of include guard: CONTROL_H */


#include "model.h"

#ifndef CONTROL_H
#define CONTROL_H

// Update model and return the next pacing rate.
u32 control_process(struct model *md, u32 rtt_meas);

// Increase the rate by a certain amount, updates model.
u32 control_gain(struct model *md, u32 rtt_meas, u32 rate_gain);

#endif /* end of include guard: CONTROL_H */

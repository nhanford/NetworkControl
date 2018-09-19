
#include "model.h"

#ifndef CONTROL_H
#define CONTROL_H

// How many times can control_process be called before the model rolls over all
// internal values.
size_t control_rollover(struct model *md);

// Update model and return the next pacing rate.
//
// @param time The current time in use.
// @param rate_meas The measured rate in bytes/s
// @param rtt_meas RTT measured in us.
s64 control_process(struct model *md, s64 time, s64 rate_meas, s64 rtt_meas);

#endif /* end of include guard: CONTROL_H */


#include "model.h"

#ifndef CONTROL_H
#define CONTROL_H

// How many times can control_process be called before the model rolls over all
// internal values.
size_t control_rollover(struct model *md);

// Update model and return the next pacing rate.
//
// @param time The current time in us.
// @param rate_meas The measured rate in bytes/s
// @param rtt_meas RTT measured in us.
scaled control_process(struct model *md, scaled time, scaled rate_meas,
        scaled rtt_meas);

#endif /* end of include guard: CONTROL_H */

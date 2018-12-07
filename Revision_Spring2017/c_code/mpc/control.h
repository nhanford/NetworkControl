
#include "model.h"

#ifndef CONTROL_H
#define CONTROL_H

// Update model and return the next pacing rate.
//
// @param time The current time in us.
// @param rate_meas The measured rate in bytes/s
// @param loss_meas Loss rate measured in bytes/s.
scaled control_process(struct model *md, scaled time, scaled rate_meas,
        scaled loss_meas);

#endif /* end of include guard: CONTROL_H */

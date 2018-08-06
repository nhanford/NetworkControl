
#include "model.h"

#ifndef CONTROL_H
#define CONTROL_H

// Update model and return the next pacing rate.
//
// If rate is not 0, then forgo the normal MPC model and simply set
// the rate to that amount. This is useful for implementing probing.
//
// @param rtt_meas RTT measured in us.
// @param rate The pacing rate to set measured in bytes/s.
s64 control_process(struct model *md, s64 rtt_meas, s64 rate);

#endif /* end of include guard: CONTROL_H */

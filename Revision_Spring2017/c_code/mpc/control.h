
#include "model.h"

#ifndef CONTROL_H
#define CONTROL_H

// Update model and return the next pacing rate.
//
// If rate_gain is not 0, then forgo the normal MPC model and simply increment
// the rate by that amount. This is useful for implementing probing.
//
// @param rtt_meas RTT measured in us.
// @param rate_gain The growth in pacing rate measured in bytes/s.
real_int control_process(struct model *md, real_int rtt_meas_us, real_int rate_gain_bs);

#endif /* end of include guard: CONTROL_H */


#include "model.h"

#ifndef CONTROL_H
#define CONTROL_H

// Update model and return the next pacing rate.
//
// If probe is true, then probing mode is initiated.
//
// @param rtt_meas RTT measured in us.
// @param probe Indicates probing mode should be initiated.
s64 control_process(struct model *md, s64 rtt_meas, bool probe);

#endif /* end of include guard: CONTROL_H */

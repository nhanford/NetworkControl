
#include "model.h"

#ifndef CONTROL_H
#define CONTROL_H

// Update model and return the next pacing rate.
u32 control_process(struct model *md, u32 rtt_meas);

#endif /* end of include guard: CONTROL_H */

################################################################################
#
# Author: David Fridovich-Keil ( dfk@eecs.berkeley.edu )
# File: latency_generator.py
#
# Class to generate a fake latency time series, given a sequence of control
# inputs (pacing rate 'r').
#
################################################################################

import numpy as np

class LatencyGenerator:

    def __init__(self, l_max, l_slope, l_cut, r_coeff, noise_sd = 0.1):
        """ Constructor."""

        self.l_max_ = l_max # Maximum value of latency before packet drop.
        self.l_slope_ = l_slope # Increment size for linear latency growth.
        self.l_cut_ = l_cut # Cuts back to this fraction of maximum after drop.
        self.r_coeff_ = r_coeff # Coefficient of most recent control input.
        self.noise_sd_ = noise_sd # Standard deviation of additive noise.

        # Keep track of most recent latency value.
        self.l_ = 0.0

    def Generate(self, r=0.0):
        self.l_ += (self.l_slope_ + self.r_coeff_ * r +
                    self.noise_sd_ * np.random.randn())
        if self.l_ > self.l_max_:
            self.l_ *= self.l_cut_

        return self.l_

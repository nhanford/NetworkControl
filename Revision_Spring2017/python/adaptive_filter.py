################################################################################
#
# Author: David Fridovich-Keil ( dfk@eecs.berkeley.edu )
# File: adaptive_filter.py
#
# Adaptive filter implementation.
#
################################################################################

import numpy as np
from collections import deque

class AdaptiveFilter:

    def __init__(self, p, q, alpha, beta):
        """ Constructor."""

        self.p_ = p # Number of autoregressive coefficients (on past latencies).
        self.q_ = q # Number of moving average coefficients (on past controls).
        self.alpha_ = alpha # Learning rate for AR coefficients.
        self.beta_ = beta # Learning rate for MA coefficients.

        # Initialize coefficients with all zeros.
        self.a_ = np.ones(p)
        self.b_ = np.ones(q)

        # History stored as a deque.
        self.l_ = deque(maxlen=p)
        self.r_ = deque(maxlen=q)

    def Predict(self, last_r=0.0, restore=False):
        """
        Pass in the next control input 'r' and predict next latency 'l'.
        If 'restore' flag is set, then does not change state. Otherwise,
        remembers thsi control input 'last_r' and potentially forgets oldest
        control.
        """
        l_hat = 0.0

        # Add control to the deque. Most recent values are on the left.
        if len(self.r_) > 0:
            oldest_r = self.r_[-1]
        self.r_.appendleft(last_r)

        # Convolve 'a' with latency history.
        for ii, l in enumerate(self.l_):
            l_hat += l * self.a_[ii]

        # Convolve 'b' with control history.
        for ii, r in enumerate(self.r_):
            l_hat += r * self.b_[ii]

        # Maybe restore to original state.
        if (restore):
            self.r_.popleft()
            if len(self.r_) == self.q_:
                self.r_.append(oldest_r)

        return l_hat

    def Update(self, l_meas, l_hat):
        """ Update 'a' and 'b' given a measurement and prediction. """
        error = l_hat - l_meas

        # Precompute the L2 norm of both latency and control histories.
        # NOTE: If this is a bottleneck, it can be sped up significantly.
        l_norm = 0.0
        for l in self.l_:
            l_norm += l * l

        r_norm = 0.0
        for r in self.r_:
            r_norm += r * r

        total_norm = l_norm + r_norm

        # Gradient descent on 'a'
        if total_norm > 1e-16:
            for ii, l in enumerate(self.l_):
                self.a_[ii] -= self.alpha_ * error * l / total_norm

        # Gradient descent on 'a'.
        if total_norm > 1e-16:
            for ii, r in enumerate(self.r_):
                self.b_[ii] -= self.beta_ * error * r / total_norm

        # Add new measurement to the history.
        self.l_.appendleft(l_meas)

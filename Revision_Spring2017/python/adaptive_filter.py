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

        # Initialize coefficients randomly.
        self.a_ = np.random.randn(p)
        self.b_ = np.random.randn(q)

        # History stored as a deque.
        self.l_ = deque(maxlen=p)
        self.r_ = deque(maxlen=q)

    def Predict(self, last_r=0.0):
        """ Pass in the next control input 'r' and predict next latency 'l'. """
        l_hat = 0.0

        # Add control to the deque. Most recent values are on the left.
        self.r_.appendleft(last_r)

        # Convolve 'a' with latency history.
        for ii, l in enumerate(self.l_):
            l_hat += l * self.a_[ii]

        # Convolve 'b' with control history.
        for ii, r in enumerate(self.r_):
            l_hat += r * self.b_[ii]

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

        # Gradient descent on 'a'.
        for ii, l in enumerate(self.l_):
            self.a_[ii] -= self.alpha_ * error * l / l_norm

        # Gradient descent on 'a'.
        for ii, r in enumerate(self.r_):
            self.b_[ii] -= self.beta_ * error * r / r_norm

        # Add new measurement to the history.
        self.l_.appendleft(l_meas)

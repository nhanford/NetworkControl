################################################################################
#
# Author: David Fridovich-Keil ( dfk@eecs.berkeley.edu )
# File: controller.py
#
# Controller implementation. The controller holds an adaptive filter inside
# which constantly updates its internal predictive model as new data arrives.
#
################################################################################

from adaptive_filter import AdaptiveFilter

import numpy as np

class Controller:

    def __init__(self, psi, xi, gamma, p, q, alpha, beta):
        """ Constructor. """

        self.psi_ = psi # Coefficient of variance term.
        self.xi_ = xi # Coefficient of throughput term.
        self.gamma_ = gamma # Parameter in nominal latency estimator.

        # Estimated nominal latency.
        self.mu_ = 0.0

        # Keep an adaptive filter as the dynamics model.
        self.model_ = AdaptiveFilter(p, q, alpha, beta)

        # Most recent prediction.
        self.l_hat_ = self.model_.Predict()

    def Process(self, l, r=None):
        """
        Process a new latency/control pair. Two steps:
        (1) updates internal model, and
        (2) computes optimal control.
        """

        # (1) Update internal model.
        self.model_.Update(l, self.l_hat_)

        # (2) Compute optimal control.
        self.mu_ = (1.0 - self.gamma_) * l + self.gamma_ * self.mu_
        self.l_hat_ = self.model_.Predict(0.0, True)

    if abs(self.psi_) < 1e-16: 
        if self.model_.b_[0] - self.xi_ < 0.0:
            r_opt = 1000.0
        else:
            r_opt = -1000.0
    else:
        r_opt = (self.mu_ - self.l_hat_ +
                     ((self.xi_ - self.model_.b_[0]) /
                               (self.psi_ * self.model_.b_[0]))) / self.model_.b_[0]

    r_opt = max(0.1, min(r_opt, 34.35))

        if r is not None:
            self.model_.r_.appendleft(r)
            self.l_hat_ += self.model_.b_[0] * r
        else:
            self.model_.r_.appendleft(r_opt)
            self.l_hat_ += self.model_.b_[0] * r_opt

        return r_opt

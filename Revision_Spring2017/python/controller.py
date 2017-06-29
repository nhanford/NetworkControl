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

    def __init__(self, psi, xi, gamma, p, q, alpha):
        """ Constructor. """

        self.psi_ = psi # Coefficient of variance term.
        self.xi_ = xi # Coefficient of throughput term.
        self.gamma_ = gamma # Parameter in nominal latency estimator.

        # Estimated nominal latency.
        self.mu_latency_ = 0.0
        self.mu_variance_ = 0.0
        self.mu_control_ = 1.0

        # Keep an adaptive filter as the dynamics model.
        self.model_ = AdaptiveFilter(p, q, alpha)

        # Most recent prediction.
        self.l_hat_ = self.model_.Predict()

    def Process(self, l, r = None):
        """
        Process a new latency/control pair. Two steps:
        (1) updates internal model, and
        (2) computes optimal control.
        """

        # (1) Update internal model.
        self.model_.Update(l, self.l_hat_)

        self.mu_latency_ = (1.0 - self.gamma_) * l + self.gamma_ * self.mu_latency_
        self.mu_variance_ = ((1.0 - self.gamma_) * (self.l_hat_ - self.mu_latency_)**2 +
                             self.gamma_ * self.mu_variance_)

        # (2) Compute optimal control.
        self.l_hat_ = self.model_.Predict(0.0, True)

        # Normalize coefficients before solving for optimal control.
        psi = self.psi_ / self.mu_variance_
        xi = self.xi_ / self.mu_control_
        b0 = self.model_.b_[0]

        r_opt = ((xi - b0 / self.mu_latency_) * (0.5 / (psi * b0)) +
                 self.mu_latency_ - self.l_hat_) / b0

        """
        (self.mu_ - self.l_hat_ +
                 ((self.xi_ - self.model_.b_[0]) /
                  (self.psi_ * self.model_.b_[0]))) / self.model_.b_[0]
        """

        # Threshold the output.
        # TODO! Make these bounds less arbitrary.
        r_opt = max(0.1, min(r_opt, 34.35))

        if r is not None:
            self.model_.r_.appendleft(r)
            self.l_hat_ += self.model_.b_[0] * r
        else:
            self.model_.r_.appendleft(r_opt)
            self.l_hat_ += self.model_.b_[0] * r_opt

        # Update control mean.
        if r is not None:
            self.mu_control_ = (1.0 - self.gamma_) * r + self.gamma_ * self.mu_control_
        else:
            self.mu_control_ = (1.0 - self.gamma_) * r_opt + self.gamma_ * self.mu_control_

        # Return both optimal control and predicted latency.
        return (r_opt, self.l_hat_)

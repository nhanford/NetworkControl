################################################################################
#
# Author: Taran Lynn ( tflynn@ucdavis.edu )
#
# A dynamic matrix control approach for networking.
#
################################################################################

import numpy as np
import random

class Controller:

    def __init__(self, weight, lookback, lambda_, kappa):
        self.weight = weight
        self.lookback = lookback

        self.g = np.repeat(0.0, lookback)
        self.rttLB = np.repeat(0.0, lookback + 1)
        self.y0 = 0.0

        self.lhat = 0.0

        self.lambda_ = lambda_
        self.kappa = kappa

        self.lAvg = 0.1
        self.rAvg = 0.1

    def process(self, l, r = None):
        self.update(l)
        r1 = self.rttLB[0]

        self.newRTT(0)
        lhat0 = self.predict()

        if r is None:
            lambda_ = self.lambda_ * self.lAvg**2 / self.rAvg**2
            kappa = self.kappa * self.lAvg**2 / self.rAvg
            g0 = self.g[0]

            if g0 == 0:
                g0 = 1

            r = r1 + (kappa - 2*g0*lhat0)/(2*lambda_ + 2*g0**2)

        r = min(max(0, r), 30)
        self.rttLB[0] = r
        self.lhat = self.predict()

        self.rAvg = self.wAvg(self.rAvg, r)

        return (r, self.lhat)

    def predict(self):
        diffs = self.rttLB[:-1] - self.rttLB[1:]
        lhat = self.y0 + sum(self.g * diffs)
        return max(0, min(lhat, 4*self.lAvg))

    def update(self, l):
        diff = self.rttLB[0] - self.rttLB[1]
        self.y0 += self.g[-1] * (self.rttLB[-2] - self.rttLB[-1])

        if diff == 0:
            self.newG(1)
        else:
            self.newG((l - self.lhat)/diff)

        self.lAvg = self.wAvg(self.lAvg, l)


    def wAvg(self, avg, x):
        return (1 - self.weight)*avg + self.weight*x

    def newG(self, g):
        self.g = np.roll(self.g, 1)
        self.g[0] = g

    def newRTT(self, rtt):
        self.rttLB = np.roll(self.rttLB, 1)
        self.rttLB[0] = rtt

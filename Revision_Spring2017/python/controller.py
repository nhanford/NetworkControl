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
    def __init__(self, maxDiff, weight):
        self.maxDiff = maxDiff
        self.k1 = 1.0
        self.k2 = 1.0
        self.weight = weight

        self.rttLast = 0.0
        self.rateLast = 0.0
        self.rateLast2 = 0.0
        self.a = 0.0

        self.avgRTT = 0.0
        self.avgRate = 0.0
        self.predRTT = 0.0

    def process(self, rtt):
        diff = self.rateLast - self.rateLast2

        if diff != 0:
            self.a = (rtt - self.rttLast)/diff

        if self.a != 0 and self.rateLast > 0:
            k1 = self.k1
            k2 = self.k2
            a = self.a
            ml = self.avgRTT
            mr = self.avgRate

            rate = self.rateLast + (k2*ml*ml/2 - a*rtt*mr*mr)/(k1*ml*ml + a*a*mr*mr)
        else:
            rate = self.rateLast + 1

        rate = min(max(0, rate), 30)

        self.rttLast = rtt
        self.rateLast2 = self.rateLast
        self.rateLast = rate
        self.predRTT = self.a*(self.rateLast - self.rateLast2) + rtt

        self.avgRTT = (1 - self.weight)*self.avgRTT + self.weight*rtt
        self.avgRate = (1 - self.weight)*self.avgRate + self.weight*rate

        self.k1 = self.avgRate/self.maxDiff

        return (rate, self.predRTT)

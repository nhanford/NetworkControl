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
    def __init__(self, c1, c2, weight):
        self.c1 = c1
        self.c2 = c2
        self.weight = weight

        self.rttLast = 0.0
        self.rateLast = 0.0
        self.rateLast2 = 0.0
        self.a = 0.0

        self.avgRTT = 0.0
        self.avgRate = 0.0
        self.avgRateDelta = 0.0
        self.predRTT = 0.0

    def process(self, rtt):
        diff = self.rateLast - self.rateLast2
        rate = self.rateLast

        self.avgRTT = (1 - self.weight)*self.avgRTT + self.weight*rtt
        self.avgRate = (1 - self.weight)*self.avgRate + self.weight*self.rateLast
        self.avgRateDelta = (1 - self.weight)*self.avgRateDelta + self.weight*diff

        if diff != 0:
            self.a = (rtt - self.rttLast)/diff

        if self.avgRate != 0 and self.avgRateDelta != 0:
            k1 = self.c1*self.avgRTT/self.avgRateDelta**2
            k2 = self.c2*self.avgRTT/self.avgRate
        else:
            k1 = 0
            k2 = 0

        if k1 != 0 or self.a != 0:
            rate += (k2 - 2*self.a*self.rttLast)/(2*k1 + 2*self.a)
        else:
            rate += 1

        rate = min(max(0, rate), 30)

        self.rttLast = rtt
        self.rateLast2 = self.rateLast
        self.rateLast = rate
        self.predRTT = self.a*(self.rateLast - self.rateLast2) + rtt

        return (rate, self.predRTT)

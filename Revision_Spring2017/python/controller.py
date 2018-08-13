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

    def __init__(self, k1, k2, weight):
        self.k1 = k1
        self.k2 = k2
        self.weight = weight

        self.rttLast = 0.0
        self.rateLast = 0.0
        self.rateLast2 = 0.0
        self.a = 0.0

        self.avgRTT = 0.0
        self.avgRate = 0.0

    def process(self, rtt):
        rate = 1

        if self.rateLast - self.rateLast2 != 0:
            self.a = (rtt - self.rttLast)/(self.rateLast - self.rateLast2)

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

        self.avgRTT = (1 - self.weight)*self.avgRTT + self.weight*rtt
        self.avgRate = (1 - self.weight)*self.avgRate + self.weight*rate

        return (rate, self.a*(self.rateLast - self.rateLast2) + rtt)

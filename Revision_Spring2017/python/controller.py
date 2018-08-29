################################################################################
#
# Author: Taran Lynn ( tflynn@ucdavis.edu )
#
# A dynamic matrix control approach for networking.
#
################################################################################

from collections import deque
import numpy as np
import random

class Controller:
    def __init__(self, numObs, k1, k2, weight):
        self.k1 = k1
        self.k2 = k2
        self.weight = weight

        self.rtt = deque([0], numObs)
        self.rate = deque([0], numObs)

        self.rttAvg = 0
        self.rateAvg = 0

    def wma(self, avg, x):
        return (1 - self.weight)*avg + self.weight*x

    def process(self, rtt):
        m = []
        b = []

        for i in range(1, len(self.rtt) + 1):
            m.append([self.rate[-i]**2, self.rate[-i], 1])
            b.append(self.rtt[-i])

        m = np.matrix(m)
        b = np.array(b)

        try:
            res = np.linalg.lstsq(m, b)
            ((a, b, c), _, _, _) = res
        except np.linalg.LinAlgError:
            (a, b, c) = (0, 0, self.rtt[-1])

        t1 = self.k2 * self.rttAvg - self.k1 * b * self.rateAvg
        t2 = 2 * self.k1 * a * self.rateAvg

        if (self.rttAvg == 0 or self.rateAvg == 2 or t2 == 0
                or a <= 0):
            rate = self.rate[-1] + 0.1
        else:
            rate = t1/t2

        rate = self.wma(self.rate[-1], rate) + random.uniform(-0.1, 0.1)
        rate = min(max(0, rate), 30)

        self.rtt.append(rtt)
        self.rate.append(rate)

        self.rttAvg = self.wma(self.rttAvg, rtt)
        self.rateAvg = self.wma(self.rateAvg, rate)

        return (rate, a*rate**2 + b*rate + c, a, b, c)

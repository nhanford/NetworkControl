################################################################################
#
# Author: Taran Lynn ( tflynn@ucdavis.edu ), based on work by
#          David Fridovich-Keil ( dfk@eecs.berkeley.edu )
# File: controller.py
#
################################################################################

from collections import deque
from math import sqrt
import numpy as np
import random


def wma(a, x, y):
    return (1 - a)*x + a*y


class Controller:
    def __init__(self, gamma, alpha, p, q):
        self.gamma = gamma
        self.alpha = alpha

        self.rand = random.Random()

        # Estimated nominal latency.
        self.avg_rtt = 0.0
        self.avg_rtt_var = 0.0
        self.avg_rate = 0.0

        self.p = p
        self.q = q

        self.a = p*[0]
        self.b = q*[0]

        self.lb_rtt = deque(p*[0], p)
        self.lb_rate = deque(q*[0], q)

        self.predicted_rtt = self.predict()


    def process(self, rtt_meas, rate = 0):
        rate_opt = self.lb_rate[0]

        self.update(rtt_meas)
        b0 = self.b[0]

        self.avg_rtt = max(1, wma(self.gamma, self.avg_rtt, rtt_meas))

        self.avg_rtt_var = max(1, wma(self.gamma, self.avg_rtt_var,
            (self.predicted_rtt - self.avg_rtt)**2))


        self.lb_rate.appendleft(0)

        if rate > 0:
            rate_opt = rate
        else:
            t1 = (1 - self.alpha)/self.alpha
            t2 = b0 / 2
            t3 = self.avg_rate / self.avg_rtt

            rate_opt = t1*t2*t3
            rate_opt *= rate_opt

        rate_opt = max(0.1, rate_opt)

        # Now we set predicted RTT to include the control.
        self.lb_rate[0] = rate_opt
        self.predicted_rtt = self.predict()

        self.avg_rate = wma(self.gamma, self.avg_rate, rate_opt)

        return (rate_opt, self.predicted_rtt)


    def predict(self):
        predicted_rtt = 0

        for i in range(0, self.p):
            rttRt = sqrt(self.lb_rtt[i])
            predicted_rtt += self.a[i] * rttRt

        for i in range(0, self.q):
            rateRt = sqrt(self.lb_rate[i])
            predicted_rtt += self.b[i] * rateRt

        return max(0, predicted_rtt)


    def update(self, rtt_meas):
        sum_lb = sum(self.lb_rtt) + sum(self.lb_rate)
        error = rtt_meas - self.predicted_rtt

        if sum_lb == 0:
            return

        for i in range(0, self.p):
            rttRt = sqrt(self.lb_rtt[i])
            delta = rttRt * error / sum_lb
            self.a[i] += delta
            print("a[{}] = {}".format(i, self.a[i]))

        for i in range(0, self.q):
            rateRt = sqrt(self.lb_rate[i])
            delta = rateRt * error / sum_lb
            self.b[i] += delta
            print("b[{}] = {}".format(i, self.b[i]))

        self.lb_rtt.appendleft(rtt_meas)

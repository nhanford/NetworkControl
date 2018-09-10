
from collections import deque
import numpy as np

INF = 1000

class Controller:

    def __init__(self, rateDiff, percRTT, percMax, weight, period, numObs):
        self.rateDiff = rateDiff
        self.percRTT = percRTT
        self.percMax = percMax
        self.weight = weight
        self.period = period
        self.numObs = numObs

        self.rB = 0
        self.reset()

    def reset(self):
        self.timer = self.period
        self.startProbe = self.numObs

        self.rate = deque([0]*self.numObs, self.numObs)
        self.rtt = deque([0]*self.numObs, self.numObs)
        self.x = deque([0]*self.numObs, self.numObs)

        self.lhat = 0

        self.avgRB = 0
        self.varRB = 0

        self.a = 0
        self.rB = self.rB/2
        self.lB = 0
        self.lP = INF

    def process(self, r, l):
        self.update(r, l)

        self.timer -= 1

        if self.startProbe > 0 or self.varRB > (self.rateDiff/2)**2:
            self.startProbe -= 1
            opt = self.rB + self.rateDiff
        elif np.mean(self.x) > self.percRTT * self.rtt[0] or self.timer <= 0:
            self.reset()
            opt = self.rB + self.rateDiff
        else:
            opt = self.percMax * self.rB
        opt = min(max(0.1, opt), 30)

        xhat = self.x[0] + (r - self.rB)
        xhat = min(max(0, xhat), self.lB - self.lP)
        self.lhat = self.a/opt + self.lP + xhat

        return (opt, self.lhat)

    def update(self, r, l):
        self.rate.appendleft(r)
        self.rtt.appendleft(l)

        A = np.column_stack([self.rate, np.ones(len(self.rate))])
        b = self.rtt

        if len(A) > 0:
            (aNew, _) = np.linalg.lstsq(A, b)[0]
            if aNew > 0 and aNew < INF:
                self.a = aNew

            if r > 0:
                self.x.appendleft(max(0, l - self.a/r - self.lP))

        self.lB = max(self.lB, l)
        self.lP = min(self.lP, l)

        # Estimate rB
        self.rB = max(self.rB, (self.x[1] + r)/(self.x[0] + 1))

        self.avgRB = self.wma(self.avgRB, self.rB)
        self.varRB = self.wma(self.varRB, (self.rB - self.avgRB)**2)

    def wma(self, avg, x):
        return (1 - self.weight)*avg + self.weight*x

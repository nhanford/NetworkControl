
from collections import deque
import numpy as np

INF = 1000

class Controller:
    def __init__(self, lr, over, c1, c2, weight, incPeriod, decPeriod):
        self.lr = lr
        self.over = over
        self.c1 = c1
        self.c2 = c2
        self.weight = weight
        self.incPeriod = incPeriod
        self.decPeriod = decPeriod

        self.reset()

    def reset(self):
        self.decreasing = False
        self.timer = self.incPeriod

        self.r = 0
        self.lhat = 0
        self.x = 0
        self.x1 = 0
        self.avgRTT = 0

        self.rB = 0
        self.lB = 0
        self.lP = INF

    def process(self, r, l):
        l = min(l, 100)

        if self.r > 0:
            self.update(self.r, l)

        self.timer -= 1

        if self.decPeriod > 0 and self.timer <= 0:
            self.decreasing = not self.decreasing

            self.lP = l
            self.lB = 0

            if self.decreasing:
                self.timer = self.decPeriod
            else:
                self.timer = self.incPeriod

        if self.decreasing:
            lt = 0
        else:
            lt = self.lP + self.over

        p = self.weight*self.c1 - self.c1 - self.c2 + 1
        t1 = p*self.rB*(1 + lt - l)
        t2 = self.weight*self.c1*self.rB*(self.avgRTT - lt)
        t3 = self.c2 * self.lP**2 * self.r
        t4 = self.c2*self.lP**2 + p
        opt = (t1 + t2 + t3)/t4

        opt = min(max(0.1, opt), 30)
        self.r = opt

        xhat = self.x + (r - self.rB)
        xhat = min(max(0, xhat), self.lB - self.lP)
        self.lhat = self.lP + xhat

        return (opt, self.lhat)

    def update(self, r, l):
        self.lB = max(self.lB, l)
        self.lP = min(self.lP, l)

        self.x1 = self.x
        self.x = max(0, l - self.lP)

        # Estimate rB
        if self.rB > 0:
            t1 = self.rB*(self.x1 - self.x - 1) + r
            t = r*t1/(self.rB**3)
        else:
            t = 1.0
        self.rB = min(max(1, self.rB + self.lr*t), 30)

        self.avgRTT = self.wma(self.avgRTT, l)

    def wma(self, avg, x):
        return (1 - self.weight)*avg + self.weight*x

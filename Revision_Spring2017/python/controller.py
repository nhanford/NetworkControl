
from collections import deque
import numpy as np

INF = 1000

class Controller:
    def __init__(self, lr, alpha, c, weight, incPeriod, decPeriod):
        self.lr = lr
        self.alphaInit = alpha
        self.c = c
        self.weight = weight
        self.incPeriod = incPeriod
        self.decPeriod = decPeriod

        self.reset()

    def reset(self):
        self.alpha = self.alphaInit
        self.timer = self.incPeriod

        self.r = 0
        self.lhat = 0
        self.x = 0
        self.x1 = 0

        self.rB = 0
        self.lB = 0
        self.lP = INF

    def process(self, r, l):
        l = min(l, 100)

        if self.r > 0:
            self.update(self.r, l)

        self.timer -= 1

        if self.decPeriod > 0 and self.timer <= 0:
            opt = self.rB

            if self.alpha >= 0:
                self.alpha = -1#self.alphaInit
                self.lP = l
                self.timer = self.decPeriod
            else:
                self.alpha = self.alphaInit
                self.lB = l
                self.timer = self.incPeriod
        else:
            lt = (1 + self.alpha) * self.lP
            opt = self.rB * (1 + (1 - self.c) * (lt - l))

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

    def wma(self, avg, x):
        return (1 - self.weight)*avg + self.weight*x

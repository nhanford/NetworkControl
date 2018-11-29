
from collections import deque
import numpy as np

INF = 1000

class Controller:
    def __init__(self, lr, lossRate, weight, c):
        self.lr = lr
        self.lossRate = lossRate
        self.weight = weight
        self.c = c

        self.reset()

    def reset(self):
        self.r = 0
        self.lhat = 0

        self.avgLoss = 0

        self.rB = 0

    def process(self, r, l):
        l = min(l, 100)

        if self.r > 0:
            self.update(self.r, l)

        opt = self.rB + self.c*self.avgLoss + (1 - self.c) * self.lossRate
        opt = min(max(0.1, opt), 30)
        self.r = opt

        self.lhat = self.r - self.rB

        self.avgLoss = self.wma(self.avgLoss, l)

        return (opt, self.lhat)

    def update(self, r, l):
        # Estimate rB
        self.rB = (1 - self.lr)*self.rB + self.lr*(r - l)
        self.rB = min(max(0, self.rB), 30)

    def wma(self, avg, x):
        return (1 - self.weight)*avg + self.weight*x

#!/usr/bin/env python
'''
This test the model in a mathematical fashion. It is a revision of
test_controller.py to work with the current controller and offer more modes of
responses.

@author: Taran Lynn
@contact: tflynn@ucdavis.edu

'''

from controller import Controller

import math
import numpy as np
import matplotlib
import matplotlib.pyplot as plt
import random

LR = 0.1
LOSSRATE = 5
WEIGHT = 0.1
C1 = 1/3
C2 = 1/3

NUM_DATA_POINTS = 5000


class Tester:
    def __init__(self):
        self.plotCount_ = 0

    def test(self, response, desc):
        """
        Tests the controller out on a given response pattern.

        @arg response A model that takes a rate and determine the connection RTT.
        This model should have a method of the form generate(rate).
        """
        rateler = Controller(LR, LOSSRATE, WEIGHT, C1, C2)

        recorded_index = np.arange(NUM_DATA_POINTS)
        recorded_loss = np.zeros(NUM_DATA_POINTS)
        predicted_loss = np.zeros(NUM_DATA_POINTS)
        recorded_rate = np.zeros(NUM_DATA_POINTS)
        rB = np.zeros(NUM_DATA_POINTS)
        last_rate = 0.0

        for i in range(1, NUM_DATA_POINTS):
            recorded_loss[i] = response.generate(recorded_rate[i - 1])

            (last_rate, predicted_loss[i]) = rateler.process(last_rate, recorded_loss[i])
            recorded_rate[i] = last_rate

            rB[i] = rateler.rB

        plt.figure(self.plotCount_)
        plt.plot(recorded_index, recorded_loss, 'r--',
                 recorded_index, predicted_loss, 'g^',
                 recorded_index, recorded_rate, 'yo',
                 recorded_index, rB, 'g-')
        plt.legend(['Actual Loss', 'Predicted loss', 'Rate', 'rB'])
        plt.title('Simulation of Loss and Control: ' + desc)
        plt.xlabel('Time step')
        plt.ylabel('Arbitrary units')
        plt.ylim(-5, 35)
        self.plotCount_ += 1

    def results(self):
        plt.show()

class Constant:
    """
    Produces a constant loss.
    """

    def __init__(self, loss):
        self.loss = loss

    def generate(self, rate):
        return self.loss

class Immediate:
    """
    An immediate response.
    """

    def __init__(self, rB, lossMax):
        self.rB = rB
        self.lossMax = lossMax

    def generate(self, rate):
        if rate > self.rB:
            return self.lossMax
        else:
            return 0

class Switch:
    """
    Switchs between two responses at a certain time.
    """

    def __init__(self, firstResponse, secondResponse, switchTime):
        self.fstRes_ = firstResponse
        self.sndRes_ = secondResponse
        self.swTime_ = switchTime

        self.time_ = 0

    def generate(self, rate):
        if self.time_ < self.swTime_:
            self.time_ += 1
            return self.fstRes_.generate(rate)
        else:
            return self.sndRes_.generate(rate)

class Noise:
    """
    Adds some noise to loss. May also occasionally return random RTTs.
    """

    def __init__(self, response, noise, percBad):
        self.response = response
        self.noise = noise
        self.percBad = percBad

    def generate(self, rate):
        if np.random.uniform() > 1.0 - self.percBad/2:
            return 0
        elif np.random.uniform() > 1.0 - self.percBad/2:
            return 10000

        return self.response.generate(rate) + random.uniform(0, self.noise)

class VarRatePenalizer:
    """
    Penalizes variations in rate. The more rapid the greater the penalty.
    """

    def __init__(self, response, penalty):
        self.response_ = response
        self.penalty_ = penalty

        self.lastRate_ = None

    def generate(self, rate):
        if self.lastRate_ is None:
            self.lastRate_ = rate
            return self.response_.generate(rate)
        else:
            addedLat = self.penalty_ * abs(self.lastRate_ - rate)
            self.lastRate_ = rate
            return self.response_.generate(rate) + addedLat

class NetRes:
    """
    Theoritical modeled response.
    """

    def __init__(self, rB):
        self.rB = rB

    def generate(self, r):
        return max(0, r - self.rB)


if __name__ == "__main__":
    tester = Tester()

    # Constant loss feedback. Could represent a high performance network that
    # works as fast as possible no matter the rate.
    tester.test(Constant(0.75), "Constant loss")

    tester.test(Immediate(10, 10), "Imm Res")
    tester.test(Noise(NetRes(10), 1.0, 1.0e-3), "Network Model")

    tester.results()

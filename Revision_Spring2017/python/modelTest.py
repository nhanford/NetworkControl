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

LR = 0.5
OVER = 3
C1 = 0.3
C2 = 0.3
W = 0.1
IPERIOD = 200
DPERIOD = 0

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
        rateler = Controller(LR, OVER, C1, C2, W, IPERIOD, DPERIOD)

        recorded_index = np.arange(NUM_DATA_POINTS)
        recorded_latency = np.zeros(NUM_DATA_POINTS)
        predicted_latency = np.zeros(NUM_DATA_POINTS)
        recorded_rate = np.zeros(NUM_DATA_POINTS)
        x = np.zeros(NUM_DATA_POINTS)
        rB = np.zeros(NUM_DATA_POINTS)
        lP = np.zeros(NUM_DATA_POINTS)
        last_rate = 0.0

        for i in range(1, NUM_DATA_POINTS):
            recorded_latency[i] = response.generate(recorded_rate[i - 1])

            (last_rate, predicted_latency[i]) = rateler.process(last_rate, recorded_latency[i])
            recorded_rate[i] = last_rate

            x[i] = rateler.x
            rB[i] = rateler.rB
            lP[i] = rateler.lP

        plt.figure(self.plotCount_)
        plt.plot(recorded_index, recorded_latency, 'r--',
                 #recorded_index, predicted_latency, 'g^',
                 recorded_index, recorded_rate, 'bo')#,
                 #recorded_index, x, 'c-',
                 #recorded_index, rB, 'g-'),
                 #recorded_index, lP, 'b-')
        #plt.legend(['Actual Latency', 'Predicted Latency', 'Rate', 'x', 'rB', 'lP'])
        plt.legend(['RTT', 'Pacing Rate'])
        plt.title('Simulation of Latency and Control: ' + desc)
        plt.xlabel('Time step')
        #plt.ylabel('Arbitrary units')
        plt.ylim(-5, 35)
        self.plotCount_ += 1

    def results(self):
        plt.show()

class Constant:
    """
    Produces a constant latency.
    """

    def __init__(self, latency):
        self.lat_ = latency

    def generate(self, rate):
        return self.lat_

class Offset:
    """
    A simple response that increases the latency by the amount the rate is off
    by.
    """

    def __init__(self, bestLatency, optimumRate, rateSensitivity):
        self.bestLat_ = bestLatency
        self.optRate_ = optimumRate
        self.rateSens_ = rateSensitivity

    def generate(self, rate):
        return self.bestLat_ + self.rateSens_ * abs(self.optRate_ - rate)

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
    Adds some noise to latency. May also occasionally return random RTTs.
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

class LatencyGenerator:
    """
    Latency generator implemented by David Fridovich.
    """

    def __init__(self, l_max, l_slope, l_cut, r_coeff, noise_sd = 0.1):
        self.l_max_ = l_max # Maximum value of latency before packet drop.
        self.l_slope_ = l_slope # Increment size for linear latency growth.
        self.l_cut_ = l_cut # Cuts back to this fraction of maximum after drop.
        self.r_coeff_ = r_coeff # Coefficient of most recent control input.
        self.noise_sd_ = noise_sd # Standard deviation of additive noise.

        # Keep track of most recent latency value.
        self.l_ = 0.0

    def generate(self, r):
        self.l_ += (self.l_slope_ + self.r_coeff_ * r +
                    self.noise_sd_ * np.random.randn())
        if self.l_ > self.l_max_:
            self.l_ *= self.l_cut_

        self.l_ = max(0.0, self.l_)

        return self.l_

class NetRes:
    """
    Theoritical modeled response.
    """

    def __init__(self, a, rB, lB, lP):
        self.a = a
        self.rB = rB
        self.lB = lB
        self.lP = lP

        self.x = 0

    def generate(self, r):
        self.x = self.x + (r - self.rB)/self.rB
        self.x = min(max(0, self.x), self.lB - self.lP)

        if r == 0:
            return self.lB
        else:
            return self.a/r + self.x + self.lP


if __name__ == "__main__":
    tester = Tester()

    # Constant latency feedback. Could represent a high performance network that
    # works as fast as possible no matter the rate.
    tester.test(Constant(5), "Constant Latency")

    # Here we model a network whose latency is best at a certain rate, with no
    # other considerations.
    tester.test(Offset(10, 5, 2), "Offset Latency")
    #tester.test(Noise(Offset(10, 5, 2), 0.5), "Offset Latency with Noise")

    # Model for when consistency is desired.
    #tester.test(VarRatePenalizer(Offset(10, 5, 2), 1),
    #        "Offset Latency with Penalized Rate Changes")

    # Here we switch from offset to constant halfway through. The idea is that
    # this could mimic a probing mode as suggested by Nate.
    #tester.test(Switch(Offset(10, 5, 2), Constant(10), NUM_DATA_POINTS/2),
    #        "Switch Offset to Constant Latency")

    #tester.test(LatencyGenerator(10, 1.0, 0.1, -0.2, 0.5),
    #        "Fridovich's Original Model")

    tester.test(Noise(NetRes(0.1*0.1, 10, 30, 15), 2.5, 0.0e-3), "Network Model")

    tester.results()

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
import matplotlib.pyplot as plt

# Adaptive filter parameters.
ALPHA = 0.5
P = 5
Q = 1

# Controller parameters.
PSI = 1.0
XI = 0.1
GAMMA = 0.5

NUM_DATA_POINTS = 200


def test(response):
    """
    Tests the controller out on a given response pattern.

    @arg response A model that takes a rate and determine the connection RTT.
    This model should have a method of the form generate(rate).
    """
    rateler = Controller(PSI, XI, GAMMA, P, Q, ALPHA)

    recorded_index = np.arange(NUM_DATA_POINTS)
    recorded_latency = np.zeros(NUM_DATA_POINTS)
    predicted_latency = np.zeros(NUM_DATA_POINTS)
    recorded_rate = np.zeros(NUM_DATA_POINTS)
    last_rate = 0.0

    for i in range(1, NUM_DATA_POINTS):
        recorded_latency[i] = response.generate(recorded_rate[i - 1])

        (last_rate, predicted_latency[i]) = rateler.Process(recorded_latency[i])
        recorded_rate[i] = last_rate

    plt.figure()
    plt.plot(recorded_index, recorded_latency, 'r--',
             recorded_index, predicted_latency, 'g^',
             recorded_index, recorded_rate, 'yo')
    plt.legend(['Actual Latency', 'Predicted Latency', 'Rate'])
    plt.title('Simulation of Latency and Control')
    plt.xlabel('Time step')
    plt.ylabel('Arbitrary units')
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

class Split:
    """
    Splits between two responses at a certain rate.
    """

    def __init__(self, firstResponse, secondResponse, cutoffRate):
        self.fstRes_ = firstResponse
        self.sndRes_ = secondResponse
        self.cutoffRate_ = cutoffRate

    def generate(self, rate):
        if rate < self.cutoffRate_:
            return self.fstRes_.generate(rate)
        else:
            return self.sndRes_.generate(rate)

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


if __name__ == "__main__":
    test(Constant(5))
    #test(Offset(10, 5, 2))
    #test(LatencyGenerator(1.0, 0.01, 0.1, -0.02, 0))

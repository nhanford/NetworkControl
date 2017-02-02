################################################################################
#
# Author: David Fridovich-Keil ( dfk@eecs.berkeley.edu )
# File: test_filter.py
#
# Test script for adaptive filter.
#
################################################################################

from latency_generator import LatencyGenerator
from adaptive_filter import AdaptiveFilter

import numpy as np
import matplotlib.pyplot as plt

NUM_DATA_POINTS = 500

# Adaptive filter parameters.
ALPHA = 0.5
BETA = 0.5
P = 5
Q = 5

# Latency generator parameters.
L_MAX = 1.0
L_SLOPE = 0.02
L_CUT = 0.1
R_COEFF = 0.01
NOISE_SD = 0.005

# Create a latency generator.
world = LatencyGenerator(L_MAX, L_SLOPE, L_CUT, R_COEFF, NOISE_SD)

# Create a filter.
model = AdaptiveFilter(P, Q, ALPHA, BETA)

# Run.
recorded_index = np.arange(NUM_DATA_POINTS)
recorded_latency = np.zeros(NUM_DATA_POINTS)
predicted_latency = np.zeros(NUM_DATA_POINTS)
for ii in range(NUM_DATA_POINTS):
    l_hat = model.Predict(0.01 * np.random.randn())

    predicted_latency[ii] = max(0.0, l_hat)
    recorded_latency[ii] = world.Generate(0.0)

    model.Update(recorded_latency[ii], predicted_latency[ii])

# Plot.
plt.figure()
plt.plot(recorded_index, recorded_latency, 'r--',
         recorded_index, predicted_latency, 'g^')
plt.legend(['Actual Latency', 'Predicted Latency'])
plt.title('Simulation of Latency Prediction')
plt.xlabel('Time step')
plt.ylabel('Arbitrary units')
plt.show()

################################################################################
#
# Author: David Fridovich-Keil ( dfk@eecs.berkeley.edu )
# File: test_filter.py
#
# Test script to tune parameters for adaptive filter based on recorded data.
#
################################################################################

from latency_generator import LatencyGenerator
from adaptive_filter import AdaptiveFilter

import numpy as np
import matplotlib.pyplot as plt

# Data path and trials.
DATA_PATH = "../data/"
TRIALS = [1, 2, 3, 4, 5]
RTT_COL = 2
TIME_COL = 0

# Adaptive filter parameters.
ALPHA = 0.5
BETA = 0.5
P = 3
Q = 5

# Create a filter.
model = AdaptiveFilter(P, Q, ALPHA, BETA)

# Read the data.
for trial in TRIALS:
    filename = DATA_PATH + "fqOn" + str(trial) + "t.csv"
    data = np.loadtxt(filename, delimiter=",", skiprows=1)
    time = data[:, TIME_COL]
    rtt = data[:, RTT_COL]

    # Run filter.
    predicted_latency = np.zeros(len(rtt))
    for ii in range(len(rtt)):
        l_hat = model.Predict(0.0)
        predicted_latency[ii] = max(0.0, l_hat)

        model.Update(rtt[ii], l_hat)

    # Plot.
    plt.figure()
    plt.plot(time, rtt, 'r--o',
             time, predicted_latency, 'g:*')
    plt.legend(['Actual Latency', 'Predicted Latency'])
    plt.title("Filter Performance for Trial " + str(trial));
    plt.xlabel('Time (s)')
    plt.ylabel('Round trip time (us)')

plt.show()

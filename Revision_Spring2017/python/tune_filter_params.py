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
from controller import Controller

import numpy as np
import matplotlib.pyplot as plt

# Data path and trials.
DATA_PATH = "../data/"
TRIALS = [1] #[1, 2, 3, 4, 5]
RTT_COL = 0 #2
TIME_COL = 0 #0
CONTROL_COL = 1

# Adaptive filter parameters.
ALPHA = 0.5
BETA = 0.5
P = 3
Q = 5

# Control params.
PSI = 0.9
XI = 1.0
GAMMA = 0.5

# Create a filter.
model = AdaptiveFilter(P, Q, ALPHA, BETA)

# Create a controller.
controller = Controller(PSI, XI, GAMMA, P, Q, ALPHA, BETA)

# Read the data.
for trial in TRIALS:
    filename = DATA_PATH + "random_control.csv" #"fqOn" + str(trial) + "t.csv"
    data = np.loadtxt(filename, delimiter=",", skiprows=1)
#    time = data[:, TIME_COL]
    time = np.arange(len(data))
    rtt = data[:, RTT_COL]
    control = data[:, CONTROL_COL]

    # Run filter.
    predicted_latency = np.zeros(len(rtt))
    predicted_control = np.zeros(len(rtt))
    for ii in range(len(rtt)):
        l_hat = model.Predict(float(control[ii]))
        predicted_latency[ii] = max(0.0, l_hat)
        model.Update(rtt[ii], l_hat)

        r_opt = controller.Process(rtt[ii])
        predicted_control[ii] = r_opt

    # Plot.
    plt.figure()
    plt.plot(time, rtt, 'r--o',
             time, predicted_latency, 'g:*',
             time, predicted_control, 'b:^')
    plt.legend(['Actual Latency', 'Predicted Latency', 'Predicted Control'])
    #plt.title("Filter Performance for Trial " + str(trial));
    plt.title("Filter Performance for Random Control")
    plt.xlabel('Time (s)')
    plt.ylabel('Round trip time (us)')

plt.show()

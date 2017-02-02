################################################################################
#
# Author: David Fridovich-Keil ( dfk@eecs.berkeley.edu )
# File: test_controller.py
#
# Test script for controller.
#
################################################################################

from latency_generator import LatencyGenerator
from controller import Controller

import numpy as np
import matplotlib.pyplot as plt

NUM_DATA_POINTS = 1000

# Adaptive filter parameters.
ALPHA = 0.5
BETA = 0.5
P = 1
Q = 1

# Controller parameters.
PSI = 0.5
XI = 0.1
GAMMA = 0.5

# Latency generator parameters.
L_MAX = 1.0
L_SLOPE = 0.02
L_CUT = 0.1
R_COEFF = 0.002
NOISE_SD = 0.0

# Create a latency generator.
world = LatencyGenerator(L_MAX, L_SLOPE, L_CUT, R_COEFF, NOISE_SD)

# Create a controller.
controller = Controller(PSI, XI, GAMMA, P, Q, ALPHA, BETA)

# Run.
recorded_index = np.arange(NUM_DATA_POINTS)
recorded_latency = np.zeros(NUM_DATA_POINTS)
predicted_latency = np.zeros(NUM_DATA_POINTS)
recorded_control = np.zeros(NUM_DATA_POINTS)
last_control = 0.0
for ii in range(NUM_DATA_POINTS):
    predicted_latency[ii] = controller.model_.Predict(last_control)
    controller.model_.r_.popleft()

    recorded_latency[ii] = world.Generate(last_control)
    last_control = max(0.0, controller.Process(recorded_latency[ii]))

#    print "Optimal control is : %f" % r
#    print "B1 coefficient is : %f" % controller.model_.b_[0]
    last_control = 0.0
    recorded_control[ii] = 0.0


# Plot.
plt.figure()
plt.plot(recorded_index, recorded_latency, 'r--',
         recorded_index, predicted_latency, 'g^',
         recorded_index, recorded_control, 'b--')
plt.legend(['Actual Latency', 'Predicted Latency', 'Control'])
plt.title('Simulation of Latency and Control')
plt.xlabel('Time step')
plt.ylabel('Arbitrary units')
plt.show()

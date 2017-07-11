import numpy as np
import matplotlib
import matplotlib.pyplot as plt


data = np.loadtxt("./fqOn1t.csv", delimiter=",", skiprows=1)

font = {'family' : 'normal',
        'weight' : 'normal',
        'size'   : 20}

matplotlib.rc('font', **font)

fig, ax1 = plt.subplots()
ax1.set_title('Uncontrolled RTT and Throughput')
p1 = ax1.plot(data[:, 6], data[:, 2], 'b', label='RTT', linewidth=2) # I know the data looks wrong here...
ax1.set_xlabel('Time (s)')
ax1.set_ylabel('RTT (ms)')
ax1.yaxis.label.set_color('blue')
ax1.tick_params(axis='y', colors='blue')
ax1.set_ylim(26000, 30000)

ax2 = ax1.twinx()
p2 = ax2.plot(data[:, 6], 1e-9 * data[:, 4], 'g', label='Throughput', linewidth=2)
ax2.set_ylabel('Throughput (Gbps)')
ax2.yaxis.label.set_color('green')
ax2.tick_params(axis='y', colors='green')
ax2.set_ylim(0, 14)

ax1.legend(p1 + p2, ['RTT', 'Throughput'], loc='best')

plt.show()

#!/usr/bin/env python

import argparse
import json
import matplotlib as mpl
import matplotlib.pyplot as plt
import numpy as np

startTime = []
rtt = []
rttVar = []
rate = []
retrans = []

parser = argparse.ArgumentParser()
parser.add_argument('FILE', type=str)
args = parser.parse_args()
dataFile = args.FILE

with open(dataFile) as data:
    pdata = json.load(data)

    for interval in pdata['intervals']:
        for strm in interval['streams']:
            startTime.append(strm['start'])
            rtt.append(strm['rtt'])
            rttVar.append(strm['rttvar'])
            rate.append(strm['bits_per_second'])
            retrans.append(strm['retransmits'])

rtt_adj = np.array(rtt)/1000
rttVar_adj = np.array(rttVar)/1000
rate_adj = list([r/(1<<20) for r in rate])

fig, (ax1, ax3) = plt.subplots(1, 2)
ax2 = ax1.twinx()
ax4 = ax3.twinx()

ax1.plot(startTime, rtt_adj, 'r-', label = 'rtt')
ax1.set_ylabel('rtt (ms)')

ax2.set_ylim(-1, max(rtt_adj))
ax2.plot(startTime, rttVar_adj, 'g', label = 'rtt variance')
ax2.set_ylabel('rtt variance (ms)')

ax3.set_ylim(-10, 2*np.percentile(rate_adj, 99))
ax3.plot(startTime, rate_adj, 'bs', label = 'rate')
ax3.set_ylabel('rate (mbit/s)')

ax4.plot(startTime, retrans, 'y', label = 'Retransmits')
ax4.set_ylabel('Retransmits')

fig.legend()

plt.show()

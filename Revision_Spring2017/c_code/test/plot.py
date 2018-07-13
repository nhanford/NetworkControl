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

# Smooth rate
for i in range(1, len(rate)):
    if rate[i] == 0:
        rate[i] = rate[i - 1]
        print(rate[i])

plt.figure(0)
plt.plot(startTime, rtt, 'r-',
        startTime, rttVar, 'g^')
plt.legend(['rtt (s)', 'rtt variance (s)'])
plt.title('BWctl Plot')
plt.xlabel('Time (s)')

plt.figure(1)
rate = list(map(lambda r: r/(1<<20), rate))
plt.ylim(0, 2*sum(rate)/len(rate))
plt.plot(startTime, rate, 'b--')
plt.legend(['rate (mbits/s)'])
plt.title('BWctl Plot')
plt.xlabel('Time (s)')
plt.show()

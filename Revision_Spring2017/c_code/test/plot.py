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

plt.figure(0)
plt.plot(startTime, np.array(rtt)/1000, 'r-',
        startTime, np.array(rttVar)/1000, 'g^')
plt.legend(['rtt (ms)', 'rtt variance 0(ms)'])
plt.title('BWctl Plot')
plt.xlabel('Time (s)')

plt.figure(1)
rate = list(map(lambda r: r/(1<<20), rate))
plt.ylim(-10, 2*sum(rate)/len(rate))
plt.plot(startTime, rate, 'bs')
plt.legend(['rate (mbits/s)'])
plt.title('BWctl Plot')
plt.xlabel('Time (s)')
plt.show()

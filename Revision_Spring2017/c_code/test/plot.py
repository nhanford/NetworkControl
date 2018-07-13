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

plt.figure()
plt.ylim(0, max(rtt))
plt.plot(startTime, rtt, 'r-',
        startTime, rttVar, 'g^',
        startTime, list(map(lambda r: r/(1<<10), rate)), 'b--')
plt.legend(['rtt (s)', 'rtt variance (s)', 'rate (kbits/s)'])
plt.title('BWctl Plot')
plt.xlabel('Time (s)')
plt.show()

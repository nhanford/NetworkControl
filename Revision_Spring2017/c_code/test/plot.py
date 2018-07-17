#!/usr/bin/env python

import argparse
import json
import matplotlib as mpl
import matplotlib.pyplot as plt
import numpy as np
import re

startTime = []
rtt = []
rttVar = []
rate = []
retrans = []
cwnd = []

mpcRTT = []
mpcRTTTime = []
mpcRate = []
mpcRateTime = []

parser = argparse.ArgumentParser()
parser.add_argument('TEST', type=str)
parser.add_argument('--title', type=str)
parser.add_argument('--output', type=str)
args = parser.parse_args()

bwctlFile = args.TEST + '-bwctl.json'
dmesgFile = args.TEST + '-dmesg.txt'
stimeFile = args.TEST + '-stime.txt'

dmRTT = re.compile('\[([0-9]*.[0-9]*)\] mpc: rtt_meas = ([0-9]+)us')
dmRate = re.compile('\[([0-9]*.[0-9]*)\] mpc: rate_opt = ([0-9]+) bytes/s')

dmesgStartTime = float(open(stimeFile).readline())

with open(bwctlFile) as data:
    pdata = json.load(data)

    for interval in pdata['intervals']:
        for strm in interval['streams']:
            startTime.append(strm['start'])
            rtt.append(strm['rtt'])
            rttVar.append(strm['rttvar'])
            rate.append(strm['bits_per_second'])
            retrans.append(strm['retransmits'])
            cwnd.append(strm['snd_cwnd'])

with open(dmesgFile) as data:
    for line in data:
        rttM = dmRTT.match(line)
        rateM = dmRate.match(line)

        if rttM is not None and float(rttM[1]) > dmesgStartTime:
            mpcRTT.append(int(rttM[2]))
            mpcRTTTime.append(float(rttM[1]) - dmesgStartTime)
        elif rateM is not None and float(rateM[1]) > dmesgStartTime:
            mpcRate.append(int(rateM[2]))
            mpcRateTime.append(float(rateM[1]) - dmesgStartTime)


rtt_adj = np.array(rtt)/1000
rttVar_adj = np.array(rttVar)/1000
rate_adj = list([r/(1<<20) for r in rate])

mpcRTT_adj = np.array(mpcRTT)/1000
mpcRate_adj = np.array(mpcRate) >> (20 - 3)

fig, ax = plt.subplots(2, 2, figsize=(10, 10))
ax1 = ax[0][0]
ax2 = ax1.twinx()
ax3 = ax[0][1]
ax4 = ax3.twinx()
ax5 = ax[1][0]
ax6 = ax[1][1]
ax7 = ax6.twinx()

ax1.plot(startTime, rtt_adj, 'r-', label = 'rtt')
ax1.set_xlabel('Time (s)')
ax1.set_ylabel('rtt (ms)')

ax2.set_ylim(-1, max(rtt_adj))
ax2.plot(startTime, rttVar_adj, 'g', label = 'rtt variance')
ax2.set_xlabel('Time (s)')
ax2.set_ylabel('rtt variance (ms)')

ax3.set_ylim(-10, 2*np.percentile(rate_adj, 99))
ax3.plot(startTime, rate_adj, 'bs', label = 'rate')
ax3.set_ylabel('rate (mbit/s)')

ax4.plot(startTime, retrans, 'y', label = 'Retransmits')
ax4.set_ylabel('Retransmits')

ax5.plot(startTime, cwnd, 'co', label = 'Congestion Window')
ax5.set_ylabel('Congestion Window')

ax6.plot(mpcRTTTime, mpcRTT_adj, 'gs', label = 'MPC Set RTT')
ax6.set_xlabel('Time (s), not related to others')
ax6.set_ylabel('RTT (ms)')

ax7.plot(mpcRateTime, mpcRate_adj, 'bs', label = 'MPC Observed Rate')
ax7.set_ylabel('Rate (mbit/s)')

fig.legend()

if args.title is not None:
    fig.suptitle(args.title)

if args.output is not None:
    plt.savefig(args.output)
else:
    plt.show()

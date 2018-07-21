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
bwctlStartTime = 0

mpcRTT = []
mpcRTTTime = []
mpcRate = []
mpcRateTime = []

parser = argparse.ArgumentParser(description="Plots test results.")
parser.add_argument('TEST', type=str,
        help="The name of the test to run.")
parser.add_argument('--title', type=str,
        help="Sets title for figure.")
parser.add_argument('--output', type=str,
        help="Sets output file.")
parser.add_argument('--limit-perc', type=int,
        help="Limits output range to within double of a certain percentile.")
args = parser.parse_args()

bwctlFile = args.TEST + '-bwctl.json'
moduleFile = args.TEST + '-module.json'
timeFile = args.TEST + '-time.txt'

with open(bwctlFile) as data:
    pdata = json.load(data)

    bwctlStartTime = pdata['start']['timestamp']['timesecs']

    for interval in pdata['intervals']:
        for strm in interval['streams']:
            startTime.append(strm['start'])
            rtt.append(strm['rtt'])
            rttVar.append(strm['rttvar'])
            rate.append(strm['bits_per_second'])
            retrans.append(strm['retransmits'])
            cwnd.append(strm['snd_cwnd'])

with open(moduleFile) as data:
    pdata = json.load(data)

    for entry in pdata:
        time = entry['time']
        sinceBWCTL = time - bwctlStartTime

        if entry['id'] == "rtt_meas_us":
            mpcRTT.append(entry['value'])
            mpcRTTTime.append(entry['time'])
        if entry['id'] == "rate_set":
            mpcRate.append(entry['value'])
            mpcRateTime.append(entry['time'])


rtt_adj = np.array(rtt)/1000
rttVar_adj = np.array(rttVar)/1000
rate_adj = list([r/(1<<20) for r in rate])
cwnd_adj = list([w/(1<<10) for w in cwnd])

mpcRTT_adj = list(map(lambda x: x/1000, mpcRTT))
mpcRate_adj = list(map(lambda x: x >> (20 - 3), mpcRate))

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
ax1.set_ylabel('RTT (ms)')

ax2.set_ylim(-1, max(rtt_adj))
ax2.plot(startTime, rttVar_adj, 'g', label = 'rtt variance')
ax2.set_xlabel('Time (s)')
ax2.set_ylabel('RTT variance (ms)')

ax3.set_ylim(-10, 2*np.percentile(rate_adj, 99))
ax3.plot(startTime, rate_adj, 'b', label = 'rate')
ax3.set_xlabel('Time (s)')
ax3.set_ylabel('Rate (mbit/s)')

ax4.plot(startTime, retrans, 'y', label = 'Retransmits')
ax4.set_ylabel('Retransmits')

ax5.plot(startTime, cwnd_adj, 'c', label = 'Congestion Window')
ax5.set_xlabel('Time (s)')
ax5.set_ylabel('Congestion Window (kbytes)')

ax6.plot(mpcRTTTime, mpcRTT_adj, 'r', label = 'MPC Observed RTT')
ax6.set_xlabel('Time (s)')
ax6.set_ylabel('MPC RTT (ms)')

ax7.plot(mpcRateTime, mpcRate_adj, 'b', label = 'MPC Set Rate')
ax7.set_ylabel('MPC Rate (mbit/s)')

if args.limit_perc is not None:
    ax1.set_ylim(0, np.percentile(rtt_adj, args.limit_perc)*2)
    ax2.set_ylim(0, np.percentile(rttVar_adj, args.limit_perc)*2)
    ax3.set_ylim(0, np.percentile(rate_adj, args.limit_perc)*2)
    ax4.set_ylim(0, np.percentile(retrans, args.limit_perc)*2)
    ax5.set_ylim(0, np.percentile(cwnd, args.limit_perc)*2)
    ax6.set_ylim(0, np.percentile(mpcRTT_adj, args.limit_perc)*2)
    ax7.set_ylim(0, np.percentile(mpcRate_adj, args.limit_perc)*2)

fig.legend()

if args.title is not None:
    fig.suptitle(args.title)

if args.output is not None:
    plt.savefig(args.output)
else:
    plt.show()

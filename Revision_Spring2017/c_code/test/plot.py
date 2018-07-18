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
bwctlTestDuration = 0

mpcRTT = []
mpcRTTTime = []
mpcRate = []
mpcRateTime = []

parser = argparse.ArgumentParser()
parser.add_argument('TEST', type=str)
parser.add_argument('--title', type=str)
parser.add_argument('--output', type=str)
parser.add_argument('--limit-perc', type=int)
args = parser.parse_args()

bwctlFile = args.TEST + '-bwctl.json'
dmesgFile = args.TEST + '-dmesg.json'
timeFile = args.TEST + '-time.txt'

dmRTT = re.compile('mpc: rtt_meas = ([0-9]+)us')
dmRate = re.compile('mpc: rate_opt = ([0-9]+) bytes/s')

with open(bwctlFile) as data:
    pdata = json.load(data)

    bwctlTestDuration = pdata['start']['test_start']['duration']

    for interval in pdata['intervals']:
        for strm in interval['streams']:
            startTime.append(strm['start'])
            rtt.append(strm['rtt'])
            rttVar.append(strm['rttvar'])
            rate.append(strm['bits_per_second'])
            retrans.append(strm['retransmits'])
            cwnd.append(strm['snd_cwnd'])

# TODO: 0.25 is just to ensure we don't miss some messages, probably should have
# a more accurate determiner for start time.
dmesgTimes = open(timeFile).readlines()
dmesgStart = float(dmesgTimes[0])
dmesgEnd = float(dmesgTimes[1])

with open(dmesgFile) as data:
    for line in data:
        msg = json.loads(line)

        rttM = dmRTT.match(msg['MESSAGE'])
        rateM = dmRate.match(msg['MESSAGE'])
        time = float(msg['__REALTIME_TIMESTAMP'])/1e6
        sinceBWCTL = time - dmesgEnd + bwctlTestDuration

        if rttM is not None and dmesgStart <= time <= dmesgEnd:
            mpcRTT.append(int(rttM[1]))
            mpcRTTTime.append(sinceBWCTL)
        elif rateM is not None and dmesgStart <= time <= dmesgEnd:
            mpcRate.append(int(rateM[1]))
            mpcRateTime.append(sinceBWCTL)


rtt_adj = np.array(rtt)/1000
rttVar_adj = np.array(rttVar)/1000
rate_adj = list([r/(1<<20) for r in rate])

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
ax1.set_ylabel('rtt (ms)')

ax2.set_ylim(-1, max(rtt_adj))
ax2.plot(startTime, rttVar_adj, 'g', label = 'rtt variance')
ax2.set_xlabel('Time (s)')
ax2.set_ylabel('rtt variance (ms)')

ax3.set_ylim(-10, 2*np.percentile(rate_adj, 99))
ax3.plot(startTime, rate_adj, 'b', label = 'rate')
ax3.set_xlabel('Time (s)')
ax3.set_ylabel('rate (mbit/s)')

ax4.plot(startTime, retrans, 'y', label = 'Retransmits')
ax4.set_ylabel('Retransmits')

ax5.plot(startTime, cwnd, 'c', label = 'Congestion Window')
ax5.set_xlabel('Time (s)')
ax5.set_ylabel('Congestion Window')

ax6.plot(mpcRTTTime, mpcRTT_adj, 'g', label = 'MPC Observed RTT')
ax6.set_xlabel('Time (s)')
ax6.set_ylabel('RTT (ms)')

ax7.plot(mpcRateTime, mpcRate_adj, 'r', label = 'MPC Set Rate')
ax7.set_ylabel('Rate (mbit/s)')

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

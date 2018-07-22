#!/usr/bin/env python

import argparse
import json
import matplotlib.pyplot as plt
import numpy as np
import pandas as pd

bwctlStartTime = 0

parser = argparse.ArgumentParser(description="Plots test results.")
parser.add_argument('TEST', type=str,
        help="The name of the test to run.")
parser.add_argument('--title', type=str,
        help="Sets title for figure.")
parser.add_argument('--output', type=str,
        help="Sets output file.")
parser.add_argument('--limit-perc', type=float,
        help="Limits output range to within double of a certain percentile.")
args = parser.parse_args()

bwctlFile = args.TEST + '-bwctl.json'
moduleFile = args.TEST + '-module.json'
timeFile = args.TEST + '-time.txt'

with open(bwctlFile) as data:
    pdata = json.load(data)
    bwctlStartTime = pdata['start']['timestamp']['timesecs']
    strms = list(map(lambda i: i['streams'], pdata['intervals']))
    strms = [x for y in strms for x in y] # Flatten
    bwctlData = pd.DataFrame(strms)

with open(moduleFile) as data:
    pdata = json.load(data)

    # columns is for when there are no entries.
    modData = pd.DataFrame(list(filter(lambda e: e != {}, pdata)),
            columns = ["time", "rtt_meas_us", "rate_set"])

    modData.time -= bwctlStartTime


rtt_adj = bwctlData.rtt/1000
rttVar_adj = bwctlData.rttvar/1000
rate_adj = bwctlData.bits_per_second/(1<<20)
cwnd_adj = bwctlData.snd_cwnd/(1<<10)

mpcRTT_adj = modData.rtt_meas_us/1000
mpcRate_adj = 8*modData.rate_set/(1 << 20)

fig, ax = plt.subplots(2, 2, figsize=(10, 10))
ax1 = ax[0][0]
ax2 = ax1.twinx()
ax3 = ax[0][1]
ax4 = ax3.twinx()
ax5 = ax[1][0]
ax6 = ax[1][1]
ax7 = ax6.twinx()

ax1.plot(bwctlData.start, rtt_adj, 'r-', label = 'rtt')
ax1.set_xlabel('Time (s)')
ax1.set_ylabel('RTT (ms)')

ax2.plot(bwctlData.start, rttVar_adj, 'g', label = 'rtt variance')
ax2.set_xlabel('Time (s)')
ax2.set_ylabel('RTT variance (ms)')

ax3.plot(bwctlData.start, rate_adj, 'b', label = 'rate')
ax3.set_xlabel('Time (s)')
ax3.set_ylabel('Rate (mbit/s)')

ax4.plot(bwctlData.start, bwctlData.retransmits, 'y', label = 'Retransmits')
ax4.set_ylabel('Retransmits')

ax5.plot(bwctlData.start, cwnd_adj, 'c', label = 'Congestion Window')
ax5.set_xlabel('Time (s)')
ax5.set_ylabel('Congestion Window (kbytes)')

ax6.plot(modData.time, mpcRTT_adj, 'r', label = 'MPC Observed RTT')
ax6.set_xlabel('Time (s)')
ax6.set_ylabel('MPC RTT (ms)')

ax7.plot(modData.time, mpcRate_adj, 'b', label = 'MPC Set Rate')
ax7.set_ylabel('MPC Rate (mbit/s)')

if args.limit_perc is not None:
    ax1.set_ylim(0, rtt_adj.quantile(args.limit_perc)*2)
    ax2.set_ylim(0, rttVar_adj.quantile(args.limit_perc)*2)
    ax3.set_ylim(0, rate_adj.quantile(args.limit_perc)*2)
    ax4.set_ylim(0, bwctlData.retransmits.quantile(args.limit_perc)*2)
    ax5.set_ylim(0, cwnd_adj.quantile(args.limit_perc)*2)
    ax6.set_ylim(0, mpcRTT_adj.quantile(args.limit_perc)*2)
    ax7.set_ylim(0, mpcRate_adj.quantile(args.limit_perc)*2)

fig.legend()

if args.title is not None:
    fig.suptitle(args.title)

if args.output is not None:
    plt.savefig(args.output)
else:
    plt.show()

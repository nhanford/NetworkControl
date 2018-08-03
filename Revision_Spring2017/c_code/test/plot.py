#!/usr/bin/env python

import argparse
import matplotlib.pyplot as plt

from data import Data

bwctlStartTime = 0

parser = argparse.ArgumentParser(description="Plots test results.")
parser.add_argument('test', type=str,
        help="The name of the test to run.")
parser.add_argument('-t', '--title', type=str,
        help="Sets title for figure.")
parser.add_argument('-o', '--output', type=str,
        help="Sets output file.")
parser.add_argument('-q', '--limit-quantile', type=float,
        help="Limits output range to within double of a certain quantile.")
parser.add_argument('-i', '--id', type=str,
        help="Only select kernel output on ID.")
args = parser.parse_args()


data = Data(args.test)

if args.id is not None:
    mid = args.id
    data.module = data.module.query("id == @mid")

rtt_adj = data.stream.rtt/1000
rttVar_adj = data.stream.rttvar/1000
rate_adj = data.stream.bits_per_second/(1<<20)
cwnd_adj = data.stream.snd_cwnd/(1<<10)

mpcRTT_adj = data.module.rtt_meas_us/1000
mpcRate_adj = 8*data.module.rate_set/(1 << 20)

fig, ax = plt.subplots(2, 2, figsize=(10, 10))
ax1 = ax[0][0]
ax2 = ax1.twinx()
ax3 = ax[0][1]
ax4 = ax3.twinx()
ax5 = ax[1][0]
ax6 = ax5.twinx()
ax7 = ax[1][1]
ax8 = ax7.twinx()

ax1.plot(data.stream.start, rtt_adj, 'r', label = 'RTT')
ax1.set_xlabel('Time (s)')
ax1.set_ylabel('RTT (ms)')

ax2.plot(data.stream.start, rttVar_adj, 'g', label = 'RTT Variance')
ax2.set_xlabel('Time (s)')
ax2.set_ylabel('RTT variance (ms)')

ax3.plot(data.stream.start, rate_adj, 'b', label = 'Rate')
ax3.set_xlabel('Time (s)')
ax3.set_ylabel('Rate (mbit/s)')

ax4.plot(data.stream.start, data.stream.retransmits, 'y', label = 'Retransmits')
ax4.set_ylabel('Retransmits')

ax5.plot(data.stream.start, cwnd_adj, 'c', label = 'Congestion Window')
ax5.set_xlabel('Time (s)')
ax5.set_ylabel('Congestion Window (kbytes)')

ax6.plot(data.module.time, data.module.probing, 'go', label = 'Probing')
ax6.set_ylabel('Probing')


ax7.plot(data.module.time, mpcRTT_adj, 'ro', label = 'MPC Observed RTT')
ax7.set_xlabel('Time (s)')
ax7.set_ylabel('MPC RTT (ms)')

ax8.plot(data.module.time, mpcRate_adj, 'bo', label = 'MPC Set Rate')
ax8.set_ylabel('MPC Rate (mbit/s)')

if args.limit_quantile is not None:
    ax1.set_ylim(0, rtt_adj.quantile(args.limit_quantile)*2)
    ax2.set_ylim(0, rttVar_adj.quantile(args.limit_quantile)*2)
    ax3.set_ylim(0, rate_adj.quantile(args.limit_quantile)*2)
    ax4.set_ylim(0, data.stream.retransmits.quantile(args.limit_quantile)*2)
    ax5.set_ylim(0, cwnd_adj.quantile(args.limit_quantile)*2)
    ax6.set_ylim(-1, 2)

    if len(mpcRTT_adj) > 0:
        ax7.set_ylim(0, mpcRTT_adj.quantile(args.limit_quantile)*2)

    if len(mpcRate_adj) > 0:
        ax8.set_ylim(0, mpcRate_adj.quantile(args.limit_quantile)*2)

fig.legend()

if args.title is not None:
    fig.suptitle(args.title)

if args.output is not None:
    plt.savefig(args.output)
else:
    plt.show()

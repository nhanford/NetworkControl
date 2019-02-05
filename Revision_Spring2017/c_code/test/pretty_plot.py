#!/usr/bin/env python3

import argparse
import matplotlib
import matplotlib.pyplot as plt

from data import Data

parser = argparse.ArgumentParser(description="Plots test results.")
parser.add_argument('test', type=str,
        help="The name of the test to run.")
parser.add_argument('min_rtt', type=float,
        help="Minimum RTT.")
parser.add_argument('max_rtt', type=float,
        help="Maximum RTT.")
parser.add_argument('min_rtt_var', type=float,
        help="Minimum RTT variance.")
parser.add_argument('max_rtt_var', type=float,
        help="Maximum RTT variance.")
parser.add_argument('min_rate', type=float,
        help="Minimum rate.")
parser.add_argument('max_rate', type=float,
        help="Maximum rate.")
parser.add_argument('title', type=str,
        help="Set plot title")
parser.add_argument('-s', '--font-size', type=int, default=22,
        help="Sets font size.")
parser.add_argument('-o', '--output', type=str,
        help="Sets output file base name.")
args = parser.parse_args()


data = Data(args.test)

rtt_adj = data.stream.rtt/1000
rttVar_adj = data.stream.rttvar/1000
rate_adj = data.stream.bits_per_second/(1<<20)
cwnd_adj = data.stream.snd_cwnd/(1<<10)

mpcRTT_adj = data.module.rtt_meas_us/1000
mpcRTTPred_adj = data.module.rtt_pred_us/1000
mpcRateM_adj = 8*data.module.rate_meas/(1 << 20)
mpcRateS_adj = 8*data.module.rate_set/(1 << 20)


matplotlib.rc('font', size=args.font_size)

fig, ax = plt.subplots(2, 1, figsize=(10, 10))
ax1 = ax[0]
ax2 = ax1.twinx()
ax3 = ax[1]
ax4 = ax3.twinx()

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


ax1.set_ylim(args.min_rtt, args.max_rtt)
ax2.set_ylim(args.min_rtt_var, args.max_rtt_var)
ax3.set_ylim(args.min_rate, args.max_rate)
#ax4.set_ylim(0, data.stream.retransmits.quantile(args.limit_quantile)*2)

fig.legend()
fig.suptitle(args.title)

if args.output is not None:
    fig.savefig(args.output)
else:
    plt.show()

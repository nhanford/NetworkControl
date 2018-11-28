#!/usr/bin/env python3

import argparse
import matplotlib.pyplot as plt

from data import Data

bwctlStartTime = 0

parser = argparse.ArgumentParser(description="Plots test results.")
parser.add_argument('test', type=str,
        help="The name of the test to run.")
parser.add_argument('-o', '--output', type=str,
        help="Sets output file base name.")
parser.add_argument('-e', '--output-extension', type=str, default="pdf",
        help="Sets the extension for output files (i.e. pdf, png, etc.).")
parser.add_argument('-q', '--limit-quantile', type=float,
        help="Limits output range to within double of a certain quantile.")
parser.add_argument('-i', '--id', type=str,
        help="Only select kernel output on ID.")
parser.add_argument('--guess-id', action='store_true',
        help="Guess the best ID to use")
args = parser.parse_args()


data = Data(args.test)
mid = args.id

if mid is None and args.guess_id:
    # We guess the appropriate MPC ID by looking at the variance in the measured
    # RTT. It has been observed that the one with the highest variance (actually
    # just a non-zero variance) is the correct id.
    groups = data.module.groupby('id')

    if not len(groups) == 0:
        mid = max(groups, key = lambda x: x[1].rtt_meas_us.std())[0]

if mid is not None:
    data.module = data.module.query("id == @mid")

rtt_adj = data.stream.rtt/1000
rttVar_adj = data.stream.rttvar/1000
rate_adj = data.stream.bits_per_second/(1<<20)
cwnd_adj = data.stream.snd_cwnd/(1<<10)

mpcRTT_adj = data.module.rtt_meas_us/1000
mpcRTTPred_adj = data.module.rtt_pred_us/1000
mpcRateM_adj = 8*data.module.rate_meas/(1 << 20)
mpcRateS_adj = 8*data.module.rate_set/(1 << 20)


bwfig, bwax = plt.subplots(2, 2, figsize=(10, 10))
ax1 = bwax[0][0]
ax2 = ax1.twinx()
ax3 = bwax[0][1]
ax4 = ax3.twinx()
ax5 = bwax[1][0]

modfig, modax = plt.subplots(2, 2, figsize=(10, 10))
ax6 = modax[0][0]
#ax7 = ax6.twinx()
ax8 = modax[0][1]
ax9 = ax8.twinx()
ax10 = modax[1][0]
ax11 = ax10.twinx()
ax12 = modax[1][1]
ax13 = ax12.twinx()

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


ax6.plot(data.module.time, mpcRTT_adj, 'ro', label = 'MPC Observed RTT')
ax6.plot(data.module.time, mpcRTTPred_adj, 'y', label = 'MPC Predicted RTT')
ax6.set_xlabel('Time (s)')
ax6.set_ylabel('MPC RTT (ms)')

ax8.plot(data.module.time, mpcRateS_adj, 'bo', label = 'MPC Set Rate')
ax8.set_xlabel('Time (s)')
ax8.set_ylabel('MPC Set Rate (mbit/s)')

ax9.plot(data.module.time, mpcRateM_adj, 'ro', label = 'MPC Observed Rate')
ax9.set_ylabel('MPC Measured Rate (mbit/s)')

ax11.plot(data.module.time, data.module.lp/1000, 'b--', label = 'lp')
ax11.set_ylabel('lp (ms)')

ax12.plot(data.module.time, 8*data.module.rb/(1 << 20), 'g--', label = 'rb')
ax12.set_xlabel('Time (s)')
ax12.set_ylabel('rb (mbits/s)')

ax13.plot(data.module.time, data.module.x/1000, 'y--', label = 'x')
ax13.set_ylabel('x (ms)')


if args.limit_quantile is not None:
    ax1.set_ylim(0, rtt_adj.quantile(args.limit_quantile)*2)
    ax2.set_ylim(0, rttVar_adj.quantile(args.limit_quantile)*2)
    ax3.set_ylim(0, rate_adj.quantile(args.limit_quantile)*2)
    ax4.set_ylim(0, data.stream.retransmits.quantile(args.limit_quantile)*2)
    ax5.set_ylim(0, cwnd_adj.quantile(args.limit_quantile)*2)

    if len(mpcRTT_adj) > 0:
        q1 = mpcRTT_adj.quantile(args.limit_quantile)
        q2 = mpcRTTPred_adj.quantile(args.limit_quantile)
        ax6.set_ylim(0, min(q1, q2)*2)

    if len(mpcRateS_adj) > 0:
        ax8.set_ylim(0, mpcRateS_adj.quantile(args.limit_quantile)*2)
        ax9.set_ylim(0, mpcRateS_adj.quantile(args.limit_quantile)*2)

bwfig.legend()
bwfig.suptitle("BWCTL")

modfig.legend()
modfig.suptitle("Kernel Module")


if args.output is not None:
    bwfig.savefig(args.output + "-test." + args.output_extension)
    modfig.savefig(args.output + "-module." + args.output_extension)
else:
    plt.show()

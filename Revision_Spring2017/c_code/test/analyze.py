
import argparse

from data import Data

parser = argparse.ArgumentParser(description="Statistically analyzes test results.")
parser.add_argument('TEST', type=str,
        help="The name of the test to analyze.")
args = parser.parse_args()


data = Data(args.TEST)

print("For BWCtl")
print(data.stream[['bits_per_second', 'rtt', 'rttvar', 'retransmits', 'snd_cwnd']].describe())

print("\nFor Kernel Module")
print(data.module[['rtt_meas_us', 'rate_set']].describe())

print("MPC ids found")
print(data.module.id.unique())


import argparse
import json
import matplotlib.pyplot as plt
import numpy as np
import pandas as pd

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

idRTTMean = pd.DataFrame(columns=['id', 'mean_rtt_meas_us'])
for x in data.module.groupby('id'):
    idRTTMean = idRTTMean.append({'id': x[0],
        'mean_rtt_meas_us': x[1].rtt_meas_us.mean()},
        ignore_index = True)

print(idRTTMean)

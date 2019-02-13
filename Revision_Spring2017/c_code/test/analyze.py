#!/usr/bin/python

import argparse
import json
import matplotlib.pyplot as plt
import numpy as np
import pandas as pd

from data import Data

parser = argparse.ArgumentParser(description="Statistically analyzes test results.")
parser.add_argument('test', type=str,
        help="The name of the test to analyze.")
args = parser.parse_args()


data = Data(args.test)

#print("For BWCtl")
#print(data.stream[['bits_per_second', 'rtt', 'rttvar', 'retransmits', 'snd_cwnd']].describe())

print("Mean RTT: {}".format(data.stream.rtt.mean()))
print("RTT Std: {}".format(np.sqrt(data.stream.rttvar.mean())))
print("Mean Rate: {:e} mbps".format(data.stream.bits_per_second.mean()/(1<<20)))
print("Rate Std: {:e} mbps".format(data.stream.bits_per_second.std()/(1<<20)))
print("Total Losses: {}".format(data.stream.retransmits.sum()))

#print("\nFor Kernel Module")
#print(data.module[['rtt_meas_us', 'rate_set']].describe())
#
#idRTTMean = pd.DataFrame(columns=['id', 'mean_rtt_meas_us', 'rtt_std'])
#for x in data.module.groupby('id'):
#    idRTTMean = idRTTMean.append({'id': x[0],
#        'mean_rtt_meas_us': x[1].rtt_meas_us.mean(),
#        'rtt_std': x[1].rtt_meas_us.std()},
#        ignore_index = True)
#
#print(idRTTMean)

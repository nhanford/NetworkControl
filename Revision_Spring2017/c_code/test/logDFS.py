#!/usr/bin/python

import argparse
import datetime
import glob
import json
import time
import os

parser = argparse.ArgumentParser(description="Log mpc debugfs output to a file.")
parser.add_argument('output', type=str,
        help="Output file.")
parser.add_argument('duration', type=float,
        help="Logging duration in seconds.")
parser.add_argument('-i', '--interval', type=float, default=0.1,
        help="Interval between samples.")
args = parser.parse_args()

def now():
    dt = datetime.datetime.now()
    return float("{}.{}".format(dt.strftime("%s"), dt.microsecond))


t = now()
end = t + args.duration

data = []
outputF = open(args.output, 'w')

while t <= end:
    t = now()

    for mpc in glob.glob("/sys/kernel/debug/mpc/*"):
        try:
            with open(mpc + "/rtt_meas_us") as rttF, open(mpc + "/rate_set") as rateF:
                rtt = int(rttF.read())
                rate = int(rateF.read())

                data.append({'time': t, 'id': os.path.basename(mpc),
                    'rtt_meas_us': rtt, 'rate_set': rate})

        except FileNotFoundError:
            pass

    s = args.interval - (now() - t)
    if s > 0:
        time.sleep(s)

json.dump(data, outputF, indent = 4)

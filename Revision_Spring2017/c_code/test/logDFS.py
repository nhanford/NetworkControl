#!/usr/bin/python

import argparse
import datetime
import glob
import json
import time
import os

parser = argparse.ArgumentParser(description="Log mpc debugfs output to a file.")
parser.add_argument('OUTPUT', type=str,
        help="Output file.")
parser.add_argument('DURATION', type=float,
        help="Logging duration in seconds.")
args = parser.parse_args()

def now():
    dt = datetime.datetime.now()
    return float("{}.{}".format(dt.strftime("%s"), dt.microsecond))


time = now()
end = time + args.DURATION

data = []
outputF = open(args.OUTPUT, 'w')

while time <= end:
    time = now()

    for mpc in glob.glob("/sys/kernel/debug/mpc/*"):
        try:
            with open(mpc + "/rtt_meas_us") as rttF, open(mpc + "/rate_set") as rateF:
                rtt = int(rttF.read())
                rate = int(rateF.read())

                data.append({'time': time, 'id': os.path.basename(mpc),
                    'rtt_meas_us': rtt, 'rate_set': rate})

        except FileNotFoundError:
            pass

    time.sleep(0.1)

json.dump(data, outputF, indent = 4)

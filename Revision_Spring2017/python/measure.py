#!/usr/bin/python3

import argparse
import datetime
import json
import subprocess
import time

parser = argparse.ArgumentParser(description = "Test pacing rate/RTT relation")
parser.add_argument('output', type = str,
        help = "The output file.")
parser.add_argument('destination', type = str,
        help = "The destination server address.")
parser.add_argument('interface', type = str,
        help = "The interface to attach the qdisc to.")
parser.add_argument('-m', '--min-rate', type = int, default = 100,
        help = "Min rate (in mbits/s) to test at.")
parser.add_argument('-x', '--max-rate', type = int, default = 40000,
        help = "Max rate (in mbits/s) to test at.")
parser.add_argument('-d', '--duration', type = float, default = 60,
        help = "Logging duration in seconds.")
parser.add_argument('-i', '--interval', type = float, default = 0.1,
        help = "Interval between samples.")
parser.add_argument('-t', '--tester', type = str, choices = ['iperf3', 'bwctl'],
        default = 'iperf3', help = "The tester to run.")
parser.add_argument('-v', '--verbose', action='store_true',
        help="Print additional information.")
args = parser.parse_args()


def now():
    dt = datetime.datetime.now()
    return float("{}.{}".format(dt.strftime("%s"), dt.microsecond))


if args.tester == 'iperf3':
    tester = subprocess.Popen(['iperf3', '-c', args.destination,
        '-i', str(args.interval), '-t', str(args.duration), '-J'],
        stdout = subprocess.PIPE)
elif args.tester == 'bwctl':
    tester = subprocess.Popen(['bwctl', '-c', args.destination,
        '-i', str(args.interval), '-t', str(args.duration), '-T', 'iperf3',
        '--parsable'],
        stdout = subprocess.PIPE)


rate = 0
end = now() + 3*args.duration

while now() < end and tester.poll() is None:
    rate = max(args.min_rate, (rate + 100) % (args.max_rate + 1))

    if args.verbose:
        print("Running test at {}mbit/s.".format(rate, tester.poll()))

    cp = subprocess.run(['tc', 'qdisc', 'replace', 'dev', args.interface,
        'root', 'netem', 'rate', '{}mbit'.format(rate)])

    if cp.returncode != 0:
        print("tc returned {}".format(cp.returncode))

    time.sleep(0.1)

subprocess.run(['tc', 'qdisc', 'del', 'dev', args.interface, 'root'])


with open(args.output, 'w') as f:
    f.write(tester.stdout.read().decode())

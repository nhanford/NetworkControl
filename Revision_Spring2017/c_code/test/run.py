#!/usr/bin/python3

import argparse
import subprocess
import os
import psutil
import time

import logger
import rate

parser = argparse.ArgumentParser(description = "Run a test.")
parser.add_argument('test', type = str,
        help = "The test to run.")
parser.add_argument('destination', type = str,
        help = "The destination server address.")
parser.add_argument('--min-rate', type = int,
        help = "Minimum pacing rate in mbps. (sysfs API only)")
parser.add_argument('--max-rate', type = int,
        help = "Maximum pacing rate in mbps. (sysfs API only)")
parser.add_argument('-d', '--duration', type = float, default = 60,
        help = "Logging duration in seconds.")
parser.add_argument('-i', '--interval', type = float, default = 0.1,
        help = "Interval between samples.")
parser.add_argument('-t', '--tester', type = str, choices = ['iperf3', 'bwctl'],
        default = 'iperf3', help = "The tester to run.")
parser.add_argument('-v', '--verbose', action='store_true',
        help="Print additional information.")
args = parser.parse_args()


print("Starting kernel logging.")
logger = logger.Logger(args.test + "-module.json", args.interval, args.verbose)
logger.start()


with open(args.test + "-test.json", mode = 'w') as testFile:
    print("Starting tester.")

    if args.tester == 'iperf3':
        tester = psutil.Popen(['iperf3', '-c', args.destination,
            '-i', str(args.interval), '-t', str(args.duration), '-J'],
            stdout = testFile)

        rateSetter = rate.RateSysfs(tester, args.min_rate, args.max_rate)
        rateSetter.start()

        tester.wait()
        print("Tester finished with code {}.".format(tester.returncode))

        rateSetter.stop()
    elif args.tester == 'bwctl':
        tester = psutil.Popen(['bwctl', '-c', args.destination,
            '-T', 'iperf3', '-i', str(args.interval), '-t', str(args.duration),
            '--parsable', '-p'],
            stdout = subprocess.PIPE)

        # Set rate for every port since I can't figure out how to get bwctl's
        # ports.
        rateSetter = rate.RateSysfs(tester, args.min_rate, args.max_rate, allPorts=True)
        rateSetter.start()

        (outputFile, _) = tester.communicate()
        outputFile = outputFile.splitlines()[0]
        print("Tester finished with code {}.".format(tester.returncode))

        rateSetter.stop()

        if outputFile != "":
            resFile = args.test + "-test.json"
            print("Moving BWCTL output file\"{}\" to \"{}\"."
                .format(outputFile, resFile))
            os.rename(outputFile, resFile)
        else:
            print("BWCTL failed to produce output.")
    else:
        print("Unrecognized tester " + args.tester + ".")

print("Stopping kernel logging")
logger.stop()

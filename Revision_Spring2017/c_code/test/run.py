#!/usr/bin/python

import argparse
import subprocess
import os

import logger

parser = argparse.ArgumentParser(description = "Run a test.")
parser.add_argument('test', type = str,
        help = "The test to run.")
parser.add_argument('destination', type = str,
        help = "The destination server address.")
parser.add_argument('-d', '--duration', type = float, default = 60,
        help = "Logging duration in seconds.")
parser.add_argument('-i', '--interval', type = float, default = 0.1,
        help = "Interval between samples.")
parser.add_argument('-t', '--tester', type = str, choices = ['iperf3', 'bwctl'],
        default = 'iperf3', help = "The tester to run.")
args = parser.parse_args()


print("Starting kernel logging.")
logger = logger.Logger(args.test + "-module.json", args.interval)
logger.start()

with open(args.test + "-test.json", mode = 'w') as testFile:
    print("Starting tester.")

    if args.tester == 'iperf3':
        tester = subprocess.Popen(['iperf3', '-c', args.destination,
            '-i', str(args.interval), '-t', str(args.duration), '-J'],
            stdout = testFile)

        tester.wait()
        print("Tester finished with code {}.".format(tester.returncode))
    elif args.tester == 'bwctl':
        tester = subprocess.Popen(['bwctl', '-c', args.destination,
            '-T', 'iperf3', '-i', str(args.interval), '-t', str(args.duration),
            '--parsable', '-p',],
            stdout = subprocess.PIPE)

        (outputFile, _) = tester.communicate()
        print("Tester finished with code {}.".format(tester.returncode))

        if outputFile != "":
            os.rename(outputFile, args.test + "-test.json")
        else:
            print("BWCTL failed to produce output.")
    else:
        print("Unrecognized tester " + args.tester + ".")

print("Stopping kernel logging")
logger.stop()

#!/usr/bin/python3

import argparse
import ipaddress
import subprocess
import os
import psutil
import time
import socket

import logger
import rate

parser = argparse.ArgumentParser(description = "Run a test.")
parser.add_argument('test', type=str,
        help="The test to run.")
parser.add_argument('destination', type=str,
        help="The destination server address.")
parser.add_argument('-r', '--rate', type=int, nargs=2, action='append',
        help="Max/min pacing rate in mbps for one stream. (sysfs API only)")
#parser.add_argument('-a', '--all-ports', action='store_true',
#        help="When setting min/max rates, apply to all ports on system.")
parser.add_argument('-P', '--parallel', type=int, default=1,
        help="When setting min/max rates, apply to all ports on system.")
parser.add_argument('-d', '--duration', type=float, default=60,
        help="Logging duration in seconds.")
parser.add_argument('-i', '--interval', type=float, default=0.1,
        help="Interval between samples.")
parser.add_argument('-t', '--tester', type=str, choices=['iperf3', 'bwctl'],
        default='iperf3', help="The tester to run.")
parser.add_argument('-v', '--verbose', action='store_true',
        help="Print additional information.")
args = parser.parse_args()


dest = int(ipaddress.ip_address(socket.gethostbyname(args.destination)))

print("Starting kernel logging.")
logger = logger.Logger(args.test + "-module.json", args.interval, args.verbose)
logger.start()


with open(args.test + "-test.json", mode = 'w') as testFile:
    print("Starting tester.")

    if args.tester == 'iperf3':
        tester = psutil.Popen(['iperf3', '-c', args.destination,
            '-i', str(args.interval), '-t', str(args.duration), '-J',
            '-P', str(args.parallel)],
            stdout = testFile)

        rateSetter = rate.RateSysfs(args.rate, dest, [], allPorts=True)
        rateSetter.start()

        tester.wait()
        print("Tester finished with code {}.".format(tester.returncode))

        rateSetter.stop()
    elif args.tester == 'bwctl':
        tester = psutil.Popen(['bwctl', '-c', args.destination,
            '-T', 'iperf3', '-i', str(args.interval), '-t', str(args.duration),
            '--parsable', '-p', '-P', str(args.parallel)],
            stdout = subprocess.PIPE)

        # Set rate for every port since I can't figure out how to get bwctl's
        # ports.
        rateSetter = rate.RateSysfs(args.rate, dest, [], allPorts=True)
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

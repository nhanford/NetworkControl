#!/bin/sh

test=$1
dest=$2

maxlog=1000

# Output start time so we know where to look in dmesg.
cat /proc/uptime | cut -d' ' -f1 > "$test-stime.txt"

outputFiles=$(bwctl -c $dest -T iperf3 -i.1 -w150m -t60 --parsable -p)

mv $outputFiles "$test-bwctl.json"

dmesg | grep 'mpc: ' | tail -n $maxlog > "$test-dmesg.txt"

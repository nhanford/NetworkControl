#!/bin/sh

test=$1
dest=$2

# Output start and end times so we know where to look in dmesg.
cat /proc/uptime | cut -d' ' -f1 > "$test-time.txt"

outputFiles=$(bwctl -c $dest -T iperf3 -i.1 -w150m -t60 --parsable -p)

cat /proc/uptime | cut -d' ' -f1 >> "$test-time.txt"

mv $outputFiles "$test-bwctl.json"

dmesg | grep 'mpc: ' > "$test-dmesg.txt"

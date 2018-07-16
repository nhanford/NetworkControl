#!/bin/sh

test=$1
dest=$2

outputFiles=$(bwctl -c $dest -T iperf3 -i.1 -w150m -t60 --parsable -p)

mv $outputFiles "$test-bwctl.json"

dmesg | grep 'mpc: ' > "$test-dmesg.txt"

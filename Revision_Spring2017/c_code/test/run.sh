#!/bin/sh

test=$1
dest=$2

# Output start and end times so we know where to look in dmesg.
data +'%s' > "$test-time.txt"

outputFiles=$(bwctl -c $dest -T iperf3 -i.1 -w150m -t60 --parsable -p)

date +'%s' >> "$test-time.txt"

mv $outputFiles "$test-bwctl.json"

journalctl -k -o json-pretty > "$test-dmesg.txt"

#!/bin/bash

test=$1
dest=$2

if [[ -n $3 ]]
then
  logTime=$3
else
  logTime=150
fi

# Limit journal output collected.
loglimit=10000

echo "Logging will take $logTime seconds."

./logDFS.sh "$test-module.json" $logTime &

# Output start and end times so we know where to look in dmesg.
date +'%s.%N' > "$test-time.txt"

outputFiles=$(bwctl -c $dest -T iperf3 -i.1 -w150m -t60 --parsable -p)

date +'%s.%N' >> "$test-time.txt"

mv $outputFiles "$test-bwctl.json"

echo "Waiting on logger"
wait

#!/bin/bash

# > run.sh TEST DEST [ LOGTIME ]

test=$1
dest=$2

if [[ -n $3 ]]
then
  logTime=$3
else
  logTime=120
fi

# Limit journal output collected.
loglimit=10000

echo "Logging will take $logTime seconds."

./logDFS.sh "$test-module.json" $logTime &

outputFiles=$(bwctl -c $dest -T iperf3 -i.1 -w150m -t60 --parsable -p)

mv $outputFiles "$test-bwctl.json"

echo "Waiting on logger"
wait

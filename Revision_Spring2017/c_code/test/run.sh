#!/bin/bash

if [[ $# -lt 2 ]]
then
	echo "> run.sh TEST DEST [ LOGTIME ]"
	exit 1
fi

test=$1
dest=$2

if [[ -n $3 ]]
then
	logTime=$3
else
	logTime=120
fi

echo "Logging will take $logTime seconds."

python logDFS.py "$test-module.json" $logTime &
echo "Started logDFS with pid $!"

outputFiles=$(bwctl -c $dest -T iperf3 -i.1 -w150m -t60 --parsable -p)

mv $outputFiles "$test-bwctl.json"

echo "Waiting on logger"
wait

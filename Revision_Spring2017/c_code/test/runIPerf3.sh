#!/bin/bash

if [[ $# -lt 1 ]]
then
    echo "> $0 TEST [ DEST ] [ LOGTIME ]"
    exit 1
fi

test=$1

if [[ -n $2 ]]
then
    dest=$2
else
    dest=10.10.1.1
fi

if [[ -n $3 ]]
then
    logTime=$3
else
    logTime=70
fi

echo "Logging will take $logTime seconds."

python logDFS.py "$test-module.json" $logTime &
loggerPID=$!
echo "Started logDFS with pid $loggerPID"

iperf3 -c $dest -i.1 -t60 -J > "$test-test.json"

if [[ $? -ne 0 ]]
then
    echo "Killing logger"
    kill $loggerPID
else
    echo "Waiting on logger"
    wait
fi

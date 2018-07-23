#!/bin/bash

# This script logs debugfs output.
# First argument: File to log to.
# Second argument: How long to log for (in seconds).

outputFile=$1
duration=$2
mpcDFS="/sys/kernel/debug/mpc"
rttFile="$mpcDFS/rtt_meas_us"
rateFile="$mpcDFS/rate_set"

chkTime=$(date +'%s')
endTime=$(($chkTime + $duration))

echo '[' > $outputFile

while [[ $chkTime -le $endTime ]]
do
  chkTime=$(date +'%s')
  time=$(date +'%s.%N')

  if [[ -e $mpcDFS ]]
  then
    rtt=$(cat $rttFile)
    rate=$(cat $rateFile)
    echo "{\"time\":$time,\"rtt_meas_us\":$rtt,\"rate_set\":$rate}," >> $outputFile
  fi

  sleep 0.1
done

echo "{}]" >> $outputFile

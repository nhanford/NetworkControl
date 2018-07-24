#!/bin/bash

# This script logs debugfs output.
# First argument: File to log to.
# Second argument: How long to log for (in seconds).

outputFile=$1
duration=$2
mpcDFS="/sys/kernel/debug/mpc"

chkTime=$(date +'%s')
endTime=$(($chkTime + $duration))

echo '[' > $outputFile

while [[ $chkTime -le $endTime ]]
do
  # Get RTT and rates from all instances.
  for dir in $mpcDFS/*
  do
    rttFile="$dir/rtt_meas_us"
    rateFile="$dir/rate_set"

    chkTime=$(date +'%s')
    time=$(date +'%s.%N')

    rtt=$(cat $rttFile 2> /dev/null)
    rate=$(cat $rateFile 2> /dev/null)

    if [[ -n $rtt && -n $rate ]]
    then
      echo "{\"time\":$time,\"id\":\"${dir##*/}\",\"rtt_meas_us\":$rtt,\"rate_set\":$rate}," >> $outputFile
    fi
  done

  sleep 0.1
done

echo "{}]" >> $outputFile

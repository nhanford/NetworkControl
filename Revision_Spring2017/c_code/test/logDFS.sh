#!/bin/bash

# This script logs debugfs output.
# First argument: File to log to.
# Second argument: How long to log for (in seconds).

outputFile=$1
duration=$2
rttFile="/sys/kernel/debug/mpc/rtt_meas_us"
rateFile="/sys/kernel/debug/mpc/rate_set"

chkTime=$(date +'%s')
endTime=$(($chkTime + $duration))

echo '[' > $outputFile

while [[ $chkTime -le $endTime ]]
do
  chkTime=$(date +'%s')
  time=$(date +'%s.%N')

  if [[ -e $rttFile ]]
  then
    rtt=$(cat $rttFile)
  else
    rtt=0
  fi

  if [[ -e $rateFile ]]
  then
    rate=$(cat $rateFile)
  else
    rate=0
  fi

  echo "{\"time\":$time,\"rtt_meas_us\":$rtt,\"rate_set\":$rate}," >> $outputFile

  sleep 0.1
done

echo "{}]" >> $outputFile

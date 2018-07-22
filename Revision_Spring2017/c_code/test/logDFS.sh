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
oldRTT=0
oldRate=0

echo '[' > $outputFile

while [[ $chkTime -le $endTime ]]
do
  chkTime=$(date +'%s')
  time=$(date +'%s.%N')

  if [[ -e $rttFile ]]
  then
    newRTT=$(cat $rttFile)
  else
    newRTT=$oldRTT
  fi

  if [[ -e $rateFile ]]
  then
    newRate=$(cat $rateFile)
  else
    newRate=$oldRate
  fi

  if [[ $oldRTT -ne $newRTT ]]
  then
    echo "{\"id\":\"rtt_meas_us\",\"time\":$time,\"value\":$newRTT}," >> $outputFile
    oldRTT=$newRTT
  fi

  if [[ $oldRate -ne $newRate ]]
  then
    echo "{\"id\":\"rate_set\",\"time\":$time,\"value\":$newRate}," >> $outputFile
    oldRate=$newRate
  fi

  sleep 0.1
done

echo "{\"id\":\"summary\"}]" >> $outputFile

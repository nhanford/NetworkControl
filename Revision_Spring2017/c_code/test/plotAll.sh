#!/bin/bash

for file in $(find . -name '*-bwctl.json')
do
  test=${file%-bwctl.json}

  if [ -e $test-dmesg.txt ] && [ -e $test-etime.txt ]
  then
    python ./plot.py $test --title "Test: $test" --output $test-plot.pdf
  fi
done

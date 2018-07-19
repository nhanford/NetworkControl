#!/bin/bash

for file in $(find . -name '*-bwctl.json')
do
  test=${file%-bwctl.json}

  if [[ -e $test-dmesg.json && -e $test-time.txt
    && ((! -e "$test-plot.pdf") || ($file -nt "$test-plot.pdf")) ]]
  then
    python ./plot.py $test --title "Test: $test" --output "$test-plot.pdf"
  fi
done

#!/bin/bash

for file in $(find . -name '*-bwctl.json')
do
  test=${file%-bwctl.json}

  if [[ -e "$test-module.json"
    && ((! -e "$test-plot.pdf") || ($file -nt "$test-plot.pdf")) ]]
  then
    python ./plot.py $test --title "Test: $test" --output "$test-plot.pdf"
  fi
done

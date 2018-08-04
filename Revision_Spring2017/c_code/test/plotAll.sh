#!/bin/bash

for file in $(find . -name '*-bwctl.json')
do
  test=${file%-bwctl.json}

  if [[ -e "$test-module.json"
    && ((! -e "$test-plot.pdf") || ($file -nt "$test-plot")) ]]
  then
    python ./plot.py $test --output "$test-plot"
  fi
done

#!/bin/sh

dir=$1
test=$2

sudo ./run.py "$dir/sacr/$test" sacr-pt1.es.net -t bwctl
sudo ./run.py "$dir/denv/$test" denv-pt1.es.net -t bwctl
sudo ./run.py "$dir/amst/$test" amst-pt1.es.net -t bwctl

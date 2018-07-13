#!/bin/sh

file=$1
dest=$2

bwctl -c $dest -T iperf3 -i.1 -w150m -t60 --parsable > $file

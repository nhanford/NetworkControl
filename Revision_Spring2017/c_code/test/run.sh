#!/bin/sh

dest=$1

bwctl -c $dest -T iperf3 -i.1 -w150m -t60 --parsable -p

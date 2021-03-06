#!/bin/bash

service irqbalance stop

cpupower -c all frequency-set -g userspace
cpupower -c all frequency-set -f 3.5G

ethtool -G eth4 rx 2048 tx 1024
ethtool -A eth4 rx off tx off
ethtool -C eth4 rx-usecs 8 rx-frames 32 tx-usecs 8 tx-frames 8
ethtool -K eth4 tso on gso on gro on tx-nocache-copy on

sysctl -w net.ipv4.tcp_congestion_control=htcp

tc qdisc del dev eth4 root

for j in {1..5}
do
    python localControllerTest.py .997 100 192.168.1.1 --on
    sleep 2
    rm *.bw
    tc qdisc del dev eth4 root
    python localControllerTest.py .997 100 192.168.1.1
    rm *.bw
    tc qdisc del dev eth4 root
    sleep 2
done
chown nate *.csv
chmod +r *.csv 
mkdir ~/`date +%F-%H-%M`
mv *.csv ~/`date +%F-%H-%M`/

#!/bin/bash

sudo service irqbalance stop

sudo cpupower -c all frequency-set -g userspace
sudo cpupower -c all frequency-set -f 3.5G

sudo ethtool -G eth4 rx 2048 tx 1024
sudo ethtool -A eth4 rx off tx off
sudo ethtool -C eth4 rx-usecs 8 rx-frames 32 tx-usecs 8 tx-frames 8
sudo ethtool -K eth4 tso on gso on gro on tx-nocache-copy on

sudo sysctl -w net.ipv4.tcp_congestion_control=htcp

sudo tc qdisc del dev eth4 root

for i in bost bois anl sacr
do
    for j in {1..3}
    do
        ssh nate@tilera "bwctl -T iperf3 -c $i-pt1.es.net -t60" &
        sudo python controllerTest.py 1 100 $i-pt1.es.net $j --on
        sudo tc qdisc del dev eth4 root
        sleep 2
        sudo rm *.bw
        sudo python controllerTest.py 1 100 $i-pt1.es.net $j
        sudo tc qdisc del dev eth4 root
        sudo rm *.bw
        sleep 2
    done
done
sudo chown nate *.csv
chmod +r *.csv 
mkdir ~/`date +%F-%H-%M`
mv *.csv ~/`date +%F-%H-%M`/

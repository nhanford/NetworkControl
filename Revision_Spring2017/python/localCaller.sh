#!/bin/bash

for j in {1..5}
do
    python localControllerTest.py .997 100 192.168.1.1 $j
    sleep 5
    rm *.bw
    tc qdisc del dev eth4 root
    sleep 2
done
chown nate *.csv
chmod +r *.csv 
mkdir ~/`date +%F`
mv *.csv ~/`date +%F`/

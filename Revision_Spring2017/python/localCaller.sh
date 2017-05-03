#!/bin/bash

for j in {1..5}
do
    python localControllerTest.py .997 100 192.168.1.1 1
    sleep 5
    mv localControlOutput.csv controlOutput-$j.csv
    mv XI-0.997-PSI-100.0-iPerfOutput.csv iPerfOutput-$j.csv
    rm *.bw
    tc qdisc del dev eth4 root
    sleep 2
done
chown nate *.csv
chmod +r *.csv 
mkdir ~/`date +%F`
mv *.csv ~/`date +%F`/

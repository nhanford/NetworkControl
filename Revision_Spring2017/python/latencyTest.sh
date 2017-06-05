#!/bin/bash

for i in bost bois anl sacr
do
    for j in {1..3}
    do
        bwctl -T iperf3 -c $i-pt1.es.net -t60 --parsable -p
        ./mtlb.py *.bw
        rm *.bw
    done
done
mkdir ~/`date +%F-%H-%M`
mv *.csv ~/`date +%F-%H-%M`/

sudo pping -vi eth4 &
./tcTest.sh &

for i in bost bois anl sacr
do
    for j in {1..3}
    do
        bwctl -T iperf3 -c $i-pt1.es.net -t60 --parsable -p
        ./mtlb.py *.bw
        rm *.bw
        
    done
done
mkdir ~/`date +%F-%H-%M`
mv *.csv ~/`date +%F-%H-%M`/

killall pping
killall tcTest.sh

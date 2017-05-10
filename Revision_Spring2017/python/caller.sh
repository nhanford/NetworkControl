#!/bin/bash

for i in bost bois anl sacr
do
    for j in {1..3}
    do
        python controllerTest.py 1 100 $i-pt1.es.net $j
        sleep 5
        rm *.bw
        sleep 2
    done
done
chown nate *.csv
chmod +r *.csv 
mkdir ~/`date +%F`
mv *.csv ~/`date +%F`/

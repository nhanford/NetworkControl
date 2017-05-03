#!/bin/bash

for j in {1..5}
do
    python controllerTest.py .997 100 192.1668.1.1 1
    sleep 5
    mv localControlOutput.csv controlOutput-$j.csv
    mv XI-0.997-PSI-100.0-iPerfOutput.csv iPerfOutput-$j.csv
    rm *.bw
    sleep 2
done
chown nate *.csv
chmod +r *.csv 
mkdir ~/`date +%F`
mv *.csv ~/`date +%F`/

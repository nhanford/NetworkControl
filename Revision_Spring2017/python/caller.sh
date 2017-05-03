#!/bin/bash

for i in bost denv bois amst
do
    for j in {1..3}
    do
        python controllerTest.py .997 100 $i-pt1.es.net 1
        sleep 5
        mv XI-0.997-PSI-100.0-controlOutput.csv controlOutput-$i-$j.csv
        mv XI-0.997-PSI-100.0-iPerfOutput.csv iPerfOutput-$i-$j.csv
        rm *.bw
        sleep 2
    done
done
chown nate *.csv
chmod +r *.csv 
mkdir ~/`date +%F`
mv *.csv ~/`date +%F`/

#!/bin/bash

tc qdisc add dev eth4 root fq
while 1:
do 
    sleep .01
    sudo tc qdisc change dev eth4 root fq maxrate 20Gbit
    sudo tc qdisc change dev eth4 root fq maxrate 21Gbit
done
tc qdisc del dev eth4 root

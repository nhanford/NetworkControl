#!/bin/bash

sudo tc qdisc add dev eth4 root fq
while :
do 
    sleep .01
    ss -itn
    sudo tc qdisc change dev eth4 root fq maxrate 20Gbit
    sudo tc qdisc change dev eth4 root fq maxrate 21Gbit
done
sudo tc qdisc del dev eth4 root

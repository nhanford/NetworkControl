#!/bin/bash

#to be run on kelewan

while :
do
    sudo tc qdisc change dev eth3 root fq maxrate 5Gbit
    sleep 10
    sudo tc qdisc change dev eth3 root fq maxrate 10Gbit
    sleep 10
done

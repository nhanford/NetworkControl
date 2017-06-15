#!/bin/bash

#to be run on kelewan

while :
do
    tc qdisc change dev eth3 root fq maxrate 5Gbit
    sleep 10
    tc qdisc change dev eth3 root fq maxrate 10Gbit
    sleep 10
done

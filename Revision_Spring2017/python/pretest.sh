#!/bin/bash

ip address add 192.168.1.1/24 dev eth3
ip route add 10.1.1.1 via 192.168.1.2

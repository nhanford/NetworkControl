#!/bin/sh

if [ $# -lt 2 ]
then
    echo "Usage: $0 testdir qdev"
    exit 1
fi

testdir=$(realpath $1)
qdev=$2
base=$(realpath ..)


vexec() {
    echo $@
    $@

    ret=$?

    if [ $ret -ne 0 ]
    then
        echo "Return status was $ret. aborting"
        exit 1
    fi
}

runSet() {
    dir=$testdir/$1

    vexec mkdir -p $dir

    vexec cd $base/test
    vexec ./run.py "$dir/sacr" sacr-pt1.es.net -t bwctl --rate 7500 10000
    vexec ./run.py "$dir/denv" denv-pt1.es.net -t bwctl --rate 7500 10000
    vexec ./run.py "$dir/amst" amst-pt1.es.net -t bwctl --rate 1000 10000
}

banner() {
    echo "===== $@ ====="
}

#banner "reno/pfifo"
#vexec sysctl -w net/ipv4/tcp_congestion_control=reno
#vexec tc qdisc replace dev $qdev root pfifo
#runSet reno-pfifo
#vexec tc qdisc delete dev $qdev root
#
#echo
#
#banner "cubic/pfifo"
#vexec sysctl -w net/ipv4/tcp_congestion_control=cubic
#vexec tc qdisc replace dev $qdev root pfifo
#runSet cubic-pfifo
#vexec tc qdisc delete dev $qdev root
#
#echo

banner "BBR/fq"
vexec sysctl -w net/ipv4/tcp_congestion_control=bbr
vexec tc qdisc replace dev $qdev root fq
runSet bbr-fq
vexec tc qdisc delete dev $qdev root

echo

banner "HTCP/pfifo"
vexec sysctl -w net/ipv4/tcp_congestion_control=htcp
vexec tc qdisc replace dev $qdev root pfifo
runSet htcp-pfifo
vexec tc qdisc delete dev $qdev root

#echo
#
#banner "reno/MPCC"
#vexec sysctl -w net/ipv4/tcp_congestion_control=reno
#vexec cd $base/qdisc
#vexec make
#vexec make start "QDEV=$qdev"
#runSet reno-mpccc
#vexec cd $base/qdisc
#vexec make stop "QDEV=$qdev"
#
#echo
#
#banner "cubic/MPCC"
#vexec sysctl -w net/ipv4/tcp_congestion_control=cubic
#vexec cd $base/qdisc
#vexec make
#vexec make start "QDEV=$qdev"
#runSet cubic-mpccc
#vexec cd $base/qdisc
#vexec make stop "QDEV=$qdev"

echo

banner "HTCP/MPCC"
vexec sysctl -w net/ipv4/tcp_congestion_control=htcp
vexec cd $base/qdisc
vexec make
vexec make start "QDEV=$qdev"
runSet htcp-mpccc
vexec cd $base/qdisc
vexec make stop "QDEV=$qdev"

echo

banner "MPCC/fq"
vexec cd $base/tcp_cc
vexec make
vexec make start "QDEV=$qdev"
runSet mpccc-fq
vexec cd $base/tcp_cc
sleep 5
vexec make stop "QDEV=$qdev"

echo

banner "cleanup"
vexec sysctl -w net/ipv4/tcp_congestion_control=cubic
vexec tc qdisc delete dev $qdev root

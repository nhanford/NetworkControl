
QDEV := eth3
MPC_DIR = ../mpc

obj-m += mpc_cc.o
mpc_cc-y += cong_control.o
mpc_cc-y += $(MPC_DIR)/scaled.o $(MPC_DIR)/model.o $(MPC_DIR)/control.o $(MPC_DIR)/dfs.o $(MPC_DIR)/sysfs.o
ccflags-y += -I$(shell realpath $(MPC_DIR)) -g -O0 -DDEBUG

all:
	make -C "/lib/modules/$(shell uname -r)/build" M=$(PWD) modules

clean:
	make -C "/lib/modules/$(shell uname -r)/build" M=$(PWD) clean

start:
	insmod mpc_cc.ko
	tc qdisc replace dev $(QDEV) root fq
	sysctl -w net.ipv4.tcp_congestion_control=mpc_cc

stop:
	sysctl -w net.ipv4.tcp_congestion_control=cubic
	tc qdisc del dev $(QDEV) root
	rmmod mpc_cc

restart:
	make stop QDEV=$(QDEV)
	make -B
	make start QDEV=$(QDEV)

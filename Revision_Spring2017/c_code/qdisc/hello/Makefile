
QDEV := enp0s3
IPROUTE2 := ./iproute2

obj-m += hiqd.o
hiqd-objs += sch_hi.o

all: q_hi.so
	make -C "/lib/modules/$(shell uname -r)/build" M=$(PWD) modules

q_hi.so: q_hi.c
	gcc -shared -o $@ $< -fPIC -I$(IPROUTE2)/include -I$(IPROUTE2)/tc

clean:
	make -C "/lib/modules/$(shell uname -r)/build" M=$(PWD) clean

start:
	insmod hiqd.ko
	tc qdisc add dev $(QDEV) root hi $(QARGS)

stop:
	tc qdisc del dev $(QDEV) root hi
	rmmod hiqd.ko

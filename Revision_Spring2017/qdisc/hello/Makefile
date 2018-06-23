
QDEV := enp0s3

obj-m += hiqd.o
hiqd-objs += module.o sch_hi.o

all:
	make -C "/lib/modules/$(shell uname -r)/build" M=$(PWD) modules

clean:
	make -C "/lib/modules/$(shell uname -r)/build" M=$(PWD) clean

start:
	insmod hiqd.ko
	tc qdisc add dev $(QDEV) root hi

stop:
	tc qdisc del dev $(QDEV) root hi
	rmmod hiqd.ko

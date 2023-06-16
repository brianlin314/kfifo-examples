obj-m += \
	bytestream-example.o inttype-example.o record-example.o producer-consumer.o

KERNELDIR ?= /lib/modules/`uname -r`/build
PWD       := $(shell pwd)

TARGET_MODULE := producer-consumer

all:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) clean

check: all
	sudo insmod $(TARGET_MODULE).ko
	sleep 11
	sudo rmmod $(TARGET_MODULE)
ifneq ($(KERNELRELEASE),)
    obj-m += threads.o
else

KERNELDIR ?= /usr/src/linux-headers-3.8.7/
#KERNELDIR ?= /home/nitin/Desktop/src/linux-3.11.1/

PWD := $(shell pwd)

default:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules
endif
clean:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) clean

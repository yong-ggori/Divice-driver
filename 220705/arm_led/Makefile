#KDIR := /lib/modules/$(shell uname -r)/build
KDIR := /home/ubuntu/udoo_linux_bsp/kernel/3.14.1

CROSS = ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf-
MOD := led
obj-m := $(MOD).o
PWD := $(shell pwd)

default:
#	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) modules
	$(MAKE) -C $(KDIR) M=$(PWD) modules $(CROSS)
	cp $(MOD).ko /srv/nfs

clean:
#	rm -rf *.ko
	rm -rf *.mod.*
	rm -rf .*.cmd
	rm -rf *.o
	rm -rf modules.order
	rm -rf Module.symvers
	rm -rf $(MOD).mod

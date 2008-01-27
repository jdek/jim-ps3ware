#
# Makefile for the PS3 RSX test module
#

list-m := ps3rsx
obj-m := ps3rsx.o

KSRC=/lib/modules/`uname -r`/build
PWD=$(shell pwd)

all:
	$(MAKE) -C $(KSRC) M=$(PWD) modules

clean:
	rm -f *.mod.c *.mod *.o *.ko .*.cmd .*.flags .lst *.lst

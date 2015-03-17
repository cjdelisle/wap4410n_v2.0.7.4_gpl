
obj-m := push_button.o 
PWD  := $(shell pwd)
KVER := 2.6

AP = pb_ap

ALL = $(AP)

all: $(ALL)
	$(MAKE) ARCH=mips CROSS_COMPILE=$(TOOLPREFIX) -C $(KERNELPATH) M=$(PWD) 
	
$(AP): pb_ap.c
	$(CC) $(CFLAGS) -Os -s -Wall -o $(AP) pb_ap.c

clean:
	rm -rf .*.cmd *.o *.mod.c *.ko .tmp_versions $(AP)


install:
	cp push_button.ko $(TARGETDIR)/lib/modules/push_button.ko
	cp $(AP) $(TARGETDIR)/usr/sbin/$(AP)

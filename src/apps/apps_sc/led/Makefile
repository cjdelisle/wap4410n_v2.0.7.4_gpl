PWD  := $(shell pwd)
KVER := 2.6
TOOLPREFIX := mips-linux-uclibc-
obj-m := led.o 

ifeq ($(WAP4410N),1)
EXTRA_CFLAGS += -DWAP4410N
endif

AP = led_ap

ALL = $(AP)

all: $(ALL)
	make ARCH=mips CROSS_COMPILE=$(TOOLPREFIX) -C $(KERNELPATH) M=$(PWD)

$(AP): led_ap.c
	$(CC) $(CFLAGS) -Os -s -Wall -o $(AP) led_ap.c
		
clean:
	rm -rf .*.cmd *.o *.mod.c *.ko .tmp_versions $(AP)
	
install:
	install -D led.ko $(TARGETDIR)/lib/modules/led.ko
	cp $(AP) $(TARGETDIR)/usr/sbin/$(AP)

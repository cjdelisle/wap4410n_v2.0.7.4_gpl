#######################################
ifndef BUILDPATH
export TOPDIR=$(shell pwd)/../../../..
export BUILDPATH = $(TOPDIR)/build

include $(BUILDPATH)/rules.mk
endif
######################################

CFLAGS += -Os -Wall -s -I$(APPSPATH)/include -fPIC -I ../../../libiconv-1.8/include/ 
LDFLAGS += -L$(TARGETDIR)/lib -lnvram
LDFLAGS +=  -L$(INSTALLDIR)/lib/	-liconv 
all: libapcfg.so

libapcfg.so : apcfg.o apply.o wifiuti.o
	$(CC) -Os -s -shared -fPIC $(LDFLAGS) -o $@ -Wl,-soname,$@ apcfg.o apply.o wifiuti.o
	
apcfg.o : apcfg.c
	$(CC) $(CFLAGS) -c apcfg.c -o $@

apply.o : apply.c
	$(CC) $(CFLAGS) -c apply.c -o $@
	
wifiuti.o : wifiuti.c
	$(CC) $(CFLAGS) -c wifiuti.c -o $@
		
install :
	cp -f *.so $(INSTALLDIR)/lib
	
clean:
	rm -rf *~ *.o *.so


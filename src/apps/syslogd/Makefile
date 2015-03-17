#######################################
ifndef BUILDPATH
export TOPDIR=$(shell pwd)/../..
export BUILDPATH = $(TOPDIR)/build

include $(BUILDPATH)/rules.mk
endif
######################################

CFLAGS += -Os -Werror -Wall -s -I$(APPSPATH)/include
CFLAGS += -L$(TARGETDIR)/lib -lnvram

BIN =syslogd #klogd

LIBS  = syslogd.c klogd.c 

all: make_links $(BIN) 

syslogd:$(LIBS)
	$(CC) $(CFLAGS) -o $@ $(LIBS) 
	$(STRIP) $@
#klogd:klogd.c
#	$(CC) $(CFLAGS) -o $@ $^

install:
	rm -f klogd
	ln -s syslogd klogd
	cp -a syslogd klogd $(INSTALLDIR)/usr/sbin/
clean:
	rm -f $(BIN) 

make_links:
	./mklnks

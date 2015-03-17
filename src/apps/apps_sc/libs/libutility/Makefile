#######################################
ifndef BUILDPATH
export TOPDIR=$(shell pwd)/../../../..
export BUILDPATH = $(TOPDIR)/build

include $(BUILDPATH)/rules.mk
endif
######################################

CFLAGS += -Os -Werror -Wall -s -I$(APPSPATH)/include  -fPIC

all: libutility.so

libutility.so : utility.o
	$(CC) -Os -s -shared -fPIC -o $@ -Wl,-soname,$@ $^
	
%.o : %.c
	$(CC) $(CFLAGS) -c $^ -o $@

install :
	cp -f *.so $(INSTALLDIR)/lib

clean:
	rm -rf *~ *.o *.so


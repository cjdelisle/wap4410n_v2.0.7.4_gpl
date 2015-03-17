#######################################
ifndef BUILDPATH
export TOPDIR=$(shell pwd)/../../../..
export BUILDPATH = $(TOPDIR)/build

include $(BUILDPATH)/rules.mk
endif
######################################

CFLAGS += -Os -Werror -Wall -s -I$(APPSPATH)/include -fPIC -I ../../../libiconv-1.8/include/ 

LDFLAGS += -L$(TARGETDIR)/lib -lnvram
LDFLAGS += -liconv 

all: libeditapcfg.so

libeditapcfg.so : editapcfg.o
	$(CC) -Os -s -shared -fPIC $(LDFLAGS) -o $@ -Wl,-soname,$@ editapcfg.o 
	
editapcfg.o : editapcfg.c
	$(CC) $(CFLAGS) -c editapcfg.c -o $@

test: editapcfg.o
	$(CC) $(LDFLAGS) -lapcfg -lutility -lscfg -o $@ editapcfg.o

install :
	cp -f *.so $(INSTALLDIR)/lib
	
clean:
	rm -rf *~ *.o *.so


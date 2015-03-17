#######################################
ifndef BUILDPATH
export TOPDIR=$(shell pwd)/../..
export BUILDPATH = $(TOPDIR)/build

include $(BUILDPATH)/rules.mk
endif
######################################

LDLIBS =  

VLAN_OBJS = vconfig.o

ALL_OBJS = ${VLAN_OBJS}
CCFLAGS += -I${KERNELPATH}/include

VCONFIG = vconfig                  #program to be created


all: ${VCONFIG}


#This is pretty silly..
vconfig.h: Makefile
	touch vconfig.h


$(VCONFIG): $(VLAN_OBJS)
	$(CC) $(CCFLAGS) $(LDFLAGS) -o $(VCONFIG) $(VLAN_OBJS) $(LDLIBS)
	$(STRIP) $(VCONFIG)

$(ALL_OBJS): %.o: %.c %.h
	@echo " "
	@echo "Making $<"
	$(CC) $(CCFLAGS) -c $<

install: all
	install -D $(VCONFIG) $(INSTALLDIR)/usr/sbin/$(VCONFIG)
	$(STRIP) $(INSTALLDIR)/usr/sbin/$(VCONFIG)

clean:
	rm -f *.o vconfig

purge: clean
	rm -f *.flc ${VCONFIG} vconfig.h
	rm -f *~






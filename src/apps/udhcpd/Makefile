# udhcp makefile
#######################################
ifndef BUILDPATH
export TOPDIR=$(shell pwd)/../..
export BUILDPATH = $(TOPDIR)/build

include $(BUILDPATH)/rules.mk
endif
######################################

prefix=$(TI_FILESYSTEM)
SBINDIR=/sbin
USRSBINDIR=${prefix}/sbin
USRBINDIR=${prefix}/bin
USRSHAREDIR=${prefix}/share

# Uncomment this to get a shared binary. Call as udhcpd for the server,
# and udhcpc for the client
COMBINED_BINARY=1

# Uncomment this for extra output and to compile with debugging symbols
#DEBUG=0

# Uncomment this to output messages to syslog, otherwise, messages go to stdout
CFLAGS += -Os -s -Wall


INSTALL = install

VER := 0.9.7

LIBS  = -L$(TARGETDIR)/lib -lapcfg -lutility -lnvram -lscfg
OBJS_SHARED = options.o socket.o packet.o pidfile.o 
DHCPD_OBJS = dhcpd.o arpping.o files.o leases.o serverpacket.o static_leases.o
DHCPC_OBJS = dhcpc.o clientpacket.o script.o

ifdef COMBINED_BINARY
EXEC1 = udhcpd
OBJS1 = $(DHCPD_OBJS) $(DHCPC_OBJS) $(OBJS_SHARED) frontend.o
CFLAGS += -DCOMBINED_BINARY
else
EXEC1 = udhcpd
OBJS1 = $(DHCPD_OBJS) $(OBJS_SHARED)

EXEC2 = udhcpc
OBJS2 = $(DHCPC_OBJS) $(OBJS_SHARED)
endif

ifeq ($(AUTOBRIDGE),1)
CFLAGS += -DAUTO_BRIDGE
endif

EXEC3 = dumpleases
OBJS3 = dumpleases.o

BOOT_PROGRAMS = udhcpc
DAEMONS = udhcpd
COMMANDS = dumpleases

ifdef SYSLOG
CFLAGS += -DSYSLOG
endif

CFLAGS += -W -Wall -Wstrict-prototypes -DVERSION='"$(VER)"'
ifdef DEBUG
CFLAGS += -g -DDEBUG
else
CFLAGS += -Os -fomit-frame-pointer
STRIP=-s
endif

CFLAGS +=-DRONSCODE
CPPFLAGS+= -I$(APPSPATH)/include

all: $(EXEC1) $(EXEC2) $(EXEC3)

$(OBJS1) $(OBJS2) $(OBJS3): *.h Makefile
$(EXEC1) $(EXEC2) $(EXEC3): Makefile

.c.o:
	$(CC) -c $(CFLAGS) $(CPPFLAGS) $<

$(EXEC1): $(OBJS1)
	$(CC) $(STRIP) $(OBJS1) -o $(EXEC1) $(LIBS)

$(EXEC2): $(OBJS2)
	$(CC) $(STRIP) $(OBJS2) -o $(EXEC2)

$(EXEC3): $(OBJS3)
	$(CC) $(STRIP) $(OBJS3) -o $(EXEC3)

install: 
	rm -f udhcpc
	ln -s udhcpd udhcpc
	cp -a udhcpd $(INSTALLDIR)/usr/sbin
	cp -a udhcpc $(INSTALLDIR)/usr/sbin	
clean:
	rm -f udhcpd udhcpc dumpleases *.o core



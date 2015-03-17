#######################################
ifndef BUILDPATH
export TOPDIR=$(shell pwd)/../..
export BUILDPATH = $(TOPDIR)/build

include $(BUILDPATH)/rules.mk
endif
######################################

OBJ:=ftp.o cmds.o cmdtab.o domacro.c main.o getpass.o ruserpass.o glob.o

%.o: %.c
	$(CC) -c $(CFLAGS) $(INC) $(KER_INC) -o $@ $<

all: ftp

ftp: $(OBJ)
	$(CC) $(CFLAGS) $(LDFLAGS) $(LDLIBS) -o $@ $^
	$(STRIP) $@
install: ftp 
	install -D ftp $(INSTALLDIR)/usr/sbin/ftp
	
clean:
	rm -f ftp *.o

.PHONY: all install clean


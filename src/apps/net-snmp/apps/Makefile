top_srcdir	= ..
srcdir		= .

CFLAGS += -Os -Wall -s -g -O2 -Dlinux 
CFLAGS += -I$(top_srcdir)/include -I../include \
		  -I.. -I$(srcdir)/.. -I$(srcdir)/../snmplib \
		   -I$(srcdir)/../agent -I../agent/helpers \
		  -I$(srcdir)/../agent/mibgroup \
		  -I$(top_srcdir)/../include

LDFLAGS = -L$(TARGETDIR)/lib

BIN = snmptrap

all: $(BIN)

install:
	install -D $(BIN) $(INSTALLDIR)/usr/sbin/$(BIN)
	$(STRIP) $(INSTALLDIR)/usr/sbin/$(BIN)

snmptrap: snmptrap.o 
	$(CC) $(CFLAGS) -o $@ $^ ../snmplib/libnetsnmp.a $(LDFLAGS) -lapcfg -lnvram -lutility -lscfg

clean:
	rm -f $(BIN) *.o


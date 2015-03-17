
CFLAGS += -Os -Werror -Wall -s
CFLAGS += -D_BIG_ENDIAN_ -DCGI -DSCFG_CLIENT -I$(APPSPATH)/include -fPIC
LDFLAGS += -L$(TARGETDIR)/lib -lnvram

LIBS  = scfgmgr.c socket_tools.c

all: libscfg.so #scfgmgr

scfgmgr.o : scfgmgr.c
	$(CC) $(CFLAGS) -g -c $^ -o $@ 

socket_tools.o : socket_tools.c
	$(CC) $(CFLAGS) -g -c $^ -o $@ 

libscfg.so : socket_tools.o
	$(CC) -Os -s -shared -fPIC $(LDFLAGS) -o $@ -Wl,-soname,$@ socket_tools.o
	
install:
	cp -f *.so $(INSTALLDIR)/lib
    	
clean:
	rm -rf *~ *.o *.so scfgmgr
	
	
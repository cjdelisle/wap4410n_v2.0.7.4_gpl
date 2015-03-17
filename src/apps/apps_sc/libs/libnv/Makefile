
CFLAGS  = -Os -s -Wall  -I$(KERNELPATH)/include

all: libnvram.so

libnvram.so : nvram.o
	$(CC) -Os -s -shared -Wl,-soname,libnvram.so -o libnvram.so nvram.o 

nvram.o : nvram.c
	$(CC) $(CFLAGS) -fPIC -g -c $^ -o $@

install:
	install -D libnvram.so $(INSTALLDIR)/lib/libnvram.so
	$(STRIP) $(INSTALLDIR)/lib/libnvram.so
	
clean:
	rm -rf *~ *.o *.so

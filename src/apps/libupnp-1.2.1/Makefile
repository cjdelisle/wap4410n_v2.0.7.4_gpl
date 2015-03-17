
TARGET_PREFIX = $(TOOLPREFIX)
TARGETDIR = $(INSTALLDIR)

all:
	$(MAKE)	CLIENT=0 TARGET=$(TARGET_PREFIX) -C upnp 
install:
	install -d $(TARGETDIR)/lib
	install ./upnp/bin/$(TARGET_PREFIX)/libixml.so $(TARGETDIR)/lib/
	$(STRIP) $(TARGETDIR)/lib/libixml.so
	install ./upnp/bin/$(TARGET_PREFIX)/libthreadutil.so $(TARGETDIR)/lib/
	$(STRIP) $(TARGETDIR)/lib/libthreadutil.so
	install ./upnp/bin/$(TARGET_PREFIX)/libupnp.so $(TARGETDIR)/lib/
	$(STRIP) $(TARGETDIR)/lib/libupnp.so
clean:
	$(MAKE) TARGET=$(TARGET_PREFIX) -C upnp clean

#
# $Id: Makefile.in,v 1.12 2004/03/04 23:31:24 shirleyma Exp $
#

#######################################
ifndef BUILDPATH
export TOPDIR=$(shell pwd)/../..
export BUILDPATH = $(TOPDIR)/build

include $(BUILDPATH)/rules.mk
endif
######################################
srcdir=	.
CFLAGS=	-g -O2 -I$(srcdir) -DPACKAGE_NAME=\"\" -DPACKAGE_TARNAME=\"\" -DPACKAGE_VERSION=\"\" -DPACKAGE_STRING=\"\" -DPACKAGE_BUGREPORT=\"\" -DYYTEXT_POINTER=1 -DHAVE_LIBCRYPTO=1 -DHAVE_LIBRESOLV=1 -DHAVE_GETADDRINFO=1 -DHAVE_GETNAMEINFO=1 -DHAVE_IF_NAMETOINDEX=1 -DSTDC_HEADERS=1 -DHAVE_SYS_TYPES_H=1 -DHAVE_SYS_STAT_H=1 -DHAVE_STDLIB_H=1 -DHAVE_STRING_H=1 -DHAVE_MEMORY_H=1 -DHAVE_STRINGS_H=1 -DHAVE_INTTYPES_H=1 -DHAVE_STDINT_H=1 -DHAVE_UNISTD_H=1 -DHAVE_FCNTL_H=1 -DHAVE_SYS_IOCTL_H=1 -DHAVE_SYS_TIME_H=1 -DHAVE_SYSLOG_H=1 -DHAVE_UNISTD_H=1 -DHAVE_IFADDRS_H=1 -DTIME_WITH_SYS_TIME=1 -DHAVE_STRUCT_TM_TM_ZONE=1 -DHAVE_TM_ZONE=1 -DGETPGRP_VOID=1 -DSETPGRP_VOID=1 -DRETSIGTYPE=void -DHAVE_MKTIME=1 -DHAVE_SELECT=1 -DHAVE_SOCKET=1 -DHAVE_ANSI_FUNC=1 -DHAVE_STDARG_H=1 
LDFLAGS= -L$(TARGETDIR)/lib
LIBOBJS= ifaddrs$U.o strlcpy$U.o strlcat$U.o
LIBS=	-lresolv -lcrypto -L../openssl-0.9.8r
YACC=	bison -y
LEX=	flex
TARGET=	dhcp6c
DESTDIR=$(INSTALLDIR)

INSTALL=install
INSTALL_PROGRAM=${INSTALL}
INSTALL_DATA=${INSTALL} -m 644
prefix=	/usr/local
exec_prefix=	${prefix}
bindir=	${exec_prefix}/bin
sbindir=${exec_prefix}/sbin
mandir=	${prefix}/man
initdir=/etc/rc.d/init.d
etc=/etc
sysconfigdir=/etc/sysconfig
CHKCONFIG=/sbin/chkconfig

CFLAGS+= -DCONF_DH6OPT_DNS_RESOLVERS=23 \
	-DCONF_DH6OPT_DOMAIN_LIST=24 \
	-DCONF_DH6OPT_IA_PD=25 \
	-DCONF_DH6OPT_IAPREFIX=26 

COMMONGENSRCS=lease_token.c
CLIENTGENSRCS=client6_parse.c client6_token.c dad_token.c ra_token.c \
		resolv_token.c radvd_token.c
CLIENTOBJS=	dhcp6c.o common.o config.o timer.o client6_addr.o \
		hash.o lease.o netlink.o\
	$(CLIENTGENSRCS:%.c=%.o) $(COMMONGENSRCS:%.c=%.o)

CLEANFILES=cf.tab.h cp.tab.h sf.tab.h dad_token.c ra_token.c client6_token.c client6_parse.c \
		server6_parse.c server6_token.c lease_token.c resolv_token.c radvd_token.c

all:	$(TARGET) 

dhcp6c:	$(CLIENTOBJS) $(LIBOBJS)
	$(CC) $(LDFLAGS) -o dhcp6c $(CLIENTOBJS) $(LIBOBJS) $(LIBS) -lnvram
	$(STRIP) $@
dad_token.c: dad_token.l
	$(LEX) -Pifyy dad_token.l
	mv lex.ifyy.c $@

ra_token.c: ra_token.l
	$(LEX) -Prayy ra_token.l
	mv lex.rayy.c $@

resolv_token.c: resolv_token.l
	$(LEX) -Prvyy resolv_token.l
	mv lex.rvyy.c $@

radvd_token.c: radvd_token.l
	$(LEX) -Prdyy radvd_token.l
	mv lex.rdyy.c $@

client6_parse.c cp.tab.h: client6_parse.y
	$(YACC) -d -p cpyy client6_parse.y
	mv y.tab.h cp.tab.h
	mv y.tab.c client6_parse.c

client6_token.c: client6_token.l
	$(LEX) -Pcpyy client6_token.l
	mv lex.cpyy.c $@	

lease_token.c: lease_token.l
	$(LEX) -Plyy lease_token.l
	mv lex.lyy.c $@

server6_parse.c sf.tab.h: server6_parse.y
	$(YACC) -d -p sfyy server6_parse.y
	mv y.tab.h sf.tab.h
	mv y.tab.c server6_parse.c 

server6_token.c: server6_token.l sf.tab.h
	$(LEX) -Psfyy server6_token.l
	mv lex.sfyy.c $@

install::
#	$(INSTALL_PROGRAM) -s -o bin -g bin $(TARGET) $(DESTDIR)$(sbindir)
	cp -a dhcp6c $(DESTDIR)/usr/sbin
#	-test -d $(DESTDIR)/usr/local || mkdir $(DESTDIR)/usr/local
#	-test -d $(DESTDIR)$(mandir) || mkdir $(DESTDIR)$(mandir)
#	-test -d $(DESTDIR)$(mandir)/man8 || mkdir $(DESTDIR)$(mandir)/man8 
#	-test -d $(DESTDIR)$(mandir)/man5 || mkdir $(DESTDIR)$(mandir)/man5
#	cp dhcp6c.8 $(DESTDIR)$(mandir)/man8/
#	cp dhcp6c.conf.5 $(DESTDIR)$(mandir)/man5/
	
	cp dhcp6c.conf $(DESTDIR)/etc/dhcp6c.conf

rh_install:: install
	$(INSTALL) -d $(initdir)
	$(INSTALL_PROGRAM) -o root -g root dhcp6c.sh $(initdir)/dhcp6c
	$(CHKCONFIG) --add dhcp6c

uninstall::
	for progs in $(TARGET); do \
		/bin/rm -f $(sbindir)/$$progs; \
	done
	/bin/rm -f $(mandir)/man8/dhcp6c.8
	/bin/rm -f $(mandir)/man5/dhcp6c.conf.5

rh_uninstall:: uninstall
	$(CHKCONFIG) --del dhcp6c
	/bin/rm -f $(initdir)/dhcp6c

includes::

clean::
	/bin/rm -f *.o $(TARGET) $(CLEANFILES) $(GENSRCS)

distclean:: clean
	/bin/rm -f Makefile config.cache config.log config.status .depend 

depend:
	mkdep ${CFLAGS:M-[ID]*} $(srcdir)/*.c

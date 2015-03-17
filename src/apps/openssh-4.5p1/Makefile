#SHELL = /bin/sh
#-----------------------------------------------------

AUTORECONF=autoreconf

prefix=
exec_prefix=${prefix}
bindir=${exec_prefix}/bin
sbindir=${exec_prefix}/sbin
libexecdir=${exec_prefix}/libexec
datadir=${datarootdir}
datarootdir=${prefix}/share
mandir=${datarootdir}/man
mansubdir=man
sysconfdir=${prefix}/etc/ssh
piddir=/var/run
srcdir=.
top_srcdir=.

DESTDIR=

SSH_PROGRAM=${exec_prefix}/usr/sbin/ssh
ASKPASS_PROGRAM=$(libexecdir)/ssh-askpass
SFTP_SERVER=$(libexecdir)/sftp-server
SSH_KEYSIGN=$(libexecdir)/ssh-keysign
RAND_HELPER=$(libexecdir)/ssh-rand-helper
PRIVSEP_PATH=/var/empty
SSH_PRIVSEP_USER=sshd
STRIP_OPT=-s

PATHS= -DSSHDIR=\"$(sysconfdir)\" \
	-D_PATH_SSH_PROGRAM=\"$(SSH_PROGRAM)\" \
	-D_PATH_SSH_ASKPASS_DEFAULT=\"$(ASKPASS_PROGRAM)\" \
	-D_PATH_SFTP_SERVER=\"$(SFTP_SERVER)\" \
	-D_PATH_SSH_KEY_SIGN=\"$(SSH_KEYSIGN)\" \
	-D_PATH_SSH_PIDDIR=\"$(piddir)\" \
	-D_PATH_PRIVSEP_CHROOT_DIR=\"$(PRIVSEP_PATH)\" \
	-DSSH_RAND_HELPER=\"$(RAND_HELPER)\"

#CC=gcc
#LD=gcc
CFLAGS+=-g -O2 -Wall -Wpointer-arith -Wuninitialized -Wsign-compare -std=gnu99 
CFLAGS += -I$(INC_APPS_PATH)
CPPFLAGS+=-I. -I$(srcdir) -I../openssl-0.9.8r/include -I../zlib-1.2.3/ $(PATHS) -DHAVE_CONFIG_H
LIBS =-lresolv -lutil -lnsl -lcrypt -L$(INSTALLDIR)/lib 
LIBS += -lcrypto -L../openssl-0.9.8r
#-lz
LIBS += -lz -L$(INSTALLDIR)/lib
LIBSELINUX=
SSHDLIBS=
LIBEDIT=
LIBPAM=
LIBWRAP=-lwrap -L../tcp_wrappers_7.6
#AR=/usr/bin/ar
AWK=gawk
RANLIB=mips-linux-uclibc-ranlib
INSTALL=/usr/bin/install -c
PERL=/usr/bin/perl
SED=/bin/sed
ENT=
XAUTH_PATH=/usr/X11R6/bin/xauth
LDFLAGS+=-L. -Lopenbsd-compat/ -L$(INSTALLDIR)/lib -L../zlib-1.2.3/
EXEEXT=

export RANLIB

INSTALL_SSH_PRNG_CMDS=
INSTALL_SSH_RAND_HELPER=

#TARGETS=ssh$(EXEEXT) sshd$(EXEEXT) ssh-add$(EXEEXT) ssh-keygen$(EXEEXT) ssh-keyscan${EXEEXT} ssh-keysign${EXEEXT} ssh-agent$(EXEEXT) scp$(EXEEXT) ssh-rand-helper${EXEEXT} sftp-server$(EXEEXT) sftp$(EXEEXT)
TARGETS=prepare_tcpwrap sshd$(EXEEXT)  #ssh-keygen$(EXEEXT)

LIBSSH_OBJS=acss.o authfd.o authfile.o bufaux.o bufbn.o buffer.o \
	canohost.o channels.o cipher.o cipher-acss.o cipher-aes.o \
	cipher-bf1.o cipher-ctr.o cipher-3des1.o cleanup.o \
	compat.o compress.o crc32.o deattack.o fatal.o hostfile.o \
	log.o match.o md-sha256.o moduli.o nchan.o packet.o \
	readpass.o rsa.o ttymodes.o xmalloc.o \
	atomicio.o key.o dispatch.o kex.o mac.o uidswap.o uuencode.o misc.o \
	monitor_fdpass.o rijndael.o ssh-dss.o ssh-rsa.o dh.o kexdh.o \
	kexgex.o kexdhc.o kexgexc.o scard.o msg.o progressmeter.o dns.o \
	entropy.o scard-opensc.o gss-genr.o

SSHOBJS= ssh.o readconf.o clientloop.o sshtty.o \
	sshconnect.o sshconnect1.o sshconnect2.o

SSHDOBJS=sshd.o auth-rhosts.o auth-passwd.o auth-rsa.o auth-rh-rsa.o \
	sshpty.o sshlogin.o servconf.o serverloop.o \
	auth.o auth1.o auth2.o auth-options.o session.o \
	auth-chall.o auth2-chall.o groupaccess.o \
	auth-skey.o auth-bsdauth.o auth2-hostbased.o auth2-kbdint.o \
	auth2-none.o auth2-passwd.o auth2-pubkey.o \
	monitor_mm.o monitor.o monitor_wrap.o kexdhs.o kexgexs.o \
	auth-krb5.o \
	auth2-gss.o gss-serv.o gss-serv-krb5.o \
	loginrec.o auth-pam.o auth-shadow.o auth-sia.o md5crypt.o \
	audit.o audit-bsm.o platform.o


PATHSUBS	= \
	-e 's|/etc/ssh/ssh_prng_cmds|$(sysconfdir)/ssh_prng_cmds|g' \
	-e 's|/etc/ssh/ssh_config|$(sysconfdir)/ssh_config|g' \
	-e 's|/etc/ssh/ssh_known_hosts|$(sysconfdir)/ssh_known_hosts|g' \
	-e 's|/etc/ssh/sshd_config|$(sysconfdir)/sshd_config|g' \
	-e 's|/usr/libexec|$(libexecdir)|g' \
	-e 's|/etc/shosts.equiv|$(sysconfdir)/shosts.equiv|g' \
	-e 's|/etc/ssh/ssh_host_key|$(sysconfdir)/ssh_host_key|g' \
	-e 's|/etc/ssh/ssh_host_dsa_key|$(sysconfdir)/ssh_host_dsa_key|g' \
	-e 's|/etc/ssh/ssh_host_rsa_key|$(sysconfdir)/ssh_host_rsa_key|g' \
	-e 's|/var/run/sshd.pid|$(piddir)/sshd.pid|g' \
	-e 's|/etc/ssh/moduli|$(sysconfdir)/moduli|g' \
	-e 's|/etc/ssh/sshrc|$(sysconfdir)/sshrc|g' \
	-e 's|/usr/X11R6/bin/xauth|$(XAUTH_PATH)|g' \
	-e 's|/var/empty|$(PRIVSEP_PATH)|g' \
	-e 's|/usr/bin:/bin:/usr/sbin:/sbin|/usr/bin:/bin:/usr/sbin:/sbin://bin|g'

FIXPATHSCMD	= $(SED) $(PATHSUBS)

all: $(TARGETS)
	$(STRIP) sshd
#	$(STRIP) ssh-keygen
		

install:all 
	cp -avf sshd $(INSTALLDIR)/usr/sbin
#	cp -avf ssh-keygen $(INSTALLDIR)/usr/sbin
	cp -avf sshd.init $(INSTALLDIR)/usr/sbin/sshd.init
	chmod +x $(INSTALLDIR)/usr/sbin/sshd.init
	test -d $(INSTALLDIR)/etc/ssh || mkdir -p $(INSTALLDIR)/etc/ssh
#	mkdir $(INSTALLDIR)/var/run
#	ln -s /var/ssh  $(INSTALLDIR)/etc/ssh/
	test -d $(INSTALLDIR)/usr/ssh ||mkdir -p $(INSTALLDIR)/usr/ssh
	cp -f sshd_config $(INSTALLDIR)/usr/ssh/
	cp -f $(TOOLPATH)/lib/libnsl-0.9.28.so $(INSTALLDIR)/lib
	cd $(INSTALLDIR)/lib && ln -fs libnsl-0.9.28.so libnsl.so.0 
	cp -f sshd_config $(INSTALLDIR)/etc/ssh 
	cp -f sshd_config $(INSTALLDIR)/etc
	cp -f ./ssh_keys/ssh_host* $(INSTALLDIR)/etc/ssh
	
#chmod +777 $(INSTALLDIR)/etc/ssh
#chmod +x $(INSTALLDIR)/etc/ssh/sshd_config
  
clean:
	rm -f *.o *.0 *.bak *.1 *.5* *.8 *.a ssh sshd ssh-add ssh-keygen ssh-keyscan ssh-keysign ssh-agent scp sftp-server sftp
	make -C openbsd-compat clean
	make -C ../tcp_wrappers_7.6 clean
    
$(LIBSSH_OBJS): Makefile.in config.h
$(SSHOBJS): Makefile.in config.h
$(SSHDOBJS): Makefile.in config.h

.c.o:
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $<

LIBCOMPAT=openbsd-compat/libopenbsd-compat.a
$(LIBCOMPAT): always
	(cd openbsd-compat && $(MAKE))
always:

libssh.a: $(LIBSSH_OBJS)
	$(AR) rv $@ $(LIBSSH_OBJS)
	$(RANLIB) $@

	
prepare_tcpwrap: 
	make -C ../tcp_wrappers_7.6 linux
	
	
sshd$(EXEEXT): libssh.a	$(LIBCOMPAT) $(SSHDOBJS)
	$(CC) -o $@ $(SSHDOBJS) $(LDFLAGS) -lssh -lopenbsd-compat $(LIBWRAP) $(LIBPAM) $(LIBSELINUX) $(SSHDLIBS) $(LIBS)

ssh-keygen$(EXEEXT): $(LIBCOMPAT) libssh.a ssh-keygen.o
	$(CC) -o $@ ssh-keygen.o $(LDFLAGS) -lssh -lopenbsd-compat $(LIBS)





ssh$(EXEEXT): $(LIBCOMPAT) libssh.a $(SSHOBJS)
	$(LD) -o $@ $(SSHOBJS) $(LDFLAGS) -lssh -lopenbsd-compat $(LIBS)

scp$(EXEEXT): $(LIBCOMPAT) libssh.a scp.o progressmeter.o
	$(LD) -o $@ scp.o progressmeter.o bufaux.o $(LDFLAGS) -lssh -lopenbsd-compat $(LIBS)

ssh-add$(EXEEXT): $(LIBCOMPAT) libssh.a ssh-add.o
	$(LD) -o $@ ssh-add.o $(LDFLAGS) -lssh -lopenbsd-compat $(LIBS)

ssh-agent$(EXEEXT): $(LIBCOMPAT) libssh.a ssh-agent.o
	$(LD) -o $@ ssh-agent.o $(LDFLAGS) -lssh -lopenbsd-compat $(LIBS)

ssh-keysign$(EXEEXT): $(LIBCOMPAT) libssh.a ssh-keysign.o
	$(LD) -o $@ ssh-keysign.o readconf.o $(LDFLAGS) -lssh -lopenbsd-compat $(LIBS)

ssh-keyscan$(EXEEXT): $(LIBCOMPAT) libssh.a ssh-keyscan.o
	$(LD) -o $@ ssh-keyscan.o $(LDFLAGS) -lssh -lopenbsd-compat -lssh $(LIBS)

sftp-server$(EXEEXT): $(LIBCOMPAT) libssh.a sftp.o sftp-common.o sftp-server.o
	$(LD) -o $@ sftp-server.o sftp-common.o $(LDFLAGS) -lssh -lopenbsd-compat $(LIBS)

sftp$(EXEEXT): $(LIBCOMPAT) libssh.a sftp.o sftp-client.o sftp-common.o sftp-glob.o progressmeter.o
	$(LD) -o $@ progressmeter.o sftp.o sftp-client.o sftp-common.o sftp-glob.o $(LDFLAGS) -lssh -lopenbsd-compat $(LIBS) $(LIBEDIT)



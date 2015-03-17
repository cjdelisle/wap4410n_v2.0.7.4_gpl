
SUBDIRS =
ifeq ($(WSC),1)
	SUBDIRS += libupnp-1.2.1
endif
SUBDIRS += libiconv-1.8
SUBDIRS += apps_sc
SUBDIRS += matrixssl-1-8-1
SUBDIRS += mini_httpd-1.17beta1
SUBDIRS += udhcpd
SUBDIRS += wpa2
SUBDIRS += busybox-1.1.0
SUBDIRS += net-snmp
SUBDIRS += wpa_supplicant-0.5.5
ifeq ($(SSHD),1)
	SUBDIRS += zlib-1.2.3
	SUBDIRS += openssl-0.9.8r
	SUBDIRS += openssh-4.5p1
endif
SUBDIRS += syslogd
SUBDIRS += vlan_config
SUBDIRS += dhcp-0.10
SUBDIRS += mDNSResponder-107.6
SUBDIRS += ftp_client

all:    applications

applications::
	@for i in ${SUBDIRS} ;  do                                                    \
            echo "-----------------------------------------------------------------"; \
            echo -e "\033[;34m                         $$i                 \033[;0m"; \
            echo "-----------------------------------------------------------------"; \
            make -C $$i || exit 1 ;                                                   \
            make -C $$i install || exit 1 ;                                           \
	done

install:


clean:	
	@for i in $(SUBDIRS) ; do $(MAKE) -C $$i clean || exit 1; done


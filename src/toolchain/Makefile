ifndef BOARD_TYPE
#error "You have to define Board Type to use this Makefile"
endif

ifndef BUILD_TYPE
#error "You have to define Build Type to use this Makefile"
endif

#
# Include the specific configuration files from the config.boardtype file
# here.  This removes the need to set environmental variables through a
# script before building
#

include scripts/$(BOARD_TYPE)/config.$(BOARD_TYPE)

#
# Put in safety checks here to ensure all required variables are defined in
# the configuration file
#

ifndef TOOLPREFIX
#error "Must specify TOOLPREFIX value"
endif

ifndef TOOLCHAIN
#error "Must specify TOOLCHAIN value"
endif

ifndef TOOLARCH
#error "Must specify TOOLARCH value"
endif

ifndef KERNEL
#error "Must specify KERNEL value"
endif

ifndef KERNELVER
#error "Must specify KERNELVER value"
endif

ifndef KERNELTARGET
#error "Must specify KERNELTARGET value"
endif

ifndef KERNELARCH
#error "Must specify KERNELARCH value"
endif

ifndef TOOLPREFIX
#error "Must specify TOOLPREFIX value"
endif

ifndef BUSYBOX
#error "Must specify BUSYBOX value"
endif

ifndef TFTPPATH
export TFTPPATH=/tftpboot/`whoami`
endif

export JFFS2FILE=$(BOARD_TYPE)$(BUILD_CONFIG)-jffs2
export UBOOTFILE=u-boot$(BUILD_CONFIG).bin
export INFOFILE=vmlinux$(BUILD_CONFIG).info
export KTFILE=$(KERNELTARGET:.bin=$(BUILD_CONFIG).bin)

#
# Other environmental variables that are configured as per the configuration file
# specified above.  These contain all platform specific configuration items.
#

export TOPDIR=$(PWD)/..
export INSTALL_ROOT=$(TOPDIR)/rootfs-$(BOARD_TYPE)$(BUILD_CONFIG).build
export IMAGEPATH=$(TOPDIR)/images/$(BOARD_TYPE)
temp_BOARD_TYPE = $(strip $(subst fus, , $(BOARD_TYPE)))

export STRIP=$(TOOLPREFIX)strip
export KERNELPATH=$(TOPDIR)/linux/kernels/$(KERNEL)
export MAKEARCH=$(MAKE) ARCH=$(KERNELARCH) CROSS_COMPILE=$(TOOLPREFIX)

export TOOLPATH=$(TOPDIR)/build/$(TOOLCHAIN)/$(TOOLARCH)/
export BOOTLOADERDIR=$(TOPDIR)/boot/redboot

export UBOOTDIR=$(TOPDIR)/boot/u-boot

export PATH:=$(TOPDIR)/build/util:$(TOOLPATH)/bin:$(TOPDIR)/linux:$(TOPDIR)/build:$(BOOTLOADERDIR)/ecos/tools/bin:`pwd`:${PATH}

# madwifi
export HAL=$(TOPDIR)/wlan/madwifi/hal/main
export ATH_PHYERR=$(TOPDIR)/wlan/madwifi/dfs
export ATH_RATE=$(TOPDIR)/wlan/madwifi/ratectrl11n/
export MODULEPATH=$(INSTALL_ROOT)/lib/modules/$(KERNELVER)/net

#
# This is to allow the target file system size to be specified on the command
# line, if desired
#

ifndef TARGETFSSIZE
export TARGETFSSIZE=2621440
endif


ENTRY=`readelf -a vmlinux|grep "Entry"|cut -d":" -f 2`
LDADR=`readelf -a vmlinux|grep "\[ 1\]"|cut -d" " -f 26`

#
# Include the board specific make file
#

include scripts/$(BOARD_TYPE)/Makefile.$(BOARD_TYPE)

#
# Common targts
#

rootfs_prep:
	@echo $(NEWPATH)
	rm -rf $(INSTALL_ROOT); mkdir $(INSTALL_ROOT)
	cp -R ../rootfs/common/* $(INSTALL_ROOT)
	cp -Rf ../rootfs/$(BOARD_TYPE)/* $(INSTALL_ROOT)
	chmod 755 $(INSTALL_ROOT)/etc/rc.d/*
	chmod 755 $(INSTALL_ROOT)/etc/ath/*
	chmod 755 $(INSTALL_ROOT)/etc/ath/default/*
	
toolchain_build:
	@echo making toolchain_build
	cd $(TOOLCHAIN) && \
	cp -f $(BOARD_TYPE).config .config && \
	cp -f toolchain/uClibc/$(BOARD_TYPE).config toolchain/uClibc/uClibc.config && \
	$(MAKE)
	# install libraries
	mkdir -p $(INSTALL_ROOT)/lib
	cd $(TOOLCHAIN) && make PREFIX=$(INSTALL_ROOT) install

check_tftp: image_prep
	if test -d $(TFTPPATH); then echo $(TFTPPATH) exists; else mkdir $(TFTPPATH); fi;

image_prep:
	if test -d $(TOPDIR)/images; then echo $(TOPDIR)/images exists; \
	else \
	mkdir $(TOPDIR)/images; \
	fi;
	if test -d $(IMAGEPATH); then echo $(IMAGEPATH) exists; \
	else \
	mkdir $(IMAGEPATH); \
	fi;

kernel_clean: 
	cd $(KERNELPATH) &&  $(MAKEARCH) mrproper 

kernel_info:
	cd $(KERNELPATH) && rm -f $(INFOFILE)
	cd $(KERNELPATH) && echo "entry:"${ENTRY} >> $(INFOFILE)
	cd $(KERNELPATH) && echo "link: 0x"${LDADR} >> $(INFOFILE)
	cp $(KERNELPATH)/$(INFOFILE) $(TFTPPATH)
	cp $(KERNELPATH)/$(INFOFILE) $(IMAGEPATH)

#
# Use different kernel_build rules depending on the type of target
#

ifeq ($(KERNELTARGET), zImage)
kernel_build: image_prep
	@echo " Making Kernel Build Version $(EXTRAVERSION)"
	exit
	cd $(KERNELPATH) && $(MAKEARCH) $(BOARD_TYPE)_defconfig
	cd $(KERNELPATH) && $(MAKEARCH)
	cd $(KERNELPATH)/arch/$(KERNELARCH)/boot && \
	cp $(KERNELTARGET) $(TFTPPATH) && cp $(KERNELTARGET) $(IMAGEPATH)
else	
kernel_build: image_prep
	@echo " Making Kernel Build Version $(EXTRAVERSION)"
	cd $(KERNELPATH) && $(MAKEARCH) $(BOARD_TYPE)$(BUILD_CONFIG)_defconfig
	cd $(KERNELPATH) && $(MAKEARCH)
	cd $(KERNELPATH) && $(MAKEARCH) $(KERNELTARGET)
	cd $(KERNELPATH)/arch/$(KERNELARCH)/boot && \
	cp $(KERNELTARGET) $(TFTPPATH)/$(KTFILE) && \
	cp $(KERNELTARGET) $(IMAGEPATH)/$(KTFILE) && \
	gzip -f $(KERNELTARGET) && \
	cp $(KERNELTARGET).gz $(TFTPPATH)/$(KTFILE).gz && \
	cp $(KERNELTARGET).gz $(IMAGEPATH)/$(KTFILE).gz;
endif

redboot_build:
	@echo making redboot...
	cd $(BOOTLOADERDIR) && make $(BOARD_TYPE)_rom
	cp $(BOOTLOADERDIR)/rom_bld/install/bin/redboot.rom $(IMAGEPATH)/redboot.$(BOARD_TYPE).rom
	cd $(BOOTLOADERDIR) && make $(BOARD_TYPE)_ram
	cp $(BOOTLOADERDIR)/ram_bld/install/bin/redboot.bin $(IMAGEPATH)/redboot.$(BOARD_TYPE).bin
	cp $(BOOTLOADERDIR)/ram_bld/install/bin/redboot.srec $(IMAGEPATH)/redboot.$(BOARD_TYPE).srec

uboot:
	cd $(UBOOTDIR) && $(MAKEARCH) mrproper
ifneq ($(BOARD_TYPE), $(temp_BOARD_TYPE))
	echo ====Using $(temp_BOARD_TYPE) config for $(BOARD_TYPE) ===
	cd $(UBOOTDIR) && $(MAKEARCH) $(temp_BOARD_TYPE)_config
else
	cd $(UBOOTDIR) && $(MAKEARCH) $(BOARD_TYPE)_config
endif
	cd $(UBOOTDIR) && $(MAKEARCH) all
	cp $(UBOOTDIR)/u-boot.bin ${IMAGEPATH}/${UBOOTFILE}

busybox_build: 
	@echo making busybox
	cd ../apps/$(BUSYBOX); $(MAKE) PREFIX=$(INSTALL_ROOT) uninstall; \
	make clean; \
	cp -f defconfig-$(BOARD_TYPE)$(BUILD_DEBUG) .config; \
	$(MAKE); $(MAKE) PREFIX=$(INSTALL_ROOT) install

wireless_tools:
	@echo making wireless_tools
	if test -n "$(WIRELESSTOOLS)"; then \
	cd ../apps/$(WIRELESSTOOLS); make clean; make; \
	$(STRIP) $(WIRELESSTOOLSLIB); \
	$(STRIP) iwpriv;  \
	$(STRIP) iwconfig; \
	$(STRIP) iwlist; \
	mkdir -p $(INSTALL_ROOT)/sbin; \
	cp -f $(WIRELESSTOOLSLIB) $(INSTALL_ROOT)/lib; \
	cp -f iwpriv $(INSTALL_ROOT)/sbin; \
	cp -f iwconfig $(INSTALL_ROOT)/sbin; \
	cp -f iwlist $(INSTALL_ROOT)/sbin; \
	cd $(TOPDIR)/wlan/madwifi/madwifi/tools && make clean \
	&&  make BUILD_STATIC=$(BUILD_STATIC) &&  $(STRIP) wlanconfig && \
	cp -f wlanconfig $(INSTALL_ROOT)/sbin && \
	$(STRIP) athstats athstatsclr athdebug 80211stats 80211debug athkey athampdutrc athcwm atrc pktlogconf pktlogdump radartool && \
	tar -czvf $(INSTALL_ROOT)/sbin/debug.tgz athstats athstatsclr athdebug 80211stats 80211debug athkey athampdutrc athcwm atrc pktlogconf pktlogdump radartool; \
	fi

diag_build:
	@echo diag_build
	cd $(HAL)/diag && make 
	
hal_build: 
	@echo making hal
	if test -n "$(MADWIFITARGET)"; then \
	cd $(HAL)/linux && make TARGET=$(MADWIFITARGET) clean &&  \
	make TARGET=$(MADWIFITARGET) && make TARGET=$(MADWIFITARGET) release; \
	fi

madwifi_build: hal_build wireless_tools
	@echo making madwifi
	cd $(TOPDIR)/wlan/madwifi/madwifi && \
	$(MAKEARCH) TARGET=$(MADWIFITARGET) clean \
	&&  $(MAKEARCH) TARGET=$(MADWIFITARGET) && \
	$(MAKEARCH) TARGET=$(MADWIFITARGET) install
	$(MAKE) $(BUILD_TYPE)_build


ifeq ($(BUILD_WPA2),y)  ####################################
# apps/wpa2 provides a second generation (as compared with apps/wpa)
#       of authentication (including WPS) programs:
#       hostapd, wpa_supplicant, etc.
#   It installs via $(INSTALL_ROOT).
#   It depends only on header files from the driver, and linux driver
#   (madwifi) header files specified by $(MADWIFIPATH)


ifndef MADWIFIPATH
# madwifi includes for wpa2 are found below $(MADWIFIPATH)
# (e.g. #include <net80211/ieee80211.h>)
export MADWIFIPATH=$(TOPDIR)/wlan/madwifi/madwifi
endif
wpa2: rootfs_prep
	   ln -s ../include $(TOPDIR)/wlan/linux/include
	   cd ../apps/wpa2 && $(MAKE)  \
	       CC=$(TOOLPREFIX)gcc AR=$(TOOLPREFIX)ar LD=$(TOOLPREFIX)ld

wpa2_clean:
	   cd ../apps/wpa2 && $(MAKE) clean
clean: wpa2_clean

else  ##################################################
# apps/wpa provides older generation of hostapd, wpa_supplicant, etc.


	
hostapd: openssl
	if ( test -e ../apps/wpa/hostapd-0.4.8 ) then \
	cd ../apps/wpa/hostapd-0.4.8; cp -f defconfig .config; make clean; make; \
	make PREFIX=$(INSTALL_ROOT)/sbin CONFIG_PATH=$(INSTALL_ROOT)/etc/ath DEFAULT_CFG=$(INSTALL_ROOT)/etc/ath/default install; \
	fi

openssl:
	if (test -e ../apps/wpa/wsc/lib/openssl-0.9.8a ) then \
	chmod -R 755 ../apps/wpa/wsc/lib/openssl-0.9.8a; \
	cd ../apps/wpa/wsc/lib/openssl-0.9.8a; make clean; make; fi

wsc: hostapd
	if (test -e ../apps/wpa/wsc/src/linux ) then \
	chmod -R 755 ../apps/wpa/wsc/src/lunux; \
	cd ../apps/wpa/wsc/src/linux; make clean; \
	make PREFIX=$(INSTALL_ROOT)/sbin CONFIG_PATH=$(INSTALL_ROOT)/etc/ath DEFAULT_CFG=$(INSTALL_ROOT)/etc/ath/default install; \
	fi

wpa_supplicant:
	if (test -e ../apps/wpa/wpa_supplicant-0.5.5 ) then \
	cd ../apps/wpa/wpa_supplicant-0.5.5; cp -f defconfig .config; make clean; \
    make; make PREFIX=$(INSTALL_ROOT)/sbin install; \
	fi

wps_enrollee:    
	if (test -e ../apps/wpa/wps_enrollee) then \
	cd ../apps/wpa/wps_enrollee; make clean; make; \
    make PREFIX=$(INSTALL_ROOT)/sbin install; \
	fi

endif

sar:
	@echo making sar
	cd ../apps/sysstat-6.0.1/ && rm -f sysstat.cron.daily && rm -f sysstat.cron.hourly && $(MAKE) CC=$(TOOLPREFIX)gcc 
	cd ../apps/sysstat-6.0.1/ && cp sar $(INSTALL_ROOT)/usr/bin/
	cd ../apps/sysstat-6.0.1/ && cp sadc $(INSTALL_ROOT)/usr/bin/

flash_mac:
	@echo making flash_mac 
	cd ../apps/flash_mac/ && make CC=$(TOOLPREFIX)gcc STRIP=$(TOOLPREFIX)strip && cp read_cfg $(INSTALL_ROOT)/usr/bin/
	

ram_build:
	@echo making ramfs
	cd util && \
        genext2fs -b 8192 -i 512 -D ../scripts/$(BOARD_TYPE)/dev.txt -d $(INSTALL_ROOT) $(BOARD_TYPE)-ramdisk
	cd util && gzip --best -f $(BOARD_TYPE)-ramdisk
	cd util && cp $(BOARD_TYPE)-ramdisk.gz $(TFTPPATH) 
	cd util && cp $(BOARD_TYPE)-ramdisk.gz $(IMAGEPATH) 

uimage:	kernel_build
	@echo making uImage
	cd util && mkuImage.sh $(UBOOTDIR)/tools $(KERNELPATH) $(BUILD_CONFIG)

jffs2_build:
	@echo making jffs2
	cd $(IMAGEPATH) && \
	$(TOPDIR)/build/util/mkfs.jffs2 --root=$(INSTALL_ROOT) --eraseblock=0x10000 -b -D $(TOPDIR)/build/scripts/$(BOARD_TYPE)/dev.txt --squash -o $(JFFS2FILE) --pad=$(TARGETFSSIZE)
	cp $(IMAGEPATH)/$(JFFS2FILE) $(TFTPPATH)




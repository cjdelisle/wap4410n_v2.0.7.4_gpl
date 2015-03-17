# ------------------------------------------------------------------------------------------
#                    Makefile for Atheros Linux 11n Access Point
#                                                      breeze zhang 
#                                                       2007-7-13
# ------------------------------------------------------------------------------------------

#######################################
ifndef BUILDPATH
export TOPDIR=$(shell pwd)/..
export BUILDPATH = $(TOPDIR)/build

include $(BUILDPATH)/rules.mk
endif

export TOPDIR=$(PWD)/..
export INSTALL_ROOT=$(TOPDIR)/build/rootfs.build

export IMAGEPATH=$(TOPDIR)/build
temp_BOARD_TYPE = $(strip $(subst fus, , $(BOARD_TYPE)))

export STRIP=$(TOOLPREFIX)strip
export KERNELPATH=$(TOPDIR)/linux/kernels/$(KERNEL)
export MAKEARCH=$(MAKE) ARCH=$(KERNELARCH) CROSS_COMPILE=$(TOOLPREFIX)

export TOOLPATH=$(TOPDIR)/toolchain/$(TOOLCHAIN)/$(TOOLARCH)/
export BOOTLOADERDIR=$(TOPDIR)/boot/redboot

export UBOOTDIR=$(TOPDIR)/u-boot

export PATH:=$(TOPDIR)/build/util:$(TOOLPATH)/bin:$(TOPDIR)/linux:$(TOPDIR)/build:$(BOOTLOADERDIR)/ecos/tools/bin:`pwd`:${PATH}

#export PATH:=$(TOPDIR)/build/util:$(TOOLPATH)/bin:$(TOPDIR)/linux:$(TOPDIR)/build:`pwd`:${PATH}

export MODULEPATH=$(INSTALL_ROOT)/lib/modules/$(KERNELVER)/net

######################################

ENTRY=`readelf -a vmlinux|grep "Entry"|cut -d":" -f 2`
LDADR=`readelf -a vmlinux|grep "\[ 1\]"|cut -d" " -f 26`


include $(KERNELPATH)/ath_version.mk

#
# Common targts
#

all:
#	make toolchain_build
	make uImage
	make kernel_info
	make enet_build
#	make hal_build
#	make madwifi_build
	make wireless_tools
	make apps_build
	make rootfs_build
	make image_build
	

# prepare the rootfs
rootfs_prep:
	@echo prepare the rootfs
	cd $(BUILDPATH)
	test -d $(INSTALL_ROOT) || mkdir $(INSTALL_ROOT)
	rm -rf $(INSTALL_ROOT)/dev/ && rm -rf common
	tar -zxf ../rootfs/common.tgz
	-cp -Rf common/* $(INSTALL_ROOT) && rm -rf common
#	cp -Rf ../rootfs/common/* $(INSTALL_ROOT)
	cp -Rf ../rootfs/$(PRODUCT_TYPE)/* $(INSTALL_ROOT)
#	rm -rf $(INSTALL_ROOT)/dev/* && mkdir -m 0755 $(INSTALL_ROOT)/dev/pts
	chmod 755 $(INSTALL_ROOT)/etc/rc.d/*;
	chmod 755 $(INSTALL_ROOT)/etc/ath/*;
	chmod 755 $(INSTALL_ROOT)/etc/ath/default/*; 
	ln -sf /var/passwd  $(INSTALL_ROOT)/etc/passwd
	ln -sf /var/resolv.conf  $(INSTALL_ROOT)/etc/resolv.conf
	@(cd $(TOPDIR); test -e wlan || ln -s wlan_$(WLANVER) wlan)

toolchain_prep:
	@echo making toolchain_prep
	test -d $(TOOLCHAIN) || (echo $(TOOLCHAIN) > extract.lst && \
	tar xfz $(TOPDIR)/gpl/gpl_files/common/toolchain.tgz -T extract.lst)
	cd $(TOOLCHAIN) && \
	cp -f $(BOARD_TYPE).config .config && \
	cp -f toolchain/uClibc/$(BOARD_TYPE).config toolchain/uClibc/uClibc.config && \
	echo "To make toolchain, exec 'make -C $(TOOLCHAIN)'"
	#$(MAKE) -e
	# install libraries
	#mkdir -p $(INSTALL_ROOT)/lib
	#cd /opt/mips_tools/tools/$(TOOLCHAIN) && make PREFIX=$(INSTALL_ROOT) install
		
# kernal build
kernel_build:
	@echo Making Kernel Build Version $(EXTRAVERSION)
	cd $(KERNELPATH) && $(MAKEARCH) $(KERNELTARGET)
	cd $(KERNELPATH) && $(MAKEARCH) modules
	cp $(KERNELPATH)/arch/$(KERNELARCH)/boot/$(KERNELTARGET) $(BUILDPATH)
	gzip -9c $(KERNELPATH)/arch/$(KERNELARCH)/boot/$(KERNELTARGET) > $(KERNELPATH)/arch/$(KERNELARCH)/boot/$(KERNELTARGET).gz
	cp $(KERNELPATH)/arch/$(KERNELARCH)/boot/$(KERNELTARGET).gz $(BUILDPATH)
	cp $(KERNELPATH)/net/ipv6/ipv6.ko $(INSTALL_ROOT)/lib/modules/
	cp $(KERNELPATH)/drivers/block/loop.ko $(INSTALL_ROOT)/lib/modules/
	@echo Making Kernel Build Version $(EXTRAVERSION) Done
	
kernel_info:
	@echo Making Kernel info
	cd $(KERNELPATH) && rm -f vmlinux.info
	cd $(KERNELPATH) && echo "entry:"${ENTRY} >> vmlinux.info
	cd $(KERNELPATH) && echo "link: 0x"${LDADR} >> vmlinux.info
	cp $(KERNELPATH)/vmlinux.info $(BUILDPATH)
	@echo Making Kernel info Done	

uImage:	kernel_build
	@echo making uImage
	@echo $(BUILDPATH)
	@echo $(KERNELPATH)
	mkuImage.sh $(BUILDPATH)/util $(KERNELPATH) 
		 
kernel_clean:
	cd $(KERNELPATH) &&  $(MAKEARCH) clean && rm -f vmlinux.info
	cd $(BUILDPATH) && rm -f $(KERNELTARGET) $(KERNELTARGET).gz vmlinux.info vmlinux vmlinux.gz.uImage

enet_build: kernel_build
	@echo Making Ethernet Driver
	cd $(ENETDIR)
	$(MAKEARCH) -C $(KERNELPATH) SUBDIRS=$(ENETDIR) modules
	test -d $(MODULEPATH) || mkdir -p $(MODULEPATH)
	cp $(ENETDIR)/ag7100_mod.ko $(MODULEPATH)
	@echo Making Ethernet Driver Done

enet_clean:
	@echo Making Ethernet Driver Clean
	cd $(ENETDIR) && rm -f *.o *.ko *.mod.*

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
	cp -f iwpriv $(INSTALL_ROOT)/usr/sbin; \
	cp -f iwconfig $(INSTALL_ROOT)/usr/sbin; \
	cp -f iwlist $(INSTALL_ROOT)/usr/sbin; \
	fi
	@echo making wireless_tools Done
	
	@echo making sar
	cd ../apps/sysstat-6.0.1/ && rm -f sysstat.cron.daily && rm -f sysstat.cron.hourly && $(MAKE) CC=$(TOOLPREFIX)gcc 
	cd ../apps/sysstat-6.0.1/ && cp sar $(INSTALL_ROOT)/usr/sbin/
	cd ../apps/sysstat-6.0.1/ && cp sadc $(INSTALL_ROOT)/usr/sbin/
	@echo making sar Done
	
uboot:
	@echo Making boot
	echo $(PATH)
	cd $(BUILDPATH)
	make -C ../u-boot BOARD_TYPE=ap83fus BUILD_TYPE=jffs2

apps_build:
	@echo Makeing apps
	cd $(BUILDPATH)
	make -C ../apps

apps_clean:
	@echo Clean apps
	cd $(BUILDPATH)
	make -C ../apps clean
	
rootfs_build:
	@echo make root file system
	cd $(BUILDPATH)
	find rootfs.build/ -name CVS | xargs rm -rf
	find rootfs.build/ -name Makefile | xargs rm -rf
	find rootfs.build/ -name _desktop.ini | xargs rm -rf
	cp product/$(PRODUCT_TYPE)/$(PRODUCT_TYPE).par image.par
	cp product/$(PRODUCT_TYPE)/$(PRODUCT_TYPE).pid $(PRODUCT_TYPE).pid
	cp product/$(PRODUCT_TYPE)/$(PRODUCT_TYPE).radio $(PRODUCT_TYPE).radio
	cp product/$(PRODUCT_TYPE)/lapBindcfg.txt lapBindcfg.txt
	cp product/$(PRODUCT_TYPE)/u-boot.bin u-boot.bin
	cp product/$(PRODUCT_TYPE)/download.bin download.bin
	cut -b 101-104 $(PRODUCT_TYPE).pid > rootfs.build/etc/fwversion
	./util/mksquashfs rootfs.build rootfs.sqsh -be -root-owned -noappend -D ./util/device_table.txt

rootfs_clean:
	@echo Clean rootfs 
	cd $(BUILDPATH) && rm -f image.par $(PRODUCT_TYPE).pid rootfs.sqsh download.bin u-boot.bin $(PRODUCT_TYPE).radio
	rm -rf $(INSTALL_ROOT)
	
image_build:
	@echo making image
	cd $(BUILDPATH)
	sh ./util/gen_image.sh
	mv linux.bin $(PRODUCT_TYPE).bin
	mv linux.img $(PRODUCT_TYPE).img
	@echo Done @ `date`
    
image_clean:
	@echo Clean image 
	cd $(BUILDPATH) && rm -f $(PRODUCT_TYPE).bin download.bin u-boot.bin $(PRODUCT_TYPE).img lapBindcfg.txt
	
# do clean
clean:
	make kernel_clean
	make enet_clean	
	make apps_clean
	make image_clean

# do cvs clean
cvsclean:
	find $(TOPDIR)/ -name CVS | xargs rm -rf
	find $(TOPDIR)/ -name .cvsignore | xargs rm -rf

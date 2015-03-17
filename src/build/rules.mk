#
#PRODUCT_TYPE: wap4410n, ap101na
#
ifndef PRODUCT_TYPE
export PRODUCT_TYPE = wap4410n
endif

#
#BOARD_TYPE: ap83fus
#
ifndef BOARD_TYPE
export BOARD_TYPE = ap83fus
endif

#
# Include the specific configuration files from the config.boardtype file
# here.  This removes the need to set environmental variables through a
# script before building
#

include ${BUILDPATH}/product/scripts/$(BOARD_TYPE)/config.$(BOARD_TYPE)
include ${BUILDPATH}/product/$(PRODUCT_TYPE)/config.$(PRODUCT_TYPE)

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

#
# Other environmental variables that are configured as per the configuration file
# specified above.  These contain all platform specific configuration items.
#

export INSTALL_ROOT=$(TOPDIR)/build/rootfs.build

export IMAGEPATH=$(TOPDIR)/build/
temp_BOARD_TYPE = $(strip $(subst fus, , $(BOARD_TYPE)))

export TOOLPREFIX=$(TOPDIR)/toolchain/$(TOOLCHAIN)/$(TOOLARCH)/bin/mips-linux-uclibc-
export STRIP=$(TOOLPREFIX)strip
export KERNELPATH=$(TOPDIR)/linux/kernels/$(KERNEL)
export MAKEARCH=$(MAKE) ARCH=$(KERNELARCH) CROSS_COMPILE=$(TOOLPREFIX)

#export TOOLPATH=/opt/mips_tools/tools/$(TOOLCHAIN)/$(TOOLARCH)/
export BOOTLOADERDIR=$(TOPDIR)/boot/redboot

export UBOOTDIR=$(TOPDIR)/boot/u-boot

export PATH:=$(TOPDIR)/build/util:$(TOOLPATH)/bin:`pwd`:${PATH}

#export PATH:=$(TOPDIR)/build/util:$(TOOLPATH)/bin:$(TOPDIR)/linux:$(TOPDIR)/build:`pwd`:${PATH}

export MODULEPATH=$(INSTALL_ROOT)/lib/modules/$(KERNELVER)/net

#
# This is to allow the target file system size to be specified on the command
# line, if desired
#

ifndef TARGETFSSIZE
export TARGETFSSIZE=2621440
endif
export WLANDIR=$(TOPDIR)/wlan
export WLANVER=7.3.0.435
export ENETDIR=$(TOPDIR)/linux/drivers/net/ag7100/
export HAL=$(WLANDIR)/common/lmac/hal

export TFTPPATH = $(BUILDPATH)
export INSTALLDIR = $(INSTALL_ROOT)
export TARGETDIR = $(INSTALL_ROOT)
export KERNEL_DIR = $(KERNELPATH)
export DESTDIR = $(INSTALL_ROOT)
export APPSPATH = $(TOPDIR)/apps

export BUILD_WPA2=y

ifeq ($(BUILD_WPA2),y)
export MADWIFIPATH=$(TOPDIR)/wlan/linux
export MADWIFIINC=$(TOPDIR)/wlan
endif

export BUS=AHB
export MADWIFITARGET=mipsisa32-be-elf
export FUSIONTARGET=mipsisa32-be-elf

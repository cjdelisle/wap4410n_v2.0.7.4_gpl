#
# Top level makefile for Build & Integration.
# 
# This file is used to facilitate checking the mDNSResponder project
# directly out of CVS and submitting to B&I at Apple.
#
# The various platform directories contain makefiles or projects
# specific to that platform.
#
#    B&I builds must respect the following target:
#         install:
#         installsrc:
#         installhdrs:
#         clean:
#

#######################################
ifndef BUILDPATH
export TOPDIR=$(shell pwd)/../..
export BUILDPATH = $(TOPDIR)/build

include $(BUILDPATH)/rules.mk
endif
######################################

MVERS = "mDNSResponder-107.6"
SRCROOT = $(shell pwd)
OS = linux

all:
	cd "$(SRCROOT)/mDNSPosix"; make os=$(OS)

install:
	cp -af $(SRCROOT)/mDNSPosix/build/prod/mDNSResponderPosix $(INSTALLDIR)/usr/sbin

installsrc:
	ditto . ${SRCROOT}

installhdrs::
	cd "$(SRCROOT)/mDNSMacOSX"; xcodebuild installhdrs OBJROOT=$(OBJROOT) SYMROOT=$(SYMROOT) DSTROOT=$(DSTROOT) MVERS=$(MVERS)

clean::
	cd "$(SRCROOT)/mDNSPosix"; make os=$(OS) clean

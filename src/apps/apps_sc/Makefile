#######################################
ifndef BUILDPATH
export TOPDIR=$(shell pwd)/../..
export BUILDPATH = $(TOPDIR)/build

include $(BUILDPATH)/rules.mk
endif
######################################

SUBDIRS =
SUBDIRS += libs
SUBDIRS += led
SUBDIRS += push_button
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


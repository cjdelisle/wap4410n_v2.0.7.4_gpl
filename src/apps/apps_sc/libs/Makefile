#######################################
ifndef BUILDPATH
export TOPDIR=$(shell pwd)/../../..
export BUILDPATH = $(TOPDIR)/build

include $(BUILDPATH)/rules.mk
endif
######################################


SUBLIBDIRS =
SUBLIBDIRS += ../../matrixssl-1-8-1
SUBLIBDIRS += libnv
SUBLIBDIRS += libscfg
SUBLIBDIRS += libapcfg
SUBLIBDIRS += libutility
SUBLIBDIRS += libeditapcfg

all:    librarys

librarys::
	@for i in ${SUBLIBDIRS} ;  do                                                    \
            echo "-----------------------------------------------------------------"; \
            echo -e "\033[;34m                         $$i                 \033[;0m"; \
            echo "-----------------------------------------------------------------"; \
            make -C $$i || exit 1 ;                                                   \
            make -C $$i install || exit 1 ;                                           \
	done

install:
		
clean:	
	@for i in $(SUBLIBDIRS) ; do $(MAKE) -C $$i clean || exit 1; done
	
	

#Makefile
#######################################
ifndef BUILDPATH
export TOPDIR=$(shell pwd)/../..
export BUILDPATH = $(TOPDIR)/build

include $(BUILDPATH)/rules.mk
endif
######################################

#-------------------------------------------------------#
#                       SUBLIBS                         #
#-------------------------------------------------------#
SUBLIBS  =
SUBLIBS  +=  lib
#SUBLIBS  +=  src

#-------------------------------------------------------#
#                       TARGETS                         #
#-------------------------------------------------------#

all: libs
	@echo All LIBS are OK!


libs : ${SUBLIBS}
	@for i in ${SUBLIBS} ;  do                                                    \
            echo "-----------------------------------------------------------------"; \
            echo -e "\033[;34m                         $$i                 \033[;0m"; \
            echo "-----------------------------------------------------------------"; \
            make -C $$i || exit 1 ;                                         \
        done

install:
	@for i in ${SUBLIBS} ;  do                                                    \
            echo "-----------------------------------------------------------------"; \
            echo -e "\033[;35m                         $$i                 \033[;0m"; \
            echo "-----------------------------------------------------------------"; \
            make -C $$i install || exit 1 ;                                   \
        done

clean:
	for i in ${SUBLIBS} ; do make -C $$i clean || exit 1 ; done

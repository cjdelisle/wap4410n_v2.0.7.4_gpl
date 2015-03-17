#!/bin/sh -x

#
# $1 == u-boot/tools path
# $2 == kernel tree path
# $3 == optional additions to filename
#
MKIMAGE=$1/mkimage
VMLINUX=$2/vmlinux
VMLINUXBIN=$2/arch/mips/boot/vmlinux.bin
TFTPPATH=/tftpboot/`whoami`

ENTRY=`readelf -a ${VMLINUX}|grep "Entry"|cut -d":" -f 2`
LDADDR=`readelf -a ${VMLINUX}|grep "\[ 1\]"|cut -d" " -f 26`

gzip -f ${VMLINUXBIN}

${MKIMAGE} -A mips -O linux -T kernel -C gzip \
        -a 0x${LDADDR} -e ${ENTRY} -n "Linux Kernel Image"    \
                -d ${VMLINUXBIN}.gz ${IMAGEPATH}/vmlinux$3.gz.uImage

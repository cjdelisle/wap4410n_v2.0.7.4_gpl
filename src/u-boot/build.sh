#!/bin/sh
PWD=`pwd`
export PATH=$PATH:$PWD/../toolchain/gcc-3.4.4-2.16.1/build_mips_nofpu/bin
make BOARD_TYPE=ap83fus BUILD_TYPE=jffs2

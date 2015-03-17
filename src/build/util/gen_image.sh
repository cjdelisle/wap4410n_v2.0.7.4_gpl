#!/bin/sh

echo ""
echo "**** Creating BIN ****"
mipsBind image.par linux.bin
echo ""


echo ""
echo "**** Creating Image ****"
lapbind -c lapBindcfg.txt -o linux.img
echo ""

#!/bin/sh
set -e

FW_DIR=/boot
if [ $# -gt 0 ] ; then
    FW_DIR=$1
fi
echo -e "\n>>0, firmware dir: $FW_DIR"

#1, umount
echo -e "\n>>1, cat /proc/mtd"
cat /proc/mtd

#2, flash_eraseall
echo -e "\n>>2, flash_eraseall"
flash_erase /dev/mtd0 0 0
flash_erase /dev/mtd1 0 0
flash_erase /dev/mtd2 0 0
flash_erase /dev/mtd3 0 0

#3, program BOOT.BIN
echo -e "\n>>3, program BOOT.BIN"
dd if=$FW_DIR/BOOT.BIN of=/dev/mtd0

#4, program image.ub
echo -e "\n>>4, program image.ub"
dd if=$FW_DIR/image.ub of=/dev/mtd2

#5, program boot.scr
echo -e "\n>>5, program boot.scr"
dd if=$FW_DIR/boot.scr of=/dev/mtd3

#6, done
echo -e "\n>>5, all done"

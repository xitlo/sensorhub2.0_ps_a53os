#!/bin/sh
set -e

FW_DIR=/boot
WORK_DIR=/home/root
TEMP_BIN=zero.bin

#0, para check
if [ $# -gt 0 ] ; then
    FW_DIR=$1
fi
echo -e "\n>>0, firmware dir: $FW_DIR"
date_start=$(date +%s)

#1, cat /proc/mtd
echo -e "\n>>1, cat /proc/mtd"
cat /proc/mtd

#2, create temp files
echo -e "\n>>2, create temp files"
dd if=/dev/zero of=$WORK_DIR/$TEMP_BIN bs=1M count=32
cat $FW_DIR/BOOT.BIN $WORK_DIR/$TEMP_BIN > $WORK_DIR/BOOT-Q.BIN
cat $FW_DIR/image.ub $WORK_DIR/$TEMP_BIN > $WORK_DIR/image-Q.ub
cat $FW_DIR/boot.scr $WORK_DIR/$TEMP_BIN > $WORK_DIR/boot-Q.scr
rm $WORK_DIR/$TEMP_BIN

#3, flash_eraseall
echo -e "\n>>3, flash_eraseall"
flash_erase /dev/mtd0 0 0
flash_erase /dev/mtd1 0 0
flash_erase /dev/mtd2 0 0
flash_erase /dev/mtd3 0 0

#4, program BOOT.BIN
echo -e "\n>>4, program BOOT.BIN"
dd if=$WORK_DIR/BOOT-Q.BIN of=/dev/mtd0 ibs=1 obs=4096 count=28M

#5, program image.ub
echo -e "\n>>5, program image.ub"
dd if=$WORK_DIR/image-Q.ub of=/dev/mtd2 ibs=1 obs=4096 count=28M

#6, program boot.scr
echo -e "\n>>6, program boot.scr"
dd if=$WORK_DIR/boot-Q.scr of=/dev/mtd3 ibs=1 obs=4096 count=512k

#7, done
rm $WORK_DIR/BOOT-Q.BIN
rm $WORK_DIR/image-Q.ub
rm $WORK_DIR/boot-Q.scr
echo -e "\n>>7, all done"
date_end=$(date +%s)
duration=$(($date_end-$date_start))
print_log -e "time: start/end/duration(s): $date_start/$date_end/$duration"

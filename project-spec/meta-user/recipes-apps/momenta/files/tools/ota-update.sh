#!/bin/sh
set -e

SCRIPT_VERSION=v1.3
OTA_DIR=/ldc
BOOT_DIR=/boot
BOOT_PART=/dev/mmcblk0p1
OTA_FILE_BASE_NAME=sensorhub_fw
OTA_FILE_NAME=$OTA_DIR/$OTA_FILE_BASE_NAME*.tar.gz
OTA_LOG=$OTA_DIR/ota.log

BASE_BOOT_SCR=$BOOT_DIR/boot.scr
BASE_BOOT_BIN=$BOOT_DIR/BOOT.BIN
BASE_IMAGE_UB=$BOOT_DIR/image.ub
BAK_BOOT_SCR=$BOOT_DIR/boot0001.scr
BAK_BOOT_BIN=$BOOT_DIR/BOOT0001.BIN
BAK_IMAGE_UB=$BOOT_DIR/image0001.ub
NEW_BOOT_SCR=$OTA_DIR/boot.scr
NEW_BOOT_BIN=$OTA_DIR/BOOT.BIN
NEW_IMAGE_UB=$OTA_DIR/image.ub


print_log() {
	echo $1 $2 >> $OTA_LOG
	echo $1 $2
}

# 0, create log file, only save latest 8 log file
utime=`date +%Y%m%d_%H%M%S`
touch $OTA_DIR/ota-$utime.log
ls -lt $OTA_DIR/ota-* | awk '{if(NR>8){print "rm " $9}}' | sh

rm -rf $OTA_LOG
ln -s $OTA_DIR/ota-$utime.log $OTA_LOG

echo -e "---------time: $utime---------\n" > $OTA_LOG
echo -e "---------time: $utime---------\n"

print_log -e "ota-update.sh VER: $SCRIPT_VERSION"

# 1, check ota file
print_log -e "\n>>0, check if has ota file"
if [ -f $OTA_FILE_NAME ]; then
	print_log ">>0.1, find ota file $OTA_FILE_NAME"
	tar -xvzf $OTA_FILE_NAME -C $OTA_DIR
	sync
else
	print_log ">>0.1, can't find ota file, exit"
	exit 1
fi

# 1, remount /boot to rw
print_log -e "\n>>1, check and remount $BOOT_DIR"
if [ -d $BOOT_DIR ] && [ -e $BOOT_PART ]; then
	print_log ">>1.1, remount $BOOT_DIR partition as rw"
	umount $BOOT_DIR
	mount $BOOT_PART $BOOT_DIR
else
	print_log ">>1.1, can't find $BOOT_DIR or $BOOT_PART, pls check!"
	exit 2
fi

# 2, check and update boot.scr
print_log -e "\n>>2, check and update boot.scr"
if [ -f $NEW_BOOT_SCR ] ; then
	print_log -e ">>2.1, find $NEW_BOOT_SCR"
	mv $BASE_BOOT_SCR $BAK_BOOT_SCR && sync
	print_log -e ">>2.2, bakup old $BASE_BOOT_SCR to $BAK_BOOT_SCR"
	cp -rfd $NEW_BOOT_SCR $BASE_BOOT_SCR && sync
	print_log -e ">>2.3, copy $NEW_BOOT_SCR to $BASE_BOOT_SCR"
fi

# 3, check and update BOOT.BIN
print_log -e "\n>>3 check and update BOOT.BIN"
if [ -f $NEW_BOOT_BIN ] ; then
	print_log -e ">>3.1, find $NEW_BOOT_BIN"
	mv $BASE_BOOT_BIN $BAK_BOOT_BIN && sync
	print_log -e ">>3.2, bakup old $BASE_BOOT_BIN to $BAK_BOOT_BIN"
	cp -rfd $NEW_BOOT_BIN $BASE_BOOT_BIN && sync
	print_log -e ">>3.3, copy $NEW_BOOT_BIN to $BASE_BOOT_BIN"
fi

# 4, check and update image.ub
print_log -e "\n>>4 check and update image.ub"
if [ -f $NEW_IMAGE_UB ] ; then
	print_log -e ">>4.1, find $NEW_IMAGE_UB"
	mv $BASE_IMAGE_UB $BAK_IMAGE_UB && sync
	print_log -e ">>4.2, bakup old $BASE_IMAGE_UB to $BAK_IMAGE_UB"
	cp -rfd $NEW_IMAGE_UB $BASE_IMAGE_UB && sync
	print_log -e ">>4.3, copy $NEW_IMAGE_UB to $BASE_IMAGE_UB"
fi

# 5, remount /boot to ro
print_log -e "\n>>5, check and remount $BOOT_DIR"
if [ -d $BOOT_DIR ] && [ -e $BOOT_PART ]; then
	print_log ">>5.1, remount $BOOT_DIR partition as ro"
	umount $BOOT_DIR
	mount -o ro $BOOT_PART $BOOT_DIR
else
	print_log ">>5.1, can't find $BOOT_DIR or $BOOT_PART, pls check!"
	exit 1
fi

# 6, delete ota files
print_log -e "\n>>6, delete ota files"
rm -rf $OTA_FILE_NAME $NEW_BOOT_SCR $NEW_BOOT_BIN $NEW_IMAGE_UB

print_log -e "\n>>7, all done, valid after next reboot"

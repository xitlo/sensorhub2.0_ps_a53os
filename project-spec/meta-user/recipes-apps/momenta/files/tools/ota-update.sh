#!/bin/sh
set -e

OTA_DIR=/ldc
BOOT_DIR=/boot
BASE_BOOT_SCR=$BOOT_DIR/boot.scr
BASE_BOOT_BIN=$BOOT_DIR/BOOT.BIN
BASE_IMAGE_UB=$BOOT_DIR/image.ub
BAK_BOOT_SCR=$BOOT_DIR/boot0001.scr
BAK_BOOT_BIN=$BOOT_DIR/BOOT0001.BIN
BAK_IMAGE_UB=$BOOT_DIR/image0001.ub
NEW_BOOT_SCR=$OTA_DIR/boot.scr
NEW_BOOT_BIN=$OTA_DIR/BOOT.BIN
NEW_IMAGE_UB=$OTA_DIR/image.ub


#1, check and update boot.scr
echo -e "\n>>1, check and update boot.scr"
if [ -f $NEW_BOOT_SCR ] ; then
	echo -e "\n>>1.1, find $NEW_BOOT_SCR"
	rm $BASE_BOOT_SCR && sync
	echo -e "\n>>1.2, remove old $BASE_BOOT_SCR"
	cp -rfpd $NEW_BOOT_SCR $BASE_BOOT_SCR && sync
	echo -e "\n>>1.3, copy $NEW_BOOT_SCR to $BASE_BOOT_SCR"
fi

#2, check and update BOOT.BIN
echo -e "\n>>2 check and update BOOT.BIN"
if [ -f $NEW_BOOT_BIN ] ; then
	echo -e "\n>>2.1, find $NEW_BOOT_BIN"
	rm $BASE_BOOT_BIN && sync
	echo -e "\n>>2.2, remove old $BASE_BOOT_BIN"
	cp -rfpd $NEW_BOOT_BIN $BASE_BOOT_BIN && sync
	echo -e "\n>>2.3, copy $NEW_BOOT_BIN to $BASE_BOOT_BIN"
fi

#3, check and update image.ub
echo -e "\n>>3 check and update image.ub"
if [ -f $NEW_IMAGE_UB ] ; then
	echo -e "\n>>3.1, find $NEW_IMAGE_UB"
	rm $BASE_IMAGE_UB && sync
	echo -e "\n>>3.2, remove old $BASE_IMAGE_UB"
	cp -rfpd $NEW_IMAGE_UB $BASE_IMAGE_UB && sync
	echo -e "\n>>3.3, copy $NEW_IMAGE_UB to $BASE_IAMGE_UB"
fi

echo -e "\n>>4, all done"

#!/bin/sh
set -e

SCRIPT_VERSION=v2.4
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
MD5_JSON=$OTA_DIR/shub_ota.json

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

# 1, check if exist only one ota file
print_log -e "\n>>1, check if exist only one ota file"
ota_file_num=`ls ${OTA_FILE_NAME} | wc -l`
if [ ${ota_file_num} -eq 1 ]; then
    print_log ">>1.1, find ota file $OTA_FILE_NAME"
    tar -xvzf $OTA_FILE_NAME -C $OTA_DIR && sync
else
    print_log ">>1.1, detect ${ota_file_num} ota files, should only be 1, delete and exit!"
    rm -rf ${OTA_FILE_NAME} && sync
    exit 1
fi

# 2, check ota files md5
print_log -e "\n>>2, check ota files md5"
if [ -f $MD5_JSON ]; then
    NEW_OTA_VER=`cat $MD5_JSON | jq .fw | sed 's/\"//g'`
    print_log -e ">>2.1, new ota version: $NEW_OTA_VER"
else
    print_log -e ">>2.1, can't find shub_ota.json file, exit"
    exit 1
fi

print_log -e "\n>>2.2, check md5 boot.scr"
if [ -f $NEW_BOOT_SCR ]; then
    NEW_SCR_MD5_JSON=`cat $MD5_JSON | jq .boot_scr.md5 | sed 's/\"//g'`
    NEW_SCR_MD5=`md5sum $NEW_BOOT_SCR | cut -d ' ' -f 1`
    if [ "$NEW_SCR_MD5_JSON" != "$NEW_SCR_MD5" ]; then
        print_log -e ">>2.2, ota file boot.scr broken, json/real md5: $NEW_SCR_MD5_JSON/$NEW_SCR_MD5"
        exit 1
    fi
    if [ -f $BASE_BOOT_SCR ]; then
        BASE_SCR_MD5=`md5sum $BASE_BOOT_SCR | cut -d ' ' -f 1`
    fi
fi

print_log -e "\n>>2.3, check md5 image.ub"
if [ -f $NEW_IMAGE_UB ]; then
    NEW_IMG_MD5_JSON=`cat $MD5_JSON | jq .image_ub.md5 | sed 's/\"//g'`
    NEW_IMG_MD5=`md5sum $NEW_IMAGE_UB | cut -d ' ' -f 1`
    if [ "$NEW_IMG_MD5_JSON" != "$NEW_IMG_MD5" ]; then
        print_log -e ">>2.3, ota file image.ub broken, json/real md5: $NEW_IMG_MD5_JSON/$NEW_IMG_MD5"
        exit 1
    fi
    if [ -f $BASE_IMAGE_UB ]; then
        BASE_IMG_MD5=`md5sum $BASE_IMAGE_UB | cut -d ' ' -f 1`
    fi
fi

print_log -e "\n>>2.4, check md5 BOOT.BIN"
if [ -f $NEW_BOOT_BIN ]; then
    NEW_BOOT_MD5_JSON=`cat $MD5_JSON | jq .BOOT_BIN.md5 | sed 's/\"//g'`
    NEW_BOOT_MD5=`md5sum $NEW_BOOT_BIN | cut -d ' ' -f 1`
    if [ "$NEW_BOOT_MD5_JSON" != "$NEW_BOOT_MD5" ]; then
        print_log -e ">>2.4, ota file BOOT.BIN broken, json/real md5: $NEW_BOOT_MD5_JSON/$NEW_BOOT_MD5"
        exit 1
    fi
    if [ -f $BASE_BOOT_BIN ]; then
        BASE_BOOT_MD5=`md5sum $BASE_BOOT_BIN | cut -d ' ' -f 1`
    fi
fi

# 3, remount /boot to rw
print_log -e "\n>>3, check and remount $BOOT_DIR"
if [ -d $BOOT_DIR ] && [ -e $BOOT_PART ]; then
    print_log ">>3.1, remount $BOOT_DIR partition as rw"
    umount $BOOT_DIR
    mount $BOOT_PART $BOOT_DIR
else
    print_log ">>3.1, can't find $BOOT_DIR or $BOOT_PART, pls check!"
    exit 2
fi

# 4, check and update boot.scr
print_log -e "\n>>4, check and update boot.scr"
if [ -f $NEW_BOOT_SCR ] ; then
    print_log -e ">>4.1, find new $NEW_BOOT_SCR"

    if [ "$BASE_SCR_MD5" != "$NEW_SCR_MD5" ]; then
        mv $BASE_BOOT_SCR $BAK_BOOT_SCR && sync
        print_log -e ">>4.2, bakup old $BASE_BOOT_SCR to $BAK_BOOT_SCR"
        cp -rfd $NEW_BOOT_SCR $BASE_BOOT_SCR && sync
        print_log -e ">>4.3, copy $NEW_BOOT_SCR to $BASE_BOOT_SCR"
    else
        print_log -e ">>4.2, base and new file md5 same, skip"
    fi
fi

# 5, check and update BOOT.BIN
print_log -e "\n>>5 check and update BOOT.BIN"
if [ -f $NEW_BOOT_BIN ] ; then
    print_log -e ">>5.1, find new $NEW_BOOT_BIN"

    if [ "$BASE_BOOT_MD5" != "$NEW_BOOT_MD5" ]; then
        mv $BASE_BOOT_BIN $BAK_BOOT_BIN && sync
        print_log -e ">>5.2, bakup old $BASE_BOOT_BIN to $BAK_BOOT_BIN"
        cp -rfd $NEW_BOOT_BIN $BASE_BOOT_BIN && sync
        print_log -e ">>5.3, copy $NEW_BOOT_BIN to $BASE_BOOT_BIN"
    else
        print_log -e ">>5.2, base and new file md5 same, skip"
    fi
fi

# 6, check and update image.ub
print_log -e "\n>>6 check and update image.ub"
if [ -f $NEW_IMAGE_UB ] ; then
    print_log -e ">>6.1, find new $NEW_IMAGE_UB"

    if [ "$BASE_IMG_MD5" != "$NEW_IMG_MD5" ]; then
        mv $BASE_IMAGE_UB $BAK_IMAGE_UB && sync
        print_log -e ">>6.2, bakup old $BASE_IMAGE_UB to $BAK_IMAGE_UB"
        cp -rfd $NEW_IMAGE_UB $BASE_IMAGE_UB && sync
        print_log -e ">>6.3, copy $NEW_IMAGE_UB to $BASE_IMAGE_UB"
    else
        print_log -e ">>6.2, base and new file md5 same, skip"
    fi
fi

# 7, remount /boot to ro
print_log -e "\n>>7, check and remount $BOOT_DIR"
if [ -d $BOOT_DIR ] && [ -e $BOOT_PART ]; then
    print_log ">>7.1, remount $BOOT_DIR partition as ro"
    umount $BOOT_DIR
    mount -o ro $BOOT_PART $BOOT_DIR
else
    print_log ">>7.1, can't find $BOOT_DIR or $BOOT_PART, pls check!"
    exit 1
fi

# 8, delete ota files
print_log -e "\n>>8, delete ota files"
rm -rf $OTA_FILE_NAME $NEW_BOOT_SCR $NEW_BOOT_BIN $NEW_IMAGE_UB && sync


print_log -e "\n>>10, all done, valid after next reboot"

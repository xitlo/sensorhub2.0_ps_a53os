#!/bin/sh
SCRIPT_VERSION=v2.3
set -e

run_mmc_case()
{
    # create 3 partitions
    fdisk /dev/mmcblk0 2>&1 1>/dev/null  << EOF
    p
    d
    1
    d
    2
    d
    3
    d
    n
    p
    1

    +512M
    t
    b
    n
    p
    2

    +512M
    n
    p
    3


    w
EOF
    sleep 2
    if [ "$?" = 0 ]; then
        if [ -e '/dev/mmcblk0p1' ] && [ -e '/dev/mmcblk0p2' ] && [ -e '/dev/mmcblk0p3' ]; then
            echo -e "MMC part ok! \n\n"
            return
        fi
    fi
    echo -e "MMC test fails \n\n"
}

echo -e "$0 VER: $SCRIPT_VERSION"

#0, check param
echo -e "\n>>0, check param"
if [ $# -lt 1 ] ; then
	echo -e "Usage: $0 <dir_of_firmware>"
	echo -e "\tdir_of_firmware, could be NULL if only partition"
	echo -e "\te.g. $0 /media/sd-mmcblk1p1"
    exit 1
fi

if [ "NULL" != "$1" ]; then
    PATH_BOOT_BIN=$1/BOOT.BIN
    PATH_BOOT_SCR=$1/boot.scr
    PATH_IMG_UB=$1/image.ub

    if [ ! -f $PATH_BOOT_BIN ] || [ ! -f $PATH_BOOT_SCR ] || [ ! -f $PATH_IMG_UB ]; then
        echo -e "can't find BOOT.BIN/boot.scr/image.ub, pls check!"
        exit 1
    fi
fi

#1, umount
echo -e "\n>>1, df & umount &fdisk"
df
has_mount=`df | grep mmcblk0 | head -1  | awk '{print $1}'`
while [ "" != "$has_mount" ]
do
    umount $has_mount
    has_mount=`df | grep mmcblk0 | head -1  | awk '{print $1}'`
done

fdisk -l

#2, re-partition
echo -e "\n>>2, fdisk"
run_mmc_case

#3, format partitions
sleep 1
echo -e "\n>>3, format"
has_mount=`df | grep mmcblk0 | head -1  | awk '{print $1}'`
while [ "" != "$has_mount" ]
do
    umount $has_mount
    has_mount=`df | grep mmcblk0 | head -1  | awk '{print $1}'`
done

mkfs.vfat /dev/mmcblk0p1
mkfs.ext4 -E nodiscard -F /dev/mmcblk0p2
mkfs.ext4 -E nodiscard -F /dev/mmcblk0p3

#4, remount
echo -e "\n>>4, remount"
if [ ! -d /boot ]; then
    mkdir /boot
fi

if [ ! -d /ldc ]; then
    mkdir /ldc
fi

if [ ! -d /data ]; then
    mkdir /data
fi

mount -o rw /dev/mmcblk0p1 /boot
mount -o rw /dev/mmcblk0p2 /ldc
mount -o rw /dev/mmcblk0p3 /data

#5, copy firmware to /boot
echo -e "\n>>5, check for firmware copy"
if [ "NULL" != $1 ]; then
    echo -e ">>5.1, copy firmware to /boot"
    cp -rf $PATH_BOOT_BIN /boot && sync
    cp -rf $PATH_BOOT_SCR /boot && sync
    cp -rf $PATH_IMG_UB /boot && sync
    cp -rf /boot/BOOT.BIN /boot/BOOT0002.BIN && sync
    cp -rf /boot/image.ub /boot/image0002.ub && sync
else
    echo -e ">>5.2, no path for firmware, skip"
fi

#6, done
echo -e "\n>>6, all done"

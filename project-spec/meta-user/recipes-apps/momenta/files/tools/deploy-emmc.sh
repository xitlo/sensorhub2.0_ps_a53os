#!/bin/sh
SCRIPT_VERSION=v2.1
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

#1, umount
echo -e "\n>>1, df & umount &fdisk"
df
has_mount=`df | grep mmcblk0 | head -1  | awk '{print $1}'`
while [ "" != $has_mount ]
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
while [ "" != $has_mount ]
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

#5, done
echo -e "\n>>5, all done"

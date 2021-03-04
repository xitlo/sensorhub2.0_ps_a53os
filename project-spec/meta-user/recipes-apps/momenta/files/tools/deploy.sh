#!/bin/sh
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

#1, umount
echo -e ">>1, df & umount &fdisk\n"
df
if [ -e '/dev/mmcblk0p1' ]; then
    umount /dev/mmcblk0p*
fi
fdisk -l

#2, re-partition
echo -e ">>2, fdisk\n"
run_mmc_case

#3, format partitions
sleep 1
echo -e ">>3, format\n"
if [ -e '/dev/mmcblk0p1' ]; then
    umount /dev/mmcblk0p*
fi
mkfs.vfat /dev/mmcblk0p1
mkfs.ext4 -E nodiscard -F /dev/mmcblk0p2
mkfs.ext4 -E nodiscard -F /dev/mmcblk0p3

#4, remount
echo -e ">>4, remount\n"
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
echo -e ">>5, all done\n"

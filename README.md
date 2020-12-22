# a53os

project for a53, arm, linux

# yocto prefer file
http://git.yoctoproject.org/cgit.cgi/poky/plain/meta/conf/bitbake.conf?h=blinky

# build petalinux project
- makesure petalinux tools could run, if not, source <path-to-petalinux-setting64.sh>
- enter a53os
- rm -rf build components
- petalinux-config --get-hw-description <path-to-xsa-file-dir> [--silentconfig]
	sample: petalinux-config --get-hw-description ../hw_description/ --silentconfig
- petalinux-build

# create BOOT.bin
- cd <dir-to-project>/image/linux
- petalinux-package --boot --fsbl --fpga --pmufw --u-boot

# boot with sdcard
- partition and format SD card, first partition must be FAT format
- cp BOOT.BIN boot.scr image.ub <path-to-FAT-partition-on-SD>
- switch boot DIP to SD BOOT mode(0101), and then power up


# a53os

project for a53, arm, linux

# yocto prefer file
http://git.yoctoproject.org/cgit.cgi/poky/plain/meta/conf/bitbake.conf?h=blinky

# prepare petalinux build env
- makesure petalinux tools could run, if not, source <path-to-petalinux-setting64.sh>
- edit a53os/project-spec/meta-user/conf/petalinuxbsp.conf,
  modify MACRO "DL_DIR" and "SSTATE_DIR" to the path of offline "downloads" and "sstate_aarch64"
  download link: https://www.xilinx.com/support/download/index.html/content/xilinx/en/downloadNav/embedded-design-tools/2020-1.html

# build petalinux project
- enter a53os
- rm -rf build components
- petalinux-config --get-hw-description <path-to-xsa-file-dir> [--silentconfig]
	sample: petalinux-config --get-hw-description ../hw_description/ --silentconfig
- petalinux-build

# package firmware
## manully
### create BOOT.bin
- cd <dir-to-project>/image/linux
- petalinux-package --boot --fsbl --fpga --pmufw --u-boot

### boot with sdcard
- partition and format SD card, first partition must be FAT format
- cp BOOT.BIN boot.scr image.ub <path-to-FAT-partition-on-SD>
- switch boot DIP to SD BOOT mode(0101), and then power up

## use script auto packet
- get script from https://gitlab.momenta.works/sensorhub2/ps/tools.git
- use build.sh to packet and get ota compressed file

#!/bin/bash
dir=`pwd`
echo "$dir"

uboot_dir=$dir/../../components/yocto/workspace/sources/u-boot-xlnx/oe-local-files
echo "$uboot_dir"
mkdir -p $uboot_dir

cp $dir/devtool-fragment.cfg $uboot_dir
echo "done!"


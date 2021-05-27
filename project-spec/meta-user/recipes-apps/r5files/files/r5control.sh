#!/bin/bash
R5_FIRMWARE_NAME="r5-openamp-echo.elf"

#1, check param
if [ $# -lt 1 ] ; then
    echo "Usage: $0 <turn_on_1_off_0> [r5_firmware_name]"
    echo " e.g.: $0 1"
    exit 1;
fi

if [ $# -ge 2 ] ; then
    R5_FIRMWARE_NAME=$2
fi

#2, turn on or off
if [ $1 -eq 1 ] ; then
    echo "turn on r5, firmware name: $R5_FIRMWARE_NAME"
    echo $R5_FIRMWARE_NAME > /sys/class/remoteproc/remoteproc0/firmware
    echo start > /sys/class/remoteproc/remoteproc0/state
    exit 0;
else
    echo "turn off r5"
    echo stop > /sys/class/remoteproc/remoteproc0/state
fi

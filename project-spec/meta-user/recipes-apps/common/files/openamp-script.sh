#!/bin/bash

#1, check param
if [ $# -lt 1 ] ; then
    echo "Usage: $0 [turn_on_1_off_0]"
    echo " e.g.: $0 1"
    exit 1;
fi

#2, turn on or off
if [ $1 -eq 1 ] ; then
    echo "turn on r5"
    echo sensorhub-2.elf > /sys/class/remoteproc/remoteproc0/firmware
    echo start > /sys/class/remoteproc/remoteproc0/state
    exit 0;
else
    echo "turn off r5"
    echo stop > /sys/class/remoteproc/remoteproc0/state
fi



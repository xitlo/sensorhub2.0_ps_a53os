#!/bin/bash
# echo "Usage: $0 <if_only_run_test> <debug_swtich>"

#2, check if only swtich debug
if [ $# -gt 1 ] ; then
   echo "only set timesync_debug_log $2"
   echo $2 > /sys/devices/platform/timesync/timesync_debug_log
   cat /sys/devices/platform/timesync/timesync_debug_log
   exit 0;
fi

#3, check if only run timesync-test
if [ $# -gt 0 ] && [ $1 -eq 1 ] ; then
   echo "only run timesync-test"
   timesync-app /dev/timesync
   exit 0;
fi

#4, enable timer and set time to 20210101-120000
# mem-test w 0x80050000 0x24 0x00000000
# mem-test w 0x80050000 0x04 0x5FEE9E40
# mem-test w 0x80050000 0x24 0x00010007

#5, load timesync driver
lsmod | grep timesync
if [ $? -eq 0 ]; then
    echo "remove timesync driver"
    rmmod timesync.ko
    sleep 1
fi

echo "load timesync driver"
insmod /lib/modules/5.4.0-xilinx-v2020.1/extra/timesync.ko
sleep 2

#6, run timesync-test
timesync-app /dev/timesync

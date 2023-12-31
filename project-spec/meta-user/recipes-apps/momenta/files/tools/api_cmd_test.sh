#!/bin/bash

# select cam isp channel. fpga set channel uart to a53
mem-test w 0x80040000 0 0x0001000c
# poweron for all cam
mem-test w 0x80000000 20 0xfffffff0

sleep 2

date_start=$(date +%s)

# enter failsafe mode
api_cmd -U1 0x12 noquery 0100000000000000
# check status
api_cmd -U1 0x10 max
# update full rom
api_cmd -U1 0x19 max AD

sleep 2
api_cmd -U1 0x84 max EE00000000000001 -i /data/isp-fw/c_mom_avm-i390-os20-tbd-i1920x1200p20-mipi-o1920x1200p20_fsync_slave_210312_ancVisable_add_version.rom
# exit failsafe mode
api_cmd -U1 0x12 noquery 0000000000000000

sleep 1
# poweroff all cam
mem-test w 0x80000000 20 0xffff0000
# unselect channel
mem-test w 0x80040000 0 0x00000000

date_end=$(date +%s)
duration=$(($date_end-$date_start))
print_log -e "time: start/end/duration(s): $date_start/$date_end/$duration"

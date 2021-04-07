#!/bin/bash

SCRIPT_VERSION=v1.1
CONFIG_FILE=/data/sensorhub2-config.json
SELF_TEST_ADDR=`cat $CONFIG_FILE | jq .a53.self_test_addr | sed 's/\"//g'`
SELF_TEST_PERIOD=`cat $CONFIG_FILE | jq .a53.self_test_period_s | sed 's/\"//g'`
IMU_CNT_ADDR=`cat $CONFIG_FILE | jq .a53.self_test_imu_cnt | sed 's/\"//g'`
BOOT_ADDR=`cat $CONFIG_FILE | jq .a53.self_test_boot_addr | sed 's/\"//g'`
BOOT_NORMAL_VAL=`cat $CONFIG_FILE | jq .a53.self_test_boot_normal | sed 's/\"//g'`

flag_val=0
flag_init=0
flag_while=0
imu_cnt_last=0
imu_cnt_cur=0

ProcessInit(){
	local temp

	temp=`/sbin/devmem $BOOT_ADDR`
	if [ "$temp" != "$BOOT_NORMAL_VAL" ]; then
		flag_init=$(printf "0x%02x" $(($flag_init+(1<<0))))
	fi

	echo -e "flag_init: $flag_init"
}

ProcessWhile(){
	local temp
	flag_while=0

	# 1, check R5
	temp=`cat /sys/class/remoteproc/remoteproc0/state`
	if [ "running" != "$temp" ]; then
		flag_while=$(printf "0x%02x" $(($flag_while+(1<<1))))
	fi

	# 2, check timesync-sync
	temp=`ps | grep timesync-app | grep -v grep | awk '{print $4}'`
	if [ "" = "$temp" ]; then
		flag_while=$(printf "0x%02x" $(($flag_while+(1<<2))))
	fi

	# 3, check ptp4l
	temp=`ps | grep ptp4l | grep -v grep | awk '{print $4}'`
	if [ "" = "$temp" ]; then
		flag_while=$(printf "0x%02x" $(($flag_while+(1<<3))))
	fi

	# 4, check task-data
	temp=`ps | grep task-data | grep -v grep | awk '{print $4}'`
	if [ "" = "$temp" ]; then
		flag_while=$(printf "0x%02x" $(($flag_while+(1<<4))))
	fi

	# 5, check task-state
	temp=`ps | grep task-state | grep -v grep | awk '{print $4}'`
	if [ "" = "$temp" ]; then
		flag_while=$(printf "0x%02x" $(($flag_while+(1<<5))))
	fi

	# 6, check imu cnt to verify R5 work normally
	imu_cnt_cur=`/sbin/devmem $IMU_CNT_ADDR`
	if [ "$imu_cnt_cur" = "$imu_cnt_last" ]; then
		flag_while=$(printf "0x%02x" $(($flag_while+(1<<6))))
	fi
	imu_cnt_last=$imu_cnt_cur

	# echo -e "flag_while: $flag_while"
}

# 1, Init, run once
echo -e "selftest.sh VER: $SCRIPT_VERSION"
ProcessInit

# 2, Main loop
while [ 1 ]
do
	ProcessWhile
	flag_val=$(printf "0x%08x" $((${flag_init} + ${flag_while})))
	/sbin/devmem $SELF_TEST_ADDR 32 $flag_val
	# echo -e "flag_val: $flag_val"
	sleep $SELF_TEST_PERIOD
done

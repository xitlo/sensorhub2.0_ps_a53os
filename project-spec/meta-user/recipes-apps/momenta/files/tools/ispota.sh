#!/bin/bash

SCRIPT_VERSION=v1.7
CONFIG_FILE=/data/sensorhub2-config.json
ISP_LOG_DIR=/data/bsplog
ISP_LOG=$ISP_LOG_DIR/ispota.log

print_log() {
	echo $1 $2 >> $ISP_LOG
	echo $1 $2
}

# function, isp firmware ota
# input 1: channel num, 0-15, e.g. 4
# input 2: firmware path
IspFirmwareOta(){
	local API_CMD_LOG=$(printf "$ISP_LOG_DIR/IspOta%02d.log" $1)
	local tty_port=`cat $CONFIG_FILE | jq .camera.cam$1.tty_port`

	# 1, enter failsafe mode
	print_log -e ">>>cam[$1], 1, enter failsafe mode, port[tty$tty_port]"
	api_cmd -U${tty_port} 0x12 noquery 0100000000000000 > $API_CMD_LOG
	if [ 0 -ne $? ]; then
		print_log -e ">>>cam[$1], error, exit"
		exit 1
	fi

	# 2, check status
	print_log -e ">>>cam[$1], 2, check status"
	api_cmd -U${tty_port} 0x10 max >> $API_CMD_LOG

	# 3, update full rom
	print_log -e ">>>cam[$1], 3, update full rom, need 3min!"
	api_cmd -U${tty_port} 0x19 max AD >> $API_CMD_LOG

	sleep 1
	api_cmd -U${tty_port} 0x84 max EE00000000000001 -i $2 >> $API_CMD_LOG
	if [ 0 -ne $? ]; then
		print_log -e ">>>cam[$1], error, exit"
		exit 3
	fi

	# 4, exit failsafe mode
	print_log -e ">>>cam[$1], 4, exit failsafe mode"
	api_cmd -U${tty_port} 0x12 noquery 0000000000000000 >> $API_CMD_LOG
	if [ 0 -ne $? ]; then
		print_log -e ">>>cam[$1], error, exit"
		exit 4
	fi

	# 5, done
	print_log -e ">>>cam[$1], 5, done"
}

# 1, check param
echo -e "> 1, check param"
if [ $# -lt 3 ] ; then
	echo "Usage: $0 <chans_to_ota> <rom_path_type_0> <rom_path_type_1>"
	echo -e "\t<rom_path_type_2> <rom_path_type_3> <rom_path_type_4>"
	echo "chans_to_ota, bit indicate chan, 0->0x0001, 15->0x8000"
	echo "rom_path_type_0, path to F30 rom file, can use NULL if not exist"
	echo "rom_path_type_1, path to F120 rom file, can use NULL if not exist"
	echo "rom_path_type_2, path to LF100/RF100 rom file, can use NULL if not exist"
	echo "rom_path_type_3, path to LR60/RR60/RU60/RD60 rom file, can use NULL if not exist"
	echo "rom_path_type_4, path to C197 rom file, can use NULL if not exist"
	echo "e.g. $0 0xfff0 ./xxx.rom NULL NULL NULL NULL NULL"
    exit 1
fi

# 2, create log file, only save latest 8 log file
utime=`date +%Y%m%d_%H%M%S`
touch $ISP_LOG_DIR/ispota-$utime.log
ls -lt $ISP_LOG_DIR/ispota-* | awk '{if(NR>8){print "rm " $9}}' | sh

rm -rf $ISP_LOG
ln -s $ISP_LOG_DIR/ispota-$utime.log $ISP_LOG

echo -e "---------time: $utime---------\n" > $ISP_LOG
echo -e "---------time: $utime---------\n"
print_log -e "cam_ver.sh VER: $SCRIPT_VERSION"

date_start=$(date +%s)

# 3, start
print_log -e "\n>>1, fakra powerup"
/sbin/devmem 0x80000020 32 0x0000ffff

print_log -e "\n>>2, sleep 2s, wait isp normal"
sleep 2

print_log -e "\n>>3, isp update for cam 0-15"
ota_ret=0
for((ch=0;ch<16;ch++))
do
{
	mask=$(printf "0x%02x" $((1 << $ch)))
	flag=$(printf "%d" $(($1 & $mask)))
	# echo "ch/mask/flag: $ch/$mask/$flag"

	if [ $flag -ne 0 ]; then
		# check rom type, use different rom
		rom_type=`cat $CONFIG_FILE | jq .camera.cam$ch.rom_type`
		if [ $rom_type -eq 0 ]; then
			rom_file=$2
		elif [ $rom_type -eq 1 ]; then
			rom_file=$3
		elif [ $rom_type -eq 2 ]; then
			rom_file=$4
		elif [ $rom_type -eq 3 ]; then
			rom_file=$5
		elif [ $rom_type -eq 4 ]; then
			rom_file=$6
		else
			rom_file=NULL
		fi

		# start update if file exits
		if [ -f $rom_file ]; then
			echo "cam[$ch] start update rom, type/file: $rom_type/$rom_file"
			IspFirmwareOta $ch $rom_file
			if [ 3 -eq $? ]; then
				echo "cam[$ch] update rom err, type/file: $rom_type/$rom_file"
				ota_ret=1
			fi
		else
			echo "cam[$ch] rom not find, skip!"
		fi
	fi
}&
done
wait

if [ 0 -eq $ota_ret ]; then
	print_log -e "\n>>4, fakra poweroff"
	/sbin/devmem 0x80000020 32 0xffff0000
else
	print_log -e "\n>>4, fakra not poweroff, since chan update err, pls check!!"
fi

print_log -e "\n>>5, all done!"
date_end=$(date +%s)
duration=$(($date_end-$date_start))
print_log -e "time: start/end/duration(s): $date_start/$date_end/$duration"

#!/bin/bash

SCRIPT_VERSION=v3.3
CAM_LOG_DIR=/data/bsplog
CAM_LOG=$CAM_LOG_DIR/cam.log

# tty port number for cam 0-15
TTY_PORT=(
	0	1	8	9
	10	11	12	13
	14	15	2	3
	4	5	6	7
)

# version registers start address for cam 0-15
REG_ADDR=(
	0x80024000 0x80024020 0x80024040 0x80024060
	0x80024080 0x800240a0 0x800240c0 0x800240e0
	0x80024100 0x80024120 0x80024140 0x80024160
	0x80024180 0x800241a0 0x800241c0 0x800241e0
)

print_log() {
	echo $1 $2 >> $CAM_LOG
	echo $1 $2
}

# function, read camera version
# input 1: channel num, 0-15, e.g. 4
# input 2: version register start address to set, hex, e.g. 0x800001d4
CamReadVer(){
	local id i j
	local REG0 REG1 REG2 REG3 temp
	local VER0 VER1 VER2 VER3 VERSION

	# dev_ch=$(printf "%02x" $1)
	print_log -e ">>>cam[$1], 1, read cam version, 16bytes"
	id=0
	for((i=0;i<16;i++))
	do
		# ver1=`api_cmd -U 1 0x53 1 0500000000fa000001000000 | grep "PAYLOAD\[00\:00\]" | awk '{print $2}'`
		local cmd_line=$(printf "api_cmd -U${TTY_PORT[$1]} 0x53 1 0500000000fa0000%02x000000 | grep \"PAYLOAD\\[00\\:00\\]\" | awk \'{print \$2}\'" $id)
		# print_log $cmd_line
		local ver[$i]=`eval $cmd_line`
		# print_log ${ver[$i]}

		if [ ! -n "${ver[$i]}" ] ; then
			for((j=0;j<16;j++))
			do
				ver[$j]=00
			done
			break;
		fi

		let id+=1
	done

	for((j=0;j<16;j++))
	do
		VERSION=$VERSION${ver[$j]}
	done
	print_log -e ">>>cam[$1], 2, VERSION: $VERSION"

	REG0=$2
	temp=$(($REG0 + 0x04))
	REG1=$(printf "0x%08x" $temp)
	temp=$(($REG1 + 0x04))
	REG2=$(printf "0x%08x" $temp)
	temp=$(($REG2 + 0x04))
	REG3=$(printf "0x%08x" $temp)

	# little endian convert
	VER0=0x${ver[3]}${ver[2]}${ver[1]}${ver[0]}
	VER1=0x${ver[7]}${ver[6]}${ver[5]}${ver[4]}
	VER2=0x${ver[11]}${ver[10]}${ver[9]}${ver[8]}
	VER3=0x${ver[15]}${ver[14]}${ver[13]}${ver[12]}
	
	print_log -e ">>>cam[$1], 3, set version to reg: $REG0/$REG1/$REG2/$REG3"
	devmem $REG0 32 $VER0
	devmem $REG1 32 $VER1
	devmem $REG2 32 $VER2
	devmem $REG3 32 $VER3

	print_log -e ">>>cam[$1], 4, done"
}

# 0, create log file, only save latest 8 log file
utime=`date +%Y%m%d_%H%M%S`
touch $CAM_LOG_DIR/cam-$utime.log
ls -lt $CAM_LOG_DIR/cam-* | awk '{if(NR>8){print "rm " $9}}' | sh

rm -rf $CAM_LOG
ln -s $CAM_LOG_DIR/cam-$utime.log $CAM_LOG

echo -e "---------time: $utime---------\n" > $CAM_LOG
echo -e "---------time: $utime---------\n"
print_log -e "cam_ver.sh VER: $SCRIPT_VERSION"

date_start=$(date +%s)

print_log -e "\n>>1, fakra powerup"
devmem 0x80000020 32 0x0000fff0

print_log -e "\n>>2, sleep 2s, wait isp normal"
sleep 2

print_log -e "\n>>3, read and set version for cam 4-15"
for((ch=4;ch<16;ch++))
do
{
	CamReadVer $ch ${REG_ADDR[$ch]}
}&
done
wait

print_log -e "\n>>4, fakra poweroff"
devmem 0x80000020 32 0xffff0000

print_log -e "\n>>5, all done!"
date_end=$(date +%s)
duration=$(($date_end-$date_start))
print_log -e "time: start/end/duration(s): $date_start/$date_end/$duration"

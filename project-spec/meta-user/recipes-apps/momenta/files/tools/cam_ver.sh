#!/bin/bash

SCRIPT_VERSION=v5.0
CONFIG_FILE=/data/sensorhub2-config.json
CAM_LOG_DIR=/data/bsplog
CAM_LOG=$CAM_LOG_DIR/cam.log
VER_BYTE_NUM=`cat $CONFIG_FILE | jq .camera.ver_byte_num`
CAM_FLAG_ADDR=`cat $CONFIG_FILE | jq .camera.flag_addr | sed 's/\"//g'`

print_log() {
	echo $1 $2 >> $CAM_LOG
	echo $1 $2
}

# function, read camera version
# input 1: channel num, 0-15, e.g. 4
# input 2: version register start address to set, hex, e.g. 0x800001d4
CamReadVer(){
	local id i j ret
	local REG0 REG1 REG2 REG3 REG4 REGFlag
	local VER0 VER1 VER2 VER3 VER4 VERSION

	# dev_ch=$(printf "%02x" $1)
	print_log -e ">>>cam[$1], 1, read cam version, ${VER_BYTE_NUM}bytes"
	local tty_port=`cat $CONFIG_FILE | jq .camera.cam$1.tty_port`
	#fist check isp status,wait for it is ready
	for((i=0;i<5;i++))
	do
		local cmd_line=$(printf "api_cmd -U$tty_port 0x10 max | grep \"PAYLOAD\\[00\\:03\\]\" | awk \'{print \$2}\'")
		local status=`eval $cmd_line`
		print_log -e "api_cmd -U$tty_port 0x10 max status: "${i},${status}
		if [ "${status}" == "02" ] ; then
			print_log -e "api_cmd -U$tty_port 0x10 max,ok can read cam version"
			break
		fi
		print_log -e "api_cmd 0x10 -U$tty_port max isp status is not 02,sleep 1"
		sleep 1
	done

	id=0
	ret=0
	for((i=0;i<${VER_BYTE_NUM};i++))
	do

		# ver1=`api_cmd -U 1 0x53 1 0500000000fa000001000000 | grep "PAYLOAD\[00\:00\]" | awk '{print $2}'`
		local cmd_line=$(printf "api_cmd -U$tty_port 0x53 1 0500000000fa0000%02x000000 | grep \"PAYLOAD\\[00\\:00\\]\" | awk \'{print \$2}\'" $id)
		# print_log $cmd_line
		local ver[$i]=`eval $cmd_line` #eval 读取一连串的参数，再依据参数本身的特性来执行
		# print_log ${ver[$i]}

		##如果执行一次不成功，那么就进入反复执行程序
		count=0;
		while [ ! -n "${ver[$i]}" ]
		do
			usleep 100000
			local cmd_line=$(printf "api_cmd -U$tty_port 0x53 1 0500000000fa0000%02x000000 | grep \"PAYLOAD\\[00\\:00\\]\" | awk \'{print \$2}\'" $id)
			local ver[$i]=`eval $cmd_line` #eval 读取一连串的参数，再依据参数本身的特性来执行
			count=`expr $count + 1`
			echo "count:$count"
			if [ $count == 5 ]; then
				break
			fi
		done

		#反复执行5次还是不成功，版本号全置为0
		if [ ! -n "${ver[$i]}" ] ; then
			for((j=0;j<${VER_BYTE_NUM};j++))
			do
				ver[$j]=00
			done
			ret=1
			break;
		fi

		let id+=1
	done

	for((j=0;j<${VER_BYTE_NUM};j++))
	do
		VERSION=$VERSION${ver[$j]} #把每一步读到的ver拼接起来
	done
	print_log -e ">>>cam[$1], 2, VERSION: $VERSION"

	REG0=$2
	REG1=$(printf "0x%08x" $(($2 + 0x04)))
	REG2=$(printf "0x%08x" $(($2 + 0x08)))
	REG3=$(printf "0x%08x" $(($2 + 0x0C)))
	REG4=$(printf "0x%08x" $(($2 + 0x10)))
	REGFlag=$(printf "0x%08x" $(($2 + 0x1F)))

	# little endian convert
	VER0=0x${ver[3]}${ver[2]}${ver[1]}${ver[0]}
	VER1=0x${ver[7]}${ver[6]}${ver[5]}${ver[4]}
	VER2=0x${ver[11]}${ver[10]}${ver[9]}${ver[8]}
	VER3=0x${ver[15]}${ver[14]}${ver[13]}${ver[12]}
	VER4=0x0000${ver[17]}${ver[16]}
	
	print_log -e ">>>cam[$1], 3, set version to reg: $REG0/$REG1/$REG2/$REG3/$REG4"
	/sbin/devmem $REG0 32 $VER0
	/sbin/devmem $REG1 32 $VER1
	/sbin/devmem $REG2 32 $VER2
	/sbin/devmem $REG3 32 $VER3
	/sbin/devmem $REG4 32 $VER4

	print_log -e ">>>cam[$1], 4, set ret[$ret] to $REGFlag"
	/sbin/devmem $REGFlag 8 $ret

	print_log -e ">>>cam[$1], 5, done"
}

# 0, create log file, only save latest 8 log file
utime=`date +%Y%m%d_%H%M%S`
mkdir -p $CAM_LOG_DIR
touch $CAM_LOG_DIR/cam-$utime.log
ls -lt $CAM_LOG_DIR/cam-* | awk '{if(NR>8){print "rm " $9}}' | sh

rm -rf $CAM_LOG
ln -s $CAM_LOG_DIR/cam-$utime.log $CAM_LOG

echo -e "---------time: $utime---------\n" > $CAM_LOG
echo -e "---------time: $utime---------\n"
print_log -e "cam_ver.sh VER: $SCRIPT_VERSION"

date_start=$(date +%s)

print_log -e "\n>>1, fakra powerup"
/sbin/devmem 0x80000020 32 0x0000ffff

#isp上电延时5秒再读版本号
print_log -e "\n>>2, sleep 5s, wait isp normal"
sleep 5

print_log -e "\n>>3, read and set version for cam 0-15"
for((ch=0;ch<16;ch++))
do
{
	ch_en[$ch]=`cat $CONFIG_FILE | jq .camera.cam$ch.enable`
	ver_addr[$ch]=`cat $CONFIG_FILE | jq .camera.cam$ch.ver_addr | sed 's/\"//g'`
}
done

for((ch=0;ch<16;ch++))
do
{
	# ch_en[$ch]=`cat $CONFIG_FILE | jq .camera.cam$ch.enable`
	if [ 0 -ne ${ch_en[$ch]} ]; then
		CamReadVer $ch ${ver_addr[$ch]}
	fi
}&
done
wait

print_log -e "\n>>4, fakra poweroff"
/sbin/devmem 0x80000020 32 0xffff0000

cam_flag_val=0
for((ch=0;ch<16;ch++))
do
{
	if [ 0 -ne ${ch_en[$ch]} ]; then
		reg_flag=$(printf "0x%08x" $((${ver_addr[$ch]} + 0x1F)))
		err_flag=$(printf "%d" $(/sbin/devmem $reg_flag 8))
		# echo "$ch, $reg_flag, $err_flag"
		if [ 0 -ne $err_flag ]; then
			cam_flag_val=$(printf "0x%02x" $(($cam_flag_val+(1<<$ch))))
		fi
	fi
}
done
/sbin/devmem $CAM_FLAG_ADDR 32 $cam_flag_val
print_log -e "\n>>5, set cam_flag_val $cam_flag_val to $CAM_FLAG_ADDR"

print_log -e "\n>>6, all done!"
date_end=$(date +%s)
duration=$(($date_end-$date_start))
print_log -e "time: start/end/duration(s): $date_start/$date_end/$duration"

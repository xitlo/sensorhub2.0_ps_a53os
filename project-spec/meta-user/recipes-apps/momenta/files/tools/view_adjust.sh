#-----------------------#
#函数名：视野光心调整-A53
#Camera_Init()
#Optical_offset()
#read_Camera()
#Write_flash()
#-----------------------#

#!/bin/bash

SCRIPT_VERSION=v1.1

CONFIG_FILE=/data/sensorhub2-config.json
CAM_LOG_DIR=/data/bsplog
CAM_LOG=$CAM_LOG_DIR/view_adjust.log

channel=$2 #只给imx390做，即cam[6]-cam[15] 每次对选定的一路相机操作
tty_port=`cat $CONFIG_FILE | jq .camera.cam${channel}.tty_port`

JBF_NAME=/home/root/table_0x140000_sys_sdk100.jbf

print_log() {
	echo $1 $2 >> $CAM_LOG
	echo $1 $2
}
	
# function: Offset_To_String
# input: $x_offset $y_offset
# output:
Offset_To_String() {
	local API_CMD_LOG=$(printf "$CAM_LOG_DIR/view_adjust%02d.log" $channel)
	## 0<=x<15  x=0-15
	if [ $1 -ge 0 ] && [ $1 -lt 16 ];then
		## 0<=y<15  y=0-15 
		if [ $2 -ge 0 ] && [ $2 -lt 16 ];then
			x_temp=`printf %x $1`
			y_temp=`printf %x $2`
			api_cmd -U$tty_port 0xa3 max 0${x_temp}0000000${y_temp}000000 > $API_CMD_LOG
		## y>=16
		elif [ $2 -ge 16 ];then
			x_temp=`printf %x $1`
			y_temp=`printf %x $2`
			api_cmd -U$tty_port 0xa3 max 0${x_temp}000000${y_temp}000000 > $API_CMD_LOG
		## y<0
		else
			x_temp=`printf %x $1`
			y_temp=`printf %x $2`
			y_temp=`echo $y_temp | cut -b 15-16`
			api_cmd -U$tty_port 0xa3 max 0${x_temp}000000${y_temp}ffffff > $API_CMD_LOG
		fi
	## x>=16 
	elif [ $1 -ge 16 ];then
		if [ $2 -ge 0 ] && [ $2 -lt 16 ];then
			x_temp=`printf %x $1`
			y_temp=`printf %x $2`
			api_cmd -U$tty_port 0xa3 max ${x_temp}0000000${y_temp}000000 > $API_CMD_LOG
		elif [ $2 -ge 16 ];then
			x_temp=`printf %x $1`
			y_temp=`printf %x $2`
			api_cmd -U$tty_port 0xa3 max ${x_temp}000000${y_temp}000000 > $API_CMD_LOG
		else
			x_temp=`printf %x $1`
			y_temp=`printf %x $2`
			y_temp=`echo $y_temp | cut -b 15-16`
			api_cmd -U$tty_port 0xa3 max ${x_temp}000000${y_temp}ffffff > $API_CMD_LOG
		fi
	## x<0	
	else
		if [ $2 -ge 0 ] && [ $2 -lt 16 ];then
			x_temp=`printf %x $1`
			x_temp=`echo $x_temp | cut -b 15-16`
			y_temp=`printf %x $2`
			api_cmd -U$tty_port 0xa3 max ${x_temp}ffffff0${y_temp}000000 > $API_CMD_LOG
		elif [ $2 -ge 16 ];then
			x_temp=`printf %x $1`
			x_temp=`echo $x_temp | cut -b 15-16`
			y_temp=`printf %x $2`
			api_cmd -U$tty_port 0xa3 max ${x_temp}ffffff${y_temp}000000 > $API_CMD_LOG
		else
			x_temp=`printf %x $1`
			x_temp=`echo $x_temp | cut -b 15-16`
			y_temp=`printf %x $2`
			y_temp=`echo $y_temp | cut -b 15-16`
			api_cmd -U$tty_port 0xa3 max ${x_temp}ffffff${y_temp}ffffff > $API_CMD_LOG
		fi
	fi
	VAR=`cat $API_CMD_LOG`
	echo "$VAR" #调试用
	CommandStatus=`cat $API_CMD_LOG | grep Done`
	if [ "$CommandStatus" = "" ] ; then
		echo -e "\e[1;31m====cam[$channel]tty_port[$tty_port],view change ===error \e[0m"
	else
		echo -e "\e[1;32m====cam[$channel]tty_port[$tty_port],view change ===ok \e[0m"
	fi
}

# function: 相机初始化
# input:
# output:
Camera_Init() {
	local API_CMD_LOG=$(printf "$CAM_LOG_DIR/view_adjust%02d.log" $channel)
	api_cmd -U${tty_port} 0x10 max > $API_CMD_LOG
    VAR=`cat $API_CMD_LOG`
	#echo "$VAR" #调试用
	CommandStatus=`cat $API_CMD_LOG | grep Done`
	if [ "$CommandStatus" = "" ] ; then
		echo -e "\e[1;31m===cam[$channel],camera init ===error \e[0m"
	else
		echo -e "\e[1;32m===cam[$channel],camera init ===ok \e[0m"
	fi
}

# function: 从相机读取table_0x140000.jbf
# input:
# output: table_0x140000.jbf
Read_Camera() {
    rm -r `pwd`/*.jbf
	rm -r `pwd`/*.json
	local API_CMD_LOG=$(printf "$CAM_LOG_DIR/view_adjust%02d.log" $channel)
	api_cmd -U$tty_port 0x91 max 0000140001 -o table_0x140000.jbf > $API_CMD_LOG
	VAR=`cat $API_CMD_LOG`
	#echo "$VAR" #调试用
	CommandStatus=`cat $API_CMD_LOG | grep Done`
	if [ "$CommandStatus" = "" ] ; then
		echo -e "\e[1;31m===cam[$channel]tty_port[$tty_port],read 'table_0x140000.jbf' from camera ===error \e[0m"
	else
		echo -e "\e[1;32m===cam[$channel]tty_port[$tty_port],read 'table_0x140000.jbf' from camera ===ok \e[0m"
	fi
}

# function: 读回设置值
# input:
# output:
Check_Set() {
	local API_CMD_LOG=$(printf "$CAM_LOG_DIR/view_adjust%02d.log" $channel)
	echo "-------------------"
	echo "power off"
	/sbin/devmem 0x80000020 32 0xffff0000
	sleep 1
	echo "power on"
	/sbin/devmem 0x80000020 32 0x0000ffff
	sleep 2
	api_cmd -U${tty_port} 0xa2 max > $API_CMD_LOG
	VAR=`cat $API_CMD_LOG`
	echo "$VAR"
}

# function: 将table_0x140000_sys_sdk100.jbf写入相机
# input: $JBF_NAME
# output:
Write_flash() {
	local API_CMD_LOG=$(printf "$CAM_LOG_DIR/view_adjust%02d.log" $channel)
	api_cmd -U${tty_port} 0x19 max AD > $API_CMD_LOG
	VAR=`cat $API_CMD_LOG`
	#echo "$VAR"
	CommandStatus=`cat $API_CMD_LOG | grep Done`
	if [ "$CommandStatus" = "" ] ; then
		echo -e "===8.1 this error can ignore==="
	else
		echo -e "\e[1;32m===cam[$channel]tty_port[$tty_port],8.1 ===ok \e[0m"
	fi
	sleep 1
	BYTE_LENGTH=`wc -c $JBF_NAME`
	BYTE_LENGTH=`echo $BYTE_LENGTH | awk '{print $1}'`
	BYTE_LENGTH=`printf %x $BYTE_LENGTH`
	if [ ${#BYTE_LENGTH} == 3 ]; then
		HIGHBYTE=`echo $BYTE_LENGTH | cut -b 2-3`
		LOWBYTE=`echo $BYTE_LENGTH | cut -b 1`
		LOWBYTE=0$LOWBYTE
	else
		HIGHBYTE=`echo $BYTE_LENGTH | cut -b 3-4`
		LOWBYTE=`echo $BYTE_LENGTH | cut -b 1-2`
	fi
	api_cmd -U${tty_port} 0x90 max 0000140001003502${HIGHBYTE}${LOWBYTE}0000 -i $JBF_NAME > $API_CMD_LOG
	VAR=`cat $API_CMD_LOG`
	#echo "$VAR"
	CommandStatus=`cat $API_CMD_LOG | grep Done`
	if [ "$CommandStatus" = "" ] ; then
		echo -e "\e[1;31m===cam[$channel]tty_port[$tty_port], Write_flash ===error \e[0m"
	else
		echo -e "\e[1;32m===cam[$channel]tty_port[$tty_port], Write_flash ===ok \e[0m"
		Check_Set
	fi
}

#--------------------------------main-----------------------------------#
#./view_adjust 0 x y
if [ $1 -eq 0 ] ; then
	Offset_To_String $3 $4
#./view_adjust 1/2
else
	#0, create log file, only save latest 8 log file
	utime=`date +%Y%m%d_%H%M%S`
	mkdir -p $CAM_LOG_DIR
	touch $CAM_LOG_DIR/view_adjust-$utime.log
	ls -lt $CAM_LOG_DIR/view_adjust-* | awk '{if(NR>8){print "rm " $9}}' | sh

	rm -rf $CAM_LOG
	ln -s $CAM_LOG_DIR/view_adjust-$utime.log $CAM_LOG

	echo -e "---------time: $utime---------\n" > $CAM_LOG

	print_log -e "\e[1;33m>>view_adjust.sh VER: $SCRIPT_VERSION \e[0m"
	echo -e "---------time: $utime---------"
	date_start=$(date +%s)

	#1, fakra powerup
	print_log -e "\e[1;33m\n>>1, fakra powerup<<\e[0m"
	/sbin/devmem 0x80000020 32 0x0000ffff

	print_log -e "\e[1;33m\n>>2, sleep 2s, wait isp normal<<\e[0m"
	sleep 2

	#2, do something for cam 0-15
	print_log -e "\e[1;33m\n>>3, do something for cam 6-15<<\e[0m"

	if [ $1 -eq 1 ]; then
		print_log -e "\e[1;33m\n========camera_init========\e[0m"
		Camera_Init

		print_log -e "\e[1;33m\n========read camera========\e[0m"
		Read_Camera

	elif [ $1 -eq 2 ]; then
		print_log -e "\e[1;33m\n========write jbf to flash========\e[0m"
		Write_flash
	fi

	#4, fakra poweroff
	print_log -e "\e[1;33m\n>>4, fakra poweroff\e[0m"
	/sbin/devmem 0x80000020 32 0xffff0000

	#5，显示一下时间
	print_log -e "\e[1;33m\n>>5, all done! Bye~\e[0m"
	date_end=$(date +%s)
	duration=$(($date_end-$date_start))
	print_log -e "\e[1;33m\n=time: start/end/duration(s): $date_start/$date_end/$duration=\e[0m"
fi
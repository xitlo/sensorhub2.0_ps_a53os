#!/bin/bash

SCRIPT_VERSION=v1.10 #old:v1.8
CONFIG_FILE=/data/sensorhub2-config.json
ISP_LOG_DIR=/data/bsplog
ISP_LOG=$ISP_LOG_DIR/isptableota.log

SET="IMX390"
#IMX="IMX390" #表示390 IMX=490表示490
IMX=  
VAL_EXIT=0
ROM_DIR=/home/root

#--------------------------------  function   --------------------------------#
print_log() {
	echo $1 $2 >> $ISP_LOG
	echo $1 $2
}

# function, isp firmware ota
# input 1: channel num, 0-15, e.g. 4
IspFirmwareOta_Table(){
	local API_CMD_LOG=$(printf "$ISP_LOG_DIR/IspOta%02d.log" $1)
	local tty_port=`cat $CONFIG_FILE | jq .camera.cam$1.tty_port` #$CONFIG_FILE=/data/sensorhub2-config.json

	##升级imx390和imx490的指令不同，所以区分
	##当$rom_type=0或1时，表示imx490相机，执行一版指令
	##当$rom_type=2或3或4时，表示imx390相机，执行一版指令
	
	##1.enter failsafe mode
	print_log -e ">>>>>>>>>>>>>>cam[$1], 1.enter failsafe mode, port[tty$tty_port]<<<<<<<<<<<<<111"
	api_cmd -U${tty_port} 0x12 noquery 0100000000000000 > $API_CMD_LOG
	VAR=`cat $API_CMD_LOG`
	#echo "$VAR" #调试用
	CommandStatus=`cat $API_CMD_LOG | grep success`
	if [ "$CommandStatus" = "" ] ; then
		echo -e "\e[1;31m======cam[$1],1.enter failsafe mode error 1, \e[0m"
		VAL_EXIT=1
	else
		echo -e "\e[1;32m======cam[$1],1.enter failsafe mode====111ok \e[0m"
	fi

	##2.app.bin
	print_log -e ">>>>>>>>>>>>>>cam[$1], 2.update app.bin, port[tty$tty_port]<<<<<<<<<<<<<<222"
	api_cmd -U${tty_port} 0x19 max AD > $API_CMD_LOG
	VAR=`cat $API_CMD_LOG`
	#echo "$VAR"
	CommandStatus=`cat $API_CMD_LOG | grep Done`
	if [ "$CommandStatus" = "" ] ; then
		echo -e "\e[1;31m=cam[$1],error 2, \e[0m"
		echo -e "this error can ignore,this step is slow,pls wait... " 
	else
		echo -e "\e[1;32m=cam[$1],ok APP_NAME：$APP_NAME\e[0m"
		echo -e "this step is slow,pls wait... " #这一步等待时间较久，给与提示
	fi
	sleep 1
	api_cmd -U${tty_port} 0x84 max 0300000000000000 -i $APP_NAME > $API_CMD_LOG
	VAR=`cat $API_CMD_LOG`
	#echo "$VAR"  #太长了
	CommandStatus=`cat $API_CMD_LOG | grep Done`
	if [ "$CommandStatus" = "" ] ; then
		echo -e "\e[1;31m====cam[$1],2.update app.bin error 2, \e[0m"
		VAL_EXIT=2
	else
		echo -e "\e[1;32m======cam[$1],2.update app.bin====222ok \e[0m"
	fi

	##3.syscfg=table_0x140000.jbf
	print_log -e ">>>>>>>>>>>>>>cam[$1], 3.update syscfg-table_0x140000.jbf, port[tty$tty_port]<<<<<<<<<<<<<333"
	api_cmd -U${tty_port} 0x19 max AD > $API_CMD_LOG
	#指令相同，暂不检测
	sleep 1
	BYTE_LENGTH=`wc -c $SYSCFG_NAME`
	BYTE_LENGTH=`echo $BYTE_LENGTH | awk '{print $1}'`
	BYTE_LENGTH=`printf %x $BYTE_LENGTH`
	if [ ${#BYTE_LENGTH} == 3 ]; then #如果转为16进制只有三位，就补一位0
		HIGHBYTE=`echo $BYTE_LENGTH | cut -b 2-3`
		LOWBYTE=`echo $BYTE_LENGTH | cut -b 1`
		LOWBYTE=0$LOWBYTE
	else
		HIGHBYTE=`echo $BYTE_LENGTH | cut -b 3-4`
		LOWBYTE=`echo $BYTE_LENGTH | cut -b 1-2`
	fi	
	api_cmd -U${tty_port} 0x90 0 0000140001003502${HIGHBYTE}${LOWBYTE}0000 -i $SYSCFG_NAME > $API_CMD_LOG	
	VAR=`cat $API_CMD_LOG`
	#echo "$VAR" #调试用
	CommandStatus=`cat $API_CMD_LOG | grep Done`
	if [ "$CommandStatus" = "" ] ; then
		echo -e "\e[1;31m====cam[$1],3.update syscfg-table_0x140000.jbf error 3, \e[0m"
		VAL_EXIT=3
	else
		echo -e "\e[1;32m====cam[$1],3.update syscfg-table_0x140000.jbf====333ok \e[0m"
	fi

	##4.warpcfg=table_0x148000.jbf
	print_log -e ">>>>>>>>>>>>>>cam[$1], 4.update warpcfg=table_0x148000.jbf, port[tty$tty_port]<<<<<<<<<<<<<444"
	api_cmd -U${tty_port} 0x19 max AD > $API_CMD_LOG
	sleep 1
	BYTE_LENGTH=`wc -c $WARPCFG_NAME`
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
	echo "WARPCFG_NAME:$WARPCFG_NAME"
	api_cmd -U${tty_port} 0x90 0 0080140001003503${HIGHBYTE}${LOWBYTE}0000 -i $WARPCFG_NAME > $API_CMD_LOG
	VAR=`cat $API_CMD_LOG`
	#echo "$VAR"
	CommandStatus=`cat $API_CMD_LOG | grep Done`
	if [ "$CommandStatus" = "" ] ; then
		echo -e "\e[1;31m====cam[$1],4.update warpcfg=table_0x148000.jbf error 4, \e[0m"
		VAL_EXIT=4
	else
		echo -e "\e[1;32m====cam[$1],4.update warpcfg=table_0x148000.jbf====444ok \e[0m"
	fi

	##5.update ispcfg=table_0x150000.jbf
	print_log -e ">>>>>>>>>>>>>>cam[$1], 5.update ispcfg=table_0x150000.jbf, port[tty$tty_port]<<<<<<<<<<<<<555"
	api_cmd -U${tty_port} 0x19 max AD > $API_CMD_LOG
	sleep 1
	BYTE_LENGTH=`wc -c $ISPCFG_NAME`
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
	echo "ISPCFG_NAME:$ISPCFG_NAME"
	api_cmd -U${tty_port} 0x90 0 0000150001003505${HIGHBYTE}${LOWBYTE}0000 -i $ISPCFG_NAME > $API_CMD_LOG
	VAR=`cat $API_CMD_LOG`
	#echo "$VAR"
	CommandStatus=`cat $API_CMD_LOG | grep Done`
	if [ "$CommandStatus" = "" ] ; then
		echo -e "\e[1;31m====cam[$1],5.update ispcfg=table_0x150000.jbf error 5, \e[0m"
		VAL_EXIT=5
	else
		echo -e "\e[1;32m====cam[$1],5.update ispcfg=table_0x150000.jbf====555ok \e[0m"
	fi

	##6.update geocfg=table_0x170000.jbf
	print_log -e ">>>>>>>>>>>>>>cam[$1], 6.update geocfg=table_0x170000.jbf, port[tty$tty_port]<<<<<<<<<<<<<666"
	api_cmd -U${tty_port} 0x19 max AD > $API_CMD_LOG
	sleep 1
	BYTE_LENGTH=`wc -c $GEOCFG_NAME`
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
	echo "GEOCFG_NAME:$GEOCFG_NAME"
	api_cmd -U${tty_port} 0x90 0 0000170001003507${HIGHBYTE}${LOWBYTE}0000 -i $GEOCFG_NAME > $API_CMD_LOG
	VAR=`cat $API_CMD_LOG`
	#echo "$VAR"
	CommandStatus=`cat $API_CMD_LOG | grep Done`
	if [ "$CommandStatus" = "" ] ; then
		echo -e "\e[1;31m====cam[$1],6.update geocfg=table_0x170000.jbf error 6, \e[0m"
		VAL_EXIT=6
	else
		echo -e "\e[1;32m====cam[$1],6.update geocfg=table_0x170000.jbf====666ok \e[0m"
	fi

	#-----------imx390为ssm48.dat  imx490为ssm48.bin---------------------------#
	#----------------从此处开始调用文件名有区别---------------#
	if [ $IMX = $SET ]; then
		##7.update ssm47=table_ssm47.bin
		print_log -e ">>>>>>>>>>>>>>imx390>cam[$1], 7.update ssm47=table_ssm47.bin, port[tty$tty_port]<<<<<<<<<<<<<777"
		api_cmd -U${tty_port} 0x19 max AD > $API_CMD_LOG
		sleep 1
		BYTE_LENGTH=`wc -c $SSM47_NAME`
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
		echo "SSM47_NAME:$SSM47_NAME"
		api_cmd -U${tty_port} 0x90 0 2F00000000003601${HIGHBYTE}${LOWBYTE}0000 -i $SSM47_NAME > $API_CMD_LOG
		VAR=`cat $API_CMD_LOG`
		##echo "$VAR"
		CommandStatus=`cat $API_CMD_LOG | grep Done`
		if [ "$CommandStatus" = "" ] ; then
			echo -e "\e[1;31m==390==cam[$1],7.update ssm47=table_ssm47.bin error 7 \e[0m"
			VAL_EXIT=7
		else
			echo -e "\e[1;32m==390==cam[$1],7.update ssm47=table_ssm47.bin==390==777ok \e[0m"
		fi

		##8.upgrade ssm48=table_ssm48.dat
		print_log -e ">>>>>>>>>>>>>>imx390>cam[$1], 8-end.update ssm48=table_ssm48.dat, port[tty$tty_port]<<<<<<<<<<<<<8-end"
		api_cmd -U${tty_port} 0x19 max AD > $API_CMD_LOG
		sleep 1
		BYTE_LENGTH=`wc -c $SSM48_NAME_DAT`
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
		echo "SSM48_NAME_DAT:$SSM48_NAME_DAT"
		api_cmd -U${tty_port} 0x90 0 3000000000003701${HIGHBYTE}${LOWBYTE}0000 -i $SSM48_NAME_DAT > $API_CMD_LOG
		VAR=`cat $API_CMD_LOG`
		#echo "$VAR"
		CommandStatus=`cat $API_CMD_LOG | grep Done`
		if [ "$CommandStatus" = "" ] ; then
			echo -e "\e[1;31m==390==cam[$1],8-end.update ssm48=table_ssm48.dat error 8 \e[0m"
			VAL_EXIT=8
		else
			echo -e "\e[1;32m==390==cam[$1],8-end.update ssm48=table_ssm48.dat==390==8end ok \e[0m"
		fi
	
	#--------------------------imx490-----------------------------#
	else  
		##7.upgrade table_ssm44.bin
		print_log -e ">>>>>>>>>>imx490>cam[$1], 7.update table_ssm44.bin, port[tty$tty_port]<<<<<<<<<<<<<777"
		api_cmd -U${tty_port} 0x19 max AD > $API_CMD_LOG
		sleep 1
		BYTE_LENGTH=`wc -c $SSM44_NAME`
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
		echo "SSM44_NAME:$SSM44_NAME"
		api_cmd -U${tty_port} 0x90 0 2C00000000003601${HIGHBYTE}${LOWBYTE}0000 -i $SSM44_NAME > $API_CMD_LOG
		VAR=`cat $API_CMD_LOG`
		#echo "$VAR"
		CommandStatus=`cat $API_CMD_LOG | grep Done`
		if [ "$CommandStatus" = "" ] ; then
			echo -e "\e[1;31m==490==cam[$1],7.update table_ssm44.bin error 9, \e[0m"
			VAL_EXIT=9
		else
			echo -e "\e[1;32m==490==cam[$1],7.update table_ssm44.bin==490==777ok \e[0m"
		fi
		
		##8.table_ssm45.bin
		print_log -e ">>>>>>>>>>imx490>cam[$1], 8.update table_ssm45.bin, port[tty$tty_port]<<<<<<<<<<<<<888"
		api_cmd -U${tty_port} 0x19 max AD > $API_CMD_LOG
		sleep 1
		BYTE_LENGTH=`wc -c $SSM45_NAME`
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
		api_cmd -U${tty_port} 0x90 0 2D00000000003601${HIGHBYTE}${LOWBYTE}0000 -i $SSM45_NAME > $API_CMD_LOG
		VAR=`cat $API_CMD_LOG`
		#echo "$VAR"
		CommandStatus=`cat $API_CMD_LOG | grep Done`
		if [ "$CommandStatus" = "" ] ; then
			echo -e "\e[1;31m==490==cam[$1],8.update table_ssm45.bin error 10, \e[0m"
			VAL_EXIT=10
		else
			echo -e "\e[1;32m==490==cam[$1],8.update table_ssm45.bin==490==888ok \e[0m"
		fi

		##9.table_ssm46.dat
		print_log -e ">>>>>>>>>>imx490>cam[$1], 9.update table_ssm46.dat, port[tty$tty_port]<<<<<<<<<<<<<999"
		api_cmd -U${tty_port} 0x19 max AD > $API_CMD_LOG
		sleep 1
		BYTE_LENGTH=`wc -c $SSM46_NAME`
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
		api_cmd -U${tty_port} 0x90 0 2E00000000003701${HIGHBYTE}${LOWBYTE}0000 -i $SSM46_NAME > $API_CMD_LOG
		VAR=`cat $API_CMD_LOG`
		##echo "$VAR"
		CommandStatus=`cat $API_CMD_LOG | grep Done`
		if [ "$CommandStatus" = "" ] ; then
			echo -e "\e[1;31m==490==cam[$1],9.update table_ssm46.dat error 11, \e[0m"
			VAL_EXIT=11
		else
			echo -e "\e[1;32m==490==cam[$1],9.update table_ssm46.dat==490==999ok \e[0m"
		fi

		##10.table_ssm47.bin
		print_log -e ">>>>>>>>>>imx490>cam[$1], 10.update table_ssm47.bin, port[tty$tty_port]<<<<<<<<<<<<<10"
		api_cmd -U${tty_port} 0x19 max AD > $API_CMD_LOG
		sleep 1
		BYTE_LENGTH=`wc -c $SSM47_NAME`
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
		api_cmd -U${tty_port} 0x90 0 2F00000000003601${HIGHBYTE}${LOWBYTE}0000 -i $SSM47_NAME > $API_CMD_LOG
		VAR=`cat $API_CMD_LOG`
		##echo "$VAR"
		CommandStatus=`cat $API_CMD_LOG | grep Done`
		if [ "$CommandStatus" = "" ] ; then
			echo -e "\e[1;31m==490==cam[$1],10.update table_ssm47.bin error 12, \e[0m"
			VAL_EXIT=12
		else
			echo -e "\e[1;32m==490==cam[$1],10.update table_ssm47.bin==490==10ok \e[0m"
		fi

		##11.table_ssm48.bin
		print_log -e ">>>>>>>>>>imx490>cam[$1], 11-end.update table_ssm48.bin, port[tty$tty_port]<<<<<<<<<<<<<11-end"
		api_cmd -U${tty_port} 0x19 max AD > $API_CMD_LOG
		sleep 1
		BYTE_LENGTH=`wc -c $SSM48_NAME_BIN`
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
		api_cmd -U${tty_port} 0x90 0 3000000000003601${HIGHBYTE}${LOWBYTE}0000 -i $SSM48_NAME_BIN > $API_CMD_LOG
		VAR=`cat $API_CMD_LOG`
		#echo "$VAR"
		CommandStatus=`cat $API_CMD_LOG | grep Done`
		if [ "$CommandStatus" = "" ] ; then
			echo -e "\e[1;31m==490==cam[$1], 11-end.update table_ssm48.bin error 13, \e[0m"
			VAL_EXIT=13
		else
			echo -e "\e[1;32m==490==cam[$1], 11-end.update table_ssm48.bin==490==11-end ok \e[0m"
		fi
	fi

	# , done
	#有错误，打印最后错误。正常,打印done
	if [ "$VAL_EXIT" != "0" ] ; then
		echo -e "\e[1;31m----IspFirmwareOta_Table error:$VAL_EXIT---- \e[0m"
	else
		print_log -e ">>>cam[$1], end.IspFirmwareOta_Table done"
	fi
}

#--------------------------------  start   --------------------------------#
# 1, check param
echo -e ">------------------------1, check param------------------------<"
if [ $# -lt 1 ] ; then
	echo "!!!pls set chans_to_ota, bit indicate chan, 0->0x0001, 15->0x8000"
	echo "!!!e.g.< $0 0xfff0 > "
    exit 1
fi

# 2, create log file, only save latest 8 log file
echo -e ">------------------------2, create log file------------------------<"
utime=`date +%Y%m%d_%H%M%S`
touch $ISP_LOG_DIR/isptableota-$utime.log
ls -lt $ISP_LOG_DIR/isptableota-* | awk '{if(NR>8){print "rm " $9}}' | sh

rm -rf $ISP_LOG
ln -s $ISP_LOG_DIR/isptableota-$utime.log $ISP_LOG

echo -e "---------time: $utime---------\n" > $ISP_LOG
echo -e "---------time: $utime---------\n"
print_log -e "cam_ver.sh VER: $SCRIPT_VERSION"

date_start=$(date +%s)

# 3, start
echo -e ">------------------------3, start------------------------<"
print_log -e "\n>-----1, fakra powerup-----<" 
/sbin/devmem 0x80000020 32 0x0000ffff    #上电的指令

print_log -e "\n>-----2, sleep 2s, wait isp normal-----<"
sleep 2

print_log -e "\n>-----3, isp update for cam 0-15-----<"
ota_ret=0
for((ch=0;ch<16;ch++))
do
{
	mask=$(printf "0x%02x" $((1 << $ch)))
	flag=$(printf "%d" $(($1 & $mask)))
	##echo "ch/mask/flag: $ch/$mask/$flag"

	if [ $flag -ne 0 ]; then
		##echo "flag: $flag"
		# check rom type, use different rom
		rom_type=`cat $CONFIG_FILE | jq .camera.cam$ch.rom_type`

		if [ $rom_type -eq 0 ]; then
			APP_NAME=${ROM_DIR}/rom0/app.bin
			SYSCFG_NAME=${ROM_DIR}/rom0/table_0x140000.jbf
			WARPCFG_NAME=${ROM_DIR}/rom0/table_0x148000.jbf
			ISPCFG_NAME=${ROM_DIR}/rom0/table_0x150000.jbf
			GEOCFG_NAME=${ROM_DIR}/rom0/table_0x170000.jbf

			SSM44_NAME=${ROM_DIR}/rom0/table_ssm44.bin
			SSM45_NAME=${ROM_DIR}/rom0/table_ssm45.bin
			SSM46_NAME=${ROM_DIR}/rom0/table_ssm46.dat
			SSM47_NAME=${ROM_DIR}/rom0/table_ssm47.bin 
			SSM48_NAME_BIN=${ROM_DIR}/rom0/table_ssm48.bin
			IMX="IMX490"
		elif [ $rom_type -eq 1 ]; then
			APP_NAME=${ROM_DIR}/rom1/app.bin
			SYSCFG_NAME=${ROM_DIR}/rom1/table_0x140000.jbf
			WARPCFG_NAME=${ROM_DIR}/rom1/table_0x148000.jbf
			ISPCFG_NAME=${ROM_DIR}/rom1/table_0x150000.jbf
			GEOCFG_NAME=${ROM_DIR}/rom1/table_0x170000.jbf

			SSM44_NAME=${ROM_DIR}/rom1/table_ssm44.bin
			SSM45_NAME=${ROM_DIR}/rom1/table_ssm45.bin
			SSM46_NAME=${ROM_DIR}/rom1/table_ssm46.dat
			SSM47_NAME=${ROM_DIR}/rom1/table_ssm47.bin 
			SSM48_NAME_BIN=${ROM_DIR}/rom1/table_ssm48.bin
			IMX="IMX490"
		elif [ $rom_type -eq 2 ]; then
			APP_NAME=${ROM_DIR}/rom2/app.bin
			SYSCFG_NAME=${ROM_DIR}/rom2/table_0x140000.jbf
			WARPCFG_NAME=${ROM_DIR}/rom2/table_0x148000.jbf
			ISPCFG_NAME=${ROM_DIR}/rom2/table_0x150000.jbf
			GEOCFG_NAME=${ROM_DIR}/rom2/table_0x170000.jbf

			SSM47_NAME=${ROM_DIR}/rom2/table_ssm47.bin
			SSM48_NAME_DAT=${ROM_DIR}/rom2/table_ssm48.dat
			IMX="IMX390"
		elif [ $rom_type -eq 3 ]; then
			APP_NAME=${ROM_DIR}/rom3/app.bin
			SYSCFG_NAME=${ROM_DIR}/rom3/table_0x140000.jbf
			WARPCFG_NAME=${ROM_DIR}/rom3/table_0x148000.jbf
			ISPCFG_NAME=${ROM_DIR}/rom3/table_0x150000.jbf
			GEOCFG_NAME=${ROM_DIR}/rom3/table_0x170000.jbf

			SSM47_NAME=${ROM_DIR}/rom3/table_ssm47.bin
			SSM48_NAME_DAT=${ROM_DIR}/rom3/table_ssm48.dat
		 	IMX="IMX390"
		elif [ $rom_type -eq 4 ]; then
			APP_NAME=${ROM_DIR}/rom4/app.bin
			SYSCFG_NAME=${ROM_DIR}/rom4/table_0x140000.jbf
			WARPCFG_NAME=${ROM_DIR}/rom4/table_0x148000.jbf
			ISPCFG_NAME=${ROM_DIR}/rom4/table_0x150000.jbf
			GEOCFG_NAME=${ROM_DIR}/rom4/table_0x170000.jbf

			SSM47_NAME=${ROM_DIR}/rom4/table_ssm47.bin
			SSM48_NAME_DAT=${ROM_DIR}/rom4/table_ssm48.dat
			IMX="IMX390"
		else
			echo "ohoh"
		fi

		if [ -f $APP_NAME ]; then
			echo ">-----4, cam[$ch] start update rom-----<"
			IspFirmwareOta_Table $ch
			if [ "$VAL_EXIT" != "0" ]; then
				echo "cam[$ch] update rom err"
				ota_ret=1  #有失败，则不下电
			fi
		else
			echo "cam[$ch] rom $APP_NAME not find, skip!"
		fi
	fi
}&
done
wait

# 4, fakra
echo -e ">------------------------4, fakra------------------------<"
if [ 0 -eq $ota_ret ]; then
	print_log -e "\n>>-4, fakra poweroff"
	/sbin/devmem 0x80000020 32 0xffff0000
else
	print_log -e "\n>>4, fakra not poweroff, since chan update err, pls check!!"
fi

# 5, printf:time
echo -e ">------------------------5, end------------------------<"
print_log -e "\n>>-5, table_ispota.sh end!!!"
date_end=$(date +%s)
duration=$(($date_end-$date_start))
print_log -e "time: start/end/duration(s): $date_start/$date_end/$duration"

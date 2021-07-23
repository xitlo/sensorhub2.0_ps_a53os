#!/bin/bash
# Author: chenshan <chenshan1@Momenta.ai>
# Last modify:
# Last modify date:

SCRIPT_VERSION=v1.0

CONFIG_FILE=/data/sensorhub2-config.json
CAM_LOG_DIR=/data/bsplog
CAM_LOG=$CAM_LOG_DIR/cam_read_cali.log
OTP_ret=0

print_log() {
	echo $1 $2 >> $CAM_LOG
	echo $1 $2
}

# function: 读取Calibration Data数据，并将读取出来的数据保存为.csv格式文件
# input:
# output:
Read_Calibration_Data(){
	CAM=$1
	if [ $CAM -ge 6 ]; then
		IMX="390"
	else
		IMX="490"
	fi
	local id i
	local API_CMD_LOG=$(printf "$CAM_LOG_DIR/cam_read_cali%02d.log" $CAM)
	local tty_port=`cat $CONFIG_FILE | jq .camera.cam${CAM}.tty_port`
	local rom_type=`cat $CONFIG_FILE | jq .camera.cam${CAM}.rom_type`
	echo "IMX:$IMX"

	#390分为三部分0x1000-0x10d7   0x10d9-0x11ca  0x11cc-0x11ec
	if [ $IMX = "390" ];then
		id=4096 #0x1000
		Id_End1=4312 #0x10d8
		value=16384 #0x4000
		for((i=4096;i<$Id_End1;i++))
		do
			Register_Adr=`printf %x $id`
			Register_Adr_HIGH=`echo $Register_Adr | cut -b 3-4`
			Register_Adr_LOW=`echo $Register_Adr | cut -b 1-2`

			Register_Value=`printf %x $value`

			print_log "-------------------------------"
			print_log "api_cmd -U$tty_port 0xC0 4 A0000000${Register_Adr_HIGH}${Register_Adr_LOW}00000201"	
			api_cmd -U$tty_port 0xC0 4 A0000000${Register_Adr_HIGH}${Register_Adr_LOW}00000201 > $API_CMD_LOG

			VAR=`cat $API_CMD_LOG`
			CommandStatus=`cat $API_CMD_LOG | grep Done`
			if [ "$CommandStatus" = "" ]; then
				echo -e "\e[1;31m====cam[$CAM],Read_Calibration_Data====error \e[0m"
				return 1
			else
				echo -e "cam[$CAM],Read_Calibration_Data ok"
			fi
			GetData=`cat $API_CMD_LOG | grep PAYLOAD | awk '{print $2}'`
			print_log "cam[$CAM],Register_Adr:$Register_Adr"
			print_log "cam[$CAM],GetData:$GetData"

			print_log "0x${Register_Value},0x${GetData}" >> $CSV1FILE

			let id+=1
			let value+=1
		done
		print_log "--------cam[$CAM],Calibration_Data_Read-1 Done ok -------"
		#第一部分与固定部分cat
		if [ $rom_type -eq 2 ]; then
			cat $CSV1FILE $FIX6_CSVFILE > $TEMP_CSVFILE
		elif [ $rom_type -eq 3 ]; then
			cat $CSV1FILE $FIX8_CSVFILE > $TEMP_CSVFILE
		elif [ $rom_type -eq 4 ]; then
			cat $CSV1FILE $FIX12_CSVFILE > $TEMP_CSVFILE
		fi
		#2--------------------------------
		#第三部分分为三小段，0x3053 0x34c0-0x34cf 0x3630-0x363f
		api_cmd -U$tty_port 0xC0 4 A0000000cc1100000201 > $API_CMD_LOG
		GetData=`cat $API_CMD_LOG | grep PAYLOAD | awk '{print $2}'`
		print_log "0x3053,0x${GetData}" > $CSV2FILE

		#0x34c0-0x34cf
		id=4557 #0x11cd
		Id_End2=4573 #0x11cd-
		value=13504 #0x34c0	-0x34cf
		for((i=4557;i<$Id_End2;i++))
		do
			Register_Adr=`printf %x $id`
			Register_Adr_HIGH=`echo $Register_Adr | cut -b 3-4`
			Register_Adr_LOW=`echo $Register_Adr | cut -b 1-2`
			
			Register_Value=`printf %x $value`
			
			print_log "-------------------------------"
			print_log "api_cmd -U$tty_port 0xC0 4 A0000000${Register_Adr_HIGH}${Register_Adr_LOW}00000201"
				
			api_cmd -U$tty_port 0xC0 4 A0000000${Register_Adr_HIGH}${Register_Adr_LOW}00000201 > $API_CMD_LOG
			VAR=`cat $API_CMD_LOG`
			CommandStatus=`cat $API_CMD_LOG | grep Done`
			if [ "$CommandStatus" = "" ]; then
				echo -e "\e[1;31m====cam[$CAM],Read_Calibration_Data====error \e[0m"
				return 1
			else
				echo -e "cam[$CAM],Read_Calibration_Data ok"
			fi
			GetData=`cat $API_CMD_LOG | grep PAYLOAD | awk '{print $2}'`
			print_log "cam[$CAM],Register_Adr:$Register_Adr"
			print_log "cam[$CAM],GetData:$GetData"
				
			print_log "0x${Register_Value},0x${GetData}" >> $CSV2FILE

			let id+=1
			let value+=1
		done
		print_log "--------cam[$CAM],Calibration_Data_Read-2 Done ok -------"
		
		#0x3630-0x363f
		id=4573 #0x
		Id_End3=4589 #0x11ed
		value=13872 #0x3630	-0x363f
		for((i=4573;i<$Id_End3;i++))
		do
			Register_Adr=`printf %x $id`
			Register_Adr_HIGH=`echo $Register_Adr | cut -b 3-4`
			Register_Adr_LOW=`echo $Register_Adr | cut -b 1-2`

			Register_Value=`printf %x $value`
			
			print_log "-------------------------------"
			print_log "api_cmd -U$tty_port 0xC0 4 A0000000${Register_Adr_HIGH}${Register_Adr_LOW}00000201"
				
			api_cmd -U$tty_port 0xC0 4 A0000000${Register_Adr_HIGH}${Register_Adr_LOW}00000201 > $API_CMD_LOG
			VAR=`cat $API_CMD_LOG`
			CommandStatus=`cat $API_CMD_LOG | grep Done`
			if [ "$CommandStatus" = "" ]; then
				echo -e "\e[1;31m====cam[$CAM],Read_Calibration_Data====error \e[0m"
				return 1
			else
				echo -e "cam[$CAM],Read_Calibration_Data ok"
			fi
			GetData=`cat $API_CMD_LOG | grep PAYLOAD | awk '{print $2}'`
			print_log "cam[$CAM],Register_Adr:$Register_Adr"
			print_log "cam[$CAM],GetData:$GetData"
				
			print_log "0x${Register_Value},0x${GetData}" >> $CSV2FILE

			let id+=1
			let value+=1
		done
		print_log "--------cam[$CAM],Calibration_Data_Read-3 Done ok -------"
		cat $TEMP_CSVFILE $CSV2FILE > $OUT_CSVFILE
##-------------------------------------------------------------##
	elif [ $IMX = "490" ];then
		id=4096 #0x1000
		Id_End1=5632 #0?
		value=2304 #0x0900
		for((i=4096;i<$Id_End1;i++))
		do
			Register_Adr=`printf %x $id`
			Register_Adr_HIGH=`echo $Register_Adr | cut -b 3-4`
			Register_Adr_LOW=`echo $Register_Adr | cut -b 1-2`

			Register_Value=`printf %x $value`
			
			print_log "-------------------------------"
			print_log "api_cmd -U$tty_port 0xC0 4 A0000000${Register_Adr_HIGH}${Register_Adr_LOW}00000201"
	
			api_cmd -U$tty_port 0xC0 4 A0000000${Register_Adr_HIGH}${Register_Adr_LOW}00000201 > $API_CMD_LOG
			VAR=`cat $API_CMD_LOG`
			# echo "$VAR" #调试用
			CommandStatus=`cat $API_CMD_LOG | grep Done`
			if [ "$CommandStatus" = "" ]; then
				echo -e "\e[1;31m====cam[$CAM],Read_Calibration_Data====error \e[0m"
				return 1
			else
				echo -e "cam[$CAM],Read_Calibration_Data ok"
			fi
			GetData=`cat $API_CMD_LOG | grep PAYLOAD | awk '{print $2}'`
			print_log "cam[$CAM],Register_Adr:$Register_Adr"
			print_log "cam[$CAM],GetData:$GetData"
			
			if [ ${#Register_Value} == 3 ]; then
				Register_Value=0$Register_Value
			else
				Register_Value=$Register_Value
			fi

			print_log "0x${Register_Value},0x${GetData}" >> $CSV1FILE

			let id+=1
			let value+=1
		done
		print_log "--------cam[$CAM],Calibration_Data_Read-1 Done ok -------"
		#0x3a66-3a75
		id=5632 #0x1000
		Id_End2=5648 #0x160f+1
		value=14950 #0x3a66
		for((i=5632;i<$Id_End2;i++))
		do
			Register_Adr=`printf %x $id`
			Register_Adr_HIGH=`echo $Register_Adr | cut -b 3-4`
			Register_Adr_LOW=`echo $Register_Adr | cut -b 1-2`
			
			Register_Value=`printf %x $value`
			
			print_log "-------------------------------"
			print_log "api_cmd -U$tty_port 0xC0 4 A0000000${Register_Adr_HIGH}${Register_Adr_LOW}00000201"
			
			api_cmd -U$tty_port 0xC0 4 A0000000${Register_Adr_HIGH}${Register_Adr_LOW}00000201 > $API_CMD_LOG
			VAR=`cat $API_CMD_LOG`
			# echo "$VAR" #调试用
			CommandStatus=`cat $API_CMD_LOG | grep Done`
			if [ "$CommandStatus" = "" ]; then
				echo -e "\e[1;31m====cam[$CAM],Read_Calibration_Data====error \e[0m"
				return 1
			else
				echo -e "cam[$CAM],Read_Calibration_Data ok"
			fi
			GetData=`cat $API_CMD_LOG | grep PAYLOAD | awk '{print $2}'`
			print_log "cam[$CAM],Register_Adr:$Register_Adr"
			print_log "cam[$CAM],GetData:$GetData"
			
			if [ ${#Register_Value} == 3 ]; then
				Register_Value=0$Register_Value
			else
				Register_Value=$Register_Value
			fi
			print_log "0x${Register_Value},0x${GetData}" >> $CSV1FILE

			let id+=1
			let value+=1
		done
		print_log "--------cam[$CAM],Calibration_Data_Read-2 Done ok -------"
		if [ $rom_type -eq 0 ]; then
			cat $CSV1FILE $FIX4_CSVFILE > $OUT_CSVFILE
		elif [ $rom_type -eq 1 ]; then
			cat $CSV1FILE $FIX5_CSVFILE > $OUT_CSVFILE
		fi
	fi
}

# function: 如果write_flash成功就设置标志位
# input:
# output:
Set_Flag(){
	CAM=$1
	print_log -e "\n>>>5, set flag <<<"
	local JSONFILE=/etc/common/isp-OTP/calibration-data-cam.json
	local API_CMD_LOG=$(printf "$CAM_LOG_DIR/cam_read_cali%02d.log" $CAM)
	local tty_port=`cat $CONFIG_FILE | jq .camera.cam${CAM}.tty_port`

	id=64 #0x0040
	for((i=0;i<11;i++))
	do
		Register_Adr=`printf %x $id`
		Register_Adr_HIGH=`echo $Register_Adr | cut -b 3-4`
		Register_Adr_LOW=`echo $Register_Adr | cut -b 1-2`
		print_log "-------------------------------"
		print_log "api_cmd -U${tty_port} 0xC0 4 A0000000${Register_Adr_HIGH}${Register_Adr_LOW}0000000201"
		api_cmd -U${tty_port} 0xC0 4 A0000000${Register_Adr_HIGH}${Register_Adr_LOW}0000000201 >$API_CMD_LOG
		VAR=`cat $API_CMD_LOG`
		# echo "$VAR" #调试用
		CommandStatus=`cat $API_CMD_LOG | grep Done`
		if [ "$CommandStatus" = "" ]; then
			echo -e "\e[1;31m====cam[$CAM],Set_Flag====error \e[0m"
			return 3
		else
			echo -e "cam[$CAM],Set_Flag ok"
		fi
		GetData[$i]=`cat $API_CMD_LOG | grep PAYLOAD | awk '{print $2}'`
		print_log "Register_Adr:$Register_Adr"
		print_log "GetData:$GetData"
		Set_VERSION=$Set_VERSION${GetData[$i]} #把每一步读到的ver拼接起来
		let id+=1
	done
	echo ">cam[$CAM] Set_VERSION: $Set_VERSION"
	sed -i "s/\"module_version${CAM}\":.*$/\"module_version${CAM}\":\"$Set_VERSION\",/" $JSONFILE
	sed -i "s/\"cal_flag${CAM}\":.*$/\"cal_flag${CAM}\":\"done\",/" $JSONFILE
}

# function: dat文件写入到ISP Flash中
# input:
# output:
Write_Flash(){
	CAM=$1
	DATFILE=/home/root/table_ssmcam$CAM.dat
	local tty_port=`cat $CONFIG_FILE | jq .camera.cam${CAM}.tty_port`

	if [ $CAM -ge 6 ]; then
		IMX="390"
	else
		IMX="490"
	fi
	local API_CMD_LOG=$(printf "$CAM_LOG_DIR/cam_read_cali%02d.log" $CAM)
	api_cmd -U${tty_port} 0x19 max AD > $API_CMD_LOG
	sleep 1

	BYTE_LENGTH=`wc -c $DATFILE`
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

	if [ $IMX = "490" ];then
		api_cmd -U${tty_port} 0x90 0 2e00000000003701${HIGHBYTE}${LOWBYTE}0000 -i $DATFILE > $API_CMD_LOG
		print_log "-------------------------------"
		print_log "api_cmd -U${tty_port} 0x90 0 2e00000000003701${HIGHBYTE}${LOWBYTE}0000 -i $DATFILE"
	else
		api_cmd -U${tty_port} 0x90 0 3000000000003701${HIGHBYTE}${LOWBYTE}0000 -i $DATFILE > $API_CMD_LOG
		print_log "-------------------------------"
		print_log "api_cmd -U${tty_port} 0x90 0 3000000000003701${HIGHBYTE}${LOWBYTE}0000 -i $DATFILE"	
	fi
	
	VAR=`cat $API_CMD_LOG`
	# echo "$VAR"
	CommandStatus=`cat $API_CMD_LOG | grep Done`
	if [ "$CommandStatus" = "" ] ; then
		echo -e "\e[1;31m====cam[$CAM],write flash====error \e[0m"
		return 2
	else
		echo -e "\e[1;32m====cam[$CAM],write flash====ok \e[0m"
		Set_Flag $CAM
	fi
}

#----------------0, main -------------------------#
utime=`date +%Y%m%d_%H%M%S`
rm -rf $CAM_LOG
echo -e "---------time: $utime---------\n" > $CAM_LOG
print_log -e "\ncalibration_VER: $SCRIPT_VERSION"

#----------------1, fakra powerup-----------------#
print_log -e "\n>>>1, fakra powerup<<<"
/sbin/devmem 0x80000020 32 0x0000ffff
print_log -e ">>>1.1, sleep 2s, wait isp normal<<<"
sleep 2

err_mask=0
if [ $2 -eq 1 ]; then
	#----------------3, read calibration data && save csv-----------------#
	for((ch=0;ch<16;ch++))
	do
	{
		mask=$(printf "0x%04x" $((1 << $ch)))
		flag=$(printf "%d" $(($1 & $mask)))

		if [ $flag -ne 0 ]; then
			CSV1FILE=/home/root/OTP${ch}_read1.csv
			FIX4_CSVFILE=/etc/common/isp-OTP/OTP4_fix.csv
			FIX5_CSVFILE=/etc/common/isp-OTP/OTP5_fix.csv
			FIX6_CSVFILE=/etc/common/isp-OTP/OTP6_fix.csv
			FIX8_CSVFILE=/etc/common/isp-OTP/OTP8_fix.csv
			FIX12_CSVFILE=/etc/common/isp-OTP/OTP12_fix.csv
			TEMP_CSVFILE=/home/root/OTP${ch}_temp.csv

			CSV2FILE=/home/root/OTP${ch}_read2.csv
			OUT_CSVFILE=/home/root/OTP${ch}.csv

			print_log -e "\n>>>3, read calibration data and save csv<<<"
			Read_Calibration_Data $ch
			if [ $? -ne 0 ]; then
				ret=$(printf "%d" $((1 + $ch)))
				echo "cam[$ch] read calibration data err, exit $ret"
				exit $ret
			fi
		fi
		exit 0
	}&
	done

	for pid in $(jobs -p)
	do
		wait $pid
		status=$?
		if [ $status -ne 0 ];then
			OTP_ret=1
		fi
	done
elif [ $2 -eq 2 ]; then
	#----------------4, .dat write flash-----------------#
	for((ch=0;ch<16;ch++))
	do
	{
		mask=$(printf "0x%04x" $((1 << $ch)))
		flag=$(printf "%d" $(($1 & $mask)))

		if [ $flag -ne 0 ]; then
			print_log -e "\n>>>4, .dat write flash<<<"
			Write_Flash $ch
			#----------------5, set flag-----------------#
			if [ $? -ne 0 ]; then
				ret=$(printf "%d" $((1 + $ch)))
				echo "cam[$ch] .dat write flash err, exit $ret"
				exit $ret
			fi
		fi
		exit 0
	}&
	done

	for pid in $(jobs -p)
	do
		wait $pid
		status=$?
		if [ $status -ne 0 ];then
			ch=$(printf "%d" $(($status - 1)))
			mask=$(printf "0x%04x" $((1 << $ch)))
			err_mask=$(printf "0x%04x" $(($err_mask | $mask)))
			print_log -e "cam[$ch] otp have some error!, err_mask is $err_mask"
		fi
	done
	rm -rf /home/root/OTP*.csv
	rm -rf /home/root/table_ssmcam*.dat
fi

#----------------6, fakra poweroff-----------------#
print_log -e "\n>>>6, fakra poweroff<<<"
/sbin/devmem 0x80000020 32 0xffff0000

#------------------7.end----------------#
rm -rf $API_CMD_LOG
print_log -e "\n>>>7.end, all done!<<<"
exit $OTP_ret

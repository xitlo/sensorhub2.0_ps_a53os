#!/bin/bash

echo -e "$0 VER: v1.6"

# version registers start address for cam 0-15
REG_ADDR=(
	0x80000184 0x80000198 0x800001ac 0x800001c0
	0x800001d4 0x800001e8 0x800001fc 0x80000210
	0x80000224 0x80000238 0x8000024c 0x80000260
	0x80000274 0x80000288 0x8000029c 0x800002b0
)

# function, read camera version
# input 1: channel num, 0-15, e.g. 4
# input 2: version register start address to set, hex, e.g. 0x800001d4
CamReadVer(){
	dev_ch=$(printf "%02x" $1)
	echo -e "\n>>>1, connect cam[$dev_ch] to a53 through uart"
	devmem 0x80040000 32 0x000100$dev_ch

	echo -e ">>>2, read cam version, 16bytes"
	id=0
	for((i=0;i<16;i++))
	do
		# ver1=`api_cmd -U 1 0x53 1 0500000000fa000001000000 | grep "PAYLOAD\[00\:00\]" | awk '{print $2}'`
		cmd_line=$(printf "api_cmd -U1 0x53 1 0500000000fa0000%02x000000 | grep \"PAYLOAD\\[00\\:00\\]\" | awk \'{print \$2}\'" $id)
		# echo $cmd_line
		ver[$i]=`eval $cmd_line`
		# echo ${ver[$i]}

		if [ ! -n "${ver[$i]}" ] ; then
			for((j=0;j<16;j++))
			do
				ver[$j]=00
			done
			break;
		fi

		let id+=1
	done

	VER0=0x${ver[0]}${ver[1]}${ver[2]}${ver[3]}
	VER1=0x${ver[4]}${ver[5]}${ver[6]}${ver[7]}
	VER2=0x${ver[8]}${ver[9]}${ver[10]}${ver[11]}
	VER3=0x${ver[12]}${ver[13]}${ver[14]}${ver[15]}

	# echo -e "ver0/1/2/3: ${ver[0]}/${ver[1]}/${ver[2]}/${ver[3]}/$ver4"
	# mem-test r 0x80000000 0x01d4 10
	# mem-test w 0x80000000 0x01d4 $ver4
	# mem-test r 0x80000000 0x01d4 10
	echo -e ">>>3, VERSION: $VER0/$VER1/$VER2/$VER3"

	REG0=$2
	temp=$(($REG0 + 0x04))
	REG1=$(printf "0x%08x" $temp)
	temp=$(($REG1 + 0x04))
	REG2=$(printf "0x%08x" $temp)
	temp=$(($REG2 + 0x04))
	REG3=$(printf "0x%08x" $temp)
	
	echo -e ">>>4, set version to reg: $REG0/$REG1/$REG2/$REG3"
	devmem $REG0 32 $VER0
	devmem $REG1 32 $VER1
	devmem $REG2 32 $VER2
	devmem $REG3 32 $VER3

	echo -e ">>>5, deconnect cam"
	devmem 0x80040000 32 0x0
}


echo -e "\n>>1, fakra powerup"
devmem 0x80000020 32 0xfffffff0

echo -e "\n>>2, sleep 2s, wait isp normal"
sleep 2

echo -e "\n>>3, read and set version for cam 4-15"
for((ch=4;ch<16;ch++))
do
	CamReadVer $ch ${REG_ADDR[$ch]}
done

echo -e "\n>>4, fakra poweroff"
devmem 0x80000020 32 0x0

echo -e "\n>>5, all done!"

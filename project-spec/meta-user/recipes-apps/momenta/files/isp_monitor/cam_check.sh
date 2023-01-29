#!/bin/bash -ex

ls -lt /data/cam_check_log/ | awk '{if(NR>100){print "rm -r /data/cam_check_log/" $9}}' | sh
time=$(date "+%Y%m%d_%H_%M_%S" )
CAM_CHECK_LOG_DIR=/data/cam_check_log/$time
mkdir -p $CAM_CHECK_LOG_DIR
cd $CAM_CHECK_LOG_DIR

cam_nums=(490-0 490-1 490-2 490-3 F30 F120 LF100 RF100 LR60 RR60 RU60 RD60 Front197 Right197 Rear197 Left197)
uart_nums=(0 0 0 0 10 11 12 13 14 15 2 3 4 5 6 7)
FILE_IN=${uart_nums[$1]}
cam_name=${cam_nums[$1]}
name_cam_check=$CAM_CHECK_LOG_DIR/result.txt
touch $name_cam_check

echo "FPGA is ready *******************************"
mem-test r 0x80000000 100 20 >>$name_cam_check #相机上电状态、通道配置状态、lock状态、输入状态
                               #各相机lock丢失计数与丢帧计数
mem-test r 0x80010000 0 1 >>$name_cam_check #9296lock状态、status状态（Vsync信号）

echo "ISP is ready *******************************"
api_0x10=$(api_cmd -U$FILE_IN 0x10 max | grep PAYLOAD)  #ISP状态
echo "$cam_name 0x10:$api_0x10" >>$name_cam_check
api_0x18_01=$(api_cmd -U$FILE_IN 0x18 max | grep PAYLOAD) #ISP输如输出状态
echo "$cam_name 0x18_01:$api_0x18_01" >>$name_cam_check
api_0x18_02=$(api_cmd -U$FILE_IN 0x18 max | grep PAYLOAD) #ISP输如输出状态
echo "$cam_name 0x18_02:$api_0x18_02" >>$name_cam_check

echo "9296 is ready *****************************"
api_22=$(api_cmd -U$FILE_IN  0xc0 04 90000000220000000201 | grep PAYLOAD)  #解串22寄存器
echo "$cam_name 22:$api_22" >>$name_cam_check
api_55c=$(api_cmd -U$FILE_IN  0xc0 04 900000005c0500000201 | grep PAYLOAD)  #解串55c寄存器
echo "$cam_name 55c:$api_55c" >>$name_cam_check
api_55d=$(api_cmd -U$FILE_IN  0xc0 04 900000005d0500000201 | grep PAYLOAD)  #解串55d寄存器
echo "$cam_name 55d:$api_55d" >>$name_cam_check
api_112=$(api_cmd -U$FILE_IN  0xc0 04 90000000120100000201 | grep PAYLOAD)  #解串输入数据Y通道 crc Error异常
echo "$cam_name 112:$api_112" >>$name_cam_check
api_124=$(api_cmd -U$FILE_IN  0xc0 04 90000000240100000201 | grep PAYLOAD)  #解串输入数据Z通道 crc Error异常
echo "$cam_name 124:$api_124" >>$name_cam_check



if [ $1 -eq 4 -o $1 -eq 5 ]; then
    echo "IMX490 is ready *******************************"
    api_490=$(api_cmd -U$UART_PORT 0xC0 04 34000000707600000201 | grep PAYLOAD)  #490状态
    echo "$cam_name 490:$api_490" >>$name_cam_check
else
    echo "IMX390 is ready *******************************"
    api_390=$(api_cmd -U$UART_PORT 0xC0 04 42000000015000000201 | grep PAYLOAD)  #390状�
    echo "$cam_name 390:$api_390" >>$name_cam_check
fi
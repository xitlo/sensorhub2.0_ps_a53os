#!/bin/bash -ex

ls -lt /data/cam_check_log/ | awk '{if(NR>100){print "rm -r /data/cam_check_log/" $9}}' | sh
time=$(date "+%Y%m%d_%H_%M_%S" )
CAM_CHECK_LOG_DIR=/data/cam_check_log/$time
mkdir -p $CAM_CHECK_LOG_DIR
cd $CAM_CHECK_LOG_DIR

cam_nums=(490-0 490-1 490-2 490-3 F30 F120 LF100 RF100 LR60 RR60 RU60 RD60 Front197 Right197 Rear197 Left197)
uart_nums=(0 0 0 0 10 11 12 13 14 15 2 3 4 5 6 7)
UART_PORT=${uart_nums[$1]}
cam_name=${cam_nums[$1]}

name_0=result_9296_cam$cam_name.txt
name_1=result_GW5x_cam$cam_name.txt
name_2=result_9295_cam$cam_name.txt
name_3=result_IMX390_cam$cam_name.txt
name_4=result_IMX490_cam$cam_name.txt
name_5=result_FPGA_cam$cam_name.txt
touch $CAM_CHECK_LOG_DIR/$name_0
touch $CAM_CHECK_LOG_DIR/$name_1
touch $CAM_CHECK_LOG_DIR/$name_2
touch $CAM_CHECK_LOG_DIR/$name_3
touch $CAM_CHECK_LOG_DIR/$name_4
touch $CAM_CHECK_LOG_DIR/$name_5
dir_9296=$CAM_CHECK_LOG_DIR/$name_0
dir_9295=$CAM_CHECK_LOG_DIR/$name_2
dir_IMX390=$CAM_CHECK_LOG_DIR/$name_3
dir_IMX490=$CAM_CHECK_LOG_DIR/$name_4
isp_gw5x=$CAM_CHECK_LOG_DIR/$name_1
dir_FPGA=$CAM_CHECK_LOG_DIR/$name_5

echo "FPGA is ready *******************************"
mem-test r 0x80000000 100 10 >>$dir_FPGA #相机上电状态、通道配置状态、lock状态、输入状态
mem-test r 0x80000000 130 20 >>$dir_FPGA #各相机lock丢失计数与丢帧计数
mem-test r 0x80010000 0 10 >>$dir_FPGA #9296lock状态、status状态（Vsync信号）

echo "ISP is ready *******************************"
api_cmd -U$UART_PORT 0x10 max | grep -E "Data|PAYLOAD" >>$isp_gw5x  #ISP状态
api_cmd -U$UART_PORT 0x18 max | grep -E "Data|PAYLOAD" >>$isp_gw5x  #ISP输如输出状态
api_cmd -U$UART_PORT 0x18 max | grep -E "Data|PAYLOAD" >>$isp_gw5x  #ISP输如输出状态
api_cmd -U$UART_PORT 0x2 max 0140005e51 | grep -E "Data|PAYLOAD" >>$isp_gw5x  #SOT Error 计数
api_cmd -U$UART_PORT 0x2 max 0154005e51 | grep -E "Data|PAYLOAD" >>$isp_gw5x  #CRC Error 计数

echo "9296 is ready *****************************"
api_cmd -U$UART_PORT  0xc0 04 900000002f0000000201 | grep -E "Data|PAYLOAD" >>$dir_9296  #解串Lock丢失
api_cmd -U$UART_PORT  0xc0 04 90000000000100000201 | grep -E "Data|PAYLOAD" >>$dir_9296  #解串输入数据通道 crc Error异常
api_cmd -U$UART_PORT  0xc0 04 90000000120100000201 | grep -E "Data|PAYLOAD" >>$dir_9296  #解串输入数据通道 crc Error异常
api_cmd -U$UART_PORT  0xc0 04 90000000240100000201 | grep -E "Data|PAYLOAD" >>$dir_9296  #解串输入数据通道 crc Error异常
api_cmd -U$UART_PORT  0xc0 04 90000000360100000201 | grep -E "Data|PAYLOAD" >>$dir_9296  #解串输入数据通道 crc Error异常
api_cmd -U$UART_PORT  0xc0 04 900000005c0500000201 | grep -E "Data|PAYLOAD" >>$dir_9296  #解串输入数据通道crc异常计数
api_cmd -U$UART_PORT  0xc0 04 900000005d0500000201 | grep -E "Data|PAYLOAD" >>$dir_9296  #解串输入数据通道crc异常计数
api_cmd -U$UART_PORT  0xc0 04 90000000dc0100000201 | grep -E "Data|PAYLOAD" >>$dir_9296  #解串输入数据lock状态
api_cmd -U$UART_PORT  0xc0 04 900000003c0100000201 | grep -E "Data|PAYLOAD" >>$dir_9296  #解串输入数据lock状态
api_cmd -U$UART_PORT  0xc0 04 900000001c0200000201 | grep -E "Data|PAYLOAD" >>$dir_9296  #解串输入数据lock状态
api_cmd -U$UART_PORT  0xc0 04 900000003c0200000201 | grep -E "Data|PAYLOAD" >>$dir_9296  #解串输入数据lock状态


echo "9295 is ready *******************************"
api_cmd -U$UART_PORT 0xC0 04 800000001d0400000201 | grep -E "Data|PAYLOAD" >>$dir_9295  #串化芯片 crc error 计数
api_cmd -U$UART_PORT 0xC0 04 80000000130000000201 | grep -E "Data|PAYLOAD" >>$dir_9295  #串化芯片输出端lock
api_cmd -U$UART_PORT 0xC0 04 80000000410300000201 | grep -E "Data|PAYLOAD" >>$dir_9295  #串化芯片输入端mipi
api_cmd -U$UART_PORT 0xC0 04 80000000420300000201 | grep -E "Data|PAYLOAD" >>$dir_9295  #串化芯片输入端mipi
api_cmd -U$UART_PORT 0xC0 04 80000000201000000201 | grep -E "Data|PAYLOAD" >>$dir_9295  #pclock检测
api_cmd -U$UART_PORT 0xC0 04 80000000a01000000201 | grep -E "Data|PAYLOAD" >>$dir_9295  #pclock检测
api_cmd -U$UART_PORT 0xC0 04 80000001201000000201 | grep -E "Data|PAYLOAD" >>$dir_9295  #pclock检测
api_cmd -U$UART_PORT 0xC0 04 80000001a01000000201 | grep -E "Data|PAYLOAD" >>$dir_9295  #pclock检测

if [ $1 -eq 4 -o $1 -eq 5 ]; then
    echo "IMX490 is ready *******************************"
    api_cmd -U$UART_PORT 0xC0 04 34000000707600000201 | grep -E "Data|PAYLOAD" >>$dir_IMX490  #490状态
else
    echo "IMX390 is ready *******************************"
    api_cmd -U$UART_PORT 0xC0 04 42000000015000000201 | grep -E "Data|PAYLOAD" >>$dir_IMX390  #390状态
fi
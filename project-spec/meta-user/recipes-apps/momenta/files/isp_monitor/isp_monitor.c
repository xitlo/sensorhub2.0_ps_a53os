/** ===================================================== **
 *Author : Momenta guansong.huang
 *Created: 2022-7-26
 *Version: 1.0
 ** ===================================================== **/

/** ===================================================== **
 * INCLUDE
 ** ===================================================== **/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <time.h>
#include <errno.h>
#include "log.h"
#include <sys/mman.h>
#include "common.h"
#include<sys/wait.h>
#include<sys/types.h>

char isp_uart_channel[12]={10,11,12,13,14,15,2,3,4,5,6,7};
int check_cmd10_res04(int index) {
    int ret=-1;
    FILE *fp;
    char buffer[1024];
    char cmd_str[128] = "";

    char *ptr;
    char str02[50]="PAYLOAD[00:03]: 02";
    char str04[50]="PAYLOAD[00:03]: 04";
    sprintf(cmd_str, "/usr/bin/api_cmd -U%d 0x10 max | grep PAYLOAD", isp_uart_channel[index]);
    _log_info("ISP:check 0x10 status:%s\n",cmd_str);
    fp = popen(cmd_str, "r");
    if (fp == NULL) {
        _log_info("ISP:Failed to run command:%s\n" ,cmd_str);
        ret=-1;
        return ret;
    }

    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        _log_info("ISP:%s", buffer);
        ptr = strstr(buffer, str02);
        if(ptr) {
            _log_info("ISP:'%s' is found in '%s'\n", str02, buffer);
            ret=2;
            break;
        }
        else {
            _log_info("ISP:'%s' is not found in '%s'\n", str02,buffer);
        }

        ptr = strstr(buffer, str04);
        if(ptr) {
            _log_info("ISP: '%s' is found in '%s'\n", str04, buffer);
            ret=4;
            break;
        }
        else {
            _log_info("ISP: '%s' is not found in '%s'\n", str04,buffer);
        }

    }

    pclose(fp);

    return ret;
}

int main(int argc, char *argv[])
{
    static char pwrinitflg=0;
    char errcnt =0;
    unsigned int  addr, value, map_size;;
    unsigned long base;
    unsigned char *map_base,*map_base1,*map_base2;
    int dev_fd;
    unsigned char index;
    unsigned int poweronflg=0x00000000;
    unsigned int powerontmp;
    unsigned int pcie_flag=0x00000000;
    pid_t status;
    char sysretval;
    char cmd_str[128] = "";
    int mipi_cnt0,mipi_cnt1,mipi_cnt2,mipi_cnt3;
    int reset_all_cnt=0;
    int retval=0;
    int power_brd_flg[12]={0};
    int j;
    printf("v2.0 camera ISP monitor app start.\n");
	sprintf(cmd_str, "echo 0 > /data/bsplog/ispmonitor.log");
	status = system(cmd_str);
    if (0 != log_init("/etc/common/zlog.conf")) {
        printf("line:%d,errno=%d,parse log config failed, please check zlog.conf", __LINE__, errno);
        return -1;
    }
    dev_fd = open("/dev/mem", O_RDWR | O_NDELAY | O_DSYNC);
    if (dev_fd < 0) {
        _log_info("open(/dev/mem) failed.");
        return 0;
    }
    base = 0x80000000;
    map_size = 0x200;
    map_base = (unsigned char *)mmap(NULL, map_size, PROT_READ | PROT_WRITE, MAP_SHARED, dev_fd, base);
    if((long)map_base == -1) {
        _log_info("map_base map err %d\n",errno);
        return -1;
    }
    base = 0x80010000;
    map_size = 0x190;
    map_base1 = (unsigned char *)mmap(NULL, map_size, PROT_READ | PROT_WRITE, MAP_SHARED, dev_fd, base);
    if((long)map_base1 == -1) {
        _log_info("map_base1 map err %d\n",errno);
        return -1;
    }
    printf("v2.0 camera ISP monitor runing.\n");
    _log_info("ISP: ISP camera ISP monitor app start.\n");
    while (1)
    {
        sleep(1);
        pcie_flag=*(volatile unsigned int *)(map_base + 0x10c);
        _log_info("ISP: pcie_flag=0x%08x \n",pcie_flag);
        //if camera is poweroff the stop check isp status
        poweronflg = *(volatile unsigned int *)(map_base + 0x20);
        if((poweronflg & 0x0000000f) != 0x00000000){
            pwrinitflg =0;
            _log_info("ISP: poweronflg=0x%08x camera service or test_demo is not running, don't check\n",poweronflg);
            continue;
		}else if((poweronflg & 0x0000fff0) == 0x00000000){
            pwrinitflg =0;
            _log_info("ISP: poweronflg=0x%08x ISP camera is all power-off, don't check\n",poweronflg);
            continue;
        }else if(pwrinitflg == 0){
            sleep(10);
           // poweronflg = *(volatile unsigned int *)(map_base + 0x20);
            pwrinitflg =1;
            continue;
        }

        value = *(volatile unsigned int *)(map_base1 + 0);
        _log_info("ISP: ISP poweronflg=0x%08x,came_status=0x%08x\n",poweronflg,value);
        if((value & 0x0000fff0) != (poweronflg & 0x0000fff0)){
            //set flag
			sprintf(cmd_str, "echo 1 > /data/bsplog/ispmonitor.log");
            status = system(cmd_str);
            for(char i=0;i<12;i++){
                if(((poweronflg >> (4+i)) & 0x00000001) == 0) //do not reset camera which is not power on
                    continue;
                if((value >> (4+i)) & 0x00000001)
                    continue;

                reset_all_cnt=0;
                //check_cam isp status
                sprintf(cmd_str, "/etc/common/cam_check.sh %d", 4+i);
                status = system(cmd_str);
                if(-1 == status)
                    _log_info("cam_check: /etc/common/cam_check.sh exec faild");
                //ther are cam error,reset all of them
                _log_info("ISP: ISP value:0x%08x cam_channel(%d) uart(%d) is error,reset it\n",value,4+i,isp_uart_channel[i]);
                errcnt=0;
                
                //*************************************************************
                //get mipi status
                base = 0x80240000;
                map_size = 0xd00000;
                map_base2 = (unsigned char *)mmap(NULL, map_size, PROT_READ | PROT_WRITE, MAP_SHARED, dev_fd, base);

                mipi_cnt0 = *(volatile unsigned int *)(map_base2 + 0x10000*i + 0x10);
                mipi_cnt1 = *(volatile unsigned int *)(map_base2 + 0x10000*i + 0x101c);
                sleep(1);
                mipi_cnt2 = *(volatile unsigned int *)(map_base2 + 0x10000*i + 0x10);
                mipi_cnt3 = *(volatile unsigned int *)(map_base2 + 0x10000*i + 0x101c);
                _log_info("dphy cnt0=%d, cnt1=%d, mipi cnt0=%d, cnt1=%d\n",mipi_cnt1,mipi_cnt3,mipi_cnt0,mipi_cnt2);
                
                if((mipi_cnt1!=mipi_cnt3)&&(mipi_cnt0==mipi_cnt2)){
                    //if fpga mipi ip coredump
                    _log_info("fpga mipi ip coredump!!!!!!\n");
                    *(volatile unsigned int *)(map_base + 0x24) = (0x00000001<<(i+4-(i+4)%4));
                    sleep(1);
                    *(volatile unsigned int *)(map_base + 0x24) = 0x0;
                    sleep(1);
                    *(volatile unsigned int *)(map_base + 0x24) = (0x00000001<<(i+4));
                    sleep(1);
                    *(volatile unsigned int *)(map_base + 0x24) = 0x0;
                    sleep(1);
                    
                    //if cam4 mipi dump need reset cam4 first
                    do{
                        sprintf(cmd_str, "/usr/bin/api_cmd -U%d 0x14 max", isp_uart_channel[i-i%4]);
                        status = system(cmd_str);
                        if(-1 == status)
                           _log_info("ISP: ISP system api_cmd 0x14 error");
                        else{
                            sysretval = status >> 8;
                            _log_info("ISP: ISP api cmd 0x14 ret=%d\n",sysretval);
                            if(sysretval != 0){
                                usleep(200 * 1000);
                                errcnt++;
                            }else
                                break;
                        }
                    }while(errcnt<3);
                    if(errcnt >= 3)
                        _log_info("ISP: ISP ----------api_cmd -U%d 0x14 max failed----------\n",isp_uart_channel[i-i%4]);
                    usleep(100 * 1000);
                    errcnt =0;
                    do{
                        sprintf(cmd_str, "/usr/bin/api_cmd -U%d 0x12 noquery 0000000000000000", isp_uart_channel[i-i%4]);
                        status = system(cmd_str);
                        if(-1 == status)
                           _log_info("ISP: system api_cmd 0x12 error");
                        else{
                            sysretval = status >> 8;
                            _log_info("ISP: api cmd 0x12 ret=%d\n",sysretval);
                            if(sysretval != 0){
                                usleep(200 * 1000);
                                errcnt++;
                            }else
                                break;
                        }
                    }while(errcnt<3);
                    if(errcnt >= 3)
                        _log_info("ISP: ----------api_cmd -U%d 0x12 noquery 0000000000000000 failed----------\n",isp_uart_channel[i-i%4]);   
                }
do_isp_reset:
                do{
                    sprintf(cmd_str, "/usr/bin/api_cmd -U%d 0x14 max", isp_uart_channel[i]);
                    status = system(cmd_str);
                    if(-1 == status)
                       _log_info("ISP: ISP system api_cmd 0x14 error");
                    else{
                        sysretval = status >> 8;
                        _log_info("ISP: ISP api cmd 0x14 ret=%d\n",sysretval);
                        if(sysretval != 0){
                            usleep(200 * 1000);
                            errcnt++;
                        }else
                            break;
                    }
                }while(errcnt<3);
                if(errcnt >= 3)
                    _log_info("ISP: ISP ----------api_cmd -U%d 0x14 max failed----------\n",isp_uart_channel[i]);
                usleep(100 * 1000);
                errcnt =0;
                do{
                    sprintf(cmd_str, "/usr/bin/api_cmd -U%d 0x12 noquery 0000000000000000", isp_uart_channel[i]);
                    status = system(cmd_str);
                    if(-1 == status)
                       _log_info("ISP: system api_cmd 0x12 error");
                    else{
                        sysretval = status >> 8;
                        _log_info("ISP: api cmd 0x12 ret=%d\n",sysretval);
                        if(sysretval != 0){
                            usleep(200 * 1000);
                            errcnt++;
                        }else
                            break;
                    }
                }while(errcnt<3);
                if(errcnt >= 3)
                    _log_info("ISP: ----------api_cmd -U%d 0x12 noquery 0000000000000000 failed----------\n",isp_uart_channel[i]);
                //新增逻辑
                sleep(2);
                retval=check_cmd10_res04(i);
                if(retval == 4){
                    //进入了failsafe模式，执行reset_all
                    do{
                        _log_info("ISP: ----------api_cmd -U%d 0xe0 0 010000009000000010000000810000000201 start----------\n",isp_uart_channel[i]);
                        sprintf(cmd_str, "/usr/bin/api_cmd -U%d  0xe0 0 010000009000000010000000810000000201", isp_uart_channel[i]);
                        status = system(cmd_str);
                        if(-1 == status)
                        _log_info("ISP: system api_cmd 0xe0 error");
                        else{
                            sysretval = status >> 8;
                            _log_info("ISP: api cmd 0xe0 ret=%d\n",sysretval);
                            if(sysretval != 0){
                                usleep(200 * 1000);
                                errcnt++;
                            }else
                                break;
                        }
                    }while(errcnt<3);
                    if(errcnt >= 3)
                        _log_info("ISP: ----------api_cmd -U%d 0xe0 0 010000009000000010000000810000000201 failed----------\n",isp_uart_channel[i]);

                    //执行过reset all后再停码流
                    sleep(2);
                    retval=check_cmd10_res04(i);
                    if(retval == 4){
                        reset_all_cnt++;
                        if(reset_all_cnt < 2)
                            goto do_isp_reset;
                        else
                            goto power_reset_mode;
                    }else  if(retval == 2){
                        continue;
                    }else {
                        goto power_reset_mode;
                    }
                }
                continue;
power_reset_mode:
                //power off
                power_brd_flg[i] ++;
                if(power_brd_flg[i] > 2){
                    _log_error("ISP: poweroff cam_channel(%d) uart(%d) over 2 times,but not yet recovery\n",4+i,isp_uart_channel[i]);
                    for(j=0;j<12;j++)
                        if(power_brd_flg[j] <= 2)
                            break;
                        if(j>=12)
                            goto sys_exit;
                    continue;
                }

                _log_info("ISP: poweroff cam_channel(%d) uart(%d),cnt(%d)\n",4+i,isp_uart_channel[i],power_brd_flg[i]);
                powerontmp=0x00000000;
                if(i < 4) {
                    powerontmp = poweronflg & 0xffffff0f;
                }else if (i<8) {
                    powerontmp = poweronflg & 0xfffff0ff;
                } else {
                    powerontmp = poweronflg & 0xffff0fff;
                }

                sprintf(cmd_str, "/usr/bin/mem-test w 0x80000000 0x0020 0x%08x", powerontmp);
                status = system(cmd_str);
                if(-1 == status)
                    _log_info("ISP: mem-test w 0x80000000 0x0020 0x%08x",powerontmp);
                sleep(1);
                //power on
                sprintf(cmd_str, "/usr/bin/mem-test w 0x80000000 0x0020 0x%08x", poweronflg);
                status = system(cmd_str);
                if(-1 == status)
                    _log_info("ISP: mem-test w 0x80000000 0x0020 0x%08x",poweronflg);
            }
            sleep(5);
            //clear flag
			sprintf(cmd_str, "echo 0 > /data/bsplog/ispmonitor.log");
            status = system(cmd_str);
        }
    }
sys_exit:
    close(dev_fd);
    log_fini();
    return 0;
}


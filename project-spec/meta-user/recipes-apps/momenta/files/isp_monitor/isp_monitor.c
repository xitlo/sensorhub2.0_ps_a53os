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
int main(int argc, char *argv[])
{
    static char pwrinitflg=0;
    char errcnt =0;
    unsigned int  addr, value, map_size;;
    unsigned long base;
    unsigned char *map_base,*map_base1;
    int dev_fd;
    unsigned char index;
    unsigned int poweronflg=0x00000000;
    pid_t status;
    char sysretval;
    char cmd_str[128] = "";
	sprintf(cmd_str, "echo 0 > /data/bsplog/ispmonitor.log");
    printf("v2.0 camera ISP monitor app start.\n");
    if (0 != log_init("/etc/common/zlog.conf")) {
        printf("parse log config failed, please check zlog.conf", __LINE__, errno);
        return -1;
    }
    dev_fd = open("/dev/mem", O_RDWR | O_NDELAY | O_DSYNC);
    if (dev_fd < 0) {
        printf("open(/dev/mem) failed.");
        return 0;
    }
    base = 0x80000000;
    map_size = 0x100;
    map_base = (unsigned char *)mmap(NULL, map_size, PROT_READ | PROT_WRITE, MAP_SHARED, dev_fd, base);
    if((long)map_base == -1) {
        printf("map_base map err %d\n",errno);
        return -1;
    }
    base = 0x80010000;
    map_size = 0x190;
    map_base1 = (unsigned char *)mmap(NULL, map_size, PROT_READ | PROT_WRITE, MAP_SHARED, dev_fd, base);
    if((long)map_base1 == -1) {
        printf("map_base1 map err %d\n",errno);
        return -1;
    }
    printf("v2.0 camera ISP monitor runing.\n");
    _log_info("ISP: ISP camera ISP monitor app start.\n");
    while (1)
    {
        sleep(1);
        //if camera is poweroff the stop check isp status
        poweronflg = *(volatile unsigned int *)(map_base + 0x20);
        if((poweronflg & 0x0000fff0) == 0x00000000){
            pwrinitflg =0;
            _log_info("ISP: ISP camera is all power-off,don't check\n",poweronflg);
            continue;
        }else if(pwrinitflg == 0){
            sleep(10);
           // poweronflg = *(volatile unsigned int *)(map_base + 0x20);
            pwrinitflg =1;
            continue;
        }

        value = *(volatile unsigned int *)(map_base1 + 0);
        _log_info("ISP: ISP poweronflg=0x%08x,came_status=0x%08x\n",poweronflg,value);
        if((value & 0x0000fff0) != poweronflg){
            //set flag
			sprintf(cmd_str, "echo 1 > /data/bsplog/ispmonitor.log");
            status = system(cmd_str);
            for(char i=0;i<12;i++){
                if(((poweronflg >> (4+i)) & 0x00000001) == 0) //do not reset camera which is not power on
                    continue;
                if((value >> (4+i)) & 0x00000001)
                    continue;
                //check_cam isp status
                sprintf(cmd_str, "/etc/common/cam_check.sh %d", 4+i);
                status = system(cmd_str);
                if(-1 == status)
                    _log_info("cam_check: /etc/common/cam_check.sh exec faild");
                //ther are cam error,reset all of them
                _log_info("ISP: ISP value:0x%08x cam_channel(%d) uart(%d) is error,reset it\n",value,4+i,isp_uart_channel[i]);
                errcnt=0;
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
            }
            sleep(5);
            //clear flag
			sprintf(cmd_str, "echo 0 > /data/bsplog/ispmonitor.log");
            status = system(cmd_str);
        }
        //clear the interrupt reg
        //*(volatile unsigned int *)(map_base1 + 0x190) = 0xffffffff;
    }
#if 0
//api_cmd -Ux 0x12 noquery 0000000000000000
    apiCode = 0x12;
    noQuery =1;
    noSizeGiven=0;
    sizeToRead=0;
    pCmdStr = "0000000000000000";
    
//api_cmd -U10 0x14 max
    apiCode = 0x14;
    noQuery =0;
    noSizeGiven = 1;
    sizeToRead=0;

    ///end of uart operate
    //Disconnect from W5
#endif
    close(dev_fd);
    log_fini();
    return 0;
}


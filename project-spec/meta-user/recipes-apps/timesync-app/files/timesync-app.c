/** ===================================================== **
 *Author : Momenta founderHAN
 *Created: 2021-1-5
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
#include "log.h"

/** ===================================================== **
 * MACRO
 ** ===================================================== **/
#define VERSION "v1.3"

/** ===================================================== **
 * STRUCT
 ** ===================================================== **/
struct timespec64 {
    long long   tv_sec;         /* seconds */
    long        tv_nsec;        /* nanoseconds */
};

struct sync_data_s {
    struct timespec64  begin;           //起始系统时间
    struct timespec64  end;             //结束系统时间
    struct timespec64  realtime;        //真实时间-读取自ps-timer寄存器
    long               diff_real_b_ns;  //真实时间与起始系统时间差值，用于精度判断
    long               handle_e_b_ns;   //结束系统时间与起始系统时间差值，用于操作时间计算
};

/** ===================================================== **
 * VARIABLE
 ** ===================================================== **/
static int loop=50;

/** ===================================================== **
 * FUNCTION
 ** ===================================================== **/
void ctrl_c_handler(int signum, siginfo_t *info, void *myact)
{
    loop = 0;
    printf("! CTRL+C %d %p %p  %d\n",signum,info,myact,loop);
}

int main(int argc, char **argv)
{
    int fd;
    int ret;
    unsigned int time_period;
    struct sync_data_s readdata;

    if ( 0 != log_init("/etc/common/zlog.conf") ) {
        fprintf(stderr, "parse log config failed, please check zlog.conf\n");
        return -1;
    }

    /*ctrl + c*/
    struct sigaction act;
    sigemptyset(&act.sa_mask);
    act.sa_flags = SA_SIGINFO;
    act.sa_sigaction = ctrl_c_handler;

    if(sigaction(SIGINT, &act, NULL) < 0) {
        fprintf(stderr, "install sigal error\n");
    }

    fprintf(stdout, "timesync-test: %s\n", VERSION);

    /* 验证输入参数个数 */
    if(3 != argc) {
        fprintf(stdout, "usage: %s <full_path_dev_name> <time_period_ms>\n", argv[0]);
        return -1;
    }

    /* 打开输入的设备文件, 获取文件句柄 */
    fd = open(argv[1], O_RDWR);
    if(fd < 0) {
        /* 打开文件失败 */
        fprintf(stderr, "Can't open file %s\r\n", argv[1]);
        return -1;
    }

    time_period = atoi(argv[2]);
    fprintf(stdout, "write[%ld] %d to dev:%s\n", sizeof(time_period), time_period, argv[1]);
    write(fd, &time_period, sizeof(time_period));

    while(loop) {
        sleep(2);
        ret = read(fd, &readdata, sizeof(readdata));
        fprintf(stdout, "SyncRe[%d]! b/r/diff, e/hdl: %lld.%09ld/%lld.%09ld/%ld, %lld.%09ld/%ld\n",
            ret,
            readdata.begin.tv_sec, readdata.begin.tv_nsec,
            readdata.realtime.tv_sec, readdata.realtime.tv_nsec, readdata.diff_real_b_ns,
            readdata.end.tv_sec, readdata.end.tv_nsec, readdata.handle_e_b_ns);
        _log_info("SyncRe[%d]! b/r/diff, e/hdl: %lld.%09ld/%lld.%09ld/%ld, %lld.%09ld/%ld\n",
            ret,
            readdata.begin.tv_sec, readdata.begin.tv_nsec,
            readdata.realtime.tv_sec, readdata.realtime.tv_nsec, readdata.diff_real_b_ns,
            readdata.end.tv_sec, readdata.end.tv_nsec, readdata.handle_e_b_ns);
    }

    /* 操作结束后关闭文件 */
    close(fd);
    log_fini();
    return 0;
}

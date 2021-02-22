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
#include <errno.h>
#include "log.h"
#include <sys/mman.h>
#include "common.h"

/** ===================================================== **
 * MACRO
 ** ===================================================== **/
#define VERSION "v1.4"

/** ===================================================== **
 * STRUCT
 ** ===================================================== **/
struct timespec64
{
    long long tv_sec; /* seconds */
    long tv_nsec;     /* nanoseconds */
};

struct sync_data_s
{
    struct timespec64 begin;    //起始系统时间
    struct timespec64 end;      //结束系统时间
    struct timespec64 realtime; //真实时间-读取自ps-timer寄存器
    long diff_real_b_ns;        //真实时间与起始系统时间差值，用于精度判断
    long handle_e_b_ns;         //结束系统时间与起始系统时间差值，用于操作时间计算
};

/** ===================================================== **
 * VARIABLE
 ** ===================================================== **/
static int dev_fd;
static int loop = 50;
static unsigned char *bram_map_base;
static A53State_s *s_pstA53State;

/** ===================================================== **
 * FUNCTION
 ** ===================================================== **/
void ctrl_c_handler(int signum, siginfo_t *info, void *myact)
{
    loop = 0;
    printf("! CTRL+C %d %p %p  %d\n", signum, info, myact, loop);
}

void print_err(char *str, int line, int err_no)
{
    fprintf(stderr, "%d, %s :%s\n", line, str, strerror(err_no));
    _exit(-1);
}

int main(int argc, char **argv)
{
    int fd;
    int ret;
    unsigned int time_period;
    struct sync_data_s readdata;

    if (0 != log_init("/etc/common/zlog.conf"))
    {
        print_err("parse log config failed, please check zlog.conf", __LINE__, errno);
        return -1;
    }

    /*ctrl + c*/
    struct sigaction act;
    sigemptyset(&act.sa_mask);
    act.sa_flags = SA_SIGINFO;
    act.sa_sigaction = ctrl_c_handler;

    if (sigaction(SIGINT, &act, NULL) < 0)
    {
        print_err("install sigal error", __LINE__, errno);
    }

    fprintf(stdout, "timesync-test: %s\n", VERSION);

    /* 验证输入参数个数 */
    if (3 != argc)
    {
        fprintf(stdout, "usage: %s <full_path_dev_name> <time_period_ms>\n", argv[0]);
        return -1;
    }

    /* 打开输入的设备文件, 获取文件句柄 */
    fd = open(argv[1], O_RDWR);
    if (fd < 0)
    {
        /* 打开文件失败 */
        fprintf(stderr, "Can't open file %s\r\n", argv[1]);
        return -1;
    }

    time_period = atoi(argv[2]);
    fprintf(stdout, "write[%ld] %d to dev:%s\n", sizeof(time_period), time_period, argv[1]);
    write(fd, &time_period, sizeof(time_period));

    /* 3, mapping pl state mem */
    dev_fd = open("/dev/mem", O_RDWR | O_NDELAY | O_DSYNC);
    if (0 > dev_fd)
    {
        close(fd);
        print_err("open /dev/mem failed", __LINE__, errno);
    }

    /* 4, mapping bram mem */
    printf("bram_map_base: 0x%lx, size: 0x%x\n", BRAM_BASE_ADDR, BRAM_MAX_SIZE);
    bram_map_base = (unsigned char *)mmap(NULL, BRAM_MAX_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, dev_fd, BRAM_BASE_ADDR);
    if (-1 == (unsigned long)bram_map_base)
    {
        close(dev_fd);
        close(fd);
        print_err("mmap failed", __LINE__, errno);
    }
    s_pstA53State = (A53State_s *)(bram_map_base + BRAM_A53_STATE_BASE_ADDR - BRAM_BASE_ADDR);

    while (loop)
    {
        sleep(2);
        ret = read(fd, &readdata, sizeof(readdata));

        s_pstA53State->uiTimeSyncBeginSec = (uint32_t)readdata.begin.tv_sec;
        s_pstA53State->uiTimeSyncBeginNsec = (uint32_t)readdata.begin.tv_nsec;
        s_pstA53State->uiTimeSyncRealSec = (uint32_t)readdata.realtime.tv_sec;
        s_pstA53State->uiTimeSyncRealNsec = (uint32_t)readdata.realtime.tv_nsec;
        s_pstA53State->uiTimeSyncEndSec = (uint32_t)readdata.end.tv_sec;
        s_pstA53State->uiTimeSyncEndNsec = (uint32_t)readdata.end.tv_nsec;
        s_pstA53State->uiTimeSyncDiffR2B = (uint32_t)readdata.diff_real_b_ns;
        s_pstA53State->uiTimeSyncDiffE2B = (uint32_t)readdata.handle_e_b_ns;

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
    close(dev_fd);
    close(fd);
    log_fini();
    return 0;
}

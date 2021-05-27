/** ===================================================== **
 *File : task-state.c
 *Brief : state data collect and send by UDP
 *Author : Momenta founderHAN
 *Created: 2021-1-21
 *Version: 1.0
 ** ===================================================== **/

/** ===================================================== **
 * INCLUDE
 ** ===================================================== **/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <errno.h>
#include <pthread.h>
#include <signal.h>
#include <time.h>
#include <fcntl.h>
#include <stdint.h>
#include <sys/mman.h>
#include "common.h"
#include "mem-mmap.h"

/** ===================================================== **
 * MACRO
 ** ===================================================== **/
#define TASK_STATE_VERSION "v1.8"

#define STATE_PL_SIZE (0x200)
#define STATE_A53_SIZE (0x180)
#define STATE_R5_SIZE (0x180)

/** ===================================================== **
 * STRUCT
 ** ===================================================== **/
typedef struct _state_data_s
{
    uint8_t aucStateA53[STATE_A53_SIZE];
    uint8_t aucStateR5[STATE_R5_SIZE];
    uint8_t aucStatePl[STATE_PL_SIZE];
} STATE_DATA_S;

/** ===================================================== **
 * VARIABLE
 ** ===================================================== **/
int cfd = -1;
static int loop = 50;

static STATE_DATA_S s_stState = {0};
static BramPtr_s s_stBram;
static PlStatePtr_s s_stPlState;

/** ===================================================== **
 * FUNCTION
 ** ===================================================== **/
void ctrl_c_handler(int signum, siginfo_t *info, void *myact)
{
    loop = 0;
    fprintf(stdout, "! CTRL+C %d %p %p  %d\n", signum, info, myact, loop);
}

void print_err(char *str, int line, int err_no)
{
    fprintf(stderr, "%d, %s :%s\n", line, str, strerror(err_no));
    _exit(-1);
}

void self_test(void)
{

}

int main()
{
    struct sockaddr_in addr0;
    int ret = -1;
    unsigned int uiCnt = 0;
    unsigned int period = 1;

    /*1, ctrl + c*/
    struct sigaction act;
    sigemptyset(&act.sa_mask);
    act.sa_flags = SA_SIGINFO;
    act.sa_sigaction = ctrl_c_handler;

    if (sigaction(SIGINT, &act, NULL) < 0)
    {
        print_err("install sigal error", __LINE__, errno);
    }

    fprintf(stdout, "task-state: %s\n", TASK_STATE_VERSION);

    /* 2, mapping pl state mem */
    if (0 != MAP_PlStateOpen(&s_stPlState))
    {
        print_err("MAP_PlStateOpen failed", __LINE__, errno);
    }

    /* 3, mapping bram mem */
    if (0 != MAP_BlockRamOpen(&s_stBram))
    {
        MAP_PlStateClose(&s_stPlState);
        print_err("MAP_BlockRamOpen failed", __LINE__, errno);
    }

    /* 4, create socket */
    cfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (-1 == cfd)
    {
        print_err("socket failed", __LINE__, errno);
    }

    addr0.sin_family = AF_INET; // tcp protocol family
    addr0.sin_port = htons(s_stBram.pstA53Data->usPortState); // port
    addr0.sin_addr.s_addr = htonl(INADDR_ANY); // ip addr
    ret = bind(cfd, (struct sockaddr *)&addr0, sizeof(addr0));
    if (-1 == ret)
    {
        print_err("bind failed", __LINE__, errno);
    }
    addr0.sin_addr.s_addr = inet_addr(s_stBram.pstA53Data->acIpAddrDest); // ip

    period = s_stBram.pstA53Data->ucStateUdpPeriodS;
    /* 4, periodly collect state and send */
    while (loop)
    {
        sleep(period);

        /* a, collect pl state */
        memcpy(s_stState.aucStateA53, (uint8_t *)s_stBram.pstA53State, STATE_A53_SIZE);
        memcpy(s_stState.aucStateR5, (uint8_t *)s_stBram.pstR5State, STATE_R5_SIZE);
        memcpy(s_stState.aucStatePl, s_stPlState.pucBase, STATE_PL_SIZE);

        /* b, send */
        // printf("send state %d\n", uiCnt++);
        ret = sendto(cfd, (void *)&s_stState, sizeof(s_stState), 0, (struct sockaddr *)&addr0, sizeof(addr0));
        if (-1 == ret)
        {
            printf("%d, accept failed :%s\n", __LINE__, strerror(errno));
            break;
        }
    }

    /* 5, close app */
    MAP_BlockRamClose(&s_stBram);
    MAP_PlStateClose(&s_stPlState);
    close(cfd);
    printf("task-state exit!\n");

    return 0;
}

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

/** ===================================================== **
 * MACRO
 ** ===================================================== **/
#define TASK_STATE_VERSION "v1.2"

#define STATE_IP "192.168.2.1"
#define STATE_PORT (8000)

#define STATE_PL_ADDR (0x80010000)
#define STATE_PL_SIZE (0x300)
#define STATE_A53_SIZE (0x100)
#define STATE_R5_SIZE (0x100)

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
static STATE_DATA_S s_stState = {0};
unsigned int *map_base;
unsigned long base;
static int dev_fd;
static int loop = 50;

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

int main()
{
    struct sockaddr_in addr0;
    int ret = -1;
    unsigned int uiCnt = 0;

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

    /* 2, create socket */
    cfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (-1 == cfd)
    {
        print_err("socket failed", __LINE__, errno);
    }

    addr0.sin_family = AF_INET;                  // tcp protocol family
    addr0.sin_port = htons(STATE_PORT);          // port
    addr0.sin_addr.s_addr = inet_addr(STATE_IP); // ip

    /* 3, mapping pl state mem */
    dev_fd = open("/dev/mem", O_RDWR | O_NDELAY | O_DSYNC);
    if (0 > dev_fd)
    {
        close(cfd);
        print_err("open /dev/mem failed", __LINE__, errno);
    }

    printf("map base: 0x%lx, size: 0x%x\n", STATE_PL_ADDR, STATE_PL_SIZE);
    map_base = (unsigned int *)mmap(NULL, STATE_PL_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, dev_fd, STATE_PL_ADDR);
    if (-1 == (unsigned long)map_base)
    {
        close(dev_fd);
        close(cfd);
        print_err("mmap failed", __LINE__, errno);
    }

    /* 4, periodly collect state and send */
    while (loop)
    {
        sleep(1);

        /* a, collect pl state */
        memcpy(s_stState.aucStatePl, map_base, STATE_PL_SIZE);

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
    munmap(map_base, STATE_PL_SIZE);
    close(dev_fd);
    close(cfd);
    printf("task-state exit!\n");

    return 0;
}

/** ===================================================== **
 *Author : Momenta founderHAN
 *Created: 2021-2-6
 *Version: 1.0
 ** ===================================================== **/

/** ===================================================== **
 * INCLUDE
 ** ===================================================== **/
#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <time.h>
#include <fcntl.h>
#include <signal.h>
#include <pthread.h>
#include <string.h>
#include <linux/rpmsg.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/time.h>
#include "log.h"
#include "mem-mmap.h"

/** ===================================================== **
 * MACRO
 ** ===================================================== **/
#define VERSION "v1.24"

#define RPMSG_GET_KFIFO_SIZE 1
#define RPMSG_GET_AVAIL_DATA_SIZE 2
#define RPMSG_GET_FREE_SPACE 3

#define RPMSG_HEADER_LEN 16
#define MAX_RPMSG_BUFF_SIZE (2048 - RPMSG_HEADER_LEN)

#define RPMSG_BUS_SYS "/sys/bus/rpmsg"

#define DATA_SENSOR_HEADER_LEN 4        /* usUdpPort + usReserved */
#define DATA_SENSOR_CONTROL_LEN 16      /* ucHeadHigh + ucHeadLow + ucType + ucCrc + uiTimeSec + uiTimeNsec + uiDataLen */

/** ===================================================== **
 * STRUCT
 ** ===================================================== **/
typedef struct DATA_PerfAnalyse
{
    unsigned int uiCnt;               /* frame count */
    int iDelayAmpUs;                 /* time delay between data timestamp to a53 recv */
    int iDelayAmpMaxUs;              /* max time delay between data timestamp to a53 recv */
    unsigned short usFreqInteger;     /* Integer of Frequence */
    unsigned short usFreqDecimal;     /* Decimal of Frequence */
    unsigned int uiLastDataTimeSec;   /* last data timestamp, sec */
    unsigned int uiLastDataTimeNsec;  /* last data timestamp, nsec */

    int iDelayUdpUs;
    int iDelayUdpMaxUs;
    struct timeval stLastTimeRecv;    /* last recv data system time */
    unsigned int uiLastCnt;           /* last analyse cnt */
    struct timeval stLastTimeAnalyse; /* last analyse system time */
    unsigned int uiCntPerf;
} DATA_PerfAnalyse_S;

/** ===================================================== **
 * VARIABLE
 ** ===================================================== **/
static const uint8_t sc_StateIndex[SENSOR_TYPE_NUM] =
    {
        STATE_CAN3,            // MASTER_CAN3 = 0x00,
        STATE_CAN1,            // MASTER_CAN1,
        STATE_CAN2,            // MASTER_CAN2,
        STATE_UART1,           // SLAVE_UART1,
        STATE_UART2,           // SLAVE_UART2,
        STATE_UART3,           // SLAVE_UART3,
        STATE_UART4,           // SLAVE_UART4,
        STATE_CAN4,            // SLAVE_CAN4,
        STATE_CAN5,            // SLAVE_CAN5,
        STATE_SENSOR_TYPE_NUM, // MOMENTA_TOTAL,
        STATE_SENSOR_TYPE_NUM, // MOMENTA_POWER,
        STATE_SENSOR_TYPE_NUM, // MOMENTA_POWERKEY,
        STATE_SENSOR_TYPE_NUM, // MOMENTA_WIRE_BT,
        STATE_SENSOR_TYPE_NUM, // MOMENTA_WIRE_SS,
        STATE_SENSOR_TYPE_NUM, // MOMENTA_SENSORHUB_F7,
        STATE_SENSOR_TYPE_NUM, // MOMENTA_SENSORHUB_F4,

        STATE_SENSOR_TYPE_NUM, // SENSOR_DATA_TYPE_COUNT,
        STATE_UART5,           // SLAVE_UART5,
        STATE_SENSOR_TYPE_NUM, // SLAVE_UART6,
        STATE_SENSOR_TYPE_NUM, // SLAVE_UART7,
        STATE_SENSOR_TYPE_NUM, // SLAVE_UART8,
        STATE_SENSOR_TYPE_NUM, // SLAVE_UART9,
        STATE_SENSOR_TYPE_NUM, // SLAVE_UART10,
        STATE_SENSOR_TYPE_NUM, // SLAVE_UART11,
        STATE_SENSOR_TYPE_NUM, // SLAVE_UART12,
        STATE_SENSOR_TYPE_NUM, // SLAVE_UART13,
        STATE_SENSOR_TYPE_NUM, // SLAVE_UART14,
        STATE_SENSOR_TYPE_NUM, // SLAVE_UART15,
        STATE_SENSOR_TYPE_NUM, // SLAVE_UART16,
        STATE_SENSOR_TYPE_NUM, // SLAVE_CAN6,
        STATE_SENSOR_TYPE_NUM, // SLAVE_CAN7,
        STATE_SENSOR_TYPE_NUM, // SLAVE_CAN8,
        STATE_SENSOR_TYPE_NUM, // SLAVE_CAN9,
        STATE_SENSOR_TYPE_NUM, // SLAVE_CAN10,
        STATE_SENSOR_TYPE_NUM, // SLAVE_CAN11,
        STATE_SENSOR_TYPE_NUM, // SLAVE_CAN12,
        STATE_SENSOR_TYPE_NUM, // SLAVE_CAN13,
        STATE_SENSOR_TYPE_NUM, // SLAVE_CAN14,
        STATE_SENSOR_TYPE_NUM, // SLAVE_CAN15,
        STATE_SENSOR_TYPE_NUM, // SLAVE_CAN16,
};

static int charfd = -1, eptfd = -1;
static int socket_fd = -1;

uint8_t aucRpmsgRecv[2048] = {0};
uint8_t aucRpmsgSend[2048] = {0};

static int loop = 50;

static int is_show_char = 1;
static int debug_level = 0;
static int delay_reset_period_cnt = 10;

static BramPtr_s s_stBram;
static DATA_PerfAnalyse_S astPerfUp[SENSOR_TYPE_NUM] = {0};
static DATA_PerfAnalyse_S astPerfDown[SENSOR_TYPE_NUM] = {0};

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
    printf("%d, %s :%s\n", line, str, strerror(err_no));
    _exit(-1);
}

void stop_remote(void)
{
    system("modprobe -r rpmsg_char");
}

static int rpmsg_create_ept(int rpfd, struct rpmsg_endpoint_info *eptinfo)
{
    int ret;

    ret = ioctl(rpfd, RPMSG_CREATE_EPT_IOCTL, eptinfo);
    if (ret)
        perror("Failed to create endpoint.\n");
    return ret;
}

static void rpmsg_destroy_ept(int rpfd)
{
    ioctl(rpfd, RPMSG_DESTROY_EPT_IOCTL);
}

static char *get_rpmsg_ept_dev_name(const char *rpmsg_char_name,
                                    const char *ept_name,
                                    char *ept_dev_name)
{
    char sys_rpmsg_ept_name_path[64];
    char svc_name[64];
    char *sys_rpmsg_path = "/sys/class/rpmsg";
    FILE *fp;
    int i;
    int ept_name_len;

    for (i = 0; i < 128; i++)
    {
        sprintf(sys_rpmsg_ept_name_path, "%s/%s/rpmsg%d/name",
                sys_rpmsg_path, rpmsg_char_name, i);
        printf("checking %s\n", sys_rpmsg_ept_name_path);
        if (access(sys_rpmsg_ept_name_path, F_OK) < 0)
            continue;
        fp = fopen(sys_rpmsg_ept_name_path, "r");
        if (!fp)
        {
            printf("failed to open %s\n", sys_rpmsg_ept_name_path);
            break;
        }
        fgets(svc_name, sizeof(svc_name), fp);
        fclose(fp);
        printf("svc_name: %s.\n", svc_name);
        ept_name_len = strlen(ept_name);
        if (ept_name_len > sizeof(svc_name))
            ept_name_len = sizeof(svc_name);
        if (!strncmp(svc_name, ept_name, ept_name_len))
        {
            sprintf(ept_dev_name, "rpmsg%d", i);
            return ept_dev_name;
        }
    }

    printf("Not able to RPMsg endpoint file for %s:%s.\n",
           rpmsg_char_name, ept_name);
    return NULL;
}

static int bind_rpmsg_chrdev(const char *rpmsg_dev_name)
{
    char fpath[256];
    char *rpmsg_chdrv = "rpmsg_chrdev";
    int fd;
    int ret;

    /* rpmsg dev overrides path */
    sprintf(fpath, "%s/devices/%s/driver_override",
            RPMSG_BUS_SYS, rpmsg_dev_name);
    fd = open(fpath, O_WRONLY);
    if (fd < 0)
    {
        fprintf(stdout, "Failed to open %s, %s\n",
                fpath, strerror(errno));
        return -EINVAL;
    }
    ret = write(fd, rpmsg_chdrv, strlen(rpmsg_chdrv) + 1);
    if (ret < 0)
    {
        fprintf(stdout, "Failed to write %s to %s, %s\n",
                rpmsg_chdrv, fpath, strerror(errno));
        return -EINVAL;
    }
    close(fd);

    /* bind the rpmsg device to rpmsg char driver */
    sprintf(fpath, "%s/drivers/%s/bind", RPMSG_BUS_SYS, rpmsg_chdrv);
    fd = open(fpath, O_WRONLY);
    if (fd < 0)
    {
        fprintf(stdout, "Failed to open %s, %s\n",
                fpath, strerror(errno));
        return -EINVAL;
    }
    ret = write(fd, rpmsg_dev_name, strlen(rpmsg_dev_name) + 1);
    if (ret < 0)
    {
        fprintf(stdout, "Failed to write %s to %s, %s\n",
                rpmsg_dev_name, fpath, strerror(errno));
        return -EINVAL;
    }
    close(fd);
    return 0;
}

static int get_rpmsg_chrdev_fd(const char *rpmsg_dev_name,
                               char *rpmsg_ctrl_name)
{
    char dpath[256];
    char fpath[256];
    char *rpmsg_ctrl_prefix = "rpmsg_ctrl";
    DIR *dir;
    struct dirent *ent;
    int fd;

    sprintf(dpath, "%s/devices/%s/rpmsg", RPMSG_BUS_SYS, rpmsg_dev_name);
    dir = opendir(dpath);
    if (dir == NULL)
    {
        fprintf(stdout, "Failed to open dir %s\n", dpath);
        return -EINVAL;
    }
    while ((ent = readdir(dir)) != NULL)
    {
        if (!strncmp(ent->d_name, rpmsg_ctrl_prefix,
                     strlen(rpmsg_ctrl_prefix)))
        {
            printf("Opening file %s.\n", ent->d_name);
            sprintf(fpath, "/dev/%s", ent->d_name);
            fd = open(fpath, O_RDWR | O_NONBLOCK);
            if (fd < 0)
            {
                fprintf(stdout, "Failed to open rpmsg char dev %s,%s\n",
                        fpath, strerror(errno));
                return fd;
            }
            sprintf(rpmsg_ctrl_name, "%s", ent->d_name);
            return fd;
        }
    }

    fprintf(stdout, "No rpmsg char dev file is found\n");
    return -EINVAL;
}

static int init_rpmsg(void)
{
    int ret;
    char *rpmsg_dev = "virtio0.rpmsg-openamp-demo-channel.-1.0";
    char fpath[256];
    char rpmsg_char_name[16];
    struct rpmsg_endpoint_info eptinfo;
    char ept_dev_name[16];
    char ept_dev_path[32];

    /* Load rpmsg_char driver */
    printf("\r\nMaster>probe rpmsg_char\r\n");
    ret = system("modprobe rpmsg_char");
    if (ret < 0)
    {
        perror("Failed to load rpmsg_char driver.\n");
        return -EINVAL;
    }

    printf("\r\n Open rpmsg dev %s! \r\n", rpmsg_dev);
    sprintf(fpath, "%s/devices/%s", RPMSG_BUS_SYS, rpmsg_dev);
    if (access(fpath, F_OK))
    {
        fprintf(stdout, "Not able to access rpmsg device %s, %s\n", fpath, strerror(errno));
        stop_remote();
        return -EINVAL;
    }
    ret = bind_rpmsg_chrdev(rpmsg_dev);
    if (ret < 0)
    {
        stop_remote();
        return ret;
    }
    charfd = get_rpmsg_chrdev_fd(rpmsg_dev, rpmsg_char_name);
    if (charfd < 0)
    {
        stop_remote();
        return charfd;
    }

    /* Create endpoint from rpmsg char driver */
    strcpy(eptinfo.name, "rpmsg-openamp-demo-channel");
    eptinfo.src = 0;
    eptinfo.dst = 0xFFFFFFFF;
    ret = rpmsg_create_ept(charfd, &eptinfo);
    if (ret)
    {
        printf("failed to create RPMsg endpoint.\n");
        close(charfd);
        stop_remote();
        return -EINVAL;
    }
    if (!get_rpmsg_ept_dev_name(rpmsg_char_name, eptinfo.name, ept_dev_name))
    {
        rpmsg_destroy_ept(charfd);
        close(charfd);
        stop_remote();
        return -EINVAL;
    }
    sprintf(ept_dev_path, "/dev/%s", ept_dev_name);
    eptfd = open(ept_dev_path, O_RDWR | O_NONBLOCK);
    if (eptfd < 0)
    {
        perror("Failed to open rpmsg device.");
        rpmsg_destroy_ept(charfd);
        close(charfd);
        stop_remote();
        return -1;
    }

    return 0;
}

static void exit_rpmsg(void)
{
    close(eptfd);
    rpmsg_destroy_ept(charfd);
    if (charfd >= 0)
        close(charfd);

    stop_remote();
}

//接收线程函数
void *receive(void *pth_arg)
{
    int ret, i, iLen;
    int bytes_sent;
    struct sockaddr_in addr = {0};
    int addr_size = sizeof(addr);
    DATA_Sensor_S *pstSensor;
    struct timeval start_time, current_time;
    float fDiffTime;

    printf("start wait for socket msg\r\n");
    //从对端ip和端口号中接收消息，指定addr0用于存放消息
    while (loop)
    {
        bzero(aucRpmsgSend, sizeof(aucRpmsgSend));
        ret = recvfrom(socket_fd, aucRpmsgSend + DATA_SENSOR_HEADER_LEN, sizeof(aucRpmsgSend) - DATA_SENSOR_HEADER_LEN, 0, (struct sockaddr *)&addr, &addr_size);
        if (-1 == ret)
        {
            fprintf(stdout, "%d, socket recv failed, %s\r\n", __LINE__, strerror(errno));
        }
        else if (ret > 0)
        {
            iLen = ret;

            if (4 < debug_level)
            {
                printf("\r\nip %s,port %d\r\n", inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));
                printf("==send[%d], hex:\r\n", iLen);
                for (i = 0; i < iLen; i++)
                {
                    printf("%02x ", aucRpmsgSend[DATA_SENSOR_HEADER_LEN + i]);
                }
                printf("\r\n");
            }

            gettimeofday(&start_time, NULL);

            bytes_sent = write(eptfd, aucRpmsgSend + DATA_SENSOR_HEADER_LEN, iLen);
            if (bytes_sent <= 0)
            {
                pstSensor = (DATA_Sensor_S *)aucRpmsgSend;
                fprintf(stdout, "%d, Error sending data, %s, type/len, %d/%d\r\n", __LINE__, strerror(errno), pstSensor->ucType, iLen);
                fprintf(stderr, "%d, Error sending data, %s, type/len, %d/%d\r\n", __LINE__, strerror(errno), pstSensor->ucType, iLen);
                continue;
            }

            if (4 < debug_level)
            {
                printf("==sent done: %d\r\n", bytes_sent);
            }

            //performance analyse
            /* check length and header skip if too short */
            pstSensor = (DATA_Sensor_S *)aucRpmsgSend;
            if ((DATA_SENSOR_CONTROL_LEN > iLen)
             || (0xAA != pstSensor->ucHeadHigh)
             || (0x55 != pstSensor->ucHeadLow)
             || (SENSOR_TYPE_NUM <= pstSensor->ucType))
            {
                continue;
            }

            if (0 == ((++astPerfDown[pstSensor->ucType].uiCnt) % s_stBram.pstA53Data->usSensorAnalysePerid))
            {
                gettimeofday(&current_time, NULL);
                astPerfDown[pstSensor->ucType].iDelayUdpUs = (start_time.tv_sec - pstSensor->uiTimeSec) * 1000000 + (start_time.tv_usec - pstSensor->uiTimeNsec / 1000);
                if (astPerfDown[pstSensor->ucType].iDelayUdpMaxUs < astPerfDown[pstSensor->ucType].iDelayUdpUs)
                {
                    astPerfDown[pstSensor->ucType].iDelayUdpMaxUs = astPerfDown[pstSensor->ucType].iDelayUdpUs;
                }
                astPerfDown[pstSensor->ucType].iDelayAmpUs = (current_time.tv_sec - start_time.tv_sec) * 1000000 + (current_time.tv_usec - start_time.tv_usec);
                if (astPerfDown[pstSensor->ucType].iDelayAmpMaxUs < astPerfDown[pstSensor->ucType].iDelayAmpUs)
                {
                    astPerfDown[pstSensor->ucType].iDelayAmpMaxUs = astPerfDown[pstSensor->ucType].iDelayAmpUs;
                }

                astPerfDown[pstSensor->ucType].uiLastDataTimeSec = pstSensor->uiTimeSec;
                astPerfDown[pstSensor->ucType].uiLastDataTimeNsec = pstSensor->uiTimeNsec;
                astPerfDown[pstSensor->ucType].stLastTimeRecv.tv_sec = current_time.tv_sec;
                astPerfDown[pstSensor->ucType].stLastTimeRecv.tv_usec = current_time.tv_usec;
            }
        }
    }
}

void *perf_analyse(void *pth_arg)
{
    int i;
    struct timeval current_time;
    float fDiffTime, fDataFreq;

    printf("start analyse thread\r\n");

    while (loop)
    {
        sleep(2);
        gettimeofday(&current_time, NULL);

        // uplink data
        for (i = 0; i < SENSOR_TYPE_NUM; i++)
        {
            fDiffTime = (current_time.tv_sec - astPerfUp[i].stLastTimeAnalyse.tv_sec) + (current_time.tv_usec - astPerfUp[i].stLastTimeAnalyse.tv_usec) / (float)1000000;
            fDataFreq = (astPerfUp[i].uiCnt - astPerfUp[i].uiLastCnt) / fDiffTime;
            astPerfUp[i].usFreqInteger = (unsigned short)fDataFreq;
            astPerfUp[i].usFreqDecimal = (unsigned short)((fDataFreq - astPerfUp[i].usFreqInteger) * 1000);
            if (0 == fDataFreq)
            {
                continue;
            }

            astPerfUp[i].uiLastCnt = astPerfUp[i].uiCnt;
            astPerfUp[i].stLastTimeAnalyse.tv_sec = current_time.tv_sec;
            astPerfUp[i].stLastTimeAnalyse.tv_usec = current_time.tv_usec;

            if (0 < debug_level)
            {
                fprintf(stdout, "UpData[%d] cnt/amp_d/amp_m/udp_d/udp_m/freq: %u/%d/%d/%d/%d/%d.%03d, last: %u.%06u->%d.%06ld\n",
                        i,
                        astPerfUp[i].uiCnt,
                        astPerfUp[i].iDelayAmpUs,
                        astPerfUp[i].iDelayAmpMaxUs,
                        astPerfUp[i].iDelayUdpUs,
                        astPerfUp[i].iDelayUdpMaxUs,
                        astPerfUp[i].usFreqInteger, astPerfUp[i].usFreqDecimal,
                        astPerfUp[i].uiLastDataTimeSec, astPerfUp[i].uiLastDataTimeNsec / 1000,
                        astPerfUp[i].stLastTimeRecv.tv_sec, astPerfUp[i].stLastTimeRecv.tv_usec);
            }

            if (STATE_SENSOR_TYPE_NUM != sc_StateIndex[i])
            {
                memcpy((uint8_t *)&s_stBram.pstA53State->astDataPerfUp[sc_StateIndex[i]],
                       (uint8_t *)&astPerfUp[i], sizeof(DATA_Perf_S));
            }

            _log_info("UpData[%d] cnt/amp_d/amp_m/udp_d/udp_m/freq: %u/%d/%d/%d/%d/%d.%03d, last: %u.%06u->%d.%06ld\n",
                      i,
                      astPerfUp[i].uiCnt,
                      astPerfUp[i].iDelayAmpUs,
                      astPerfUp[i].iDelayAmpMaxUs,
                      astPerfUp[i].iDelayUdpUs,
                      astPerfUp[i].iDelayUdpMaxUs,
                      astPerfUp[i].usFreqInteger, astPerfUp[i].usFreqDecimal,
                      astPerfUp[i].uiLastDataTimeSec, astPerfUp[i].uiLastDataTimeNsec / 1000,
                      astPerfUp[i].stLastTimeRecv.tv_sec, astPerfUp[i].stLastTimeRecv.tv_usec);

            if (delay_reset_period_cnt && (0 == ((++astPerfUp[i].uiCntPerf) % delay_reset_period_cnt)))
            {
                if (0 < debug_level)
                {
                    fprintf(stdout, ">up[%d] reset cnt, %d/%d/%d/%d/%d/%d\n",
                            i, astPerfUp[i].uiCntPerf, delay_reset_period_cnt,
                            astPerfUp[i].iDelayAmpMaxUs, astPerfUp[i].iDelayAmpUs,
                            astPerfUp[i].iDelayUdpMaxUs, astPerfUp[i].iDelayUdpUs);
                }
                astPerfUp[i].iDelayAmpMaxUs = astPerfUp[i].iDelayAmpUs;
                astPerfUp[i].iDelayUdpMaxUs = astPerfUp[i].iDelayUdpUs;
            }
        }

        // downlink data
        for (i = 0; i < SENSOR_TYPE_NUM; i++)
        {
            fDiffTime = (current_time.tv_sec - astPerfDown[i].stLastTimeAnalyse.tv_sec) + (current_time.tv_usec - astPerfDown[i].stLastTimeAnalyse.tv_usec) / (float)1000000;
            fDataFreq = (astPerfDown[i].uiCnt - astPerfDown[i].uiLastCnt) / fDiffTime;
            astPerfDown[i].usFreqInteger = (unsigned short)fDataFreq;
            astPerfDown[i].usFreqDecimal = (unsigned short)((fDataFreq - astPerfDown[i].usFreqInteger) * 1000);
            if (0 == fDataFreq)
            {
                continue;
            }

            astPerfDown[i].uiLastCnt = astPerfDown[i].uiCnt;
            astPerfDown[i].stLastTimeAnalyse.tv_sec = current_time.tv_sec;
            astPerfDown[i].stLastTimeAnalyse.tv_usec = current_time.tv_usec;

            if (0 < debug_level)
            {
                fprintf(stdout, "DownData[%d] cnt/amp_d/amp_m/udp_d/udp_m/freq: %u/%d/%d/%d/%d/%d.%03d, last: %u.%06u->%d.%06ld\n",
                        i,
                        astPerfDown[i].uiCnt,
                        astPerfDown[i].iDelayAmpUs,
                        astPerfDown[i].iDelayAmpMaxUs,
                        astPerfDown[i].iDelayUdpUs,
                        astPerfDown[i].iDelayUdpMaxUs,
                        astPerfDown[i].usFreqInteger, astPerfDown[i].usFreqDecimal,
                        astPerfDown[i].uiLastDataTimeSec, astPerfDown[i].uiLastDataTimeNsec / 1000,
                        astPerfDown[i].stLastTimeRecv.tv_sec, astPerfDown[i].stLastTimeRecv.tv_usec);
            }

            if (STATE_SENSOR_TYPE_NUM != sc_StateIndex[i])
            {
                memcpy((uint8_t *)&s_stBram.pstA53State->astDataPerfDown[sc_StateIndex[i]],
                       (uint8_t *)&astPerfDown[i], sizeof(DATA_Perf_S));
            }

            _log_info("DownData[%d] cnt/amp_d/amp_m/udp_d/udp_m/freq: %u/%d/%d/%d/%d/%d.%03d, last: %u.%06u->%d.%06ld\n",
                      i,
                      astPerfDown[i].uiCnt,
                      astPerfDown[i].iDelayAmpUs,
                      astPerfDown[i].iDelayAmpMaxUs,
                      astPerfDown[i].iDelayUdpUs,
                      astPerfDown[i].iDelayUdpMaxUs,
                      astPerfDown[i].usFreqInteger, astPerfDown[i].usFreqDecimal,
                      astPerfDown[i].uiLastDataTimeSec, astPerfDown[i].uiLastDataTimeNsec / 1000,
                      astPerfDown[i].stLastTimeRecv.tv_sec, astPerfDown[i].stLastTimeRecv.tv_usec);

            if (delay_reset_period_cnt && (0 == ((++astPerfDown[i].uiCntPerf) % delay_reset_period_cnt)))
            {
                if (0 < debug_level)
                {
                    fprintf(stdout, ">down[%d] reset cnt, %d/%d/%d/%d/%d/%d\n",
                            i, astPerfDown[i].uiCntPerf, delay_reset_period_cnt,
                            astPerfDown[i].iDelayAmpMaxUs, astPerfDown[i].iDelayAmpUs,
                            astPerfDown[i].iDelayUdpMaxUs, astPerfDown[i].iDelayUdpUs);
                }
                astPerfDown[i].iDelayAmpMaxUs = astPerfDown[i].iDelayAmpUs;
                astPerfDown[i].iDelayUdpMaxUs = astPerfDown[i].iDelayUdpUs;
            }
        }
    }
}

void display_help_msg(void)
{
    printf("\r\ntask-data\r\n");
    printf("-d  Debug level, 1-5\n");
    printf("-h  Displays this help message.\n");
}

int main(int argc, char *argv[])
{
    int i, bytes_rcvd;
    int ret;
    int opt;
    DATA_Sensor_S *pstSensor;
    unsigned short send_port;
    char *str_def = "hello r5!";
    struct timeval start_time, current_time;
    float fDiffTime;

    /*ctrl + c*/
    struct sigaction act;
    sigemptyset(&act.sa_mask);
    act.sa_flags = SA_SIGINFO;
    act.sa_sigaction = ctrl_c_handler;

    if (sigaction(SIGINT, &act, NULL) < 0)
    {
        fprintf(stdout, "install sigal error\n");
    }

    fprintf(stdout, "task-data: %s\n", VERSION);

    while ((opt = getopt(argc, argv, "d:h")) != -1)
    {
        switch (opt)
        {
        case 'd':
            debug_level = atoi(optarg);
            break;
        case 'h':
            display_help_msg();
            return 0;
        default:
            printf("getopt return unsupported option: -%c\n", opt);
            display_help_msg();
            return 0;
        }
    }

    if (0 != log_init("/etc/common/zlog.conf"))
    {
        print_err("parse log config failed, please check zlog.conf", __LINE__, errno);
    }

    // 0, mapping bram mem
    if (0 != MAP_BlockRamOpen(&s_stBram))
    {
        print_err("MAP_BlockRamOpen failed", __LINE__, errno);
    }

    //1, 创建tcp/ip协议族，指定通信方式为无链接不可靠的通信
    socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (-1 == socket_fd)
    {
        print_err("socket failed", __LINE__, errno);
    }

    //2, 进行端口号和ip的绑定
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;                                  //设置tcp协议族
    addr.sin_port = htons(s_stBram.pstA53Data->usPortDataDown); //设置端口号
    addr.sin_addr.s_addr = htonl(INADDR_ANY);                   //设置ip地址
    ret = bind(socket_fd, (struct sockaddr *)&addr, sizeof(addr));
    if (-1 == ret)
    {
        print_err("bind failed", __LINE__, errno);
    }

    //3, init rpmsg openamp
    ret = init_rpmsg();
    if (0 != ret)
    {
        print_err("init_rpmsg failed", __LINE__, errno);
    }

    //4, 创建线程函数，用于处理数据接收
    pthread_t pth_socket_recv;
    ret = pthread_create(&pth_socket_recv, NULL, receive, NULL);
    if (-1 == ret)
    {
        exit_rpmsg();
        print_err("pthread_create failed", __LINE__, errno);
    }

    //5, 创建analyse线程，性能分析
    delay_reset_period_cnt = s_stBram.pstA53Data->ucDelayResetPeriod;
    pthread_t pth_analyse;
    ret = pthread_create(&pth_analyse, NULL, perf_analyse, NULL);
    if (-1 == ret)
    {
        exit_rpmsg();
        print_err("pthread_create failed", __LINE__, errno);
    }

    //6, main task, receive r5 message, send by udp
    struct sockaddr_in addr0;
    addr0.sin_family = AF_INET;                                           //设置tcp协议族
    addr0.sin_port = htons(s_stBram.pstA53Data->usPortDataUp);            //设置端口号
    addr0.sin_addr.s_addr = inet_addr(s_stBram.pstA53Data->acIpAddrDest); //设置ip地址

    printf("start wait for r5 msg\r\n");
    if (0 >= write(eptfd, str_def, strlen(str_def)))
    {
        exit_rpmsg();
        print_err("rpmsg send failed", __LINE__, errno);
    }

    while (loop)
    {
        bytes_rcvd = read(eptfd, aucRpmsgRecv, sizeof(aucRpmsgRecv));
        if (0 >= bytes_rcvd)
            continue;

        gettimeofday(&start_time, NULL);

        if (1 < debug_level)
        {
            printf("\r\n==recv[%d], hex:\r\n", bytes_rcvd);
            for (i = 0; i < bytes_rcvd; i++)
            {
                printf("%02x ", aucRpmsgRecv[i]);
            }
            printf("\r\n\r\n");
        }

        if (2 < debug_level)
        {
            printf("\r\n==recv[%d], char:\r\n", bytes_rcvd);
            for (i = 0; i < bytes_rcvd; i++)
            {
                printf("%c", aucRpmsgRecv[i]);
            }
            printf("\r\n\r\n");
        }

        if (DATA_SENSOR_HEADER_LEN > bytes_rcvd)
        {
            continue;
        }

        //发送消息时需要绑定对方的ip和端口号
        pstSensor = (DATA_Sensor_S *)aucRpmsgRecv;
        addr0.sin_port = htons(pstSensor->usUdpPort);
        ret = sendto(socket_fd, aucRpmsgRecv + DATA_SENSOR_HEADER_LEN, bytes_rcvd - DATA_SENSOR_HEADER_LEN, 0, (struct sockaddr *)&addr0, sizeof(addr0));
        if (-1 == ret)
        {
            fprintf(stdout, "%d, socket send failed, %s\r\n", __LINE__, strerror(errno));
            continue;
        }

        //performance analyse
        /* check length and header skip if too short */
        if ((DATA_SENSOR_HEADER_LEN + DATA_SENSOR_CONTROL_LEN > bytes_rcvd)
         || (0xAA != pstSensor->ucHeadHigh)
         || (0x55 != pstSensor->ucHeadLow)
         || (SENSOR_TYPE_NUM <= pstSensor->ucType))
        {
            continue;
        }

        if (0 == ((++astPerfUp[pstSensor->ucType].uiCnt) % s_stBram.pstA53Data->usSensorAnalysePerid))
        {
            //skip udp port
            if (8772 == pstSensor->usUdpPort)
            {
                // printf(">>skip: %d/%d\n", pstSensor->usUdpPort, pstSensor->ucType);
                continue;
            }

            gettimeofday(&current_time, NULL);
            astPerfUp[pstSensor->ucType].iDelayAmpUs = (start_time.tv_sec - pstSensor->uiTimeSec) * 1000000 + (start_time.tv_usec - pstSensor->uiTimeNsec / 1000);
            if (astPerfUp[pstSensor->ucType].iDelayAmpMaxUs < astPerfUp[pstSensor->ucType].iDelayAmpUs)
            {
                astPerfUp[pstSensor->ucType].iDelayAmpMaxUs = astPerfUp[pstSensor->ucType].iDelayAmpUs;
            }

            astPerfUp[pstSensor->ucType].iDelayUdpUs = (current_time.tv_sec - start_time.tv_sec) * 1000000 + (current_time.tv_usec - start_time.tv_usec);
            if (astPerfUp[pstSensor->ucType].iDelayUdpMaxUs < astPerfUp[pstSensor->ucType].iDelayUdpUs)
            {
                astPerfUp[pstSensor->ucType].iDelayUdpMaxUs = astPerfUp[pstSensor->ucType].iDelayUdpUs;
            }

            astPerfUp[pstSensor->ucType].uiLastDataTimeSec = pstSensor->uiTimeSec;
            astPerfUp[pstSensor->ucType].uiLastDataTimeNsec = pstSensor->uiTimeNsec;
            astPerfUp[pstSensor->ucType].stLastTimeRecv.tv_sec = current_time.tv_sec;
            astPerfUp[pstSensor->ucType].stLastTimeRecv.tv_usec = current_time.tv_usec;
        }
    }

    printf("\r\n close...\r\n");
    shutdown(socket_fd, SHUT_RDWR);
    pthread_join(pth_analyse, NULL);
    pthread_join(pth_socket_recv, NULL);
    exit_rpmsg();
    MAP_BlockRamClose(&s_stBram);
    log_fini();

    return 0;
}

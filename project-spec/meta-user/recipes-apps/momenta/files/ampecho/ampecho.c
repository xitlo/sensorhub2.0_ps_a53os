/** ===================================================== **
 *Author : Momenta founderHAN
 *Created: 2021-1-5
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

/** ===================================================== **
 * MACRO
 ** ===================================================== **/
#define VERSION "v1.9"

#define RPMSG_GET_KFIFO_SIZE 1
#define RPMSG_GET_AVAIL_DATA_SIZE 2
#define RPMSG_GET_FREE_SPACE 3

#define RPMSG_HEADER_LEN 16
#define MAX_RPMSG_BUFF_SIZE (2048 - RPMSG_HEADER_LEN)

#define RPMSG_BUS_SYS "/sys/bus/rpmsg"

/** ===================================================== **
 * STRUCT
 ** ===================================================== **/

/** ===================================================== **
 * VARIABLE
 ** ===================================================== **/
static int charfd = -1, eptfd = -1;

uint8_t aucRecv[MAX_RPMSG_BUFF_SIZE] = {0};
uint8_t aucCmd[1024] = {0};

static int loop = 50;
static int is_show_char = 0;

/** ===================================================== **
 * FUNCTION
 ** ===================================================== **/
void ctrl_c_handler(int signum, siginfo_t *info, void *myact)
{
    loop = 0;
    printf("! CTRL+C %d %p %p  %d\n", signum, info, myact, loop);
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
        fprintf(stderr, "Failed to open %s, %s\n",
                fpath, strerror(errno));
        return -EINVAL;
    }
    ret = write(fd, rpmsg_chdrv, strlen(rpmsg_chdrv) + 1);
    if (ret < 0)
    {
        fprintf(stderr, "Failed to write %s to %s, %s\n",
                rpmsg_chdrv, fpath, strerror(errno));
        return -EINVAL;
    }
    close(fd);

    /* bind the rpmsg device to rpmsg char driver */
    sprintf(fpath, "%s/drivers/%s/bind", RPMSG_BUS_SYS, rpmsg_chdrv);
    fd = open(fpath, O_WRONLY);
    if (fd < 0)
    {
        fprintf(stderr, "Failed to open %s, %s\n",
                fpath, strerror(errno));
        return -EINVAL;
    }
    ret = write(fd, rpmsg_dev_name, strlen(rpmsg_dev_name) + 1);
    if (ret < 0)
    {
        fprintf(stderr, "Failed to write %s to %s, %s\n",
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
        fprintf(stderr, "Failed to open dir %s\n", dpath);
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
                fprintf(stderr,
                        "Failed to open rpmsg char dev %s,%s\n",
                        fpath, strerror(errno));
                return fd;
            }
            sprintf(rpmsg_ctrl_name, "%s", ent->d_name);
            return fd;
        }
    }

    fprintf(stderr, "No rpmsg char dev file is found\n");
    return -EINVAL;
}

/*************************************************
Function:       string2hex
Description:    字符串转换成hex,要求str只能是大写字母ABCDEF和数字
Input:          str:要转换的字符串
Output:         hex:转换后的hex字符数组
Return:         -1 不符合规则，失败
                >0 成功，转换的字节数
*************************************************/
static int string2hex(char *str, uint8_t *hex)
{
    int i = 0;
    int j = 0;
    uint8_t temp = 0;
    int str_len = 0;
    char str_cpy[128] = {'0'};
    strcpy(str_cpy, str);
    str_len = strlen(str_cpy);
    if (str_len == 0)
    {
        return -1;
    }
    while (i < str_len)
    {
        if (str_cpy[i] >= 'a' && str_cpy[i] <= 'f')
        {
            str_cpy[i] -= 0x20;
        }

        if (str_cpy[i] >= '0' && str_cpy[i] <= 'F')
        {
            if ((str_cpy[i] >= '0' && str_cpy[i] <= '9'))
            {
                temp = (str_cpy[i] & 0x0f) << 4;
            }
            else if (str_cpy[i] >= 'A' && str_cpy[i] <= 'F')
            {
                temp = ((str_cpy[i] + 0x09) & 0x0f) << 4;
            }
            else
            {
                return -1;
            }
        }
        else
        {
            return -1;
        }
        i++;

        if (str_cpy[i] >= 'a' && str_cpy[i] <= 'f')
        {
            str_cpy[i] -= 0x20;
        }

        if (str_cpy[i] >= '0' && str_cpy[i] <= 'F')
        {
            if (str_cpy[i] >= '0' && str_cpy[i] <= '9')
            {
                temp |= (str_cpy[i] & 0x0f);
            }
            else if (str_cpy[i] >= 'A' && str_cpy[i] <= 'F')
            {
                temp |= ((str_cpy[i] + 0x09) & 0x0f);
            }
            else
            {
                return -1;
            }
        }
        else
        {
            return -1;
        }
        i++;
        hex[j] = temp;
        //printf("%02x",temp);
        j++;
    }
    //printf("\n");
    return j;
}

//接收线程函数
void *receive(void *pth_arg)
{
    int i, bytes_rcvd;

    printf("start wait for r5 msg\r\n");
    while (loop)
    {
        bytes_rcvd = read(eptfd, aucRecv, sizeof(aucRecv));
        if (0 >= bytes_rcvd)
            continue;

        printf("\r\n==recv[%d], hex:\r\n", bytes_rcvd);
        for (i = 0; i < bytes_rcvd; i++)
        {
            printf("%02x ", aucRecv[i]);
        }

        if (is_show_char)
        {
            printf("\r\n==recv[%d], char:\r\n", bytes_rcvd);
            for (i = 0; i < bytes_rcvd; i++)
            {
                printf("%c", aucRecv[i]);
            }
        }
        printf("\r\n\r\n");
    }
}

void display_help_msg(void)
{
    printf("\r\nopenamp-test\r\n");
    printf("-c  Display char of msg received from r5.\n");
    printf("-h  Displays this help message.\n");
}

int main(int argc, char *argv[])
{
    int ret, i;
    int bytes_sent;
    int opt;
    char *rpmsg_dev = "virtio0.rpmsg-openamp-demo-channel.-1.0";
    char fpath[256];
    char rpmsg_char_name[16];
    struct rpmsg_endpoint_info eptinfo;
    char ept_dev_name[16];
    char ept_dev_path[32];
    int iLen;
    char acMsgStr[512] = {0};

    /*ctrl + c*/
    struct sigaction act;
    sigemptyset(&act.sa_mask);
    act.sa_flags = SA_SIGINFO;
    act.sa_sigaction = ctrl_c_handler;

    if (sigaction(SIGINT, &act, NULL) < 0)
    {
        fprintf(stderr, "install sigal error\n");
    }

    fprintf(stdout, "openamp-test: %s\n", VERSION);

    while ((opt = getopt(argc, argv, "ch")) != -1)
    {
        switch (opt)
        {
        case 'c':
            is_show_char = 1;
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
        fprintf(stderr, "Not able to access rpmsg device %s, %s\n", fpath, strerror(errno));
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

    //创建线程函数，用于处理数据接收
    printf("create thread for r5 msg recevie\r\n");
    pthread_t pth_r5_recv;
    ret = pthread_create(&pth_r5_recv, NULL, receive, NULL);
    if (-1 == ret)
    {
        printf("%d, pthread_create failed: %s\n", __LINE__, strerror(errno));
        rpmsg_destroy_ept(charfd);
        close(charfd);
        stop_remote();
        return -1;
    }

    printf("\r\npls input hex msg, e.g: ABCDEF0123456 ...\r\n");
    while (loop)
    {
        memset(acMsgStr, 0, sizeof(acMsgStr));
        memset(aucCmd, 0, sizeof(aucCmd));

        scanf("%s", acMsgStr);
        printf("get msg string: %s\n", acMsgStr);
        iLen = string2hex(acMsgStr, aucCmd);
        if (0 >= iLen)
        {
            printf("cmd err, please check!\n");
            continue;
        }

        printf("\r\n==send[%d], hex:\r\n", iLen);
        for (i = 0; i < iLen; i++)
        {
            printf("%02x ", aucCmd[i]);
        }
        printf("\r\n");

        bytes_sent = write(eptfd, aucCmd, iLen);
        if (bytes_sent <= 0)
        {
            printf("\r\n Error sending data\r\n");
            break;
        }
        printf("==sent done: %d\r\n", bytes_sent);
    }

    printf("\r\n close...\r\n");
    pthread_join(pth_r5_recv, NULL);

    close(eptfd);
    rpmsg_destroy_ept(charfd);
    if (charfd >= 0)
        close(charfd);

    stop_remote();

    return 0;
}

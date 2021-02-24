/** ===================================================== **
 *File : task-state.c
 *Brief : parse config file, json format
 *Author : Momenta founderHAN
 *Created: 2021-2-22
 *Version: 1.0
 ** ===================================================== **/

/** ===================================================== **
 * INCLUDE
 ** ===================================================== **/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <errno.h>
#include <pthread.h>
#include <signal.h>
#include <time.h>
#include <fcntl.h>
#include <stdint.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include "common.h"
#include "cJSON.h"

/** ===================================================== **
 * MACRO
 ** ===================================================== **/
#define CONFIG_PARSE_VERSION "v1.1"

/** ===================================================== **
 * STRUCT
 ** ===================================================== **/

/** ===================================================== **
 * VARIABLE
 ** ===================================================== **/
static int dev_fd;
static int loop = 50;
static int debug = 0;
static unsigned char *bram_map_base;
static A53State_s *s_pstA53State;
static A53Data_s *s_pstA53Data;
static unsigned char *s_pucR5State;
static cJSON *pstJsonRoot;

/** ===================================================== **
 * FUNCTION - INTERNAL - JSON
 ** ===================================================== **/
static int get_file_size(const char *path)
{
    struct stat buf;
    stat(path, &buf);
    printf("json file size:%ld\n", buf.st_size);
    return buf.st_size;
}

static int json_read(const char *path, char **json_ret)
{
    int filesize;
    FILE *fp = fopen(path, "rb");
    char *json;

    if (fp == NULL)
    {
        printf("json file open failed!,exit...\n");
        return -1;
    }

    filesize = get_file_size(path);
    json = (char *)malloc(filesize + 1);
    json[filesize] = '\0';

    if (1 > fread(json, filesize, 1, fp))
    {
        printf("fread failed!\n");
        return -1;
    }

    *json_ret = json;
    return 0;
}

static void json_read_done(char *pcJsonStr)
{
    if (NULL != pcJsonStr)
        free(pcJsonStr);
}

/** ===================================================== **
 * FUNCTION - INTERNAL
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

void display_help_msg(void)
{
    printf("\r\nconfig-parse\r\n");
    printf("-d  debug level.\n");
    printf("-p  config file fullpath.\n");
    printf("-h  Displays this help message.\n");
}

static int config_file_init(char *pcPath)
{
    char *pcJsonStr;

    if (0 > json_read(pcPath, &pcJsonStr))
    {
        fprintf(stderr, "%s: json_read err\n", __func__);
        return -1;
    }

    pstJsonRoot = cJSON_Parse(pcJsonStr);
    json_read_done(pcJsonStr);
    if (NULL == pstJsonRoot)
    {
        fprintf(stderr, "%s: conf json parse err\n", __func__);
        return -1;
    }

    return 0;
}

static void config_file_exit(void)
{
    cJSON_Delete(pstJsonRoot);
    printf("%s ok\n", __func__);
}

static int config_file_set_string(cJSON *pstJson, char *pcItem, char *pcDestAddr)
{
    char *pcString;

    if (cJSON_HasObjectItem(pstJson, pcItem))
    {
        pcString = cJSON_GetObjectItem(pstJson, pcItem)->valuestring;
        strcpy(pcDestAddr, pcString);
        printf("%s_mem: %s\n", pcItem, pcDestAddr);
    }
    else
    {
        fprintf(stderr, "%s: can't find item %s\n", __func__, pcItem);
        return -1;
    }

    return 0;
}

static int config_file_get_int(cJSON *pstJson, char *pcItem)
{
    int iVal;

    if (cJSON_HasObjectItem(pstJson, pcItem))
    {
        iVal = cJSON_GetObjectItem(pstJson, pcItem)->valueint;
        return iVal;
    }
    else
    {
        fprintf(stderr, "%s: can't find item %s\n", __func__, pcItem);
        return -1;
    }
}

static int config_file_a53(void)
{
    cJSON *pstJsonItem;
    char *pcItem, *pcString, *pcDestAddr;
    int iVal;

    pstJsonItem = cJSON_GetObjectItem(pstJsonRoot, "a53");
    if (NULL == pstJsonItem)
    {
        fprintf(stderr, "%s: conf json parse err\n", __func__);
        return -1;
    }

    /* 1, ip_local */
    if (0 != config_file_set_string(pstJsonItem, "ip_local", s_pstA53Data->acIpAddrLocal))
    {
        fprintf(stderr, "%s: conf json parse err\n", __func__);
        return -1;
    }

    /* 2, ip_dest */
    if (0 != config_file_set_string(pstJsonItem, "ip_dest", s_pstA53Data->acIpAddrDest))
    {
        fprintf(stderr, "%s: conf json parse err\n", __func__);
        return -1;
    }

    /* 3, port_data_up */
    iVal = config_file_get_int(pstJsonItem, "port_data_up");
    if (0 > iVal )
    {
        fprintf(stderr, "%s: conf json parse err\n", __func__);
        return -1;
    }
    s_pstA53Data->usPortDataUp = (uint16_t)iVal;

    /* 4, port_data_down */
    iVal = config_file_get_int(pstJsonItem, "port_data_down");
    if (0 > iVal )
    {
        fprintf(stderr, "%s: conf json parse err\n", __func__);
        return -1;
    }
    s_pstA53Data->usPortDataDown = (uint16_t)iVal;

    /* 5, port_state */
    iVal = config_file_get_int(pstJsonItem, "port_state");
    if (0 > iVal )
    {
        fprintf(stderr, "%s: conf json parse err\n", __func__);
        return -1;
    }
    s_pstA53Data->usPortState = (uint16_t)iVal;

    /* 6, time_sync_period_ms */
    iVal = config_file_get_int(pstJsonItem, "time_sync_period_ms");
    if (0 > iVal )
    {
        fprintf(stderr, "%s: conf json parse err\n", __func__);
        return -1;
    }
    s_pstA53Data->usTimeSyncPeriodMs = (uint16_t)iVal;


    return 0;
}

int main(int argc, char **argv)
{
    int opt;
    char config_file_path[256] = "";

    /*1, ctrl + c*/
    struct sigaction act;
    sigemptyset(&act.sa_mask);
    act.sa_flags = SA_SIGINFO;
    act.sa_sigaction = ctrl_c_handler;

    if (sigaction(SIGINT, &act, NULL) < 0)
    {
        print_err("install sigal error", __LINE__, errno);
    }

    fprintf(stdout, "config-parse: %s\n", CONFIG_PARSE_VERSION);

    /* 2, input parameters */
    while ((opt = getopt(argc, argv, "d:p:h")) != -1)
    {
        switch (opt)
        {
        case 'd':
            debug = atoi(optarg);
            break;
        case 'p':
            strcpy(config_file_path, optarg);
            break;
        case 'h':
            display_help_msg();
            return 0;
        default:
            fprintf(stdout, "getopt return unsupported option: -%c\n", opt);
            display_help_msg();
            return 0;
        }
    }

    if (0 == strlen(config_file_path))
    {
        display_help_msg();
        printf("no config file\n");
        return -1;
    }
    fprintf(stdout, "config file: %s\n", config_file_path);

    /* 3, mapping pl state mem */
    dev_fd = open("/dev/mem", O_RDWR | O_NDELAY | O_DSYNC);
    if (0 > dev_fd)
    {
        print_err("open /dev/mem failed", __LINE__, errno);
    }

    /* 4, mapping bram mem */
    printf("bram_map_base: 0x%lx, size: 0x%x\n", BRAM_BASE_ADDR, BRAM_MAX_SIZE);
    bram_map_base = (unsigned char *)mmap(NULL, BRAM_MAX_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, dev_fd, BRAM_BASE_ADDR);
    if (NULL == bram_map_base)
    {
        close(dev_fd);
        print_err("mmap failed", __LINE__, errno);
    }

    s_pstA53State = (A53State_s *)(bram_map_base + BRAM_A53_STATE_BASE_ADDR - BRAM_BASE_ADDR);
    s_pstA53Data = (A53Data_s *)(bram_map_base + BRAM_A53_DATA_BASE_ADDR - BRAM_BASE_ADDR);
    s_pucR5State = (unsigned char *)(bram_map_base + BRAM_R5_STATE_BASE_ADDR - BRAM_BASE_ADDR);
    // printf("state/data/r5state: 0x%lx/0x%lx/0x%lx\n", s_pstA53State, s_pstA53Data, s_pucR5State);
    memset((uint8_t *)s_pstA53Data, 0, BRAM_A53_DATA_SIZE);

    /* 5, read data from config file, set to BRAM */
    config_file_init(config_file_path);
    config_file_a53();
    config_file_exit();

    /* 6, close app */
    munmap(bram_map_base, BRAM_MAX_SIZE);
    close(dev_fd);
    printf("config-parse exit!\n");

    return 0;
}

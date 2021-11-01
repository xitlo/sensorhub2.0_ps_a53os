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
#include "mem-mmap.h"

/** ===================================================== **
 * MACRO
 ** ===================================================== **/
#define CONFIG_PARSE_VERSION "v1.13"

/** ===================================================== **
 * STRUCT
 ** ===================================================== **/

/** ===================================================== **
 * VARIABLE
 ** ===================================================== **/
static int loop = 50;
static int debug = 0;

static uint32_t uiVerA53;
static cJSON *pstJsonRoot;
static BramPtr_s s_stBram;  

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

static void set_a53_version(void)
{
    int ver_major, ver_minor;
    char cmd_str[128] = "";

    sscanf(VRESION_A53, "v%d.%d", &ver_major, &ver_minor);
    uiVerA53 = (uint32_t)(((ver_major & 0xFF) << 16) | ((ver_minor & 0xFF) << 8) | (VERSION_DEBUG & 0xFF));

    sprintf(cmd_str, "/sbin/devmem 0x%08x 32 0x%08x", VERSION_A53_REG_ADDR, uiVerA53);
    system(cmd_str);
    printf("\nA53 version: %s-%d, 0x%08x\n", VRESION_A53, VERSION_DEBUG, uiVerA53);

    memset(cmd_str, 0, sizeof(cmd_str));
    sprintf(cmd_str, "echo -n \"PL version: \" && /sbin/devmem 0x%08x", VERSION_PL_REG_ADDR);
    system(cmd_str);
    printf("\n");
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
        printf("==%s: %s\n", pcItem, pcDestAddr);
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
static int config_file_r5(void)
{
     cJSON *pstJsonItem, *pstJsonItemUart, *pstJsonItemUartItem, \
        *pstJsonItemCan, *pstJsonItemCanItem;
    char *pcItem;
    int iVal;
    int json_arr_size;

    s_stBram.pstR5Data->magic_num = R5_DATA_MAGIC_NUMBER;

    pstJsonItem = cJSON_GetObjectItem(pstJsonRoot, "r5");
    if (NULL == pstJsonItem)
    {
        fprintf(stderr, "%s: conf json parse err\n", __func__);
        return -1;
    }
//---set imu board en---
    s_stBram.pstR5Data->imu_boad_en = config_file_get_int(pstJsonItem, "imu_board_en");

//--------------------set uart config array--------------------------
    pstJsonItemUart = cJSON_GetObjectItem(pstJsonItem, "uart");
    if (NULL == pstJsonItemUart)
    {
        fprintf(stderr, "%s: conf json parse err\n", __func__);
        return -1;
    }

    json_arr_size = cJSON_GetArraySize(pstJsonItemUart);
    if(json_arr_size != INTERFACE_UART_NUMBER)
    {
        fprintf(stderr, "%s: conf json parse err, r5 uart number\n", __func__);
        return -1;
    }
    for(int i = 0;i < json_arr_size;i++)
    {
        pstJsonItemUartItem = cJSON_GetArrayItem(pstJsonItemUart, i);
        if (NULL == pstJsonItemUartItem)
        {
            fprintf(stderr, "%s: conf json parse err\n", __func__);
            return -1;
        }
        config_file_set_string(pstJsonItemUartItem, "name", s_stBram.pstR5Data->uart_config[i].name);
        s_stBram.pstR5Data->uart_config[i].enable = config_file_get_int(pstJsonItemUartItem, "enable");
        s_stBram.pstR5Data->uart_config[i].bps = config_file_get_int(pstJsonItemUartItem, "bps");
        s_stBram.pstR5Data->uart_config[i].id = config_file_get_int(pstJsonItemUartItem, "id");
        s_stBram.pstR5Data->uart_config[i].idle = config_file_get_int(pstJsonItemUartItem, "idle");
    }
//-----------------------set can config array------------------------
    pstJsonItemCan = cJSON_GetObjectItem(pstJsonItem, "can");
    if (NULL == pstJsonItemCan)
    {
        fprintf(stderr, "%s: conf json parse err\n", __func__);
        return -1;
    }

    json_arr_size = cJSON_GetArraySize(pstJsonItemCan);
    if(json_arr_size != INTERFACE_CAN_NUMBER)
    {
        fprintf(stderr, "%s: conf json parse err, r5 can number\n", __func__);
        return -1;
    }

     for(int i = 0;i < json_arr_size;i++)
    {
        pstJsonItemCanItem = cJSON_GetArrayItem(pstJsonItemCan, i);
        if (NULL == pstJsonItemCanItem)
        {
            fprintf(stderr, "%s: conf json parse err\n", __func__);
            return -1;
        }
        config_file_set_string(pstJsonItemCanItem, "name", s_stBram.pstR5Data->can_config[i].name);
        s_stBram.pstR5Data->can_config[i].enable = config_file_get_int(pstJsonItemCanItem, "enable");
        s_stBram.pstR5Data->can_config[i].id = config_file_get_int(pstJsonItemCanItem, "id");
    }

}
static int config_file_a53(void)
{
    cJSON *pstJsonItem;
    char *pcItem;
    int iVal;

    pstJsonItem = cJSON_GetObjectItem(pstJsonRoot, "a53");
    if (NULL == pstJsonItem)
    {
        fprintf(stderr, "%s: conf json parse err\n", __func__);
        return -1;
    }

    /* 1, ip_local */
    if (0 != config_file_set_string(pstJsonItem, "ip_local", s_stBram.pstA53Data->acIpAddrLocal))
    {
        fprintf(stderr, "%s: conf json parse err\n", __func__);
        return -1;
    }

    /* 2, ip_dest */
    if (0 != config_file_set_string(pstJsonItem, "ip_dest", s_stBram.pstA53Data->acIpAddrDest))
    {
        fprintf(stderr, "%s: conf json parse err\n", __func__);
        return -1;
    }

    /* 3, port_data_up */
    pcItem = "port_data_up";
    iVal = config_file_get_int(pstJsonItem, pcItem);
    if (0 > iVal)
    {
        fprintf(stderr, "%s: conf json parse err\n", __func__);
        return -1;
    }
    s_stBram.pstA53Data->usPortDataUp = (uint16_t)iVal;
    printf("==%s: %d\n", pcItem, s_stBram.pstA53Data->usPortDataUp);

    /* 4, port_data_down */
    pcItem = "port_data_down";
    iVal = config_file_get_int(pstJsonItem, pcItem);
    if (0 > iVal)
    {
        fprintf(stderr, "%s: conf json parse err\n", __func__);
        return -1;
    }
    s_stBram.pstA53Data->usPortDataDown = (uint16_t)iVal;
    printf("==%s: %d\n", pcItem, s_stBram.pstA53Data->usPortDataDown);

    /* 5, port_state */
    pcItem = "port_state";
    iVal = config_file_get_int(pstJsonItem, pcItem);
    if (0 > iVal)
    {
        fprintf(stderr, "%s: conf json parse err\n", __func__);
        return -1;
    }
    s_stBram.pstA53Data->usPortState = (uint16_t)iVal;
    printf("==%s: %d\n", pcItem, s_stBram.pstA53Data->usPortState);

    /* 6, time_sync_period_ms */
    pcItem = "time_sync_period_ms";
    iVal = config_file_get_int(pstJsonItem, "time_sync_period_ms");
    if (0 > iVal)
    {
        fprintf(stderr, "%s: conf json parse err\n", __func__);
        return -1;
    }
    s_stBram.pstA53Data->usTimeSyncPeriodMs = (uint16_t)iVal;
    printf("==%s: %d\n", pcItem, s_stBram.pstA53Data->usTimeSyncPeriodMs);

    /* 7, data_analyse_period */
    pcItem = "data_analyse_period";
    iVal = config_file_get_int(pstJsonItem, "data_analyse_period");
    if (0 > iVal)
    {
        fprintf(stderr, "%s: conf json parse err\n", __func__);
        return -1;
    }
    s_stBram.pstA53Data->usSensorAnalysePerid = (uint16_t)iVal;
    printf("==%s: %d\n", pcItem, s_stBram.pstA53Data->usSensorAnalysePerid);

    /* 8, state_udp_period_s */
    pcItem = "state_udp_period_s";
    iVal = config_file_get_int(pstJsonItem, "state_udp_period_s");
    if (0 > iVal)
    {
        fprintf(stderr, "%s: conf json parse err\n", __func__);
        return -1;
    }
    s_stBram.pstA53Data->ucStateUdpPeriodS = (uint8_t)iVal;
    printf("==%s: %d\n", pcItem, s_stBram.pstA53Data->ucStateUdpPeriodS);

    /* 9, data_delay_reset_period */
    pcItem = "data_delay_reset_period";
    iVal = config_file_get_int(pstJsonItem, "data_delay_reset_period");
    if (0 > iVal)
    {
        fprintf(stderr, "%s: conf json parse err\n", __func__);
        return -1;
    }
    s_stBram.pstA53Data->ucDelayResetPeriod = (uint8_t)iVal;
    printf("==%s: %d\n", pcItem, s_stBram.pstA53Data->ucDelayResetPeriod);

    return 0;
}

static void set_state_a53(void)
{
    /* 0, a53_version */
    s_stBram.pstA53State->uiA53Version = uiVerA53;
}

int main(int argc, char **argv)
{
    int opt;
    char config_file_path[256] = "";

    /*0, set a53 version*/
    set_a53_version();

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

    if (0 != MAP_BlockRamOpen(&s_stBram))
    {
        print_err("MAP_BlockRamOpen failed", __LINE__, errno);
    }
    memset((uint8_t *)s_stBram.pstA53Data, 0, BRAM_A53_DATA_SIZE);
    memset((uint8_t *)s_stBram.pstR5Data, 0, BRAM_R5_DATA_SIZE);

    /* 5, read data from config file, set to BRAM */
    config_file_init(config_file_path);
    config_file_a53();
    config_file_r5();
    config_file_exit();

    /* 6, set state */
    set_state_a53();

    /* 7, close app */
    MAP_BlockRamClose(&s_stBram);
    printf("config-parse exit!\n");

    return 0;
}

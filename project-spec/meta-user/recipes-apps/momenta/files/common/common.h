/*******************************************************************************
*
* Copyright 2020-2025 Momenta
* All rights reserved.
*
*******************************************************************************/

/*!
 * \file common.h
 * \brief a53 and r5 bram public header file.
 * \defgroup COMMON
 *
 * bram struct.
 *
 * @{
 */

#ifndef __COMMON_H__
#define __COMMON_H__


/*******************************************************************************
 Includes
*******************************************************************************/
#include <stdint.h>
#include <sys/types.h>

/*******************************************************************************
 Exported Macro, Enum, Union, Structure, And Type Definitions
*******************************************************************************/
// #ifdef __cplusplus
// extern "C"
// {
// #endif // __cplusplus

#define VRESION_A53                 "v1.18"
#define VERSION_DEBUG               7
#define VERSION_A53_REG_ADDR        (0x8000017c)
#define VERSION_R5_REG_ADDR         (0x80000178)
#define VERSION_PL_REG_ADDR         (0x80000174)

#define BRAM_BASE_ADDR              (0x80020000)
#define BRAM_MAX_SIZE               (0x10000)

#define BRAM_A53_STATE_BASE_ADDR    (BRAM_BASE_ADDR)                                    /*!< a53 state */
#define BRAM_A53_STATE_SIZE         (0x1000)
#define BRAM_A53_DATA_BASE_ADDR     (BRAM_A53_STATE_BASE_ADDR + BRAM_A53_STATE_SIZE)    /*!< a53 data, e.g. confige data */
#define BRAM_A53_DATA_SIZE          (0x1000)
#define BRAM_R5_STATE_BASE_ADDR     (BRAM_A53_DATA_BASE_ADDR + BRAM_A53_DATA_SIZE)      /*!< r5 state */
#define BRAM_R5_STATE_SIZE          (0x1000)
#define BRAM_R5_DATA_BASE_ADDR      (BRAM_R5_STATE_BASE_ADDR + BRAM_R5_STATE_SIZE)      /*!< r5 data, e.g. confige data */
#define BRAM_R5_DATA_SIZE           (0x1000)
#define BRAM_CAM_VER_BASE_ADDR      (BRAM_R5_DATA_BASE_ADDR + BRAM_R5_DATA_SIZE)        /*!< cam version, 5 uint32_t * 16 cam */
#define BRAM_CAM_VER_SIZE           (0x1000)

#define STATE_PL_ADDR               (0x80010000)
#define STATE_PL_SIZE               (0x300)

/* state udp packet data len 1280(0x500), A53+R5+PL */
#define STATE_A53_SIZE              (0x1F0)
#define STATE_R5_SIZE               (0x180)
#define STATE_PL_SIZE               (0x190)

typedef enum BramType
{
    BRAM_A53_STATE = 0,
    BRAM_A53_DATA,
    BRAM_R5_STATE,
    BRAM_R5_DATA,
    BRAM_TYPE_NUM
} BramType_e;

typedef enum DATA_SensorType
{
    MASTER_CAN3 = 0x00,
    MASTER_CAN1,
    MASTER_CAN2,
    SLAVE_UART1,
    SLAVE_UART2,
    SLAVE_UART3,
    SLAVE_UART4,
    SLAVE_CAN4,
    SLAVE_CAN5,
    MOMENTA_TOTAL,
    MOMENTA_POWER,
    MOMENTA_POWERKEY,
    MOMENTA_WIRE_BT,
    MOMENTA_WIRE_SS,
    MOMENTA_SENSORHUB_F7,
    MOMENTA_SENSORHUB_F4,

    SENSOR_DATA_TYPE_COUNT,
    SLAVE_UART5,
    SLAVE_UART6,
    SLAVE_UART7,
    SLAVE_UART8,
    SLAVE_UART9,
    SLAVE_UART10,
    SLAVE_UART11,
    SLAVE_UART12,
    SLAVE_UART13,
    SLAVE_UART14,
    SLAVE_UART15,
    SLAVE_UART16,
    SLAVE_CAN6,
    SLAVE_CAN7,
    SLAVE_CAN8,
    SLAVE_CAN9,
    SLAVE_CAN10,
    SLAVE_CAN11,
    SLAVE_CAN12,
    SLAVE_CAN13,
    SLAVE_CAN14,
    SLAVE_CAN15,
    SLAVE_CAN16,

    SENSOR_TYPE_NUM
} DATA_SensorType_E;

/* state udp packet, A53 0x1F0(496 Bytes), DATA_Perf_S 16 Bytes per sensor,
   14_sensors_up_and_down -> 448 Byptes, max!!! */
typedef enum State_SensorType
{
    STATE_CAN1 = 0x00,
    STATE_CAN2,
    STATE_CAN3,
    STATE_CAN4,
    STATE_CAN5,
    STATE_CAN6,
    STATE_CAN7,

    STATE_UART1,
    STATE_UART2,
    STATE_UART3,
    STATE_UART4,
    STATE_UART5,
    STATE_UART6,
    STATE_UART7,

    STATE_SENSOR_TYPE_NUM
} State_SensorType_E;

typedef struct DATA_Sensor
{
    uint16_t usUdpPort;
    uint16_t usReserved;
    uint32_t uiAmpTimeSec;
    uint32_t uiAmpTimeNsec;
    uint8_t ucHeadHigh;
    uint8_t ucHeadLow;
    uint8_t ucType;
    uint8_t ucCrc;
    uint32_t uiTimeSec;
    uint32_t uiTimeNsec;
    uint32_t uiDataLen;
    uint8_t *pstData;
} DATA_Sensor_S;

typedef struct DATA_Perf
{
    unsigned int uiCnt;             /*!< frame count */
    int iTimeDelayUs;               /*!< time delay between data timestamp to a53 recv */
    int iTimeDelayMaxUs;            /*!< max time delay between data timestamp to a53 recv */
    unsigned short usFreqInteger;   /*!< Integer of Frequence */
    unsigned short usFreqDecimal;   /*!< Decimal of Frequence */
} DATA_Perf_S;

typedef struct A53SelfCheck
{
    // uint32_t uiSelfCheckFlag;       /*!< self check flag, every bit indicate one thing, 0-ok, 1-err */
    uint32_t bErrFirmware        : 1;
    uint32_t bErrR5Power         : 1;
    uint32_t bErrTimeSync        : 1;
    uint32_t bErrPtp             : 1;
    uint32_t bErrTaskData        : 1;
    uint32_t bErrTaskState       : 1;
    uint32_t bErrUpDataLink      : 1;
    uint32_t bReserved           : 25;
    uint32_t uiCamIspCheckFlag;     /*!< self check flag for camera isp, every bit indicate one channel, 0-ok, 1-err */
} A53SelfCheck_S;

typedef struct A53State
{
    uint32_t uiHeader;              /*!< fix to 0xAABBCCDD */
    uint8_t ucMultibootVal;         /*!< multiboot val, BOOT.BIN start check */
    uint8_t ucBootScrVer;           /*!< boot script file version */
    uint8_t ucImageVal;             /*!< image file num val, image.ub start check */
    uint8_t ucReserved;
    uint32_t uiUbootCustomVer;      /*!< custom version of Uboot */
    uint32_t uiA53Version;          /*!< version of A53 */
    A53SelfCheck_S stSelfCheck;     /*!< selfcheck */
    uint32_t uiTimeSyncRealSec;     /*!< timesync read pl real time second */
    uint32_t uiTimeSyncRealNsec;    /*!< timesync read pl real time nand second */
    uint32_t uiTimeSyncDiffR2B;     /*!< timesync diff real -> begin */
    uint32_t uiTimeSyncDiffE2B;     /*!< timesync diff end -> begin */
    DATA_Perf_S astDataPerfUp[STATE_SENSOR_TYPE_NUM];   /*!< uplink data performace, 5can 5uart, refer to DATA TYPE */
    DATA_Perf_S astDataPerfDown[STATE_SENSOR_TYPE_NUM]; /*!< downlink data performace, 5can 5uart, refer to DATA TYPE */
} A53State_s;

typedef struct A53Data
{
    char acIpAddrLocal[32];         /*!< local ip addr */
    char acIpAddrDest[32];          /*!< dest ip addr */
    uint16_t usPortDataUp;          /*!< uplink data port */
    uint16_t usPortDataDown;        /*!< downlink data port */
    uint16_t usPortState;           /*!< uplink state port */
    uint16_t usTimeSyncPeriodMs;    /*!< time sync period, ms */
    uint16_t usSensorAnalysePerid;  /*!< performace analyse, sensor data count period */
    uint8_t ucStateUdpPeriodS;      /*!< period to send state udp */
    uint8_t ucDelayResetPeriod;     /*!< period to reset max delay value in task-data, unit: count */
} A53Data_s;

typedef struct R5State
{
    volatile uint32_t uiHeader;              /*!< fix to 0xEEFF1122 */
    volatile uint32_t uiR5Version;           /*!< version of R5 */
} R5State_s;

typedef struct R5Data
{
    volatile uint32_t uiCom1Baud;            /*!< BaudRate for COM1 */
} R5Data_s;

typedef struct BramPtr
{
    int fd;
    uint8_t *pucBase;
    A53State_s *pstA53State;
    A53Data_s *pstA53Data;
    R5State_s *pstR5State;
    R5Data_s *pstR5Data;
} BramPtr_s;

typedef struct PlStatePtr
{
    int fd;
    uint8_t *pucBase;
} PlStatePtr_s;

/*******************************************************************************
 Exported Variable Declarations
*******************************************************************************/


/*******************************************************************************
 Inline Function Definitions
*******************************************************************************/


/*******************************************************************************
 Global Scope Function Prototypes
*******************************************************************************/


// #ifdef __cplusplus
// }
// #endif // __cplusplus
#endif // __COMMON_H__

/*! @}*/

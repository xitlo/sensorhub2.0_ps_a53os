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

#define VRESION_A53                 "v1.11"
#define VERSION_DEBUG               0
#define VERSION_A53_REG_ADDR        (0x8000017c)
#define VERSION_R5_REG_ADDR         (0x80000178)

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

#define STATE_PL_ADDR               (0x80010000)
#define STATE_PL_SIZE               (0x300)

typedef enum BramType {
    BRAM_A53_STATE = 0,
    BRAM_A53_DATA,
    BRAM_R5_STATE,
    BRAM_R5_DATA,
    BRAM_TYPE_NUM
} BramType_e;

typedef struct A53State {
    uint32_t uiHeader;              /*!< fix to 0xAABBCCDD */
    uint8_t ucMultibootVal;         /*!< multiboot val, BOOT.BIN start check */
    uint8_t ucBootScrVer;           /*!< boot script file version */
    uint8_t ucImageVal;             /*!< image file num val, image.ub start check */
    uint8_t ucReserved;
    uint32_t uiA53Version;          /*!< version of A53 */
    uint32_t uiTimeSyncBeginSec;    /*!< timesync begin local time second */
    uint32_t uiTimeSyncBeginNsec;   /*!< timesync begin local time nand second */
    uint32_t uiTimeSyncRealSec;     /*!< timesync read pl real time second */
    uint32_t uiTimeSyncRealNsec;    /*!< timesync read pl real time nand second */
    uint32_t uiTimeSyncEndSec;      /*!< timesync end local time second */
    uint32_t uiTimeSyncEndNsec;     /*!< timesync end local time nand second */
    uint32_t uiTimeSyncDiffR2B;     /*!< timesync diff real -> begin */
    uint32_t uiTimeSyncDiffE2B;     /*!< timesync diff end -> begin */
} A53State_s;

typedef struct A53Data {
    char acIpAddrLocal[32];         /*!< local ip addr */
    char acIpAddrDest[32];          /*!< dest ip addr */
    uint16_t usPortDataUp;          /*!< uplink data port */
    uint16_t usPortDataDown;        /*!< downlink data port */
    uint16_t usPortState;           /*!< uplink state port */
    uint16_t usTimeSyncPeriodMs;    /*!< time sync period, ms */
} A53Data_s;

typedef struct R5State {
    uint32_t uiHeader;              /*!< fix to 0xEEFF1122 */
    uint32_t uiR5Version;           /*!< version of R5 */
} R5State_s;

typedef struct R5Data {
    uint32_t uiCom1Baud;            /*!< BaudRate for COM1 */
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

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

#define BRAM_BASE_ADDR              (0x80020000)
#define BRAM_MAX_SIZE               (0x10000)

#define BRAM_A53_STATE_BASE_ADDR    (BRAM_BASE_ADDR)
#define BRAM_A53_STATE_SIZE         (0x1000)
#define BRAM_A53_DATA_BASE_ADDR     (BRAM_A53_STATE_BASE_ADDR + BRAM_A53_STATE_SIZE)
#define BRAM_A53_DATA_SIZE          (0x1000)
#define BRAM_R5_STATE_BASE_ADDR     (BRAM_A53_DATA_BASE_ADDR + BRAM_A53_DATA_SIZE)
#define BRAM_R5_STATE_SIZE          (0x1000)
#define BRAM_R5_DATA_BASE_ADDR      (BRAM_R5_STATE_BASE_ADDR + BRAM_R5_STATE_SIZE)
#define BRAM_R5_DATA_SIZE           (0x1000)

typedef struct A53State {
    uint32_t uiHeader;              /*!< fix to 0xAABBCCDD */
    uint8_t ucMultibootVal;         /*!< multiboot val, BOOT.BIN start check */
    uint8_t ucBootScrVer;           /*!< boot script file version */
    uint8_t ucImageVal;             /*!< image file num val, image.ub start check */
    uint8_t ucReserved;
} A53State_s;

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

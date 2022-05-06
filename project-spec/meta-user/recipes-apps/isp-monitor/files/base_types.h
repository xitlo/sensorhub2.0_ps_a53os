/*******************************************************************************
*
* The content of this file or document is CONFIDENTIAL and PROPRIETARY
* to GEO Semiconductor. It is subject to the terms of a License Agreement
* between Licensee and GEO Semiconductor, restricting among other things,
* the use, reproduction, distribution and transfer. Each of the embodiments,
* including this information and any derivative work shall retain this
* copyright notice.
*
* Copyright 2013-2018 GEO Semiconductor, Inc.
* All rights reserved.
*
*******************************************************************************/

/*!
 * \file base_types.h
 * \brief Base data type definitions.
 *
 * Common base data types.
 *
 * @{
 */

#ifndef __BASE_TYPES_H__
#define __BASE_TYPES_H__

/*******************************************************************************
 Includes
*******************************************************************************/
#include <stdint.h>
#if (__STDC_VERSION__ >= 199901L) // has C99 support
#include <stdbool.h>
#endif // __STDC_VERSION__ >= 199901L


/*******************************************************************************
 Exported Macro, Enum, Union, Structure, And Type Definitions
*******************************************************************************/


/*******************************************************************************
 Inline Function Definitions
*******************************************************************************/


/*******************************************************************************
 Exported Variable Declarations
*******************************************************************************/
typedef uint8_t U8;
typedef int8_t S8;
typedef uint16_t U16;
typedef int16_t S16;
typedef uint32_t U32;
typedef int32_t S32;
typedef uint64_t U64;
typedef int64_t S64;

#if __bool_true_false_are_defined
typedef bool Bool;
#else // !(__bool_true_false_are_defined)
typedef enum {
    false = 0,
    true = 1
} Bool;
#endif // !(__bool_true_false_are_defined)

/*******************************************************************************
 Global Scope Function Prototypes
*******************************************************************************/


#endif // __BASE_TYPES_H__

/*! @}*/

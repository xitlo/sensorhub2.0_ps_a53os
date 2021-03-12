/*******************************************************************************

                        Copyright GEO Semiconductor 2015
                               All Rights Reserved.

 THIS WORK CONTAINS TRADE SECRET AND PROPRIETARY INFORMATION WHICH IS THE
 PROPERTY OF GEO SEMICONDUCTOR OR ITS LICENSORS AND IS SUBJECT TO LICENSE TERMS.
*******************************************************************************/

/*!
 * \file platform.h
 * \brief Platform abstraction declarations.
 * \defgroup OS_PLATFORM
 * \ingroup W5COM
 *
 * Platform abstraction declarations.
 *
 * @{
 */

#ifndef __PLATFORM_H__
#define __PLATFORM_H__

/*******************************************************************************
 Includes
*******************************************************************************/
#include <stdint.h>
#include <time.h>


/*******************************************************************************
 Exported Macro, Enum, Union, Structure, And Type Definitions
*******************************************************************************/
// #ifdef __cplusplus
// extern "C"
// {
// #endif // __cplusplus


/*******************************************************************************
 Exported Variable Declarations
*******************************************************************************/


/*******************************************************************************
 Inline Function Definitions
*******************************************************************************/
inline void PlatSleepMs(uint32_t milliseconds)
{
    struct timespec ts;
    ts.tv_sec = (time_t)(milliseconds / 1000);
    ts.tv_nsec = (milliseconds % 1000) * 1000000;
    nanosleep(&ts, NULL);
}

inline void PlatDelayUs(uint32_t microseconds)
{
    struct timespec ts;
    ts.tv_sec = (time_t)(microseconds / 1000000);
    ts.tv_nsec = (microseconds % 1000000) * 1000000;
    nanosleep(&ts, NULL);
}

/*******************************************************************************
 Global Scope Function Prototypes
*******************************************************************************/


// #ifdef __cplusplus
// }
// #endif // __cplusplus
#endif // __PLATFORM_H__

/*! @}*/

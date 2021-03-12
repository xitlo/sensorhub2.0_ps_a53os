/*******************************************************************************

                        Copyright GEO Semiconductor 2015
                               All Rights Reserved.

 THIS WORK CONTAINS TRADE SECRET AND PROPRIETARY INFORMATION WHICH IS THE
 PROPERTY OF GEO SEMICONDUCTOR OR ITS LICENSORS AND IS SUBJECT TO LICENSE TERMS.
*******************************************************************************/

/*!
 * \file linux/w5_com_platform.h
 * \brief W5 communication library platform specific declarations.
 * \ingroup W5COM
 *
 * W5 communication library platform specific declarations.
 *
 * @{
 */

#ifndef __W5_COM_PLATFORM_H__
#define __W5_COM_PLATFORM_H__


/*******************************************************************************
 Includes
*******************************************************************************/


/*******************************************************************************
 Exported Macro, Enum, Union, Structure, And Type Definitions
*******************************************************************************/
#if defined(W5_COM_API_EXPORTS)
#define W5_COM_API __attribute__((visibility("default")))
#else  // W5_COM_API_EXPORTS
#define W5_COM_API
#endif // W5_COM_API_EXPORTS

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


/*******************************************************************************
 Global Scope Function Prototypes
*******************************************************************************/


// #ifdef __cplusplus
// }
// #endif // __cplusplus
#endif // __W5_COM_PLATFORM_H__

/*! @}*/

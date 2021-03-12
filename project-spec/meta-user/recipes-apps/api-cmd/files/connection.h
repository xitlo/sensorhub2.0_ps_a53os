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
 * \file connection.h
 * \brief W5COM device connection private declarations.
 * \ingroup W5COM
 *
 * W5COM device connection private declarations.
 *
 * @{
 */

#ifndef __CONNECTION_H__
#define __CONNECTION_H__


/*******************************************************************************
 Includes
*******************************************************************************/
#include "w5_com.h"
#include "w5_connection.h"


/*******************************************************************************
 Exported Macro, Enum, Union, Structure, And Type Definitions
*******************************************************************************/
// #ifdef __cplusplus
// extern "C"
// {
// #endif // __cplusplus

struct W5Connection {
    W5ConnectionType_e type;
    struct SerialConnection *serialConnPtr;
    struct FtdiConnection *ftdiConnPtr;
    size_t maxBytesPerRead;
    size_t maxBytesPerWrite;
};

typedef W5ComReturn_e(*ConnOpen_f)(W5ComConfig_s *configPtr,
                                    W5connValidateConn_f validateConn,
                                    void *validateConnCustomDataPtr,
                                    struct W5Connection **connPtrPtr);

typedef W5ComReturn_e(*ConnConfigure_f)(struct W5Connection *connPtr,
                                         const W5ConnectionParams_s *paramsPtr);

typedef W5ComReturn_e(*ConnDetectProto_f)(struct W5Connection *connPtr,
                                           W5ProtocolType_e *protoTypePtr);

typedef W5ComReturn_e(*ConnRead_f)(const struct W5Connection *connPtr,
                                    uint8_t *readBufferPtr,
                                    size_t bytesToRead,
                                    size_t *bytesReadPtr);

typedef W5ComReturn_e(*ConnWrite_f)(const struct W5Connection *connPtr,
                                     const uint8_t *writeDataPtr,
                                     size_t bytesToWrite,
                                     size_t *bytesWrittenPtr,
                                     size_t bytesToRead);

typedef W5ComReturn_e(*ConnClose_f)(struct W5Connection **connPtrPtr);

#define UNUSED(x) (void)(x)


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
#endif // __CONNECTION_H__

/*! @}*/

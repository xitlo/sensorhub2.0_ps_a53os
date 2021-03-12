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
 * \file w5_com.c
 * \brief W5 communication library implementation.
 * \addtogroup W5COM
 *
 * W5 communication library implementation.
 */


/*******************************************************************************
 Includes
*******************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include "w5_com.h"
#include "connection.h"
#include "protocol.h"
#if !defined(_MSC_VER)
#include <unistd.h>
#endif


/*******************************************************************************
 Exported Variable Definitions
*******************************************************************************/


/*******************************************************************************
 Extern Inline Function Declarations
*******************************************************************************/


/*******************************************************************************
 Internal Macro, Enum, Union, Structure, And Type Definitions
*******************************************************************************/
#define W5_MSG_GEO_OVERHEAD (3U) //API Code/Msg ID (1), Chunk number(2)
#define W5_MSG_RESULT_OVERHEAD (7U) //Cmd Status(1), Chunk number(2), PayloadDataValidLen(4)

#define LAST_CHUNK_MASK (0x8000)

struct W5ComHandle {
    W5Connection_s *connectionPtr;
    W5Protocol_s *protocolPtr;
    int32_t numCommands;
    W5Command_s *cmdQueueHeadPtr;
    W5Command_s *cmdQueueTailPtr;
};


/*******************************************************************************
 File Scope Variable Declarations And Definitions
*******************************************************************************/
static const ConnOpen_f ConnectionOpen[w5_num_connection_types] = {
    NULL, // w5_connection_type_unspecified
    W5connSerialOpen, // w5_connection_type_serial
    W5connFtdi4222Open // w5_connection_type_ftdi4222
};

static const ConnDetectProto_f DetectProtocol[w5_num_connection_types] = {
    NULL, // w5_connection_type_unspecified
    W5connSerialDetectProtocol, // w5_connection_type_serial
    W5connFtdi4222DetectProtocol // w5_connection_type_ftdi4222
};

static const ConnRead_f ConnectionRead[w5_num_connection_types] = {
    NULL, // w5_connection_type_unspecified
    W5connSerialRead, // w5_connection_type_serial
    W5connFtdi4222Read // w5_connection_type_ftdi4222
};

static const ConnWrite_f ConnectionWrite[w5_num_connection_types] = {
    NULL, // w5_connection_type_unspecified
    W5connSerialWrite, // w5_connection_type_serial
    W5connFtdi4222Write // w5_connection_type_ftdi4222
};

static const ConnClose_f ConnectionClose[w5_num_connection_types] = {
    NULL, // w5_connection_type_unspecified
    W5connSerialClose, // w5_connection_type_serial
    W5connFtdi4222Close // w5_connection_type_ftdi4222
};

static const ProtoAttach_f ProtocolAttach[w5_num_protocol_types] = {
    NULL, // w5_protocol_type_unspecified
    W5protoUartAttach, // w5_protocol_type_uart
    W5protoI2cAttach // w5_protocol_type_i2c
};

static const ProtoReconfigMaxBytes_f ProtocolReconfigMaxBytes[w5_num_protocol_types] = {
    NULL, // w5_protocol_type_unspecified
    W5protoUartReconfigMaxBytes, //  w5_protocol_type_uart
    W5protoI2cReconfigMaxBytes // w5_protocol_type_i2c
};

static const ProtoW5CommandsHandler_f ProcessW5Commands[w5_num_protocol_types] = {
    NULL, // w5_protocol_type_unspecified
    W5protoUartProcessW5Commands, // w5_protocol_type_uart
    W5protoI2cProcessW5Commands // w5_protocol_type_i2c
};

static const ProtoDetach_f ProtocolDetach[w5_num_protocol_types] = {
    NULL, // w5_protocol_type_unspecified
    W5protoUartDetach, // w5_protocol_type_uart
    W5protoI2cDetach // w5_protocol_type_i2c
};


/*******************************************************************************
 File Scope Function Prototypes
*******************************************************************************/
static W5ComReturn_e configureConnection(const W5ComConfig_s *configPtr,
                                         struct W5Connection *connPtr,
                                         void *customDataPtr);

static W5ComReturn_e getMaxPayloadSizeFromW5(const W5ComHandle_s *handlePtr, U32 *sizePtr);

/*******************************************************************************
 File Scope Function Definitions
*******************************************************************************/
static W5ComReturn_e configureConnection(const W5ComConfig_s *configPtr,
                                         struct W5Connection *connPtr,
                                         void *customDataPtr)
{
    ProtoAttach_f protoAttach = ProtocolAttach[configPtr->protoType];
    W5Protocol_s *protoPtr = NULL;
    W5ComReturn_e ret;
    W5ComHandle_s *w5ComHandlePtr = (W5ComHandle_s *)customDataPtr;

    // determine and attach protocol
    if (protoAttach != NULL) {
        ret = protoAttach(configPtr, connPtr, &protoPtr);
    }
    else if (configPtr->protoType != w5_protocol_type_unspecified) {
        // config specified a protocol type that is not supported
        ret = w5_com_ret_not_supported;
    }
    else if (DetectProtocol[connPtr->type] != NULL) {
        W5ProtocolType_e protoType = w5_protocol_type_unspecified;
        // detect the protocol type from the connection
        ret = DetectProtocol[connPtr->type](connPtr, &protoType);
        if (ret == w5_com_ret_ok) {
            protoAttach = ProtocolAttach[protoType];
            if (protoAttach != NULL) {
                ret = protoAttach(configPtr, connPtr, &protoPtr);
            }
        }
    }
    else {
        // connection doens't support protocol detection
        ret = w5_com_ret_general_error;
    }
    if (ret != w5_com_ret_ok) {
        return ret;
    }

    w5ComHandlePtr->protocolPtr = protoPtr;

    return w5_com_ret_ok;
}

static W5ComReturn_e getMaxPayloadSizeFromW5(const W5ComHandle_s *handlePtr, U32 *sizePtr)
{
    W5ComReturn_e ret;
    uint8_t msgID;
    const size_t sizeNeeded = offsetof(typeof(Info_s), maxRspPayloadSizePerChunk) + sizeof(((Info_s *)0)->maxRspPayloadSizePerChunk);
    U8 buf[sizeNeeded];

    ret = W5comCallApi(handlePtr, api_get_info, 0, NULL, LAST_CHUNK_MASK, &msgID);

    if (ret == w5_com_ret_ok) {

        int queryRetry = 10;
        size_t sizeReceived = 0;

        while (queryRetry > 0 && sizeReceived < sizeNeeded) {
            uint8_t cmdStatus;
            size_t dataAvail;
            uint16_t chunkNumber;

            ret = W5comCallQuery(handlePtr, msgID, sizeNeeded - sizeReceived, &buf[sizeReceived], &cmdStatus, &dataAvail, &chunkNumber);
            if (ret != w5_com_ret_ok) {
                queryRetry = 0;
            }
            else { // !if (ret != w5_com_ret_ok)
                switch(cmdStatus) {
                case CMD_STATUS_PENDING:
                    dataAvail = 0;
                case CMD_STATUS_DONE:
                    if (dataAvail == 0) {
#if defined(_MSC_VER)
                        Sleep(50);         // sleep for 50 milliseconds
#else
                        usleep(50 * 1000); // sleep for 50 milliseconds
#endif
                        ret = w5_com_ret_device_busy;
                        queryRetry--;
                    }
                    else {
                        sizeReceived += dataAvail;
                        if (sizeReceived >= sizeNeeded) {
                            *sizePtr = ((Info_s *)buf)->maxRspPayloadSizePerChunk;
                            ret = w5_com_ret_ok;
                            queryRetry = 0;
                        }
                    }
                    break;
                case CMD_STATUS_FAIL:
                case CMD_STATUS_UNAVAILABLE:
                default:
                    ret = w5_com_ret_general_error;
                    queryRetry = 0;
                    break;
                }
            } // !if (ret != w5_com_ret_ok)
        } // while (queryRetry > 0 && sizeReceived < sizeof(rsp))
    } // if (ret == w5_com_ret_ok)

    return ret;
}


/*******************************************************************************
 Global Scope Function Definitions
*******************************************************************************/
W5ComReturn_e W5comConnect(W5ComConfig_s *configPtr,
                           W5ComHandle_s **handlePtrPtr)
{
    W5ComReturn_e ret;
    ConnOpen_f connOpen;
    W5ComHandle_s *w5ComHandlePtr;
    uint32_t newReadSize;

    if ((configPtr == NULL)
        || (configPtr->devSpecType >= w5_num_device_specifier_types)
        || (configPtr->connType >= w5_num_connection_types)
        || (configPtr->protoType >= w5_num_protocol_types)
        || (handlePtrPtr == NULL)
        || (*handlePtrPtr != NULL)) {
        return w5_com_ret_invalid_param;
    }

    w5ComHandlePtr = (W5ComHandle_s *)calloc(1, sizeof(W5ComHandle_s));
    if (w5ComHandlePtr == NULL) {
        return w5_com_ret_out_of_memory;
    }

    connOpen = ConnectionOpen[configPtr->connType];
    if (connOpen != NULL) {
        ret = connOpen(configPtr, configureConnection, w5ComHandlePtr,
                       &w5ComHandlePtr->connectionPtr);
    }
    else {
        // config specified a connection type that is not supported
        ret = w5_com_ret_not_supported;
    }
    if (ret != w5_com_ret_ok) {
        free(w5ComHandlePtr);
        return ret;
    }

    if (getMaxPayloadSizeFromW5(w5ComHandlePtr, &newReadSize) == w5_com_ret_ok) {
        // Reconfigure w5ComHandlePtr->protocolPtr->maxBytesPerRead if a new response payload size value, newReadSize, is available
        // newReadSize should be larger than the default w5ComHandlePtr->protocolPtr->maxBytesPerRea

        ProtoReconfigMaxBytes_f protoReconfigMaxBytes = ProtocolReconfigMaxBytes[configPtr->protoType];

        if (protoReconfigMaxBytes != NULL) {
            ret = protoReconfigMaxBytes(configPtr,
                                        w5ComHandlePtr->connectionPtr,
                                        w5ComHandlePtr->protocolPtr,
                                        w5ComHandlePtr->protocolPtr->maxBytesPerWrite,
                                        newReadSize);
        }
    }
    // if the call fails, w5ComHandlePtr->protocolPtr->maxBytesPerRead remains with the default size

    *handlePtrPtr = w5ComHandlePtr;

    return w5_com_ret_ok;
}

void W5comDisconnect(W5ComHandle_s **handlePtrPtr)
{
    W5ComReturn_e ret;
    W5Protocol_s *protoPtr;
    W5Connection_s *connPtr;

    if ((NULL == handlePtrPtr)
        || (NULL == *handlePtrPtr)) {
        return;
    }

    protoPtr = (*handlePtrPtr)->protocolPtr;
    ret = ProtocolDetach[protoPtr->type](&protoPtr);
    if (ret != w5_com_ret_ok) {
        fprintf(stderr, "Failed to detach protocol: %d", ret);
    }

    connPtr = (*handlePtrPtr)->connectionPtr;
    ret = ConnectionClose[connPtr->type](&connPtr);
    if (ret != w5_com_ret_ok) {
        fprintf(stderr, "Failed to close connection: %d", ret);
    }

    while ((*handlePtrPtr)->cmdQueueHeadPtr != NULL) {
        W5Command_s *cmdPtr = (*handlePtrPtr)->cmdQueueHeadPtr;
        (*handlePtrPtr)->cmdQueueHeadPtr = cmdPtr->nextPtr;
        free(cmdPtr);
    }

    free(*handlePtrPtr);
    *handlePtrPtr = NULL;
}

W5ComReturn_e W5comGetConnectionType(const W5ComHandle_s *handlePtr,
                                     W5ConnectionType_e *connTypePtr)
{
    if ((handlePtr == NULL)
        || (connTypePtr == NULL)) {
        return w5_com_ret_invalid_param;
    }

    if (handlePtr->connectionPtr == NULL) {
        return w5_com_ret_not_connected;
    }

    *connTypePtr = handlePtr->connectionPtr->type;

    return w5_com_ret_ok;
}

W5ComReturn_e W5comGetProtocolType(const W5ComHandle_s *handlePtr,
                                   W5ProtocolType_e *protoTypePtr)
{
    if ((handlePtr == NULL)
        || (protoTypePtr == NULL)) {
        return w5_com_ret_invalid_param;
    }

    if (handlePtr->protocolPtr == NULL) {
        return w5_com_ret_not_connected;
    }

    *protoTypePtr = handlePtr->protocolPtr->type;

    return w5_com_ret_ok;
}

W5ComReturn_e W5comGetProtocolVersion(const W5ComHandle_s *handlePtr,
                                      W5ProtocolVersion_e *protoVersionPtr)
{
    if ((handlePtr == NULL)
        || (protoVersionPtr == NULL)) {
        return w5_com_ret_invalid_param;
    }

    if (handlePtr->protocolPtr == NULL) {
        return w5_com_ret_not_connected;
    }

    *protoVersionPtr = handlePtr->protocolPtr->version;

    return w5_com_ret_ok;
}

W5ComReturn_e W5comGetSerialPortNumber(const W5ComHandle_s *handlePtr,
                                       uint32_t *portNumberPtr)
{
    if ((handlePtr == NULL)
        || (portNumberPtr == NULL)) {
        return w5_com_ret_invalid_param;
    }

    if (handlePtr->protocolPtr == NULL) {
        return w5_com_ret_not_connected;
    }

    if ((handlePtr->protocolPtr->type != w5_protocol_type_uart)
        || (handlePtr->connectionPtr->type != w5_connection_type_serial)) {
        return w5_com_ret_not_supported;
    }

    return W5connSerialGetPort(handlePtr->connectionPtr, portNumberPtr);
}

W5ComReturn_e W5comGetUartDuplex(const W5ComHandle_s *handlePtr,
                                 W5UartDuplex_e *uartDuplexPtr)
{
    if ((handlePtr == NULL)
        || (uartDuplexPtr == NULL)) {
        return w5_com_ret_invalid_param;
    }

    if (handlePtr->protocolPtr == NULL) {
        return w5_com_ret_not_connected;
    }
    else if (handlePtr->protocolPtr->type != w5_protocol_type_uart) {
        return w5_com_ret_not_supported;
    }

    *uartDuplexPtr = handlePtr->protocolPtr->uartInfo.duplex;

    return w5_com_ret_ok;
}

W5ComReturn_e W5comGetMaxBytesPerWrite(const W5ComHandle_s *handlePtr,
                                       size_t *maxBytesPerWritePtr)
{
    size_t max;

    if ((handlePtr == NULL)
        || (maxBytesPerWritePtr == NULL)) {
        return w5_com_ret_invalid_param;
    }

    max = handlePtr->protocolPtr->maxBytesPerWrite - W5_MSG_GEO_OVERHEAD;
    *maxBytesPerWritePtr = (max < MAX_API_WRITE_SIZE) ? max : MAX_API_WRITE_SIZE;

    return w5_com_ret_ok;
}

W5ComReturn_e W5comGetMaxBytesPerRead(const W5ComHandle_s *handlePtr,
                                      size_t *maxBytesPerReadPtr)
{
    if ((handlePtr == NULL)
        || (maxBytesPerReadPtr == NULL)) {
        return w5_com_ret_invalid_param;
    }

    *maxBytesPerReadPtr = handlePtr->protocolPtr->maxBytesPerRead - W5_MSG_RESULT_OVERHEAD;

    return w5_com_ret_ok;
}

W5ComReturn_e W5comReadBytes(const W5ComHandle_s *handlePtr,
                             uint8_t *readBufferPtr, size_t bytesToRead,
                             size_t *bytesReadPtr)
{
    ConnRead_f connRead;

    if ((handlePtr == NULL)
        || (readBufferPtr == NULL)
        || (bytesToRead > handlePtr->connectionPtr->maxBytesPerRead)
        || (bytesReadPtr == NULL)) {
        return w5_com_ret_invalid_param;
    }

    connRead = ConnectionRead[handlePtr->connectionPtr->type];
    return connRead(handlePtr->connectionPtr, readBufferPtr,
                    bytesToRead, bytesReadPtr);
}

W5ComReturn_e W5comWriteBytes(const W5ComHandle_s *handlePtr,
                              const uint8_t *writeBytesPtr,
                              size_t bytesToWrite,
                              size_t *bytesWrittenPtr,
                              size_t bytesToRead)
{
    ConnWrite_f connWrite;

    if ((handlePtr == NULL)
     || (writeBytesPtr == NULL)
     || (bytesToWrite > handlePtr->connectionPtr->maxBytesPerWrite)
     || (bytesWrittenPtr == NULL)) {
        return w5_com_ret_invalid_param;
    }

    connWrite = ConnectionWrite[handlePtr->connectionPtr->type];
    return connWrite(handlePtr->connectionPtr, writeBytesPtr,
                     bytesToWrite, bytesWrittenPtr, bytesToRead);
}

W5ComReturn_e W5comCallApi(const W5ComHandle_s *handlePtr, APICode_e apiCode,
                           size_t bytesToWrite, const uint8_t *writeDataPtr,
                           uint16_t chunkNum, uint8_t *msgIdPtr)
{
    W5Command_s cmd;
    W5ComReturn_e ret;
    uint8_t *tempBuf;
    //GEO API consists of api code (1) + chunk number (2) + bytes to write
    size_t tempBufSize = bytesToWrite + W5_MSG_GEO_OVERHEAD;

    if ((handlePtr == NULL)
     || (tempBufSize > handlePtr->protocolPtr->maxBytesPerWrite)
     || ((bytesToWrite > 0) && (writeDataPtr == NULL))
     || (msgIdPtr == NULL)) {
        return w5_com_ret_invalid_param;
    }

    tempBuf = (uint8_t *)calloc(1, tempBufSize);
    if (tempBuf == NULL) {
        return w5_com_ret_out_of_memory;
    }

    tempBuf[geo_api_pkt_cmd_off_api_code] = apiCode;
    tempBuf[geo_api_pkt_cmd_off_chunk_num_lsb] = (uint8_t)(chunkNum & 0xff);
    tempBuf[geo_api_pkt_cmd_off_chunk_num_msb] = (uint8_t)((chunkNum >> 8) & 0xff);
    memcpy(&tempBuf[geo_api_pkt_cmd_off_api_payload], writeDataPtr, bytesToWrite);

    cmd.msgType = PROTOCOL_MSG_TYPE_GEO;
    cmd.rspMsgType = PROTOCOL_MSG_TYPE_ACK;
    cmd.apiCode = apiCode;
    cmd.writeSizeInBytes = tempBufSize;
    cmd.writeDataPtr = tempBuf;
    cmd.readSizeInBytes = 0;
    cmd.readBufferPtr = NULL;
    cmd.resultCb = NULL;
    cmd.userDataPtr = NULL;
    cmd.nextPtr = NULL;

    ret = ProcessW5Commands[handlePtr->protocolPtr->type](handlePtr->protocolPtr,
                                                          &cmd);

    *msgIdPtr = cmd.rspMsgId;

    return ret;
}

W5ComReturn_e W5comCallQuery(const W5ComHandle_s *handlePtr, uint8_t msgId,
                             size_t bytesToRead, uint8_t *readDataBufPtr,
                             uint8_t *cmdStatusPtr, size_t *dataAvailPtr,
                             uint16_t *chunkNumberPtr)
{
    W5Command_s cmd;
    W5ComReturn_e ret;

    //Query consists of 7 fixed bytes to write:
    //msg id (1), chunk number (2) and requested data size (4)
    uint8_t writeBuf[7];

    uint8_t *tempBuf;

    size_t tempBufSize = bytesToRead + W5_MSG_RESULT_OVERHEAD;

    if ((handlePtr == NULL)
     || ((bytesToRead + W5_MSG_RESULT_OVERHEAD) > handlePtr->protocolPtr->maxBytesPerRead)
     || (cmdStatusPtr == NULL)
     || ((bytesToRead > 0) && (dataAvailPtr == NULL))
     || ((bytesToRead > 0) && (chunkNumberPtr == NULL))) {
        return w5_com_ret_invalid_param;
    }

    tempBuf = (uint8_t *)calloc(1, tempBufSize);
    if (tempBuf == NULL) {
        return w5_com_ret_out_of_memory;
    }

    writeBuf[query_pkt_msg_off_msg_id] = msgId;
    if (chunkNumberPtr == NULL) {
        writeBuf[query_pkt_msg_off_chunk_num_lsb] = 0x00;
        writeBuf[query_pkt_msg_off_chunk_num_msb] = 0x00;
    }
    else {
        writeBuf[query_pkt_msg_off_chunk_num_lsb] = (uint8_t)(*chunkNumberPtr);
        writeBuf[query_pkt_msg_off_chunk_num_msb] = (uint8_t)((*chunkNumberPtr) >> 8);
    }
    writeBuf[query_pkt_msg_off_bytes_to_read_byte_0_lsb] = (uint8_t)(bytesToRead & 0xFF);
    writeBuf[query_pkt_msg_off_bytes_to_read_byte_1] = (uint8_t)((bytesToRead >> 8) & 0xFF);
    writeBuf[query_pkt_msg_off_bytes_to_read_byte_2] = (uint8_t)((bytesToRead >> 16) & 0xFF);
    writeBuf[query_pkt_msg_off_bytes_to_read_byte_3_msb] = (uint8_t)((bytesToRead >> 24) & 0xFF);

    cmd.msgType = PROTOCOL_MSG_TYPE_QUERY;
    cmd.rspMsgType = PROTOCOL_MSG_TYPE_RESULT;
    cmd.msgId = msgId;
    cmd.writeSizeInBytes = 7;
    cmd.writeDataPtr = writeBuf;
    cmd.readSizeInBytes = tempBufSize;
    cmd.readBufferPtr = tempBuf;
    cmd.resultCb = NULL;
    cmd.userDataPtr = NULL;
    cmd.nextPtr = NULL;

    ret = ProcessW5Commands[handlePtr->protocolPtr->type](handlePtr->protocolPtr,
                                                          &cmd);

    if (ret == w5_com_ret_ok) {
        if (cmd.msgId != cmd.rspMsgId) {
            ret = w5_com_ret_msg_id_mismatched;
        }
        else {
            //First W5_MSG_RESULT_OVERHEAD bytes are command status (1), chunk number (2), data valid length (4)
            *cmdStatusPtr = tempBuf[rsp_pkt_result_off_cmd_status];
            if (chunkNumberPtr != NULL) {
                *chunkNumberPtr = (tempBuf[rsp_pkt_result_off_chunk_num_msb] << 8) | tempBuf[rsp_pkt_result_off_chunk_num_lsb];
            }

            if (bytesToRead) {
                *dataAvailPtr = (tempBuf[rsp_pkt_result_off_valid_len_byte_3_msb] << 24) | (tempBuf[rsp_pkt_result_off_valid_len_byte_2] << 16)
                                | (tempBuf[rsp_pkt_result_off_valid_len_byte_1] << 8) | tempBuf[rsp_pkt_result_off_valid_len_byte_0_lsb];
                memcpy(readDataBufPtr, &tempBuf[rsp_pkt_result_off_payload], *dataAvailPtr);
            }
        }
    }

    free(tempBuf);
    return ret;
}

W5ComReturn_e W5comLibSetVerbosity(W5ComHandle_s *handlePtr, W5ComVerbosity_e verbosityLevel)
{
    W5ComReturn_e ret = w5_com_ret_ok;

    if ((handlePtr == NULL) || (handlePtr->protocolPtr == NULL)) {
        return w5_com_ret_invalid_param;
    }

    switch (verbosityLevel) {
    case verbosity_normal:
    case verbosity_verbose:
        handlePtr->protocolPtr->verbosityLevel = verbosityLevel;
        break;
    default:
        ret = w5_com_ret_invalid_param;
        break;
    }

    return ret;
}

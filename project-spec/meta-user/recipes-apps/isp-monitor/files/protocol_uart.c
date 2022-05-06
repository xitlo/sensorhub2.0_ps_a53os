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
 * \file protocol_uart.c
 * \brief W5 UART communication protocol handling implementation.
 * \addtogroup W5PROTO
 *
 * W5 UART communication protocol handling implementation.
 */


/*******************************************************************************
 Includes
*******************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#if (__STDC_VERSION__ >= 199901L) // has C99 or above support
#include <stdbool.h>
#endif // __STDC_VERSION__ >= 199901L
#include <string.h>
#include "w5_connection.h"
#include "protocol.h"
#include "connection.h"
#include "platform.h"
#if !(__bool_true_false_are_defined)
typedef enum {
    false = 0,
    true = 1
} bool;
#endif // !(__bool_true_false_are_defined)


/*******************************************************************************
 Exported Variable Definitions
*******************************************************************************/


/*******************************************************************************
 Extern Inline Function Declarations
*******************************************************************************/


/*******************************************************************************
 Internal Macro, Enum, Union, Structure, And Type Definitions
*******************************************************************************/
#define W5_UART_START_FRAME PROTOCOL_START_FRAME
#define W5_UART_END_FRAME PROTOCOL_END_FRAME

// UART API command overhead consists of
// start frame(1), end frame(1) + protocol overhead
#define W5_UART_V3_API_CMD_OVERHEAD (2 + W5_V3_API_CMD_OVERHEAD)
#define W5_UART_V3_QUERY_CMD_OVERHEAD (2 + W5_V3_QUERY_CMD_OVERHEAD)
#define MAX_W5_UART_V3_CMD_OVERHEAD (2 + MAX_W5_V3_CMD_OVERHEAD)
// UART API response overhead consists of
// start frame(1), end frame(1) + protocol overhead
#define W5_UART_V3_API_RSP_OVERHEAD (2 + W5_V3_API_RSP_OVERHEAD)
#define W5_UART_V3_QUERY_RSP_OVERHEAD (2 + W5_V3_QUERY_RSP_OVERHEAD)
#define MAX_W5_UART_V3_RSP_OVERHEAD (2 + MAX_W5_V3_RSP_OVERHEAD)

#define UART_MAX_WRITE_SIZE (MAX_PROTOCOL_MSG_API_CMD_PAYLOAD_SIZE)
#define UART_MAX_READ_SIZE (MAX_PROTOCOL_MSG_QUERY_RSP_PAYLOAD_SIZE)

#define TEST_DATA (0xDA7AB10BU)

/*******************************************************************************
 File Scope Variable Declarations And Definitions
*******************************************************************************/
static const ConnConfigure_f ConnectionConfigure[w5_num_connection_types] = {
    NULL, // w5_connection_type_unspecified
    W5connSerialConfigure, // w5_connection_type_serial
    NULL // w5_connection_type_ftdi4222
};

static const ConnRead_f ConnectionRead[w5_num_connection_types] = {
    NULL, // w5_connection_type_unspecified
    W5connSerialRead, // w5_connection_type_serial
    NULL // w5_connection_type_ftdi4222
};

static const ConnWrite_f ConnectionWrite[w5_num_connection_types] = {
    NULL, // w5_connection_type_unspecified
    W5connSerialWrite, // w5_connection_type_serial
    NULL // w5_connection_type_ftdi4222
};


/*******************************************************************************
 File Scope Function Prototypes
*******************************************************************************/
static inline W5ComReturn_e sendW5UartCommand(const W5Protocol_s *protoPtr,
                                              uint8_t apiCode,
                                              size_t writePayloadSizeInBytes,
                                              const uint8_t *writePayloadPtr,
                                              size_t readPayloadSizeInBytes);

static inline W5ComReturn_e receiveW5UartResponse(const W5Protocol_s *protoPtr,
                                                  uint8_t apiCode,
                                                  size_t readPayloadSizeInBytes,
                                                  uint8_t *readPayloadBuf,
                                                  uint8_t *rspMsgId);

static W5ComReturn_e detectDuplex(const W5Connection_s *connPtr,
                                  W5UartDuplex_e *duplexPtr);

static inline void getW5protoUartOverheads(const W5ComConfig_s *configPtr,
                                           size_t *maxCmdOverhead,
                                           size_t *maxRspOverhead);

static inline size_t getW5protoUartMaxBytesPerWrite(const W5Connection_s *currentConnPtr, 
                                                    size_t maxCmdOverhead,
                                                    size_t maxWriteSizeRequested);

static inline size_t getW5protoUartMaxBytesPerRead(const W5Connection_s *currentConnPtr, 
                                                   size_t maxRspOverhead,
                                                   size_t maxReadSizeRequested);


/*******************************************************************************
 File Scope Function Definitions
*******************************************************************************/
static inline W5ComReturn_e sendW5UartCommand(const W5Protocol_s *protoPtr,
                                              uint8_t msgType,
                                              size_t writePayloadSizeInBytes,
                                              const uint8_t *writePayloadPtr,
                                              size_t readPayloadSizeInBytes)
{
    uint8_t *cmdBufPtr = NULL;
    size_t cmdSize = writePayloadSizeInBytes;
    size_t bytesWritten = 0;
    uint8_t checksum;
    W5ComReturn_e ret;

    switch (protoPtr->version) {
    case w5_protocol_v3:
    default:
        if (msgType == PROTOCOL_MSG_TYPE_QUERY) {
            cmdSize += W5_UART_V3_QUERY_CMD_OVERHEAD;
        }
        else {
            cmdSize += W5_UART_V3_API_CMD_OVERHEAD;
        }
        break;
    }
    cmdBufPtr = (uint8_t *)calloc(1, cmdSize);
    if (cmdBufPtr == NULL) {
        return w5_com_ret_out_of_memory;
    }

    cmdBufPtr[geo_api_pkt_uart_off_frame_start] = W5_UART_START_FRAME;
    cmdBufPtr[geo_api_pkt_uart_off_proto_id] = W5_V3_CODE;
    cmdBufPtr[geo_api_pkt_uart_off_msg_type] = msgType;
    if (msgType == PROTOCOL_MSG_TYPE_QUERY) {
        cmdBufPtr[query_pkt_uart_off_data_size] = (uint8_t)(writePayloadSizeInBytes & 0xff);

        if (writePayloadSizeInBytes > 0) {
            memcpy(&cmdBufPtr[query_pkt_uart_off_data_size + 1], writePayloadPtr, writePayloadSizeInBytes);
        }

        // cmdSize minus 1: checksum does not apply on frame start, frame end, and checksum field itself
        checksum = calcCheckSum(&cmdBufPtr[query_pkt_uart_off_proto_id], cmdSize - 3);
        cmdBufPtr[query_pkt_uart_off_checksum] = checksum;
        cmdBufPtr[query_pkt_uart_off_frame_end] = W5_UART_END_FRAME;
    }
    else {
        cmdBufPtr[geo_api_pkt_uart_off_data_size_byte_0_lsb] = (uint8_t)(writePayloadSizeInBytes & 0xff);
        cmdBufPtr[geo_api_pkt_uart_off_data_size_byte_1] = (uint8_t)((writePayloadSizeInBytes >> 8) & 0xff);
        cmdBufPtr[geo_api_pkt_uart_off_data_size_byte_2] = (uint8_t)((writePayloadSizeInBytes >> 16) & 0xff);
        cmdBufPtr[geo_api_pkt_uart_off_data_size_byte_3_msb] = (uint8_t)((writePayloadSizeInBytes >> 24) & 0xff);

        if (writePayloadSizeInBytes > 0) {
            memcpy(&cmdBufPtr[geo_api_pkt_uart_off_data_size_byte_3_msb + 1], writePayloadPtr, writePayloadSizeInBytes);
        }

        // cmdSize minus 1: checksum does not apply on frame start, frame end, and checksum field itself
        checksum = calcCheckSum(&cmdBufPtr[geo_api_pkt_uart_off_proto_id], cmdSize - 3);
        cmdBufPtr[geo_api_pkt_uart_off_data_size_byte_3_msb + 1 + writePayloadSizeInBytes + geo_api_pkt_uart_after_payload_off_checksum] = checksum;
        cmdBufPtr[geo_api_pkt_uart_off_data_size_byte_3_msb + 1 + writePayloadSizeInBytes + geo_api_pkt_uart_after_payload_off_frame_end] = W5_UART_END_FRAME;
    }

    if (protoPtr->verbosityLevel == verbosity_verbose) {
        printf("TX: ");
        size_t i;
        for (i = 0; i < cmdSize; i++) {
            printf("%.2x ", cmdBufPtr[i]);
        }
        printf("\n");
    }

    ret = ConnectionWrite[protoPtr->connPtr->type](protoPtr->connPtr,
                                                   cmdBufPtr, cmdSize,
                                                   &bytesWritten,
                                                   readPayloadSizeInBytes);
    if ((ret != w5_com_ret_ok) || (bytesWritten != cmdSize)) {
        // write failed
        ret = w5_com_ret_general_error;
    }
    else if (protoPtr->uartInfo.duplex == w5_uart_duplex_half) {
        // UART is half duplex, read back the sent command
        size_t readSize = cmdSize;
        uint8_t *bufPtr = cmdBufPtr;
        while (readSize > 0) {
            size_t bytesRead = 0;
            ret = ConnectionRead[protoPtr->connPtr->type](protoPtr->connPtr,
                                                          bufPtr, readSize,
                                                          &bytesRead);
            if (ret != w5_com_ret_ok) {
                ret = w5_com_ret_general_error;
                ret = w5_com_ret_not_supported;
            }

            bufPtr += bytesRead;
            readSize -= bytesRead;
        }
    }

    free(cmdBufPtr);

    return ret;
}

static inline W5ComReturn_e receiveW5UartResponse(const W5Protocol_s *protoPtr,
                                                  uint8_t msgType,
                                                  size_t readPayloadSizeInBytes,
                                                  uint8_t *readPayloadBufPtr,
                                                  uint8_t *rspMsgId)
{
    uint8_t *rspBufPtr;
    size_t rspSize = readPayloadSizeInBytes;
    size_t bytesRead = 0;
    int retries = 0;
    W5ComReturn_e ret;

    switch (protoPtr->version) {
    case w5_protocol_v3:
    default:
        if (msgType == PROTOCOL_MSG_TYPE_RESULT) {
            rspSize += W5_UART_V3_QUERY_RSP_OVERHEAD;
        }
        else {
            rspSize += W5_UART_V3_API_RSP_OVERHEAD;
        }
        break;
    }
    rspBufPtr = (uint8_t *)calloc(1, rspSize);
    if (rspBufPtr == NULL) {
        return w5_com_ret_out_of_memory;
    }

    while (bytesRead < rspSize) {
        size_t nBytes = 0;
        ret = ConnectionRead[protoPtr->connPtr->type](protoPtr->connPtr,
                                                      rspBufPtr + bytesRead,
                                                      rspSize - bytesRead,
                                                      &nBytes);
        if (ret != w5_com_ret_ok) {
            free(rspBufPtr);
            return w5_com_ret_general_error;
        }
        else if (nBytes > 0) {
            bytesRead += nBytes;
            retries = 0;
        }
        else if (retries < READ_RETRIES) { // read 0 bytes, retry read
            ++retries;
        }
        else { // exhausted read retries
            free(rspBufPtr);
            return w5_com_ret_timed_out;
        }
    }

    if (protoPtr->verbosityLevel == verbosity_verbose) {
        printf("RX: ");
        size_t i;
        for (i = 0; i < bytesRead; i++) {
            printf("%.2x ", rspBufPtr[i]);
        }
        printf("\n");
    }

    ret = w5_com_ret_ok;
    if ((rspBufPtr[0]           == W5_UART_START_FRAME)
        && (rspBufPtr[rsp_pkt_uart_off_proto_id] == W5_V3_CODE)
        && (rspBufPtr[rsp_pkt_uart_off_msg_type] == msgType)
        && (rspBufPtr[rspSize - 1] == W5_UART_END_FRAME)) {

        if (msgType == PROTOCOL_MSG_TYPE_RESULT) {
            //checksum excludes checksum, start frame, end frame
            if (calcCheckSum(&rspBufPtr[rsp_pkt_uart_off_proto_id], rspSize - 3) != rspBufPtr[rspSize - 2]) {
                ret = w5_com_ret_rsp_cs_failed;  // check sum does not match
            }
            else if (rspBufPtr[rsp_pkt_uart_off_ack_nack] == W5_RSP_NACK_CS) {
                ret = w5_com_ret_cmd_cs_failed;
            }
            else if (rspBufPtr[rsp_pkt_uart_off_ack_nack] == W5_RSP_NACK_BUSY) {
                ret = w5_com_ret_device_busy;
            }
            else if (rspBufPtr[rsp_pkt_uart_off_ack_nack] == W5_RSP_NACK_ERR) {
                ret = w5_com_ret_general_error;
            }
            else if (rspBufPtr[rsp_pkt_uart_off_ack_nack] == W5_RSP_ACK) { // ACK
                *rspMsgId = rspBufPtr[rsp_pkt_uart_off_msg_id];
                memcpy(readPayloadBufPtr, &rspBufPtr[rsp_pkt_uart_off_cmd_status], readPayloadSizeInBytes);
            }
        }
        else {
            //checksum excludes checksum, start frame, end frame
            if (calcCheckSum(&rspBufPtr[ack_pkt_uart_off_proto_id], rspSize - 3) != rspBufPtr[rspSize - 2]) {
                ret = w5_com_ret_rsp_cs_failed;  // check sum does not match
            }
            else if (rspBufPtr[ack_pkt_uart_off_ack_nack] == W5_RSP_NACK_CS) {
                ret = w5_com_ret_cmd_cs_failed;
            }
            else if (rspBufPtr[ack_pkt_uart_off_ack_nack] == W5_RSP_NACK_BUSY) {
                ret = w5_com_ret_device_busy;
            }
            else if (rspBufPtr[ack_pkt_uart_off_ack_nack] == W5_RSP_NACK_ERR) {
                ret = w5_com_ret_general_error;
            }
            else if (rspBufPtr[ack_pkt_uart_off_ack_nack] == W5_RSP_ACK) { // ACK
                *rspMsgId = rspBufPtr[ack_pkt_uart_off_msg_id];
            }
        }
    }
    else {
        ret = w5_com_ret_general_error;
    }

    free(rspBufPtr);
    return ret;
}

static W5ComReturn_e detectDuplex(const W5Connection_s *connPtr,
                                  W5UartDuplex_e *duplexPtr)
{
    W5ComReturn_e ret;
    ConnRead_f connRead = ConnectionRead[connPtr->type];
    ConnWrite_f connWrite = ConnectionWrite[connPtr->type];
    uint32_t testData = TEST_DATA;
    size_t nBytes = 0;
    uint8_t *bufPtr = (uint8_t *)&testData;
    size_t readSize = sizeof(testData);
    int retries = 0;

    ret = connWrite(connPtr, (uint8_t *)&testData, sizeof(testData), &nBytes,
                    readSize);
    if ((ret != w5_com_ret_ok)
        || (sizeof(testData) != nBytes)) {
        return w5_com_ret_general_error;
    }

    testData = 0;
    for (retries = 4; retries > 0; --retries) {
        nBytes = 0;
        ret = connRead(connPtr, bufPtr, readSize, &nBytes);
        if (ret != w5_com_ret_ok) {
            return ret;
        }

        readSize -= nBytes;
        if (readSize == 0) {
            break;
        }
        bufPtr += nBytes;
    }

    if (readSize == sizeof(testData)) {
        *duplexPtr = w5_uart_duplex_full;
    }
    else if ((readSize == 0)
             && (testData == TEST_DATA)) {
        *duplexPtr = w5_uart_duplex_half;
    }
    else {
        *duplexPtr = w5_uart_duplex_unrecognized;
    }

    return w5_com_ret_ok;
}

static inline void getW5protoUartOverheads(const W5ComConfig_s *configPtr, size_t *maxCmdOverhead, size_t *maxRspOverhead)
{
    // check for supported protocol version
    switch (configPtr->protoVersion) {
    case w5_protocol_v3:
    default:
        *maxCmdOverhead = MAX_W5_UART_V3_CMD_OVERHEAD;
        *maxRspOverhead = MAX_W5_UART_V3_RSP_OVERHEAD;
        break;
    }
}

static inline size_t getW5protoUartMaxBytesPerWrite(const W5Connection_s *currentConnPtr, 
                                                   size_t maxCmdOverhead,
                                                   size_t maxWriteSizeRequested)
{
    size_t connMaxBytesPerWrite = currentConnPtr->maxBytesPerWrite - maxCmdOverhead;

    // protocol size is upper-bounded by physical connection limit
    return (maxWriteSizeRequested > connMaxBytesPerWrite) ? connMaxBytesPerWrite : maxWriteSizeRequested;
}

static inline size_t getW5protoUartMaxBytesPerRead(const W5Connection_s *currentConnPtr, 
                                                   size_t maxRspOverhead,
                                                   size_t maxReadSizeRequested)
{
    size_t  connMaxBytesPerRead = currentConnPtr->maxBytesPerRead - maxRspOverhead;

    // protocol size is upper-bounded by physical connection limit
    return (maxReadSizeRequested > connMaxBytesPerRead) ? connMaxBytesPerRead : maxReadSizeRequested;
}


/*******************************************************************************
 Global Scope Function Definitions
*******************************************************************************/
W5ComReturn_e W5protoUartAttach(const W5ComConfig_s *configPtr,
                                W5Connection_s *currentConnPtr,
                                W5Protocol_s **protoPtrPtr)
{
    W5Protocol_s *protocolPtr;
    W5ComReturn_e ret;
    size_t maxCmdOverhead = 0;
    size_t maxRspOverhead = 0;

    if ((configPtr == NULL)
        || (configPtr->protoParams.uart.duplex >= w5_num_uart_duplexes)
        || (currentConnPtr == NULL)
        || (protoPtrPtr == NULL)
        || (*protoPtrPtr != NULL)) {
        return w5_com_ret_invalid_param;
    }

    getW5protoUartOverheads(configPtr, &maxCmdOverhead, &maxRspOverhead);

    // check for connection read and write support for this protocol
    if ((ConnectionRead[currentConnPtr->type] == NULL)
        || (ConnectionWrite[currentConnPtr->type] == NULL)) {
        return w5_com_ret_not_supported;
    }

    protocolPtr = (W5Protocol_s *)calloc(1, sizeof(*protocolPtr));
    if (protocolPtr == NULL) {
        return w5_com_ret_out_of_memory;
    }

    ret = ConnectionConfigure[currentConnPtr->type](currentConnPtr,
                                                    &configPtr->connParams);
    if (ret != w5_com_ret_ok) {
        free(protocolPtr);
        return ret;
    }

    if (configPtr->protoParams.uart.duplex == w5_uart_duplex_unspecified) {
        ret = detectDuplex(currentConnPtr, &protocolPtr->uartInfo.duplex);
        if (ret != w5_com_ret_ok) {
            protocolPtr->uartInfo.duplex = w5_uart_duplex_unrecognized;
        }
    }
    else {
        protocolPtr->uartInfo.duplex = configPtr->protoParams.uart.duplex;
    }
    if (protocolPtr->uartInfo.duplex == w5_uart_duplex_unrecognized) {
        free(protocolPtr);
        return w5_com_ret_general_error;
    }

    protocolPtr->maxBytesPerWrite = 
        getW5protoUartMaxBytesPerWrite(currentConnPtr, maxCmdOverhead, UART_MAX_WRITE_SIZE);
    protocolPtr->maxBytesPerRead = 
        getW5protoUartMaxBytesPerRead(currentConnPtr, maxRspOverhead, UART_MAX_READ_SIZE);
    protocolPtr->connPtr = currentConnPtr;
    protocolPtr->type = w5_protocol_type_uart;
    protocolPtr->version = configPtr->protoVersion;
    *protoPtrPtr = protocolPtr;

    return ret;
}

W5ComReturn_e W5protoUartReconfigMaxBytes(const W5ComConfig_s *configPtr,
                                          const W5Connection_s *currentConnPtr,
                                          W5Protocol_s *protoPtr,
                                          size_t maxBytesPerWriteRequested,
                                          size_t maxBytesPerReadRequested)
{
    size_t maxCmdOverhead = 0;
    size_t maxRspOverhead = 0;

    getW5protoUartOverheads(configPtr, &maxCmdOverhead, &maxRspOverhead);
    protoPtr->maxBytesPerWrite = 
        getW5protoUartMaxBytesPerWrite(currentConnPtr, maxCmdOverhead, maxBytesPerWriteRequested);
    protoPtr->maxBytesPerRead = 
        getW5protoUartMaxBytesPerRead(currentConnPtr, maxRspOverhead, maxBytesPerReadRequested);

    return w5_com_ret_ok;
}

W5ComReturn_e W5protoUartProcessW5Commands(const W5Protocol_s *protoPtr,
                                           W5Command_s *cmdQueuePtr)
{
    int cmdNumInQueue = 0;
    W5ComReturn_e ret = w5_com_ret_ok;

    if ((protoPtr == NULL)
        || (cmdQueuePtr == NULL)) {
        return w5_com_ret_invalid_param;
    }

    while (cmdQueuePtr != NULL) {
        W5Command_s *cmdPtr = cmdQueuePtr;
        W5ComReturn_e cmdResult;
        int retries;

        switch (protoPtr->version) {
        case w5_protocol_v3:
        default:
            retries = COMMAND_RETRIES;
            break;
        }

        while (retries) {
            cmdResult = sendW5UartCommand(protoPtr, cmdPtr->msgType,
                                          cmdPtr->writeSizeInBytes,
                                          cmdPtr->writeDataPtr,
                                          cmdPtr->readSizeInBytes);
            if (cmdResult != w5_com_ret_ok) {
                break;
            }

            cmdResult = receiveW5UartResponse(protoPtr, cmdPtr->rspMsgType,
                                              cmdPtr->readSizeInBytes,
                                              cmdPtr->readBufferPtr,
                                              &cmdPtr->rspMsgId);

            if (cmdResult == w5_com_ret_cmd_cs_failed) {
                retries--;
            }
            else if (cmdResult == w5_com_ret_device_busy) {
                // try infinitely while GW5 responds busy
                // (don't count this case as a "REtry"; busy means the "try" didn't even go through)
                continue;
            }
            else {
                break;
            }
        }

        if (cmdPtr->resultCb != NULL) {
            size_t bytesWritten = 0;
            size_t bytesRead = 0;
            if (cmdResult == w5_com_ret_ok) {
                bytesWritten = cmdPtr->writeSizeInBytes;
                bytesRead = cmdPtr->readSizeInBytes;
            }
            cmdPtr->resultCb(cmdResult, ++cmdNumInQueue, bytesWritten,
                             bytesRead, cmdPtr->userDataPtr);
        }

        if (cmdResult != w5_com_ret_ok) {
            ret = cmdResult;
            break;
        }
        cmdQueuePtr = cmdPtr->nextPtr;
    }

    return ret;
}

W5ComReturn_e W5protoUartDetach(W5Protocol_s **protoPtrPtr)
{
    if ((protoPtrPtr == NULL)
        || (*protoPtrPtr == NULL)) {
        return w5_com_ret_invalid_param;
    }

    free(*protoPtrPtr);
    *protoPtrPtr = NULL;

    return w5_com_ret_ok;
}

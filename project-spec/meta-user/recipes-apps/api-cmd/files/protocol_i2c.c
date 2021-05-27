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
* \file protocol_i2c.c
* \brief W5 I2C communication protocol handling implementation.
* \addtogroup W5PROTO
*
* W5 I2C communication protocol handling implementation.
*/


/*******************************************************************************
Includes
*******************************************************************************/
#include <stdlib.h>
#include <string.h>
#include "w5_connection.h"
#include "protocol.h"
#include "connection.h"
#include "platform.h"


/*******************************************************************************
Exported Variable Definitions
*******************************************************************************/


/*******************************************************************************
Extern Inline Function Declarations
*******************************************************************************/


/*******************************************************************************
Internal Macro, Enum, Union, Structure, And Type Definitions
*******************************************************************************/
#define W5_I2C_V3_API_CMD_OVERHEAD (W5_V3_API_CMD_OVERHEAD)
#define W5_I2C_V3_QUERY_CMD_OVERHEAD (W5_V3_QUERY_CMD_OVERHEAD)
#define MAX_W5_I2C_V3_CMD_OVERHEAD (MAX_W5_V3_CMD_OVERHEAD)

#define W5_I2C_V3_API_RSP_OVERHEAD (W5_V3_API_RSP_OVERHEAD)
#define W5_I2C_V3_QUERY_RSP_OVERHEAD (W5_V3_QUERY_RSP_OVERHEAD)
#define MAX_W5_I2C_V3_RSP_OVERHEAD (MAX_W5_V3_RSP_OVERHEAD)

#define I2C_MAX_WRITE_SIZE (MAX_PROTOCOL_MSG_API_CMD_PAYLOAD_SIZE)
#define I2C_MAX_READ_SIZE (MAX_PROTOCOL_MSG_QUERY_RSP_PAYLOAD_SIZE)


/*******************************************************************************
File Scope Variable Declarations And Definitions
*******************************************************************************/
static const ConnConfigure_f ConnectionConfigure[w5_num_connection_types] = {
    NULL, // w5_connection_type_unspecified
    NULL, // w5_connection_type_serial
    W5connFtdi4222ConfigureI2c // w5_connection_type_ftdi4222
};

static const ConnRead_f ConnectionRead[w5_num_connection_types] = {
    NULL, // w5_connection_type_unspecified
    NULL, // w5_connection_type_serial
    W5connFtdi4222Read // w5_connection_type_ftdi4222
};

static const ConnWrite_f ConnectionWrite[w5_num_connection_types] = {
    NULL, // w5_connection_type_unspecified
    NULL, // w5_connection_type_serial
    W5connFtdi4222Write // w5_connection_type_ftdi4222
};

static const uint16_t DelayWriteTimeUsPerByte[w5_num_i2c_delay_write_profiles] = {
    90, // w5_i2c_delay_write_profile_default
    200 // w5_i2c_delay_write_profile_conservative
};


/*******************************************************************************
File Scope Function Prototypes
*******************************************************************************/
static inline W5ComReturn_e sendW5I2cCommand(const W5Protocol_s *protoPtr,
                                             uint8_t msgType,
                                             size_t writePayloadSizeInBytes,
                                             const uint8_t *writePayloadPtr,
                                             size_t readPayloadSizeInBytes);

static inline W5ComReturn_e receiveW5I2cResponse(const W5Protocol_s *protoPtr,
                                                 uint8_t msgType,
                                                 size_t rspSize,
                                                 uint8_t *rspBufPtr,
                                                 uint8_t *rspMsgId);

static inline void getW5protoI2cOverheads(const W5ComConfig_s *configPtr,
                                          size_t *maxCmdOverhead,
                                          size_t *maxRspOverhead);

static inline size_t getW5protoI2cMaxBytesPerWrite(const W5Connection_s *currentConnPtr, 
                                                   size_t maxCmdOverhead,
                                                   size_t maxWriteSizeRequested);

static inline size_t getW5protoI2cMaxBytesPerRead(const W5Connection_s *currentConnPtr, 
                                                  size_t maxRspOverhead,
                                                  size_t maxReadSizeRequested);


/*******************************************************************************
File Scope Function Definitions
*******************************************************************************/
static inline W5ComReturn_e sendW5I2cCommand(const W5Protocol_s *protoPtr,
                                             uint8_t msgType,
                                             size_t writePayloadSizeInBytes,
                                             const uint8_t *writePayloadPtr,
                                             size_t readPayloadSizeInBytes)
{
    uint8_t *cmdBufPtr = NULL;
    size_t cmdSize = writePayloadSizeInBytes;
    size_t bytesWritten = 0;
    uint32_t delayWriteTime;
    uint8_t checksum;
    W5ComReturn_e ret;

    switch (protoPtr->version) {
    case w5_protocol_v3:
    default:
        if (msgType == PROTOCOL_MSG_TYPE_QUERY) {
            cmdSize += W5_I2C_V3_QUERY_CMD_OVERHEAD;
        }
        else {
            cmdSize += W5_I2C_V3_API_CMD_OVERHEAD;
        }
        break;
    }

    cmdBufPtr = (uint8_t *)calloc(1, cmdSize);
    if (cmdBufPtr == NULL) {
        return w5_com_ret_out_of_memory;
    }

    cmdBufPtr[geo_api_pkt_i2c_off_proto_id] = W5_V3_CODE;
    cmdBufPtr[geo_api_pkt_i2c_off_msg_type] = msgType;
    if (msgType == PROTOCOL_MSG_TYPE_QUERY) {
        cmdBufPtr[query_pkt_i2c_off_data_size] = (uint8_t)(writePayloadSizeInBytes & 0xff);

        if (writePayloadSizeInBytes > 0) {
            memcpy(&cmdBufPtr[query_pkt_i2c_off_data_size + 1], writePayloadPtr, writePayloadSizeInBytes);
        }

        // cmdSize minus 1: checksum does not apply on checksum field itself
        checksum = calcCheckSum((const uint8_t *)&cmdBufPtr[query_pkt_i2c_off_proto_id], cmdSize - 1);
        cmdBufPtr[query_pkt_i2c_off_checksum] = checksum;

        readPayloadSizeInBytes += W5_I2C_V3_QUERY_RSP_OVERHEAD;
    }
    else {
        cmdBufPtr[geo_api_pkt_i2c_off_data_size_byte_0_lsb] = (uint8_t)(writePayloadSizeInBytes & 0xff);
        cmdBufPtr[geo_api_pkt_i2c_off_data_size_byte_1] = (uint8_t)((writePayloadSizeInBytes >> 8) & 0xff);
        cmdBufPtr[geo_api_pkt_i2c_off_data_size_byte_2] = (uint8_t)((writePayloadSizeInBytes >> 16) & 0xff);
        cmdBufPtr[geo_api_pkt_i2c_off_data_size_byte_3_msb] = (uint8_t)((writePayloadSizeInBytes >> 24) & 0xff);

        if (writePayloadSizeInBytes > 0) {
            memcpy(&cmdBufPtr[geo_api_pkt_i2c_off_data_size_byte_3_msb + 1], writePayloadPtr, writePayloadSizeInBytes);
        }

        // cmdSize minus 1: checksum does not apply on checksum field itself
        checksum = calcCheckSum((const uint8_t *)&cmdBufPtr[geo_api_pkt_i2c_off_proto_id], cmdSize - 1);
        cmdBufPtr[geo_api_pkt_i2c_off_data_size_byte_3_msb + 1 + writePayloadSizeInBytes + geo_api_pkt_i2c_after_payload_off_checksum] = checksum;

        readPayloadSizeInBytes += W5_I2C_V3_API_RSP_OVERHEAD;
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
    delayWriteTime =
        DelayWriteTimeUsPerByte[protoPtr->i2cInfo.delayWriteProfile]
        * bytesWritten;
    PlatDelayUs(delayWriteTime);
    if ((ret != w5_com_ret_ok) || (bytesWritten != cmdSize)) {
        // write failed
        ret = w5_com_ret_general_error;
    }

    free(cmdBufPtr);

    return ret;
}

static inline W5ComReturn_e receiveW5I2cResponse(const W5Protocol_s *protoPtr,
                                                 uint8_t msgType,
                                                 size_t readPayloadSizeInBytes,
                                                 uint8_t *readPayloadBufPtr,
                                                 uint8_t *rspMsgId)
{
    uint8_t *rspBufPtr;
    size_t rspSize = readPayloadSizeInBytes;
    size_t bytesRead = 0;
    int retries = 0;
    W5ComReturn_e ret = w5_com_ret_ok;

    switch (protoPtr->version) {
    case w5_protocol_v3:
    default:
        if (msgType == PROTOCOL_MSG_TYPE_RESULT) {
            rspSize += W5_I2C_V3_QUERY_RSP_OVERHEAD;
        }
        else {
            rspSize += W5_I2C_V3_API_RSP_OVERHEAD;
        }
        break;
    }
    rspBufPtr = (uint8_t *)malloc(rspSize);
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
    if ((rspBufPtr[rsp_pkt_i2c_off_proto_id] == W5_V3_CODE)
        && (rspBufPtr[rsp_pkt_i2c_off_msg_type] == msgType)) {

        if (msgType == PROTOCOL_MSG_TYPE_RESULT) {
            // checksum excludes checksum byte
            if (calcCheckSum(&rspBufPtr[rsp_pkt_i2c_off_proto_id], rspSize - 1) != rspBufPtr[rspSize - 1]) {
                ret = w5_com_ret_rsp_cs_failed;  // check sum does not match
            }
            else if (rspBufPtr[rsp_pkt_i2c_off_ack_nack] == W5_RSP_NACK_CS) {
                ret = w5_com_ret_cmd_cs_failed;
            }
            else if (rspBufPtr[rsp_pkt_i2c_off_ack_nack] == W5_RSP_NACK_BUSY) {
                ret = w5_com_ret_device_busy;
            }
            else if (rspBufPtr[rsp_pkt_i2c_off_ack_nack] == W5_RSP_NACK_ERR) {
                ret = w5_com_ret_general_error;
            }
            else if (rspBufPtr[rsp_pkt_i2c_off_ack_nack] == W5_RSP_ACK) { // ACK
                *rspMsgId = rspBufPtr[rsp_pkt_i2c_off_msg_id];
                memcpy(readPayloadBufPtr, &rspBufPtr[rsp_pkt_i2c_off_cmd_status], readPayloadSizeInBytes);
            }
        }
        else {
            // checksum excludes checksum byte
            if (calcCheckSum(&rspBufPtr[ack_pkt_i2c_off_proto_id], rspSize - 1) != rspBufPtr[rspSize - 1]) {
                ret = w5_com_ret_rsp_cs_failed;  // check sum does not match
            }
            else if (rspBufPtr[ack_pkt_i2c_off_ack_nack] == W5_RSP_NACK_CS) {
                ret = w5_com_ret_cmd_cs_failed;
            }
            else if (rspBufPtr[ack_pkt_i2c_off_ack_nack] == W5_RSP_NACK_BUSY) {
                ret = w5_com_ret_device_busy;
            }
            else if (rspBufPtr[ack_pkt_i2c_off_ack_nack] == W5_RSP_NACK_ERR) {
                ret = w5_com_ret_general_error;
            }
            else if (rspBufPtr[ack_pkt_i2c_off_ack_nack] == W5_RSP_ACK) { // ACK
                *rspMsgId = rspBufPtr[ack_pkt_i2c_off_msg_id];
            }
        }
    }
    else {
        ret = w5_com_ret_general_error;
    }
    free(rspBufPtr);

    return ret;
}

static inline void getW5protoI2cOverheads(const W5ComConfig_s *configPtr, size_t *maxCmdOverhead, size_t *maxRspOverhead)
{
    // check for supported protocol version
    switch (configPtr->protoVersion) {
    case w5_protocol_v3:
    default:
        *maxCmdOverhead = MAX_W5_I2C_V3_CMD_OVERHEAD;
        *maxRspOverhead = MAX_W5_I2C_V3_RSP_OVERHEAD;
        break;
    }
}

static inline size_t getW5protoI2cMaxBytesPerWrite(const W5Connection_s *currentConnPtr, 
                                                   size_t maxCmdOverhead,
                                                   size_t maxWriteSizeRequested)
{
    size_t connMaxBytesPerWrite = currentConnPtr->maxBytesPerWrite - maxCmdOverhead;

    // protocol size is upper-bounded by physical connection limit
    return (maxWriteSizeRequested > connMaxBytesPerWrite) ? connMaxBytesPerWrite : maxWriteSizeRequested;
}

static inline size_t getW5protoI2cMaxBytesPerRead(const W5Connection_s *currentConnPtr, 
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
W5ComReturn_e W5protoI2cAttach(const W5ComConfig_s *configPtr,
                               W5Connection_s *currentConnPtr,
                               W5Protocol_s **protoPtrPtr)
{
    W5Protocol_s *protocolPtr;
    W5ComReturn_e ret;
    size_t maxCmdOverhead = 0;
    size_t maxRspOverhead = 0;

    if ((configPtr == NULL)
        || (configPtr->protoParams.i2c.delayWriteProfile >=
            w5_num_i2c_delay_write_profiles)
        || (currentConnPtr == NULL)
        || (protoPtrPtr == NULL)
        || (*protoPtrPtr != NULL)) {
        return w5_com_ret_invalid_param;
    }

    getW5protoI2cOverheads(configPtr, &maxCmdOverhead, &maxRspOverhead);

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

    protocolPtr->i2cInfo.address = configPtr->protoParams.i2c.address;
    protocolPtr->i2cInfo.delayWriteProfile =
        configPtr->protoParams.i2c.delayWriteProfile;
    protocolPtr->maxBytesPerWrite = 
        getW5protoI2cMaxBytesPerWrite(currentConnPtr, maxCmdOverhead, I2C_MAX_WRITE_SIZE);
    protocolPtr->maxBytesPerRead = 
        getW5protoI2cMaxBytesPerRead(currentConnPtr, maxRspOverhead, I2C_MAX_READ_SIZE);
    protocolPtr->connPtr = currentConnPtr;
    protocolPtr->type = w5_protocol_type_i2c;
    protocolPtr->version = configPtr->protoVersion;
    *protoPtrPtr = protocolPtr;

    return ret;
}

W5ComReturn_e W5protoI2cReconfigMaxBytes(const W5ComConfig_s *configPtr,
                                         const W5Connection_s *currentConnPtr,
                                         W5Protocol_s *protoPtr,
                                         size_t maxBytesPerWriteRequested,
                                         size_t maxBytesPerReadRequested)
{
    size_t maxCmdOverhead = 0;
    size_t maxRspOverhead = 0;

    getW5protoI2cOverheads(configPtr, &maxCmdOverhead, &maxRspOverhead);
    protoPtr->maxBytesPerWrite = 
        getW5protoI2cMaxBytesPerWrite(currentConnPtr, maxCmdOverhead, maxBytesPerWriteRequested);
    protoPtr->maxBytesPerRead = 
        getW5protoI2cMaxBytesPerRead(currentConnPtr, maxRspOverhead, maxBytesPerReadRequested);

    return w5_com_ret_ok;
}

W5ComReturn_e W5protoI2cProcessW5Commands(const W5Protocol_s *protoPtr,
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

        while (retries > 0) {
            cmdResult = sendW5I2cCommand(protoPtr, cmdPtr->msgType,
                                         cmdPtr->writeSizeInBytes,
                                         cmdPtr->writeDataPtr,
                                         cmdPtr->readSizeInBytes);

            if (cmdResult != w5_com_ret_ok) {
                break;
            }

            cmdResult = receiveW5I2cResponse(protoPtr, cmdPtr->rspMsgType,
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

W5ComReturn_e W5protoI2cDetach(W5Protocol_s **protoPtrPtr)
{
    if ((protoPtrPtr == NULL)
        || (*protoPtrPtr == NULL)) {
        return w5_com_ret_invalid_param;
    }

    free(*protoPtrPtr);
    *protoPtrPtr = NULL;

    return w5_com_ret_ok;
}

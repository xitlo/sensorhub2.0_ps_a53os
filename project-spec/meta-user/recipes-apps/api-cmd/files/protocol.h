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
 * \file protocol.h
 * \brief W5COM library W5 communication protocol private declarations.
 * \defgroup W5PROTO W5 Communication Protocol
 * \ingroup W5COM
 *
 * W5COM library W5 communication protocol private declarations.
 *
 * @{
 */

#ifndef __PROTOCOL_H__
#define __PROTOCOL_H__

/*******************************************************************************
 Includes
*******************************************************************************/
#include <stdio.h>
#include "api.h"
#include "w5_com.h"
#include "w5_connection.h"
#include "platform.h"


/*******************************************************************************
 Exported Macro, Enum, Union, Structure, And Type Definitions
*******************************************************************************/
// #ifdef __cplusplus
// extern "C"
// {
// #endif // __cplusplus

typedef struct W5Protocol {
    W5ProtocolType_e type;
    W5Connection_s *connPtr;
    union {
        struct W5UartParams uartInfo;
        struct W5I2cParams i2cInfo;
    };
    size_t maxBytesPerRead;
    size_t maxBytesPerWrite;
    W5ProtocolVersion_e version;
    W5ComVerbosity_e verbosityLevel;
} W5Protocol_s;

typedef struct W5Command {
    uint8_t msgType;
    uint8_t rspMsgType;
    uint8_t msgId;
    uint8_t rspMsgId;
    uint8_t apiCode;
    size_t writeSizeInBytes;
    const uint8_t *writeDataPtr;
    size_t readSizeInBytes;
    uint8_t *readBufferPtr;
    W5CommandResultCallback_f resultCb;
    void *userDataPtr;
    struct W5Command *nextPtr;
} W5Command_s;

typedef W5ComReturn_e(*ProtoAttach_f)(const W5ComConfig_s *configPtr,
                                      W5Connection_s *currentConnPtr,
                                      W5Protocol_s **protoPtrPtr);

typedef W5ComReturn_e(*ProtoW5CommandsHandler_f)(const W5Protocol_s *protoPtr,
                                                 W5Command_s *cmdQueuePtr);

typedef W5ComReturn_e(*ProtoReconfigMaxBytes_f)(const W5ComConfig_s *configPtr,
                                                const W5Connection_s *currentConnPtr,
                                                W5Protocol_s *protoPtr,
                                                size_t maxBytesPerWrite,
                                                size_t maxBytesPerRead);

typedef W5ComReturn_e(*ProtoDetach_f)(W5Protocol_s **protoPtrPtr);

#define W5_V3_CODE  PROTOCOL_VERSION

// Ack/Nack code
#define W5_RSP_ACK       ACK
#define W5_RSP_NACK_ERR  NACK_GENERAL_ERROR
#define W5_RSP_NACK_CS   NACK_FAILED_CHECKSUM
#define W5_RSP_NACK_BUSY NACK_BUSY // command not accepted

// API command overhead consists of
// protocol version(1), message type(1), write payload size(4),
// and checksum(1)
#define W5_V3_API_CMD_OVERHEAD (7U)
// Query command overhead consists of
// protocol version(1), message type(1), write payload size(1),
// and checksum(1)
#define W5_V3_QUERY_CMD_OVERHEAD (4U)
#define MAX_W5_V3_CMD_OVERHEAD (W5_V3_API_CMD_OVERHEAD)
// API response overhead consists of
// protocol version(1), message type(1), read payload size(1), message ID(1),
// ACK/NACK (1), and checksum (1)
#define W5_V3_API_RSP_OVERHEAD (6U)
// Query response overhead consists of
// protocol version(1), message type(1), read payload size(4), message ID(1),
// ACK/NACK (1), and checksum (1)
#define W5_V3_QUERY_RSP_OVERHEAD (9U)
#define MAX_W5_V3_RSP_OVERHEAD (W5_V3_QUERY_RSP_OVERHEAD)

#define READ_RETRIES (10)
#define COMMAND_RETRIES (8)

typedef enum W5ComV3ApiPacketUart {
    // Offset
    geo_api_pkt_uart_off_frame_start              = 0,
    geo_api_pkt_uart_off_proto_id,             // = 1
    geo_api_pkt_uart_off_msg_type,             // = 2
    geo_api_pkt_uart_off_data_size_byte_0_lsb, // = 3
    geo_api_pkt_uart_off_data_size_byte_1,     // = 4
    geo_api_pkt_uart_off_data_size_byte_2,     // = 5
    geo_api_pkt_uart_off_data_size_byte_3_msb, // = 6
    geo_api_pkt_uart_off_api_code,             // = 7
    geo_api_pkt_uart_off_chunk_num_lsb,        // = 8
    geo_api_pkt_uart_off_chunk_num_msb,        // = 9
    geo_api_pkt_uart_off_api_payload,          // = 10
    geo_api_pkt_uart_min_off_after_payload = geo_api_pkt_uart_off_api_payload,
    geo_api_pkt_uart_max_off_after_payload = geo_api_pkt_uart_off_data_size_byte_3_msb + 1 + MAX_PROTOCOL_MSG_API_CMD_PAYLOAD_SIZE,
    geo_api_pkt_uart_after_payload_off_checksum         = 0,
    geo_api_pkt_uart_after_payload_off_frame_end,    // = 1

    cus_api_pkt_uart_off_frame_start          = geo_api_pkt_uart_off_frame_start,
    cus_api_pkt_uart_off_proto_id             = geo_api_pkt_uart_off_proto_id,
    cus_api_pkt_uart_off_msg_type             = geo_api_pkt_uart_off_msg_type,
    cus_api_pkt_uart_off_data_size_byte_0_lsb = geo_api_pkt_uart_off_data_size_byte_0_lsb,
    cus_api_pkt_uart_off_data_size_byte_1     = geo_api_pkt_uart_off_data_size_byte_1,
    cus_api_pkt_uart_off_data_size_byte_2     = geo_api_pkt_uart_off_data_size_byte_2,
    cus_api_pkt_uart_off_data_size_byte_3_msb = geo_api_pkt_uart_off_data_size_byte_3_msb,
    cus_api_pkt_uart_off_cus_payload,      // = geo_api_pkt_uart_off_data_size_byte_3_msb + 1
    cus_api_pkt_uart_min_off_after_payload    = cus_api_pkt_uart_off_cus_payload,
    cus_api_pkt_uart_max_off_after_payload    = cus_api_pkt_uart_off_data_size_byte_3_msb + 1 + MAX_PROTOCOL_MSG_API_CMD_PAYLOAD_SIZE,
    cus_api_pkt_uart_after_payload_off_checksum         = 0,
    cus_api_pkt_uart_after_payload_off_frame_end,    // = 1

    // Length
    geo_api_pkt_uart_len_no_payload     = geo_api_pkt_uart_min_off_after_payload + geo_api_pkt_uart_after_payload_off_frame_end + 1,
    geo_api_pkt_uart_len_max_payload    = geo_api_pkt_uart_max_off_after_payload + geo_api_pkt_uart_after_payload_off_frame_end + 1,
    cus_api_pkt_uart_len_no_payload     = cus_api_pkt_uart_min_off_after_payload + cus_api_pkt_uart_after_payload_off_frame_end + 1,
    cus_api_pkt_uart_len_max_payload    = cus_api_pkt_uart_max_off_after_payload + cus_api_pkt_uart_after_payload_off_frame_end + 1
} W5ComV3ApiPacketUart_e;

typedef enum W5ComV3ApiPacketI2c {
    // Offset
    geo_api_pkt_i2c_off_proto_id                 = 0,
    geo_api_pkt_i2c_off_msg_type,             // = 1
    geo_api_pkt_i2c_off_data_size_byte_0_lsb, // = 2
    geo_api_pkt_i2c_off_data_size_byte_1,     // = 3
    geo_api_pkt_i2c_off_data_size_byte_2,     // = 4
    geo_api_pkt_i2c_off_data_size_byte_3_msb, // = 5
    geo_api_pkt_i2c_off_api_code,             // = 6
    geo_api_pkt_i2c_off_chunk_num_lsb,        // = 7
    geo_api_pkt_i2c_off_chunk_num_msb,        // = 8
    geo_api_pkt_i2c_off_api_payload,          // = 9
    geo_api_pkt_i2c_min_off_after_payload = geo_api_pkt_i2c_off_api_payload,
    geo_api_pkt_i2c_max_off_after_payload = geo_api_pkt_i2c_off_data_size_byte_3_msb + 1 + MAX_PROTOCOL_MSG_API_CMD_PAYLOAD_SIZE,
    geo_api_pkt_i2c_after_payload_off_checksum  = 0,

    cus_api_pkt_i2c_off_proto_id             = geo_api_pkt_i2c_off_proto_id,
    cus_api_pkt_i2c_off_msg_type             = geo_api_pkt_i2c_off_msg_type,
    cus_api_pkt_i2c_off_data_size_byte_0_lsb = geo_api_pkt_i2c_off_data_size_byte_0_lsb,
    cus_api_pkt_i2c_off_data_size_byte_1     = geo_api_pkt_i2c_off_data_size_byte_1,
    cus_api_pkt_i2c_off_data_size_byte_2     = geo_api_pkt_i2c_off_data_size_byte_2,
    cus_api_pkt_i2c_off_data_size_byte_3_msb = geo_api_pkt_i2c_off_data_size_byte_3_msb,
    cus_api_pkt_i2c_off_cus_payload,      // = geo_api_pkt_i2c_off_data_size_byte_3_msb + 1
    cus_api_pkt_i2c_min_off_after_payload    = cus_api_pkt_i2c_off_cus_payload,
    cus_api_pkt_i2c_max_off_after_payload    = cus_api_pkt_i2c_off_data_size_byte_3_msb + 1 + MAX_PROTOCOL_MSG_API_CMD_PAYLOAD_SIZE,
    cus_api_pkt_i2c_after_payload_off_checksum  = 0,

    // Length
    geo_api_pkt_i2c_len_no_payload  = geo_api_pkt_i2c_min_off_after_payload + geo_api_pkt_i2c_after_payload_off_checksum + 1,
    geo_api_pkt_i2c_len_max_payload = geo_api_pkt_i2c_max_off_after_payload + geo_api_pkt_i2c_after_payload_off_checksum + 1,
    cus_api_pkt_i2c_len_no_payload  = cus_api_pkt_i2c_min_off_after_payload + cus_api_pkt_i2c_after_payload_off_checksum + 1,
    cus_api_pkt_i2c_len_max_payload = cus_api_pkt_i2c_max_off_after_payload + cus_api_pkt_i2c_after_payload_off_checksum + 1
} W5ComV3ApiPacketI2c_e;

// Only bytes after dataSize are passed back from W5_COM CallApi() as byte buffer
// Since relative offset are the same for UART and I2C, we use I2C's absolute offset for calculation
typedef enum W5ComV3GeoApiPacketCmd {
    geo_api_pkt_cmd_off_api_code      = geo_api_pkt_i2c_off_api_code      - geo_api_pkt_i2c_off_api_code,
    geo_api_pkt_cmd_off_chunk_num_lsb = geo_api_pkt_i2c_off_chunk_num_lsb - geo_api_pkt_i2c_off_api_code,
    geo_api_pkt_cmd_off_chunk_num_msb = geo_api_pkt_i2c_off_chunk_num_msb - geo_api_pkt_i2c_off_api_code,
    geo_api_pkt_cmd_off_api_payload   = geo_api_pkt_i2c_off_api_payload   - geo_api_pkt_i2c_off_api_code
} W5ComV3GeoApiPackeCmd_e;

typedef enum W5ComV3AckPacketUart {
    // Offset
    ack_pkt_uart_off_frame_start       = 0,
    ack_pkt_uart_off_proto_id,      // = 1
    ack_pkt_uart_off_msg_type,      // = 2
    ack_pkt_uart_off_data_size,     // = 3
    ack_pkt_uart_off_msg_id,        // = 4
    ack_pkt_uart_off_ack_nack,      // = 5
    ack_pkt_uart_off_checksum,      // = 6
    ack_pkt_uart_off_frame_end,     // = 7

    // Length
    ack_pkt_uart_len                   = ack_pkt_uart_off_frame_end + 1
} W5ComV3AckPacketUart_e;

typedef enum W5ComV3AckPacketI2c {
    // Offset
    ack_pkt_i2c_off_proto_id            = 0,
    ack_pkt_i2c_off_msg_type,        // = 1
    ack_pkt_i2c_off_data_size,       // = 2
    ack_pkt_i2c_off_msg_id,          // = 3
    ack_pkt_i2c_off_ack_nack,        // = 4
    ack_pkt_i2c_off_checksum,        // = 5

    // Length
    ack_pkt_i2c_pkt_len                 = ack_pkt_i2c_off_checksum + 1
} W5ComV3AckPacketI2c_e;

typedef enum W5ComV3QueryPacketUart {
    // Offset
    query_pkt_uart_off_frame_start                  = 0,
    query_pkt_uart_off_proto_id,                 // = 1
    query_pkt_uart_off_msg_type,                 // = 2
    query_pkt_uart_off_data_size,                // = 3
    query_pkt_uart_off_msg_id,                   // = 4
    query_pkt_uart_off_chunk_num_lsb,            // = 5
    query_pkt_uart_off_chunk_num_msb,            // = 6
    query_pkt_uart_off_bytes_to_read_byte_0_lsb, // = 7
    query_pkt_uart_off_bytes_to_read_byte_1,     // = 8
    query_pkt_uart_off_bytes_to_read_byte_2,     // = 9
    query_pkt_uart_off_bytes_to_read_byte_3_msb, // = 10
    query_pkt_uart_off_checksum,                 // = 11
    query_pkt_uart_off_frame_end,                // = 12

    // Length
    query_pkt_uart_len                              = query_pkt_uart_off_frame_end + 1
} W5ComV3QueryPacketUart_e;

typedef enum W5ComV3QueryPacketI2c {
    // Offset
    query_pkt_i2c_off_proto_id                     = 0,
    query_pkt_i2c_off_msg_type,                 // = 1
    query_pkt_i2c_off_data_size,                // = 2
    query_pkt_i2c_off_msg_id,                   // = 3
    query_pkt_i2c_off_chunk_num_lsb,            // = 4
    query_pkt_i2c_off_chunk_num_msb,            // = 5
    query_pkt_i2c_off_bytes_to_read_byte_0_lsb, // = 6
    query_pkt_i2c_off_bytes_to_read_byte_1,     // = 7
    query_pkt_i2c_off_bytes_to_read_byte_2,     // = 8
    query_pkt_i2c_off_bytes_to_read_byte_3_msb, // = 9
    query_pkt_i2c_off_checksum,                 // = 10

    // Length
    query_pkt_i2c_len                   = query_pkt_i2c_off_checksum + 1
} W5ComV3QueryPacketI2c_e;

// Only bytes after dataSize are passed back from W5_COM CallQuery() as byte buffer
// Since relative offset are the same for UART and I2C, we use I2C's absolute offset for calculation
typedef enum W5ComV3QueryPacketMsg {
    query_pkt_msg_off_msg_id                   = query_pkt_i2c_off_msg_id        - query_pkt_i2c_off_msg_id,
    query_pkt_msg_off_chunk_num_lsb            = query_pkt_i2c_off_chunk_num_lsb - query_pkt_i2c_off_msg_id,
    query_pkt_msg_off_chunk_num_msb            = query_pkt_i2c_off_chunk_num_msb - query_pkt_i2c_off_msg_id,
    query_pkt_msg_off_bytes_to_read_byte_0_lsb = query_pkt_i2c_off_bytes_to_read_byte_0_lsb - query_pkt_i2c_off_msg_id,
    query_pkt_msg_off_bytes_to_read_byte_1     = query_pkt_i2c_off_bytes_to_read_byte_1 - query_pkt_i2c_off_msg_id,
    query_pkt_msg_off_bytes_to_read_byte_2     = query_pkt_i2c_off_bytes_to_read_byte_2 - query_pkt_i2c_off_msg_id,
    query_pkt_msg_off_bytes_to_read_byte_3_msb = query_pkt_i2c_off_bytes_to_read_byte_3_msb - query_pkt_i2c_off_msg_id
} W5ComV3QueryPacketMsg_e;

typedef enum W5ComV3RspPacketUart {
    // Offset
    rsp_pkt_uart_off_frame_start              = 0,
    rsp_pkt_uart_off_proto_id,             // = 1
    rsp_pkt_uart_off_msg_type,             // = 2
    rsp_pkt_uart_off_data_size_byte_0_lsb, // = 3
    rsp_pkt_uart_off_data_size_byte_1,     // = 4
    rsp_pkt_uart_off_data_size_byte_2,     // = 5
    rsp_pkt_uart_off_data_size_byte_3_msb, // = 6
    rsp_pkt_uart_off_msg_id,               // = 7
    rsp_pkt_uart_off_ack_nack,             // = 8
    rsp_pkt_uart_off_cmd_status,           // = 9
    rsp_pkt_uart_off_chunk_num_lsb,        // = 10
    rsp_pkt_uart_off_chunk_num_msb,        // = 11
    rsp_pkt_uart_off_valid_len_byte_0_lsb, // = 12
    rsp_pkt_uart_off_valid_len_byte_1,     // = 13
    rsp_pkt_uart_off_valid_len_byte_2,     // = 14
    rsp_pkt_uart_off_valid_len_byte_3_msb, // = 15
    rsp_pkt_uart_off_payload,              // = 16
    rsp_pkt_uart_min_off_after_payload  = rsp_pkt_uart_off_payload,
    rsp_pkt_uart_max_off_after_payload  = rsp_pkt_uart_off_data_size_byte_3_msb + 1 + MAX_PROTOCOL_MSG_QUERY_RSP_PAYLOAD_SIZE,
    rsp_pkt_uart_after_payload_off_checksum        = 0,
    rsp_pkt_uart_after_payload_off_frame_end,   // = 1

    // Length
    rsp_pkt_uart_len_no_payload     = rsp_pkt_uart_min_off_after_payload + rsp_pkt_uart_after_payload_off_frame_end + 1,
    rsp_pkt_uart_len_max_payload    = rsp_pkt_uart_max_off_after_payload + rsp_pkt_uart_after_payload_off_frame_end + 1
} W5ComV3RspPacketUart_e;


typedef enum W5ComV3RspPacketI2c {
    // Offset
    rsp_pkt_i2c_off_proto_id                 = 0,
    rsp_pkt_i2c_off_msg_type,             // = 1
    rsp_pkt_i2c_off_data_size_byte_0_lsb, // = 2
    rsp_pkt_i2c_off_data_size_byte_1,     // = 3
    rsp_pkt_i2c_off_data_size_byte_2,     // = 4
    rsp_pkt_i2c_off_data_size_byte_3_msb, // = 5
    rsp_pkt_i2c_off_msg_id,               // = 6
    rsp_pkt_i2c_off_ack_nack,             // = 7
    rsp_pkt_i2c_off_cmd_status,           // = 8
    rsp_pkt_i2c_off_chunk_num_lsb,        // = 9
    rsp_pkt_i2c_off_chunk_num_msb,        // = 10
    rsp_pkt_i2c_off_valid_len_byte_0_lsb, // = 11
    rsp_pkt_i2c_off_valid_len_byte_1,     // = 12
    rsp_pkt_i2c_off_valid_len_byte_2,     // = 13
    rsp_pkt_i2c_off_valid_len_byte_3_msb, // = 14
    rsp_pkt_i2c_off_payload,              // = 15
    rsp_pkt_i2c_min_off_after_payload  = rsp_pkt_i2c_off_payload,
    rsp_pkt_i2c_max_off_after_payload  = rsp_pkt_i2c_off_data_size_byte_3_msb + 1 + MAX_PROTOCOL_MSG_QUERY_RSP_PAYLOAD_SIZE,
    rsp_pkt_i2c_after_payload_off_checksum        = 0,

    // Length
    rsp_pkt_i2c_len_no_payload  = rsp_pkt_i2c_min_off_after_payload + rsp_pkt_i2c_after_payload_off_checksum + 1,
    rsp_pkt_i2c_len_max_payload = rsp_pkt_i2c_max_off_after_payload + rsp_pkt_i2c_after_payload_off_checksum + 1
} W5ComV3RspPacketI2c_e;


// Only bytes after ack/nack are passed back to W5_COM CallQuery() as byte buffer
// Since relative offset are the same for UART and I2C, we use I2C's absolute offset for calculation
typedef enum W5ComV3RspPacketResult {
    rsp_pkt_result_off_cmd_status           = rsp_pkt_i2c_off_cmd_status           - rsp_pkt_i2c_off_cmd_status,
    rsp_pkt_result_off_chunk_num_lsb        = rsp_pkt_i2c_off_chunk_num_lsb        - rsp_pkt_i2c_off_cmd_status,
    rsp_pkt_result_off_chunk_num_msb        = rsp_pkt_i2c_off_chunk_num_msb        - rsp_pkt_i2c_off_cmd_status,
    rsp_pkt_result_off_valid_len_byte_0_lsb = rsp_pkt_i2c_off_valid_len_byte_0_lsb - rsp_pkt_i2c_off_cmd_status,
    rsp_pkt_result_off_valid_len_byte_1     = rsp_pkt_i2c_off_valid_len_byte_1     - rsp_pkt_i2c_off_cmd_status,
    rsp_pkt_result_off_valid_len_byte_2     = rsp_pkt_i2c_off_valid_len_byte_2     - rsp_pkt_i2c_off_cmd_status,
    rsp_pkt_result_off_valid_len_byte_3_msb = rsp_pkt_i2c_off_valid_len_byte_3_msb - rsp_pkt_i2c_off_cmd_status,
    rsp_pkt_result_off_payload              = rsp_pkt_i2c_off_payload              - rsp_pkt_i2c_off_cmd_status
} W5ComV3RspPacketResult_e;

/*******************************************************************************
 Exported Variable Declarations
*******************************************************************************/


/*******************************************************************************
 Inline Function Definitions
*******************************************************************************/
static inline uint8_t calcCheckSum(const uint8_t *payloadPtr,
                                   size_t payloadSize)
{
    uint8_t cs = 0;
    size_t i;
    for (i = 0; i < payloadSize; i++) {
        cs += payloadPtr[i];
    }
    return cs;
}

/*******************************************************************************
 Global Scope Function Prototypes
*******************************************************************************/
W5ComReturn_e W5protoUartAttach(const W5ComConfig_s *configPtr,
                                W5Connection_s *currentConnPtr,
                                W5Protocol_s **protoPtrPtr);

W5ComReturn_e W5protoUartProcessW5Commands(const W5Protocol_s *protoPtr,
                                           W5Command_s *cmdQueuePtr);

W5ComReturn_e W5protoUartReconfigMaxBytes(const W5ComConfig_s *configPtr,
                                          const W5Connection_s *currentConnPtr,
                                          W5Protocol_s *protoPtr,
                                          size_t maxBytesPerWriteRequested,
                                          size_t maxBytesPerReadRequested);

W5ComReturn_e W5protoUartDetach(W5Protocol_s **protoPtrPtr);

W5ComReturn_e W5protoI2cAttach(const W5ComConfig_s *configPtr,
                               W5Connection_s *currentConnPtr,
                               W5Protocol_s **protoPtrPtr);

W5ComReturn_e W5protoI2cReconfigMaxBytes(const W5ComConfig_s *configPtr,
                                         const W5Connection_s *currentConnPtr,
                                         W5Protocol_s *protoPtr,
                                         size_t maxBytesPerWriteRequested,
                                         size_t maxBytesPerReadRequested);

W5ComReturn_e W5protoI2cProcessW5Commands(const W5Protocol_s *protoPtr,
                                          W5Command_s *cmdQueuePtr);

W5ComReturn_e W5protoI2cDetach(W5Protocol_s **protoPtrPtr);


// #ifdef __cplusplus
// }
// #endif // __cplusplus
#endif // __PROTOCOL_H__

/*! @}*/

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
 * \file w5_com.h
 * \brief W5 communication library public header file.
 * \defgroup W5COM W5 Communication
 *
 * W5 communication public API.
 *
 * @{
 */

#ifndef __W5_COM_H__
#define __W5_COM_H__


/*******************************************************************************
 Includes
*******************************************************************************/
#include <stdint.h>
#include "w5_com_platform.h"
#include "api.h"


/*******************************************************************************
 Exported Macro, Enum, Union, Structure, And Type Definitions
*******************************************************************************/
// #ifdef __cplusplus
// extern "C"
// {
// #endif // __cplusplus

#define W5_DEFAULT_I2C_ADDRESS (0x6D)

#pragma pack(push)  /* push current alignment to stack */
#pragma pack(4)     /* set alignment to 4 byte boundary */
typedef struct W5ComHandle W5ComHandle_s;

typedef enum W5DeviceSpecifierType {
    w5_dev_spec_type_none, /*!< No device specifier */
    w5_dev_spec_type_port, /*!< Specify device by port, i.e. COM port number, USB number */
    w5_dev_spec_type_device_id, /*!< Specify device by ID */
    w5_dev_spec_type_index, /*!< Specify device by index */
    w5_dev_spec_type_serial_number, /*!< Specify device by serial number */
    w5_dev_spec_type_description, /*!< Specify device by description sub-string */
    w5_num_device_specifier_types
} W5DeviceSpecifierType_e;

typedef struct W5DeviceSpecifier {
    uint32_t port;
    unsigned long deviceId;
    unsigned long index;
    char *serialNumber;
    char *description;
} W5DeviceSpecifier_s;

typedef enum W5ConnectionType {
    w5_connection_type_unspecified = 0,
    w5_connection_type_unrecognized = 0,
    w5_connection_type_serial,
    w5_connection_type_ftdi4222,
    w5_num_connection_types
} W5ConnectionType_e;

typedef enum W5BaudRate {
    w5_baud_rate_default,

    // UART
    w5_baud_rate_4800,
    w5_baud_rate_9600,
    w5_baud_rate_14400,
    w5_baud_rate_19200,
    w5_baud_rate_38400,
    w5_baud_rate_57600,
    w5_baud_rate_115200,
    w5_baud_rate_230400,
    w5_baud_rate_460800,
    w5_baud_rate_921600,

    // I2C
    w5_baud_rate_100k,
    w5_baud_rate_400k,
    w5_baud_rate_1000k,
    w5_baud_rate_3400k,

    w5_num_baud_rates
} W5BaudRate_e;

typedef enum W5Parity {
    w5_parity_none,
    w5_parity_odd,
    w5_parity_even,
    w5_num_parities
} W5Parity_e;

typedef struct W5ConnectionParams {
    W5BaudRate_e baud;
    W5Parity_e parity;
} W5ConnectionParams_s;

typedef enum W5ProtocolType {
    w5_protocol_type_unspecified = 0,
    w5_protocol_type_unrecognized = 0,
    w5_protocol_type_uart,
    w5_protocol_type_i2c,
    w5_num_protocol_types
} W5ProtocolType_e;

typedef enum W5ProtocolVersion {
    w5_protocol_unspecified = 0,
    w5_protocol_v1_unsupported = 1,
    w5_protocol_v2_unsupported = 2,
    w5_protocol_v3 = 3,
    w5_num_protocol_versions,
    w5_protocol_version_default = w5_protocol_v3
} W5ProtocolVersion_e;

typedef enum W5UartDuplex {
    w5_uart_duplex_unspecified = 0,
    w5_uart_duplex_unrecognized = 0,
    w5_uart_duplex_half,
    w5_uart_duplex_full,
    w5_num_uart_duplexes,
} W5UartDuplex_e;

struct W5UartParams {
    W5UartDuplex_e duplex;
};

typedef enum W5I2cDelayWriteProfile {
    w5_i2c_delay_write_profile_default,
    w5_i2c_delay_write_profile_conservative,
    w5_num_i2c_delay_write_profiles
} W5I2cDelayWriteProfile_e;

struct W5I2cParams {
    uint8_t address;
    W5I2cDelayWriteProfile_e delayWriteProfile;
};

typedef struct W5ProtocolParams {
    struct W5UartParams uart;
    struct W5I2cParams i2c;
} W5ProtocolParams_s;

typedef struct W5ComConfig {
    W5DeviceSpecifierType_e devSpecType;
    W5DeviceSpecifier_s devSpec;
    W5ConnectionType_e connType;
    W5ConnectionParams_s connParams;
    W5ProtocolType_e protoType;
    W5ProtocolParams_s protoParams;
    W5ProtocolVersion_e protoVersion;
} W5ComConfig_s;

typedef enum W5ComVerbosity {
    verbosity_normal = 0,
    verbosity_verbose = 1,
    verbosity_default = verbosity_normal
} W5ComVerbosity_e;

typedef enum W5ComReturn {
    w5_com_ret_bad_crc_warning = 1,
    w5_com_ret_ok = 0,
    w5_com_ret_general_error = -1,
    w5_com_ret_invalid_param = -2,
    w5_com_ret_not_supported = -3,
    w5_com_ret_out_of_memory = -4,
    w5_com_ret_invalid_device = -5,
    w5_com_ret_timed_out = -6,
    w5_com_ret_not_connected = -7,
    w5_com_ret_address_out_of_bounds = -8,
    w5_com_ret_cmd_cs_failed = -9,
    w5_com_ret_device_busy = -10,
    w5_com_ret_rsp_cs_failed = -11,
    w5_com_ret_msg_id_mismatched = -12,
    w5_com_ret_read_size_error = -13
} W5ComReturn_e;
#pragma pack(pop)   /* restore original alignment from stack */

typedef void (*W5CommandResultCallback_f)(W5ComReturn_e cmdResult,
                                          uint32_t commandNumberInQueue,
                                          size_t bytesWritten,
                                          size_t bytesRead,
                                          void *userData);

/*******************************************************************************
 Exported Variable Declarations
*******************************************************************************/


/*******************************************************************************
 Inline Function Definitions
*******************************************************************************/


/*******************************************************************************
 Global Scope Function Prototypes
*******************************************************************************/
/*!
 * \brief Establish connection to W5 device.
 *
 * Establish connection to W5 device based on configuration \p config.
 * Allocates and returns \p handle on success.
 * \param[in] configPtr W5 communication configuration.
 * \param[out] handlePtrPtr Address of a NULL pointer. Used to return a valid
 *                          W5 communication handle.
 * \return Status of function. w5_com_ret_ok on success, errors otherwise.
 */
W5_COM_API W5ComReturn_e W5comConnect(W5ComConfig_s *configPtr,
                                      W5ComHandle_s **handlePtrPtr);
/*!
 * \brief Terminate W5 device connection.
 *
 * Terminates connection to W5 device contained in \p handle.
 * Deallocates \p handle and sets it to NULL on success.
 * \param[inout] handlePtrPtr Address of a pointer to a valid W5 communcation
 *                            handle.
 * \return Void.
 */
W5_COM_API void W5comDisconnect(struct W5ComHandle **handlePtrPtr);

/*!
 * \brief Get the connection type of the communication to the W5 device.
 *
 * With a valid connection to a W5 device, the type of the connection is
 * returned. If there is no connection, a w5_com_ret_not_connected error is
 * returned.
 * \param[in] handlePtr Address of a pointer to a valid W5 communcation
 *                      handle.
 * \param[out] connTypePtr W5 device connection's type.
 * \return Status of function. w5_com_ret_ok on success, errors otherwise.
 */
W5_COM_API W5ComReturn_e W5comGetConnectionType(const W5ComHandle_s
                                                *handlePtr,
                                                W5ConnectionType_e *connTypePtr);

/*!
 * \brief Get the protocol type of the communication to the W5 device.
 *
 * With a valid connection to a W5 device, the type of the protocol is
 * returned. If there is no connection, a w5_com_ret_not_connected error is
 * returned.
 * \param[in] handlePtr Address of a pointer to a valid W5 communcation
 *                      handle.
 * \param[out] protoTypePtr W5 device connection's protocol type.
 * \return Status of function. w5_com_ret_ok on success, errors otherwise.
 */
W5_COM_API W5ComReturn_e W5comGetProtocolType(const W5ComHandle_s
                                              *handlePtr,
                                              W5ProtocolType_e *protoTypePtr);

/*!
 * \brief Get the protocol version of the communication to the W5 device.
 *
 * With a valid connection to a W5 device, the version of the protocol is
 * returned. If there is no connection, a w5_com_ret_not_connected error is
 * returned.
 * \param[in] handlePtr Address of a pointer to a valid W5 communcation
 *                      handle.
 * \param[out] protoVersionPtr W5 device connection's protocol version.
 * \return Status of function. w5_com_ret_ok on success, errors otherwise.
 */
W5_COM_API W5ComReturn_e W5comGetProtocolVersion(const W5ComHandle_s
                                                 *handlePtr,
                                                 W5ProtocolVersion_e *protoVersionPtr);

/*!
 * \brief Get the serial port number of the connected W5 device.
 *
 * With a valid serial connection to a W5 device, the serial port number is
 * returned. If there is no connection, a w5_com_ret_not_connected error is
 * returned.
 * \param[in] handlePtr Address of a pointer to a valid W5 communcation
 *                      handle.
 * \param[out] portNumberPtr W5 device's serial port number.
 * \return Status of function. w5_com_ret_ok on success, errors otherwise.
 */
W5_COM_API W5ComReturn_e W5comGetSerialPortNumber(const W5ComHandle_s
                                                  *handlePtr,
                                                  uint32_t *portNumberPtr);

/*!
 * \brief Get the duplex of the UART protocol in use.
 *
 * With a valid UART connection to a W5 device, the duplex of the UART protocol
 * is returned. If there is no connection, a w5_com_ret_not_connected error is
 * returned. If the connection is not using the UART protocol, a
 * w5_com_ret_not_supported error is returned.
 * \param[in] handlePtr Address of a pointer to a valid W5 communcation
 *                      handle.
 * \param[out] uartDuplexPtr W5 device connection's UART protocol duplex.
 * \return Status of function. w5_com_ret_ok on success, errors otherwise.
 */
W5_COM_API W5ComReturn_e W5comGetUartDuplex(const W5ComHandle_s *handlePtr,
                                            W5UartDuplex_e *uartDuplexPtr);

/*!
* \brief Get the maximum number of bytes on each write.
*
* Returns the maximum number of allowable bytes for each write.
* \param[in] handlePtr Address of a pointer to a valid W5 communication handle.
* \param[out] maxBytesPerWritePtr Maximum number of bytes for each write.
* \return Status of function. w5_com_ret_ok on success, errors otherwise.
*/
W5_COM_API W5ComReturn_e W5comGetMaxBytesPerWrite(const W5ComHandle_s *handlePtr,
    size_t *maxBytesPerWritePtr);

/*!
* \brief Get the maximum number of bytes on each read.
*
* Returns the maximum number of allowable bytes for each read.
* \param[in] handlePtr Address of a pointer to a valid W5 communication handle.
* \param[out] maxBytesPerReadPtr Maximum number of bytes for each read.
* \return Status of function. w5_com_ret_ok on success, errors otherwise.
*/
W5_COM_API W5ComReturn_e W5comGetMaxBytesPerRead(const W5ComHandle_s *handlePtr,
    size_t *maxBytesPerReadPtr);

/*!
 * \brief Read raw bytes from the connected W5 device.
 *
 * Read raw bytes from the connected W5 device. If the connection speed is low,
 * the read may time out. It is the caller's responsibility to call this API to
 * read all expected bytes.
 * This is a low level API that directly accesses the W5 device connection for
 * experts only.
 * \param[in] handlePtr Address of a pointer to a valid W5 communication handle.
 * \param[in] readBufferPtr Pointer to a buffer for storing the data read.
 * \param[in] bytesToRead Number of bytes of data to read into \p readBufferPtr.
 * \param[out] bytesReadPtr Pointer to a variable to return the actual number
 *                          of bytes read.
 * \return Status of function. w5_com_ret_ok on success, errors otherwise.
 */
W5_COM_API W5ComReturn_e W5comReadBytes(const W5ComHandle_s *handlePtr,
                                        uint8_t *readBufferPtr,
                                        size_t bytesToRead,
                                        size_t *bytesReadPtr);

/*!
 * \brief Write raw bytes to the connected W5 device.
 *
 * Write raw bytes to the connected W5 device.
 * This is a low level API that directly accesses the W5 device connection for
 * experts only.
 * \param[in] handlePtr Address of a pointer to a valid W5 communication handle.
 * \param[in] writeBytesPtr Pointer to the buffer containing the bytes to write.
 * \param[in] bytesToWrite Number of bytes of data to write.
 * \param[out] bytesWrittenPtr Pointer to a variable to return the actual number
 *                             of bytes written.
 * \param[in] bytesToRead      For \ref w5_connection_type_ftdi4222, if \p bytesToRead
 *                             is non-zero, this function will not put a STOP
 *                             bit on I2C bus after the writing operation and
 *                             \ref W5comReadBytes must be called afterwards to
 *                             complete the I2C transaction. This function does
 *                             not execute the reading operation. If \p bytesToRead
 *                             is zero, this function put a STOP bit on I2C bus
 *                             after the writing operation.
 *                             For \ref w5_connection_type_ftdi4222, this parameter
 *                             is ignored.
 * \return Status of function. w5_com_ret_ok on success, errors otherwise.
 */
W5_COM_API W5ComReturn_e W5comWriteBytes(const W5ComHandle_s *handlePtr,
                                         const uint8_t *writeBytesPtr,
                                         size_t bytesToWrite,
                                         size_t *bytesWrittenPtr,
                                         size_t bytesToRead);

/*!
 * \brief Call a W5 device specific API.
 *
 * Issue an API call to the connected W5 device by directly specifying the
 * API code. The API code provided will NOT be validated against the APICode_e
 * values in api.h
 * \param[in] handlePtr Address of a pointer to a valid W5 communication handle.
 * \param[in] apiCode W5 device API code.
 * \param[in] bytesToWrite Number of bytes in \p writeData to send to the
 *                         device.
 * \param[in] writeDataPtr Pointer to data to be sent to the device.
 * \param[in] chunkNum 16-bit chunk number. First chunk must start with 0.
 *                     To indicate last chunk, set bit 15 to 1.
 * \param[out] msgIdPtr Message ID assigned to the command by device.
 * \return Status of function. w5_com_ret_ok on success, errors otherwise.
 */
W5_COM_API W5ComReturn_e W5comCallApi(const W5ComHandle_s *handlePtr,
                                      APICode_e apiCode,
                                      size_t bytesToWrite,
                                      const uint8_t *writeDataPtr,
                                      uint16_t chunkNum,
                                      uint8_t *msgIdPtr);

/*!
 * \brief Call a W5 device specific Query.
 *
 * Issue an Query call to the connected W5 device by directly specifying the
 * Message ID.
 * \param[in] handlePtr Address of a pointer to a valid W5 communication handle.
 * \param[in] msgId W5 device message ID.
 * \param[in] bytesToRead Number of bytes to be read from device.
 * \param[out] readDataBufPtr Pointer to data to be read from the device.
 * \param[out] cmdStatusPtr status of the command represented by the message ID.
 * \param[out] dataAvailPtr data size available to be read for the message ID.
 *                          Only returned if bytesToRead > 0.
 * \param[out] chunkNumber 16-bit chunk number. First chunk starts with 0.
 *                         Last chunk has bit 15 set to 1.
 * \return Status of function. w5_com_ret_ok on success, errors otherwise.
 */
W5_COM_API W5ComReturn_e W5comCallQuery(const W5ComHandle_s *handlePtr,
                                        uint8_t msgId,
                                        size_t bytesToRead,
                                        uint8_t *readDataBufPtr,
                                        uint8_t *cmdStatusPtr,
                                        size_t *dataAvailPtr,
                                        uint16_t *chunkNumber);

/*!
* \brief Set the W5 verbosity level.
*
* Sets the W5 verbosity level which determines if raw TX/RX bytes will be printed.
* \param[in] handlePtr Address of a pointer to a valid W5 communication handle.
* \param[in] verbosityLevel Verbosity level.
* \return Status of function. w5_com_ret_ok on success, errors otherwise.
*/
W5_COM_API W5ComReturn_e W5comLibSetVerbosity(W5ComHandle_s *handlePtr,
                                              W5ComVerbosity_e verbosityLevel);

// #ifdef __cplusplus
// }
// #endif // __cplusplus
#endif // __W5_COM_H__

/*! @}*/

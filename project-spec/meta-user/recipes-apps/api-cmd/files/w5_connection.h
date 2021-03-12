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
 * \file w5_connection.h
 * \brief W5COM device connection declarations.
 * \defgroup W5CONN W5COM Device Connection
 * \ingroup W5COM
 *
 * W5COM device connection declarations.
 *
 * @{
 */

#ifndef __W5_CONNECTION_H__
#define __W5_CONNECTION_H__


/*******************************************************************************
 Includes
*******************************************************************************/
#include "w5_com.h"


/*******************************************************************************
 Exported Macro, Enum, Union, Structure, And Type Definitions
*******************************************************************************/
// #ifdef __cplusplus
// extern "C"
// {
// #endif // __cplusplus

typedef struct W5Connection W5Connection_s;

typedef W5ComReturn_e(*W5connValidateConn_f)(const W5ComConfig_s *configPtr,
                                             struct W5Connection *connPtr,
                                             void *customDataPtr);

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
 * \brief Open a connection to a W5 device over COM/TTY access.
 *
 * Open a connection to a W5 device using COM port or TTY with the supplied
 * configuration.
 * Allocates and returns a W5Connection_s structure with a valid connection
 * if the call is successful.
 * \param[in] configPtr Pointer to W5 communication configuration structure.
 * \param[in] validateConn Caller supplied callback function to validate the
 *                         connection. Optional, can be NULL.
 * \param[in] validateConnCustomDataPtr Pointer to custom data to be passed to
 *                                      \p validateConn. Optional, can be NULL.
 * \param[out] connPtrPtr Address of a pointer to a W5Connection_s structure.
 *                        The pointer must be NULL. On success, a pointer to a
 *                        valid W5Connection_s structure will be allocated and
 *                        returned here.
 * \return Status of function. w5_com_ret_ok on success, errors otherwise.
 */
W5_COM_API W5ComReturn_e W5connSerialOpen(W5ComConfig_s *configPtr,
                                          W5connValidateConn_f validateConn,
                                          void *validateConnCustomDataPtr,
                                          W5Connection_s **connPtrPtr);

/*!
 * \brief Detect the communication protocol of the W5 device connected over
 * COM/TTY access.
 *
 * Detect and return the type of protocol of the W5 device connected over
 * COM/TTY access. Only UART protocol is supported.
 * \param[in] connPtr Pointer to a W5Connection_s structure with a valid
 *                    serial connection to a W5 device.
 * \param[out] protoTypePtr Pointer to a variable to return the result of the
 *                          protocol detection.
 * \return Status of function. w5_com_ret_ok on success, errors otherwise.
 */
W5ComReturn_e W5connSerialDetectProtocol(W5Connection_s *connPtr,
                                         W5ProtocolType_e *protoTypePtr);

/*!
 * \brief Configure the serial connection to a W5 device.
 *
 * Configure the serial connection to a W5 device with the supplied connection
 * parameters.
 * \param[inout] connPtr Pointer to a W5Connection_s structure with a valid
 *                       serial connection to a W5 device.
 * \param[in] paramsPtr Pointer to the connection parameters used to configure
 *                      the connection.
 * \return Status of function. w5_com_ret_ok on success, errors otherwise.
 */
W5_COM_API W5ComReturn_e W5connSerialConfigure(W5Connection_s *connPtr,
                                               const W5ConnectionParams_s
                                               *paramsPtr);

/*!
 * \brief Get the port number of the W5 device serial connection.
 *
 * Get the port number of the W5 device is currently connected on. This is only
 * valid for UART protocol connections over virtual serial port.
 *
 * \param[in] connPtr Pointer to a W5Connection_s structure with a valid
 *                    serial connection to a W5 device.
 * \param[out] portPtr Pointer to a variable to store the currently connected
 *                     port's number.
 * \return Status of function. w5_com_ret_ok on success, errors otherwise.
 */
W5_COM_API W5ComReturn_e W5connSerialGetPort(const W5Connection_s *connPtr,
                                             uint32_t *portPtr);

/*!
 * \brief Read data from the serial connection.
 *
 * Read data from the serial connection. If the connection speed is low, the
 * read may time out. It is the caller's responsibility to call this API to read
 * all expected bytes.
 * \param[in] connPtr Pointer to a W5Connection_s structure with a valid
 *                    serial connection to a W5 device.
 * \param[in] readBufferPtr Pointer to a buffer for storing the data read.
 * \param[in] bytesToRead Number of bytes of data to read into \p readBufferPtr.
 * \param[out] bytesReadPtr Pointer to a variable to return the actual number
 *                          of bytes read.
 * \return Status of function. w5_com_ret_ok on success, errors otherwise.
 */
W5_COM_API W5ComReturn_e W5connSerialRead(const W5Connection_s *connPtr,
                                          uint8_t *readBufferPtr,
                                          size_t bytesToRead,
                                          size_t *bytesReadPtr);

/*!
 * \brief Write data to the serial connection.
 *
 * Write data to the serial connection.
 * \param[in] connPtr Pointer to a W5Connection_s structure with a valid
 *                    serial connection to a W5 device.
 * \param[in] writeDataPtr Pointer to the buffer containing the data to write..
 * \param[in] bytesToWrite Number of bytes of data to write.
 * \param[out] bytesWrittenPtr Pointer to a variable to return the actual number
 *                             of bytes written.
 * \param[in] bytesToRead      This parameter is ignored. Please put 0.
 * \return Status of function. w5_com_ret_ok on success, errors otherwise.
 */
W5_COM_API W5ComReturn_e W5connSerialWrite(const W5Connection_s *connPtr,
                                           const uint8_t *writeDataPtr,
                                           size_t bytesToWrite,
                                           size_t *bytesWrittenPtr,
                                           size_t bytesToRead);

/*!
 * \brief Close the serial connection.
 *
 * Close the serial connection. This API closes the connection and frees the
 * W5Connection_s structure.
 * \param[inout] connPtrPtr Address of the pointer to a valid W5Connection_s
 *                          structure with a serial connection to the W5 device.
 *                          On success, the W5Connection_s structure will be
 *                          freed, and NULL will be returned here.
 * \return Status of function. w5_com_ret_ok on success, errors otherwise.
 */
W5_COM_API W5ComReturn_e W5connSerialClose(W5Connection_s **connPtrPtr);

/*!
 * \brief Open a connection to a W5 device over FTDI access.
 *
 * Open a connection to a W5 device using FTDI with the supplied configuration.
 * Allocates and returns a W5Connection_s structure with a valid connection
 * if the call is successful.
 * \param[in] configPtr Pointer to W5 communication configuration structure.
 * \param[in] validateConn Caller supplied callback function to validate the
 *                         connection. Optional, can be NULL.
 * \param[in] validateConnCustomDataPtr Pointer to custom data to be passed to
 *                                      \p validateConn. Optional, can be NULL.
 * \param[out] connPtrPtr Address of a pointer to a W5Connection_s structure.
 *                        The pointer must be NULL. On success, a pointer to a
 *                        valid W5Connection_s structure will be allocated and
 *                        returned here.
 * \return Status of function. w5_com_ret_ok on success, errors otherwise.
 */
W5_COM_API W5ComReturn_e W5connFtdi4222Open(W5ComConfig_s *configPtr,
                                            W5connValidateConn_f validateConn,
                                            void *validateConnCustomDataPtr,
                                            W5Connection_s **connPtrPtr);

/*!
 * \brief Detect the communication protocol of the W5 device connected over
 * FTDI access.
 *
 * Detect and return the type of protocol of the W5 device connected over
 * FTDI access.
 * \param[in] connPtr Pointer to a W5Connection_s structure with a valid
 *                    FTDI connection to a W5 device.
 * \param[out] protoTypePtr Pointer to a variable to return the result of the
 *                          protocol detection.
 * \return Status of function. w5_com_ret_ok on success, errors otherwise.
 */
W5_COM_API W5ComReturn_e W5connFtdi4222DetectProtocol(
    W5Connection_s *connPtr,
    W5ProtocolType_e
    *protoTypePtr);

/*!
 * \brief Configure the FTDI connection to a W5 device for use with UART
 * protocol.
 *
 * Configure the FTDI connection to a W5 device with the supplied connection
 * parameters to be used with the UART protocol.
 * \param[inout] connPtr Pointer to a W5Connection_s structure with a valid
 *                       FTDI connection to a W5 device.
 * \param[in] paramsPtr Pointer to the connection parameters used to configure
 *                      the connection for UART.
 * \return Status of function. w5_com_ret_ok on success, errors otherwise.
 */
W5_COM_API W5ComReturn_e W5connFtdi4222ConfigureUart(W5Connection_s *connPtr,
                                                     const W5ConnectionParams_s
                                                     *paramsPtr);

/*!
 * \brief Configure the FTDI connection to a W5 device for use with I2C
 * protocol.
 *
 * Configure the FTDI connection to a W5 device with the supplied connection
 * parameters to be used with the I2C protocol.
 * \param[inout] connPtr Pointer to a W5Connection_s structure with a valid
 *                       FTDI connection to a W5 device.
 * \param[in] paramsPtr Pointer to the connection parameters used to configure
 *                      the connection for I2C.
 * \return Status of function. w5_com_ret_ok on success, errors otherwise.
 */
W5_COM_API W5ComReturn_e W5connFtdi4222ConfigureI2c(W5Connection_s *connPtr,
                                                    const W5ConnectionParams_s
                                                    *paramsPtr);

/*!
 * \brief Read data from the FTDI connection.
 *
 * Read data from the FTDI connection. If the connection speed is low, the
 * read may time out. It is the caller's responsibility to call this API to read
 * all expected bytes.
 * \param[in] connPtr Pointer to a W5Connection_s structure with a valid
 *                    FTDI connection to a W5 device.
 * \param[in] readBufferPtr Pointer to a buffer for storing the data read.
 * \param[in] bytesToRead Number of bytes of data to read into \p readBufferPtr.
 * \param[out] bytesReadPtr Pointer to a variable to return the actual number
 *                          of bytes read.
 * \return Status of function. w5_com_ret_ok on success, errors otherwise.
 */
W5_COM_API W5ComReturn_e W5connFtdi4222Read(const W5Connection_s *connPtr,
                                            uint8_t *readBufferPtr,
                                            size_t bytesToRead,
                                            size_t *bytesReadPtr);

/*!
 * \brief Write data to the FTDI connection.
 *
 * Write data to the FTDI connection.
 * \param[in] connPtr Pointer to a W5Connection_s structure with a valid
 *                    FTDI connection to a W5 device.
 * \param[in] writeDataPtr Pointer to the buffer containing the data to write..
 * \param[in] bytesToWrite Number of bytes of data to write.
 * \param[out] bytesWrittenPtr Pointer to a variable to return the actual number
 *                             of bytes written.
 * \param[in] bytesToRead      If \p bytesToRead is non-zero, this function will
 *                             not put a STOP bit on I2C bus after the writing
 *                             operation and \ref W5connFtdi4222Read must be called
 *                             afterwards to complete the I2C transaction. This
 *                             function does not execute the reading operation.
 *                             If \p bytesToRead is zero, this function put a
 *                             STOP bit on I2C bus after the writing operation.
 * \return Status of function. w5_com_ret_ok on success, errors otherwise.
 */
W5_COM_API W5ComReturn_e W5connFtdi4222Write(const W5Connection_s *connPtr,
                                             const uint8_t *writeDataPtr,
                                             size_t bytesToWrite,
                                             size_t *bytesWrittenPtr,
                                             size_t bytesToRead);

/*!
 * \brief Close the FTDI connection.
 *
 * Close the FTDI connection. This API closes the connection and frees the
 * W5Connection_s structure.
 * \param[inout] connPtrPtr Address of the pointer to a valid W5Connection_s
 *                          structure with a FTDI connection to the W5 device.
 *                          On success, the W5Connection_s structure will be
 *                          freed, and NULL will be returned here.
 * \return Status of function. w5_com_ret_ok on success, errors otherwise.
 */
W5_COM_API W5ComReturn_e W5connFtdi4222Close(W5Connection_s **connPtrPtr);

// #ifdef __cplusplus
// }
// #endif // __cplusplus
#endif // __W5_CONNECTION_H__

/*! @}*/

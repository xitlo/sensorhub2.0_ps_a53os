/*******************************************************************************

                        Copyright GEO Semiconductor 2015
                               All Rights Reserved.

 THIS WORK CONTAINS TRADE SECRET AND PROPRIETARY INFORMATION WHICH IS THE
 PROPERTY OF GEO SEMICONDUCTOR OR ITS LICENSORS AND IS SUBJECT TO LICENSE TERMS.
*******************************************************************************/

/*!
 * \file connection_serial.c
 * \brief Linux specific serial connection handling.
 * \addtogroup W5CONN
 *
 * Linux specific serial connection handling.
 */


/*******************************************************************************
 Includes
*******************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <linux/serial.h>
#include "w5_connection.h"
#include "connection.h"


/*******************************************************************************
 Exported Variable Definitions
*******************************************************************************/


/*******************************************************************************
 Extern Inline Function Declarations
*******************************************************************************/


/*******************************************************************************
 Internal Macro, Enum, Union, Structure, And Type Definitions
*******************************************************************************/
struct SerialConnection {
    int fd;
    uint32_t port;
};

#define MIN_COMM_PORT_NUMBER (0)
#define MAX_COMM_PORT_NUMBER (99)

#define SERIAL_MAX_BYTES_PER_READ (SIZE_MAX)
#define SERIAL_MAX_BYTES_PER_WRITE (SIZE_MAX)


/*******************************************************************************
 File Scope Variable Declarations And Definitions
*******************************************************************************/
static const speed_t SerialSpeeds[w5_num_baud_rates] = {
    B921600, // w5_baud_rate_default
    B4800, // w5_baud_rate_4800
    B9600, // w5_baud_rate_9600
    B0,  // w5_baud_rate_14400 (irregular baud rate)
    B19200, // w5_baud_rate_19200
    B38400, // w5_baud_rate_38400
    B57600, // w5_baud_rate_57600
    B115200, // w5_baud_rate_115200
    B230400, // w5_baud_rate_230400
    B460800, // w5_baud_rate_460800
    B921600 // w5_baud_rate_921600
};

static const int BaudRates[w5_num_baud_rates] = {
    921600, // w5_baud_rate_default
    4800, // w5_baud_rate_4800
    9600, // w5_baud_rate_9600
    14400,  // w5_baud_rate_14400
    19200, // w5_baud_rate_19200
    38400, // w5_baud_rate_38400
    57600, // w5_baud_rate_57600
    115200, // w5_baud_rate_115200
    230400, // w5_baud_rate_230400
    460800, // w5_baud_rate_460800
    921600 // w5_baud_rate_921600

};


/*******************************************************************************
 File Scope Function Prototypes
*******************************************************************************/
static inline W5ComReturn_e openSerialPort(uint32_t portNumber,
                                            W5Connection_s *connPtr,
                                            const W5ComConfig_s *configPtr,
                                            W5connValidateConn_f validateConn,
                                            void *validateConnCustomDataPtr);


/*******************************************************************************
 File Scope Function Definitions
*******************************************************************************/
static inline W5ComReturn_e openSerialPort(uint32_t portNumber,
                                            W5Connection_s *connPtr,
                                            const W5ComConfig_s *configPtr,
                                            W5connValidateConn_f validateConn,
                                            void *validateConnCustomDataPtr)
{
    int *fdPtr;
    char fileName[14] = {0};

    if (portNumber > MAX_COMM_PORT_NUMBER) {
        return w5_com_ret_invalid_param;
    }

    connPtr->serialConnPtr = (struct SerialConnection *)calloc(1, sizeof(*connPtr->serialConnPtr));
    if (connPtr->serialConnPtr == NULL) {
        return w5_com_ret_out_of_memory;
    }
    fdPtr = &connPtr->serialConnPtr->fd;

    snprintf(fileName, sizeof(fileName) - 1, "/dev/ttyS%d", portNumber);
    *fdPtr = open(fileName, O_RDWR | O_NOCTTY | O_SYNC);
    if (*fdPtr < 0) {
        free(connPtr->serialConnPtr);
        connPtr->serialConnPtr = NULL;
        return w5_com_ret_general_error;
    }

    connPtr->type = w5_connection_type_serial;

    if (validateConn != NULL) {
        W5ComReturn_e ret = validateConn(configPtr, connPtr,
                                          validateConnCustomDataPtr);
        if (ret != w5_com_ret_ok) {
            close(*fdPtr);
            free(connPtr->serialConnPtr);
            connPtr->serialConnPtr = NULL;
            return ret;
        }
    }

    connPtr->serialConnPtr->port = portNumber;

    return w5_com_ret_ok;
}


/*******************************************************************************
 Global Scope Function Definitions
*******************************************************************************/
W5ComReturn_e W5connSerialOpen(W5ComConfig_s *configPtr,
                                W5connValidateConn_f validateConn,
                                void *validateConnCustomDataPtr,
                                W5Connection_s **connPtrPtr)
{
    W5Connection_s *connectionPtr;
    W5ComReturn_e ret;

    if ((configPtr == NULL)
        || (connPtrPtr == NULL)
        || (*connPtrPtr != NULL)) {
        return w5_com_ret_invalid_param;
    }

    connectionPtr = (W5Connection_s *)calloc(1, sizeof(*connectionPtr));
    if (connectionPtr == NULL) {
        return w5_com_ret_out_of_memory;
    }

    if (configPtr->devSpecType == w5_dev_spec_type_port) {
        ret = openSerialPort(configPtr->devSpec.port, connectionPtr, configPtr,
                             validateConn, validateConnCustomDataPtr);
    }
    else if (configPtr->devSpecType == w5_dev_spec_type_none) {
        uint32_t port = MIN_COMM_PORT_NUMBER;
        for (; port <= MAX_COMM_PORT_NUMBER; ++port) {
            ret = openSerialPort(port, connectionPtr, configPtr,
                                 validateConn, validateConnCustomDataPtr);
            if (ret == w5_com_ret_ok) {
                break;
            }
        }
    }
    else {
        // unsupported serial device specifiers
        ret = w5_com_ret_not_supported;
    }

    if (ret != w5_com_ret_ok) {
        free(connectionPtr);
        return ret;
    }

    *connPtrPtr = connectionPtr;

    return ret;
}

W5ComReturn_e W5connSerialDetectProtocol(W5Connection_s *connPtr,
                                          W5ProtocolType_e *protoTypePtr)
{
    if ((connPtr == NULL)
        || (protoTypePtr == NULL)) {
        return w5_com_ret_invalid_param;
    }

    // serial connection cannot detect protocol and only supports UART
    *protoTypePtr = w5_protocol_type_uart;

    return w5_com_ret_ok;
}

W5ComReturn_e W5connSerialConfigure(W5Connection_s *connPtr,
                                     const W5ConnectionParams_s *paramsPtr)
{
    int fd;
    struct termios options;
    int result;
    struct serial_struct ss;
    speed_t speed;

    if ((connPtr == NULL)
        || (connPtr->type != w5_connection_type_serial)
        || (connPtr->serialConnPtr == NULL)
        || (paramsPtr == NULL)
        || (paramsPtr->baud >= w5_num_baud_rates)
        || (paramsPtr->parity >= w5_num_parities)) {
        return w5_com_ret_invalid_param;
    }

    fd = connPtr->serialConnPtr->fd;
    fcntl(fd, F_SETFL, 0); // enable blocking read

    memset(&options, 0, sizeof(options));
    result = tcgetattr(fd, &options);
    if (result != 0) {
        return w5_com_ret_general_error;
    }

    // get the serial structure for configuring the baud rate
    memset(&ss, 0, sizeof(ss));
    result = ioctl(fd, TIOCGSERIAL, &ss);
    if (result < 0) {
        return w5_com_ret_general_error;
    }

    speed = SerialSpeeds[paramsPtr->baud];
    if (speed == B0) {
        // configure non-standard baud rates
        int baudRate = BaudRates[paramsPtr->baud];

        options.c_cflag |= B38400;
        result = tcsetattr(fd, TCSANOW, &options);
        if (result != 0) {
            return w5_com_ret_general_error;
        }

        ss.custom_divisor = (ss.baud_base + (baudRate / 2)) / baudRate;
        ss.flags = (ss.flags & ~ASYNC_SPD_MASK) | ASYNC_SPD_CUST;
        speed = B38400;
    }
    else {
        ss.flags &= ~ASYNC_SPD_MASK;
    }

    // set the serial structure to configure the baud rate
    result = ioctl(fd, TIOCSSERIAL, &ss);
    if (result < 0) {
        return w5_com_ret_general_error;
    }

    cfsetospeed(&options, speed);
    cfsetispeed(&options, speed);

    switch (paramsPtr->parity) {
    case w5_parity_odd:
        options.c_cflag |= PARENB;
        options.c_cflag |= PARODD;
        break;

    case w5_parity_even:
        options.c_cflag |= PARENB;
        options.c_cflag &= ~PARODD;
        break;

    case w5_parity_none:
    default:
        options.c_cflag &= ~PARENB;
        options.c_iflag &= ~(INPCK | ISTRIP); // disable parity checking
        break;
    }

    options.c_cflag &= ~CSTOPB; // 1 stop bit
    options.c_cflag &= ~CSIZE; // mask character size bits
    options.c_cflag |= CS8; // 8 data bits
    options.c_cflag |= (CLOCAL | CREAD); // ignore modem controls, enable reading
    options.c_cflag &= ~CRTSCTS; // disable hardware flow control

    options.c_lflag = 0; // no signaling chars, no echo, no canonical processing
    // disable IGNBRK for mismatched speed tests; otherwise receive break
    // as \000 chars
    options.c_iflag &= ~IGNBRK; // disable break processing
    options.c_iflag &= ~(IXON | IXOFF | IXANY); // shut off xon/xoff ctrl
    options.c_iflag &= ~(INLCR | ICRNL); // shut 0x0d -> 0x0a

    options.c_oflag = 0; // no remapping, no delays

    options.c_cc[VMIN]  = 0;
    options.c_cc[VTIME] = 1; // 0.1 seconds read timeout

    result = tcsetattr(fd, TCSANOW, &options);
    if (result != 0) {
        return w5_com_ret_general_error;
    }

    connPtr->maxBytesPerRead = SERIAL_MAX_BYTES_PER_READ;
    connPtr->maxBytesPerWrite = SERIAL_MAX_BYTES_PER_WRITE;

    return w5_com_ret_ok;
}

W5ComReturn_e W5connSerialGetPort(const W5Connection_s *connPtr,
                                   uint32_t *portPtr)
{
    if ((connPtr == NULL)
        || (connPtr->type != w5_connection_type_serial)
        || (portPtr == NULL)) {
        return w5_com_ret_invalid_param;
    }

    *portPtr = connPtr->serialConnPtr->port;

    return w5_com_ret_ok;
}

W5ComReturn_e W5connSerialRead(const W5Connection_s *connPtr,
                                uint8_t *readBufferPtr, size_t bytesToRead,
                                size_t *bytesReadPtr)
{
    ssize_t bytesRead;

    if ((connPtr == NULL)
        || (connPtr->type != w5_connection_type_serial)
        || (readBufferPtr == NULL)
        || (bytesToRead == 0)
        || (bytesToRead > SERIAL_MAX_BYTES_PER_READ)
        || (bytesReadPtr == NULL)) {
        return w5_com_ret_invalid_param;
    }

    bytesRead = read(connPtr->serialConnPtr->fd, readBufferPtr, bytesToRead);
    if (bytesRead < 0) {
        return w5_com_ret_general_error;
    }

    *bytesReadPtr = bytesRead;

    return w5_com_ret_ok;
}

W5ComReturn_e W5connSerialWrite(const W5Connection_s *connPtr,
                                 const uint8_t *writeDataPtr,
                                 size_t bytesToWrite,
                                 size_t *bytesWrittenPtr,
                                 size_t bytesToRead)
{
    ssize_t nBytes;
    UNUSED(bytesToRead);

    if ((connPtr == NULL)
        || (connPtr->type != w5_connection_type_serial)
        || (writeDataPtr == NULL)
        || (bytesToWrite == 0)
        || (bytesToWrite > SERIAL_MAX_BYTES_PER_WRITE)
        || (bytesWrittenPtr == NULL)) {
        return w5_com_ret_invalid_param;
    }

    *bytesWrittenPtr = 0;
    nBytes = write(connPtr->serialConnPtr->fd, writeDataPtr, bytesToWrite);
    if (0 > nBytes) {
        return w5_com_ret_general_error;
    }

    *bytesWrittenPtr = nBytes;

    return w5_com_ret_ok;
}

W5ComReturn_e W5connSerialClose(W5Connection_s **connPtrPtr)
{
    int result;

    if ((connPtrPtr == NULL)
        || (*connPtrPtr == NULL)
        || ((*connPtrPtr)->serialConnPtr == NULL)) {
        return w5_com_ret_invalid_param;
    }

    result = close((*connPtrPtr)->serialConnPtr->fd);
    if (result < 0) {
    }

    free((*connPtrPtr)->serialConnPtr);
    free(*connPtrPtr);
    *connPtrPtr = NULL;

    return w5_com_ret_ok;
}

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
* \file connection_ftdi4222.c
* \brief FTDI driver connection handling.
* \addtogroup W5CONN
*
* FTDI driver connection handling.
*/

/*******************************************************************************
Includes
*******************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <limits.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include "w5_connection.h"
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
struct FtdiConnection {
    int fd;
    char *dev_name;
    uint8_t address;    // 8-bit address
};

#define I2C_DEV_NAME_DEFAULT ("/dev/i2c-0")
#define FTDI_MAX_BYTES_PER_READ (255)
#define FTDI_MAX_BYTES_PER_WRITE (255)

/*******************************************************************************
File Scope Variable Declarations And Definitions
*******************************************************************************/
static uint8_t tempReadBuf[FTDI_MAX_BYTES_PER_READ];
static uint8_t tempReadLen = 0;

/*******************************************************************************
File Scope Function Prototypes
*******************************************************************************/

/*******************************************************************************
File Scope Function Definitions
*******************************************************************************/

/*******************************************************************************
Global Scope Function Definitions
*******************************************************************************/
W5ComReturn_e W5connFtdi4222Open(W5ComConfig_s *configPtr,
                                  W5connValidateConn_f validateConn,
                                  void *validateConnCustomDataPtr,
                                  W5Connection_s **connPtrPtr)
{
    W5ComReturn_e ret;
    W5Connection_s *connectionPtr;
    struct FtdiConnection *ftdiConnPtr;
    char serialNumber[16] = { 0 };
    int nDeviceIndices = 1;
    int i = 0;

    if ((configPtr == NULL)
        || (connPtrPtr == NULL)
        || (*connPtrPtr != NULL)) {
        return w5_com_ret_invalid_param;
    }

    if (configPtr->protoType >= w5_num_protocol_types) {
        return w5_com_ret_invalid_param;
    }

    connectionPtr = (W5Connection_s *)calloc(1, sizeof(*connectionPtr));
    if (connectionPtr == NULL) {
        return w5_com_ret_out_of_memory;
    }

    ftdiConnPtr = (struct FtdiConnection *)calloc(1, sizeof(*ftdiConnPtr));
    if (ftdiConnPtr == NULL) {
        free(connectionPtr);
        return w5_com_ret_out_of_memory;
    }

    connectionPtr->type = w5_connection_type_ftdi4222;
    connectionPtr->ftdiConnPtr = ftdiConnPtr;

    if (NULL == configPtr->devSpec.description) {
        ftdiConnPtr->dev_name = I2C_DEV_NAME_DEFAULT;
    }
    else {
        ftdiConnPtr->dev_name = configPtr->devSpec.description;
    }

    ftdiConnPtr->fd = open(ftdiConnPtr->dev_name, O_RDWR);

    if (0 == ftdiConnPtr->fd) {
        perror("Error open i2c dev");
        free(ftdiConnPtr);
        free(connectionPtr);
        return w5_com_ret_invalid_device;
    }

    if (validateConn != NULL) {
        ret = validateConn(configPtr, connectionPtr,
                            validateConnCustomDataPtr);
    }

    if (configPtr->protoParams.i2c.address == 0) {
        ftdiConnPtr->address = W5_DEFAULT_I2C_ADDRESS;
    }
    else {
        ftdiConnPtr->address = configPtr->protoParams.i2c.address;
    }

    *connPtrPtr = connectionPtr;

    return ret;
}

W5ComReturn_e W5connFtdi4222DetectProtocol(W5Connection_s *connPtr,
                                            W5ProtocolType_e *protoTypePtr)
{
    if ((connPtr == NULL)
        || (protoTypePtr == NULL)) {
        return w5_com_ret_invalid_param;
    }

    *protoTypePtr = w5_protocol_type_i2c;
    return w5_com_ret_ok;
}

W5ComReturn_e W5connFtdi4222ConfigureI2c(W5Connection_s *connPtr,
                                          const W5ConnectionParams_s *paramsPtr)
{
    if ((connPtr == NULL)
        || (connPtr->type != w5_connection_type_ftdi4222)
        || (paramsPtr == NULL)
        || (paramsPtr->baud >= w5_num_baud_rates)) {
        return w5_com_ret_invalid_param;
    }

    ioctl(connPtr->ftdiConnPtr->fd, I2C_TIMEOUT, 2);
    ioctl(connPtr->ftdiConnPtr->fd, I2C_RETRIES, 1);
    if (ioctl(connPtr->ftdiConnPtr->fd, I2C_SLAVE, connPtr->ftdiConnPtr->address) < 0) {
        return w5_com_ret_invalid_param;
    }

    connPtr->maxBytesPerRead = FTDI_MAX_BYTES_PER_READ;
    connPtr->maxBytesPerWrite = FTDI_MAX_BYTES_PER_WRITE;

    return w5_com_ret_ok;
}

W5ComReturn_e W5connFtdi4222Read(const W5Connection_s *connPtr,
                                  uint8_t *readBufferPtr,
                                  size_t bytesToRead,
                                  size_t *bytesReadPtr)
{
    int ret;
    struct i2c_rdwr_ioctl_data msgst;
	struct i2c_msg msg[2];

    if ((connPtr == NULL)
        || (connPtr->type != w5_connection_type_ftdi4222)
        || (readBufferPtr == NULL)
        || (bytesToRead == 0)
        || (bytesToRead > FTDI_MAX_BYTES_PER_READ)
        || (bytesReadPtr == NULL)) {
        return w5_com_ret_invalid_param;
    }

    *bytesReadPtr = 0;

    if (0 < tempReadLen) {
        printf("tempReadLen/bytesToRead: %d/%d\n", tempReadLen, bytesToRead);
        if (tempReadLen != bytesToRead) {
            perror("Error reading data");
            return w5_com_ret_general_error;
        }

        memcpy((uint8_t *)bytesReadPtr, tempReadBuf, bytesToRead);
        *bytesReadPtr = (size_t)bytesToRead;
        tempReadLen = 0;
    }
    else {
        ret = read(connPtr->ftdiConnPtr->fd, readBufferPtr, bytesToRead);
        if (0 > ret) {
            perror("Error reading data");
            return w5_com_ret_general_error;
        }
        *bytesReadPtr = (size_t)ret;
    }


    return w5_com_ret_ok;
}

W5ComReturn_e W5connFtdi4222Write(const W5Connection_s *connPtr,
                                   const uint8_t *writeDataPtr,
                                   size_t bytesToWrite,
                                   size_t *bytesWrittenPtr,
                                   size_t bytesToRead)
{
    int ret;
    struct i2c_rdwr_ioctl_data msgst;
	struct i2c_msg msg[2];

    if ((connPtr == NULL)
        || (connPtr->type != w5_connection_type_ftdi4222)
        || (writeDataPtr == NULL)
        || (bytesToWrite == 0)
        || (bytesToWrite > FTDI_MAX_BYTES_PER_WRITE)
        || (bytesWrittenPtr == NULL)) {
        return w5_com_ret_invalid_param;
    }

    memset(tempReadBuf, 0, sizeof(tempReadBuf));
    tempReadLen = 0;
    *bytesWrittenPtr = 0;

    msgst.msgs = msg;

    msg[0].addr = connPtr->ftdiConnPtr->address;
    msg[0].flags = 0;
    msg[0].len = bytesToWrite;
    msg[0].buf = (uint8_t *)bytesWrittenPtr;

    if ( 0 < bytesToRead) {
        msg[1].addr = connPtr->ftdiConnPtr->address;
        msg[1].flags = I2C_M_RD;
        msg[1].len = bytesToRead;
        msg[1].buf = tempReadBuf;

        msgst.nmsgs = 2;
    }
    else {
        msgst.nmsgs = 1;
    }

#if 0
    ret = write(connPtr->ftdiConnPtr->fd, writeDataPtr, bytesToWrite);
    if ((0 > ret) || (ret != bytesToWrite)) {
        perror("Error writing data");
        return w5_com_ret_general_error;
    }
    *bytesWrittenPtr = (size_t)ret;
#endif

    ret = ioctl(connPtr->ftdiConnPtr->fd, I2C_RDWR, &msgst);
    if ((0 > ret) || (ret != msgst.nmsgs)) {
        perror("Error writing data");
        return w5_com_ret_general_error;
    }
    *bytesWrittenPtr = (size_t)bytesToWrite;

    if ( 0 < bytesToRead) {
        tempReadLen = bytesToRead;
    }

    return w5_com_ret_ok;
}

W5ComReturn_e W5connFtdi4222Close(W5Connection_s **connPtrPtr)
{
    if ((connPtrPtr == NULL)
        || (*connPtrPtr == NULL)
        || ((*connPtrPtr)->ftdiConnPtr == NULL)) {
        return w5_com_ret_invalid_param;
    }

    close((*connPtrPtr)->ftdiConnPtr->fd);

    free((*connPtrPtr)->ftdiConnPtr);
    free(*connPtrPtr);
    *connPtrPtr = NULL;

    return w5_com_ret_ok;
}

/*******************************************************************************
*
* The content of this file or document is CONFIDENTIAL and PROPRIETARY
* to GEO Semiconductor. It is subject to the terms of a License Agreement
* between Licensee and GEO Semiconductor, restricting among other things,
* the use, reproduction, distribution and transfer. Each of the embodiments,
* including this information and any derivative work shall retain this
* copyright notice.
*
* Copyright 2013-2019 GEO Semiconductor, Inc.
* All rights reserved.
*
*******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include "w5_com.h"
#include "argparse.h"
#include "tools_rev.h"
#include <unistd.h>
#include <errno.h>

#define MIN_COMM_PORT_NUMBER (0)
#define MAX_COMM_PORT_NUMBER (99)

#define MAX_HEX_STRING_LEN (MAX_PROTOCOL_MSG_SIZE * 2)
#define LAST_CHUNK_MASK (0x8000)
#define MAX_QUERY_RETRY (10)

// Escape sequences for non-hex values in payload argument
#define INT_ESCAPE ("\\d")
#define SHORT_ESCAPE ("\\s")
#define FLOAT_ESCAPE ("\\f")
#define END_ESCAPE ("\\")

#define ESCAPED_STRING_BUFFER_SIZE (200) // To support lengthiest possible float string

#define CMD_STATUS_DESC(cmdStatus)  (\
                                     ((cmdStatus == CMD_STATUS_UNAVAILABLE) ? "Unavailable" : \
                                      ((cmdStatus == CMD_STATUS_PENDING) ? "Pending" : \
                                       ((cmdStatus == CMD_STATUS_DONE) ? "Done" : \
                                        ((cmdStatus == CMD_STATUS_FAIL) ? "Fail" : "Unknown") \
                                       ) \
                                      ) \
                                     ) \
                                    )

static const char *const usage[] = {
    "api_cmd --version\n \
      api_cmd [-h|--help]\n \
      api_cmd [<-I [-f#] [-2] [-a#]>|<-U# [-b#] [-p#]>]  [-v#]  [-t]  API_CODE  QUERY_SPECIFIER  [DATA_HEX_STRING]  [-i INPUT_BINARY_FILE]  [-o OUTPUT_BINARY_FILE]\n \
      api_cmd [<-I [-f#] [-2] [-a#]>|<-U# [-b#] [-p#]>]  [-v#]  [-t]  -s SCRIPT_FILE  [-o OUTPUT_BINARY_FILE]\n \
      Examples:\n \
          api_cmd -I  0x12 noquery 0100000000000000\n \
          api_cmd -I  0x10 4\n \
          api_cmd -I  0x23 0       1B000200000100000000ff\n \
          api_cmd -I  0x85 max     03  -o fw.bin\n \
          api_cmd -I  0x01 max\n \
          api_cmd -U6 0x90 0       0000150001003502e1000000  -i syscfg.jbf\n \
          api_cmd -I  -s {SCRIPT_FILE}\n\n \
      API_CODE:\n \
      0x0 to 0xfe  - (only valid if the API code is defined) \n\n \
      QUERY_SPECIFIER (Multiple queries may be sent to retrieve the complete response of \n \
                       the API command):\n \
      noquery      - No query will be sent\n \
      0            - Query will be sent. Only command status will be returned\n \
      0x1 to 0xf9  - Query will be sent. Command status and the specified number of bytes   \n \
                     will be returned per query response.\n \
      max          - Query will be repeatedly sent until GW5 indicates last data. Command  \n \
                     status and some bytes will be returned after each query.\n\n \
      DATA_HEX_STRING (Input data sent in GEO Command):\n \
      Data bytes in hexadecimal ASCII representation from left to right without space between consecutive bytes.\n \
      For example:\n \
      000102030405060708090A0B0C0D\n \
      api_cmd will send out 0x00 first, then 0x01, then 0x02, ... and so on from left to right\n\n \
      Float and decimal values can be inserted into hex string using escape sequences\n \
      \\f   - Float\n \
      \\d   - 32-bit decimal\n \
      \\s   - 16-bit decimal\n \
      \\    - End escape\n \
      For example:\n \
      0011\\f12.3\\2233\\d255\\4455\\s-1\\66\n \
      will be converted to\n \
      00 11 |cd cc 44 41| 22 33 |ff 00 00 00| 44 55 |ff ff| 66\n \
      DATA_HEX_STRING can be specified either on command line or in script file.\n \
      If it is specified on command line, the data bytes are sent before binary bytes from INPUT_BINARY_FILE, if specified.\n\n\
Options below can only be used on command line. They are invalid inside script file.", NULL,
};

// Accepts string with starting escape sequence already stripped away
static int extractEscapedString(const char *inputString, char *subString, const uint32_t subStringBufSize) {
    char *end;

    end = strstr(inputString, END_ESCAPE);
    if (end) {
        (void)subStringBufSize;
        strncpy(subString, inputString, end - inputString);
        subString[end - inputString] = '\0';
    }
    else {
        printf("Error: Escape sequence end '%s' missing\n", END_ESCAPE);
        return -1;
    }

    if (subString[0] == '\0') {
        printf("Error: Empty escaped string\n");
        return -1;
    }

    return 0;
}

static int convertStringToByteArray(const char *inputString, uint8_t *byteArray, size_t *nBytes)
{
    if (inputString[0] == '\0') {
        printf("Error: Input string is empty\n");
        return -1;
    }

    size_t i = 0;
    char subString[ESCAPED_STRING_BUFFER_SIZE] = {0};
    char *endPtr = NULL;

    while (*inputString) {

        // 32-bit decimal
        if (inputString == strstr(inputString, INT_ESCAPE)) {
            int64_t longVal;

            if (extractEscapedString(inputString + strlen(INT_ESCAPE), subString, sizeof(subString)) == -1) {
                return -1;
            }

            errno = 0;
            longVal = strtoll(subString, &endPtr, 10);

            if (endPtr == subString) {
                printf("Error: \"%s\" is not a number\n", subString);
                return -1;
            }
            else if (endPtr < subString + strlen(subString)) {
                printf("Error: \"%s\" is not a number (invalid trailing characters)\n", subString);
                return -1;
            }
            else if (errno == ERANGE || longVal > 4294967295 || longVal < -2147483648) {
                printf("Error: \"%s\" is out of range\n", subString);
                return -1;
            }

            byteArray[i] = longVal & 0x000000ff;
            byteArray[i + 1] = (longVal & 0x0000ff00) >> 8;
            byteArray[i + 2] = (longVal & 0x00ff0000) >> 16;
            byteArray[i + 3] = (longVal & 0xff000000) >> 24;

            i += sizeof(uint32_t);
            inputString += strlen(INT_ESCAPE) + strlen(subString) + strlen(END_ESCAPE);
        }

        // 16-bit decimal
        else if (inputString == strstr(inputString, SHORT_ESCAPE)) {
            long int shortVal;

            if (extractEscapedString(inputString + strlen(SHORT_ESCAPE), subString, sizeof(subString)) == -1) {
                return -1;
            }

            errno = 0;
            shortVal = strtol(subString, &endPtr, 10);

            if (endPtr == subString) {
                printf("Error: \"%s\" is not a number\n", subString);
                return -1;
            }
            else if (endPtr < subString + strlen(subString)) {
                printf("Error: \"%s\" is not a number (invalid trailing characters)\n", subString);
                return -1;
            }
            else if (errno == ERANGE || shortVal > 65535 || shortVal < -32768) {
                printf("Error: \"%s\" is out of range\n", subString);
                return -1;
            }

            byteArray[i] = shortVal & 0x00ff;
            byteArray[i + 1] = (shortVal & 0xff00) >> 8;

            i += sizeof(uint16_t);
            inputString += strlen(SHORT_ESCAPE) + strlen(subString) + strlen(END_ESCAPE);
        }

        // Float
        else if (inputString == strstr(inputString, FLOAT_ESCAPE)) {
            float floatVal;

            if (extractEscapedString(inputString + strlen(FLOAT_ESCAPE), subString, sizeof(subString)) == -1) {
                return -1;
            }

            floatVal = strtof(subString, &endPtr);

            if (*endPtr != '\0') {
                printf("Error: Invalid float string\n");
                return -1;
            }

            // TODO: endianness of other host platforms?
            memcpy(&byteArray[i], &floatVal, sizeof(float));

            i += sizeof(float);
            inputString += strlen(FLOAT_ESCAPE) + strlen(subString) + strlen(END_ESCAPE);
        }

        // 8-bit hex
        else {
            uint8_t hexVal;

            // Check for any odd length hex strings
            if (inputString[1] == '\\' || inputString[1] == '\0') {
                printf("Error: Odd length hex string, please specify both nibbles of each byte\n");
                return -1;
            }

            memcpy(subString, inputString, 2);
            subString[2] = '\0';

            hexVal = strtol(subString, &endPtr, 16);

            if (*endPtr != '\0') {
                printf("Error: Invalid character(s) in hex string\n");
                return -1;
            }

            byteArray[i] = hexVal;

            i += sizeof(uint8_t);
            inputString += strlen(subString);
        }
    }

    *nBytes = i;
    return 0;
}

static int openFile(FILE **pFPtr, const char *fileName, const char *mode)
{
    *pFPtr = fopen(fileName, mode);
    if (*pFPtr == NULL) {
        printf("Cannot open file %s\n", fileName);
        return 1; //failed to open file
    }
    return 0; //success
}


static int parseReadSize(char *str, char **pStr, size_t *pReadSize, size_t maxReadSize, int *pNoQuery, int *pNoSizeGiven)
{
    if (strncmp(str, "noquery", 7) == 0) {
        *pNoQuery = 1;
        *pNoSizeGiven = 0;
        *pReadSize = 0;
        if (pStr != NULL) {
            *pStr = str + 7;
        }
        return 0;
    }
    else if (strncmp(str, "max", 3) == 0) {
        *pNoQuery = 0;
        *pNoSizeGiven = 1;
        *pReadSize = 0;
        if (pStr != NULL) {
            *pStr = str + 3;
        }
        return 0;
    }
    else {
        char *endptr;
        size_t val = (size_t) strtol(str, &endptr, 0);

        if (pStr != NULL) {
            *pStr = endptr;
        }
        // Not a valid number
        if ((errno == ERANGE)
         || (endptr == str)
         || (val > maxReadSize)) {
            printf("Query specifier must be one of the followings: \"noquery\", \"max\", 0, 1 -  %u\n", (unsigned int)maxReadSize);
            return 1;
        }
        else {
            *pNoQuery = 0;
            *pNoSizeGiven = 0;
            *pReadSize = val;
            return 0;
        }
    }
}


static int readApiCmdFromFile(FILE *pFile, uint8_t *pApiCode, size_t *pReadSize,
                              size_t maxReadSize, int *pNoQuery, int *pNoSizeGiven, char *pCmdBuf)
{
    static int scriptLineNum = 0;
    char str[MAX_HEX_STRING_LEN * 2] = { 0 };
    char *pStr;

    if (!pFile || !pApiCode || !pReadSize || !pCmdBuf) {
        return 0;
    }

    //Script file format: <Api Code> <Read Size> [Cmd Hex String]
    while (fgets(str, sizeof(str) - 1, pFile) != NULL) { //reserve last char as '\0'
        scriptLineNum++;
        if (str[strlen(str) - 1] == '\n') {
            str[strlen(str) - 1] = '\0';
        }

        if (str[0] == '#') {
            if (strlen(&str[1]) > (MAX_HEX_STRING_LEN - 1)) {
                printf("Error: Command too long at line # %d\n", scriptLineNum);
                break;
            }
            continue;
        }
        else if (str[0] == '\0') {
            continue;
        }

        *pApiCode = (uint8_t)strtol(str, &pStr, 0);

        for (; isspace(*pStr); pStr++); //trim leading white space
        if (parseReadSize(pStr, &pStr, pReadSize, maxReadSize, pNoQuery, pNoSizeGiven) != 0) {
            break;
        }

        pCmdBuf[0] = '\0';
        for (; isspace(*pStr); pStr++); //trim leading white space
        if (strlen(pStr) > (MAX_HEX_STRING_LEN - 1)) {
            printf("Error: Command too long at line # %d\n", scriptLineNum);
            break;
        }
        strcpy(pCmdBuf, pStr);
        return 1;
    }

    return 0;
}

static size_t getWriteDataFromBin(FILE *pFile, size_t maxSize, uint8_t *writeBuf, int *isLast)
{
    static size_t fileSize = 0;
    static size_t totalRead = 0;
    size_t readSize;

    if (!fileSize) {
        fseek(pFile, 0, SEEK_END);
        fileSize = ftell(pFile);
        rewind(pFile);
    }

    *isLast = 0;
    readSize = fread(writeBuf, 1, maxSize, pFile);
    totalRead += readSize;

    if (totalRead == fileSize) {
        *isLast = 1;
    }

    return readSize;
}

static size_t putReadDataToBin(FILE *pFile, size_t size, uint8_t *readBuf)
{
    return (size_t) fwrite(readBuf, 1, size, pFile);
}

static W5ComReturn_e establishConnection(W5ComConfig_s *pW5ComConfig, W5ComHandle_s **pW5ComHandle)
{
    W5ComReturn_e ret = w5_com_ret_ok;
    if (pW5ComConfig->protoType == w5_protocol_type_unspecified) {
        //Detect connection
        printf("Detecting connection from port %d to %d\n", MIN_COMM_PORT_NUMBER, MAX_COMM_PORT_NUMBER);
        int portNum;
        for (portNum = MIN_COMM_PORT_NUMBER; portNum < MAX_COMM_PORT_NUMBER; portNum++) {
            pW5ComConfig->protoType = w5_protocol_type_uart;
            pW5ComConfig->connType = w5_connection_type_serial;
            pW5ComConfig->devSpecType = w5_dev_spec_type_port;
            pW5ComConfig->devSpec.port = portNum;

            ret = W5comConnect(pW5ComConfig, pW5ComHandle);
            if (ret == w5_com_ret_ok) {
                uint8_t msgID;
                ret = W5comCallApi(*pW5ComHandle, api_get_gw5_state, 0, NULL, 0x8000, &msgID);
                if (ret == w5_com_ret_ok) {
                    printf("Connected to W5 by UART-COM%d\n", portNum);
                    break;
                }
                else {
                    W5comDisconnect(pW5ComHandle);
                }
            }
        }

        if (ret != w5_com_ret_ok) {
            pW5ComConfig->protoType = w5_protocol_type_i2c;
            pW5ComConfig->connType = w5_connection_type_ftdi4222;
            pW5ComConfig->protoParams.i2c.delayWriteProfile =
                w5_i2c_delay_write_profile_default;

            ret = W5comConnect(pW5ComConfig, pW5ComHandle);
            if (ret == w5_com_ret_ok) {
                uint8_t msgID;
                ret = W5comCallApi(*pW5ComHandle, api_get_gw5_state, 0, NULL, 0x8000, &msgID);
                if (ret == w5_com_ret_ok) {
                    printf("Connected to W5 by I2C-FTDI4222\n.");
                }
                else {
                    W5comDisconnect(pW5ComHandle);
                }
            }
        }
    }
    else {
        ret = W5comConnect(pW5ComConfig, pW5ComHandle);
    }

    return ret;
}


int main(int argc, char *argv[])
{
    W5ComReturn_e ret = w5_com_ret_ok;
    W5ComConfig_s w5ComConfig;
    W5ComHandle_s *w5ComHandle = NULL;
    struct argparse argparse;

    //**************************************************
    // Protocol and other common options
    //**************************************************
    int useI2c = 0;
    int ftIndex = -1;
    int useBasicUart = -1;
    int useI2cProfile2 = 0;
    int uartBaudRate = 0;
    int uartParity = 0;
    int toolsRevision = 0;
    int displayTime = 0;
    int verbosityLevel = 0;
    int i2cAddr = 0;
    int notShowData = 0;

    //***************************************************
    // Variables related to the phase of W5comCallApi()
    //***************************************************

    // W5Com library / hardware capability
    size_t maxWriteSize = 0;                            // Output from W5comGetMaxBytesPerWrite()
                                                        // getWriteDataFromBin() uses maxWriteSize to upper-bound writeSize
                                                        // for the simplicity of this sample code, convertStringToByteArray does not use maxWriteSize to upper-bound writeSize

    // Script mode (mutually exclusive to command line mode) variables
    char *scriptFileName = NULL;                        // Input from user to set pScriptFile
    FILE *pScriptFile = NULL;                           // Input to readApiCmdFromFile()
    char cmdBuf[MAX_HEX_STRING_LEN] = {0};              // Output from readApiCmdFromFile() after copying user input (script file) into write chunk data in ASCII
                                                        // Input to convertStringToByteArray() to convert write chunk data from ASCII to binary

    // Command line mode (mutually exclusive to script mode) variables
    char *pCmdStr = NULL;                               // Input to convertStringToByteArray() to convert write chunk data from (argv) ASCII to binary
                                                        // For input binary file (additional multiple write chunks) support:
    char *inputBinFileName = NULL;                      // Input from user to set pCmdInputBinFile
    FILE *pCmdInputBinFile = NULL;                      // Input to getWriteDataFromBin() for parsing pCmdInputBinFile into write chunk data in binary
    int isInputBinFile = 0;                             // Boolean

    // Parameters for controlling the iteration to the phase of W5comCallApi():
    int writeChunkNum = LAST_CHUNK_MASK;                // Default to one chunk

    // Parameters for W5comCallApi(), not all are listed
    uint8_t apiCode = 0;                                // [IN]  Output from parseReadSize() or readApiCmdFromFile() after parsing user input
    size_t writeSize = 0;                               // [IN}  Output from getWriteDataFromBin() or convertStringToByteArray() after parsing user input,
                                                        //       specifying the size of one chunk of API Payload
    uint8_t writeBuf[MAX_PROTOCOL_MSG_SIZE] = { 0 };    // [IN]  Output from getWriteDataFromBin() or convertStringToByteArray() from user input,
                                                        //       storing one chunk of API Payload
    uint8_t msgID = 0;                                  // [OUT]

    size_t totalWriteSize = 0;                          // For informational purpose

    //***************************************************
    // Variables related to the phase of W5comCallQuery()
    //***************************************************

    // W5Com library / hardware capability
    size_t maxReadSize = 0;                             // Output from W5comGetMaxBytesPerRead()
                                                        // parseReadSize() and readApiCmdFromFile() use maxReadSize to upper-bound sizeToRead

    // Command line mode (mutually exclusive to script mode) variables
                                                        // For output binary file (multiple read chunks) support:
    char *outputBinFileName = NULL;                     // Input from user to set pCmdOutputBinFile
    FILE *pCmdOutputBinFile = NULL;                     // Input to putReadDataToBin() for storing read chunk data to pCmdOutputBinFile
    int isOutputBinFile = 0;                            // Boolean

    // Parameters for controlling the iteration to the phase of W5comCallQuery():
    size_t sizeToRead = 0;                              // Output from parseReadSize() or readApiCmdFromFile(), after parsing user input
    int noQuery = 0;                                    // Output from parseReadSize() or readApiCmdFromFile(), after parsing user input
    int noSizeGiven = 0;                                // Output from parseReadSize() or readApiCmdFromFile(), after parsing user input

    // Parameters for W5comCallQuery(), not all are listed
    uint8_t *readBuf = NULL;                            // [OUT]
    size_t readDataAvailable = 0;                       // [OUT]
    uint8_t cmdStatus = 0;                              // [OUT]

    uint16_t readChunkNum = -1;                         // Sets input to W5comCallQuery()

    size_t sizeReceived;                                // For informational purpose

    //***************************************************
    // Other variables
    //***************************************************
    size_t i;
    clock_t clockStart;                                 // For informational purpose
    clock_t clockDiff;                                  // For informational purpose
    clock_t clockTotal = 0;                             // For informational purpose

    struct argparse_option options[] = {
        //protocol and other common options
        OPT_BOOLEAN('I', "i2c", &useI2c,
                    "Use I2C (FTDI4222) to communicate with the board.",
                    NULL, 0, 0),
        OPT_INTEGER('a', "i2c_address", &i2cAddr,
                    "8-bit I2C address of W5 on the board.",
                    NULL, 0, 0),
        OPT_INTEGER('f', "ft_index", &ftIndex,
                    "Select which FTDI device index to use.",
                    NULL, 0, 0),
        OPT_INTEGER('U', "basic_uart", &useBasicUart,
                    "Use non-FTDI UART (COMn) to communicate with the board.",
                    NULL, 0, 0),
        OPT_BOOLEAN('2', "profile2", &useI2cProfile2,
                    "Use I2C second profile (only effective with -i).",
                    NULL, 0, 0),
        OPT_INTEGER('b', "uart_baudrate", &uartBaudRate,
                    "In UART modes, baud rate for communication (921600 if not specified).",
                    NULL, 0, 0),
        OPT_INTEGER('p', "uart_parity", &uartParity,
                    "In UART modes, parity for communication, 0 = no parity(default), 1 = odd, 2 = even).",
                    NULL, 0, 0),
        OPT_BOOLEAN(0, "version", &toolsRevision,
                    "Get tool revision number.\n",
                    NULL, 0, 0),
        //app-specific options
        OPT_INTEGER('c', "chunk_num", &writeChunkNum,
                    "Sequential number 0-32767 to indicate multiple data chunks, each enclosed in a command message, to be sent for an API. Set b15 to 1 (0x8000) to indicate last chunk.",
                    NULL, 0, 0),
        OPT_STRING('i', "input_binary_file", &inputBinFileName,
                   "Binary bytes from INPUT_BINARY_FILE is sent after data bytes (converted from ASCII) in DATA_HEX_STRING, if specified. All data are sent in chunk(s) in GEO command message(s)",
                   NULL, 0, 0),
        OPT_STRING('o', "output_binary_file", &outputBinFileName,
                   "Data received in chunk(s) is written as binary bytes to OUTPUT_BINARY_FILE",
                   NULL, 0, 0),
        OPT_STRING('s', "script_file", &scriptFileName,
                   "Script file with each line contains API_CODE  QUERY_SPECIFIER and optionally DATA_HEX_STRING\n",
                   NULL, 0, 0),
        OPT_BOOLEAN('t', "time_display", &displayTime,
                    "Display the time spent in W5Com calls\n",
                    NULL, 0, 0),
        OPT_INTEGER('v', "verbosity_level", &verbosityLevel,
                    "Set verbosity level: 0 - normal (default); 1 - verbose\n",
                    NULL, 0, 0),
        OPT_INTEGER('n', "not show data", &notShowData,
                    "Set switch: 0 - show; 1 - not show\n",
                    NULL, 0, 0),

        OPT_HELP(),
        OPT_END()
    };
    argparse_init(&argparse, options, usage, 0);

    //Parse options on command line
    argc = argparse_parse(&argparse, argc, argv);

    if (toolsRevision) {
        if (TOOLS_REVISION == -1) {
            printf("Version: unknown Built on %s %s\n", __DATE__, __TIME__);
        }
        else {
            printf("Version: %llX (%s) Built on %s %s\n", TOOLS_REVISION, ((IS_MODIFIED_TOOLS_REVISION)? "Modified" : "Clean"), __DATE__, __TIME__);
        }
        return 0;
    }

    // If API_CODE and QUERY_SPECIFIER are not both given in non-script-file mode, print help message
    if ((scriptFileName == NULL) && (argc < 2)) {
        argparse_usage(&argparse);
        return 1;
    }

    memset(&w5ComConfig, 0, sizeof(W5ComConfig_s));

    if (useI2c) {
        w5ComConfig.protoType = w5_protocol_type_i2c;
        w5ComConfig.connType = w5_connection_type_ftdi4222;
        if (ftIndex >= 0) {
            w5ComConfig.devSpecType = w5_dev_spec_type_index;
            w5ComConfig.devSpec.index = ftIndex;
        }
        if (useI2cProfile2) {
            w5ComConfig.protoParams.i2c.delayWriteProfile =
                w5_i2c_delay_write_profile_conservative;
        }
        else {
            w5ComConfig.protoParams.i2c.delayWriteProfile =
                w5_i2c_delay_write_profile_default;
        }
        if (i2cAddr) {
            w5ComConfig.protoParams.i2c.address = (uint8_t) i2cAddr;
        }
        w5ComConfig.devSpec.description = NULL;
    }
    else if (useBasicUart != -1) {
        w5ComConfig.protoType = w5_protocol_type_uart;
        w5ComConfig.connType = w5_connection_type_serial;
        w5ComConfig.devSpecType = w5_dev_spec_type_port;
        w5ComConfig.devSpec.port = useBasicUart; // UART port/USB number
    }

    switch (uartBaudRate) {
    case 0:
        w5ComConfig.connParams.baud = w5_baud_rate_default;
        break;
    case 4800:
        w5ComConfig.connParams.baud = w5_baud_rate_4800;
        break;
    case 9600:
        w5ComConfig.connParams.baud = w5_baud_rate_9600;
        break;
    case 14400:
        w5ComConfig.connParams.baud = w5_baud_rate_14400;
        break;
    case 19200:
        w5ComConfig.connParams.baud = w5_baud_rate_19200;
        break;
    case 38400:
        w5ComConfig.connParams.baud = w5_baud_rate_38400;
        break;
    case 57600:
        w5ComConfig.connParams.baud = w5_baud_rate_57600;
        break;
    case 115200:
        w5ComConfig.connParams.baud = w5_baud_rate_115200;
        break;
    case 230400:
        w5ComConfig.connParams.baud = w5_baud_rate_230400;
        break;
    case 460800:
        w5ComConfig.connParams.baud = w5_baud_rate_460800;
        break;
    case 921600:
        w5ComConfig.connParams.baud = w5_baud_rate_921600;
        break;
    default:
        fprintf(stderr, "Unsupported baud rate %d", uartBaudRate);
        return 1;
    }
    switch (uartParity) {
    case 1:
        w5ComConfig.connParams.parity = w5_parity_odd;
        break;
    case 2:
        w5ComConfig.connParams.parity = w5_parity_even;
        break;
    default:
        w5ComConfig.connParams.parity = w5_parity_none;
        break;
    }

    //Connect to W5
    printf(">>0, before establish\n");
    ret = establishConnection(&w5ComConfig, &w5ComHandle);
    if (w5_com_ret_ok != ret) {
        printf("Failed to connect to device (%d)\n", ret);
        return 1;
    }
    printf(">>1, establish\n");

    switch (verbosityLevel) {
    case 1:
        W5comLibSetVerbosity(w5ComHandle, verbosity_verbose);
        break;
    case 0:
        W5comLibSetVerbosity(w5ComHandle, verbosity_normal);
        break;
    default:
        W5comLibSetVerbosity(w5ComHandle, verbosity_default);
        break;
    }

    //Get maximum sizes from W5com per connection
    W5comGetMaxBytesPerWrite(w5ComHandle, &maxWriteSize);
    W5comGetMaxBytesPerRead(w5ComHandle, &maxReadSize);

    //Get parameters for W5comCallApi()
    if (scriptFileName) { //Either script mode
        if (openFile(&pScriptFile, scriptFileName, "r")) {
            W5comDisconnect(&w5ComHandle);
            return 1;
        }

        if (inputBinFileName) {
            printf("*** WARNING: Ignored input binary file '%s' because script file '%s' is in use! ***\n", inputBinFileName, scriptFileName);
        }
    }
    else { //Or command line mode
        apiCode = strtol(argv[0], NULL, 0);
        printf("==%d/%s, apiCode: 0x%02x\n", argc, argv[0], apiCode);

        // Parse Query Specifier
        if (parseReadSize(argv[1], NULL, &sizeToRead, maxReadSize, &noQuery, &noSizeGiven) != 0) {
            argparse_usage(&argparse);
            return 1;
        }
        printf("==%d/%s\n", argc, argv[1]);

        // Parse Data Hex String if any
        if (argc == 3) {
            pCmdStr = argv[2];      // pCmdStr is the command hex string
            printf("==%d/%s/%s\n", argc, argv[2], pCmdStr);
        }

        if (inputBinFileName) {
            if (openFile(&pCmdInputBinFile, inputBinFileName, "rb")) {
                W5comDisconnect(&w5ComHandle);
                return 1;
            }
            isInputBinFile = 1;
        }

        writeChunkNum = -1; //increment to 0 for first chunk
    }

    if (outputBinFileName) {
        if (openFile(&pCmdOutputBinFile, outputBinFileName, "wb")) {
            W5comDisconnect(&w5ComHandle);
            return 1;
        }
        isOutputBinFile = 1;
        readChunkNum = -1; //increment to 0 for first chunk
    }

    printf(">>2, main loop\n");
    //main loop
    do {
        int ret = 0;

        // the below if statement should set the following variables, except loop-invariant ones:
        // apiCode, sizeToRead, noQuery, noSizeGiven, writeChunkNum, readChunkNum, writeSize, writeBuf[]
        if (pScriptFile) { //Either script mode
            //come here once per every line in script file

            writeChunkNum = LAST_CHUNK_MASK;    // for the simplicity of this sample code, cmdBuf must be sent in one chunk
            readChunkNum = -1;                  // reset to -1 for every line/command in script file

            if (!readApiCmdFromFile(pScriptFile, &apiCode, &sizeToRead, maxReadSize, &noQuery, &noSizeGiven, cmdBuf)) {
                break;  //exit if no more commands
            }

            if (strlen(cmdBuf) == 0) {
                writeSize = 0;
            }
            else if (convertStringToByteArray(cmdBuf, writeBuf, &writeSize) == -1) {
                printf("Failed to convert command string to byte array\n");
                break;
            }
        }
        else { //Or command line mode (apiCode, sizeToRead, noQuery, noSizeGiven are set; writeChunkNum is initialized to -1)
            // come here once for command string, and every write chunk read from binary file

            if ((writeChunkNum != -1) && (writeChunkNum & LAST_CHUNK_MASK)) {
                break;     // last command chunk is already sent, W5ComCallQuery() should have finished.
                           // one command is done. exit from here as command line mode performs only one command.
            }

            ++writeChunkNum;                    // increment chunk number from -1
            writeChunkNum &= ~LAST_CHUNK_MASK;
            readChunkNum = -1;                  // stay as -1 when sending write chunks

            if (pCmdStr) { // Send command string in a standalone first chunk
                if (convertStringToByteArray(pCmdStr, writeBuf, &writeSize) == -1) {
                    printf("Failed to convert command string to byte array\n");
                    break;
                }
                printf("writeSize:%d\n", writeSize);
                pCmdStr = NULL;                 // mark command string as sent

                // writeChunkNum should be 0 at this point
                if (!isInputBinFile) {
                    writeChunkNum |= LAST_CHUNK_MASK;
                }
            }
            else if (isInputBinFile) {          // pCmdStr is already sent or not specified
                // send binary file one chunk at a time
                int isLastWriteChunk = 0;

                writeSize = getWriteDataFromBin(pCmdInputBinFile, maxWriteSize, writeBuf, &isLastWriteChunk);
                if (writeSize == 0) {
                    if (!isLastWriteChunk) {
                        printf("File reading is incomplete due to fread() error\n");
                        break; //exit if file read fails
                    }
                    // else empty file on first read: send 0-size chunk with LAST_CHUNK_MASK in writeChunkNum
                }
                totalWriteSize += writeSize;

                if (isLastWriteChunk) {
                    writeChunkNum |= LAST_CHUNK_MASK;
                }
            }
            else {  // pCmdStr is not specified and input binary file is not specified
                writeSize = 0;
                // writeChunkNum should be 0 at this point
                writeChunkNum |= LAST_CHUNK_MASK;
            }
        } // !if (pScriptFile)

        printf("API code = 0x%x\nChunk# = 0x%04x\n", apiCode, writeChunkNum);
        if (0 == notShowData)
        {
            printf("Data = ");
            for (i = 0; i < writeSize; i++) {
                printf("%.2x ", writeBuf[i]);
            }
            printf("\n");
        }

        //Send command to W5 using W5comCallApi()
        clockStart = clock();
        ret = W5comCallApi(w5ComHandle, apiCode, writeSize, writeBuf, (uint16_t) writeChunkNum, &msgID);
        clockDiff = clock() - clockStart;
        if (w5_com_ret_ok != ret) {
            printf("W5comCallApi failed (%d)\n", ret);
            break; //exit
        }
        clockTotal += clockDiff;

        printf("W5comCallApi success. Msg ID = %d", msgID);
        if (displayTime) {
            printf(", time taken = %.2fms\n\n", (clockDiff * 1000.0 / CLOCKS_PER_SEC));
        }
        else {
            printf("\n\n");
        }

        sizeReceived = 0;

        //Send query to W5 using W5comCallQuery()
        if (!noQuery && (writeChunkNum & LAST_CHUNK_MASK)) { //only query after last chunk sent
            do { //while (sizeToRead > 0)
                int queryRetry = MAX_QUERY_RETRY; //retry query up to MAX_QUERY_RETRY times
                uint16_t readChunkNumResponded = ++readChunkNum;     //increment expected chunk number;
                size_t chunkSizeToRead = ((sizeToRead > maxReadSize) || noSizeGiven) ? maxReadSize : sizeToRead;

                readBuf = NULL;
                if (chunkSizeToRead > 0) {
                    readBuf = (uint8_t *)calloc(1, chunkSizeToRead);
                    if (readBuf == NULL) {
                        printf("memory allocation failed\n");
                        ret = w5_com_ret_out_of_memory;
                        break; //queryRetry = 0;
                    }
                }

                readDataAvailable = 0;
                while (queryRetry > 0) {
                    clockStart = clock();
                    ret = W5comCallQuery(w5ComHandle,
                                         msgID,
                                         chunkSizeToRead,
                                         (chunkSizeToRead == 0) ? NULL : readBuf,
                                         &cmdStatus,
                                         (chunkSizeToRead == 0) ? NULL : &readDataAvailable,
                                         &readChunkNumResponded);
                    clockDiff = clock() - clockStart;
                    clockTotal += clockDiff;

                    if (w5_com_ret_ok != ret) {
                        printf("W5comCallQuery failed (%d)\n", ret);
                        queryRetry = 0;
                    }
                    else {
                        printf("W5comCallQuery success. Command Status = %d (%s)", cmdStatus, CMD_STATUS_DESC(cmdStatus));
                        if (displayTime) {
                            printf(", time taken = %.2fms\n\n", (clockDiff * 1000.0 / CLOCKS_PER_SEC));
                        }
                        else {
                            printf("\n\n");
                        }

                        if (cmdStatus == CMD_STATUS_PENDING) {
                            if (--queryRetry == 0) {
                                printf("Query stop after retrying %d times.\n", MAX_QUERY_RETRY);
                            }
                            usleep(50 * 1000); // sleep for 50 milliseconds
                        }
                        else {
                            queryRetry = 0;
                        }
                    }
                }  //while (queryRetry > 0)

                if (w5_com_ret_ok == ret) {
                    if (chunkSizeToRead > 0) {
                        if (!noSizeGiven && (readDataAvailable != chunkSizeToRead)) {
                            printf("WARNING! %d-th %s chunk: Requested 0x%08X bytes != Responded 0x%08X bytes!\n",
                                   (readChunkNumResponded & ~LAST_CHUNK_MASK),
                                   ((readChunkNumResponded & LAST_CHUNK_MASK) ? "(last)" : ""),
                                   (unsigned int)chunkSizeToRead, (unsigned int)readDataAvailable);
                        }

                        if (isOutputBinFile) { //put a chunk of data to binary file
                            putReadDataToBin(pCmdOutputBinFile, readDataAvailable, readBuf);
                        }
                        else {
                            printf("%d-th chunk: 0x%02X bytes:\n", (readChunkNumResponded & ~LAST_CHUNK_MASK), (unsigned int)readDataAvailable);

                            for (i = 0; i < readDataAvailable; i += 16) {
                                unsigned int j;
                                uint8_t sectionLen = ((i + 16) <= readDataAvailable) ? 16 : (readDataAvailable % 16);

                                printf("PAYLOAD[%02X:%02X]: ", (unsigned int)i, (unsigned int)(sectionLen + i - 1));

                                for (j = 0; j < 16; j++) {
                                    if (i + j < readDataAvailable) {
                                        printf("%02x ", readBuf[i + j]);
                                    }
                                    else {
                                        printf("   ");
                                    }
                                }

                                printf("       ");

                                for (j = 0; j < 16; j++) {
                                    if (i + j < readDataAvailable) {
                                        if (isgraph(readBuf[i + j]) || (readBuf[i + j] == ' ')) {
                                            printf("%c", readBuf[i + j]);
                                        }
                                        else {
                                            printf(".");
                                        }
                                    }
                                }
                                printf("\n");
                            }
                        }
                        if (!noSizeGiven) {
                            sizeToRead -= readDataAvailable;
                        }
                        sizeReceived += readDataAvailable;
                    }
                } //if (w5_com_ret_ok == ret)

                if (readBuf) {
                    free(readBuf);
                    readBuf = NULL;
                }

                if (readChunkNumResponded & LAST_CHUNK_MASK) {
                    if (!noSizeGiven && (sizeToRead > 0)) {
                        printf("Last Chunk sent by W5. However, there are still 0x%X bytes to read. Abort and move onto next command.\n", (unsigned int)sizeToRead);
                    }
                    printf("Total 0x%X bytes received.\n", (unsigned int)sizeReceived);
                    break;
                }

                if (w5_com_ret_ok != ret) { // query failed after retries. move on to next command
                    break;
                }
            } while ((sizeToRead > 0) || noSizeGiven);
        } //if (!noQuery && (writeChunkNum & LAST_CHUNK_MASK))
    } while (pScriptFile || pCmdInputBinFile); //loop for file input

    if (displayTime) {
        printf("The sum of time taken in W5comCallApi() and W5comCallQuery() is %.2fms\n", (clockTotal * 1000.0 / CLOCKS_PER_SEC));
    }

    //clean up and exit
    if (pScriptFile) {
        fclose(pScriptFile);
    }

    if (pCmdInputBinFile) {
        fclose(pCmdInputBinFile);
    }

    if (pCmdOutputBinFile) {
        fclose(pCmdOutputBinFile);
    }

    //Disconnect from W5
    W5comDisconnect(&w5ComHandle);
    return 0;
}

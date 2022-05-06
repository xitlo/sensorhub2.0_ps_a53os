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
#include "tools_rev.h"
#include <unistd.h>
#include <errno.h>
#include <stdint.h>
#include <fcntl.h>
#include <sys/mman.h>

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

W5ComConfig_s w5ComConfig;
W5ComHandle_s *w5ComHandle = NULL;
size_t maxWriteSize = 0;                            // Output from W5comGetMaxBytesPerWrite()
size_t maxReadSize = 0;                             // Output from W5comGetMaxBytesPerRead()
int isp_uart_reset(uint8_t uartport,uint8_t apiCode,int noQuery,int noSizeGiven,size_t sizeToRead,char *pCmdStr)
{
    W5ComReturn_e ret = w5_com_ret_ok;
    int displayTime = 0;
    int writeChunkNum = LAST_CHUNK_MASK;                // Default to one chunk
                                                        // For input binary file (additional multiple write chunks) support:
    // Parameters for W5comCallApi(), not all are listed
    size_t writeSize = 0;                               // [IN}  Output from getWriteDataFromBin() or convertStringToByteArray() after parsing user input,
                                                        //       specifying the size of one chunk of API Payload
    uint8_t writeBuf[MAX_PROTOCOL_MSG_SIZE] = { 0 };    // [IN]  Output from getWriteDataFromBin() or convertStringToByteArray() from user input,
                                                        //       storing one chunk of API Payload
    uint8_t msgID = 0;                                  // [OUT]

    size_t totalWriteSize = 0;                          // For informational purpose

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
    printf(">>isp_uart_reset start\n");
    /*****************open the uart******************/
    memset(&w5ComConfig, 0, sizeof(W5ComConfig_s));
    w5ComConfig.protoType = w5_protocol_type_uart;
    w5ComConfig.connType = w5_connection_type_serial;
    w5ComConfig.devSpecType = w5_dev_spec_type_port;
    w5ComConfig.devSpec.port = uartport; // UART port/USB number
    w5ComConfig.connParams.baud = w5_baud_rate_default;
    w5ComConfig.connParams.parity = w5_parity_none;
    //Connect to W5
    printf(">>0, before open\n");
    ret = W5comConnect(&w5ComConfig, &w5ComHandle);
    if (w5_com_ret_ok != ret) {
        printf("Failed to connect to device (%d)\n", ret);
        return ret;
    }
    printf(">>1, open uart success\n");
    W5comLibSetVerbosity(w5ComHandle, verbosity_default);
    //Get maximum sizes from W5com per connection
    W5comGetMaxBytesPerWrite(w5ComHandle, &maxWriteSize);
    W5comGetMaxBytesPerRead(w5ComHandle, &maxReadSize);
    /**********************************/
    writeChunkNum = -1; //increment to 0 for first chunk
       //main loop
    do {
        // apiCode, sizeToRead, noQuery, noSizeGiven, writeChunkNum, readChunkNum, writeSize, writeBuf[]
        //Or command line mode (apiCode, sizeToRead, noQuery, noSizeGiven are set; writeChunkNum is initialized to -1)
        ++writeChunkNum;                    // increment chunk number from -1
        writeChunkNum &= ~LAST_CHUNK_MASK;
        readChunkNum = -1;                  // stay as -1 when sending write chunks
        if (pCmdStr) { 
            // 把字符转成hex
            if (convertStringToByteArray(pCmdStr, writeBuf, &writeSize) == -1) {
                printf("Failed to convert command string to byte array\n");
                break;
            }
            printf("writeSize:%d\n", writeSize);
            pCmdStr = NULL;                 // mark command string as sent
            writeChunkNum |= LAST_CHUNK_MASK;
        }else {
            writeSize = 0;
            writeChunkNum |= LAST_CHUNK_MASK;
        }
        printf("API code = 0x%x Chunk# = 0x%04x\n", apiCode, writeChunkNum);
        if(writeSize){
            printf("Data = ");
            for (i = 0; i < writeSize; i++) 
                printf("%.2x ", writeBuf[i]);
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
        //Send query to W5 using W5comCallQuery() if max condition
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
                    } else {
                        printf("W5comCallQuery success. Command Status = %d (%s)", cmdStatus, CMD_STATUS_DESC(cmdStatus));
                        if (displayTime) 
                            printf(", time taken = %.2fms\n\n", (clockDiff * 1000.0 / CLOCKS_PER_SEC));
                        else 
                            printf("\n\n");

                        if (cmdStatus == CMD_STATUS_PENDING) {
                            if (--queryRetry == 0) {
                                printf("Query stop after retrying %d times.\n", MAX_QUERY_RETRY);
                                ret = w5_com_ret_timed_out;
                            }
                            usleep(50 * 1000); // sleep for 50 milliseconds
                        }
                        else {
                            queryRetry = 0;
                            ret = w5_com_ret_rsp_cs_failed;//query failed
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
    } while (0); //loop for file input

    if (displayTime) {
        printf("The sum of time taken in W5comCallApi() and W5comCallQuery() is %.2fms\n", (clockTotal * 1000.0 / CLOCKS_PER_SEC));
    }
    W5comDisconnect(&w5ComHandle);
    return ret;
}
char isp_uart_channel[12]={10,11,12,13,14,15,2,3,4,5,6,7};
int main(int argc, char *argv[])
{
    static char poweronflg=0;
    char errcnt =0;
    W5ComReturn_e ret = w5_com_ret_ok;
    unsigned int  addr, value, map_size;;
    unsigned long base;
    unsigned char *map_base,*map_base1;
    int dev_fd;
    unsigned char index;
    dev_fd = open("/dev/mem", O_RDWR | O_NDELAY | O_DSYNC);
    if (dev_fd < 0) {
        printf("open(/dev/mem) failed.");
        return 0;
    }
    base = 0x80000000;
    map_size = 0x100;
    map_base = (unsigned char *)mmap(NULL, map_size, PROT_READ | PROT_WRITE, MAP_SHARED, dev_fd, base);
    if((long)map_base == -1) {
        printf("map_base map err %d\n",errno);
        return -1;
    }
    base = 0x80010000;
    map_size = 0x190;
    map_base1 = (unsigned char *)mmap(NULL, map_size, PROT_READ | PROT_WRITE, MAP_SHARED, dev_fd, base);
    if((long)map_base1 == -1) {
        printf("map_base1 map err %d\n",errno);
        return -1;
    }
     //uart init
    printf("camera ISP monitor app start.\n");
    while (1)
    {
        sleep(1);
        //if camera is poweroff the stop check isp status
        value = *(volatile unsigned int *)(map_base + 0x20);
        if((value & 0x0000fff0) != 0x0000fff0){
            poweronflg =0;
            printf("value=0x%08x,camera is power-off,don't check\n",value);
            continue;
        }
        if(poweronflg == 0)
            sleep(10);
        poweronflg =1;
        value = *(volatile unsigned int *)(map_base1 + 0);
        if((value & 0x0000fff0) != 0x0000fff0){
            for(char i=0;i<12;i++){
                if((value >> (4+i)) & 0x00000001)
                    continue;
                //ther are cam error,reset all of them
                printf("value:0x%08x cam_channel(%d) uart(%d) is error,reset it\n",value,4+i,isp_uart_channel[i]);
                errcnt=0;
                do{
                    ret = isp_uart_reset(isp_uart_channel[i],0x14,0,1,0,NULL);
                    if(ret != w5_com_ret_ok){
                        usleep(200 * 1000);
                        errcnt++;
                    }else{
                        errcnt = 3;
                    }
                }while(errcnt<3);
                if(errcnt >= 3)
                    printf("----------api_cmd -U10 0x14 max failed----------\n");
                usleep(100 * 1000);
                errcnt =0;
                do{
                    ret = isp_uart_reset(isp_uart_channel[i],0x12,1,0,0,"0000000000000000");
                    if(ret != w5_com_ret_ok){
                        usleep(200 * 1000);
                        errcnt++;
                    }else{
                        errcnt = 3;
                    }
                }while(errcnt<3);
                if(errcnt >= 3)
                    printf("----------api_cmd -Ux 0x12 noquery 0000000000000000 failed----------\n");
            }
            sleep(5);
        }
        //clear the interrupt reg
        //*(volatile unsigned int *)(map_base1 + 0x190) = 0xffffffff;
    }
#if 0
//api_cmd -Ux 0x12 noquery 0000000000000000
    apiCode = 0x12;
    noQuery =1;
    noSizeGiven=0;
    sizeToRead=0;
    pCmdStr = "0000000000000000";
    
//api_cmd -U10 0x14 max
    apiCode = 0x14;
    noQuery =0;
    noSizeGiven = 1;
    sizeToRead=0;

    ///end of uart operate
    //Disconnect from W5
#endif
    return 0;
}

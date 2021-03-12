/*******************************************************************************
*
* The content of this file or document is CONFIDENTIAL and PROPRIETARY
* to GEO Semiconductor. It is subject to the terms of a License Agreement
* between Licensee and GEO Semiconductor, restricting among other things,
* the use, reproduction, distribution and transfer. Each of the embodiments,
* including this information and any derivative work shall retain this
* copyright notice.
*
* Copyright 2013-2020 GEO Semiconductor, Inc.
* All rights reserved.
*
*******************************************************************************/

/*!
 * \file api.h
 * \brief commands and structures for host communication
 * \defgroup HOSTCOMM Host Communication API
 *
 * This file defines the values and structures for host communication. \n
 * Multi-byte primitive data types are in little endian byte order.
 *
 *
 * @{
 */

#ifndef __API_H__
#define __API_H__

/********************************************************************************
 Includes
*******************************************************************************/
#include "base_types.h"
#include "gw5_failures.h"

/*******************************************************************************
 Exported Macro, Enum, Union, Structure, And Type Definitions
*******************************************************************************/

#define GW5_OSD_NUM_CHANNELS 7
#define GW5_OSD_NUM_CLUT_OVERRIDES 16
#define GW5_OSD_MAX_CLUT_ENTRIES 256

/* ------------------------------------------------ */
/* BEGIN: Host Communication Protocol-Level Section */
/* ------------------------------------------------ */
/* *IF* host application communicates to GW5FW through GW5 Host Communication Library (W5Com), */
/* macros under this host communication protocol-level section should be ignored. */

/*! \name Macros - Protocol ACK/NACK Codes Used in ACK and RESPONSE Messages
 *
 *  \brief Host application that communicates to GW5FW through the GEO-provided 
 *  W5Com host library should ignore these host communication protocol-level macros.
 *  These macros are not exposed outside of the GEO-provided W5Com host library.
 *
 *  Conversely, use these macros for implementing your own host library for 
 *  GW5FW-Host communication.
 *
 *  Click on a macro below for more details.
 */
//@{
#define ACK                      (0x01U)   /*!< The message received by GW5FW is valid and it does not have protocol error. */
#define NACK_GENERAL_ERROR       (0x02U)   /*!< The message received by GW5FW is invalid. */
#define NACK_FAILED_CHECKSUM     (0x03U)   /*!< The message received by GW5FW is invalid and it has a checksum error. The message should be re-sent. */
#define NACK_BUSY                (0x04U)   /*!< GW5FW communication queue is full. Retry at a later time */
//@}

/*! \name Macros - Protocol Special Bytes
 *
 *  \brief Host application that communicates to GW5FW through the GEO-provided 
 *  W5Com host library should ignore these host communication protocol-level macros.
 *  These macros are not exposed outside of the GEO-provided W5Com host library.
 *
 *  Conversely, use these macros for implementing your own host library for 
 *  GW5FW-Host communication.
 */
//@{
#define PROTOCOL_START_FRAME     (0x7eU)   /* DEL */
#define PROTOCOL_VERSION         (0x33U)   /* '3' */
#define PROTOCOL_MSG_TYPE_GEO    (0x47U)   /* 'G' */
#define PROTOCOL_MSG_TYPE_ACK    (0x41U)   /* 'A' */
#define PROTOCOL_MSD_TYPE_CUS    (0x43U)   /* 'C' */
#define PROTOCOL_MSG_TYPE_QUERY  (0x51U)   /* 'Q' */
#define PROTOCOL_MSG_TYPE_RESULT (0x52U)   /* 'R' */
#define PROTOCOL_END_FRAME       (0x7fU)   /* '~' */
//@}

/*! \name Macros - Protocol Overhead Sizes
 *
 *  \brief Host application that communicates to GW5FW through the GEO-provided 
 *  W5Com host library should ignore these host communication protocol-level macros.
 *  These macros are not exposed outside of the GEO-provided W5Com host library.
 *
 *  Conversely, use these macros for implementing your own host library for 
 *  GW5FW-Host communication.
 */
//@{
#define PROTOCOL_API_MSG_OVERHEAD    (3U)
#define PROTOCOL_RESULT_MSG_OVERHEAD (7U)
//@}

/*! \name Macros - Protocol Message Sizes
 *
 *  \brief Host application that communicates to GW5FW through the GEO-provided 
 *  W5Com host library should ignore these host communication protocol-level macros.
 *  These macros are not exposed outside of the GEO-provided W5Com host library.
 *
 *  User of the GEO-provided W5Com host library should obtain the maximum 
 *  command write (resp. response read) user-level payload size through 
 *  W5comGetMaxBytesPerWrite() (resp. W5comGetMaxBytesPerRead())
 *
 *  Conversely, use these macros for implementing your own host library for 
 *  GW5FW-Host communication.
 *
 *  Click on a macro below for more details.
 */
//@{
#define MAX_API_WRITE_SIZE                      (0xFFU)    /* To be deprecated */   /*!<  To be deprecated */
#define MAX_PROTOCOL_MSG_API_CMD_PAYLOAD_SIZE   (0x1F7U)   /*!<  0x200 - 9 bytes of API CMD protocol message overhead (includes start and end frame from UART) */
#define MAX_PROTOCOL_MSG_QUERY_RSP_PAYLOAD_SIZE (0x207U)   /*!<  0x200 + 7 bytes of result overhead (command status (1) + chunk number (2) + payload data valid len (4)) */
#define MAX_PROTOCOL_MSG_QUERY_LARGE_RSP_PAYLOAD_SIZE (0x4007U) /*!<  0x4000 + 7 bytes of result overhead (command status (1) + chunk number (2) + payload data valid len (4)) */
//@}

/* ------------------------------------------------ */
/* END: Host Communication Protocol-Level Section   */
/* ------------------------------------------------ */
/*! \name Macros - Protocol Message Sizes (To be deprecated)
 *
 *  \brief To be deprecated. The maximum size host application should allocate for its
 *  outgoing API CMD buffer. The actual size needed/used may be smaller depending 
 *  on the connection hardware/protocol
 */
//@{
#define MAX_PROTOCOL_MSG_SIZE                   (0x200U)   /* To be deprecated */
//@}

/*! \name Macros - Command Status for a Given Message ID
 *
 *  \brief If host application communicates to GW5FW through the GEO-provided 
 *  W5Com host library, the library function W5comCallQuery() returns command 
 *  status from one of its output parameters
 */
//@{
#define CMD_STATUS_UNAVAILABLE   (0U)
#define CMD_STATUS_PENDING       (1U)
#define CMD_STATUS_DONE          (2U)
#define CMD_STATUS_FAIL          (3U)
//@}

/*! \name Macros - Message IDs Used in Host Communication
 *
 *  \brief Click on a macro below for more details.
 */
//@{
#define MAX_MSG_IDS              (16U)     /*!< Maximum number of unique msgID that can be assigned by GW5FW */
#define MAX_MSG_ID               (0x0fU)   /*!< msgID ranges from 0x00 to MAX_MSG_ID */
//@}

/*! \section APICodes API Codes 
 * \name Macros - API Codes and API Related Definitions
 */
//@{
#define MAX_API_CODES                           (0xffU)
#define MAX_API_CODE                            (0xfeU)   /* maximum possible API code for a packet */

#define API_CODE_GET_INFO                       (0x01U)
#define API_CODE_GET_GW5_REGISTERS              (0x02U)

#define API_CODE_GET_CLEAR_FAILURES             (0x0EU)
#define API_CODE_GET_CLEAR_FAILURE_STATUS       (0x0FU)
#define API_CODE_GET_GW5_STATE                  (0x10U)
#define API_CODE_RESET_GW5                      (0x12U)
#define API_CODE_START_VIDEO                    (0x13U)
#define API_CODE_STOP_VIDEO                     (0x14U)
#define API_CODE_GET_VIDEO_FRAMECOUNT           (0x15U)
/* Reserved API Code                            (0x16U) */
/* Reserved API Code                            (0x17U) */
#define API_CODE_GET_VIDEO_INPUT_STATUS         (0x18U)
#define API_CODE_READ_ONLY_MODE                 (0x19U)

/* Reserved API Code                            (0x20U) formerly API_CODE_SET_OSD_LAYER */
/* Reserved API Code                            (0x21U) */
/* Reserved API Code                            (0x22U) formerly API_CODE_LOAD_OSD_LAYER */
/* Reserved API Code                            (0x23U) formerly API_CODE_LOAD_AND_SET_OSD_LAYER */
#define API_CODE_RESIZE_GPU_OSD_LAYER           (0x24U)
#define API_CODE_LOAD_OSD_IMAGE_SLOT            (0x25U)
#define API_CODE_LOAD_OSD_CLUT_SLOT             (0x26U)
#define API_CODE_CONFIG_OSD_CHANNEL             (0x27U)
#define API_CODE_STOP_SPLASH_SCREEN             (0x28U)
#define API_CODE_LOAD_OSD_CLUT                  (0x29U)
#define API_CODE_READ_OSD_CLUT                  (0x2AU)

#define API_CODE_VG_DRAW_PRIMITIVES             (0x30U)
#define API_CODE_VG_DRAW_OBJECTS                (0x31U)
#define API_CODE_VG_DRAW_COLORFILLS             (0x32U)
#define API_CODE_VG_RESET_CMD_BUFFER            (0x33U)

#define API_CODE_LOAD_HOST_WARPMAP              (0x40U)
#define API_CODE_LOAD_APPLY_HOST_ISPCFGBIN      (0x41U)
#define API_CODE_GET_ISP_PARAMS                 (0x42U)
#define API_CODE_ISP_TEST_PATTERN_CTRL          (0x43U)
#define API_CODE_LOAD_APPLY_ISPCFGBIN           (0x44U)

/* Reserved API Code                            (0x50U) formerly API_CODE_SET_CURRENT_CFGPARAM */
#define API_CODE_GET_CURRENT_CFGPARAM           (0x51U)
/* Reserved API Code                            (0x52U) formerly API_CODE_SET_CURRENT_CFG_ARRAY_PARAM */
#define API_CODE_GET_CURRENT_CFG_ARRAY_PARAM    (0x53U)
/* Reserved API Code                            (0x54U) formerly API_CODE_APPLY_CURRENT_CFG */
#define API_CODE_UPDATE_CURRENT_CFGPARAM        (0x55U)
#define API_CODE_CFG_CREATE_FLASH_TBL           (0x56U)

#define API_CODE_GET_RAW_CAPTURE_IMAGE          (0x60U)
#define API_CODE_CROP_OUTPUT_VIDEO              (0x61U)
#define API_CODE_FRAME_SYNC_OPERATIONS          (0x62U)



#define API_CODE_WRITE_BINARY_TO_FLASH          (0x84U)
#define API_CODE_READ_BINARY_FROM_FLASH         (0x85U)


#define API_CODE_WRITE_TBL_TO_FLASH             (0x90U)
#define API_CODE_READ_TBL_FROM_FLASH            (0x91U)
#define API_CODE_CRC_FLASH_AREA                 (0x92U)

#define API_CODE_FLASH_HW_PROTECT               (0x9DU)
#define API_CODE_FLASH_WPN_CONTROL              (0x9EU)

#define API_CODE_GET_SENSOR_FLIP                (0xA0U)
#define API_CODE_SET_SENSOR_FLIP                (0xA1U)
#define API_CODE_GET_SENSOR_OFFSET              (0xA2U)
#define API_CODE_SET_SENSOR_OFFSET              (0xA3U)

/* Reserved API Code                            (0xB0U) */

#define API_CODE_I2C_SLAVE_READ                 (0xC0U)

#define API_CODE_GET_PVT                        (0xD0U)
#define API_CODE_SET_TRIM                       (0xD1U)

#define API_CODE_EVALUATION_API                 (0xE0U)

/* Reserved API Code                            (0xF0U) */

#define API_CODE_INVALID                        (0xFFU)


#define MAX_REGISTERS_TO_READ (16) /* maximum number of registers to read for the api_read_registers command */

#define EVAL_API_CODE_I2C_SLAVE_WRITE           (0x01U)
#define EVAL_API_CODE_CAN_WRITE                 (0x02U)
#define EVAL_API_CODE_CAN_READ                  (0x03U)
/* Retired EVAL API Code                        (0x04U) */
#define EVAL_API_CODE_GET_TARGETLESS_AUTOCAL_RESULT                 (0x05U)
#define EVAL_API_CODE_INPUT_WARP_ADJUST_WARPMAP_FROM_AND_TO_FLASH   (0x06U)
#define EVAL_API_CODE_OUTPUT_WARP_ADJUST_CURRENT_WARPMAP            (0x07U)
//@}

/*!
 * \anchor MultipleCommandPayloadChunks
 * **Sending GEO API Command Message API Payload in Multiple Chunks :** \n
 * A large \ref GeoCommand API Payload may be divided into sequential chunks. 
 * Each chunk is contained in a \ref GeoCommand. Thus, multiple \ref GeoCommand's and \ref AckResponse's are needed.
 * Maximum size per chunk is upper-bounded by \ref MAX_API_WRITE_SIZE and 
 * external factors described in the \ref GeoCommand section in "GW5 API and Firmware User Guide". \n
 * \n
 * \anchor MultipleResponsePayloadChunks
 * **Receiving Query Response Message API Payload in Multiple Chunks :** \n
 * A large \ref QueryResponse API Payload may be divided into sequential chunks. 
 * Each chunk is contained in a \ref QueryResponse. Thus, multiple \ref QueryCommand's and \ref QueryResponse's are needed.
 * Maximum size per chunk is upper-bounded by \ref MAX_PROTOCOL_MSG_QUERY_RSP_PAYLOAD_SIZE (or \ref MAX_PROTOCOL_MSG_QUERY_LARGE_RSP_PAYLOAD_SIZE in compatible ROM) and 
 * external factors described in the \ref QueryResponse section in "GW5 API and Firmware User Guide". \n
 * \n
 * &nbsp;
 */
typedef enum APIDefinition {
    api_get_info = API_CODE_GET_INFO,
    /*!< To get hardware and software information specified as \ref Info_s \n
    * | API Code | \ref GeoCommand API Payload | \ref QueryResponse Payload Data |
    * | :--- | :---- | :---- |
    * | \ref API_CODE_GET_INFO | None | \ref Info_s |
    * | | | |
    *
    */
    api_get_clear_failures = API_CODE_GET_CLEAR_FAILURES,
    /*!< To retrieve (and optionally, clear) all GW5 failure status flags: \ref Gw5FailCodeList_s \n
    * | API Code | \ref GeoCommand API Payload | \ref QueryResponse Payload Data |
    * | :--- | :---- | :---- |
    * | \ref API_CODE_GET_CLEAR_FAILURES | U8 isClearAllFailures flag | \ref Gw5FailCodeList_s |
    * | | | |
    */
    api_get_clear_failure_status = API_CODE_GET_CLEAR_FAILURE_STATUS ,
    /*!< To retrieve (and optionally, clear) status of particular GW5 failure: \ref Gw5FailureStatus_s \n
    * | API Code | \ref GeoCommand API Payload | \ref QueryResponse Payload Data |
    * | :--- | :---- | :---- |
    * | \ref API_CODE_GET_CLEAR_FAILURE_STATUS  | \ref GetClearFailureStatus_s | \ref Gw5FailureStatus_s |
    * | | | |
    */
    api_get_gw5_state = API_CODE_GET_GW5_STATE,
    /*!< To get GW5 State and failure code specified as \ref SysState_s \n
    * | API Code | \ref GeoCommand API Payload | \ref QueryResponse Payload Data |
    * | :--- | :---- | :---- |
    * | \ref API_CODE_GET_GW5_STATE | None | \ref SysState_s |
    * | | | |
    */
    api_reset_gw5 = API_CODE_RESET_GW5,
    /*!< To reset GW5 to operate in a new system state. Not all state transitions are allowed. Please see \ref OperationStates in "GW5 API and Firmware User Guide" \n
    * | API Code | \ref GeoCommand API Payload | \ref QueryResponse Payload Data |
    * | :--- | :---- | :---- |
    * | \ref API_CODE_RESET_GW5 | \ref Reset_s | None. Do not call W5comCallQuery() at all. W5comCallQuery() is not supported for this API. GW5 will be reset after W5comCallApi() is completed |
    * | | | |
    *
    */
    api_start_video = API_CODE_START_VIDEO,
    /*!< To start the video pipeline \n
    * | API Code | \ref GeoCommand API Payload | \ref QueryResponse Payload Data |
    * | :--- | :---- | :---- |
    * | \ref API_CODE_START_VIDEO | None | Optional: None |
    * | | | |
    *
    */
    api_stop_video = API_CODE_STOP_VIDEO,
    /*!< To stop the video pipeline \n
    * | API Code | \ref GeoCommand API Payload | \ref QueryResponse Payload Data |
    * | :--- | :---- | :---- |
    * | \ref API_CODE_STOP_VIDEO | None | Optional: None |
    * | | | |
    *
    */
    api_get_video_framecount = API_CODE_GET_VIDEO_FRAMECOUNT,
    /*!< To read the counter indicating the number of frames has been outputted from GW5  \n
    * | API Code | \ref GeoCommand API Payload | \ref QueryResponse Payload Data |
    * | :--- | :---- | :---- |
    * | \ref API_CODE_GET_VIDEO_FRAMECOUNT | None | U32 payload specifying the framecount |
    * | | | |
    *
    */
    api_get_video_input_status = API_CODE_GET_VIDEO_INPUT_STATUS,
    /*!< To read the input status indicating whether the input of video pipeline is running normally. \n
    * | API Code | \ref GeoCommand API Payload | \ref QueryResponse Payload Data |
    * | :--- | :---- | :---- |
    * | \ref API_CODE_GET_VIDEO_INPUT_STATUS | None | \ref VideoInputStatus_s |
    * | | | |
    *
    */
    api_read_only_mode = API_CODE_READ_ONLY_MODE,
    /*!< To enable or disable firmware flash erase/write features. \n
    * | API Code | \ref GeoCommand API Payload | \ref QueryResponse Payload Data |
    * | :--- | :---- | :---- |
    * | \ref API_CODE_READ_ONLY_MODE |\ref ReadOnlyMode_s | Optional: None |
    * | | | |
    *
    */
    api_read_registers = API_CODE_GET_GW5_REGISTERS,
    /*!< To read up to 32bit registers from the GW5. The maximum number of registers to read is \ref MAX_REGISTERS_TO_READ. \n
    * | API Code | \ref GeoCommand API Payload | \ref QueryResponse Payload Data |
    * | :--- | :---- | :---- |
    * | \ref API_CODE_GET_GW5_REGISTERS | \ref RegisterRead_s | \ref RegisterReadResult_s |
    * | | | |
    *
    */
    api_resize_gpu_osd_layer = API_CODE_RESIZE_GPU_OSD_LAYER,
    /*!< To resize the OSD layer specified attached to the GPU \n
    * | API Code | \ref GeoCommand API Payload | \ref QueryResponse Payload Data |
    * | :--- | :---- | :---- |
    * | \ref API_CODE_RESIZE_GPU_OSD_LAYER | \ref ResizeGpuOsdLayer_s | Optional: None |
    * | | | |
    *
    */
    api_load_osd_image_slot = API_CODE_LOAD_OSD_IMAGE_SLOT,
    /*!< To load OSD image slot with image data from flash \n
    * | API Code | \ref GeoCommand API Payload | \ref QueryResponse Payload Data |
    * | :--- | :---- | :---- |
    * | \ref API_CODE_LOAD_OSD_IMAGE_SLOT | \ref LoadOsdImageSlot_s | Optional: None |
    * | | | |
    *
    */
    api_load_osd_clut_slot = API_CODE_LOAD_OSD_CLUT_SLOT,
    /*!< To load OSD CLUT slot with CLUT data from flash \n
    * | API Code | \ref GeoCommand API Payload | \ref QueryResponse Payload Data |
    * | :--- | :---- | :---- |
    * | \ref API_CODE_LOAD_OSD_CLUT_SLOT | \ref LoadOsdClutSlot_s | Optional: None |
    * | | | |
    *
    */
    api_config_osd_channel = API_CODE_CONFIG_OSD_CHANNEL,
    /*!< To configure one or several OSD channels. Changes are applied synchronously \n
    * | API Code | \ref GeoCommand API Payload | \ref QueryResponse Payload Data |
    * | :--- | :---- | :---- |
    * | \ref API_CODE_CONFIG_OSD_CHANNEL | 1 to \ref GW5_OSD_NUM_CHANNELS \ref ConfigOsdChannel_s | Optional: None |
    * | | | |
    *
    */
    api_stop_splash_screen = API_CODE_STOP_SPLASH_SCREEN,
    /*!< To stop splash screen animation and show live video \n
    * | API Code | \ref GeoCommand API Payload | \ref QueryResponse Payload Data |
    * | :--- | :---- | :---- |
    * | \ref API_CODE_STOP_SPLASH_SCREEN | None | Optional: None |
    * | | | |
    *
    */
    api_load_osd_clut = API_CODE_LOAD_OSD_CLUT,
    /*!< Load the OSD CLUT to an unreferenced OSD CLUT slot \n
    *    \n
    *    A large \ref LoadOsdClut_s may require multiple chunks to send. For details, please see \ref MultipleCommandPayloadChunks. \n
    * | API Code | \ref GeoCommand API Payload | \ref QueryResponse Payload Data |
    * | :--- | :---- | :---- |
    * | \ref API_CODE_LOAD_OSD_CLUT | \ref LoadOsdClut_s | Optional: None |
    * | | | |
    *
    */
    api_read_osd_clut = API_CODE_READ_OSD_CLUT,
    /*!< Read the OSD CLUT from a slot \n
    *    \n
    *    A large \ref ClutTableEntry_s may require multiple chunks to receive. For details, please see \ref MultipleResponsePayloadChunks. \n
    * | API Code | \ref GeoCommand API Payload | \ref QueryResponse Payload Data |
    * | :--- | :---- | :---- |
    * | \ref API_CODE_READ_OSD_CLUT | U8 clutSlotId | Up to 256 \ref ClutTableEntry_s |
    * | | | |
    *
    */
    api_draw_vg_primitives = API_CODE_VG_DRAW_PRIMITIVES,
    /*!< To draw one or more graphics primitives specified by \ref DrawMultipleVgPrimitives_s \n
    *    \n
    *    A large \ref DrawMultipleVgPrimitives_s may require multiple chunks to send. For details, please see \ref MultipleCommandPayloadChunks. \n
    * | API Code | \ref GeoCommand API Payload | \ref QueryResponse Payload Data |
    * | :--- | :---- | :---- |
    * | \ref API_CODE_VG_DRAW_PRIMITIVES | \ref DrawMultipleVgPrimitives_s | Optional: None |
    * | | | |
    *
    */
    api_draw_vg_objects = API_CODE_VG_DRAW_OBJECTS,
    /*!< To draw one or more high level graphics object specified by \ref DrawMultipleVgObject_s \n
    *    \n
    *    A large \ref DrawMultipleVgObject_s may require multiple chunks to send. For details, please see \ref MultipleCommandPayloadChunks. \n
    * | API Code | \ref GeoCommand API Payload | \ref QueryResponse Payload Data |
    * | :--- | :---- | :---- |
    * | \ref API_CODE_VG_DRAW_OBJECTS | \ref DrawMultipleVgObject_s | Optional: None |
    * | | | |
    *
    */
    api_draw_vg_colorfills = API_CODE_VG_DRAW_COLORFILLS,
    /*!< To draw a series of simple colorfills on screen specified by \ref DrawMultipleVgColorFills_s \n
    *    \n
    *    A large \ref DrawMultipleVgColorFills_s may require multiple chunks to send. For details, please see \ref MultipleCommandPayloadChunks. \n
    * | API Code | \ref GeoCommand API Payload | \ref QueryResponse Payload Data |
    * | :--- | :---- | :---- |
    * | \ref API_CODE_VG_DRAW_COLORFILLS | \ref DrawMultipleVgColorFills_s | Optional: None |
    * | | | |
    *
    */
    api_reset_vg_cmd_buffer = API_CODE_VG_RESET_CMD_BUFFER,
    /*!< To reset the GPU command buffer removing all drawing from the buffer \n
    * | API Code | \ref GeoCommand API Payload | \ref QueryResponse Payload Data |
    * | :--- | :---- | :---- |
    * | \ref API_CODE_VG_RESET_CMD_BUFFER | None | Optional: None |
    * | | | |
    *
    */
    api_load_host_warpmap = API_CODE_LOAD_HOST_WARPMAP,
    /*!< To load and apply a warpmap from Host to GW5 memory. \n
    *    \n
    *    A large warpmap may require multiple chunks to send. For details, please see \ref MultipleCommandPayloadChunks. \n
    * | API Code | \ref GeoCommand API Payload | \ref QueryResponse Payload Data |
    * | :--- | :---- | :---- |
    * | \ref API_CODE_LOAD_HOST_WARPMAP | warpmap data | Optional: None |
    * | | | |
    *
    */
    api_load_apply_host_ispcfgbin = API_CODE_LOAD_APPLY_HOST_ISPCFGBIN,
    /*!< To load the new ISP Tuning configuration binary (in JBF format generated by genbin.py) from Host and then apply the new configuration settings on top of current configuration \n
    *    \n
    *    The binary can be the entire or partial configuration data. \n
    *    \n
    *    A large configuration binary may require multiple chunks to send. For details, please see \ref MultipleCommandPayloadChunks. \n
    * | API Code | \ref GeoCommand API Payload | \ref QueryResponse Payload Data |
    * | :--- | :---- | :---- |
    * | \ref API_CODE_LOAD_APPLY_HOST_ISPCFGBIN | ISP configuration JBF binary | Optional: None |
    * | | | |
    *
    */
    api_get_isp_params = API_CODE_GET_ISP_PARAMS,
    /*!< To get live ISP parameters used for live tuning as specified in \ref IspLiveParams_s \n
    * | API Code | \ref GeoCommand API Payload | \ref QueryResponse Payload Data |
    * | :--- | :---- | :---- |
    * | \ref API_CODE_GET_ISP_PARAMS | None | \ref IspLiveParams_s |
    * | | | |
    *
    */
    api_isp_test_pattern_ctrl = API_CODE_ISP_TEST_PATTERN_CTRL,
    /*!< Enable/disable the ISP test pattern generation and set pattern type as specified in \ref IspTestPattern_s \n
    * | API Code | \ref GeoCommand API Payload | \ref QueryResponse Payload Data |
    * | :--- | :---- | :---- |
    * | \ref API_CODE_ISP_TEST_PATTERN_CTRL | \ref IspTestPattern_s | Optional: None |
    * | | | |
    *
    */
    api_load_apply_ispcfgbin = API_CODE_LOAD_APPLY_ISPCFGBIN,
    /*!< To load the new ISP Tuning configuration binary (in JBF format generated by genbin.py) from SSM and then apply the new configuration settings on top of current configuration \n
    *    \n
    *    The binary can be the entire or partial configuration data.\n
    * | API Code | \ref GeoCommand API Payload | \ref QueryResponse Payload Data |
    * | :--- | :---- | :---- |
    * | \ref API_CODE_LOAD_APPLY_ISPCFGBIN | U16 representing SSM index | Optional: None |
    * | | | |
    *
    */
    api_get_current_cfg_param = API_CODE_GET_CURRENT_CFGPARAM,
    /*!< To get a current configuration parameter value with specified by CfgParamRequest_s \n
    * | API Code | \ref GeoCommand API Payload | \ref QueryResponse Payload Data |
    * | :--- | :---- | :---- |
    * | \ref API_CODE_GET_CURRENT_CFGPARAM | \ref CfgParamRequest_s | \ref CfgParamValue_s |
    * | | | |
    *
    */
    api_get_current_cfg_array_param = API_CODE_GET_CURRENT_CFG_ARRAY_PARAM,
    /*!< To get a current configuration array parameter value with specified by CfgArrayParamRequest_s \n
    * | API Code | \ref GeoCommand API Payload | \ref QueryResponse Payload Data |
    * | :--- | :---- | :---- |
    * | \ref API_CODE_GET_CURRENT_CFG_ARRAY_PARAM | \ref CfgArrayParamRequest_s | \ref CfgParamValue_s |
    * | | | |
    *
    */
    api_update_current_cfg_param = API_CODE_UPDATE_CURRENT_CFGPARAM,
    /*!< To set and optionally apply the current configuration parameters specified by \ref CfgParamUpdate_s \n
    *    \n
    *    A large \ref CfgParamUpdate_s may require multiple chunks to send. For details, please see \ref MultipleCommandPayloadChunks. \n
    * | API Code | \ref GeoCommand API Payload | \ref QueryResponse Payload Data |
    * | :--- | :---- | :---- |
    * | \ref API_CODE_UPDATE_CURRENT_CFGPARAM | \ref CfgParamUpdate_s | Optional: None |
    * | | | |
    *
    */
    api_cfg_create_flash_tbl = API_CODE_CFG_CREATE_FLASH_TBL,
    /*!< To write the configuration parameters specified by \ref CfgParamTblCreateList_s to a table in flash \n
    *    \n
    *    A large \ref CfgParamTblCreateList_s may require multiple chunks to send. For details, please see \ref MultipleCommandPayloadChunks. \n
    * | API Code | \ref GeoCommand API Payload | \ref QueryResponse Payload Data |
    * | :--- | :---- | :---- |
    * | \ref API_CODE_CFG_CREATE_FLASH_TBL | \ref CfgParamTblCreateList_s | Optional: None |
    * | | | |
    *
    */
    api_get_raw_capture_image = API_CODE_GET_RAW_CAPTURE_IMAGE,
    /*!< To get a raw image data captured from multiple frames out of the sensor \n
    *    \n
    *    A large \ref RawImageData_s may require multiple chunks to receive. For details, please see \ref MultipleResponsePayloadChunks. \n
    * | API Code | \ref GeoCommand API Payload | \ref QueryResponse Payload Data |
    * | :--- | :---- | :---- |
    * | \ref API_CODE_GET_RAW_CAPTURE_IMAGE | \ref RawImageInfo_s | \ref RawImageData_s |
    * | | | |
    *
    */
    api_crop_output_video = API_CODE_CROP_OUTPUT_VIDEO,
    /*!< To configure output video crop feature \n
    * | API Code | \ref GeoCommand API Payload | \ref QueryResponse Payload Data |
    * | :--- | :---- | :---- |
    * | \ref API_CODE_CROP_OUTPUT_VIDEO | \ref CropOutputVideo_s | Optional: None |
    * | | | |
    *
    */
    api_frame_sync_operations = API_CODE_FRAME_SYNC_OPERATIONS,
    /*!< To perform multiple video pipeline video-pipeline-related operations on the same output frame \n
    * | API Code | \ref GeoCommand API Payload | \ref QueryResponse Payload Data |
    * | :--- | :---- | :---- |
    * | \ref API_CODE_FRAME_SYNC_OPERATIONS | \ref FrameSyncOp_s | Optional: None |
    * | | | |
    *
    */
    api_write_binary_to_flash = API_CODE_WRITE_BINARY_TO_FLASH,
    /*!< To write binary data in the flash specified by \ref Binary_s \n
    *    \n
    *    A large \ref Binary_s may require multiple chunks to send. For details, please see \ref MultipleCommandPayloadChunks. \n
    *    \n
    *    The first chunk will notify the GW5FW to erase the blocks in the flash to be updated \n
    *    \n
    *    This API involves GW5FW's access to flash. For prerequisites to execute this API, please see \ref FlashLayoutAndAccessUsage in "GW5 API and Firmware User Guide" \n
    * | API Code | \ref GeoCommand API Payload | \ref QueryResponse Payload Data |
    * | :--- | :---- | :---- |
    * | \ref API_CODE_WRITE_BINARY_TO_FLASH | \ref Binary_s | Optional: None |
    * | | | |
    *
    */
    api_read_binary_from_flash = API_CODE_READ_BINARY_FROM_FLASH,
    /*!< To read back the binary data from flash \n
    *    \n
    *    A large binary may require multiple chunks to receive. For details, please see \ref MultipleResponsePayloadChunks. \n
    *    \n
    *    This API involves GW5FW's access to flash. For prerequisites to execute this API, please see \ref FlashLayoutAndAccessUsage in "GW5 API and Firmware User Guide" \n
    * | API Code | \ref GeoCommand API Payload | \ref QueryResponse Payload Data |
    * | :--- | :---- | :---- |
    * | \ref API_CODE_READ_BINARY_FROM_FLASH | 1 byte payload containing the \ref Binary_e | flash binary as raw bytes |
    * | | | |
    *
    */
    api_write_tbl_to_flash = API_CODE_WRITE_TBL_TO_FLASH,
    /*!< To write the specified ID to flash specified by \ref FlashTable_s. \n
    *    \n
    *    A large \ref FlashTable_s may require multiple chunks to send. For details, please see \ref MultipleCommandPayloadChunks. \n
    *    \n
    *    The first chunk will notify the GW5FW to erase the blocks in the flash to be updated \n
    *    The last chunk will notify the GW5FW to write the metadata header information of the table into the flash\n
    *    \n
    *    This API involves GW5FW's access to flash. For prerequisites to execute this API, please see \ref FlashLayoutAndAccessUsage in "GW5 API and Firmware User Guide" \n
    * | API Code | \ref GeoCommand API Payload | \ref QueryResponse Payload Data |
    * | :--- | :---- | :---- |
    * | \ref API_CODE_WRITE_TBL_TO_FLASH |\ref FlashTable_s | Optional: None |
    * | | | |
    *
    */
    api_read_tbl_from_flash = API_CODE_READ_TBL_FROM_FLASH,
    /*!< To read the specified ID from flash. \n
    *    \n
    *    A large \ref FlashTable_s may require multiple chunks to receive. For details, please see \ref MultipleResponsePayloadChunks. \n
    *    \n
    *    This API involves GW5FW's access to flash. For prerequisites to execute this API, please see \ref FlashLayoutAndAccessUsage in "GW5 API and Firmware User Guide" \n
    * | API Code | \ref GeoCommand API Payload | \ref QueryResponse Payload Data |
    * | :--- | :---- | :---- |
    * | \ref API_CODE_READ_TBL_FROM_FLASH | \ref FlashTableLocation_s | \ref FlashTable_s |
    * | | | |
    *
    */
    api_crc_flash_area = API_CODE_CRC_FLASH_AREA,
    /*!< To compute the CRC of an area of flash. \n
    *    \n
    *    This API involves GW5FW's access to flash. For prerequisites to execute this API, please see \ref FlashLayoutAndAccessUsage in "GW5 API and Firmware User Guide" \n
    * | API Code | \ref GeoCommand API Payload | \ref QueryResponse Payload Data |
    * | :--- | :---- | :---- |
    * | \ref API_CODE_CRC_FLASH_AREA | \ref FlashCrcRequest_s | U32 representing the requested flash CRC |
    * | | | |
    *
    */
    api_flash_hw_protect = API_CODE_FLASH_HW_PROTECT,
    /*!< To configure flash hardware protection. \n
    * | API Code | \ref GeoCommand API Payload | \ref QueryResponse Payload Data |
    * | :--- | :---- | :---- |
    * | \ref API_CODE_FLASH_HW_PROTECT | \ref FlashHwProtect_s | Optional: None |
    * | | | |
    *
    */
    api_flash_wpn_control = API_CODE_FLASH_WPN_CONTROL,
    /*!< To assert/deassert GW5 pin connected to flash WP# pin. \n
    * | API Code | \ref GeoCommand API Payload | \ref QueryResponse Payload Data |
    * | :--- | :---- | :---- |
    * | \ref API_CODE_FLASH_WPN_CONTROL | \ref FlashWpnControl_s | Optional: None |
    * | | | |
    *
    */
    api_get_sensor_flip = API_CODE_GET_SENSOR_FLIP,
    /*!< To get the current sensor vertical and horizontal flip state \n
    * | API Code | \ref GeoCommand API Payload | \ref QueryResponse Payload Data |
    * | :--- | :---- | :---- |
    * | \ref API_CODE_GET_SENSOR_FLIP | None | \ref SensorFlip_s |
    * | | | |
    *
    */
    api_set_sensor_flip = API_CODE_SET_SENSOR_FLIP,
    /*!< To set the sensor vertical and horizontal flip \n
    * | API Code | \ref GeoCommand API Payload | \ref QueryResponse Payload Data |
    * | :--- | :---- | :---- |
    * | \ref API_CODE_SET_SENSOR_FLIP | \ref SensorFlip_s | Optional: None |
    * | | | |
    *
    */
    api_get_sensor_offset = API_CODE_GET_SENSOR_OFFSET,
    /*!< To get the current sensor X- and Y- offset \n
    * | API Code | \ref GeoCommand API Payload | \ref QueryResponse Payload Data |
    * | :--- | :---- | :---- |
    * | \ref API_CODE_GET_SENSOR_OFFSET | None | \ref SensorOffset_s |
    * | | | |
    *
    */
    api_set_sensor_offset = API_CODE_SET_SENSOR_OFFSET,
    /*!< To set the sensor X- and Y- offset \n
    * | API Code | \ref GeoCommand API Payload | \ref QueryResponse Payload Data |
    * | :--- | :---- | :---- |
    * | \ref API_CODE_SET_SENSOR_OFFSET | \ref SensorOffset_s | Optional: None |
    * | | | |
    *
    */
    api_i2c_slave_read = API_CODE_I2C_SLAVE_READ,
    /*!< To read from an I2C slave connected to GW5. \n
    * | API Code | \ref GeoCommand API Payload | \ref QueryResponse Payload Data |
    * | :--- | :---- | :---- |
    * | \ref API_CODE_I2C_SLAVE_READ | \ref I2cSlaveRead_s | U32 payload containing data read from slave |
    * | | | |
    *
    */
    api_get_pvt = API_CODE_GET_PVT,
    /*!< To get the current PVT (process, voltage, temperature) values \n
    * | API Code | \ref GeoCommand API Payload | \ref QueryResponse Payload Data |
    * | :--- | :---- | :---- |
    * | \ref API_CODE_GET_PVT| None            | \ref PVTValues_s |
    * | | | |
    *
    */
    api_set_trim = API_CODE_SET_TRIM,
    /*!< To set the trim value for temperature \n
    * | API Code | \ref GeoCommand API Payload | \ref QueryResponse Payload Data |
    * | :--- | :---- | :---- |
    * | \ref API_CODE_SET_TRIM | U8 payload specifying the trim value             | Optional: None |
    * | | | |
    *
    */
    api_evaluation_api = API_CODE_EVALUATION_API,
    /*!< To trigger an evaluation API  See \ref EvalAPICode_e for the list of evaluation API codes. \n
    * | API Code | \ref GeoCommand API Payload | \ref QueryResponse Payload Data |
    * | :--- | :---- | :---- |
    * | \ref API_CODE_EVALUATION_API | \ref EvalApi_s | varies; see \ref EvalAPICode_e |
    * | | | |
    *
    */
    api_maxCode = MAX_API_CODE
    /*!< Maximum Possible API Code is \ref MAX_API_CODE */
} APICode_e;    /*!< **APICode_e value**: \n
                 *   The upper nibble (Bits 7:4) specifies the groupID and the lower nibble (Bits 3:0) specifies the subgroupID. \n
                 *   If the group has more than 15 subgroupID API, the lower nibble is set to 0xF and the first byte of the payload specifies the subgroupID API code up to 256 followed by the actual payload. */


/*! See \ref api_evaluation_api for details on how to trigger an Evaluation API; */
typedef enum EvalAPIDefinition {
    eval_api_i2c_slave_write = EVAL_API_CODE_I2C_SLAVE_WRITE,
    /*!< To write to an I2C slave connected to GW5. \n
    * | \ref EvalApi_s.evalApiCode | \ref EvalApi_s.payload | \ref QueryResponse Payload Data |
    * | :--- | :---- | :---- |
    * | \ref EVAL_API_CODE_I2C_SLAVE_WRITE | \ref I2cSlaveWrite_s | Optional: None |
    * | | | |
    *
    */
    eval_api_can_write = EVAL_API_CODE_CAN_WRITE,
    /*!< To write a message to CAN bus and also print the message over UART1. \n
    * | \ref EvalApi_s.evalApiCode | \ref EvalApi_s.payload | \ref QueryResponse Payload Data |
    * | :--- | :---- | :---- |
    * | \ref EVAL_API_CODE_CAN_WRITE | \ref CanWrite_s | Optional: None |
    * | | | |
    *
    */
    eval_api_can_read = EVAL_API_CODE_CAN_READ,
    /*!< To read the GW5 fw buffer of the last received message from CAN bus triggered by the CAN Host. \n
    * | \ref EvalApi_s.evalApiCode | \ref EvalApi_s.payload | \ref QueryResponse Payload Data |
    * | :--- | :---- | :---- |
    * | \ref EVAL_API_CODE_CAN_READ | None | \ref CanReadResponse_s |
    * | | | |
    *
    */
    eval_api_get_targetless_autocal_result = EVAL_API_CODE_GET_TARGETLESS_AUTOCAL_RESULT,
    /*!< To report result from performing targetless AutoCAL. \n
    * | \ref EvalApi_s.evalApiCode | \ref EvalApi_s.payload | \ref QueryResponse Payload Data |
    * | :--- | :---- | :---- |
    * | \ref EVAL_API_CODE_GET_TARGETLESS_AUTOCAL_RESULT | \ref TargetlessAutoCalCmd_s | \ref TargetlessAutoCalResult_s |
    * | | | |
    *
    */
    eval_api_input_warp_adjust_warpmap_from_and_to_flash = EVAL_API_CODE_INPUT_WARP_ADJUST_WARPMAP_FROM_AND_TO_FLASH,
    /*!< To perform the following operations:
    *    - (warpmap source) read a warpmap from the specified SSM slot
    *    - (warpmap modification) adjust the warpmap read
    *    - (warpmap destination) write the adjusted warpmap to the specified SSM slot
    *    .
    *    This API involves GW5FW's access to flash. For prerequisites to execute this API, please see \ref FlashLayoutAndAccessUsage in "GW5 API and Firmware User Guide" \n
    *    \n
    *    Operation failures include but are not limited to:
    *    - Warpmap read is invalid.
    *    - Flash access policy is not configured for writing.
    *    .
    * | \ref EvalApi_s.evalApiCode | \ref EvalApi_s.payload | \ref QueryResponse Payload Data |
    * | :--- | :---- | :---- |
    * | \ref EVAL_API_CODE_INPUT_WARP_ADJUST_WARPMAP_FROM_AND_TO_FLASH | \ref InputWarpAdjWmFromAndToFlash_s | Optional: None  |
    * | | | |
    *
    */
    eval_api_output_warp_adjust_current_warpmap = EVAL_API_CODE_OUTPUT_WARP_ADJUST_CURRENT_WARPMAP,
    /*!< To perform the following operations:
    *    - (warpmap source #1) read the warpmap from SSM, and copy the warpmap to the warpmap software buffer,
    *    - (OR warpmap source #2) read the warpmap from the warpmap software buffer, 
    *                             which can be populated by a prior execution of this API with warpmap source set to SSM
    *    - (warpmap modification) apply centering, rotation, shifting, and scaling on the output space of the warpmap read
    *    - (warpmap destination #1) load the adjusted warpmap to eWarp hardware engine
    *    - (AND warpmap destination #2) write the adjusted warpmap to the warpmap software buffer
    *    .
    *    Operation failures include but are not limited to:
    *    - ROM configuration does not include the warpmap software buffer.
    *    - Warpmap read is invalid.
    *    - The adjusted view is partially outside of the pre-adjusted output space resolution.
    *    .
    *    API usage (in sequence) example for repeated rotating/shifting/scaling: \n
    *    - \ref eval_api_get_targetless_autocal_result
    *    - \ref api_read_only_mode
    *    - \ref eval_api_input_warp_adjust_warpmap_from_and_to_flash
    *    - \ref eval_api_output_warp_adjust_current_warpmap (from SSM)
    *    - \ref eval_api_output_warp_adjust_current_warpmap (from the warpmap software buffer) (repeatedly with different \ref OutputWarpAdjCurrentWm_s parameters)
    *    .
    *    The warpmap software buffer is available only in certain ROM(s).
    * | \ref EvalApi_s.evalApiCode | \ref EvalApi_s.payload | \ref QueryResponse Payload Data |
    * | :--- | :---- | :---- |
    * | \ref EVAL_API_CODE_OUTPUT_WARP_ADJUST_CURRENT_WARPMAP | \ref OutputWarpAdjCurrentWm_s | Optional: None  |
    * | | | |
    *
    */
} EvalAPICode_e;



typedef enum {
    api_get_info_device_w5_evk      = 0x00000001,
    api_get_info_device_w5_dwarpcam = 0x00000002,
    api_get_info_device_customer    = 0x00000003
} DeviceName_e;

typedef enum {
    system_reset                = 0,
    system_init                 = 1,
    system_active               = 2,
    system_lowpower             = 3,
    system_failsafe             = 4
} SysState_e;

typedef enum {
    resetaction_none            = 0,
    resetaction_failsafestate   = 1,    /*!< After reset, GW5 will boot into Failsafe State ("param" in \ref Reset_s is unused) */
    resetaction_alt_config      = 2,    /*!< After reset, GW5 will boot into Main firmware and load alternative JSON configuration from preallocated fixed flash location ("param" in \ref Reset_s is unused) */
    resetaction_alt_config_ssm  = 3     /*!< After reset, GW5 will boot into Main firmware and load alternative JSON configuration from SSM index
                                             "param" in \ref Reset_s is the SSM index of the first JBF (syscfg) in the alternative set
                                             subsequent JBFs (warpcfg, ispcfg, geocfg) are located at consecutively increasing indices */
} ResetAction_e;

typedef enum {
    binary_bootloader           = 0,
    binary_bootconfig           = 1,
    binary_failsafefw           = 2,
    binary_lx6fw                = 3,
    /* values 4, 5 reserved */
    binary_ssm                  = 6,
    binary_full_rom             = 0xEE
} Binary_e;

typedef enum {
    json_spec_id_syscfg         = 2,
    json_spec_id_warpcfg        = 3,

    json_spec_id_ispcfg         = 5
} JsonSpecId_e;

typedef enum {
    little_endian               = 0,
    big_endian                  = 1
} Endianness_e;

typedef enum {
    flat_field                  = 0,
    horizontal_bars             = 1,
    vertical_bars               = 2,
    vertical_color_bars         = 3,
    small_rectangle             = 4,
    black_with_white_border     = 5
} IspTestPatternType_e;

typedef enum {
    ssm_index                   = 0,
    flash_offset                = 1
} FlashIdType_e;

typedef enum {
    payload_type_warpmap        = 0x0131,
    payload_type_warp_blockmask = 0x0231,
    payload_type_osdpkg         = 0x0134,
    // payload_type_json_reserved  = 0x0135,
    payload_type_json_syscfg    = 0x0235,
    payload_type_json_warpcfg   = 0x0335,
    payload_type_json_ispcfg    = 0x0535,
    payload_type_json_geocfg    = 0x0735,
    // payload type 0xNN36 is reserved for variations of ld_module
    payload_type_ld_module      = 0x0136,
    payload_type_lens_data      = 0x0137,
    payload_type_lens_model     = 0x0138,
    payload_type_p5_executable  = 0x013e,
    payload_type_p5_init_data   = 0x023e,
    payload_type_vg_asset       = 0x013f,
    payload_type_user_custom    = 0x0040
} FlashPayloadType_e;

/* vector graphics primitives - small primitives stored in URAM */
typedef enum {
    vg_graphics_primitive_line     = 0x01,
    vg_graphics_primitive_circle   = 0x02,
    vg_graphics_primitive_square    = 0x03,
    vg_graphics_primitive_triangle = 0x04,
} VgGraphicsPrimitiveType_e;

typedef enum {
    vg_graphics_primitive_place_in_cmd_buf = 0x0, /*!< place the primitive in the command buffer, but don't draw anything yet */
    vg_graphics_primitive_draw = 0x01, /*!< the graphics objects in the GPU command buffer should be drawn after the current object has been placed in the command buffer */
    vg_graphics_primitive_wait_for_render_completion = 0x800, /*!< the primitive draw command should wait for render completion before returning. */
                                                              /*!< this flag can only be used in conjunction with vg_graphics_primitive_draw. */
} VgGraphicsPrimitiveFlags_e;

typedef enum {
    vg_graphics_colorfill_place_in_cmd_buf = 0x0, /*!< place the color fill in the command buffer, but don't draw anything yet */
    vg_graphics_colorfill_draw = 0x01,            /*!< execute the command buffer after the current colorfill has been placed in the command buffer */
    vg_graphics_colorfill_wait_for_render_completion = 0x800, /*!< the primitive draw command should wait for the render completion before returning. */
                                                              /*!< this flag can only be used in conjunction with vg_graphics_colorfill_draw. */
} VgGraphicsColorFillFlags_e;

/* vector graphics objects - complex graphics objects read from flash */
typedef enum {
    /* the fields below contain additional information about how the object should be rendered, but do not affect the size of the DrawVgObject_s structure sent over host communication */         
    vg_graphics_transformation_draw        =  0x01, /*!< the graphics objects in the GPU command buffer should be drawn after the current object has been placed in the command buffer */
    vg_graphics_transformation_bezier      =  0x02, /*!< the lines making up the object are in Bezier curves. If this flag is not set the lines are assumed to be samples */
    vg_graphics_transformation_3d          =  0x04, /*!< use the transformation libraries to transform the 3D world coordinates of the object to 2D screen coordinates before warp transformation */
    /* the fields below are parameters included in the transformationParams field of the DrawVgObject_s structure sent over host communication */
    /* If the fields below are included they must be included in the order below. If vg_graphics_transformation_3d is set these transformations are applied after the 3D->2D transformation */
    /* and before the warp transformation. If the vg_graphics_transformation_3d bit is not set then these parameters are applied before the warp transformation. */
    vg_graphics_transformation_center      =  0x08, /*!< the center of the object is included in an X, Y float pair */
    vg_graphics_transformation_screen      =  0x10, /*!< the screen position of the object is included in an X, Y float pair */
    vg_graphics_transformation_scale       =  0x20, /*!< the scaling of the object in an X, Y float pair */
    vg_graphics_transformation_rotate      =  0x40, /*!< the rotation of the object as a float in degrees. This is applied after the 3D->2D transformation but before the warp transformation */ 
    vg_graphics_transformation_parameter_0 =  0x80, /*!< extra transformation parameters as a float into the transformation function. This parameter can be used to specify the wheel angle*/
    vg_graphics_transformation_parameter_1 = 0x100, /*!< for future expansion */
    vg_graphics_transformation_parameter_2 = 0x200, /*!< for future expansion */
    vg_graphics_transformation_parameter_3 = 0x400, /*!< for future expansion */
    vg_graphics_transformation_wait_for_render_completion = 0x800, /*!< the primitive draw command should wait for the render completion before returning. */
                                                                   /*!< this flag can only be used in conjunction with vg_graphics_transformation_draw. */
} VgGraphicsTransformationFlags_e;

/****************************************************************************/
/* Note: in the structures below values larger than U8 are little endian.   */
/* These structures are the content of the packets sent from the host to the */
/* host communication task.                                                 */
/****************************************************************************/

#pragma pack(1)
typedef struct {
    U64 apiVersion;         /*!< | Size | Type | Short Description | Return Value |
                             *   | ----: | :---- | :---- | :---- |
                             *   | 8 Bytes | Binary Number | Version of Host Communication API | <span style="color:grey">`0xFFFFFFFFFFFFFFFF` (Information unavailable)</span> |
                             */
    U32 maxRspPayloadSizePerChunk; /*!< | Size | Type | Short Description | Return Value |
                             *   | ----: | :---- | :---- | :---- |
                             *   | 4 Bytes | Binary Number | Maximum response payload size per chunk \n (\ref PROTOCOL_RESULT_MSG_OVERHEAD included) | Size in bytes, reported as a 32-bit number |
                             */
    U32 chipSku;            /*!< | Size | Type | Short Description | Return Value |
                             *   | ----: | :---- | :---- | :---- |
                             *   | 4 Bytes | Binary Number | Chip SKU (Stock Keeping Unit) identification: \n GW5200, GW5210, GW5300, GW5310, GW5400, GW5410 | - `0xFFFFFFFF` if information unavailable, OR \n - Otherwise, contact GEO representative for the meaning of SKU ID |
                             */
    U32 deviceName;         /*!< | Size | Type | Short Description | Return Value |
                             *   | ----: | :---- | :---- | :---- |
                             *   | 4 Bytes | Binary Number | Platform device identification: \n EVK, WarpCAM, customer | In active state: \n - One of the values in \ref DeviceName_e, OR \n - `0xFFFFFFFF` if `syscfg.device.name` is not specified \n\n In failsafe state: `0xFFFFFFFF` (Information unavailable) |
                             */
    U8  sensorID[16];       /*!< | Size | Type | Short Description | Return Value |
                             *   | ----: | :--- | :---- | :---- |
                             *   | 16 Bytes | NULL-terminating ASCII string | Sensor driver identification: \n HDMI, IMX390, AR0233, \n OS08A10, MN34430, ... etc | In active state: \n - `"HDMI"` if `syscfg.videoInput.source = HDMI`, OR \n - ASCII string of sensor driver name if `syscfg.videoInput.source = sensor` \n\n In failsafe state: `""` (Information unavailable) |
                             */
    U32 sensorRev;          /*!< | Size | Type | Short Description | Return Value |
                             *   | ----: | :--- | :---- | :---- |
                             *   | 4 Bytes | Binary Number | Connected Sensor hardware revision | <span style="color:grey">`0xFFFFFFFF` (Information unavailable)</span> |
                             */
    U64 sysFWID;            /*!< | Size | Type | Short Description | Return Value |
                             *   | ----: | :--- | :---- | :---- |
                             *   | 8 Bytes | Binary Number | System firmware build revision | Most significant 8 bytes of revision number |
                             */
    U8  sysFWBuildDate[8];  /*!< | Size | Type | Short Description | Return Value |
                             *   | ----: | :--- | :---- | :---- |
                             *   | 8 Bytes | Array of ASCII characters | System firmware build date | - Date in ASCII numerical characters in the form of yyyymmdd (e.g. ``'2' '0' '1' '7' '1' '2' '3' '1'``), OR \n - ``'N' '/' 'A' '\0' '\0' '\0' '\0' '\0'`` if information unavailable |
                             */
    U64 p5FWID;             /*!< | Size | Type | Short Description | Return Value |
                             *   | ----: | :--- | :---- | :---- |
                             *   | 8 Bytes | Binary Number | Computer vision firmware build revision | <span style="color:grey">`0xFFFFFFFFFFFFFFFF` (Information unavailable)</span> |
                             */
    U8  p5FWBuildDate[8];   /*!< | Size | Type | Short Description | Return Value |
                             *   | ----: | :--- | :---- | :---- |
                             *   | 8 Bytes | Array of ASCII characters | Computer vision firmware build date | <span style="color:grey">``'N' '/' 'A' '\0' '\0' '\0' '\0' '\0'`` (Information unavailable)</span> |
                             */
    U64 sensorBinRev;       /*!< | Size | Type | Short Description | Return Value |
                             *   | ----: | :--- | :---- | :---- |
                             *   | 8 Bytes | Binary Number | Sensor driver build revision | In active state: Most significant 8 bytes of revision number \n\n In failsafe state: `0xFFFFFFFFFFFFFFFF` (Information unavailable) |
                             */
    U8  sensorBuildDate[8]; /*!< | Size | Type | Short Description | Return Value |
                             *   | ----: | :--- | :---- | :---- |
                             *   | 8 Bytes | Array of ASCII characters |  Sensor driver build date | In active state: \n - Date in ASCII numerical characters in the form of yyyymmdd (e.g. ``'2' '0' '1' '7' '1' '2' '3' '1'``), OR \n - ``'N' '/' 'A' '\0' '\0' '\0' '\0' '\0'`` if information unavailable \n\n In failsafe state: ``'N' '/' 'A' '\0' '\0' '\0' '\0' '\0'`` (Information unavailable) |
                             */
    U64 bootloaderBinRev;   /*!< | Size | Type | Short Description | Return Value |
                             *   | ----: | :--- | :---- | :---- |
                             *   | 8 Bytes |  Binary Number | Bootloader driver build revision | In active state: Most significant 8 bytes of revision number \n\n In failsafe state: `0xFFFFFFFFFFFFFFFF` (Information unavailable) |
                             */
    U8  bootloaderBuildDate[8]; /*!< | Size | Type | Short Description | Return Value |
                             *   | ----: | :--- | :---- | :---- |
                             *   | 8 Bytes | Array of ASCII characters |  Bootloader driver build date | In active state: \n - Date in ASCII numerical characters in the form of yyyymmdd (e.g. ``'2' '0' '1' '7' '1' '2' '3' '1'``), OR \n - ``'N' '/' 'A' '\0' '\0' '\0' '\0' '\0'`` if information unavailable \n\n In failsafe state: ``'N' '/' 'A' '\0' '\0' '\0' '\0' '\0'`` (Information unavailable) |
                             */
    U8  romBuildDate[8];    /*!< | Size | Type | Short Description | Return Value |
                             *   | ----: | :--- | :---- | :---- |
                             *   | 8 Bytes | Array of ASCII characters |  ROM build date | - Date in ASCII numerical characters in the form of yyyymmdd (e.g. ``'2' '0' '1' '7' '1' '2' '3' '1'``), OR \n - ``'N' '/' 'A' '\0' '\0' '\0' '\0' '\0'`` if information unavailable |
                             */
    U8  romBuildTime[8];    /*!< | Size | Type | Short Description | Return Value |
                             *   | ----: | :--- | :---- | :---- |
                             *   | 8 Bytes | Array of ASCII characters |  ROM build time on the date | - Time in ASCII numerical characters in the form of hhmm (e.g. ``'2' '3' '5' '9' '\0' '\0' '\0' '\0'``), OR \n - ``'N' '/' 'A' '\0' '\0' '\0' '\0' '\0'`` if information unavailable |
                             */
    U32 romUserId;          /*!< | Size | Type | Short Description | Return Value |
                             *   | ----: | :--- | :---- | :---- |
                             *   | 4 Bytes | Binary Number |  An ID for user to identify a ROM. \n Also appeared as `"user_id"` field value \n in extracted ROM layout json file. | - 32-bit number, OR \n - `0xFFFFFFFF` if information unavailable |
                             */
    U8  romProgramName[20]; /*!< | Size | Type | Short Description | Return Value |
                             *   | ----: | :--- | :---- | :---- |
                             *   | 20 Bytes | NULL-terminating ASCII string |  Program name assigned by GEO. \n Also appeared as `"program_name"` field value \n in extracted ROM layout json file. | - ASCII String OR \n - `""` if information unavailable |
                             */
} Info_s;
#pragma pack(0)

#pragma pack(1)
typedef struct {
    U8 state;              /*!< corresponds to \ref SysState_e */
    U8 fatalFailcode;      /*!< corresponds to \ref Gw5FailCodeFatal_e */
    U8 failcode;           /*!< corresponds to \ref Gw5FailCode_e */
    U8 reserved0;          /*!< value to be ignored */
} SysState_s;
#pragma pack(0)

#pragma pack(1)
typedef struct {
    U32   action;          /*!< see \ref ResetAction_e for permissible values */
    U32   param;           /*!< meaning depends on \ref action */
} Reset_s;
#pragma pack(0)

#pragma pack(1)
typedef struct {
    U8    primitiveId;     /*!< Object ID. This ID is for a GPU primitive to be drawn on screen. This be a \ref VgGraphicsPrimitiveType_e element */
    U16   flags;           /*!< flags for drawing the primitive. Must be of type \ref VgGraphicsPrimitiveFlags_e */
    U32   color;           /*!< Specify color in ABGR */
    float screenX;         /*!< x position of object in screen pixels specified as a float */
    float screenY;         /*!< y position of object in screen pixels specified as a float */
    float scaleX;          /*!< x scaling of object specified as a float. A scale value of 1.f means the object is one pixel in the x dimension */
    float scaleY;          /*!< y scaling of object specified as a float. A scale value of 1.f means the object is one pixel in the y dimension */
    float lineWidth;       /*!< floating point width of the lines used to draw the object in screen pixels */
    float rotation;        /*!< floating point rotation of object in degrees. 0.f degrees is no rotation */
} DrawVgPrimitive_s;

typedef struct {
    U8 numPrimitives;      /*!< number of primitives contained in this \ref api_draw_vg_primitives command */
    DrawVgPrimitive_s graphicsPrimitives[1];
} DrawMultipleVgPrimitives_s;
#pragma pack(0)

#pragma pack(1)
typedef struct {
    U16 ssmId;                /*!< ssm ID for this graphics object. 
                                   NOTE: the object data from the SSM index may be cached in memory so it is recommended NOT to update the object data in flash at run time.
                                   Doing so can lead to a mismatch between the object that is rendered and the object stored in SNOR flash. */
    U16 transformationFlags;  /*!< flags specify fields and operations to be performed on the graphics object. These flags are of type \ref VgGraphicsTransformationFlags_e */
    U32 transformationParams[2];
} DrawVgObject_s;

typedef struct {
    U8 numObjects;      /*!< the number of graphics objects in this \ref api_draw_vg_objects command */
    DrawVgObject_s objects[1];
} DrawMultipleVgObject_s;
#pragma pack(0)

#pragma pack(1)
typedef struct {
    U32 color;  /*!< color for the fill in 32 bit ABGR */
    U16 x;      /*!< left coordinate of the fill rectangle in screen pixels */
    U16 y;      /*!< top coordinate of the fill rectangle in screen pixels */
    U16 width;  /*!< width of the fill rectangle in screen pixels */
    U16 height; /*!< height of the fill rectangle in screen pixels */
    U16 flags;  /*!< flags for drawing the colorfill. Must be of type \ref VgGraphicsColorFillFlags_e */
} DrawVgColorFill_s;

typedef struct {
    U8 numColorFills; /*!< number of color filles contained in this \ref api_draw_vg_colorfills command */
    DrawVgColorFill_s colorFills[1];
} DrawMultipleVgColorFills_s;
#pragma pack(0)

#define API_READ_ONLY_MODE_OFF_MAGIC    (0xADU)

#pragma pack(1)
typedef struct {
    U8    code;            /*!< \ref API_READ_ONLY_MODE_OFF_MAGIC : allow firmware to write to flash (disable read-only mode), any other value: re-enable read-only mode */
} ReadOnlyMode_s;
#pragma pack(0)

#pragma pack(1)
typedef struct {
    U16 posX;
    U16 posY;
    U16 width;
    U16 height;
    U32 reserved;
} ResizeGpuOsdLayer_s;
#pragma pack(0)

#pragma pack(1)
typedef struct {
    U16 ssmIndex;        /*!< SSM Index */
    U16 osdId;           /*!< OSD event/image ID as generated by OSD event manager */
    U8  imageSlotId;     /*!< OSD image slot ID */
} LoadOsdImageSlot_s;
#pragma pack(0)

#pragma pack(1)
typedef struct {
    U16 ssmIndex;        /*!< SSM Index */
    U16 osdId;           /*!< OSD event/image ID as generated by OSD event manager */
    U8  clutSlotId;      /*!< OSD CLUT slot ID */
} LoadOsdClutSlot_s;
#pragma pack(0)

#pragma pack(1)
typedef struct {
    U8 channel;         /*!< OSD channel index, 0 ... (\ref GW5_OSD_NUM_CHANNELS - 1) */
    U8 enable;
    U8 alpha;
    U8 pad[1];          /*!< Padding (set to 0x00) */
    U16 offsetX;
    U16 offsetY;
    U8 imageSlotId;     /*!< OSD image slot ID */
    U8 clutSlotId;      /*!< OSD CLUT slot ID */
} ConfigOsdChannel_s;
#pragma pack(0)

#pragma pack(1)
typedef struct {
    U8    binaryType;      /*!< see \ref Binary_e for permissible values */
    U8    pad[3];
    U32   binaryLen;       /*!< size to erase in flash (rounded up to next 32kB). Set to 0 to default to the full allocation of \ref binaryType */
} FlashBinaryInfo_s;
#pragma pack(0)

#pragma pack(1)
typedef struct {
    FlashBinaryInfo_s binInfo;      /*!< describes this flash binary being written/read */
    U8   *data;
} Binary_s;
#pragma pack(0)

#pragma pack(1)
typedef struct {
    U8    inputPortId;     /*!< Raw capture input port: 0 - Mipi port 0, 1 - Mipi port 1 */
    U8    exposureId;      /*!< Sensor exposure to capture: 0, 1, 2, 3 - depending on sensor */
    U16   offsetX;         /*!< Capture image horizontal start offset in pixels */
    U16   offsetY;         /*!< Capture image vertical start offset in pixels */
    U16   width;           /*!< Capture image width in pixels */
    U16   height;          /*!< Capture image height in pixels */
} RawImageInfo_s;
#pragma pack(0)

#pragma pack(1)
typedef struct {
    U16   width;           /*!< Width of raw capture */
    U16   height;          /*!< Height of raw capture */
    U8    bitDepth;        /*!< Bit depth: 8, 10, 12, 14, 16, 20 */
    U8    endian;          /*!< Endianness: 0 - little endian, 1 - big endian \ref Endianness_e */
    U8    reserved[2];
} RawImageHdr_s;
#pragma pack(0)

#pragma pack(1)
typedef struct {
    RawImageHdr_s hdr;
    U8   *data;            /*!< Data array */
} RawImageData_s;
#pragma pack(0)

#pragma pack(1)
typedef struct {
    U8    enable;          /*!< 0 - disable pattern, 1 - enable pattern */
    U8    patternType;     /*!< Isp pattern type \ref IspTestPatternType_e */
} IspTestPattern_s;
#pragma pack(0)

#pragma pack(1)
typedef struct {
    U8    bytes[8];        /*!< The least significant byte will be byte 0 and data are valid up to the size of the parameter requested */
} CfgParamValue_s;
#pragma pack(0)


#pragma pack(1)
typedef struct {
    U32   specID;          /*!< See \ref JsonSpecId_e for permissible values. Operation may not be permitted for some JSON spec types */
    U32   paramID;         /*!< "id" value of array parameter specified in the respective cfg_spec.json in ../configuration/ */
    U32   rowIdx;          /*!< Specify the row index of the array parameter */
} CfgArrayParamRequest_s;
#pragma pack(0)

#pragma pack(1)
typedef struct {
    U32   specID;          /*!< See \ref JsonSpecId_e for permissible values. Operation may not be permitted for some JSON spec types */
    U32   paramID;         /*!< "id" value of parameter specified in the respective cfg_spec.json in ../configuration/ */
} CfgParamRequest_s;
#pragma pack(0)

#pragma pack(1)
typedef struct {
    U16   paramId;         /*!< "id" value of parameter specified in the respective cfg_spec.json in ../configuration/ */
    U16   numRow;          /*!< Number of rows in array. Should be set to 1 for non-array type. */
    U16   rowDataSize;     /*!< Data size of single row element */
    U8    data[];
} CfgSingleParam_s;
#pragma pack(0)

#pragma pack(1)
typedef struct {
    U8    specId;          /*!< See \ref JsonSpecId_e for permissible values. Operation may not be permitted for some JSON spec types */
    U8    apply;           /*!< set to 0 to only update the parameter values, but not apply them. 
                             set to 1 to update THEN APPLY the new parameter values (only valid if \ref specId = \ref json_spec_id_warpcfg) */
    U8    params[];        /*!< Zero or more of \ref CfgSingleParam_s */
} CfgParamUpdate_s;
#pragma pack(0)

#pragma pack(1)
typedef struct {
    U32   flashOffset;     /*!< Absolute flash offset to write table to */
    U8    specId;          /*!< See \ref JsonSpecId_e for permissible values */
    U8    pad[1];
    U8    params[];        /*!< Zero or more of \ref CfgSingleParam_s */
} CfgParamTblCreateList_s;
#pragma pack(0)

#pragma pack(1)
typedef struct {
    // AE
    float totalGain[4];
    float sensorGain[4];
    float ispGain[4];
    float unclippedEv[4];
    float unclippedEvAvg;
    U16   integrationLines[4];
    U16   vts;
    U8    padAe[2];     /*!< Pad bytes to have 32-bit alignment for AE section */
    // Sensor
    float frameRate;
    float sensorAnalogGain;
    float sensorDigitalGain;
    float sensorDualConversionGain;
    float sensorTemperature;
    U8    numberOfSensors;
    U8    padSensor[3];  /*!< Pad bytes to have 32-bit alignment for Sensor section */
    // AWB
    U16   awbGains[4];
    U32   awbColorTemperature;
    U32   awbRGRatio;
    U32   awbBGRatio;
    // Combiner
    U16   longMediumRatio;
    U16   mediumShortRatio;
    U16   shortVeryShortRatio;
    U8    padCombiner[2];  /*!< Pad bytes to have 32-bit alignment for Combiner section */
    // CCM
    S16   colorMatrix[9];
    U8    padCcm[2];       /*!< Pad bytes to have 32-bit alignment for CCM section */
    // LTM
    U16   ltmGain;
    U8    padLtm[2];       /*!< Pad bytes to have 32-bit alignment for LTM section */
    // Black Levels
    U16   blackOffsetR;
    U16   blackOffsetGR;
    U16   blackOffsetGB;
    U16   blackOffsetB;
    // Histogram
    float histAvg12bLIrDsp;
    float histAvg12bLGrDsp;
    float histAvg12bLGrIspPrc;
    // Lux Estimation
    float luxEstimation;
} IspLiveParams_s;
#pragma pack(0)

#pragma pack(1)
typedef struct {
    U8    manufactureID;
    U8    deviceID;
    U16   skuID;
} FlashProductID_s;
#pragma pack(0)


#pragma pack(1)
typedef struct {
    U32   flashId;         /*!< If \ref flashIdType is \ref ssm_index, this is an SSM index; 
                                if \ref flashIdType is \ref flash_offset, this is an absolute flash offset */
    U8    flashIdType;     /*!< See \ref FlashIdType_e for permissible values */
} FlashTableLocation_s;
#pragma pack(0)

#pragma pack(1)
typedef struct {
    FlashTableLocation_s tableLoc;         /*!< Describes the location of the flash table being written/read */
    U8    pad[1];
    U16   payloadType;     /*!< See \ref FlashPayloadType_e for permissible values */
    U32   payloadLen;
} FlashTableInfo_s;
#pragma pack(0)

#pragma pack(1)
typedef struct {
    FlashTableInfo_s tableInfo;     /*!< Describes this flash table being written/read */
    U8   *data;
} FlashTable_s;
#pragma pack(0)

/*!
 * \brief \ref flashOffset + \ref len must not be greater than the total size of the flash part.
 */
#pragma pack(1)
typedef struct {
    U32   flashOffset;     /*!< Absolute flash offset at which to begin CRC calculation */
    U32   len;             /*!< length (in bytes) of area upon which to calculate CRC */
} FlashCrcRequest_s;
#pragma pack(0)

#pragma pack(1)
typedef struct {
    U8    bpBits;          /*!< Value to write to SNOR flash Status Register's Block Protect bits.  
                                Only bits [5:2] are valid, that is, the mask 0x3C will be applied to this value */
} FlashHwProtect_s;
#pragma pack(0)

#pragma pack(1)
typedef struct {
    U8    assert;          /*!< 0 to deassert flash WP# pin (enable writes to flash Status Register), 1 to assert (disable SR writes) */
} FlashWpnControl_s;
#pragma pack(0)

#pragma pack(1)
typedef struct {
    U8    registerCount;    /*!< Valid values: 1 to \ref MAX_REGISTERS_TO_READ */
    U32   registerOffsets[MAX_REGISTERS_TO_READ];   /*!< Placeholder for an array of U32. May contain 1 to \ref MAX_REGISTERS_TO_READ of U32 whereas the number of U32 matches \ref registerCount */
} RegisterRead_s;
#pragma pack(0)

#pragma pack(1)
typedef struct {
    U32   registerOffset;
    U32   registerValue;
} RegisterOffsetAndValue_s;
#pragma pack(0)

#pragma pack(1)
typedef struct {
    U8    registerCount;    /*!< Valid values: 1 to \ref MAX_REGISTERS_TO_READ*/
    RegisterOffsetAndValue_s   result[MAX_REGISTERS_TO_READ];   /*!< Placeholder for an array of \ref RegisterOffsetAndValue_s. May contain 1 to \ref MAX_REGISTERS_TO_READ of \ref RegisterOffsetAndValue_s whereas the number of \ref RegisterOffsetAndValue_s matches \ref registerCount */
} RegisterReadResult_s;
#pragma pack(0)

#pragma pack(1)
typedef struct {
    U8    vFlip;           /*!< 0: No flipping; 1: Vertical flipping enabled */
    U8    hFlip;           /*!< 0: No flipping; 1: Horizontal flipping enabled */
} SensorFlip_s;
#pragma pack(0)

#pragma pack(1)
typedef struct {
    U32   xOffset;
    U32   yOffset;
} SensorOffset_s;
#pragma pack(0)

#pragma pack(1)
typedef struct {
    U32   frameInputCount;
    U32   frameOutputCount;
    U16   inputStall;      /*!< Input stall counter */
    U16   outputStall;     /*!< Output stall counter */
} VideoInputStatus_s;
#pragma pack(0)

#pragma pack(1)
typedef struct {
    U8    slaveAddr;       /*!< I2C slave device 8-bit address. Least significant bit (R/W bit) is ignored */
    U8    port;            /*!< I2C port. Valid values: 0, 1 */
    U8    pad[2];
    U32   addr;            /*!< I2C sub-address/offset/register value. Only the least significant \ref addrValidLen bytes will be used*/
    U8    addrValidLen;    /*!< valid values: 1-4*/
    U8    dataValidLen;    /*!< Specifies the valid lowest significant bytes of U32 Query Response; valid values: 1-4*/
} I2cSlaveRead_s;
#pragma pack(0)

#pragma pack(1)
typedef struct {
    U16   processHVT;      /*!< Raw register value */
    U16   processSVT;      /*!< Raw register value */
    float voltage;         /*!< In volts */
    float temperature;     /*!< In Celsius */
} PVTValues_s;
#pragma pack(0)



#pragma pack(1)
typedef struct {
    U8    evalApiCode;     /*!< An API code in \ref EvalAPICode_e */
    U8    pad[3];
    U8    payload[];       /*!< API payload structure (dependent on value of \ref evalApiCode); see list in \ref EvalAPICode_e */
} EvalApi_s;
#pragma pack(0)


#pragma pack(1)
typedef struct {
    U8    slaveAddr;       /*!< I2C slave device 8-bit address. Least significant bit (R/W bit) is ignored */
    U8    port;            /*!< I2C port. Valid values: 0, 1 */
    U8    pad[2];
    U32   addr;            /*!< I2C sub-address/offset/register value. Only the least significant \ref addrValidLen bytes will be used*/
    U32   data;            /*!< Only the least significant \ref dataValidLen bytes will be used*/
    U8    addrValidLen;    /*!< valid values: 1-4*/
    U8    dataValidLen;    /*!< valid values: 1-4*/
} I2cSlaveWrite_s;
#pragma pack(0)

#pragma pack(1)
typedef struct {
    U32 id;    /*!< Identifier in either standard or extended format */
    U8 ide;    /*!< Identifier Extension. 0-Standard Format: ID(10:0), 1-Extended Format: ID(28:0)  */
    U8 length; /*!< Message length - maximum can be 64 for CAN FD */
    U8 edl;    /*!< Extended Data Length. 0-CAN 2.0 frame (up to 8 bytes long), 1-CAN FD frame (up to 64 bytes long) */
    U8 brs;    /*!< Bit Rate Switch. 0-nominal/slow bit rate for the complete frame, 1-switch to data/fast bit rate for the data payload and the CRC */
    U8 data[64];
} CanWrite_s;
#pragma pack(0)

#pragma pack(1)
typedef struct {
    U32 id;    /*!< Identifier in either standard or extended format */
    U8 ide;    /*!< Identifier Extension. 0-Standard Format: ID(10:0), 1-Extended Format: ID(28:0)  */
    U8 length; /*!< Message length - maximum can be 64 for CAN FD */
    U8 edl;    /*!< Extended Data Length. 0-CAN 2.0 frame (up to 8 bytes long), 1-CAN FD frame (up to 64 bytes long) */
    U8 brs;    /*!< Bit Rate Switch. 0-nominal/slow bit rate for the complete frame, 1-switch to data/fast bit rate for the data payload and the CRC */
    U8 data[64];
} CanReadResponse_s;
#pragma pack(0)

/*!
 * \brief A structure for the command payload of \ref eval_api_get_targetless_autocal_result.
 */
#pragma pack(1)
typedef struct {
    U8  loopControl;            /*!< Number of iterations that AutoCAL runs for: 1 to 128 */
    U8  disableYawCorrection;   /*!< 0: Return correction matrix as is;
                                 *   1: Zero out yaw correction (i.e. rotation around y-axis) and return the resultant correction matrix
                                 */
} TargetlessAutoCalCmd_s;
#pragma pack(0)

/*!
 * \brief A structure for the response payload of \ref eval_api_get_targetless_autocal_result.
 */
#pragma pack(1)
typedef struct {
    float correctionMatrix[3][3];   /*!< For executing the adjustment, please copy the result to 
                                     *   InputWarpAdjWmFromAndToFlash_s#correctionMatrix and execute the
                                     *   \ref eval_api_input_warp_adjust_warpmap_from_and_to_flash command.
                                     */
} TargetlessAutoCalResult_s;
#pragma pack(0)

/*!
 * \brief A structure for the command payload of \ref eval_api_input_warp_adjust_warpmap_from_and_to_flash
 *
 * Please contact your GEO representative for more details about the fields of InputWarpAdjWmFromAndToFlash_s 
 * (\ref eval_api_input_warp_adjust_warpmap_from_and_to_flash is for evaluation purpose and subject to changes).
 */
#pragma pack(1)
typedef struct {
    float correctionMatrix[3][3];   /*!< For adjustment with the result from 
                                     * \ref eval_api_get_targetless_autocal_result.
                                     * Or set to identity matrix for no transformation.
                                     */
    float rotateX;                  /*!< Rotation around the X-axis. In terms of degrees. Set to 0 for no rotation. */
    float rotateY;                  /*!< Rotation around the Y-axis. In terms of degrees. Set to 0 for no rotation. */
    float rotateZ;                  /*!< Rotation around the Z-axis (i.e. rotateZ alone is the 2D rotation of the XY-plane).
                                     *   In terms of degrees. Set to 0 for no rotation.
                                     */
    float pre3dAdjShiftX;           /*!< In terms of pixels. Set to 0 for no shifting. */
    float pre3dAdjShiftY;           /*!< In terms of pixels. Set to 0 for no shifting. */
    float pre3dAdjScaleX;           /*!< Set to 1 for no scaling. Must not be greater than 1. */
    float pre3dAdjScaleY;           /*!< Set to 1 for no scaling. Must not be greater than 1. */
    U16 ssmIndexForSrcWm;           /*!< SSM index to read source warpmap */
    U16 ssmIndexForDstWm;           /*!< SSM index to store adjusted warpmap */
} InputWarpAdjWmFromAndToFlash_s;
#pragma pack(0)

/*!
 * \brief A structure for the command payload of \ref eval_api_output_warp_adjust_current_warpmap
 *
 * Please contact your GEO representative for more details about the fields of OutputWarpAdjCurrentWm_s 
 * (\ref eval_api_output_warp_adjust_current_warpmap is for evaluation purpose and subject to changes).
 */
#pragma pack(1)
typedef struct {
    U16 ssmIndexForSrcWm;           /*!< If set to 0xFFFF, warpmap is read from the warpmap software buffer.
                                     *   Otherwise, warpmap is read from specified SSM slot and copied
                                     *   to the warpmap software buffer.
                                     *   Centering, rotation, shifting, and scaling are then applied on the output space of the warpmap read.
                                     *   The centered/rotated/shifted/scaled warpmap is then loaded to eWarp hardware engine.
                                     *   The warpmap software buffer is available only in certain ROM(s).
                                     */
    float centerX;                  /*!< X-centering of output warp. In terms of pixels. Suggest setting to the center X-coordinate of original output resolution. */
    float centerY;                  /*!< Y-centering of output warp. In terms of pixels. Suggest setting to the center Y-coordinate of original output resolution. */
    float rotateZ;                  /*!< Output warp rotation of the XY-plane (i.e. 2D) around \ref centerX and \ref centerY. In terms of degrees. Set to 0 for no rotation. */
    float shiftX;                   /*!< Output warp shifting (panning) along X-dimension. In terms of pixels. Set to 0 for no shifting */
    float shiftY;                   /*!< Output warp shifting (panning) along Y-dimension. In terms of pixels. Set to 0 for no shifting */
    float scaleX;                   /*!< Output warp scaling (zooming-in) along X-dimension. Set to 1 for no scaling. Must not be greater than 1. */
    float scaleY;                   /*!< Output warp scaling (zooming-in) along Y-dimension. Set to 1 for no scaling. Must not be greater than 1. */
} OutputWarpAdjCurrentWm_s;
#pragma pack(0)


#pragma pack(1)
typedef struct {
    U8 failCode;    /*!< Failure code, \ref Gw5FailCode_e */
    U8 clearStatus; /*!< If non-zero, clear status of this failure after reading */
} GetClearFailureStatus_s;
#pragma pack(0)

#pragma pack(1)
typedef struct {
    U8 channelId; /*!< Video channel ID, 0 ... 1 */
    U8 isEnable; /*!< 0 to disable cropping, enable otherwise */
    U16 offsetX; /*!< Horizontal offset, pixels */
    U16 offsetY; /*!< Vertical offset, pixels */
    U16 width; /*!< Cropped area width, pixels */
    U16 height; /*!< Cropped area height, pixels */
} CropOutputVideo_s;
#pragma pack(0)

#pragma pack(1)
typedef struct {
    U8 clutOverrideEntryId;             /*!< ID OSD CLUT override entry that will be modified by this structure */
    U8 clutSlotId;                      /*!< ID of OSD slot containing the OSD CLUT to set to the OSD layer */
    U16 srcClutEntryIndex;              /*!< Starting index of the entry in the CLUT slot to use as the source of the override */
    U16 destClutEntryIndex;             /*!< Starting index of the CLUT entry to override */
    U16 numEntries;                     /*!< Number of CLUT entries to override */
} OverrideOsdClutEntry_s;
#pragma pack(0)

#pragma pack(1)
typedef struct {
    U8 channel;                         /*!< OSD channel to which this override applies */
    U8 isClearing;                      /*!< if 0, the \ref numOverrideEntries CLUT override entries in \ref entries[] are applied
                                             if 1, this OSD channel's CLUT overrides are cleared */
    U8 numOverrideEntries;              /*!< number of elements in \ref entries[] (must be 0 if \ref isClearing is 1) */
    OverrideOsdClutEntry_s entries[];   /*!< 0 to \ref GW5_OSD_NUM_CLUT_OVERRIDES elements */
} OverrideOsdChannelClut_s;
#pragma pack(0)

#pragma pack(1)
typedef struct {
    U8 isUpdatingEnable;                /*!< if 0, the ConfigOsdChannel_s#enable field in \ref channelConfig is ignored */
    U8 isUpdatingAlpha;                 /*!< if 0, the ConfigOsdChannel_s#alpha field in \ref channelConfig is ignored */
    U8 isUpdatingOffset;                /*!< if 0, the ConfigOsdChannel_s#offsetX and ConfigOsdChannel_s#offsetY fields in \ref channelConfig are ignored */
    U8 isUpdatingImageSlot;             /*!< if 0, the ConfigOsdChannel_s#imageSlotId field in \ref channelConfig is ignored (image slot is not modified) */
    U8 isUpdatingClutSlot;              /*!< if 0, the ConfigOsdChannel_s#clutSlotId field in \ref channelConfig is ignored (CLUT slot is not modified) */
    ConfigOsdChannel_s channelConfig;   /*!< structure describing OSD channel configuration to apply */
} UpdateOsdChannel_s;
#pragma pack(0)

#pragma pack(1)
typedef struct {
    U8 numOsdChannelConfigs;            /*!< 0 to \ref GW5_OSD_NUM_CHANNELS */
    U8 numOsdChannelClutOverrides;      /*!< 0 to \ref GW5_OSD_NUM_CHANNELS */
    U8 reserved[14];                    /*!< must be filled with 0 */
    U8 data[];                          /*!< this variable-length buffer should contain, in order: 
                                             \ref numOsdChannelConfigs elements       of \ref UpdateOsdChannel_s 
                                             \ref numOsdChannelClutOverrides elements of \ref OverrideOsdChannelClut_s */
} FrameSyncOp_s;
#pragma pack(0)

#pragma pack(1)
typedef struct {
    U8 alpha; /*!< Alpha value. */
    U8 redCr; /*!< Red or Cr component value in RGB or YCbCr format respectively. */
    U8 blueCb; /*!< Blue or Cb component value in RGB or YCbCr format respectively. */
    U8 greenY; /*!< Green or Y component value in RGB or YCbCr format respectively. */
} ClutTableEntry_s;
#pragma pack(0)

#pragma pack(1)
typedef struct {
    U8 clutSlotId; /*!< CLUT slot ID. */
    U16 nClutEntries; /*!< Number of CLUT entries. Valid values: 1 to \ref GW5_OSD_MAX_CLUT_ENTRIES */
    ClutTableEntry_s tableEntries[]; /*!< 1 to \ref GW5_OSD_MAX_CLUT_ENTRIES array elements. */
} LoadOsdClut_s;
#pragma pack(0)

/*******************************************************************************
 Exported Variable Declarations
*******************************************************************************/


/*******************************************************************************
 Inline Function Definitions
*******************************************************************************/


/*******************************************************************************
 Global Scope Function Prototypes
*******************************************************************************/

#endif // __API_H__

/*! @} */


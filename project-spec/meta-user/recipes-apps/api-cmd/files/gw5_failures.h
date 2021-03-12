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

#ifndef __GW5_FAILURES_H__
#define __GW5_FAILURES_H__

typedef enum {
    gw5_failcode_init                           = 0,  /*!< System initialization failure */
    gw5_failcode_configuration                  = 1,  /*!< Invalid configuration */
    gw5_failcode_fw                             = 2,  /*!< Internal firmware failure */
    gw5_failcode_voltage                        = 3,  /*!< VCORE voltage is out of safe range */
    gw5_failcode_high_temperature               = 4,  /*!< Die temperature exceeded threshold */
    gw5_failcode_imgsensor_access               = 5,  /*!< Image sensor driver is not able to access sensor through I2C */
    gw5_failcode_imgsensor_init                 = 6,  /*!< Image sensor initialization failure */
    gw5_failcode_imgsensor_hw                   = 7,  /*!< Image sensor hardware failure */
    gw5_failcode_imgsensor_not_found            = 8,  /*!< Cannot detect image sensor */
    gw5_failcode_imgsensor_disconnected         = 9,  /*!< Image sensor was disconnected */
    gw5_failcode_no_input_video                 = 10, /*!< No input video to GW5 */
    gw5_failcode_input_video_frozen             = 11, /*!< Input video to GW5 froze */
    gw5_failcode_no_output_video                = 12, /*!< No output video from GW5 */
    gw5_failcode_output_video_frozen            = 13, /*!< Output video from GW5 froze */
    gw5_failcode_video_pipeline_stalled         = 14, /*!< Video data is not flowing through GW5 video pipeline */
    gw5_failcode_video_pipeline_hw              = 15, /*!< GW5 video pipeline hardware failure */
    gw5_failcode_mipi_receiver                  = 16, /*!< MIPI receiver failure */
    gw5_failcode_i2c0_ctrl_hw                   = 17, /*!< I2C0 controller hardware failure */
    gw5_failcode_i2c1_ctrl_hw                   = 18, /*!< I2C1 controller hardware failure */
    gw5_failcode_can_ctrl_hw                    = 19, /*!< CAN controller hardware failure */
    gw5_failcode_spi0_ctrl_hw                   = 20, /*!< SPI0 controller hardware failure */
    gw5_failcode_spi1_ctrl_hw                   = 21, /*!< SPI1 controller hardware failure */
    gw5_failcode_spi_slave_ctrl_hw              = 22, /*!< SPI slave controller hardware failure */
    gw5_failcode_peripheral_device_hw           = 23, /*!< Peripheral device hardware failure */
    gw5_failcode_iqs_overexposure_detected      = 24, /*!< ImageQualitySafety overexposure detected */
    gw5_failcode_iqs_underexposure_detected     = 25, /*!< ImageQualitySafety underexposure detected */
    gw5_failcode_serializer_access              = 26, /*!< Serializer read / write failure */
    gw5_failcode_deserializer_access            = 27, /*!< Deserializer read / write failure */
    gw5_failcode_iqs_low_contrast_detected      = 28, /*!< ImageQualitySafety low contrast detected */
    gw5_failcode_iqs_color_tint_detected        = 29, /*!< ImageQualitySafety color tint detected */

    gw5_failcodes_n_total,
    gw5_failcode_none                   = 255
} Gw5FailCode_e;

typedef enum {
    gw5_failcode_fatal_cpu_exception    = 0,  /*!< GW5 CPU exception occurred */
    gw5_failcode_fatal_watchdog         = 1,  /*!< Watchdog timeout caused GW5 reset */
    gw5_failcode_fatal_oskernel         = 2,  /*!< RTOS kernel reported an unrecoverable failure */
    gw5_failcode_fatal_fw_corrupted     = 3,  /*!< Firmware image integrity check failed */
    gw5_failcode_fatal_failsafe_forced  = 4,  /*!< Failsafe boot has been forced */

    gw5_failcodes_fatal_n_total,
    gw5_failcode_fatal_none             = 255
} Gw5FailCodeFatal_e;

#pragma pack(1)
typedef struct {
    uint64_t firstOccurOsTick; /*!< OS tick of first failure occurrence */
    uint64_t lastOccurOsTick;  /*!< OS tick of last failure occurrence */
    uint32_t occurCount;       /*!< Occurrence counter since \ref firstOccurOsTick */
} Gw5FailureStatus_s;
#pragma pack(0)

#define GW5_FAILURE_FLAGS_N_BYTES ((gw5_failcodes_n_total + 7) / 8)

#pragma pack(1)
typedef struct {
    uint8_t flags[GW5_FAILURE_FLAGS_N_BYTES]; /*!< Per-failure flags. Bitmask representation of which failure has occurred.
                                               *   Bit position is governed by the \ref Gw5FailCode_e value of that failure.
                                               *   (e.g. Bit 0 of \ref flags[] is set to binary `1` if the failure with 
                                               *   \ref Gw5FailCode_e value 0 has occurred, ... etc. All bits are bitwise-OR'ed.)
                                               *   A bit remains set until it is cleared by an API call.
                                               */
} Gw5FailCodeList_s;
#pragma pack(0)

/*!
 * Retrieve status flag of particular failure code
 *
 * \param[in]    failureListPtr  Gw5FailCodeList_s
 * \param[in]    failCode        Failure code
 *
 * \return       none
 */
#define GW5_FAILURE_IS_SET(failureListPtr, failCode) \
    (!!((failureListPtr)->flags[failCode / 8] & (1 << (failCode % 8))))

#endif // __GW5_FAILURES_H__

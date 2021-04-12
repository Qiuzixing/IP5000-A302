/******************************************************************************
 *
 * Copyright 2013, Silicon Image, Inc.  All rights reserved.
 * No part of this work may be reproduced, modified, distributed, transmitted,
 * transcribed, or translated into any language or computer format, in any form
 * or by any means without written permission of
 * Silicon Image, Inc., 1140 East Arques Avenue, Sunnyvale, California 94085
 *
 *****************************************************************************/
/**
 * @file si_drv_adapter.h
 *
 * @brief HDCP 1.x/2.x to HDCP 2.x/1.x adapter. Common API for both Transmitter and Receiver.
 *
 *****************************************************************************/


#ifndef SI_DRV_ADAPTER_H
#define SI_DRV_ADAPTER_H

#ifdef __cplusplus
extern "C"{
#endif

/***** #include statements ***************************************************/
#include "si_drv_adapter_platform_api.h"

/***** public macro definitions **********************************************/
#define SII_DRV_ADAPT_DEFAULT_I2C_DEVICE_ID             0x70
#define SII_DRV_ADAPT_DEFAULT_I2C_DEVICE_ID_BASE        0x60
#define SII_DRV_ADAPT_DEFAULT_I2C_DEVICE_ID_F0          0xF0
#define SII_DRV_ADAPT_DEFAULT_I2C_DEVICE_ID_AA          0xAA

/*****************************************************************************/
/** @defgroup SII_DRV_ADAPT_EVENT Adapter driver events
* @brief Events from Adapter Driver
*
* These notifications are passed through a parameter of the callback function
* when corresponding status is changed.
*
******************************************************************************/
/* @{ */
#define SII_DRV_ADAPT_EVENT__BOOT_DONE              0x00000001 //!< Booting is done.
        //!< @see SiiDrvAdaptBootStatus_t, SiiDrvAdaptBootStatusGet()
#define SII_DRV_ADAPT_EVENT__ACCESS_DONE            0x00000002 //!< Memory read/write/update is done.
        //!< @see SiiDrvAdaptAccessStatus_t, SiiDrvAdaptAccessStatusGet(),
        //!< @see SiiDrvAdaptBootCfgNvramWrite, SiiDrvAdaptBootCfgNvramRead(),
        //!< @see SiiDrvAdaptEdidSramWrite, SiiDrvAdaptEdidSramRead(),
        //!< @see SiiDrvAdaptDsEdidRead()
#define SII_DRV_ADAPT_EVENT__TX_HDCP_STATUS_CHNG    0x00000004 //!< Downstream HDCP status changed.
        //!< @see SiiDrvAdaptHdcpTxStatus_t, SiiDrvAdaptTxHdcpDsStatusGet()
#define SII_DRV_ADAPT_EVENT__CKDT_STATUS_CHNG       0x00000008 //!< RX TMDS clock detection status changed.
        //!< @see SiiDrvAdaptCkdtStatusGet()
#define SII_DRV_ADAPT_EVENT__SCDT_STATUS_CHNG       0x00000010 //!< RX TMDS Sync detection status changed.
        //!< @see SiiDrvAdaptScdtStatusGet()
#define SII_DRV_ADAPT_EVENT__RX_AV_MUTE_STATUS_CHNG 0x00000020 //!< RX AV Mute status changed.
        //!< @see SiiDrvAdaptRxAvMuteStatusGet()
#define SII_DRV_ADAPT_EVENT__RX_PLUS_5V_STATUS_CHNG 0x00000040 //!< RX +5 V status changed.
        //!< @see SiiDrvAdaptRxPlus5vStatusGet()
#define SII_DRV_ADAPT_EVENT__DS_CONNECT_STATUS_CHNG_ON 0x00000080 //!< Downstream connection status changed.
        //!< @see SiiDrvAdaptDsConnectStatusGet()
#define SII_DRV_ADAPT_EVENT__DS_EDID_STATUS_CHNG    0x00000100 //!< Downstream EDID availability status changed.
        //!< @see SiiDrvAdaptDsEdidStatusGet()
#define SII_DRV_ADAPT_EVENT__DS_CONNECT_STATUS_CHNG_OFF         0x00000200 //!< Cable connection changed.
        //!< @see SiiDrvAdaptMhlLinkStateGet()
#define SII_DRV_ADAPT_EVENT__VBUS_REQUEST           0x00000400 //!< Request to provide/remove VBUS power.
        //!< @see SiiDrvAdaptVbusRequestGet()
#define SII_DRV_ADAPT_EVENT__CBUS_EVENT             0x00000800 //!< CBUS event.
        //!< @see SiiDrvAdaptCbusEventGet()
#define SII_DRV_ADAPT_EVENT__CBUS_STATE_CHNG        0x00001000 //!< CBUS status changed.
        //!<
#define SII_DRV_ADAPT_EVENT__RX_HDCP_STATUS_CHNG    0x00002000
        //!<@see SiiDrvAdaptRxCdSenseGet()
#define SII_DRV_ADAPT_EVENT__RX_CD_SENSE_STATUS_CHG 0x00004000 //!< CD SENSE status changed
        //!<@see SiiDrvAdaptRxLinkTypeGet()
#define SII_DRV_ADAPT_EVENT__US_CONNECT_STATUS_CHNG 0x00008000 //!< upstream connect changed
        //!<@SiiDrvAdaptMhlVersionGet()
#define SII_DRV_ADAPT_EVENT__MHL_VER_CHNG           0x00010000 //!< MHL version changed
        //!<@SiiDrvAdaptCbusModeGet()
#define SII_DRV_ADAPT_EVENT__CBUS_MODE_CHNG         0x00020000 //!< CBUS mode changed
        //!<@SiiDrvAdaptTxRxSenseStatusGet()
#define SII_DRV_ADAPT_EVENT__RX_SENSE_STATUS_CHNG   0x00040000 //!< RxSense changed

#define SII_DRV_ADAPT_EVENT__MHL_MASK               (SII_DRV_ADAPT_EVENT__VBUS_REQUEST \
                                                        |SII_DRV_ADAPT_EVENT__CBUS_EVENT \
                                                        |SII_DRV_ADAPT_EVENT__CBUS_STATE_CHNG)

/* @} */

#define SII_DRV_ADAPT_HDCP_EVENT__TX_AUTH_DONE      0x00000001
#define SII_DRV_ADAPT_HDCP_EVENT__TX_RCVID_CHG      0x00000002
#define SII_DRV_ADAPT_HDCP_EVENT__TX_SMNG_DONE      0x00000004
#define SII_DRV_ADAPT_HDCP_EVENT__TX_STATUS_CHG     0x00000008
#define SII_DRV_ADAPT_HDCP_EVENT__TX_AUTH_FAIL      0x00000010

#define SII_DRV_ADAPT_HDCP_EVENT__RX_AUTH_START     0x00000020
#define SII_DRV_ADAPT_HDCP_EVENT__RX_AUTH_DONE      0x00000040
#define SII_DRV_ADAPT_HDCP_EVENT__RX_RCVID_DONE     0x00000080
#define SII_DRV_ADAPT_HDCP_EVENT__RX_SMNG_CHG       0x00000100
#define SII_DRV_ADAPT_HDCP_EVENT__RX_AUTH_FAIL      0x00000200

#define SII_DRV_ADAPT_HDCP_EVENT__TX_MASK           (SII_DRV_ADAPT_HDCP_EVENT__TX_AUTH_DONE \
                                                        |SII_DRV_ADAPT_HDCP_EVENT__TX_RCVID_CHG \
                                                        |SII_DRV_ADAPT_HDCP_EVENT__TX_SMNG_DONE \
                                                        |SII_DRV_ADAPT_HDCP_EVENT__TX_AUTH_FAIL)
#define SII_DRV_ADAPT_HDCP_EVENT__RX_MASK           (SII_DRV_ADAPT_HDCP_EVENT__RX_AUTH_START \
                                                        |SII_DRV_ADAPT_HDCP_EVENT__RX_AUTH_DONE \
                                                        |SII_DRV_ADAPT_HDCP_EVENT__RX_RCVID_DONE \
                                                        |SII_DRV_ADAPT_HDCP_EVENT__RX_SMNG_CHG \
                                                        |SII_DRV_ADAPT_HDCP_EVENT__RX_AUTH_FAIL)

/***** public type definitions ***********************************************/

/**
* @brief Boot-loader status
*/
typedef enum
{
    SII_DRV_ADAPTER_BOOTING__SUCCESS,     //!< Booting is done successfully
    SII_DRV_ADAPTER_BOOTING__IN_PROGRESS, //!< Booting is in progress
    SII_DRV_ADAPTER_BOOTING__FAILURE,     //!< Booting failed
} SiiDrvAdaptBootStatus_t;

/**
* @brief Firmware upload, EDID, and NVM read/write status
*/
typedef enum
{
    SII_DRV_ADAPTER_ACCESS__SUCCESS,     //!< Reading/writing is done successfully
    SII_DRV_ADAPTER_ACCESS__IN_PROGRESS, //!< Reading/writing is in progress
    SII_DRV_ADAPTER_ACCESS__FAILURE,     //!< Reading/writing failed
} SiiDrvAdaptAccessStatus_t;

/**
* @brief Interrupt pin configuration: push-pull versus open drain
*/
typedef enum
{
    SII_DRV_ADAPTER_INT__OPENDRAIN,      //!< Interrupt pin is an "open drain"
    SII_DRV_ADAPTER_INT__PUSHPULL        //!< Interrupt pin is "push-pull"
} SiiInterruptOutputType_t;

/**
* @brief Interrupt pin polarity
*/
typedef enum
{
    SII_DRV_ADAPTER_INT__POL_LOW, //!< Interrupt pin is low when the interrupt occurs
    SII_DRV_ADAPTER_INT__POL_HIGH //!< Interrupt pin is high when the interrupt occurs
} SiiInterruptPolarity_t;

/**
* @brief Output mute type
*/
typedef enum
{
    SII_DRV_ADAPTER_MUTE__OFF,           //!< Neither Clear AV Mute nor Set AV Mute are send
    SII_DRV_ADAPTER_MUTE__AV_MUTE_OFF,   //!< Send Clear AV Mute packet
    SII_DRV_ADAPTER_MUTE__AV_MUTE_ON,    //!< Send Set AV Mute packet
    SII_DRV_ADAPTER_MUTE__VIDEO,         //!< Blank video and send Clear AV Mute
    SII_DRV_ADAPTER_MUTE__AUDIO,         //!< Mute audio and send Clear AV Mute
    SII_DRV_ADAPTER_MUTE__AUDIO_VIDEO    //!< Mute audio video, blank video, and send Clear AV Mute
} SiiDrvAdaptMute_t;

typedef enum
{
    SII_DRV_ADAPTER_HDCP_VER__NOT_SUPPORTED,               //!< HDCP not supported by the source device.
    SII_DRV_ADAPTER_HDCP_VER__1x,                          //!< HDCP 1.x supported by source device
    SII_DRV_ADAPTER_HDCP_VER__20,                          //!< HDCP 2.0 supported by source device
    SII_DRV_ADAPTER_HDCP_VER__21,                          //!< HDCP 2.1 supported by source device
    SII_DRV_ADAPTER_HDCP_VER__22,                          //!< HDCP 2.2 supported by source device
} SiiDrvAdaptHdcpVer_t;

/**
* @brief Error type returned by @ref SiiDrvAdaptHdcpRevocListGet() function
*/
typedef enum
{
    SII_DRV_ADAPTER_KSV_LOAD__OK,            //!< success
    SII_DRV_ADAPTER_KSV_LOAD__NOT_AVAILABLE, //!< BKSV/RxID list was attempted to be read
                                             //!< before authentication or after failure.
    SII_DRV_ADAPTER_KSV_LOAD__BUFFER_ERROR,  //!< BKSV/RxID buffer is too small to fit all BKSV/RxIDs.
} SiiDrvAdaptHdcpKsvLoadError_t;


/**
* @brief HDCP 2.2 content type
*/
typedef enum
{
    SII_DRV_ADAPTER_HDCP_CONTENT__UNKNOWN, //!< Unknown content type
    SII_DRV_ADAPTER_HDCP_CONTENT__TYPE0,   //!< HDCP for content type 0
    SII_DRV_ADAPTER_HDCP_CONTENT__TYPE1    //!< HDCP for content type 1
} SiiDrvAdaptHdcpContenrType_t;

/**
* @brief Chip link type
*/
typedef enum
{
    SII_DRV_LINK_DISCONNECTED,            //!< No device connected with the chip
    SII_DRV_LINK_HDMI,                    //!< HDMI device connected with the chip
    SII_DRV_LINK_MHL12,                   //!< MHL12 device connected with the chip
    SII_DRV_LINK_MHL3                     //!< MHL3 device connected with the chip
} SiiDrvAdaptLinkType_t;

/**
* @brief RxSense status
*/
typedef enum
{
    SII_DRV_ADAPTER_TX_NO_RXSENSED,               //!< No Rxsensed
    SII_DRV_ADAPTER_TX_RXSENSED                   //!< Rxsensed
} SiiDrvAdaptTxRxSenseType_t;

typedef enum
{
    SII_DRV_ADAPT_PLATFORM__SII9679,
    SII_DRV_ADAPT_PLATFORM__SII9678,
    SII_DRV_ADAPT_PLATFORM__AVR
} SiiDrvAdaptPlatformId_t;

typedef struct
{
    uint32_t GeneralEvents;
    uint32_t TxEvents;
    uint32_t RxEvents;
    uint32_t HdcpEvents;
    uint32_t CbusEvents;
    uint32_t CecEvents;
    uint32_t EdidEvents;
} SiiDrvAdaptEvents_t;

/**
* @brief Chip and firmware identification
*/
typedef struct
{
    uint16_t chipId;            //!< Chip ID
    uint8_t  chipRev;           //!< Chip revision
    uint16_t fwVersion;         //!< Firmware version
    uint8_t  fwCustomerVersion; //!< Customer version control
//  uint16_t driverVersion;     //!< Driver version
} SiiDrvAdaptVersionInfo_t;

typedef struct
{
    uint8_t eraseCmd;        // Sector erase command of SPI flash being used
    uint8_t writeEnCmd;      // Write enable command of SPI flash being used
    uint8_t pageWrCmd;       // Page write command of SPI flash being used
} SiiDrvAdapterSpiConfig_t;

/**
* @brief Adapter configuration
*/
typedef struct
{
    SiiDrvAdaptPlatformId_t  platformId;        //!< Platform id
    SiiInterruptOutputType_t interruptPinType;  //!< Interrupt pin type
    SiiInterruptPolarity_t   interruptPolarity; //!< Interrupt signal polarity
    uint8_t                  i2cSlaveAddr;      //!< I2C slave address or 0 to keep default one
    uint8_t                  i2cSlaveAddrBase;  //!< HW slave address when used at debug mode
    uint8_t                  i2cSlaveAddrF0;    //!< Desired slave address when in debug mode, or 0 to keep default 0xF0
    uint8_t                  i2cSlaveAddrAA;    //!< Desired slave address when in ISP mode, or 0 to keep default 0xAA
    SiiDrvAdapterSpiConfig_t spiConfig;         //!< SPI Flash configuration
} SiiDrvAdaptConfig_t;

typedef struct
{
    uint8_t     *pBuffer;
    uint32_t    ucBufferSize;
    uint32_t    ucOffset;
} SiiDrvAdaptFwBuffer_t;

/**
* @brief HDCP Downstream Repeater RxInfo
*/
typedef struct
{
    bool_t  bHdcp1DeviceDs;         //!< HDCP 1.X compliant device in the topology if set to true, HDCP 2.X use only
    bool_t  bHdcp20DeviceDs;        //!< HDCP 2.0 compliant device in the topology if set to true, HDCP 2.X use only
    bool_t  bMaxCascadeExceeded;    //!< More than seven level for HDCP 1.X or four levels for HDCP 2.X of repeaters cascaded together if set to true
    bool_t  bMaxDevsExceeded;       //!< More than 31 devices (for HDCP1.X) or 127 devices (for HDCP 2.X) as attached if set to true
    uint8_t deviceCount;            //!< Total number of attached downstream devices
    uint8_t depth;                  //!< Repeater cascade depth
    uint32_t seqNumV;              //!< seq_num_V value, HDCP 2.X use only
} SiiDrvAdaptHdcpTopology_t;

/**
* @brief HDCP 2.X Stream Manage Info
*/
typedef struct
{
    uint32_t  seqNumM;           //!< The seq_num_M for the RepeaterAuth_Stream_Manage message
    uint16_t  k;                    //!< The K value for the RepeaterAuth_Stream_Manage message, always be 0x001 in HDMI
    uint16_t  streamIdType[4];       //!< The streamID_Type for the RepeaterAuth_Stream_Manage message, the buffer length shoule be K
}SiiDrvAdaptHdcpStreamManageInfo_t;


//#define SiiInst_t void *

/**
* @brief Color Space Type
*/

typedef enum
{
    SII_DRV_ADAPT_COLOR_SPACE_UNKNOWN,
    SII_DRV_ADAPT_COLOR_SPACE_RGB,
    SII_DRV_ADAPT_COLOR_SPACE_YCBCR444,
    SII_DRV_ADAPT_COLOR_SPACE_YCBCR422
} SiiDrvAdaptColorSpaceType_t;

/*****************************************************************************/
/**
* @brief Notification callback prototype
*
* This callback is generated from @ref SiiDrvAdaptTaskExecute() when there is
* any notifications about driver status change.
*
* @param[in] inst  Driver instance caused the callback notification
* @param[in] flags Bit mask of the events
*
* @see SII_DRV_ADAPT_EVENT
* @see SiiDrvAdaptCreate()
* @see SiiDrvAdaptTaskExecute()
* @see SiiLibEvtCbFunc_t
*
*****************************************************************************/
typedef void (*SiiLibEvtCbFunc_t)(SiiInst_t inst, uint32_t addr);

/***** public functions ******************************************************/

/*****************************************************************************/
/**
* @brief Adapter Driver creation
*
* Allocates resources for Adapter Driver and all child modules.
* This function has to be called before using of any other module function.
*
* This function also registers a callback function that is called
* when the driver status is changed.
* The callback is generated from @ref SiiDrvAdaptTaskExecute().
* If \c pCallback is NULL, no callbacks are generated.
*
* @param[in] irqDrvInst Reserved for future API versions. Set to \c (SiiInst_t) \c 0.
* @param[in] hwId       Reserved to support multiple HW instances. Use 0.
* @param[in] pCallback  Pointer to a callback function to be called on status change
*
* @return Handle to created object.
*
* @see SiiDrvAdaptDelete(), SiiDrvAdaptConfigure()
* @see SiiDrvAdaptStart(), SiiDrvAdaptStop()
* @see SII_DRV_ADAPT_EVENT
* @see SiiDrvAdaptTaskExecute()
* @see SiiLibEvtCbFunc_t
*
*****************************************************************************/
SiiInst_t SiiDrvAdaptCreate(SiiInst_t irqDrvInst, uint8_t hwId, SiiLibEvtCbFunc_t pCallback);

/*****************************************************************************/
/**
* @brief Adapter Driver deletion
*
* Frees resources taken by Adapter Driver and all child modules.
* After this function is called, no other module functions are allowed
* to be called until SiiDrvAdaptCreate() is called.
*
* @param[in] inst Adapter driver instance returned by @ref SiiDrvAdaptCreate()
*
* @see SiiDrvAdaptCreate(), SiiDrvAdaptConfigure()
* @see SiiDrvAdaptStart(), SiiDrvAdaptStop()
*
*****************************************************************************/
void SiiDrvAdaptDelete(SiiInst_t inst);

/*****************************************************************************/
/**
* @brief Adapter Driver configuring
*
* Configures Adapter Driver and all child modules.
* This function can be called only after SiiDrvAdaptCreate() or
* SiiDrvAdaptStop() before SiiDrvAdaptStart().
*
* @param[in] inst Adapter driver instance returned by @ref SiiDrvAdaptCreate()
* @param[in] pCfg Pointer to a structure with required configuration
*
* @see SiiDrvAdaptCreate(), SiiDrvAdaptDelete()
* @see SiiDrvAdaptStart(), SiiDrvAdaptStop()
*
*****************************************************************************/
void SiiDrvAdaptConfigure(SiiInst_t inst, SiiDrvAdaptConfig_t *pCfg);

/*****************************************************************************/
/**
* @brief Adapter Driver staring
*
* Starts Adapter, receiver, and transmitter operations
* The function should be called only after the module is configured by
* SiiDrvAdaptConfigure().
*
* @param[in] inst Adapter driver instance returned by @ref SiiDrvAdaptCreate()
*
* @see SiiDrvAdaptCreate(), SiiDrvAdaptConfigure()
* @see SiiDrvAdaptDelete(), SiiDrvAdaptStop()
*
*****************************************************************************/
void SiiDrvAdaptStart(SiiInst_t inst);

/*****************************************************************************/
/**
* @brief Adapter Driver stopping
*
* Stops Adapter, receiver, and transmitter operations.
* After the call, it is allowed to call only the following module
* functions: SiiDrvAdaptConfigure(), SiiDrvAdaptStart(), or
* SiiDrvAdaptDelete().
*
* @param[in] inst Adapter driver instance returned by @ref SiiDrvAdaptCreate()
*
* @see SiiDrvAdaptCreate(), SiiDrvAdaptConfigure()
* @see SiiDrvAdaptStart(), SiiDrvAdaptDelete()
*
*****************************************************************************/
void SiiDrvAdaptStop(SiiInst_t inst);

/*****************************************************************************/
/**
* @brief Adapter Driver periodical task
*
* This function is expected to be called by an upper layer
* after any interrupt from the adapter chip. If chip interrupts are not
* connected to the CPU, this function should periodically be called.
*
* This function checks if there are any pending interrupts from the chip,
* serves them if needed and generates @ref SII_DRV_ADAPT_EVENT "notifications"
* by calling @ref SiiLibEvtCbFunc_t callback function
* registered through @ref SiiDrvAdaptCreate().
*
* @param[in]  inst       Adapter driver instance returned by @ref SiiDrvAdaptCreate()
*
* @see SII_DRV_ADAPT_EVENT
* @see SiiDrvAdaptCreate()
* @see SiiLibEvtCbFunc_t
*
*****************************************************************************/
void SiiDrvAdaptTaskExecute(SiiInst_t inst);


/*****************************************************************************/
/**
* @brief Booting status inquiry
*
* Returns chip boot status.
*
* @param[in]  inst       Adapter driver instance returned by @ref SiiDrvAdaptCreate()
* @param[out] poBootStat Pointer to a variable to return booting status
*
*****************************************************************************/
void SiiDrvAdaptBootStatusGet(SiiInst_t inst, SiiDrvAdaptBootStatus_t *poBootStat);

/*****************************************************************************/
/**
* @brief Control the Chip GPIO, set chip GPIO power level according to the GPIO num
*
* Returns
*
* @param[in]  inst          Adapter driver instance returned by @ref SiiDrvAdaptCreate()
* @param[in]  GpioNo        GPIO No. on Chip, range: 0~3
* @param[in]  GpioPowLevel  Set GPIO Power Level
*
*****************************************************************************/
void SiiDrvAdaptChipGpioCtrl(SiiInst_t inst, uint8_t GpioNo, bool_t bGpioPowLevel);

/*****************************************************************************/
/**
* @brief Read the Chip GPIO, get chip GPIO power level according to the GPIO num
*
* Returns
*
* @param[in]  inst          Adapter driver instance returned by @ref SiiDrvAdaptCreate()
* @param[in]  GpioNo        GPIO No. on Chip, range: 0~3
* @param[out] poGpioStatus  Pointer to the variable of GPIO Status
*
*****************************************************************************/
void SiiDrvAdaptChipGpioRead(SiiInst_t inst, uint8_t GpioNo, bool_t *poGpioStatus);

/*****************************************************************************/
/**
* @brief Chip, driver and firmware version inquiry
*
* Returns Chip, driver and firmware version information.
*
* @param[in]  inst          Adapter driver instance returned by @ref SiiDrvAdaptCreate()
* @param[out] poVersionInfo Pointer to a variable to return version information
*
*****************************************************************************/
uint8_t SiiDrvAdaptVersionGet(SiiInst_t inst, SiiDrvAdaptVersionInfo_t *poVersionInfo);

/*****************************************************************************/
/**
* @brief Chip version inquiry
*
* Returns Chip version information when Firmware boot fail.
*
* @param[in]  inst          Adapter driver instance returned by @ref SiiDrvAdaptCreate()
* @param[out] pChipId       Pointer to a variable to return chip version information
*
*****************************************************************************/
void SiiDrvAdaptChipIdGet(SiiInst_t inst, uint16_t *pChipId);

/*****************************************************************************/
/**
* @brief Firmware update status interrogation
*
* Returns status of firmware update initiated by @ref SiiDrvAdaptFirmwareUpdate().
*
* @param[in]  inst           Adapter driver instance returned by @ref SiiDrvAdaptCreate()
* @param[out] poUpdateStatus Pointer to a variable to copy the status of firmware update
*
* @see SiiDrvAdaptFirmwareUpdate()
* @see SiiDrvAdaptEdidNvramWrite(), SiiDrvAdaptEdidNvramRead()
* @see SiiDrvAdaptBootCfgNvramWrite(), SiiDrvAdaptBootCfgNvramRead()
* @see SiiDrvAdaptBootCfgNvramSet()
* @see SiiDrvAdaptDsEdidRead()
* @see SII_DRV_ADAPT_EVENT__ACCESS_DONE
*
*****************************************************************************/
void SiiDrvAdaptAccessStatusGet(SiiInst_t inst,
        SiiDrvAdaptAccessStatus_t *poUpdateStatus);

/*****************************************************************************/
/**
* @brief Firmware update
*
* Initiates firmware update.
* When firmware update is done,  @ref SiiLibEvtCbFunc_t callback is called
* with @ref SII_DRV_ADAPT_EVENT__ACCESS_DONE flag set in its \c flags parameter
* (if the callback is registered through @ref SiiDrvAdaptCreate() function).
*
* @param[in]  inst     Adapter driver instance returned by @ref SiiDrvAdaptCreate()
* @param[in]  pBuffer  Pointer to a buffer with firmware image
* @param[in]  length   Firmware image length
*
* @see SiiDrvAdaptAccessStatusGet()
* @see SII_DRV_ADAPT_EVENT__ACCESS_DONE
*
*****************************************************************************/
void SiiDrvAdaptFirmwareUpdate(SiiInst_t inst, SiiDrvAdaptFwBuffer_t *pFwBuffer);

/*****************************************************************************/
/**
* @brief Firmware update initialization
*
* Prepare for firmware update.
* Before calling SiiDrvAdaptFirmwareUpdate(), host FW must call this API
* to erase the SPI flash.
* Host FW also needs to have some delay after calling this API because
* SPI flash will require some time to erase the chip.
* If SiiDrvAdaptFirmwareUpdate() is called without delay from SiiDrvAdaptFirmwareUpdateInit(),
* FW update will mostlikely be failed.
* See your datasheet of SPI flash to determine how much delay required after the erase command.
*
* @param[in]  inst     Adapter driver instance returned by @ref SiiDrvAdaptCreate()
*
* @see SiiDrvAdaptFirmwareUpdate()
* @see SiiDrvAdaptAccessStatusGet()
* @see SII_DRV_ADAPT_EVENT__ACCESS_DONE
*
*****************************************************************************/
void SiiDrvAdaptFirmwareUpdateInit(SiiInst_t inst);
void SiiDrvAdaptFirmwareReadInit(SiiInst_t inst);

/*****************************************************************************/
/**
* @brief Chip register reading
*
* Reads block of chip registers.
*
* @note This function is used for debugging purpose only.
*
* @param[in]  inst    Adapter driver instance returned by @ref SiiDrvAdaptCreate()
* @param[in]  address First register address with page number in MSB and offset in LSB
* @param[out] poData  Pointer to a buffer to copy read register values
* @param[in]  length  Number of registers to read
*
*****************************************************************************/
void SiiDrvAdaptChipRegRead(SiiInst_t inst,
        uint16_t address,
        uint8_t  *poData,
        uint16_t length);

void SiiDrvAdaptChipRegWrite(SiiInst_t inst,
        uint16_t address,
        uint8_t  *poData,
        uint16_t length);


#ifdef __cplusplus
}
#endif

#endif // SI_DRV_ADAPTER_H


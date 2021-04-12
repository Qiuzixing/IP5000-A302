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
 * @file
 * si_drv_adapte_rx.h
 *
 * @brief HDCP 1.x/2.x to HDCP 2.x/1.x adapter. Upstream API.
 *
 *****************************************************************************/


#ifndef SI_DRV_ADAPTER_TX_H
#define SI_DRV_ADAPTER_TX_H

#ifdef __cplusplus
extern "C"{
#endif

/***** #include statements ***************************************************/
#include "si_drv_adapter_platform_api.h"

/***** public macro definitions **********************************************/
/**
* @brief TMDS output state
*
* Used in @ref SiiDrvAdaptTxTmdsOutSet() function
*/
typedef enum
{
    SII_DRV_ADAPT_TMDS_OUT__DISABLE, //!< TMDS output is disabled
    SII_DRV_ADAPT_TMDS_OUT__ENABLE   //!< TMDS output is enabled
} SiiDrvAdaptTmdsOut_t;

/***** public type definitions ***********************************************/

/*****************************************************************************/
/**
* @brief Output TMDS swing control
*
* Configures TMDS swing level.
*
* @param[in]  inst           Adapter driver instance returned by @ref SiiDrvAdaptCreate()
* @param[in]  tmdsSwingLevel TMDS swing level in 0...7 range
*
*****************************************************************************/
void SiiDrvAdaptTmdsSwingLevelSet(SiiInst_t inst, uint8_t tmdsSwingLevel);

/*****************************************************************************/
/**
* @brief Output muting
*
* Mutes output by blanking audio and/or video or by sending General Control packet
* with AV Mute bit set.
*
* @param[in] inst Adapter driver instance returned by @ref SiiDrvAdaptCreate()
* @param[in] mute Mute type selection
*
*****************************************************************************/
/**
 * @note Current implementation supports only @ref SII_DRV_ADAPTER_MUTE__AV_MUTE_ON and
 *       @ref SII_DRV_ADAPTER_MUTE__AV_MUTE_OFF
 */
void SiiDrvAdaptTxMuteSet(SiiInst_t inst, SiiDrvAdaptMute_t mute);

/*****************************************************************************/
/**
* @brief TMDS output control
*
* Enables or disables TMDS output.
*
* @param[in]  inst      Adapter driver instance returned by @ref SiiDrvAdaptCreate()
* @param[in]  tmdsCtrl  TMDS output control
*
*
*****************************************************************************/
void SiiDrvAdaptTxTmdsOutSet(SiiInst_t inst, SiiDrvAdaptTmdsOut_t tmdsCtrl);

/**
* @brief HDCP enabling/disabling
*/
typedef enum
{
    SII_DRV_ADAPTER_HDCP_PROTECT__OFF, //!< Disable HDCP protection
    SII_DRV_ADAPTER_HDCP_PROTECT__ON   //!< Enable HDCP protection
} SiiDrvAdaptHdcpProtect_t;

/**
* @brief Downstream HDCP status
*/
typedef enum
{
    SII_DRV_ADAPTER_HDCP_TX_STATUS__OFF,            //!< Authentication is not enabled
    SII_DRV_ADAPTER_HDCP_TX_STATUS__SUCCESS,        //!< Authentication succeeded
  //  SII_DRV_ADAPTER_HDCP_TX_STATUS__RCVID_CHANGE,   //!< Authentication for repeater, got RcvID from downstream
    SII_DRV_ADAPTER_HDCP_TX_STATUS__AUTHENTICATING, //!< Authentication is in progress
    SII_DRV_ADAPTER_HDCP_TX_STATUS__FAILED,         //!< Authentication failed and does not re-try
    SII_DRV_ADAPTER_HDCP_TX_STATUS__RCVID_CHG,
    SII_DRV_ADAPTER_HDCP_TX_STATUS__SMNG_DONE,
    SII_DRV_ADAPTER_HDCP_TX_STATUS__AUTH_DONE,
} SiiDrvAdaptHdcpTxStatus_t;

/**
* @brief Downstream HDCP Failure Reason
*/
typedef enum
{
    SII_DRV_ADAPTER_HDCP_TX_FAIL_REASON__NONE       = 0x00,      //!< No failure detected so far
    SII_DRV_ADAPTER_HDCP_TX_FAIL_REASON__INTERNAL   = 0x01,      //!< Downstream device does not acknowledge HDCP registers; firmware continues trying
    SII_DRV_ADAPTER_HDCP_TX_FAIL_REASON__CAS_EXC    = 0x02,      //!< Cascade exceeded error; firmware does not try until HPD Low to High transition is done
    SII_DRV_ADAPTER_HDCP_TX_FAIL_REASON__DEV_EXC    = 0x04,      //!< Too many devices; firmware does not try until HPD Low to High transition
    SII_DRV_ADAPTER_HDCP_TX_FAIL_REASON__V          = 0x08,      //!< V verification failed; firmware continues trying
    SII_DRV_ADAPTER_HDCP_TX_FAIL_REASON__TIMEOUT    = 0x10,      //!< Authentication timed out; firmware continues trying
    SII_DRV_ADAPTER_HDCP_TX_FAIL_REASON__OTHER      = 0x20,      //!< Other authentication errors; firmware continues trying
} SiiDrvAdaptHdcpTxFailureReason_t;
#if 0
/**
* @brief Downstream device detection status
*
* Used in @ref SiiDrvAdaptDsConnectStatusGet() function
*/
typedef enum
{
    SII_DRV_ADAPT_DS_CONNECT__NONE,     //!< No downstream devices deteced
    SII_DRV_ADAPT_DS_CONNECT__HDMI_DVI, //!< Downstream device is HDMI or DVI
    SII_DRV_ADAPT_DS_CONNECT__MHL       //!< Downstream device is MHL
} SiiDrvAdaptDsConnectStatus_t;
#endif
/**
* @brief TMDS output state
*
* Used in @ref SiiDrvAdaptDsEdidStatusGet() function
*/
typedef enum
{
    SII_DRV_ADAPT_DS_EDID__NOT_AVAILABLE, //!< Downstream EDID is not available
    SII_DRV_ADAPT_DS_EDID__AVAILABLE      //!< Downstream EDID is available
} SiiDrvAdaptDsEdidStatus_t;

/***** public functions ***********************************************/

/*****************************************************************************/
/**
* @brief Downstream HDCP 2.X RxInfo and HDCP 1.X BStatus interrogation
*
*
* @note This function is only expected to be called when Tx is used in AVR repeater case.
*
* @param[in]  inst               Adapter driver instance returned by @ref SiiDrvAdaptCreate()
* @param[out] poDsRxInfo         Pointer to a structure to return downstream RxInfo list
*
*****************************************************************************/
void SiiDrvAdaptTxHdcpDsTopologyGet(SiiInst_t inst, SiiDrvAdaptHdcpTopology_t *poDsRxInfo);

/*****************************************************************************/
/**
* @brief Downstream connection status interrogation
*
* Checks if a downstream device is connected and if it is,
* returns type of downstream device.
*
* @ref SII_DRV_ADAPT_EVENT__DS_CONNECT_STATUS_CHNG event is fired when connection status changes.
*
* @param[in]  inst            Adapter driver instance returned by @ref SiiDrvAdaptCreate()
* @param[out] poConType       Pointer to a a variable to story downstream device connection status
*
* @retval SII_DRV_LINK_DISCONNECTED          No downstream devices sensed
* @retval SII_DRV_LINK_HDMI                  Downstream device is HDMI or DVI
* @retval SII_DRV_LINK_MHL12                 Downstream device is MHL12
* @retval SII_DRV_LINK_MHL3                  Downstream device is MHL3
*
* @see SII_DRV_ADAPT_EVENT__DS_CONNECT_STATUS_CHNG
*
*****************************************************************************/
void SiiDrvAdaptTxLinkTypeGet(SiiInst_t inst, SiiDrvAdaptLinkType_t *poConType);

/*****************************************************************************/
/**
* @brief Downstream EDID availability interrogation
*
* Checks if downstream EDID is available (HPD is high) or not (HPD is low).
*
* @ref SII_DRV_ADAPT_EVENT__DS_EDID_STATUS_CHNG event is fired when EDID
* availability changes.
*
* @param[in]  inst         Adapter driver instance returned by @ref SiiDrvAdaptCreate()
* @param[out] poEdidStatus Pointer to a a variable to story downstream device connection status
*
* @retval SII_DRV_ADAPT_DS_EDID__NOT_AVAILABLE Downstream EDID is not available
* @retval SII_DRV_ADAPT_DS_EDID__AVAILABLE     Downstream EDID is available
*
* @see SII_DRV_ADAPT_EVENT__DS_EDID_STATUS_CHNG
*
*****************************************************************************/
void SiiDrvAdaptTxDsEdidStatusGet(SiiInst_t inst, SiiDrvAdaptDsEdidStatus_t *poEdidStatus);


/*****************************************************************************/
/**
* @brief Downstream HDCP 1.X/2.X AKSV/RxID interrogation
*
* Returns downstream AKSV for HDCP 1.X, and also RxID for HDCP 2.X.
*
* @param[in]  inst         Adapter driver instance returned by @ref SiiDrvAdaptCreate()
* @param[out] poAksvBuffer Pointer to a buffer to return downstream BKSV/RxID list
*
* @retval SII_DRV_ADAPTER_KSV_LOAD__OK BKSV, RxID successfully loaded into the buffer
* @retval SII_DRV_ADAPTER_KSV_LOAD__NOT_AVAILABLE BKSV or RxID reading failed
*
*****************************************************************************/
SiiDrvAdaptHdcpKsvLoadError_t SiiDrvAdaptTxHdcpDsRxIdListGet(SiiInst_t inst,
        uint8_t *poRxId,
        uint16_t ListLen,
        uint16_t Offset);

/***** public functions ******************************************************/
/*****************************************************************************/
/**
* @brief Downstream HDCP support interrogation
*
* Returns information about downstream device HDCP capabilities.
*
* @param[in] inst       Adapter driver instance returned by @ref SiiDrvAdaptCreate()
* @param[out] poHdcpVer Pointer to a variable to copy downstream device HDCP capabilities
*
*****************************************************************************/
void SiiDrvAdaptTxHdcpDsVerGet(SiiInst_t inst, SiiDrvAdaptHdcpVer_t *poHdcpVer);

/*****************************************************************************/
/**
* @brief Downstream HDCP status interrogation
*
* Returns current downstream HDCP status.
*
* @param[in] inst Adapter driver instance returned by @ref SiiDrvAdaptCreate()
* @param[in] poHdcpStatus Pointer to a variable to copy HDCP status
*
* @see SII_DRV_ADAPT_EVENT__TX_HDCP_STATUS_CHNG
* @see SiiDrvAdaptDsHdcpLastFailureReasonGet()
*
*****************************************************************************/
void SiiDrvAdaptTxHdcpDsStatusGet(SiiInst_t inst, SiiDrvAdaptHdcpTxStatus_t *poHdcpStatus);

/*****************************************************************************/
/**
* @brief Downstream HDCP downstream repeater bit interrogation
*
* Returns downstream repeater status.
*
* @note This function is only expected to be called when Tx is used in AVR repeater case.
*
* @param[in]  inst               Adapter driver instance returned by @ref SiiDrvAdaptCreate()
* @param[out] poIsDsRepeater     Pointer to a variable to copy the downstream repeater bit, \c *poIsDsRepeater:
*                                          - \c true  Downstream device is a repeater
*                                          - \c false Downstream device is not a repeater
*****************************************************************************/
void SiiDrvAdaptTxHdcpDsRepeaterBitGet(SiiInst_t inst, bool_t *poIsDsRepeater);

/*****************************************************************************/
/**
* @brief Downstream HDCP failure interrogation
*
* Returns the reason of last downstream (TX) HDCP failure.
*
* @note HDCP authentication status returned by @ref SiiDrvAdaptTxHdcpDsStatusGet()
*       can indicate success, but this function returns information about the
*       last HDCP error.
*
* @param[in] inst             Adapter driver instance returned by @ref SiiDrvAdaptCreate()
* @param[in] poHdcpFailReason Pointer to a variable to copy HDCP failure reason
*
* @see SiiDrvAdaptTxHdcpDsStatusGet()
*
*****************************************************************************/
void SiiDrvAdaptDsHdcpLastFailureReasonGet(SiiInst_t inst,
        uint8_t *poHdcpFailReason);

/*****************************************************************************/
/**
* @brief Downstream HDCP 1.X/2.X AKSV/RxID interrogation
*
* Returns downstream AKSV for HDCP 1.X, and also RxID for HDCP 2.X.
*
* @param[in]  inst         Adapter driver instance returned by @ref SiiDrvAdaptCreate()
* @param[out] poAksvBuffer Pointer to a buffer to return downstream BKSV/RxID list
*
* @retval SII_DRV_ADAPTER_KSV_LOAD__OK BKSV, RxID successfully loaded into the buffer
* @retval SII_DRV_ADAPTER_KSV_LOAD__NOT_AVAILABLE BKSV or RxID reading failed
*
*****************************************************************************/
SiiDrvAdaptHdcpKsvLoadError_t SiiDrvAdaptTxHdcpRcvIdGet
    (SiiInst_t inst, uint8_t poRcvidBuffer[5]);

/*****************************************************************************/
/**
* @brief Downstream HDCP 1.X AKSV interrogation
*
* Returns downstream AKSV for HDCP 1.X.
*
* @param[in]  inst         Adapter driver instance returned by @ref SiiDrvAdaptCreate()
* @param[out] poAksvBuffer Pointer to a buffer to return downstream BKSV/RxID list
*
* @retval SII_DRV_ADAPTER_KSV_LOAD__OK BKSV successfully loaded into the buffer
* @retval SII_DRV_ADAPTER_KSV_LOAD__NOT_AVAILABLE BKSV reading failed
*
*****************************************************************************/
SiiDrvAdaptHdcpKsvLoadError_t SiiDrvAdaptTxHdcpAksvGet(SiiInst_t inst, uint8_t poAksvBuffer[5]);

/*****************************************************************************/
/**
* @brief Reads downstream EDID
*
* When reading is done,  @ref SiiLibEvtCbFunc_t callback is called
* with @ref SII_DRV_ADAPT_EVENT__ACCESS_DONE flag set in its \c flags parameter
* (if the callback is registered through @ref SiiDrvAdaptCreate() function).
*
* @param[in]  inst     Adapter driver instance returned by @ref SiiDrvAdaptCreate()
* @param[in]  blockNmb EDID block number (each block is 128 bytes)
* @param[in]  offset   Offset in the block
* @param[out] poData   Pointer to an input buffer
* @param[in]  length   Number of bytes to read
*
* @see SiiDrvAdaptEdidNvramWrite()
* @see SiiDrvAdaptAccessStatusGet()
* @see SII_DRV_ADAPT_EVENT__ACCESS_DONE
*
*****************************************************************************/
void SiiDrvAdaptTxDsEdidRead(SiiInst_t inst,
        uint8_t blockNmb,
        uint8_t offset,
        uint8_t *poData,
        uint16_t length);

/*****************************************************************************/
/**
* @brief Trigger SiI9678 to re-read DS EDID data.
*
* will trigger an interrupt to notify host software that the EDID reading has been completed.
*
* @param[in] inst       Adapter driver instance returned by @ref SiiDrvAdaptCreate()
*
*****************************************************************************/
void SiiDrvAdaptTxDsEdidReRead(SiiInst_t inst);

/*****************************************************************************/
/**
* @brief Downstream HDCP 2.X Stream Manage message set
*
* Propagate the HDCP 2.X Stream_Manage message to the downstream repeater.
*
* @note This function is only expected to be called when Tx is used in AVR repeater case, and attached downstream device is also a repeater.
*
* @param[in]  inst                 Adapter driver instance returned by @ref SiiDrvAdaptCreate()
* @param[in]  streamManageInfo     HDCP 2.X stream manage info
*
*****************************************************************************/
void SiiDrvAdaptTxHdcpStreamManageMsgSet(SiiInst_t inst, SiiDrvAdaptHdcpStreamManageInfo_t *poStreamManageInfo);

/*****************************************************************************/
/**
* @brief Downstream HDCP 2.2 type setter
*
* This function selects HDCP 2.2 content type.
*
* Content type 0 is allowed to be transmitted to any downstream HDCP device.
* Content type 1 can only be transmitted to HDMI 2.X sinks and HDMI 2.1
* or later repeaters.
*
* @param[in] inst        Adapter driver instance returned by @ref SiiDrvAdaptCreate()
* @param[in] contentType HDCP 2.2 content type
*   - @ref SII_DRV_ADAPTER_HDCP_CONTENT__TYPE0 HDCP content type 0
*   - @ref SII_DRV_ADAPTER_HDCP_CONTENT__TYPE1 HDCP content type 1
*
*****************************************************************************/
void SiiDrvAdaptTxHdcpContentTypeSet(SiiInst_t inst, SiiDrvAdaptHdcpContenrType_t contentType);

/*****************************************************************************/
/**
* @brief Downstream HDCP enabling control
*
* Enables/disables downstream HDCP.
*
* @note This function is only used for debugging purpose and only with
*       debugging version of adapter firmware.
*
* @note HDCP is enabled by default.
*
* @param[in] inst        Adapter driver instance returned by @ref SiiDrvAdaptCreate()
* @param[in] protectType HDCP configuration
*   - SII_DRV_ADAPTER_HDCP_PROTECT__OFF   HDCP disabled
*   - SII_DRV_ADAPTER_HDCP_PROTECT__ON    HDCP enabled
*
*****************************************************************************/
void SiiDrvAdaptTxHdcpProtectionSet(SiiInst_t inst, SiiDrvAdaptHdcpProtect_t protectType);
#if 0
void SiiDrvAdaptTxHdcpEventGet(SiiInst_t inst, uint8_t *poTxHdcpEvent);

void SiiDrvAdaptTxHdcpEventClear(SiiInst_t inst, uint8_t ucTxHdcpEvent);
#endif
/*****************************************************************************/
/**
* @brief Downstream Rx sense status interrogation
*
* Checks if a downstream Rx sense is detected and if it is,
* returns the status of Rx sense.
*
* @ref SII_DRV_ADAPT_EVENT__RX_SENSE_STATUS_CHNG event is fired when Rx sense status changes.
*
* @param[in]  inst            Adapter driver instance returned by @ref SiiDrvAdaptCreate()
* @param[out] poConType       Pointer to a a variable to store Rx sense status
*
* @retval SII_DRV_TX_NO_RXSENSED          No Rx sensed
* @retval SII_DRV_TX_RXSENSED               Rx sensed
*
* @see SII_DRV_ADAPT_EVENT__DS_CONNECT_STATUS_CHNG
*
*****************************************************************************/
void SiiDrvAdaptTxRxSenseStatusGet(SiiInst_t inst, SiiDrvAdaptTxRxSenseType_t *poRxStatus);

/*****************************************************************************/
/**
* @brief Downstream TMDS analog power status
*
* Returns downstream TMDS analog power status.
*
* @param[in]  inst     Adapter driver instance returned by @ref SiiDrvAdaptCreate()
* @param[out] true     TX TMDS analog power/PHY is enabled
*             false    TX TMDS analog power/PHY is not enabled
*****************************************************************************/
bool_t SiiDrvAdaptTxTmdsAnalogPowerStatusGet(SiiInst_t inst);

/*****************************************************************************/
/**
* @brief Downstream diagnostics power down IDCK status
*
* Returns downstream diagnostics power down IDCK status.
*
* @param[in]  inst     Adapter driver instance returned by @ref SiiDrvAdaptCreate()
* @param[out] true     Normal operation
*             false    Power down
*****************************************************************************/
bool_t SiiDrvAdaptTxPowerDownIdckStatusGet(SiiInst_t inst);

/*****************************************************************************/

/**
* @brief Output color space control
*
* Set output color space to RGB or YCbCr444 or YCbCr422.
*
* @note This API is only available for SiI9678.
*
* @param[in] inst           Adapter driver instance returned by @ref SiiDrvAdaptCreate()
* @param[in] colorSpaceType Color space type to be set at output port
*   - SII_DRV_ADAPT_COLOR_SPACE_RGB       Set output color space to RGB
*   - SII_DRV_ADAPT_COLOR_SPACE_YCBCR444  Set output color space to YCbCr444
*   - SII_DRV_ADAPT_COLOR_SPACE_YCBCR422  Set output color space to YCbCr422
*
******************************************************************************/
void SiiDrvAdaptTxColorSpaceSet(SiiInst_t inst, SiiDrvAdaptColorSpaceType_t colorSpaceType);

#ifdef __cplusplus
}
#endif

#endif // SI_DRV_ADAPTER_TX_H


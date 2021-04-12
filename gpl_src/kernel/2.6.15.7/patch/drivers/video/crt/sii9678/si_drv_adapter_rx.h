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


#ifndef SI_DRV_ADAPTER_RX_H
#define SI_DRV_ADAPTER_RX_H

#ifdef __cplusplus
extern "C"{
#endif

/***** #include statements ***************************************************/
#include "si_drv_adapter_platform_api.h"
#include "si_drv_adapter.h"

/***** public macro definitions **********************************************/

/**
* @brief Number of inputs (RXs)
*/
#define SII_DRV_ADAPT__NMB_OF_INPUTS 1

/***** public type definitions ***********************************************/

/***** public functions ******************************************************/

/**
* @brief Upstream HDCP Status
*/
typedef enum
{
    SII_DRV_ADAPTER_HDCP_RX_STATUS__OFF,
    SII_DRV_ADAPTER_HDCP_RX_STATUS__SUCCESS,
    SII_DRV_ADAPTER_HDCP_RX_STATUS__AUTHENTICATING,
    SII_DRV_ADAPTER_HDCP_RX_STATUS__FAIL,
} SiiDrvAdaptHdcpRxStatus_t;

/**
* @brief Input control selection
*
* Used in @ref SiiDrvAdaptRxHpdSet(), @ref SiiDrvAdaptRxTermSet(),
* @ref SiiDrvAdaptEdidDdcSet(), and @ref SiiDrvAdaptHdcpDdcSet()
* functions.
*
*/
typedef enum
{
    SII_DRV_ADAPT_INP_CTRL__BYPASS,  //!< Copy from downstream
    SII_DRV_ADAPT_INP_CTRL__DISABLE, //!< Disable or set LOW
    SII_DRV_ADAPT_INP_CTRL__ENABLE,  //!< Enable or set HIGH
} SiiDrvAdaptInpCtrl_t;

typedef enum
{
    SII_DRV_ADAPTER_CBUS_RX_IMPEDANCE__1K,      //!< Overwrite CBUS impedance as 1K
    SII_DRV_ADAPTER_CBUS_RX_IMPEDANCE__HI_Z,    //!< Overwrite CBUS impedance as 100k
    SII_DRV_ADAPTER_CBUS_RX_IMPEDANCE__BYPASS,  //!< FW automatically control the CBUS impedance
} SiiDrvAdaptCbusRxImpedance_t;

/*****************************************************************************/
/**
* @brief  Chip link type get
*
* Get the link type of the chip to know current connection is HDMI or MHL.For the Tx chip, this function tells the connection type on the downstream,
* while for the Rx chip, it returns the connection type on the upstream.
*
*
* @note This function is used for dual mode adapter only.
*
* @param[in]  inst    Adapter driver instance returned by @ref SiiDrvAdaptCreate()
*
*
*****************************************************************************/
void SiiDrvAdaptRxLinkTypeGet(SiiInst_t inst, SiiDrvAdaptLinkType_t *poConType);

/*****************************************************************************/
/**
* @brief Upstream HDCP status interrogation
*
* Returns current upstream HDCP status.
*
* @param[in] inst Adapter driver instance returned by @ref SiiDrvAdaptCreate()
* @param[in] poHdcpStatus Pointer to a variable to copy HDCP status
*
*****************************************************************************/
void SiiDrvAdaptRxHdcpUsStatusGet(SiiInst_t inst, SiiDrvAdaptHdcpRxStatus_t *poHdcpStatus);

/*****************************************************************************/
/**
* @brief RX HPD Control
*
* Set RX Hot Plug.
*
* @param[in]  inst  Adapter driver instance returned by @ref SiiDrvAdaptCreate()
* @param[in]  hpd   Upstream HPD state
*                     - @ref SII_DRV_ADAPT_INP_CTRL__DISABLE HPD is low
*                     - @ref SII_DRV_ADAPT_INP_CTRL__ENABLE  HPD is connected to HDMI +5V
*                     - @ref SII_DRV_ADAPT_INP_CTRL__BYPASS  Currently not supported
*
*****************************************************************************/
void SiiDrvAdaptRxHpdSet(SiiInst_t inst, SiiDrvAdaptInpCtrl_t hpd);

/*****************************************************************************/
/**
* @brief RX HDCP DDC control
*
* Control HDCP DDC registers accessibility from upstream.
*
* @param[in]  inst  Adapter driver instance returned by @ref SiiDrvAdaptCreate()
* @param[in]  ddc   RX HDCP DDC state
*                     - @ref SII_DRV_ADAPT_INP_CTRL__DISABLE HDCP DDC is disabled
*                     - @ref SII_DRV_ADAPT_INP_CTRL__ENABLE  HDCP DDC is enabled
*                     - @ref SII_DRV_ADAPT_INP_CTRL__BYPASS  Currently not supported
*
*****************************************************************************/
void SiiDrvAdaptHdcpDdcSet(SiiInst_t inst, SiiDrvAdaptInpCtrl_t ddc);

/*****************************************************************************/
/**
* @brief Enable Rx HDCP repeater bit to work as a HDCP repeater
*
*
* @note This function is only expected to be called when Tx is used in AVR repeater case.
*
* @param[in]  inst               Adapter driver instance returned by @ref SiiDrvAdaptCreate()
* @param[in]  repeaterMode       Repeater bit , \c repeaterMode:
*                                          - \c true  Set the Rx to work as a HDCP repeater
*                                          - \c false Set the Rx to work as a HDCP receiver
*****************************************************************************/
void SiiDrvAdaptRxHdcpRepeaterModeSet(SiiInst_t inst, bool_t repeaterMode);

/*****************************************************************************/
/**
* @brief RX Termination control
*
* Sets RX termination.
*
* @param[in]  inst  Adapter driver instance returned by @ref SiiDrvAdaptCreate()
* @param[in]  term  RX termination state
*                     - @ref SII_DRV_ADAPT_INP_CTRL__DISABLE Termination is OFF
*                     - @ref SII_DRV_ADAPT_INP_CTRL__ENABLE  Termination is ON
*                     - @ref SII_DRV_ADAPT_INP_CTRL__BYPASS  Currently not supported
*
*****************************************************************************/
void SiiDrvAdaptRxTermSet(SiiInst_t inst, SiiDrvAdaptInpCtrl_t term);

/*****************************************************************************/
/**
* @brief RX CBUS impedance control
*
* Sets RX CBUS impedance.
*
* @param[in]  inst  Adapter driver instance returned by @ref SiiDrvAdaptCreate()
* @param[in]  term  RX termination state
*                     - @ref SII_DRV_ADAPTER_CBUS_RX_IMPEDANCE__1K Set CBUS impedance 1k ohm
*                     - @ref SII_DRV_ADAPTER_CBUS_RX_IMPEDANCE__HI_Z  Set CBUS impedance 100k ohm
*
*****************************************************************************/
void SiiDrvAdaptRxCbusImpedanceSet(SiiInst_t inst, SiiDrvAdaptCbusRxImpedance_t CbusImpedance);
#if 0
/*****************************************************************************/
/**
* @brief Set the valid flag after finish loading the Receiver list
*
* Actually if Rx work as a HDCP 1.X to HDCP 2.2 convertor of HDCP 1.X repeater
* this function would also be responsible for triggering the HDCP1.4 engine to do all work for the step2 of the authentication
*
* @note This function is only expected to be called when Rx is used in AVR repeater case.
*
* @param[in]  inst               Adapter driver instance returned by @ref SiiDrvAdaptCreate()
* @param[in]  valid              RcvId List Flag , \c valid:
*                                          - \c true  The RcvID list is valid
*                                          - \c false The RcvID list is invalid
*****************************************************************************/
void SiiDrvAdaptRxHdcpRcvIdListValidSet (SiiInst_t inst, bool_t valid);
#endif
/*****************************************************************************/
/**
* @brief Repeater HDCP Topology - HDCP 2.X RxInfo or HDCP 1.X BStatus propagation
*
*
* @note This function is only expected to be called when Rx is used in AVR repeater case.
*
* @param[in]  inst               Adapter driver instance returned by @ref SiiDrvAdaptCreate()
* @param[in]  topology           Downstream device topology information to set for the HDCP 2.x Send_ReceiverID_List message or BStatus of HDCP 1.X
*
*****************************************************************************/
void SiiDrvAdaptRxHdcpTopologySet(SiiInst_t inst, SiiDrvAdaptHdcpTopology_t *poTopology);
#if 0
/*****************************************************************************/
/**
* @brief Set the pointer to the specified offset in the Receiver ID List
*
* There are mainly two cases for calling this function, one is to make sure we read the Receiver ID list
* from the beginning, another case is that if there are more than 256 bytes in the list, several times calling
* this function is needed to read all the bytes out, for @ref SiiDrvRxAdaptHdcpRxIdListGet can only read 256 bytes one time.
*
* @note This function is only expected to be called when Rx is used in AVR repeater case.
*
* @param[in]  inst             Adapter driver instance returned by @ref SiiDrvAdaptCreate()
* @param[in]  prtOffset        The offset into the ReceiverID list
*
******************************************************************************/
void SiiDrvAdaptRxHdcpRxIdListPtrSet(SiiInst_t inst,  uint16_t prtOffset);
#endif

/*****************************************************************************/
/**
* @brief in AVR mode, when TX authentacation failed, should reset Ri to restart HDCP auth process
*
*
* @note This function is only expected to be called when Rx is used in AVR repeater case.
*
* @param[in]  inst               Adapter driver instance returned by @ref SiiDrvAdaptCreate()
*
*****************************************************************************/
void SiiDrvAdaptRxHdcpRiReset(SiiInst_t inst);

/*****************************************************************************/
/**
* @brief Downstream HDCP 2.X RxInfo and HDCP 1.X KSV list propagation
*
*
* @note This function is only expected to be called when Rx is used in AVR repeater case and the attached device is a repeater.
*
* @param[in]  inst             Adapter driver instance returned by @ref SiiDrvAdaptCreate()
* @param[in]  deviceCnt        The total number of device attached to repeater, @ref SiiDrvAdaptRxHdcpRxInfoSet()
* @param[in]  poDsRxIdList     Pointer to a buffer to store the receiverID list
*
******************************************************************************/
void SiiDrvAdaptRxHdcpRxIdListSet(SiiInst_t inst,  uint8_t deviceCnt, uint8_t *poDsRxIdList, uint16_t offset);

/*****************************************************************************/
/**
* @brief Trigger to send Rx ID list in Fifo
*
*
* @note This function is only expected to be called when Rx is used in AVR repeater case and the attached device is a repeater.
*
* @param[in]  inst             Adapter driver instance returned by @ref SiiDrvAdaptCreate()
*
******************************************************************************/
void SiiDrvAdaptRxHdcpRxIdListTriggerToSend(SiiInst_t inst);

/*****************************************************************************/
/**
* @brief Upstream HDCP 2.X Stream Manage info propagation
*
* @note This function is only expected to be called when Rx is used in AVR repeater case, and attached downstream device is also a repeater.
*
* @param[in]   inst                 Adapter driver instance returned by @ref SiiDrvAdaptCreate()
* @param[out]  streamManageInfo     Set HDCP 2.X stream manage info
*
*****************************************************************************/
void SiiDrvAdaptRxHdcpStreamManageMsgSet(SiiInst_t inst, SiiDrvAdaptHdcpStreamManageInfo_t *poStreamManageInfo);

/*****************************************************************************/
/**
* @brief RX EDID DDC control
*
* Control EDID accessibility from upstream.
*
* @param[in]  inst  Adapter driver instance returned by @ref SiiDrvAdaptCreate()
* @param[in]  ddc   RX EDID DDC state
*                     - @ref SII_DRV_ADAPT_INP_CTRL__DISABLE EDID DDC is disabled
*                     - @ref SII_DRV_ADAPT_INP_CTRL__ENABLE  EDID DDC is enabled
*                     - @ref SII_DRV_ADAPT_INP_CTRL__BYPASS  Currently not supported
*
*****************************************************************************/
void SiiDrvAdaptEdidDdcSet(SiiInst_t inst, SiiDrvAdaptInpCtrl_t ddc);

/*****************************************************************************/
/**
* @brief Upstream HDCP 1.X BKSV interrogation
*
* Return upstream (own) BKSV for HDCP 1.X.
*
* @param[in]  inst         Adapter driver instance returned by @ref SiiDrvAdaptCreate()
* @param[out] poBksvBuffer Pointer to a buffer to return downstream BKSV/RxID list
*
* @retval SII_DRV_ADAPTER_KSV_LOAD__OK BKSV successfully loaded into the buffer
* @retval SII_DRV_ADAPTER_KSV_LOAD__NOT_AVAILABLE BKSV reading failed
*
* @see SiiDrvAdaptRxHdcpRxIdGet()
*
*****************************************************************************/
SiiDrvAdaptHdcpKsvLoadError_t SiiDrvAdaptRxHdcpBksvGet(SiiInst_t inst, uint8_t poBksvBuffer[5]);

/*****************************************************************************/
/**
* @brief Upstream HDCP 2.2 RX ID interrogation
*
* Returns upstream (own) RX ID for HDCP 2.2.
*
* @param[in]  inst         Adapter driver instance returned by @ref SiiDrvAdaptCreate()
* @param[out] poRxIdBuffer Pointer to a buffer to return downstream BKSV/RxID list
*
* @retval SII_DRV_ADAPTER_KSV_LOAD__OK RX ID successfully read into the buffer
* @retval SII_DRV_ADAPTER_KSV_LOAD__NOT_AVAILABLE RX ID reading failed
*
* @see SiiDrvAdaptRxHdcpBksvGet()
*
*****************************************************************************/
SiiDrvAdaptHdcpKsvLoadError_t SiiDrvAdaptRxHdcpRxIdGet(SiiInst_t inst, uint8_t poRxIdBuffer[5]);

/*****************************************************************************/
/**
* @brief Upstream HDCP 2.X Stream Manage info Interrogation
*
* @note This function is only expected to be called when Tx is used in AVR repeater case, and attached downstream device is also a repeater.
*
* @param[in]   inst                 Adapter driver instance returned by @ref SiiDrvAdaptCreate()
* @param[out]  poStreamManageInfo   Get HDCP 2.X stream manage info
*
*****************************************************************************/
void SiiDrvAdaptRxHdcpStreamManageMsgGet(SiiInst_t inst, SiiDrvAdaptHdcpStreamManageInfo_t *poStreamManageInfo);

/*****************************************************************************/
/**
* @brief RX Clock detection Interrogation
*
* Returns RX clock detection status.
*
* @param[in]  inst           Adapter driver instance returned by @ref SiiDrvAdaptCreate()
* @param[out] poCkdtStatus   Pointer to a variable to copy the clock status. \c *poCkdtStatus:
*                              - \c true  TMDS clock is present
*                              - \c false TMDS clock is not present
*
* @see SII_DRV_ADAPT_EVENT__CKDT_STATUS_CHNG
*
*****************************************************************************/
void SiiDrvAdaptCkdtStatusGet(SiiInst_t inst, bool_t *poCkdtStatus);

/*****************************************************************************/
/**
* @brief RX Sync detection Interrogation
*
* Returns RX Sync detection status.
*
* @param[in]  inst            Adapter driver instance returned by @ref SiiDrvAdaptCreate()
* @param[out] poScdtDetStatus Pointer to a variable to copy the clock status, \c *poScdtDetStatus:
*                               - \c true  TMDS signal is locked
*                               - \c false TMDS signal is not locked
*
* @see SII_DRV_ADAPT_EVENT__SCDT_STATUS_CHNG
*
*****************************************************************************/
void SiiDrvAdaptScdtStatusGet(SiiInst_t inst, bool_t *poScdtDetStatus);

/*****************************************************************************/
/**
* @brief RX AV Mute status interrogation
*
* Returns RX Mute status.
*
* @param[in]  inst           Adapter driver instance returned by @ref SiiDrvAdaptCreate()
* @param[out] poMuteStat     Pointer to a variable to copy the mute status. \c *poMuteStat:
*                              - SII_DRV_ADAPTER_MUTE__AV_MUTE_OFF Not muted
*                              - SII_DRV_ADAPTER_MUTE__AV_MUTE_ON  Muted
*
* @see SII_DRV_ADAPT_EVENT__RX_AV_MUTE_STATUS_CHNG
*
*****************************************************************************/
void SiiDrvAdaptRxAvMuteStatusGet(SiiInst_t inst, SiiDrvAdaptMute_t *poMuteStat);

/*****************************************************************************/
/**
* @brief RX +5V status interrogation
*
* Returns RX +5V status.
*
* @param[in]  inst           Adapter driver instance returned by @ref SiiDrvAdaptCreate()
* @param[out] poPlus5vStat   Pointer to a variable to copy RX +5V status, \c *poPlus5vStat:
*                               - \c true  No +5V signal
*                               - \c false +5V signal detected
*
* @see SII_DRV_ADAPT_EVENT__RX_AV_MUTE_STATUS_CHNG
*
*****************************************************************************/
void SiiDrvAdaptRxPlus5vStatusGet(SiiInst_t inst, bool_t *poPlus5vStat);

/*****************************************************************************/
/**
* @brief RX EDID SRAM Reading
*
* Reads data from EDID SRAM.
*
* @note  This can be checked by calling  @ref SiiDrvAdaptAccessStatusGet() function.
*       If it returns @ref SII_DRV_ADAPTER_ACCESS__IN_PROGRESS, a previous operation
*       has not been finished yet.
*
* @param[in]  inst    Adapter driver instance returned by @ref SiiDrvAdaptCreate()
* @param[in]  offset Offset in EDID SRAM memory
* @param[out] poData  Pointer to output buffer
* @param[in]  length  Number of bytes to read
*
* @see SiiDrvAdaptEdidSramWrite()
* @see SiiDrvAdaptAccessStatusGet()
*
*****************************************************************************/
void SiiDrvAdaptRxEdidSramRead(SiiInst_t inst,
        uint16_t offset,
        uint8_t  *poData,
        uint16_t length);

/*****************************************************************************/
/**
* @brief RX EDID SRAM Updating
*
* Writes data into EDID SRAM.
*
* @note This can be checked by calling  @ref SiiDrvAdaptAccessStatusGet() function.
*       If it returns @ref SII_DRV_ADAPTER_ACCESS__IN_PROGRESS, a previous operation
*       has not been finished yet.
*
* @param[in]  inst    Adapter driver instance returned by @ref SiiDrvAdaptCreate()
* @param[in]  offset Offset in EDID SRAM memory
* @param[in]  pData   Pointer to input buffer
* @param[in]  length  Number of bytes to write
*
* @see SiiDrvAdaptEdidSramRead()
* @see SiiDrvAdaptAccessStatusGet()
*
*****************************************************************************/
void SiiDrvAdaptRxEdidSramWrite(SiiInst_t inst,
        uint16_t offset,
        const uint8_t  *pData,
        uint16_t length);

/*****************************************************************************/
/**
* @brief Reads EDID from EEPROM
*
* When reading is done,  @ref SiiLibEvtCbFunc_t callback is called
* with @ref SII_DRV_ADAPT_EVENT__ACCESS_DONE flag set in its \c flags parameter
* (if the callback is registered through @ref SiiDrvAdaptCreate() function).
*
* @param[in]  inst     Adapter driver instance returned by @ref SiiDrvAdaptCreate()
* @param[in]  offset   Offset in the block
* @param[out] poData   Pointer to an input buffer
* @param[in]  length   Number of bytes to read
*
* @see SiiDrvAdaptEdidNvramWrite()
* @see SiiDrvAdaptAccessStatusGet()
* @see SII_DRV_ADAPT_EVENT__ACCESS_DONE
*
*****************************************************************************/
void SiiDrvAdaptEepromEdidRead(SiiInst_t inst,
        uint8_t offset,
        uint8_t *poData,
        uint16_t length);

/*****************************************************************************/
/**
* @brief Write EDID to EEPROM
*
* When reading is done,  @ref SiiLibEvtCbFunc_t callback is called
* with @ref SII_DRV_ADAPT_EVENT__ACCESS_DONE flag set in its \c flags parameter
* (if the callback is registered through @ref SiiDrvAdaptCreate() function).
*
* @param[in]  inst     Adapter driver instance returned by @ref SiiDrvAdaptCreate()
* @param[in]  offset   Offset in the block
* @param[out] poData   Pointer to an input buffer
* @param[in]  length   Number of bytes to read
*
* @see SiiDrvAdaptEdidNvramWrite()
* @see SiiDrvAdaptAccessStatusGet()
* @see SII_DRV_ADAPT_EVENT__ACCESS_DONE
*
*****************************************************************************/
void SiiDrvAdaptEepromEdidWrite(SiiInst_t inst,
        uint8_t offset,
        uint8_t *poData,
        uint16_t length);

/*****************************************************************************/
/**
* @brief Get whether Rx work in HDCP repeater mode or reciever mode
*
*
* @note This function is only expected to be called when Rx is used in AVR repeater case.
*
* @param[in]  inst               Adapter driver instance returned by @ref SiiDrvAdaptCreate()
* @param[in]  repeaterMode       Repeater bit , \c *repeaterMode:
*                                          - \c true  The Rx is working as a HDCP repeater
*                                          - \c false The Rx is working as a HDCP receiver
*****************************************************************************/
void SiiDrvAdaptRxHdcpRepeaterModeGet(SiiInst_t inst, bool_t *poIsDsRepeater);

/*****************************************************************************/
/**
* @brief
*
*
* @note This function is only expected to be called when Rx is used in AVR repeater case.
*
* @param[in]  inst               Adapter driver instance returned by @ref SiiDrvAdaptCreate()
* @param[in]  poValid            RcvId List Flag , \c *poValid:
*                                          - \c true  Set the RcvID list is valid
*                                          - \c false Set the RcvID list is invalid
*****************************************************************************/
void SiiDrvAdaptRxHdcpRcvIdListValidGet(SiiInst_t inst, bool_t *poValid);

/*****************************************************************************/
/**
* @brief Repeater HDCP Topology - 2.X RxInfo or HDCP 1.X BStatus interrogation
*
*
* @note This function is only expected to be called when Rx is used in AVR repeater case.
*
* @param[in]  inst               Adapter driver instance returned by @ref SiiDrvAdaptCreate()
* @param[in]  dsTopology         Downstream device topology information to Get from the HDCP 2.x Send_ReceiverID_List message or BStatus of HDCP 1.X
*
*****************************************************************************/
void SiiDrvAdaptRxHdcpTopologyGet(SiiInst_t inst, SiiDrvAdaptHdcpTopology_t *poTopology);

/*****************************************************************************/
/**
* @brief Revocation list retrieving
*
* Returns the list of downstream device BKSVs/RxIDs.
* The first BKSV/RxID in the list is the immediate sink BKSV/RxID.
*
* @note This function is only expected to be called after successful authentication.
*
* @param[in]  inst          Adapter driver instance returned by @ref SiiDrvAdaptCreate()
* @param[out] poBksvNumber  Pointer to a variable to copy number of devices in the buffer
* @param[out] poRevocBuffer Pointer to a buffer to return downstream BKSV/RxID list
* @param[in]  bufferSize    \c poRevocBuffer buffer size in bytes
*
* @retval SII_DRV_ADAPTER_KSV_LOAD__OK Revocation list successfully loaded into the buffer
* @retval SII_DRV_ADAPTER_KSV_LOAD__NOT_AVAILABLE Revocation list is not available due to wrong HDCP state
* @retval SII_DRV_ADAPTER_KSV_LOAD__BUFFER_ERROR  BKSV/RxID buffer is too small to fit all BKSV/RxIDs
*
*****************************************************************************/
SiiDrvAdaptHdcpKsvLoadError_t SiiDrvAdaptRxHdcpRxIdListGet(SiiInst_t inst,
        uint8_t *poBksvNumber,
        uint8_t *poRevocBuffer,
        uint16_t bufferSize);

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
void SiiDrvAdaptRxHdcpUsVerGet(SiiInst_t inst, SiiDrvAdaptHdcpVer_t *poHdcpVer);

void SiiDrvAdaptRxHdcp2xCapabilitySet(SiiInst_t inst, bool_t bVal);

void SiiDrvAdaptRxCdSenseGet(SiiInst_t inst, bool_t * poCdSense);

#ifdef __cplusplus
}
#endif

#endif // SI_DRV_ADAPTER_RX_H


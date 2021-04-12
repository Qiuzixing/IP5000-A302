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
 * @file si_drv_adapter_tx.c
 *
 * @brief HDCP 1.x/2.x to HDCP 2.x/1.x adapter. RX side functions.
 *
 *****************************************************************************/


/***** #include statements ***************************************************/
#include "si_drv_adapter_internal.h"
#include "si_drv_adapter_tx.h"
#include "si_virtual_page.h"

/***** local macro definitions ***********************************************/

/***** local type definitions ************************************************/

/***** global variable declarations ******************************************/

/***** local variable declarations *******************************************/

/***** local function prototypes *********************************************/

/***** local functions *******************************************************/

/***** public functions ******************************************************/

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
void SiiDrvAdaptTmdsSwingLevelSet(SiiInst_t inst, uint8_t tmdsSwingLevel)
{
    DrvAdaptInst_t *pDrvAdapt = DRV_ADAPT_INST2PTR(inst);

    //SII_OS_DEBUG_ASSERT(tmdsSwingLevel < 8); // check if it fits into 4 bits

    SiiDrvVirtualRegModify(pDrvAdapt, TMDS_SWING_LEVEL, SWING_LEVEL_MASK, tmdsSwingLevel << 4);
}

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
void SiiDrvAdaptTxMuteSet(SiiInst_t inst, SiiDrvAdaptMute_t mute)
{
    DrvAdaptInst_t *pDrvAdapt = DRV_ADAPT_INST2PTR(inst);

    switch(mute)
    {
        case SII_DRV_ADAPTER_MUTE__AV_MUTE_OFF:
            SiiDrvVirtualRegModify(pDrvAdapt, AV_CTRL, AV_CTRL_MUTE_MASK, AV_CTRL_AV_MUTE_OFF);
            //DEBUG_PRINT(MSG_DBG,("AV mute off\n"));
            break;

        case SII_DRV_ADAPTER_MUTE__AV_MUTE_ON:
            SiiDrvVirtualRegModify(pDrvAdapt, AV_CTRL, AV_CTRL_MUTE_MASK, AV_CTRL_AV_MUTE_ON);
            //DEBUG_PRINT(MSG_DBG,("AV mute on\n"));
            break;

        case SII_DRV_ADAPTER_MUTE__OFF:
            SiiDrvVirtualRegModify(pDrvAdapt, AV_CTRL, AV_CTRL_MUTE_MASK, AV_CTRL_MUTE_OFF);
            //DEBUG_PRINT(MSG_DBG,("AV mute on\n"));
            break;

        case SII_DRV_ADAPTER_MUTE__VIDEO:
            SiiDrvVirtualRegModify(pDrvAdapt, AV_CTRL, AV_CTRL_MUTE_MASK, AV_CTRL_MUTE_VIDEO);
            //DEBUG_PRINT(MSG_DBG,("AV mute on\n"));
            break;

        case SII_DRV_ADAPTER_MUTE__AUDIO:
            SiiDrvVirtualRegModify(pDrvAdapt, AV_CTRL, AV_CTRL_MUTE_MASK, AV_CTRL_MUTE_AUDIO);
            //DEBUG_PRINT(MSG_DBG,("AV mute on\n"));
            break;

        case SII_DRV_ADAPTER_MUTE__AUDIO_VIDEO:
            SiiDrvVirtualRegModify(pDrvAdapt, AV_CTRL, AV_CTRL_MUTE_MASK, AV_CTRL_MUTE_AUDIO_VIDEO);
            //DEBUG_PRINT(MSG_DBG,("AV mute on\n"));
            break;

        default:
            SII_OS_DEBUG_ASSERT(0);
            break;
    }
}

/*****************************************************************************/
/**
* @brief TMDS output control
*
* Enables or disables TMDS output
*
* @param[in]  inst      Adapter driver instance returned by @ref SiiDrvAdaptCreate()
* @param[in]  tmdsCtrl  TMDS output control
*
*
*****************************************************************************/
void SiiDrvAdaptTxTmdsOutSet(SiiInst_t inst, SiiDrvAdaptTmdsOut_t tmdsCtrl)
{
    DrvAdaptInst_t *pDrvAdapt = DRV_ADAPT_INST2PTR(inst);

    switch(tmdsCtrl)
    {
        case SII_DRV_ADAPT_TMDS_OUT__DISABLE:
            SiiDrvVirtualRegModify(pDrvAdapt, AV_CTRL, TMDS_OUTPUT_CTRL_MASK, TMDS_OUTPUT_DISABLE);
            break;

        case SII_DRV_ADAPT_TMDS_OUT__ENABLE:
            SiiDrvVirtualRegModify(pDrvAdapt, AV_CTRL, TMDS_OUTPUT_CTRL_MASK, TMDS_OUTPUT_ENABLE);
            break;

        default:
            SII_OS_DEBUG_ASSERT(0);
            break;
    }
}

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
SiiDrvAdaptHdcpKsvLoadError_t SiiDrvAdaptTxHdcpAksvGet
    (SiiInst_t inst, uint8_t poAksvBuffer[5])
{

    uint8_t i;
    DrvAdaptInst_t *pDrvAdapt = DRV_ADAPT_INST2PTR(inst);

#if 0
    bool_t bEdidAvailable = (BIT0_MASK == (SiiDrvVirtualPageRead(pDrvAdapt, EDID_STATE)&EDID_AVAILABLE_MASK))?true:false;

    if(!bEdidAvailable)
    {
        DEBUG_PRINT(MSG_DBG,("EDID not available\n"));
        return SII_DRV_ADAPTER_KSV_LOAD__NOT_AVAILABLE;
    }
#endif

    SiiDrvVirtualRegModify(pDrvAdapt, HDCP_READ, READ_AKSV_MASK,READ_AKSV_MASK);
    SiiDrvVirtualPageReadBlock(pDrvAdapt, HDCP_AKSV_FIFO, &poAksvBuffer[0], HDCP_RX_ID_FIFO_LEN);

    for(i = 0; i < 5; i++)
    {
       DEBUG_PRINT(MSG_DBG,("HDCP_AKSV_FIFO: = %02x\n",(int)poAksvBuffer[i]));
    }

    return SII_DRV_ADAPTER_KSV_LOAD__OK;
}

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
        uint16_t length)
{
    DrvAdaptInst_t *pDrvAdapt = DRV_ADAPT_INST2PTR(inst);
    uint16_t EdidOffset = (blockNmb*128 + offset);
    uint16_t i,LocalOffset;

    switch (pDrvAdapt->config.platformId)
    {
        case SII_DRV_ADAPT_PLATFORM__SII9678:
        case SII_DRV_ADAPT_PLATFORM__SII9679:
            SiiDrvVirtualPageWriteWord(pDrvAdapt, EDID_OFFSET_L, EdidOffset);
            SiiDrvVirtualPageReadBlock(pDrvAdapt, EDID_FIFO, poData, length);
            break;

        case SII_DRV_ADAPT_PLATFORM__AVR:
            for (i = 0, LocalOffset = 0; i< length/8; i++)  //
            {
                LocalOffset = EdidOffset + i*8;
                SiiDrvVirtualPageWriteWord(pDrvAdapt, EDID_OFFSET_L, LocalOffset);
                SiiDrvVirtualPageReadBlock(pDrvAdapt, EDID_FIFO, poData + i*8, 8);
            }
            break;

        default:
            //there is a bug
            DEBUG_PRINT(MSG_ALWAYS, ("EDID:: platform ID configure error.\n"));
            break;
    }

}

/*****************************************************************************/
/**
* @brief Trigger SiI9678 to re-read DS EDID data.
*
* will trigger an interrupt to notify host software that the EDID reading has been completed.
*
* @param[in] inst       Adapter driver instance returned by @ref SiiDrvAdaptCreate()
*
*****************************************************************************/
void SiiDrvAdaptTxDsEdidReRead(SiiInst_t inst)
{
    DrvAdaptInst_t *pDrvAdapt = DRV_ADAPT_INST2PTR(inst);
    uint8_t ucRegVal = 0;

    ucRegVal = SiiDrvVirtualPageRead(pDrvAdapt, HDCP_CTRL);

    if (HDCP_TX_EN_MASK != (ucRegVal&HDCP_TX_EN_MASK))
    {
        SiiDrvVirtualRegModify(pDrvAdapt, EDID_CTRL, EDID_REREAD_MASK, EDID_REREAD);
    }
}

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
void SiiDrvAdaptTxHdcpDsVerGet(SiiInst_t inst, SiiDrvAdaptHdcpVer_t *poHdcpVer)
{

    uint8_t linkVer;
    DrvAdaptInst_t *pDrvAdapt = DRV_ADAPT_INST2PTR(inst);

    *poHdcpVer = (SiiDrvAdaptHdcpVer_t)HDCP_TX_LINK_NOHDCP;

    linkVer = SiiDrvVirtualPageRead(pDrvAdapt, HDCP_TX_STATE);
    switch(linkVer&HDCP_TX_LINK_TYPE_MASK)
    {
        case HDCP_TX_LINK_NOHDCP:
            *poHdcpVer = SII_DRV_ADAPTER_HDCP_VER__NOT_SUPPORTED;
            break;

        case HDCP_TX_LINK_HDCP_1x:
            *poHdcpVer = SII_DRV_ADAPTER_HDCP_VER__1x;
            break;

        case HDCP_TX_LINK_HDCP_2:
            *poHdcpVer = SII_DRV_ADAPTER_HDCP_VER__20;
            break;

        case HDCP_TX_LINK_HDCP_2x:
            *poHdcpVer = SII_DRV_ADAPTER_HDCP_VER__22;
            break;

        default:
            SII_OS_DEBUG_ASSERT(0);
            break;
    }

}

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
void SiiDrvAdaptTxLinkTypeGet(SiiInst_t inst, SiiDrvAdaptLinkType_t *poConType)
{
    DrvAdaptInst_t *pDrvAdapt = DRV_ADAPT_INST2PTR(inst);
    SiiDrvAdaptLinkType_t TxConType = SII_DRV_LINK_DISCONNECTED;
    uint8_t TxGeneralState;

    TxGeneralState = SiiDrvVirtualPageRead(pDrvAdapt, TX_GENERAL_STAT);

    switch (TxGeneralState&TX_LINK_TYPE_MASK)
    {
        case TX_LINK_TYPE_DISCONN:
            TxConType = SII_DRV_LINK_DISCONNECTED;
            break;

        case TX_LINK_TYPE_HDMI:
            TxConType = SII_DRV_LINK_HDMI;
            break;

        case TX_LINK_TYPE_MHL12:
            TxConType = SII_DRV_LINK_MHL12;
            break;

        case TX_LINK_TYPE_MHL3:
            TxConType = SII_DRV_LINK_MHL3;
            break;

        default:
            SII_OS_DEBUG_ASSERT(0);
            break;
    }

    *poConType = TxConType;
}

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
void SiiDrvAdaptTxHdcpDsStatusGet(SiiInst_t inst, SiiDrvAdaptHdcpTxStatus_t *poHdcpStatus)
{
    DrvAdaptInst_t *pDrvAdapt = DRV_ADAPT_INST2PTR(inst);
    SiiDrvAdaptHdcpTxStatus_t hdcpStatus = SII_DRV_ADAPTER_HDCP_TX_STATUS__OFF;
    SiiDrvAdaptHdcpVer_t internHdcpVer;
    uint8_t ucHdcpStatus;
    SiiDrvAdaptTxHdcpDsVerGet(inst, &internHdcpVer);

    ucHdcpStatus = SiiDrvVirtualPageRead(pDrvAdapt, HDCP_STATE);

    switch(internHdcpVer)
    {
        case SII_DRV_ADAPTER_HDCP_VER__NOT_SUPPORTED:
            hdcpStatus = SII_DRV_ADAPTER_HDCP_TX_STATUS__OFF;
            break;

        case SII_DRV_ADAPTER_HDCP_VER__1x:
        case SII_DRV_ADAPTER_HDCP_VER__20:
        case SII_DRV_ADAPTER_HDCP_VER__22:
            {
                switch((ucHdcpStatus& HDCP_TX_AUTH_STATE_MASK))
                {
                    case HDCP_TX_STATUS_OFF:
                        hdcpStatus = SII_DRV_ADAPTER_HDCP_TX_STATUS__OFF;
                        break;

                    case HDCP_TX_STATUS_AUTHENTICATING:
                        hdcpStatus = SII_DRV_ADAPTER_HDCP_TX_STATUS__AUTHENTICATING;
                        break;

                    case HDCP_TX_STATUS_AUTHENTICATED:
                        hdcpStatus = SII_DRV_ADAPTER_HDCP_TX_STATUS__SUCCESS;
                        break;

                    case HDCP_TX_STATUS_FAIL:
                        hdcpStatus = SII_DRV_ADAPTER_HDCP_TX_STATUS__FAILED;
                        break;

                    default:
                        SII_OS_DEBUG_ASSERT(0);
                        break;
                }
            }
            break;

        default:
            SII_OS_DEBUG_ASSERT(0);
            break;
    }

    *poHdcpStatus = hdcpStatus;
}

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
void SiiDrvAdaptTxHdcpDsRepeaterBitGet(SiiInst_t inst, bool_t *poIsDsRepeater)
{
    DrvAdaptInst_t *pDrvAdapt = DRV_ADAPT_INST2PTR(inst);
    bool_t bRepeater = false;
    bRepeater = SiiDrvVirtualPageRead(pDrvAdapt, HDCP_CTRL)&HDCP_REPEATER_MASK ? true : false;
    if (bRepeater)
    {
        DEBUG_PRINT(MSG_DBG,("DownStream is a repeater\n"));
    }
    else
    {
        DEBUG_PRINT(MSG_DBG,("DownStream is not a repeater\n"));
    }

    *poIsDsRepeater = bRepeater;
}

/*****************************************************************************/
/**
* @brief Downstream HDCP failure interrogation
*
* Returns the reason of last downstream (TX) HDCP failure.
*
* @note HDCP authentication status returned by @ref SiiDrvAdaptTxHdcpDsStatusGet()
*       can indicate success, but this function returns information about
*       last HDCP error.
*
* @param[in] inst             Adapter driver instance returned by @ref SiiDrvAdaptCreate()
* @param[in] poHdcpFailReason Pointer to a variable to copy HDCP failure reason
*
* @see SiiDrvAdaptTxHdcpDsStatusGet()
*
*****************************************************************************/
void SiiDrvAdaptDsHdcpLastFailureReasonGet(SiiInst_t inst,
        uint8_t *poHdcpFailReason)
{
    DrvAdaptInst_t *pDrvAdapt = DRV_ADAPT_INST2PTR(inst);
    uint8_t failureReason     = (uint8_t)SII_DRV_ADAPTER_HDCP_TX_FAIL_REASON__NONE;
    uint8_t ucErrReasonVal    = SiiDrvVirtualPageRead(pDrvAdapt, HDCP_ERR_REASON);

    if (ucErrReasonVal&HDCP_TX_FAIL_REASON_INTERNAL)
    {
        failureReason |= (uint8_t)SII_DRV_ADAPTER_HDCP_TX_FAIL_REASON__INTERNAL;
    }

    if (ucErrReasonVal&HDCP_TX_FAIL_REASON_CAS_EXC)
    {
        failureReason |= (uint8_t)SII_DRV_ADAPTER_HDCP_TX_FAIL_REASON__CAS_EXC;
    }

    if (ucErrReasonVal&HDCP_TX_FAIL_REASON_DEV_EXC)
    {
        failureReason |= (uint8_t)SII_DRV_ADAPTER_HDCP_TX_FAIL_REASON__DEV_EXC;
    }

    *poHdcpFailReason = failureReason;
}

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
    (SiiInst_t inst, uint8_t poRcvidBuffer[5])
{
    DrvAdaptInst_t *pDrvAdapt = DRV_ADAPT_INST2PTR(inst);
    SiiDrvAdaptHdcpVer_t HdcpVersion = SII_DRV_ADAPTER_HDCP_VER__NOT_SUPPORTED;

#if 0
    uint8_t bEdidAvailable = (BIT0_MASK == (SiiDrvVirtualPageRead(pDrvAdapt, EDID_STATE)&EDID_AVAILABLE_MASK))?true:false;
#endif

    SiiDrvAdaptTxHdcpDsVerGet(inst, &HdcpVersion);

#if 0
    if(!bEdidAvailable)
    {
        DEBUG_PRINT(MSG_DBG,("EDID not available\n"));
        return SII_DRV_ADAPTER_KSV_LOAD__NOT_AVAILABLE;
    }
#endif

    switch (HdcpVersion)
    {
        case SII_DRV_ADAPTER_HDCP_VER__1x:
            SiiDrvVirtualRegModify(pDrvAdapt, HDCP_READ, READ_BKSV_MASK, READ_BKSV_MASK);
            break;

        case SII_DRV_ADAPTER_HDCP_VER__22:
            SiiDrvVirtualRegModify(pDrvAdapt, HDCP_READ, READ_RCVID_MASK, READ_RCVID_MASK);
            break;

        case SII_DRV_ADAPTER_HDCP_VER__20:
        case SII_DRV_ADAPTER_HDCP_VER__21:
        case SII_DRV_ADAPTER_HDCP_VER__NOT_SUPPORTED:
            DEBUG_PRINT(MSG_ALWAYS,("HDCP version not support, BKSV/RcvId get error.\n"));
            break;
    }

    SiiDrvVirtualPageReadBlock(pDrvAdapt, HDCP_RX_ID_FIFO, &poRcvidBuffer[0], HDCP_RX_ID_FIFO_LEN);

    return SII_DRV_ADAPTER_KSV_LOAD__OK;
}

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
void SiiDrvAdaptTxDsEdidStatusGet(SiiInst_t inst, SiiDrvAdaptDsEdidStatus_t *poEdidStatus)
{
    DrvAdaptInst_t *pDrvAdapt = DRV_ADAPT_INST2PTR(inst);
    bool_t bEdidAvailable = false;

    bEdidAvailable = (BIT0_MASK == (SiiDrvVirtualPageRead(pDrvAdapt, EDID_STATE)&EDID_AVAILABLE_MASK))?true:false;

    if (bEdidAvailable)
    {
        *poEdidStatus = SII_DRV_ADAPT_DS_EDID__AVAILABLE;
    }
    else
    {
        *poEdidStatus = SII_DRV_ADAPT_DS_EDID__NOT_AVAILABLE;
    }
}

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
SiiDrvAdaptHdcpKsvLoadError_t SiiDrvAdaptTxHdcpDsRxIdListGet
    (SiiInst_t inst, uint8_t *poRxId, uint16_t ListLen, uint16_t Offset)
{
    DrvAdaptInst_t *pDrvAdapt = DRV_ADAPT_INST2PTR(inst);

    SiiDrvVirtualPageWriteWord(pDrvAdapt, HDCP_RCV_ID_FIFO_OFFSET_L, Offset);
    //SiiDrvVirtualPageWrite(pDrvAdapt, HDCP_RCV_ID_FIFO_LEN, ListLen);
    //SiiDrvVirtualRegModify(pDrvAdapt, HDCP_READ, READ_RCVID_MASK,READ_RCVID_MASK);
    SiiDrvVirtualPageReadBlock(pDrvAdapt, HDCP_RCV_ID_FIFO, poRxId, ListLen);

    return SII_DRV_ADAPTER_KSV_LOAD__OK;
}

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
void SiiDrvAdaptTxHdcpDsTopologyGet(SiiInst_t inst, SiiDrvAdaptHdcpTopology_t *poTopology)
{
    DrvAdaptInst_t *pDrvAdapt = DRV_ADAPT_INST2PTR(inst);

    uint8_t ucSeqNumV[3] = {0, 0, 0};
    uint8_t ucHdcpInfoMisc = SiiDrvVirtualPageRead(pDrvAdapt, HDCP_RX_INFO_MISC);
    SiiDrvAdaptHdcpVer_t HdcpVersion;

    poTopology->bHdcp1DeviceDs = (ucHdcpInfoMisc&HDCP1_DEV_DS_MASK)?true:false;
    poTopology->bHdcp20DeviceDs = (ucHdcpInfoMisc&HDCP20_DEV_DS_MASK)?true:false;

    SiiDrvAdaptTxHdcpDsVerGet(pDrvAdapt, &HdcpVersion);

    if (SII_DRV_ADAPTER_HDCP_VER__22 ==  HdcpVersion)
    {
        poTopology->bMaxCascadeExceeded = (ucHdcpInfoMisc&MAX_CASCADE_EXCEEDED)?true:false;
        poTopology->bMaxDevsExceeded = (ucHdcpInfoMisc&MAX_DEVS_EXCEEDED)?true:false;

        poTopology->depth = SiiDrvVirtualPageRead(pDrvAdapt, HDCP_RX_INFO_DEPTH);
        poTopology->deviceCount = SiiDrvVirtualPageRead(pDrvAdapt,HDCP_RX_INFO_DEV_COUNT);

        SiiDrvVirtualPageReadBlock(pDrvAdapt, HDCP_RX_INFO_SEQ_NUM_V_FIFO, ucSeqNumV, 3);

        poTopology->seqNumV |= (uint32_t)ucSeqNumV[0];
        poTopology->seqNumV |= (((uint32_t)ucSeqNumV[1])<<8);
        poTopology->seqNumV |= (((uint32_t)ucSeqNumV[2])<<16);

        if (poTopology->bHdcp1DeviceDs)
        {
            DEBUG_PRINT(MSG_DBG,("        HDCP 1.x compliant Device in the topology!\n"));
        }

        if (poTopology->bHdcp20DeviceDs)
        {
            DEBUG_PRINT(MSG_DBG,("        HDCP 2.0 compliant Repeater in the topology!\n"));
        }

        if (poTopology->bMaxCascadeExceeded)
        {
            DEBUG_PRINT(MSG_DBG,("        More than 7(for HDCP 1.x) or 4(for HDCP 2.x) levels of repeaters have been cascaded together!\n"));
        }

        if (poTopology->bMaxDevsExceeded)
        {
            DEBUG_PRINT(MSG_DBG,("        More than 127(for HDCP1.X) or 31(for HDCP 2.x) downstream devices are attached!\n"));
        }

        DEBUG_PRINT(MSG_DBG,("        Repeater cascade depth: %02X\n", (int)poTopology->depth));
        DEBUG_PRINT(MSG_DBG,("        Total number of attached downstream devices: %02X\n", (int)poTopology->deviceCount));

    }
    else if (SII_DRV_ADAPTER_HDCP_VER__1x == HdcpVersion)
    {
        poTopology->bMaxCascadeExceeded = (ucHdcpInfoMisc&MAX_CASCADE_EXCEEDED)?true:false;
        poTopology->bMaxDevsExceeded = (ucHdcpInfoMisc&MAX_DEVS_EXCEEDED)?true:false;

        poTopology->depth = SiiDrvVirtualPageRead(pDrvAdapt, HDCP_RX_INFO_DEPTH);
        poTopology->deviceCount = SiiDrvVirtualPageRead(pDrvAdapt,HDCP_RX_INFO_DEV_COUNT);

        if (poTopology->bMaxCascadeExceeded)
        {
            DEBUG_PRINT(MSG_DBG,("        More than 7(for HDCP 1.x) or 4(for HDCP 2.x) levels of repeaters have been cascaded together!\n"));
        }

        if (poTopology->bMaxDevsExceeded)
        {
            DEBUG_PRINT(MSG_DBG,("        More than 127(for HDCP1.X) or 31(for HDCP 2.x) downstream devices are attached!\n"));
        }

        DEBUG_PRINT(MSG_DBG,("        Repeater cascade depth: %d\n", (int)poTopology->depth));
        DEBUG_PRINT(MSG_DBG,("        Total number of attached downstream devices: %d\n", (int)poTopology->deviceCount));
    }
    else
    {
            //version not support
    }
}

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
void SiiDrvAdaptTxHdcpContentTypeSet(SiiInst_t inst, SiiDrvAdaptHdcpContenrType_t contentType)
{
    DrvAdaptInst_t *pDrvAdapt = DRV_ADAPT_INST2PTR(inst);

    switch(contentType)
    {
        case SII_DRV_ADAPTER_HDCP_CONTENT__TYPE0:
            SiiDrvVirtualRegModify(pDrvAdapt, HDCP_CTRL, HDCP_CONTENT_TYPE_MASK, HDCP_CONTENT_TYPE0);
            DEBUG_PRINT(MSG_DBG,("Tx Set HDCP content: Type 0"));
            break;

        case SII_DRV_ADAPTER_HDCP_CONTENT__TYPE1:
            SiiDrvVirtualRegModify(pDrvAdapt, HDCP_CTRL, HDCP_CONTENT_TYPE_MASK, HDCP_CONTENT_TYPE1);
            DEBUG_PRINT(MSG_DBG,("Tx Set HDCP content: Type 1"));
            break;

        case SII_DRV_ADAPTER_HDCP_CONTENT__UNKNOWN:
        default:
            SII_OS_DEBUG_ASSERT(0);
            break;
    }
}

/*****************************************************************************/
/**
* @brief Downstream HDCP enabling control
*
* Enables/disables downstream HDCP.
*
* @note This function is only used for debugging purpose.
* @note HDCP is enabled by default
*
* @param[in] inst        Adapter driver instance returned by @ref SiiDrvAdaptCreate()
* @param[in] protectType HDCP configuration
*   - SII_DRV_ADAPTER_HDCP_PROTECT__OFF   HDCP disabled
*   - SII_DRV_ADAPTER_HDCP_PROTECT__ON    HDCP enabled
*
*****************************************************************************/
void SiiDrvAdaptTxHdcpProtectionSet(SiiInst_t inst, SiiDrvAdaptHdcpProtect_t protectType)
{
    DrvAdaptInst_t *pDrvAdapt = DRV_ADAPT_INST2PTR(inst);

    switch(protectType)
    {
        case SII_DRV_ADAPTER_HDCP_PROTECT__OFF:
            SiiDrvVirtualRegModify(pDrvAdapt, HDCP_CTRL, HDCP_TX_EN_MASK, HDCP_DIS);
            break;

        case SII_DRV_ADAPTER_HDCP_PROTECT__ON:
            SiiDrvVirtualRegModify(pDrvAdapt, HDCP_CTRL, HDCP_TX_EN_MASK, HDCP_EN);
            break;

        default:
            SII_OS_DEBUG_ASSERT(0);
            break;
    }
}

u32 SiiDrvAdaptTxHdcpProtectionGet(SiiInst_t inst)
{
	DrvAdaptInst_t *pDrvAdapt = DRV_ADAPT_INST2PTR(inst);
	u8 val = 0;

	val = SiiDrvVirtualPageRead(pDrvAdapt, HDCP_CTRL);

	if (val & HDCP_TX_EN_MASK)
		return SII_DRV_ADAPTER_HDCP_PROTECT__ON;

	return SII_DRV_ADAPTER_HDCP_PROTECT__OFF;
}

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
void SiiDrvAdaptTxHdcpStreamManageMsgSet(SiiInst_t inst, SiiDrvAdaptHdcpStreamManageInfo_t *poStreamManageInfo)
{
    uint16_t i = 0;
    uint8_t ucSeqNumM[3] = {0, 0, 0};
    uint8_t ucStreamIdType[HDCP_MAX_K*2] = {0, 0, 0, 0, 0, 0, 0, 0};
    DrvAdaptInst_t *pDrvAdapt = DRV_ADAPT_INST2PTR(inst);

    //seq_num_M value is handled by FW, host should always set 0.
    //ucSeqNumM[0] = (uint8_t)poStreamManageInfo->seqNumM;
    //ucSeqNumM[1] = (uint8_t)(poStreamManageInfo->seqNumM>>8);
    //ucSeqNumM[2] = (uint8_t)(poStreamManageInfo->seqNumM>>16);

    SiiDrvVirtualPageWriteBlock(pDrvAdapt, HDCP_SMNG_SEQ_NUM_M_FIFO, ucSeqNumM, 3);
    SiiDrvVirtualPageWriteWord(pDrvAdapt, HDCP_SMNG_VALUE_K_L, poStreamManageInfo->k);

    for (i = 0; i<2*(poStreamManageInfo->k); i+=2)
    {
        //FP2711
        //ucStreamIdType[i]     = (uint8_t)poStreamManageInfo->streamIdType[i];
        //ucStreamIdType[i+1]   = (uint8_t)(poStreamManageInfo->streamIdType[i]>>8);
        ucStreamIdType[i]     = (uint8_t)(poStreamManageInfo->streamIdType[i]>>8);
        ucStreamIdType[i+1]   = (uint8_t)poStreamManageInfo->streamIdType[i];

    }

    SiiDrvVirtualPageWriteBlock(pDrvAdapt, HDCP_SMNG_STREAMID_TYPE_FIFO, ucStreamIdType, (poStreamManageInfo->k)*2);
    SiiDrvVirtualRegModify(pDrvAdapt, HDCP_CTRL, HDCP_SMNG_SET_START, HDCP_SMNG_SET_START);
}
#if 0
void SiiDrvAdaptTxHdcpEventGet(SiiInst_t inst, uint8_t *poTxHdcpEvent)
{
    DrvAdaptInst_t *pDrvAdapt   = DRV_ADAPT_INST2PTR(inst);

    *poTxHdcpEvent = SiiDrvVirtualPageRead(pDrvAdapt, HDCP_TX_INT);
}

void SiiDrvAdaptTxHdcpEventClear(SiiInst_t inst, uint8_t ucTxHdcpEvent)
{
    DrvAdaptInst_t *pDrvAdapt   = DRV_ADAPT_INST2PTR(inst);

    uint8_t retryCount = 200;
    ucTxHdcpEvent &= HDCP_TX_INT_AVAILABLE_MASK;
    SiiDrvVirtualPageWrite(pDrvAdapt, HDCP_TX_INT, ucTxHdcpEvent);
    while ((retryCount--) > 0)
    {
        if (0 == (SiiDrvVirtualPageRead(pDrvAdapt, HDCP_TX_INT) & ucTxHdcpEvent))
        {
            break;
        }
    }
}
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
void SiiDrvAdaptTxRxSenseStatusGet(SiiInst_t inst, SiiDrvAdaptTxRxSenseType_t *poRxStatus)
{
    DrvAdaptInst_t *pDrvAdapt = DRV_ADAPT_INST2PTR(inst);
    SiiDrvAdaptTxRxSenseType_t TxRxSenseStatus = SII_DRV_ADAPTER_TX_NO_RXSENSED;
    uint8_t TxGeneralState;

    TxGeneralState = SiiDrvVirtualPageRead(pDrvAdapt, TX_GENERAL_STAT);

    switch (TxGeneralState&TX_RXSENSE_STATUS_MASK)
    {
        case TX_NO_RXSENSED:
            TxRxSenseStatus = SII_DRV_ADAPTER_TX_NO_RXSENSED;
            break;

        case TX_RXSENSED:
            TxRxSenseStatus = SII_DRV_ADAPTER_TX_RXSENSED;
            break;

        default:
            SII_OS_DEBUG_ASSERT(0);
            break;
    }

    *poRxStatus = TxRxSenseStatus;
}

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
bool_t SiiDrvAdaptTxTmdsAnalogPowerStatusGet(SiiInst_t inst)
{
    DrvAdaptInst_t *pDrvAdapt = DRV_ADAPT_INST2PTR(inst);
    bool_t bResult = SiiDrvVirtualPageRead(pDrvAdapt, DEBUG_TX_TMDS_CTRL)&TX_TMDS_ENABLE_ANALOG_POWER_MASK ? true : false;

    return bResult;
}

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
bool_t SiiDrvAdaptTxPowerDownIdckStatusGet(SiiInst_t inst)
{
    DrvAdaptInst_t *pDrvAdapt = DRV_ADAPT_INST2PTR(inst);
    bool_t bResult = SiiDrvVirtualPageRead(pDrvAdapt, DEBUG_DIAGNOSTICS_POWER_DOWN)&BIT2_MASK ? true : false;

    return bResult;
}


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
 * @param[in]  Offset  Offset in EDID SRAM memory
 * @param[out] poData  Pointer to output buffer
 * @param[in]  length  Number of bytes to read
 *
 * @see SiiDrvAdaptEdidSramWrite()
 * @see SiiDrvAdaptAccessStatusGet()
 *
 *****************************************************************************/
void SiiDrvAdaptRxEdidSramRead(SiiInst_t inst, uint16_t offset, uint8_t  *poData, uint16_t length)
{
	DrvAdaptInst_t *pDrvAdapt = DRV_ADAPT_INST2PTR(inst);
	uint16_t i, LocalOffset;

	switch (pDrvAdapt->config.platformId) {
	case SII_DRV_ADAPT_PLATFORM__SII9678:
	case SII_DRV_ADAPT_PLATFORM__SII9679:
		SiiDrvVirtualPageWrite(pDrvAdapt, EDID_OFFSET_H, (offset>>8));
		SiiDrvVirtualPageWrite(pDrvAdapt, EDID_OFFSET_L, (uint8_t)(offset&0x00FF));
		SiiDrvVirtualPageReadBlock(pDrvAdapt, EDID_FIFO, poData, length);
		break;
	case SII_DRV_ADAPT_PLATFORM__AVR:  //Now on this pc platform there is limitation to read/write EDID
		for (i = 0, LocalOffset = 0; i < length/8; i++) {
			LocalOffset = offset + i*8;
			SiiDrvVirtualPageWrite(pDrvAdapt, EDID_OFFSET_L, (uint8_t)LocalOffset);
			SiiDrvVirtualPageWrite(pDrvAdapt, EDID_OFFSET_H, (uint8_t)(LocalOffset>>8));
			SiiDrvVirtualPageReadBlock(pDrvAdapt, EDID_FIFO, poData + i*8, 8);
		}
		break;
	default:
		//there is a bug
		DEBUG_PRINT(MSG_ALWAYS, ("EDID:: platform ID configure error.\n"));
		break;
	}
}

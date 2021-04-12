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
 * @file si_drv_adapter_rx.c
 *
 * @brief HDCP 1.x/2.x to HDCP 2.x/1.x adapter. RX side functions.
 *
 *****************************************************************************/


/***** #include statements ***************************************************/
#include "si_drv_adapter_rx.h"
#include "si_drv_adapter_internal.h"
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
void SiiDrvAdaptRxLinkTypeGet(SiiInst_t inst, SiiDrvAdaptLinkType_t *poConType)
{
    DrvAdaptInst_t *pDrvAdapt = DRV_ADAPT_INST2PTR(inst);
    SiiDrvAdaptLinkType_t RxConType = SII_DRV_LINK_DISCONNECTED;
    uint8_t RxGeneralState = SiiDrvVirtualPageRead(pDrvAdapt, RX_GENERAL_STATE);

    switch ((RxGeneralState & RX_LINK_STATUS_MASK))
    {
        case NO_CONNECTED:
            RxConType = SII_DRV_LINK_DISCONNECTED;
            break;

        case HDMI_CONNECTED:
            RxConType = SII_DRV_LINK_HDMI;
            break;

        case MHL12_CONNECTED:
            RxConType = SII_DRV_LINK_MHL12;
            break;
        case MHL3_CONNECTED:
            RxConType = SII_DRV_LINK_MHL3;
            break;

        default:
            SII_OS_DEBUG_ASSERT(0);
            break;
    }

    *poConType = RxConType;
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
void SiiDrvAdaptRxHdcpUsVerGet(SiiInst_t inst, SiiDrvAdaptHdcpVer_t *poHdcpVer)
{

    uint8_t linkVer;
    DrvAdaptInst_t *pDrvAdapt = DRV_ADAPT_INST2PTR(inst);

    *poHdcpVer = SII_DRV_ADAPTER_HDCP_VER__NOT_SUPPORTED;

    linkVer = SiiDrvVirtualPageRead(pDrvAdapt, HDCP_STATE)&HDCP_RX_US_LINK_MASK;
    switch(linkVer)
    {
        case LINK_NOHDCP:
            *poHdcpVer = SII_DRV_ADAPTER_HDCP_VER__NOT_SUPPORTED;
            break;

        case LINK_HDCP_1x:
            *poHdcpVer = SII_DRV_ADAPTER_HDCP_VER__1x;
            break;

        case LINK_HDCP_2:
            *poHdcpVer = SII_DRV_ADAPTER_HDCP_VER__20;
            break;

        case LINK_HDCP_2x:
            *poHdcpVer = SII_DRV_ADAPTER_HDCP_VER__22;
            break;

        default:
            SII_OS_DEBUG_ASSERT(0);
            break;
    }
}

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
void SiiDrvAdaptRxHpdSet(SiiInst_t inst, SiiDrvAdaptInpCtrl_t hpd)
{
    DrvAdaptInst_t *pDrvAdapt = DRV_ADAPT_INST2PTR(inst);

    switch (hpd)
    {
        case SII_DRV_ADAPT_INP_CTRL__ENABLE:
            SiiDrvVirtualRegModify(pDrvAdapt, RX_CTRL, RX_HPD_CTRL_MASK, RX_HPD_CTRL_HIGH);
            break;

        case SII_DRV_ADAPT_INP_CTRL__DISABLE:
            SiiDrvVirtualRegModify(pDrvAdapt, RX_CTRL, RX_HPD_CTRL_MASK, RX_HPD_CTRL_LOW);
            break;

        case SII_DRV_ADAPT_INP_CTRL__BYPASS:
            // no break here
        default:
            SII_OS_DEBUG_ASSERT(0);
            break;
    }
}

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
void SiiDrvAdaptRxTermSet(SiiInst_t inst, SiiDrvAdaptInpCtrl_t term)
{
    DrvAdaptInst_t *pDrvAdapt = DRV_ADAPT_INST2PTR(inst);

    switch (term)
    {
        case SII_DRV_ADAPT_INP_CTRL__ENABLE:
            SiiDrvVirtualRegModify(pDrvAdapt, RX_CTRL, TERM_CTRL_MASK, TERM_CTRL_ENABLE);
            break;

        case SII_DRV_ADAPT_INP_CTRL__DISABLE:
            SiiDrvVirtualRegModify(pDrvAdapt, RX_CTRL, TERM_CTRL_MASK, TERM_CTRL_DISABLE);
            break;

        case SII_DRV_ADAPT_INP_CTRL__BYPASS:
            //no break here
        default:
            SII_OS_DEBUG_ASSERT(0);
            break;
    }
}

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
void SiiDrvAdaptRxCbusImpedanceSet(SiiInst_t inst, SiiDrvAdaptCbusRxImpedance_t CbusImpedance)
{
    DrvAdaptInst_t *pDrvAdapt = DRV_ADAPT_INST2PTR(inst);

    switch (CbusImpedance)
    {
        case SII_DRV_ADAPTER_CBUS_RX_IMPEDANCE__1K:
            SiiDrvVirtualRegModify(pDrvAdapt, CBUS_RX_CTRL, RX_CBUS_IMPEDANCE_MASK, RX_CBUS_IMPEDANCE_1K);
            break;

        case SII_DRV_ADAPTER_CBUS_RX_IMPEDANCE__HI_Z:
            SiiDrvVirtualRegModify(pDrvAdapt, CBUS_RX_CTRL, RX_CBUS_IMPEDANCE_MASK, RX_CBUS_IMPEDANCE_HI_Z);
            break;

        case SII_DRV_ADAPTER_CBUS_RX_IMPEDANCE__BYPASS:
            SiiDrvVirtualRegModify(pDrvAdapt, CBUS_RX_CTRL, RX_CBUS_IMPEDANCE_MASK, RX_CBUS_IMPEDANCE_BYPASS);
            break;

        default:
            SII_OS_DEBUG_ASSERT(0);
            break;
    }
}

/*****************************************************************************/
/**
* @brief RX DDC control, both HDCP and EDID DDC can be controlled by this function.
*
* Control HDCP&EDID DDC registers accessibility from upstream.
*
* @param[in]  inst  Adapter driver instance returned by @ref SiiDrvAdaptCreate()
* @param[in]  ddc   RX DDC state
*                     - @ref SII_DRV_ADAPT_INP_CTRL__DISABLE RX HDCP&EDID DDC is disabled
*                     - @ref SII_DRV_ADAPT_INP_CTRL__ENABLE  RX HDCP&EDID DDC is enabled
*                     - @ref SII_DRV_ADAPT_INP_CTRL__BYPASS  Currently not supported
*
*****************************************************************************/
void SiiDrvAdaptHdcpDdcSet(SiiInst_t inst, SiiDrvAdaptInpCtrl_t ddc)
{
    DrvAdaptInst_t *pDrvAdapt = DRV_ADAPT_INST2PTR(inst);

    switch (ddc)
    {
        case SII_DRV_ADAPT_INP_CTRL__ENABLE:
            SiiDrvVirtualRegModify(pDrvAdapt, HDCP_CTRL_1, RX_DDC_CTRL_MASK, RX_DDC_CTRL_ENABLE);
            break;

        case SII_DRV_ADAPT_INP_CTRL__DISABLE:
            SiiDrvVirtualRegModify(pDrvAdapt, HDCP_CTRL_1, RX_DDC_CTRL_MASK, RX_DDC_CTRL_DISABLE);
            break;

        case SII_DRV_ADAPT_INP_CTRL__BYPASS:
            //no break here
        default:
            SII_OS_DEBUG_ASSERT(0);
            break;
    }
}

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
void SiiDrvAdaptRxHdcpRepeaterModeSet(SiiInst_t inst, bool_t repeaterMode)
{
    DrvAdaptInst_t *pDrvAdapt = DRV_ADAPT_INST2PTR(inst);

    if (repeaterMode)
    {
        SiiDrvVirtualRegModify(pDrvAdapt, HDCP_CTRL, HDCP_SET_REPEATER_BIT_MASK, HDCP_SET_REPEATER_BIT_MASK);
    }
    else
    {
        SiiDrvVirtualRegModify(pDrvAdapt, HDCP_CTRL, HDCP_SET_REPEATER_BIT_MASK, 0x00);
    }
}

/*****************************************************************************/
/**
* @brief Upstream HDCP status interrogation
*
* Returns current upstream HDCP status.
*
* @param[in] inst Adapter driver instance returned by @ref SiiDrvAdaptCreate()
* @param[in] poHdcpStatus Pointer to a variable to copy HDCP status
*
* @see SiiDrvAdaptUsHdcpLastFailureReasonGet()
*
*****************************************************************************/
void SiiDrvAdaptRxHdcpUsStatusGet(SiiInst_t inst, SiiDrvAdaptHdcpRxStatus_t *poHdcpStatus)
{
    DrvAdaptInst_t *pDrvAdapt = DRV_ADAPT_INST2PTR(inst);
    SiiDrvAdaptHdcpRxStatus_t retVal = SII_DRV_ADAPTER_HDCP_RX_STATUS__FAIL;

    switch (SiiDrvVirtualPageRead(pDrvAdapt, HDCP_RX_STATE_MACHINE) &  RX_HDCP_AUTH_STATE_MASK)
    {
        case HDCP_RX_STATUS_OFF:
            retVal = SII_DRV_ADAPTER_HDCP_RX_STATUS__OFF;
            break;

        case HDCP_RX_STATUS_AUTHENTICATED:
            retVal = SII_DRV_ADAPTER_HDCP_RX_STATUS__SUCCESS;
            break;

        case HDCP_RX_STATUS_AUTHENTICATING:
            retVal = SII_DRV_ADAPTER_HDCP_RX_STATUS__AUTHENTICATING;
            break;

        case HDCP_RX_STATUS_FAIL:
            retVal = SII_DRV_ADAPTER_HDCP_RX_STATUS__FAIL;
            break;

        default:
            SII_OS_DEBUG_ASSERT(0);
            break;
    }

    *poHdcpStatus = retVal;
}

/*****************************************************************************/
/**
* @brief Upstream HDCP 1.X BKSV interrogation
*
* Returns upstream (own) BKSV for HDCP 1.X.
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
SiiDrvAdaptHdcpKsvLoadError_t SiiDrvAdaptRxHdcpBksvGet(SiiInst_t inst, uint8_t poBksvBuffer[5])
{
    DrvAdaptInst_t *pDrvAdapt = DRV_ADAPT_INST2PTR(inst);

    SiiDrvVirtualPageWrite(pDrvAdapt, HDCP_READ, READ_RX_BKSV_MASK);
    SiiDrvVirtualPageReadBlock(pDrvAdapt, HDCP_RX_BKSV_FIFO, &poBksvBuffer[0], HDCP_RX_ID_FIFO_LEN);


    return SII_DRV_ADAPTER_KSV_LOAD__OK;
}

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
SiiDrvAdaptHdcpKsvLoadError_t SiiDrvAdaptRxHdcpRxIdGet(SiiInst_t inst, uint8_t poRxIdBuffer[5])
{
    DrvAdaptInst_t *pDrvAdapt = DRV_ADAPT_INST2PTR(inst);
    uint8_t i = 0;

    SiiDrvVirtualPageWrite(pDrvAdapt, HDCP_READ, READ_RX_ID_MASK);
    SiiDrvVirtualPageReadBlock(pDrvAdapt, HDCP_RX_ID_FIFO, poRxIdBuffer, HDCP_RX_ID_FIFO_LEN);

    for (i =0; i<5; i++)
    {
        DEBUG_PRINT(MSG_DBG,("SiiDrvAdaptRxHdcpRxIdGet::Rx ID: %02x\n",(int)poRxIdBuffer[i]));
    }

    return SII_DRV_ADAPTER_KSV_LOAD__OK;
}

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
void SiiDrvAdaptCkdtStatusGet(SiiInst_t inst, bool_t *poCkdtStatus)
{
    DrvAdaptInst_t *pDrvAdapt = DRV_ADAPT_INST2PTR(inst);

    *poCkdtStatus =
            (SiiDrvVirtualPageRead(pDrvAdapt, CHIP_GENERAL_INT_STATUS) & CKDT_STATE_MASK) ?
                    true : false;
}

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
void SiiDrvAdaptScdtStatusGet(SiiInst_t inst, bool_t *poScdtDetStatus)
{
    DrvAdaptInst_t *pDrvAdapt = DRV_ADAPT_INST2PTR(inst);

    *poScdtDetStatus =
            (SiiDrvVirtualPageRead(pDrvAdapt, CHIP_GENERAL_INT_STATUS) & SCDT_STATE_MASK) ?
                    true : false;
}

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
void SiiDrvAdaptRxAvMuteStatusGet(SiiInst_t inst, SiiDrvAdaptMute_t *poMuteStat)
{
    uint8_t regVal;
    DrvAdaptInst_t *pDrvAdapt = DRV_ADAPT_INST2PTR(inst);

    regVal = SiiDrvVirtualPageRead(pDrvAdapt, MISC_STATUS);

    switch (regVal&TX_AV_MUTE_STATUS_MASK)
    {
        case TX_AV_MUTE_STATUS_OFF:
            *poMuteStat = SII_DRV_ADAPTER_MUTE__AV_MUTE_OFF;
            break;

        case TX_AV_MUTE_STATUS_ON:
            *poMuteStat = SII_DRV_ADAPTER_MUTE__AV_MUTE_ON;
            break;

        default:
            SII_OS_DEBUG_ASSERT(0);
            break;
    }

}

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
void SiiDrvAdaptRxPlus5vStatusGet(SiiInst_t inst, bool_t *poPlus5vStat)
{
    DrvAdaptInst_t *pDrvAdapt = DRV_ADAPT_INST2PTR(inst);

    *poPlus5vStat = (SiiDrvVirtualPageRead(pDrvAdapt, RX_GENERAL_STATE) & PWR5V_STATE_MASK) ? true:false;
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
void SiiDrvAdaptRxEdidSramRead(SiiInst_t inst,
        uint16_t offset,
        uint8_t  *poData,
        uint16_t length)
{
    DrvAdaptInst_t *pDrvAdapt = DRV_ADAPT_INST2PTR(inst);
    uint16_t i,LocalOffset;

    switch (pDrvAdapt->config.platformId)
    {
        case SII_DRV_ADAPT_PLATFORM__SII9678:
        case SII_DRV_ADAPT_PLATFORM__SII9679:
            SiiDrvVirtualPageWrite(pDrvAdapt, EDID_OFFSET_H, (offset>>8));
            SiiDrvVirtualPageWrite(pDrvAdapt, EDID_OFFSET_L, (uint8_t)(offset&0x00FF));
            SiiDrvVirtualPageReadBlock(pDrvAdapt, EDID_FIFO, poData, length);
            break;

        case SII_DRV_ADAPT_PLATFORM__AVR:  //Now on this pc platform there is limitation to read/write EDID
            for (i = 0, LocalOffset = 0; i< length/8; i++)  //
            {
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
        uint16_t length)
{

    DrvAdaptInst_t *pDrvAdapt = DRV_ADAPT_INST2PTR(inst);
    uint16_t i,LocalOffset;

    switch (pDrvAdapt->config.platformId)
    {
        case SII_DRV_ADAPT_PLATFORM__SII9678:
        case SII_DRV_ADAPT_PLATFORM__SII9679:
            SiiDrvVirtualPageWriteWord(pDrvAdapt, EDID_OFFSET_L, offset);
            SiiDrvVirtualPageWriteBlock(pDrvAdapt, EDID_FIFO, pData, length);
            break;

        case SII_DRV_ADAPT_PLATFORM__AVR: //Now on this pc platform there is limitation to read/write EDID
            for (i = 0, LocalOffset = 0; i< length/8; i++)  //
            {
                LocalOffset = offset + i*8;
                SiiDrvVirtualPageWriteWord(pDrvAdapt, EDID_OFFSET_L, LocalOffset);
                SiiDrvVirtualPageWriteBlock(pDrvAdapt, EDID_FIFO, pData + i*8, 8);
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
        uint16_t length)
{
    #define EEPROM_FIFO_SIZE 4
    DrvAdaptInst_t *pDrvAdapt = DRV_ADAPT_INST2PTR(inst);

    uint8_t i = 0;
    uint8_t LocalLen = (uint8_t)(length/EEPROM_FIFO_SIZE);
    uint8_t LastLen = (uint8_t)(length%EEPROM_FIFO_SIZE);

    (void)SiiOsTaskSleepUsec(10);
    SiiDrvVirtualRegModify(pDrvAdapt, EDID_CTRL, EDID_LOCAL_I2C_MASK, EDID_LOCAL_I2C_ENABLE);

    for (i = 0; i < LocalLen; i++)
    {
        (void)SiiOsTaskSleepUsec(10);  //Limitation of EEPROM
        SiiDrvRegReadBlock(pDrvAdapt, 0xA0, (offset+i*EEPROM_FIFO_SIZE), &poData[i*EEPROM_FIFO_SIZE], EEPROM_FIFO_SIZE);
    }

    if (0 != LastLen)
    {
        (void)SiiOsTaskSleepUsec(10);
        SiiDrvRegReadBlock(pDrvAdapt, 0xA0, (offset+i*EEPROM_FIFO_SIZE), &poData[i*EEPROM_FIFO_SIZE], LastLen);
    }

    //SiiHalTimerWait(10);
    //SiiDrvRegReadBlock(pDrvAdapt, 0xA0, offset, poData, length);
    //SiiHalTimerWait(10);

    SiiDrvVirtualRegModify(pDrvAdapt, EDID_CTRL, EDID_LOCAL_I2C_MASK, EDID_LOCAL_I2C_DISABLE);
}

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
        uint16_t length)
{
    #define EEPROM_FIFO_SIZE 4
    DrvAdaptInst_t *pDrvAdapt = DRV_ADAPT_INST2PTR(inst);
    uint8_t i = 0;
    uint8_t LocalLen = (uint8_t)(length/EEPROM_FIFO_SIZE);
    uint8_t LastLen = (uint8_t)(length%EEPROM_FIFO_SIZE);

    SiiDrvVirtualRegModify(pDrvAdapt, EDID_CTRL, EDID_LOCAL_I2C_MASK, EDID_LOCAL_I2C_ENABLE);

    for (i = 0; i < LocalLen; i++)
    {
        SiiDrvRegWriteBlock(pDrvAdapt, 0xA0, (offset+i*EEPROM_FIFO_SIZE), &poData[i*EEPROM_FIFO_SIZE], EEPROM_FIFO_SIZE);
        (void)SiiOsTaskSleepUsec(10);  //Limitation of EEPROM
    }

    if (0 != LastLen)
    {
        SiiDrvRegWriteBlock(pDrvAdapt, 0xA0, (offset+i*EEPROM_FIFO_SIZE), &poData[i*EEPROM_FIFO_SIZE], LastLen);
    }

    SiiDrvVirtualRegModify(pDrvAdapt, EDID_CTRL, EDID_LOCAL_I2C_MASK, EDID_LOCAL_I2C_DISABLE);
}

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
void SiiDrvAdaptRxHdcpRepeaterModeGet(SiiInst_t inst, bool_t *poIsDsRepeater)
{
    DrvAdaptInst_t *pDrvAdapt = DRV_ADAPT_INST2PTR(inst);
    bool_t bRepeater =false;

    bRepeater = SiiDrvVirtualPageRead(pDrvAdapt, HDCP_CTRL)&HDCP_SET_REPEATER_BIT_MASK ? true : false;

    *poIsDsRepeater = bRepeater;
}

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
void SiiDrvAdaptRxHdcpRcvIdListValidGet(SiiInst_t inst, bool_t *poValid)
{
    uint8_t ucRcvIdListValid;
    DrvAdaptInst_t *pDrvAdapt = DRV_ADAPT_INST2PTR(inst);

    ucRcvIdListValid = SiiDrvVirtualPageRead(pDrvAdapt, HDCP_STATE);

    if (ucRcvIdListValid&HDCP_RX_RECEIVER_ID_LIST_VALID_STATUS_MASK)
    {
        *poValid = true;
        DEBUG_PRINT(MSG_DBG,("Rx HDCP Receiver ID list is valid.\n"));
    }
    else
    {
        *poValid = false;
        DEBUG_PRINT(MSG_DBG,("Rx HDCP Receiver ID list is invalid.\n"));
    }
}

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
void SiiDrvAdaptRxHdcpTopologyGet(SiiInst_t inst, SiiDrvAdaptHdcpTopology_t *poTopology)
{
    DrvAdaptInst_t *pDrvAdapt   = DRV_ADAPT_INST2PTR(inst);
    uint8_t ucSeqNumV[4] = {0, 0, 0, 0};
    uint8_t i = 0;
    uint8_t ucTopologyMiscInfo  = SiiDrvVirtualPageRead(pDrvAdapt, HDCP_RX_TOPOLOGY_MISC);

    poTopology->bHdcp1DeviceDs      = (ucTopologyMiscInfo&HDCP1_TOPOLOGY_DEV_DS_MASK)?true:false;
    poTopology->bHdcp20DeviceDs     = (ucTopologyMiscInfo&HDCP20_TOPOLOGY_DEV_DS_MASK)?true:false;
    poTopology->bMaxCascadeExceeded = (ucTopologyMiscInfo&TOPOLOGY_MAX_CASCADE_EXCEEDED)?true:false;
    poTopology->bMaxDevsExceeded    = (ucTopologyMiscInfo&TOPOLOGY_MAX_DEVS_EXCEEDED)?true:false;
    poTopology->deviceCount         = SiiDrvVirtualPageRead(pDrvAdapt, HDCP_RX_TOPOLOGY_DEV_COUNT);
    poTopology->depth               = SiiDrvVirtualPageRead(pDrvAdapt, HDCP_RX_TOPOLOGY_DEPTH);

    SiiDrvVirtualPageReadBlock(pDrvAdapt, HDCP_RX_TOPOLOGY_SEQ_NUM_V_FIFO, ucSeqNumV, 4);

    poTopology->seqNumV |= (uint32_t)ucSeqNumV[0];
    poTopology->seqNumV |= (((uint32_t)ucSeqNumV[1])<<8);
    poTopology->seqNumV |= (((uint32_t)ucSeqNumV[2])<<16);
    poTopology->seqNumV |= (((uint32_t)ucSeqNumV[3])<<24);

    if (poTopology->bHdcp1DeviceDs)
    {
        DEBUG_PRINT(MSG_DBG,("HDCP 1.X compliant device in the topology\n"));
    }
    else
    {
        DEBUG_PRINT(MSG_DBG,("No HDCP 1.X compliant device in the topology\n"));
    }

    if (poTopology->bHdcp20DeviceDs)
    {
        DEBUG_PRINT(MSG_DBG,("HDCP 2.0 compliant device in the topology\n"));
    }
    else
    {
        DEBUG_PRINT(MSG_DBG,("No HDCP 2.0 compliant device in the topology\n"));
    }

    if (poTopology->bMaxCascadeExceeded)
    {
        DEBUG_PRINT(MSG_DBG,("More than seven level for HDCP 1.X or four levels for HDCP 2.X of repeaters cascaded together\n"));
    }
    else
    {
        DEBUG_PRINT(MSG_DBG,("Normal cascade\n"));
    }

    if (poTopology->bMaxDevsExceeded)
    {
        DEBUG_PRINT(MSG_DBG,("More than 31 devices (for HDCP1.X) or 127 devices (for HDCP 2.X) as attached\n"));
    }
    else
    {
        DEBUG_PRINT(MSG_DBG,("Normal device count\n"));
    }

    DEBUG_PRINT(MSG_DBG,("Total number of attached downstream devices: %d",(int)(poTopology->deviceCount)));
    DEBUG_PRINT(MSG_DBG,("Repeater cascade depth: %d",(int)(poTopology->depth)));

    for (i = 0; i<4; i++)
    {
        DEBUG_PRINT(MSG_DBG,("seqNumV = %02x\n",(int)ucSeqNumV[i]));
    }

}

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
void SiiDrvAdaptRxHdcpRiReset(SiiInst_t inst)
{
    DrvAdaptInst_t *pDrvAdapt   = DRV_ADAPT_INST2PTR(inst);

    SiiDrvVirtualRegModify(pDrvAdapt, HDCP_CTRL_1, HDCP_RX_RESET_RI_ENABLE, HDCP_RX_RESET_RI_ENABLE);
}

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
        uint16_t bufferSize)
{
    DrvAdaptInst_t *pDrvAdapt   = DRV_ADAPT_INST2PTR(inst);
    uint8_t i = 0;

    SiiDrvVirtualPageWrite(pDrvAdapt, HDCP_RX_ID_LIST_BKSV_NUM, *poBksvNumber);
    //SiiDrvVirtualPageWrite(pDrvAdapt, HDCP_RX_ID_LIST_FIFO_SIZE_L, (uint8_t)bufferSize);
    //SiiDrvVirtualPageWrite(pDrvAdapt, HDCP_RX_ID_LIST_FIFO_SIZE_H, (uint8_t)(bufferSize>>8));
    SiiDrvVirtualPageReadBlock(pDrvAdapt, HDCP_RX_ID_LIST_FIFO, poRevocBuffer, (*poBksvNumber)*5);

    for (i=0; i<bufferSize; i++)
    {
        DEBUG_PRINT(MSG_DBG,("RX ID LIST: %02x\n",(int)poRevocBuffer[i]));
    }

    return SII_DRV_ADAPTER_KSV_LOAD__OK;
}

/*****************************************************************************/
/**
* @brief Upstream HDCP 2.X Stream Manage info Interrogation
*
* @note This function is only expected to be called when Rx is used in AVR repeater case, and attached downstream device is also a repeater.
*
* @param[in]   inst                 Adapter driver instance returned by @ref SiiDrvAdaptCreate()
* @param[out]  poStreamManageInfo   Get HDCP 2.X stream manage info
*
*****************************************************************************/
void SiiDrvAdaptRxHdcpStreamManageMsgGet(SiiInst_t inst, SiiDrvAdaptHdcpStreamManageInfo_t *poStreamManageInfo)
{
    DrvAdaptInst_t *pDrvAdapt   = DRV_ADAPT_INST2PTR(inst);
    uint8_t ucSeqNumM[3] = {0, 0, 0};
    uint8_t i = 0;
    uint8_t ucStreamIdType[HDCP_RX_MAX_K*2] = {0, 0, 0, 0, 0, 0, 0, 0};

    memset(poStreamManageInfo, 0x00, sizeof(SiiDrvAdaptHdcpStreamManageInfo_t));

    //SiiDrvVirtualPageWrite(pDrvAdapt, HDCP_CTRL_1, READ_RX_SMNG_MASK);

    SiiDrvVirtualPageReadBlock(pDrvAdapt, HDCP_RX_SMNG_SEQ_NUM_M_FIFO, ucSeqNumM, 3);
    poStreamManageInfo->seqNumM |= (uint32_t)ucSeqNumM[0];
    poStreamManageInfo->seqNumM |= (((uint32_t)ucSeqNumM[1])<<8);
    poStreamManageInfo->seqNumM |= (((uint32_t)ucSeqNumM[2])<<16);

    poStreamManageInfo->k  |= (uint16_t)SiiDrvVirtualPageRead(pDrvAdapt, HDCP_RX_SMNG_VALUE_K_L);
    poStreamManageInfo->k  |= (((uint16_t)SiiDrvVirtualPageRead(pDrvAdapt, HDCP_RX_SMNG_VALUE_K_H))<<8);

    SiiDrvVirtualPageReadBlock(pDrvAdapt, HDCP_RX_SMNG_STREAMID_TYPE_FIFO, ucStreamIdType, HDCP_RX_MAX_K*2);

    //sample: ucStreamIdType[0]:ID, ucStreamIdType[1]:Type. streamIdType[0] =  ucStreamIdType[0]|ucStreamIdType[1]
    for (i = 0; i < HDCP_RX_MAX_K; i++)
    {
        poStreamManageInfo->streamIdType[i] = ((uint16_t)ucStreamIdType[i*2])|((uint16_t)(ucStreamIdType[i*2+1]<<8));
    }

    DEBUG_PRINT(MSG_DBG,("Get DS Stream_Manage Message:\n"));

    DEBUG_PRINT(MSG_DBG, ("        SeqNumM: %02X %02X %02X\n",
        (int)ucSeqNumM[0], (int)ucSeqNumM[1], (int)ucSeqNumM[2]));

    DEBUG_PRINT(MSG_DBG,("    K: %04X\n",(int)poStreamManageInfo->k));

    DEBUG_PRINT(MSG_DBG, ("        StreamIdType: %02X %02X %02X %02X\n",
        (int)ucStreamIdType[1], (int)ucStreamIdType[0], (int)ucStreamIdType[3], (int)ucStreamIdType[2]));

    DEBUG_PRINT(MSG_DBG, ("        StreamIdType: %02X %02X %02X %02X\n",
        (int)ucStreamIdType[5], (int)ucStreamIdType[4], (int)ucStreamIdType[7],(int)ucStreamIdType[6]));
}

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
void SiiDrvAdaptEdidDdcSet(SiiInst_t inst, SiiDrvAdaptInpCtrl_t ddc)
{
    DrvAdaptInst_t *pDrvAdapt = DRV_ADAPT_INST2PTR(inst);

    switch (ddc)
    {
        case SII_DRV_ADAPT_INP_CTRL__DISABLE:
            SiiDrvVirtualRegModify(pDrvAdapt, RX_CTRL, RX_EDID_DDC_CTRL_MASK, RX_EDID_DDC_CTRL_DISABLE);
            break;

        case SII_DRV_ADAPT_INP_CTRL__ENABLE:
            SiiDrvVirtualRegModify(pDrvAdapt, RX_CTRL, RX_EDID_DDC_CTRL_MASK, RX_EDID_DDC_CTRL_ENABLE);
            break;

        case SII_DRV_ADAPT_INP_CTRL__BYPASS:
            // no break here
        default:
            SII_OS_DEBUG_ASSERT(0);
            break;
    }

}

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
void SiiDrvAdaptRxHdcpTopologySet(SiiInst_t inst, SiiDrvAdaptHdcpTopology_t *poTopology)
{
    uint8_t ucTopologyMisc      = 0x00;
    uint8_t ucSeqNumV[3]        = {0, 0, 0};
    DrvAdaptInst_t *pDrvAdapt   = DRV_ADAPT_INST2PTR(inst);
    SiiDrvAdaptHdcpVer_t HdcpVersion = SII_DRV_ADAPTER_HDCP_VER__NOT_SUPPORTED;

    SiiDrvAdaptRxHdcpUsVerGet(inst, &HdcpVersion);

    if (SII_DRV_ADAPTER_HDCP_VER__22 == HdcpVersion)    // HDCP2.2
    {
        if (poTopology->bHdcp1DeviceDs)
        {
            ucTopologyMisc |= HDCP1_TOPOLOGY_DEV_DS_MASK;
        }

        if (poTopology->bHdcp20DeviceDs)
        {
            ucTopologyMisc |= HDCP20_TOPOLOGY_DEV_DS_MASK;
        }

        if (poTopology->bMaxCascadeExceeded)
        {
            ucTopologyMisc |= TOPOLOGY_MAX_CASCADE_EXCEEDED;
        }

        if (poTopology->bMaxDevsExceeded)
        {
            ucTopologyMisc |= TOPOLOGY_MAX_DEVS_EXCEEDED;
        }

        ucSeqNumV[0] = (uint8_t)poTopology->seqNumV;
        ucSeqNumV[1] = (uint8_t)((poTopology->seqNumV)>>8);
        ucSeqNumV[2] = (uint8_t)((poTopology->seqNumV)>>16);

        SiiDrvVirtualPageWrite(pDrvAdapt, HDCP_RX_TOPOLOGY_MISC, ucTopologyMisc);
        SiiDrvVirtualPageWrite(pDrvAdapt, HDCP_RX_TOPOLOGY_DEV_COUNT, poTopology->deviceCount);
        SiiDrvVirtualPageWrite(pDrvAdapt, HDCP_RX_TOPOLOGY_DEPTH, poTopology->depth);
        SiiDrvVirtualPageWriteBlock(pDrvAdapt, HDCP_RX_TOPOLOGY_SEQ_NUM_V_FIFO, ucSeqNumV, 3);
        SiiDrvVirtualRegModify(pDrvAdapt, HDCP_CTRL_1, HDCP_RX_SET_TOPOLOGY_START, HDCP_RX_SET_TOPOLOGY_START);
    }
    else if (SII_DRV_ADAPTER_HDCP_VER__1x == HdcpVersion)    // HDCP1.x
    {
        if (poTopology->bMaxCascadeExceeded)
        {
            ucTopologyMisc |= TOPOLOGY_MAX_CASCADE_EXCEEDED;
        }

        if (poTopology->bMaxDevsExceeded)
        {
            ucTopologyMisc |= TOPOLOGY_MAX_DEVS_EXCEEDED;
        }

        SiiDrvVirtualPageWrite(pDrvAdapt, HDCP_RX_TOPOLOGY_MISC, ucTopologyMisc);
        SiiDrvVirtualPageWrite(pDrvAdapt, HDCP_RX_TOPOLOGY_DEV_COUNT, poTopology->deviceCount);
        SiiDrvVirtualPageWrite(pDrvAdapt, HDCP_RX_TOPOLOGY_DEPTH, poTopology->depth);
        SiiDrvVirtualRegModify(pDrvAdapt, HDCP_CTRL_1, HDCP_RX_SET_TOPOLOGY_START, HDCP_RX_SET_TOPOLOGY_START);
    }
    else
    {
        // not support version
    }
}
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
void SiiDrvAdaptRxHdcpRxIdListPtrSet(SiiInst_t inst,  uint16_t prtOffset)
{
    DrvAdaptInst_t *pDrvAdapt   = DRV_ADAPT_INST2PTR(inst);

    SiiDrvVirtualPageWrite(pDrvAdapt, HDCP_RX_ID_LIST_OFFSET_L, (uint8_t)prtOffset);
    SiiDrvVirtualPageWrite(pDrvAdapt, HDCP_RX_ID_LIST_OFFSET_H, (uint8_t)(prtOffset>>8));
}
#endif

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
void SiiDrvAdaptRxHdcpRxIdListSet(SiiInst_t inst,  uint8_t deviceCnt, uint8_t *poDsRxIdList, uint16_t offset)
{
    DrvAdaptInst_t *pDrvAdapt   = DRV_ADAPT_INST2PTR(inst);

    SiiDrvVirtualPageWrite(pDrvAdapt, HDCP_RX_ID_LIST_BKSV_NUM, deviceCnt);

    //work around
    //limitation of Master I2C driver(PC with black box), only support 5 bytes read once
    //so be care of deviceCnt when you call this function.
    SiiDrvVirtualPageWriteWord(pDrvAdapt, HDCP_RX_ID_LIST_FIFO_SIZE_L, offset);
    SiiDrvVirtualPageWriteBlock(pDrvAdapt, HDCP_RX_ID_LIST_FIFO, poDsRxIdList, deviceCnt*5);

}

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
void SiiDrvAdaptRxHdcpRxIdListTriggerToSend(SiiInst_t inst)
{
    DrvAdaptInst_t *pDrvAdapt   = DRV_ADAPT_INST2PTR(inst);
    SiiDrvAdaptHdcpVer_t HdcpVersion;

    SiiDrvAdaptRxHdcpUsVerGet(inst, &HdcpVersion);
    if (SII_DRV_ADAPTER_HDCP_VER__1x == HdcpVersion)
    {
        SiiDrvVirtualRegModify(pDrvAdapt, HDCP_CTRL_1, HDCP_RX_RECEIVER_ID_LIST_VALID_MASK, HDCP_RX_RECEIVER_ID_LIST_VALID_MASK);
    }
    else
    {
        SiiDrvVirtualRegModify(pDrvAdapt, HDCP_CTRL_1, HDCP_RX_SET_RCVID_LIST_START, HDCP_RX_SET_RCVID_LIST_START);
    }
}

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
void SiiDrvAdaptRxHdcpStreamManageMsgSet(SiiInst_t inst, SiiDrvAdaptHdcpStreamManageInfo_t *poStreamManageInfo)
{
    DrvAdaptInst_t *pDrvAdapt   = DRV_ADAPT_INST2PTR(inst);
    uint8_t ucSeqNumM[4] = {0, 0, 0, 0};
    uint8_t i = 0;
    uint8_t ucStreamIdType[HDCP_RX_MAX_K*2] = {0, 0, 0, 0, 0, 0, 0, 0};

    ucSeqNumM[0] = (uint8_t)(poStreamManageInfo->seqNumM);
    ucSeqNumM[1] = (uint8_t)((poStreamManageInfo->seqNumM)>>8);
    ucSeqNumM[2] = (uint8_t)((poStreamManageInfo->seqNumM)>>16);
    ucSeqNumM[3] = (uint8_t)((poStreamManageInfo->seqNumM)>>24);

    SiiDrvVirtualPageWriteBlock(pDrvAdapt, HDCP_RX_SMNG_SEQ_NUM_M_FIFO, ucSeqNumM, 4);
    SiiDrvVirtualPageWrite(pDrvAdapt, HDCP_RX_SMNG_VALUE_K_L, (uint8_t)(poStreamManageInfo->k));
    SiiDrvVirtualPageWrite(pDrvAdapt, HDCP_RX_SMNG_VALUE_K_H, (uint8_t)(poStreamManageInfo->k>>8));

    for (i=0; i<2*(poStreamManageInfo->k); i+=2)
    {
        ucStreamIdType[i]     = (uint8_t)(poStreamManageInfo->streamIdType[i]);
        ucStreamIdType[i+1]   = (uint8_t)(poStreamManageInfo->streamIdType[i]>>8);
    }

    SiiDrvVirtualPageWriteBlock(pDrvAdapt, HDCP_RX_SMNG_STREAMID_TYPE_FIFO, ucStreamIdType, (poStreamManageInfo->k)*2);
    SiiDrvVirtualPageWrite(pDrvAdapt, HDCP_CTRL_1 , HDCP_RX_SET_SMNG_START);
}


void SiiDrvAdaptRxHdcp2xCapabilitySet(SiiInst_t inst, bool_t bVal)
{
    DrvAdaptInst_t *pDrvAdapt   = DRV_ADAPT_INST2PTR(inst);

    if (bVal)
    {
        SiiDrvVirtualRegModify(pDrvAdapt, HDCP_CTRL_1, HDCP_RX_HDCP2X_CAP_ENABLE_MASK, HDCP_RX_HDCP2X_CAP_ENABLE);
    }
    else
    {
        SiiDrvVirtualRegModify(pDrvAdapt, HDCP_CTRL_1, HDCP_RX_HDCP2X_CAP_ENABLE_MASK, HDCP_RX_HDCP2X_CAP_DISABLE);
    }
}

void SiiDrvAdaptRxCdSenseGet(SiiInst_t inst, bool_t * poCdSense)
{
    DrvAdaptInst_t *pDrvAdapt   = DRV_ADAPT_INST2PTR(inst);

    *poCdSense = (SiiDrvVirtualPageRead(pDrvAdapt, RX_GENERAL_STATE)&CD_SENSE_STATUS) ? true : false;
}


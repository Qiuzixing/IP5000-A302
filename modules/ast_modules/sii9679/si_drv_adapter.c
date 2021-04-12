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
 * @file si_drv_adapter.c
 *
 * @brief @brief HDCP 1.x/2.x to HDCP 2.x/1.x adapter. Functions common for RX and TX sides.
 *
 *****************************************************************************/


/***** #include statements ***************************************************/
#include "si_drv_adapter_internal.h"
#include "si_virtual_page.h"
#include "si_drv_adapter_flash.h"

/***** local macro definitions ***********************************************/

/***** local type definitions ************************************************/

/***** global variable declarations ******************************************/

/***** local variable declarations *******************************************/
static DrvAdaptInst_t drvAdaptInstRecord[MAX_NMB_OF_ADAPTER_INSTANCES];
static uint8_t Page0Addr = 0x60;
//static SiiDrvAdaptEventsInterface_t DrvAdapterEvents;

/***** local function prototypes *********************************************/

/***** local functions *******************************************************/
// Driver initialization
static void DrvConfigure(DrvAdaptInst_t *pDrvAdapt)
{
    pDrvAdapt->accessStatus = SII_DRV_ADAPTER_ACCESS__SUCCESS;

    SiiDrvVirtualRegModify(pDrvAdapt, INT_STATE, INT_POLARITY_MASK,
            ((uint8_t)(pDrvAdapt->config.interruptPolarity) << 1));
    SiiDrvVirtualRegModify(pDrvAdapt, INT_STATE, INT_OD_MASK,
            ((uint8_t)(pDrvAdapt->config.interruptPinType) << 2));

    switch (pDrvAdapt->config.platformId)
    {
        case SII_DRV_ADAPT_PLATFORM__SII9679:
             SiiDrvVirtualPageWrite(pDrvAdapt, MHL3_INT_MASK, 0xFF);
        case SII_DRV_ADAPT_PLATFORM__SII9678:
            //enable interrupt mask
            SiiDrvVirtualPageWrite(pDrvAdapt, CHIP_GENERAL_INT_MASK,0xFF);
            SiiDrvVirtualPageWrite(pDrvAdapt, HDCP_RX_INT_MASK, 0xFF);
            SiiDrvVirtualPageWrite(pDrvAdapt, HDCP_TX_INT_MASK, 0xFF);
            SiiDrvVirtualPageWrite(pDrvAdapt, EDID_INT_MASK, EDID_CHG_INT_MASK);
            SiiDrvVirtualPageWrite(pDrvAdapt, CBUS_INT_MASK, 0xFF);
            SiiDrvVirtualPageWrite(pDrvAdapt, CBUS_INT_1_MASK, 0xFF);
            SiiDrvVirtualPageWrite(pDrvAdapt, TX_GENERAL_INT_MASK, 0xFF);
            SiiDrvVirtualPageWrite(pDrvAdapt, RX_GENERAL_INT_MASK, 0xFF);
            break;

        case SII_DRV_ADAPT_PLATFORM__AVR:
            SiiDrvVirtualRegModify(pDrvAdapt, MODE_CONFIG, CHIP_AVR_MODE_MASK, CHIP_AVR_MODE_MASK);//set AVR mode control
            //enable interrupt mask
            SiiDrvVirtualPageWrite(pDrvAdapt, CHIP_GENERAL_INT_MASK, 0xFF);
            SiiDrvVirtualPageWrite(pDrvAdapt, HDCP_RX_INT_MASK, HDCP_RX_AUTH_DONE_INT|HDCP_RX_RCVID_DONE_INT|HDCP_RX_SMNG_CHG_INT|HDCP_RX_AUTH_FAIL|HDCP_RX_AUTH_START);
            SiiDrvVirtualPageWrite(pDrvAdapt, HDCP_TX_INT_MASK, HDCP_TX_AUTH_DONE_INT|HDCP_TX_RCVID_CHG_INT|HDCP_TX_SMNG_DONE_INT|HDCP_TX_STATUS_CHG_INT|HDCP_TX_AUTH_FAIL_INT);

            SiiDrvVirtualPageWrite(pDrvAdapt, TX_GENERAL_INT_MASK, TX_CONN_ON_INT_MASK|TX_RXSENSE_CHG_MASK|TX_CONN_OFF_INT_MASK);
            SiiDrvVirtualPageWrite(pDrvAdapt, EDID_INT_MASK, EDID_CHG_INT_MASK);
            SiiDrvVirtualPageWrite(pDrvAdapt, CBUS_INT_MASK, 0xFF);
            SiiDrvVirtualPageWrite(pDrvAdapt, CBUS_INT_1_MASK, 0xFF);
            break;

        default:
            //there is a bug
            DEBUG_PRINT(MSG_ALWAYS, ("Platform ID configure error\n"));
            break;
    }
}

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
SiiInst_t SiiDrvAdaptCreate(SiiInst_t irqDrvInst, uint8_t hwId, SiiLibEvtCbFunc_t pCallback)
{
    DrvAdaptInst_t *pDrvAdapt = &drvAdaptInstRecord[hwId]; // or use malloc() instead

    irqDrvInst= irqDrvInst;
    // Instance record initialization
    pDrvAdapt->instCra              = hwId;
    pDrvAdapt->pNtfCallback         = pCallback;
    pDrvAdapt->accessStatus         = SII_DRV_ADAPTER_ACCESS__IN_PROGRESS;
    //pDrvAdapt->isBootingInProgress  = true;
    pDrvAdapt->upgradeMode          = false;
    pDrvAdapt->isError              = false;

    return (SiiInst_t) pDrvAdapt;
}

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
void SiiDrvAdaptDelete(SiiInst_t inst)
{
    //DrvAdaptInst_t *pDrvAdapt = DRV_ADAPT_INST2PTR(inst);
    inst = inst;
    // If pDrvAdapt was allocated through malloc(), release memory here
}

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
void SiiDrvAdaptConfigure(SiiInst_t inst, SiiDrvAdaptConfig_t *pCfg)
{
    DrvAdaptInst_t *pDrvAdapt = DRV_ADAPT_INST2PTR(inst);
    pDrvAdapt->config = *pCfg;

    //after boot done, this flag is set
    //pDrvAdapt->isBootingInProgress = false;

    // Used default I2C address
    if (pDrvAdapt->config.i2cSlaveAddr == 0 )
    {
        pDrvAdapt->config.i2cSlaveAddr = SII_DRV_ADAPT_DEFAULT_I2C_DEVICE_ID;
    }

    if( pDrvAdapt->config.i2cSlaveAddrF0 == 0 )
    {
        pDrvAdapt->config.i2cSlaveAddrF0 = SII_DRV_ADAPT_DEFAULT_I2C_DEVICE_ID_F0;
    }

    if( pDrvAdapt->config.i2cSlaveAddrAA == 0 )
    {
        pDrvAdapt->config.i2cSlaveAddrAA = SII_DRV_ADAPT_DEFAULT_I2C_DEVICE_ID_AA;
    }
}

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
void SiiDrvAdaptStart(SiiInst_t inst)
{
    DrvAdaptInst_t *pDrvAdapt = DRV_ADAPT_INST2PTR(inst);

    //boot done, then enable all interrupt
    //if (!(pDrvAdapt->isBootingInProgress))
    {
        DrvConfigure(pDrvAdapt);
    }

    //Make firmware start to run
    SiiDrvVirtualRegModify(pDrvAdapt, MODE_CONFIG, CHIP_FW_START_MODE, CHIP_FW_START_MODE);
}

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
void SiiDrvAdaptStop(SiiInst_t inst)
{
    DrvAdaptInst_t *pDrvAdapt = DRV_ADAPT_INST2PTR(inst);

    // Disable interrupts
    //if(!pDrvAdapt->isBootingInProgress)
    {
        SiiDrvVirtualPageWrite(pDrvAdapt, CHIP_GENERAL_INT_MASK, 0x00);
        SiiDrvVirtualPageWrite(pDrvAdapt, CBUS_INT_MASK, 0x00);
        SiiDrvVirtualPageWrite(pDrvAdapt, HDCP_RX_INT_MASK, 0x00);
        SiiDrvVirtualPageWrite(pDrvAdapt, HDCP_TX_INT_MASK, 0x00);
        SiiDrvVirtualPageWrite(pDrvAdapt, EDID_INT_MASK, 0x00);
        SiiDrvVirtualPageWrite(pDrvAdapt, RX_GENERAL_INT_MASK, 0x00);
    }

    SiiDrvVirtualRegModify(pDrvAdapt, MODE_CONFIG, CHIP_FW_START_MODE, 0x00);
}

static void DrvAdaptNtfSend(DrvAdaptInst_t *pDrvAdapt, uint32_t EventAddr)
{
    SiiDrvAdaptEvents_t DrvAdapterEvents;
    bool_t bEvents;

    memcpy(&DrvAdapterEvents, (SiiDrvAdaptEvents_t*)EventAddr, sizeof(SiiDrvAdaptEvents_t));

    bEvents = (DrvAdapterEvents.GeneralEvents|DrvAdapterEvents.TxEvents|
                      DrvAdapterEvents.RxEvents|DrvAdapterEvents.HdcpEvents|
                      DrvAdapterEvents.CbusEvents|DrvAdapterEvents.CecEvents|
                      DrvAdapterEvents.EdidEvents)?true:false;
    if (pDrvAdapt->pNtfCallback && bEvents)
    {
        pDrvAdapt->pNtfCallback(DRV_ADAPT_PTR2INST(pDrvAdapt), EventAddr);
    }
}

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
void SiiDrvAdaptFirmwareUpdateInit(SiiInst_t inst)
{
	DrvAdaptInst_t *pDrvAdapt = DRV_ADAPT_INST2PTR(inst);

    SiiDrvAdaptBootStatus_t bootstat;

    SiiDrvAdaptBootStatusGet(inst,&bootstat);

	if (I2C_ADDR_DEBUG == pDrvAdapt->config.i2cSlaveAddr)
		bootstat = SII_DRV_ADAPTER_BOOTING__FAILURE;

	SiiSpiFlashInit(inst, bootstat);

    // Erase
    SiiSpiFlashChipErase(inst);
}

/* copy and modify from SiiDrvAdaptFirmwareUpdateInit */
void SiiDrvAdaptFirmwareReadInit(SiiInst_t inst)
{
	SiiDrvAdaptBootStatus_t bootstat;

	SiiDrvAdaptBootStatusGet(inst, &bootstat);
	SiiSpiFlashInit(inst, bootstat);
}

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
void SiiDrvAdaptFirmwareUpdate(SiiInst_t inst, SiiDrvAdaptFwBuffer_t *pFwBuffer)
{
	DrvAdaptInst_t *pDrvAdapt = DRV_ADAPT_INST2PTR(inst);
	SiiDrvAdaptEvents_t AdaptEvents;

	memset(&AdaptEvents, 0, sizeof(SiiDrvAdaptEvents_t));

	if (SPI_MAX_PAGE_SIZE < pFwBuffer->ucBufferSize) {
		SiiSpiFlashBlockProgram(inst, pFwBuffer->ucOffset + SPI_MAX_PAGE_SIZE, pFwBuffer->pBuffer + SPI_MAX_PAGE_SIZE, pFwBuffer->ucBufferSize - SPI_MAX_PAGE_SIZE);
		SiiSpiFlashBlockProgram(inst, pFwBuffer->ucOffset, pFwBuffer->pBuffer, SPI_MAX_PAGE_SIZE);
	} else {
		SiiSpiFlashBlockProgram(inst, pFwBuffer->ucOffset, pFwBuffer->pBuffer, pFwBuffer->ucBufferSize);
	}

	AdaptEvents.GeneralEvents |= SII_DRV_ADAPT_EVENT__ACCESS_DONE;
	DrvAdaptNtfSend(pDrvAdapt, (uint32_t)&AdaptEvents);
}

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
void SiiDrvAdaptTaskExecute(SiiInst_t inst)
{
    DrvAdaptInst_t *pDrvAdapt = DRV_ADAPT_INST2PTR(inst);
    SiiDrvAdaptEvents_t DrvAdaptEvents;

    // Read interrupts
    uint8_t ucGeneralInt        = SiiDrvVirtualPageRead(pDrvAdapt, CHIP_GENERAL_INT);
    uint8_t ucTxGeneralInt      = SiiDrvVirtualPageRead(pDrvAdapt, TX_GENERAL_INT);
    uint8_t ucRxGeneralInt      = SiiDrvVirtualPageRead(pDrvAdapt, RX_GENERAL_INT);
    uint8_t ucHdcpTxInt         = SiiDrvVirtualPageRead(pDrvAdapt, HDCP_TX_INT);
    uint8_t ucHdcpRxInt         = SiiDrvVirtualPageRead(pDrvAdapt, HDCP_RX_INT);
    uint8_t ucEdidInt           = SiiDrvVirtualPageRead(pDrvAdapt, EDID_INT);
    uint8_t ucMhl3Int           = 0;

    if(SII_DRV_ADAPT_PLATFORM__SII9678 != pDrvAdapt->config.platformId)
    {
        ucMhl3Int = SiiDrvVirtualPageRead(pDrvAdapt, MHL3_INT);
    }

    memset(&DrvAdaptEvents, 0, sizeof(SiiDrvAdaptEvents_t));

    if (ucGeneralInt & HW_BOOT_DONE_INT)
    {
        // chip boot done event
        DrvAdaptEvents.GeneralEvents |= SII_DRV_ADAPT_EVENT__BOOT_DONE;
        SiiDrvAdaptStart(inst); //if unexpected HW reset, should configure interrupt mask and notify FW continue to run.
        SiiDrvVirtualPageWrite(pDrvAdapt, CHIP_GENERAL_INT, HW_BOOT_DONE_INT);
    }

    if (ucGeneralInt & RX_CONN_CHG_INT)
    {
        DrvAdaptEvents.RxEvents |= SII_DRV_ADAPT_EVENT__US_CONNECT_STATUS_CHNG;
        SiiDrvVirtualPageWrite(pDrvAdapt, CHIP_GENERAL_INT, RX_CONN_CHG_INT);
    }

    if (ucGeneralInt & CKDT_CHG_INT)
    {
        DrvAdaptEvents.GeneralEvents |= SII_DRV_ADAPT_EVENT__CKDT_STATUS_CHNG;
        SiiDrvVirtualPageWrite(pDrvAdapt, CHIP_GENERAL_INT, CKDT_CHG_INT);
    }

    if (ucGeneralInt & SCDT_CHG_INT)
    {
        DrvAdaptEvents.GeneralEvents |= SII_DRV_ADAPT_EVENT__SCDT_STATUS_CHNG;
        SiiDrvVirtualPageWrite(pDrvAdapt, CHIP_GENERAL_INT, SCDT_CHG_INT);
    }

    if(SII_DRV_ADAPT_PLATFORM__SII9678 != pDrvAdapt->config.platformId)
    {
        if (ucGeneralInt & CBUS_EVENT_INT)
        {
            DrvAdaptEvents.GeneralEvents |= SII_DRV_ADAPT_EVENT__CBUS_EVENT;
        }

        if (ucGeneralInt & CBUS_STATUS_CHG_INT)
        {
            DrvAdaptEvents.GeneralEvents |= SII_DRV_ADAPT_EVENT__CBUS_STATE_CHNG;
            SiiDrvVirtualPageWrite(pDrvAdapt, CHIP_GENERAL_INT, CBUS_STATUS_CHG_INT);
        }

        if (ucGeneralInt & VBUS_REQ_INT)
        {
            DrvAdaptEvents.GeneralEvents |= SII_DRV_ADAPT_EVENT__VBUS_REQUEST;
        }
    }

    if (ucGeneralInt & CD_SENSE_INT)
    {
        DrvAdaptEvents.GeneralEvents |= SII_DRV_ADAPT_EVENT__RX_CD_SENSE_STATUS_CHG;
        SiiDrvVirtualPageWrite(pDrvAdapt, CHIP_GENERAL_INT, CD_SENSE_INT);
    }

    if (ucTxGeneralInt & (TX_RXSENSE_CHG_INT | TX_CONN_OFF_INT | TX_CONN_ON_INT))
    {
        //DEBUG_PRINT(MSG_ALWAYS, ("ucTxGeneralInt =%x\n",ucTxGeneralInt));
        SiiDrvVirtualPageWrite(pDrvAdapt, TX_GENERAL_INT, ucTxGeneralInt);
        if (ucTxGeneralInt & TX_RXSENSE_CHG_INT)
        {
            DrvAdaptEvents.TxEvents |= SII_DRV_ADAPT_EVENT__RX_SENSE_STATUS_CHNG;
        }

        if (ucTxGeneralInt & TX_CONN_OFF_INT)
        {
            DrvAdaptEvents.TxEvents |= SII_DRV_ADAPT_EVENT__DS_CONNECT_STATUS_CHNG_OFF;
        }

        if (ucTxGeneralInt & TX_CONN_ON_INT)
        {
            DrvAdaptEvents.TxEvents |= SII_DRV_ADAPT_EVENT__DS_CONNECT_STATUS_CHNG_ON;
        }
    }

    if (ucRxGeneralInt & RX_AV_MUTE_STATUS_CHG_INT)
    {
        //DEBUG_PRINT(MSG_DBG,("RECEIVED AV MUTE CHG INT\n"));
        DrvAdaptEvents.RxEvents |= SII_DRV_ADAPT_EVENT__RX_AV_MUTE_STATUS_CHNG;
        SiiDrvVirtualPageWrite(pDrvAdapt, RX_GENERAL_INT, RX_AV_MUTE_STATUS_CHG_INT);
    }

    if (ucHdcpTxInt & (HDCP_TX_AUTH_DONE_INT|HDCP_TX_SMNG_DONE_INT|HDCP_TX_RCVID_CHG_INT|HDCP_TX_STATUS_CHG_INT|HDCP_TX_AUTH_FAIL_INT))
    {
        SiiDrvVirtualPageWrite(pDrvAdapt, HDCP_TX_INT, ucHdcpTxInt);

        if (ucHdcpTxInt & HDCP_TX_AUTH_DONE_INT)
        {
            DrvAdaptEvents.HdcpEvents |= SII_DRV_ADAPT_HDCP_EVENT__TX_AUTH_DONE;
        }
        if (ucHdcpTxInt & HDCP_TX_RCVID_CHG_INT)
        {
            DrvAdaptEvents.HdcpEvents |= SII_DRV_ADAPT_HDCP_EVENT__TX_RCVID_CHG;
        }
        if (ucHdcpTxInt & HDCP_TX_SMNG_DONE_INT)
        {
            DrvAdaptEvents.HdcpEvents |= SII_DRV_ADAPT_HDCP_EVENT__TX_SMNG_DONE;
        }
        if (ucHdcpTxInt & HDCP_TX_STATUS_CHG_INT)
        {
            DrvAdaptEvents.HdcpEvents |= SII_DRV_ADAPT_HDCP_EVENT__TX_STATUS_CHG;
        }
        if (ucHdcpTxInt & HDCP_TX_AUTH_FAIL_INT)
        {
            DrvAdaptEvents.HdcpEvents |= SII_DRV_ADAPT_HDCP_EVENT__TX_AUTH_FAIL;
        }

    }

    if (ucHdcpRxInt&(HDCP_RX_AUTH_DONE_INT|HDCP_RX_RCVID_DONE_INT|HDCP_RX_SMNG_CHG_INT|HDCP_RX_AUTH_FAIL|HDCP_RX_AUTH_START))
    {
        SiiDrvVirtualPageWrite(pDrvAdapt, HDCP_RX_INT, ucHdcpRxInt);

        if (ucHdcpRxInt & HDCP_RX_AUTH_START)
        {
            DrvAdaptEvents.HdcpEvents |= SII_DRV_ADAPT_HDCP_EVENT__RX_AUTH_START;
        }
        if (ucHdcpRxInt & HDCP_RX_AUTH_DONE_INT)
        {
            DrvAdaptEvents.HdcpEvents |= SII_DRV_ADAPT_HDCP_EVENT__RX_AUTH_DONE;
        }
        if (ucHdcpRxInt & HDCP_RX_RCVID_DONE_INT)
        {
            DrvAdaptEvents.HdcpEvents |= SII_DRV_ADAPT_HDCP_EVENT__RX_RCVID_DONE;
        }
        if (ucHdcpRxInt & HDCP_RX_SMNG_CHG_INT)
        {
            DrvAdaptEvents.HdcpEvents |= SII_DRV_ADAPT_HDCP_EVENT__RX_SMNG_CHG;
        }
        if (ucHdcpRxInt & HDCP_RX_AUTH_FAIL)
        {
            DrvAdaptEvents.HdcpEvents |= SII_DRV_ADAPT_HDCP_EVENT__RX_AUTH_FAIL;
        }

        /*
        if (ucHdcpRxInt & HDCP_RX_STATUS_CHG_INT)
        {
            DrvAdaptEvents.RxEvents |= SII_DRV_ADAPT_EVENT__RX_HDCP_STATUS_CHNG;
        }
        */
    }

    if (ucEdidInt & EDID_CHG_INT_MASK)
    {
        //Edid change
        DrvAdaptEvents.EdidEvents |= SII_DRV_ADAPT_EVENT__DS_EDID_STATUS_CHNG;
        SiiDrvVirtualPageWrite(pDrvAdapt, EDID_INT, EDID_CHG_INT_MASK);  //clear EDID change inte
    }

    if (ucMhl3Int & MHL_VER_CHG_INT)
    {
        DrvAdaptEvents.CbusEvents |= SII_DRV_ADAPT_EVENT__MHL_VER_CHNG;
        SiiDrvVirtualPageWrite(pDrvAdapt, MHL3_INT, MHL_VER_CHG_INT_MASK);
    }

    if (ucMhl3Int & MHL_CBUS_MODE_CHG_INT)
    {
        DrvAdaptEvents.CbusEvents |= SII_DRV_ADAPT_EVENT__CBUS_MODE_CHNG;
        SiiDrvVirtualPageWrite(pDrvAdapt, MHL3_INT, MHL_CBUS_MODE_CHG_INT_MASK);
    }

    DrvAdaptNtfSend(pDrvAdapt, (uint32_t)&DrvAdaptEvents);
}

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
void SiiDrvAdaptBootStatusGet(SiiInst_t inst, SiiDrvAdaptBootStatus_t *poBootStat)
{
    DrvAdaptInst_t          *pDrvAdapt  = DRV_ADAPT_INST2PTR(inst);
    SiiDrvAdaptVersionInfo_t DeviceInfo = {0,0,0,0};
    SiiDrvAdaptBootStatus_t BootStat;
    uint8_t                 RegVal;
    uint8_t FwVerH, FwVerM, FwVerL;

    (void)SiiDrvAdaptVersionGet(inst, &DeviceInfo);

    if( (DeviceInfo.chipId == 0x9394) || (DeviceInfo.chipId == 0x9679))
    {
        FwVerH = (DeviceInfo.fwVersion       ) >> 13;
        FwVerM = (uint8_t)((DeviceInfo.fwVersion&0x1FFF) >> 5);
        FwVerL = (DeviceInfo.fwVersion&0x001F)     ;

        if( FwVerH == 0 )
        {
            if( FwVerM == 0x50 ) // Version 0.50.xx is the FW based on 1.00.03 with UI updated
            {
                RegVal      = SiiDrvVirtualPageRead(pDrvAdapt, CHIP_GENERAL_INT_STATUS);
            }
            else // other 0.xx.xx Bit assignment is also different
            {
                RegVal      = SiiDrvVirtualPageRead(pDrvAdapt, OLD_CHIP_GENERAL_INT_STATUS);
                RegVal = (RegVal & 0x08) ? BOOT_SUCCESS : 0;
            }
        }
        else if( FwVerH == 1 )
        {
            if( FwVerM == 0 )
            {
                if( FwVerL <= 7 )
                {
                    RegVal      = SiiDrvVirtualPageRead(pDrvAdapt, OLD_CHIP_GENERAL_INT_STATUS);
                }
                else
                {
                    RegVal      = SiiDrvVirtualPageRead(pDrvAdapt, CHIP_GENERAL_INT_STATUS);
                }
            }
            else
            {
                RegVal      = SiiDrvVirtualPageRead(pDrvAdapt, CHIP_GENERAL_INT_STATUS);
            }
        }
        else // VerH > 1
        {
            RegVal      = SiiDrvVirtualPageRead(pDrvAdapt, CHIP_GENERAL_INT_STATUS);
        }
    }
    else if (DeviceInfo.chipId == 0x9678)
    {
        RegVal      = SiiDrvVirtualPageRead(pDrvAdapt, CHIP_GENERAL_INT_STATUS);
    }
    else
    {
        *poBootStat = SII_DRV_ADAPTER_BOOTING__FAILURE;
        return;  //if get version failed, then will returned here.
    }

    BootStat    = SII_DRV_ADAPTER_BOOTING__FAILURE;
    // RegVal      = SiiDrvVirtualPageRead(pDrvAdapt, CHIP_GENERAL_INT_STATUS);

    switch(RegVal & BSM_MASK)
    {
        case BOOT_INPROGRESS:
            BootStat = SII_DRV_ADAPTER_BOOTING__IN_PROGRESS;
            //pDrvAdapt->isBootingInProgress = true;
            break;

        case BOOT_FAILURE:
            BootStat = SII_DRV_ADAPTER_BOOTING__FAILURE;
            //pDrvAdapt->isBootingInProgress = false;
            break;

        case BOOT_SUCCESS:
            BootStat = SII_DRV_ADAPTER_BOOTING__SUCCESS;
            //pDrvAdapt->isBootingInProgress = false;
            break;

        default:
            SII_OS_DEBUG_ASSERT(0);
            break;
    }

    *poBootStat = BootStat;
}

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
uint8_t SiiDrvAdaptVersionGet(SiiInst_t inst, SiiDrvAdaptVersionInfo_t *poVersionInfo)
{
    uint8_t chipIdL;
    uint8_t chipIdH;
    uint8_t fwVerL;
    uint8_t fwVerH;
    uint8_t RetVal = (uint8_t)SII_OS_STATUS_SUCCESS;

    DrvAdaptInst_t *pDrvAdapt = DRV_ADAPT_INST2PTR(inst);

    chipIdL = SiiDrvVirtualPageRead(pDrvAdapt, CHIP_ID_LOW);

    //firstly visit I2C, store I2C access status here to notify App layer if I2C ready.
    if (SII_OS_STATUS_SUCCESS != SiiDrvGetI2cAccessStatus())
    {
        return (uint8_t)SiiDrvGetI2cAccessStatus();
    }

    chipIdH = SiiDrvVirtualPageRead(pDrvAdapt, CHIP_ID_HIGH);
    poVersionInfo->chipRev = SiiDrvVirtualPageRead(pDrvAdapt, REV_ID);
    poVersionInfo->fwCustomerVersion = SiiDrvVirtualPageRead(pDrvAdapt, FW_CUSTOMER_SUB_VER);
    /*support FW version range:
    version high: 0~15,
    version low:  0~255
    revision:     0~31*/
    fwVerL = SiiDrvVirtualPageRead(pDrvAdapt, FW_VER_L);
    fwVerH = SiiDrvVirtualPageRead(pDrvAdapt, FW_VER_H);

    poVersionInfo->chipId = (chipIdH << 8) | chipIdL;
    poVersionInfo->fwVersion = (fwVerH << 8) | fwVerL;

    return RetVal;
}

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
void SiiDrvAdaptChipIdGet(SiiInst_t inst, uint16_t *pChipId)
{
    uint8_t chipIdL;
    uint8_t chipIdH;
    DrvAdaptInst_t *pDrvAdapt = DRV_ADAPT_INST2PTR(inst);
    chipIdL = SiiDrvRegReadByte(pDrvAdapt, Page0Addr, CHIP_ID_LOW);
    chipIdH = SiiDrvRegReadByte(pDrvAdapt, Page0Addr, CHIP_ID_HIGH);

    *pChipId = (chipIdH << 8) | chipIdL;
}

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
void SiiDrvAdaptChipGpioCtrl(SiiInst_t inst, uint8_t GpioNo, bool_t bGpioPowLevel)
{
    DrvAdaptInst_t *pDrvAdapt = DRV_ADAPT_INST2PTR(inst);
    uint8_t GpioCtrlVal = 0;

    if (GpioNo < 4) //now, actually fw only support GPIO 0/1/3 control
    {
        GpioCtrlVal |= ((bGpioPowLevel)?(GPIO_POW_HIGH):(GPIO_POW_LOW));
        GpioCtrlVal |= GpioNo;
        GpioCtrlVal |= GPIO_WRITE_CTRL;
        SiiDrvVirtualRegModify(pDrvAdapt, GPIO_CTRL, (GPIO_NUM_MASK|GPIO_R_W_CTRL_MASK|GPIO_POW_LEVEL_MASK), GpioCtrlVal);
    }
    else
    {
        //DEBUG_PRINT(MSG_ALWAYS, ("GPIO:: Set GPIO No. error.\n"));
    }
}

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
void SiiDrvAdaptChipGpioRead(SiiInst_t inst, uint8_t GpioNo, bool_t *poGpioStatus)
{
    DrvAdaptInst_t *pDrvAdapt = DRV_ADAPT_INST2PTR(inst);
    uint8_t GpioCtrlVal = 0, GpioStatus = 0;

    if (GpioNo < 4) //now, actually fw only support GPIO 0/1/3 control
    {
        GpioCtrlVal |= GpioNo;
        GpioCtrlVal |= GPIO_READ_CTRL;
        SiiDrvVirtualRegModify(pDrvAdapt, GPIO_CTRL, (GPIO_NUM_MASK|GPIO_R_W_CTRL_MASK), GpioCtrlVal);

        GpioStatus = SiiDrvVirtualPageRead(pDrvAdapt, GPIO_READ);
        *poGpioStatus = (bool_t)((GpioStatus >> GpioNo) & 0x01);
    }
    else
    {
        //DEBUG_PRINT(MSG_ALWAYS, ("GPIO:: Get GPIO No. error.\n"));
    }
}

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
* @see SiiDrvAdaptDsEdidRead()
* @see SII_DRV_ADAPT_EVENT__ACCESS_DONE
*
*****************************************************************************/
void SiiDrvAdaptAccessStatusGet(SiiInst_t inst,
        SiiDrvAdaptAccessStatus_t *poUpdateStatus)
{
    DrvAdaptInst_t *pDrvAdapt = DRV_ADAPT_INST2PTR(inst);
    *poUpdateStatus = pDrvAdapt->accessStatus;
}

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
        uint16_t length)
{
    uint32_t i;
    DrvAdaptInst_t *pDrvAdapt = DRV_ADAPT_INST2PTR(inst);

    for(i = 0; i < length; i=i+16)
    {
        SiiDrvVirtualPageWrite(pDrvAdapt, LEGACY_VIRTUAL_REQ_ADDR, (address >> 8) & 0xFF);
        SiiDrvVirtualPageWrite(pDrvAdapt, LEGACY_VIRTUAL_REQ_OFFSET, address & 0xFF);
        SiiDrvVirtualPageReadBlock(pDrvAdapt, LEGACY_VIRTUAL_REQ_RDWR, poData+i, 16);
        address += 16;
    }
}

/*****************************************************************************/
/**
* @brief Chip register writing
*
* writes block of chip registers.
*
* @note This function is used for debugging purpose only.
*
* @param[in]  inst    Adapter driver instance returned by @ref SiiDrvAdaptCreate()
* @param[in]  address First register address with page number in MSB and offset in LSB
* @param[out] poData  Pointer to a buffer to copy read register values
* @param[in]  length  Number of registers to read
*
*****************************************************************************/
void SiiDrvAdaptChipRegWrite(SiiInst_t inst,
        uint16_t address,
        uint8_t  *poData,
        uint16_t length)
{
    DrvAdaptInst_t *pDrvAdapt = DRV_ADAPT_INST2PTR(inst);
    length = length;
    SiiDrvVirtualPageWrite(pDrvAdapt, LEGACY_VIRTUAL_REQ_ADDR,(address >> 8) & 0xFF);
    SiiDrvVirtualPageWrite(pDrvAdapt, LEGACY_VIRTUAL_REQ_OFFSET,address & 0xFF);
    SiiDrvVirtualPageWrite(pDrvAdapt, LEGCY_VIRTUAL_REQ_WRITE,*poData);

    //There is a bug in FW, Read vitrual page to reset FW state machine, after write 'LEGCY_VIRTUAL_REQ_WRITE'
    (void)SiiDrvVirtualPageRead(pDrvAdapt, 0x00);
    (void)SiiDrvVirtualPageRead(pDrvAdapt, 0x00);
}



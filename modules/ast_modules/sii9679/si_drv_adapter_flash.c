/*
 *****************************************************************************
 *
 * Copyright 2008-2012, Silicon Image, Inc.  All rights reserved.
 * No part of this work may be reproduced, modified, distributed, transmitted,
 * transcribed, or translated into any language or computer format, in any form
 * or by any means without written permission of: Silicon Image, Inc., 1140
 * East Arques Avenue, Sunnyvale, California 94085
 *****************************************************************************
 */

/**
 * @file
 * si_drv_adapter_flash.c
 * @brief the function of spi flash driver
 *
 *
 *****************************************************************************/
/***** #include statements ***************************************************/
#include "si_drv_adapter_internal.h"
#include "si_virtual_page.h"
#include "si_drv_adapter_flash.h"

/***** local variable declarations *******************************************/
#define BIT_SPI_READ_STATUS     0x05
#define BIT_RDSR_WIP            0x01
#define DUMMY_BYTE              0xF3

#define REG_SPI_INS_BYTE_CODE       0x00
#define REG_SPI_ADDR_HI             0x01
#define REG_SPI_ADDR_MID            0x02
#define REG_SPI_ADDR_LO             0x03
#define REG_SPI_WRITE_REG           0x04
#define REG_SPI_READ_REG            0x05
#define REG_SPI_RXD_STATUS_PORT     0x06
#define REG_SPI_TXD_STATUS_PORT     0x07

#define SPI_INSTRUCTION_RD_DATA         0x03
#define SPI_INSTRUCTION_WR_BYTE         0x02
#define SPI_INSTRUCTION_CHIP_ERASE      0x60

#define SPI_INSTRUCTION_WR_ENABLE       0x06
#define SPI_INSTRUCTION_WR_DISABLE      0x04
#define SPI_INSTRUCTION_RD_STATUS       0x05

#define SPI_INSTRUCTION_WR_STATUS       0x01
#define SPI_INSTRUCTION_EWSR            0x50

#define SPI_INSTRUCTION_AAI_PROGRAM     0xAD


#define SiiSpiFlashInstructionSet(pDrvAdapt, cmd) SiiDrvRegWriteByte(pDrvAdapt, SlaveAA, REG_SPI_INS_BYTE_CODE, cmd)
#define SiiSpiFlashAddressSet(pDrvAdapt, addr)  \
    SiiDrvRegWriteByte(pDrvAdapt, SlaveAA, REG_SPI_ADDR_HI, ((addr>>16)&0x0FF)); \
    SiiDrvRegWriteByte(pDrvAdapt, SlaveAA, REG_SPI_ADDR_MID, ((addr>>8)&0x0FF)); \
    SiiDrvRegWriteByte(pDrvAdapt, SlaveAA, REG_SPI_ADDR_LO, (addr&0x0FF))

static uint8_t SlaveAA = 0xAA;

/***** public functions ******************************************************/

/*****************************************************************************/
/**
* @brief get the status of flash
*
* @param[in]  inst           Adapter driver instance returned by @ref SiiDrvAdaptCreate()
*
* @return the status of flash
* @see
*
*****************************************************************************/
static uint8_t SiiSpiFlashWaitInProgress(SiiInst_t inst)
{
    uint8_t ucStatus = 0;
    DrvAdaptInst_t *pDrvAdapt = DRV_ADAPT_INST2PTR(inst);

    SiiSpiFlashInstructionSet(pDrvAdapt, SPI_INSTRUCTION_RD_STATUS);
    ucStatus = SiiDrvRegReadByte(pDrvAdapt, SlaveAA, REG_SPI_TXD_STATUS_PORT);
    while( ucStatus & BIT_RDSR_WIP )
    {
        SiiSpiFlashInstructionSet(pDrvAdapt, SPI_INSTRUCTION_RD_STATUS);
        ucStatus = SiiDrvRegReadByte(pDrvAdapt, SlaveAA, REG_SPI_TXD_STATUS_PORT);
    }

    return ucStatus;
}

/*****************************************************************************/
/**
* @brief get the status of flash
*
* @param[in]  inst           Adapter driver instance returned by @ref SiiDrvAdaptCreate()
* @param[in]  value          The stauts for writing
* @see
*
*****************************************************************************/
static void SiiSpiFlashWriteStatus(SiiInst_t inst, uint8_t value)
{
   DrvAdaptInst_t *pDrvAdapt = DRV_ADAPT_INST2PTR(inst);

    SiiSpiFlashInstructionSet(pDrvAdapt, SPI_INSTRUCTION_EWSR); // First need to enable write status
    SiiSpiFlashInstructionSet(pDrvAdapt, SPI_INSTRUCTION_WR_STATUS); // then write status
    SiiDrvRegWriteByte(pDrvAdapt, SlaveAA,REG_SPI_RXD_STATUS_PORT,value);
}

/*****************************************************************************/
/**
* @brief Erase the flash
*
* @param[in]  inst           Adapter driver instance returned by @ref SiiDrvAdaptCreate()
*
* @see
*
*****************************************************************************/
 void SiiSpiFlashChipErase(SiiInst_t inst)
{
    DrvAdaptInst_t *pDrvAdapt = DRV_ADAPT_INST2PTR(inst);

    //SiiSpiFlashInstructionSet(pDrvAdapt, pDrvAdapt->config.spiConfig.writeEnCmd);
    //SiiSpiFlashInstructionSet(pDrvAdapt, SPI_INSTRUCTION_WR_STATUS);
    if(SiiSpiFlashWaitInProgress(inst))
    {
        // Chip is in may be software based write protection enabled.
        // may need to override the status register
        SiiSpiFlashWriteStatus(inst, 0);
        (void)SiiSpiFlashWaitInProgress(inst);
    }

    SiiSpiFlashInstructionSet(pDrvAdapt, pDrvAdapt->config.spiConfig.writeEnCmd);
    SiiSpiFlashInstructionSet(pDrvAdapt, pDrvAdapt->config.spiConfig.eraseCmd);
    (void)SiiSpiFlashWaitInProgress(inst);
}



/*****************************************************************************/
/**
* @brief Init the SPI flash
*
* @param[in]  inst           Adapter driver instance returned by @ref SiiDrvAdaptCreate()
*
* @see
*
*****************************************************************************/
void SiiSpiFlashInit(SiiInst_t inst, SiiDrvAdaptBootStatus_t bBootStatus)
{
    DrvAdaptInst_t *pDrvAdapt = DRV_ADAPT_INST2PTR(inst);
    uint8_t initValue = 0x09;  //Configure 0x09: AAI programing and ISP enable
    SiiDrvAdaptVersionInfo_t VersionInfo;
    uint8_t FwVerH, FwVerM;    //, FwVerL;

    if(pDrvAdapt->config.spiConfig.pageWrCmd != 0xAD)  //Cmd 0xAD: AAI Programing
    {
        initValue = 0x0D; //Configure 0x0D: Page programing and ISP enable
    }

    memset(&VersionInfo, 0, sizeof(SiiDrvAdaptVersionInfo_t));
    (void)SiiDrvAdaptVersionGet(inst, &VersionInfo);

    FwVerH = (VersionInfo.fwVersion       ) >> 13;
    FwVerM = (uint8_t)((VersionInfo.fwVersion&0x1FFF) >> 5);
    //FwVerL = (VersionInfo.fwVersion&0x001F)     ;

    if ((FwVerH >= 1)&&(FwVerM >= 1))  //in order to compatible with previous FW. (FwVersion >= 1.1.0)
    {
        if(SII_DRV_ADAPTER_BOOTING__SUCCESS == bBootStatus)
        {
            //use 'SiiDrvVirtualPageWrite' to instead of 'SiiDrvAdaptChipRegWrite',
            //API 'SiiDrvAdaptChipRegWrite' is redundant.

            //ISP flash command congigure
            SiiDrvVirtualPageWrite(pDrvAdapt, ISP_CONFIGURE_SLAVE_ADDR, SlaveAA);
            SiiDrvVirtualPageWrite(pDrvAdapt, ISP_CONFIGURE_ERASE_CMD, pDrvAdapt->config.spiConfig.eraseCmd); //must write before Page3 0x30 init
            SiiDrvVirtualPageWrite(pDrvAdapt, ISP_CONFIGUER_WRITE_EN_CMD, pDrvAdapt->config.spiConfig.writeEnCmd);
            //enable ISP function and enable page program(256 bytes)
            SiiDrvVirtualPageWrite(pDrvAdapt, ISP_CONFIGURE_ENABLE_PAGE, initValue);
        }
        else
        {
            SiiDrvRegWriteByte(pDrvAdapt, pDrvAdapt->config.i2cSlaveAddrBase, 0x17, pDrvAdapt->config.i2cSlaveAddrF0);
            SiiDrvRegWriteByte(pDrvAdapt, pDrvAdapt->config.i2cSlaveAddrF0, 0x40, SlaveAA);
            SiiDrvRegWriteByte(pDrvAdapt, pDrvAdapt->config.i2cSlaveAddrF0, 0x34, pDrvAdapt->config.spiConfig.eraseCmd);
            SiiDrvRegWriteByte(pDrvAdapt, pDrvAdapt->config.i2cSlaveAddrF0, 0x30, initValue);
        }
    }
    else
    {
        //ISP Enable
        if(SII_DRV_ADAPTER_BOOTING__SUCCESS == bBootStatus)
        {
            SiiDrvAdaptChipRegWrite(inst, 0xF034, &pDrvAdapt->config.spiConfig.eraseCmd, 1);
            SiiDrvAdaptChipRegWrite(inst, 0xF030, &initValue, 1);
        }
        else
        {
            SiiDrvRegWriteByte(pDrvAdapt, pDrvAdapt->config.i2cSlaveAddrBase, 0x17, pDrvAdapt->config.i2cSlaveAddrF0);
            SiiDrvRegWriteByte(pDrvAdapt, pDrvAdapt->config.i2cSlaveAddrF0, 0x40, SlaveAA);
            SiiDrvRegWriteByte(pDrvAdapt, pDrvAdapt->config.i2cSlaveAddrF0, 0x34, pDrvAdapt->config.spiConfig.eraseCmd);
            SiiDrvRegWriteByte(pDrvAdapt, pDrvAdapt->config.i2cSlaveAddrF0, 0x30, initValue);
        }
    }
}

/*****************************************************************************/
/**
* @brief Program the flash with I2C block writing
*
* @param[in]  inst           Adapter driver instance returned by @ref SiiDrvAdaptCreate()
* @param[in]  addr           The start address for programming
* @param[in]  pData          Pointer to the data for programming
* @param[in]  num            the number of bytes for writing
* @see
*
*****************************************************************************/
void SiiSpiFlashBlockProgram(SiiInst_t inst, uint32_t addr, uint8_t *pData, uint32_t num)
{
    DrvAdaptInst_t *pDrvAdapt = DRV_ADAPT_INST2PTR(inst);
#if 0
    uint16_t bytes_to_write;
#else
    uint32_t bytes_to_write;
#endif

    pDrvAdapt->accessStatus = SII_DRV_ADAPTER_ACCESS__IN_PROGRESS;

    while(num)
    {
#if 0
        bytes_to_write =(uint16_t)((num>SPI_MAX_PAGE_SIZE) ? SPI_MAX_PAGE_SIZE : num);
#else
        bytes_to_write =(uint32_t)((num>SPI_MAX_PAGE_SIZE) ? SPI_MAX_PAGE_SIZE : num);
#endif
        SiiSpiFlashInstructionSet(pDrvAdapt, pDrvAdapt->config.spiConfig.writeEnCmd);
        SiiSpiFlashInstructionSet(pDrvAdapt, pDrvAdapt->config.spiConfig.pageWrCmd);
        SiiSpiFlashAddressSet(pDrvAdapt, addr);
        SiiDrvRegWriteBlock(pDrvAdapt, SlaveAA, REG_SPI_WRITE_REG, pData, bytes_to_write);

        (void)SiiSpiFlashWaitInProgress(inst);
        SiiSpiFlashInstructionSet(pDrvAdapt, SPI_INSTRUCTION_WR_DISABLE);

        num   -= bytes_to_write;
        pData += bytes_to_write;
        addr  += bytes_to_write;
        kick_wdt();
    }

    pDrvAdapt->accessStatus = SII_DRV_ADAPTER_ACCESS__SUCCESS;
}
#if 0
/*****************************************************************************/
/**
* @brief Program the flash with I2C page writing
*
* @param[in]  inst           Adapter driver instance returned by @ref SiiDrvAdaptCreate()
* @param[in]  addr           The start address for programming
* @param[in]  pData          Pointer to the data for programming
* @see
*
*****************************************************************************/
void SiiSpiPageProgram(SiiInst_t inst, uint32_t addr, uint8_t *pData)
{
    uint16_t bytes_to_write = 256;
    DrvAdaptInst_t *pDrvAdapt = DRV_ADAPT_INST2PTR(inst);

    SiiSpiFlashInstructionSet(pDrvAdapt, pDrvAdapt->config.spiConfig.writeEnCmd);
    SiiSpiFlashInstructionSet(pDrvAdapt, pDrvAdapt->config.spiConfig.pageWrCmd);
    SiiSpiFlashAddressSet(pDrvAdapt, addr);
    SiiDrvRegWriteBlock(pDrvAdapt, SlaveAA, REG_SPI_WRITE_REG, pData, bytes_to_write);
    SiiSpiFlashWaitInProgress(inst);
}
#endif
/*****************************************************************************/
/**
* @brief read the flash with I2C block reading
*
* @param[in]  inst           Adapter driver instance returned by @ref SiiDrvAdaptCreate()
* @param[in]  addr           The start address for reading
* @param[in]  pData          Pointer to the data for reading
* @param[in]  num            the number of bytes for reading
* @see
*
*****************************************************************************/
void SiiSpiFlashBlockRead(SiiInst_t inst, uint32_t addr, uint8_t *pData, uint32_t num)
{
	DrvAdaptInst_t *pDrvAdapt = DRV_ADAPT_INST2PTR(inst);
	uint32_t bytes_to_read;

	while (num) {
		bytes_to_read = (uint32_t)((num > SPI_MAX_PAGE_SIZE) ? SPI_MAX_PAGE_SIZE : num);

		SiiSpiFlashInstructionSet(pDrvAdapt, SPI_INSTRUCTION_RD_DATA);
		SiiSpiFlashAddressSet(pDrvAdapt, addr);
		SiiDrvRegReadBlock(pDrvAdapt, SlaveAA, REG_SPI_READ_REG, pData, bytes_to_read);

		num   -= bytes_to_read;
		pData += bytes_to_read;
		addr  += bytes_to_read;
	}
}

//------------------------------------------------------------------------------
// Function:    SiiRemoteBlockWrite_8BA
// Description: Translate I2C Write from legacy call
//------------------------------------------------------------------------------
uint8_t SiiRemoteBlockWrite_8BA(SiiInst_t inst, I2CCommandType *I2CCommand)
{
    bool_t result;
    SiiDrvRegWriteBlock((DrvAdaptInst_t*)inst,
        I2CCommand->SlaveAddr,
        I2CCommand->RegAddrL,
        I2CCommand->Data,
        I2CCommand->NBytes
        );
    result = true;

    return ((result) ? IIC_OK : IIC_NOACK);
}

//------------------------------------------------------------------------------
// Function:    SiiRemoteBlockRead_8BA
// Description: Translate I2C Read from legacy call
//------------------------------------------------------------------------------
uint8_t SiiRemoteBlockRead_8BA(SiiInst_t inst, I2CCommandType *I2CCommand)
{
    bool_t result;

    SiiDrvRegReadBlock((DrvAdaptInst_t*)inst,
        I2CCommand->SlaveAddr,
        I2CCommand->RegAddrL,
        I2CCommand->Data,
        I2CCommand->NBytes);
    result = true;

    return ((result) ? IIC_OK : IIC_NOACK);
}


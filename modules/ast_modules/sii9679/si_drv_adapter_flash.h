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
 * si_drv_adapter_flash.h
 *
 * @brief
 * headers for si_drv_adapter_flash.c
 *
 *****************************************************************************/
#ifndef _SI_DRV_ADAPTER_FLASH_H_
#define _SI_DRV_ADAPTER_FLASH_H_

#ifdef __cplusplus
extern "C"{
#endif

/***** #include statements ***************************************************/
#include "si_drv_adapter_platform_api.h"
#include "si_drv_adapter.h"

/***** public functions ******************************************************/
//Silicon Image internal debug use, for read/write registers
#define IIC_CAPTURED    1
#define IIC_NOACK       2
#define IIC_OK          0

//Silicon Image internal debug use, for read/write registers
typedef struct
{
    uint8_t Dummy;
    uint8_t NBytes;
    uint8_t Flags;
    uint8_t Ctrl;
    uint8_t SlaveAddr;
    uint8_t RegAddrL;
    uint8_t RegAddrH;
    uint8_t Data[16];
} I2CCommandType;

/*****************************************************************************/
/**
* @brief Erase the flash
*
* @param[in]  inst           Adapter driver instance returned by @ref SiiDrvAdaptCreate()
*
* @see
*
*****************************************************************************/
void SiiSpiFlashChipErase(SiiInst_t inst);

/*****************************************************************************/
/**
* @brief Init the SPI flash
*
* @param[in]  inst           Adapter driver instance returned by @ref SiiDrvAdaptCreate()
*
* @see
*
*****************************************************************************/
void SiiSpiFlashInit(SiiInst_t inst, SiiDrvAdaptBootStatus_t bBootStatus);

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
void SiiSpiFlashBlockProgram(SiiInst_t inst, uint32_t addr, uint8_t *pData, uint32_t num);

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
void SiiSpiPageProgram(SiiInst_t inst, uint32_t addr, uint8_t *pData);

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
void SiiSpiFlashBlockRead(SiiInst_t inst, uint32_t addr, uint8_t *pData, uint32_t num);

//------------------------------------------------------------------------------
// Function:    SiiRemoteBlockWrite_8BA
// Description: Translate I2C Write from legacy call
//------------------------------------------------------------------------------
uint8_t SiiRemoteBlockWrite_8BA(SiiInst_t inst, I2CCommandType *I2CCommand);

//------------------------------------------------------------------------------
// Function:    SiiRemoteBlockRead_8BA
// Description: Translate I2C Read from legacy call
//------------------------------------------------------------------------------
uint8_t SiiRemoteBlockRead_8BA(SiiInst_t inst, I2CCommandType *I2CCommand);

#ifdef __cplusplus
}
#endif

#endif//_SI_DRV_ADAPTER_FLASH_H_


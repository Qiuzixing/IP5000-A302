/******************************************************************************
 *
 * Copyright 2011, Silicon Image, Inc.  All rights reserved.
 * No part of this work may be reproduced, modified, distributed, transmitted,
 * transcribed, or translated into any language or computer format, in any form
 * or by any means without written permission of
 * Silicon Image, Inc., 1140 East Arques Avenue, Sunnyvale, California 94085
 *
 *****************************************************************************/
/**
 * file
 *
 * @brief Internal Adapter Driver definitions
 *
 *****************************************************************************/


#ifndef SI_DRV_ADAPT_INTERNAL_H
#define SI_DRV_ADAPT_INTERNAL_H

#ifdef __cplusplus
extern "C"{
#endif

/***** #include statements ***************************************************/
#include "si_drv_adapter_platform_api.h"
#include "si_drv_adapter.h"

/***** public macro definitions **********************************************/

#define DRV_ADAPT_INST2PTR(inst) ((DrvAdaptInst_t *) inst)
#define DRV_ADAPT_PTR2INST(inst) ((SiiInst_t) inst)

#define I2C_ADDR_DEBUG  0x60


// Maximum number of adapter driver instances
#define MAX_NMB_OF_ADAPTER_INSTANCES                2


#define FW_UPDATE_SLAVE_ADDRESS     0xAA
#define NUM_EXT_BLOCKS_INDEX        126
#define MAX_EXT_BLOCKS              1
#define EDID_BLOCK_SIZE             128
#define BOOT_BLOCK_SIZE             64
#define MAX_EDID_SRAM_SIZE          256
#define MAX_BOOT_SRAM_SIZE          64
#define SPI_MAX_PAGE_SIZE           256

/***** public type definitions ***********************************************/

// Internal instance record
typedef struct
{
    uint8_t                   instCra;   // CRA instance
    SiiDrvAdaptConfig_t       config;    // Driver configuration
    SiiDrvAdaptAccessStatus_t accessStatus; // FW update, SRAM and EDID access status
    SiiLibEvtCbFunc_t         pNtfCallback;  // Pointer to the notification callback function
    //bool_t                    isBootingInProgress; // true during booting

    bool_t                    upgradeMode;
    bool_t                    isError;
} DrvAdaptInst_t;

/***** public functions ******************************************************/
SiiOsStatus_t   SiiDrvGetI2cAccessStatus(void);
void            SiiDrvVirtualPageWriteWord(DrvAdaptInst_t *pDrvAdapt, uint8_t virtualAddr, uint16_t value);
void            SiiDrvVirtualPageWrite(DrvAdaptInst_t *pDrvAdapt, uint8_t virtualAddr, uint8_t value);
void            SiiDrvVirtualPageWriteBlock (DrvAdaptInst_t *pDrvAdapt, uint8_t virtualAddr, const uint8_t *pBuffer, uint16_t count);
uint8_t         SiiDrvVirtualPageRead (DrvAdaptInst_t *pDrvAdapt, uint8_t virtualAddr);
void            SiiDrvVirtualPageReadBlock (DrvAdaptInst_t *pDrvAdapt, uint8_t virtualAddr, uint8_t *pBuffer, uint16_t count);
void            SiiDrvVirtualRegModify (DrvAdaptInst_t *pDrvAdapt, uint8_t virtualAddr, uint8_t mask, uint8_t value);
void            SiiDrvVirtualRegBitsSet (DrvAdaptInst_t *pDrvAdapt, uint8_t virtualAddr, uint8_t bitMask, bool_t setBits);
void            SiiDrvVirtualPageReadWord(DrvAdaptInst_t *pDrvAdapt, uint8_t virtualAddr, uint16_t *value);
uint8_t         SiiDrvRegReadByte(DrvAdaptInst_t *pDrvAdapt,  uint8_t pageAddr, uint8_t pageoffset);
void            SiiDrvRegWriteByte(DrvAdaptInst_t *pDrvAdapt,  uint8_t pageAddr, uint8_t pageoffset, uint8_t value);
void            SiiDrvRegReadBlock(DrvAdaptInst_t *pDrvAdapt,  uint8_t pageAddr, uint8_t pageoffset, uint8_t *pBuffer, uint16_t count);
void            SiiDrvRegWriteBlock(DrvAdaptInst_t *pDrvAdapt,  uint8_t pageAddr, uint8_t pageoffset, uint8_t *pBuffer, uint32_t count);

#if AST_HDMITX
void kick_wdt(void);
#endif

#ifdef __cplusplus
}
#endif

#endif // SI_DRV_ADAPT_INTERNAL_H


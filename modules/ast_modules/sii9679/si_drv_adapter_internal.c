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
 * @file si_drv_adapter_internal.c
 *
 * @brief Adapter Driver private functions.
 *
 *****************************************************************************/

/***** #include statements ***************************************************/
#include "si_drv_adapter_internal.h"
#include "si_virtual_page.h"
#include <linux/delay.h>

/***** local macro definitions ***********************************************/
static SiiOsStatus_t I2cAccessStatus = SII_OS_STATUS_ERR_FAILED;

/***** public functions ******************************************************/

/*****************************************************************************/
/**
* @brief Return I2C status, ACK(SII_OS_STATUS_SUCCESS)/NACK(SII_OS_STATUS_ERR_FAILED)
*
* @param[in] N/A
*
* @return     SiiOsStatus_t status of operation (0 == success).
*
*****************************************************************************/
SiiOsStatus_t SiiDrvGetI2cAccessStatus(void)
{
    return I2cAccessStatus;
}

/*****************************************************************************/
/**
* @brief Read a byte or bytes from the specified bus and i2c device.
*
* @param[in] busIndex    - I2C bus selection
* @param[in] deviceId    - I2C device address, eight-bit format (addr + r/w bit)
* @param[in] regAddr     - Eight or sixteen bit starting register address
* @param[in]  pBuffer     - destination buffer
* @param[in]  count       - number of registers (bytes) to read
*
* @return     SiiOsStatus_t status of operation (0 == success).
*
*****************************************************************************/
SiiOsStatus_t CraReadBlockI2c(uint8_t busIndex, uint8_t deviceId, uint16_t regAddr, uint8_t *pBuffer, uint32_t count)
{
#if AST_HDMITX
#if 0
	int i;

	for (i = 0; i < count; i++) {
		if (GetI2CReg(busIndex, deviceId, (regAddr + i), pBuffer + i) != 0) {
			yield();
			I2cAccessStatus = SII_OS_STATUS_ERR_FAILED;
			return 1; /* means fail */
		}
		yield();
	}

	I2cAccessStatus = SII_OS_STATUS_SUCCESS;
#else
	if (GetI2CBlock(busIndex, deviceId, regAddr, pBuffer, count) != 0) {
		yield();
		I2cAccessStatus = SII_OS_STATUS_ERR_FAILED;
		return 1; /* means fail */
	}
	yield();
#endif
	I2cAccessStatus = SII_OS_STATUS_SUCCESS;
	return 0; /* means success */
#else
    SiiOsStatus_t       status = SII_OS_STATUS_SUCCESS;
    SiiI2cMsg_t         msgs[2];
    uint8_t             addrData[2];
    int                 addrLen;
    uint8_t             ucRetryCount = 0x00;

    // Only one byte register address
    addrData[0] = (uint8_t)regAddr;
    addrLen = 1;

    // Split into register address write and register data read messages.
    msgs[0].addr        = deviceId;
    msgs[0].cmdFlags    = SII_MI2C_WR;
    msgs[0].len         = (uint16_t)addrLen;
    msgs[0].pBuf        = &addrData[0];
    msgs[1].addr        = deviceId;
    msgs[1].cmdFlags    = SII_MI2C_RD;
    msgs[1].len         = count;
    msgs[1].pBuf        = pBuffer;

    I2cAccessStatus = status;

    //add retry mechanism to fix bug i2c not stable
    while(SII_OS_STATUS_SUCCESS != SiiHalMasterI2cTransfer(busIndex, &msgs[0], sizeof(msgs) / sizeof(SiiI2cMsg_t)))
    {
        ucRetryCount++;

        if (ucRetryCount == 3)
        {
            //DEBUG_PRINT(MSG_ERR, ("I2C Read failed\n"));
            I2cAccessStatus = SII_OS_STATUS_ERR_FAILED; //update I2C access status

            return SII_OS_STATUS_ERR_FAILED;
        }
    }

    return status;
#endif
}

/*****************************************************************************/
/**
* @brief Write a byte or bytes from the specified bus and i2c device.
*
* @param[in] busIndex    - I2C bus selection
* @param[in] deviceId    - I2C device address, eight-bit format (addr + r/w bit)
* @param[in] regAddr     - Eight or sixteen bit starting register address
* @param[in]  pBuffer     - source data buffer.
* @param[in]  count       - number of registers (bytes) to write (maximum 256)
*
* @return     SiiOsStatus_t status of operation (0 == success).
*
*****************************************************************************/
SiiOsStatus_t CraWriteByteI2c(uint8_t busIndex, uint8_t deviceId, uint16_t regAddr, const uint8_t *pBuffer)
{
	if (SetI2CReg(busIndex, deviceId, regAddr, *pBuffer)) {
		yield();
		I2cAccessStatus = SII_OS_STATUS_ERR_FAILED;
		return 1;
	}
	yield();
	I2cAccessStatus = SII_OS_STATUS_SUCCESS;
	return 0;
}
#if 0
SiiOsStatus_t CraWriteBlockI2c(uint8_t busIndex, uint8_t deviceId, uint16_t regAddr, const uint8_t *pBuffer, uint16_t count)
#else
SiiOsStatus_t CraWriteBlockI2c(uint8_t busIndex, uint8_t deviceId, uint16_t regAddr, const uint8_t *pBuffer, uint32_t count)
#endif
{
#if AST_HDMITX
#if (3 <= CONFIG_AST1500_SOC_VER)
	if (SetI2CBlock(busIndex, deviceId, regAddr, pBuffer, count)) {
		I2cAccessStatus = SII_OS_STATUS_ERR_FAILED;
		return 1;
	}
#else
	uint32_t i;

	for (i = 0; i < count; i++) {
		if (SetI2CReg(busIndex, deviceId, (regAddr + i), *(pBuffer + i))) {
			yield();
			I2cAccessStatus = SII_OS_STATUS_ERR_FAILED;
			return 1;
		}
		yield();
	}
#endif
	I2cAccessStatus = SII_OS_STATUS_SUCCESS;
	return 0;
#else
    SiiOsStatus_t       status = SII_OS_STATUS_SUCCESS;
    SiiI2cMsg_t         msg;
    uint8_t             srcData [2 + 256]; // Room for 256 register data bytes and one or two register address bytes
    uint16_t            msgLen;
    uint8_t             ucRetryCount = 0x00;

    // Only one byte register address
    srcData[0] = (uint8_t)regAddr;
    msgLen = 1;

    // Copy payload data to src buffer following the register address.
    memcpy(&srcData[msgLen], (char*)pBuffer, count);
    msgLen += count;

    // Create message
    msg.addr            = deviceId;
    msg.cmdFlags        = SII_MI2C_WR;
    msg.len             = msgLen;
    msg.pBuf            = &srcData[0];

    I2cAccessStatus = status;

    //add retry mechanism to fix bug i2c not stable
    while(SII_OS_STATUS_SUCCESS != SiiHalMasterI2cTransfer(busIndex, &msg, 1))
    {
        ucRetryCount++;

        if (ucRetryCount == 3)
        {
            //DEBUG_PRINT(MSG_ERR, ("I2C Write failed\n"));
            I2cAccessStatus = SII_OS_STATUS_ERR_FAILED; //update I2C access status

            return SII_OS_STATUS_ERR_FAILED;
        }
    }

    return status;
#endif
}

/*****************************************************************************/
/**
* @brief Read a byte from the virtual page of i2c device.
*
* @param[in] pDrvAdapt		the pointer to the Adapter Instance
* @param[in] virtualAddr    starting register address
*
* @return the got data from the virtual page
*
*****************************************************************************/
uint8_t SiiDrvVirtualPageRead(DrvAdaptInst_t *pDrvAdapt,  uint8_t virtualAddr)
{
    uint8_t             value = 0;
#if 1
    (void) CraReadBlockI2c(pDrvAdapt->instCra, pDrvAdapt->config.i2cSlaveAddr, virtualAddr, &value, 1);
#else
    (void) CraReadByteI2c(pDrvAdapt->instCra, pDrvAdapt->config.i2cSlaveAddr, virtualAddr, &value);
#endif
    return value;
}

/*****************************************************************************/
/**
* @brief Write a byte to the virtual page of i2c device.
*
* @param[in] pDrvAdapt      the pointer to the Adapter Instance
* @param[in] virtualAddr    starting register address
* @param[in] value          source data value.
* @return
*
*****************************************************************************/
void SiiDrvVirtualPageWrite(DrvAdaptInst_t *pDrvAdapt, uint8_t virtualAddr, uint8_t value)
{
#if 0
    (void) CraWriteBlockI2c(pDrvAdapt->instCra, pDrvAdapt->config.i2cSlaveAddr, virtualAddr, &value, 1);
#else
    (void) CraWriteByteI2c(pDrvAdapt->instCra, pDrvAdapt->config.i2cSlaveAddr, virtualAddr, &value);
#endif
}

void SiiDrvVirtualPageWriteWord(DrvAdaptInst_t *pDrvAdapt, uint8_t virtualAddr, uint16_t value)
{
    uint8_t buffer[2];
    buffer[0] = value &0xFF;
    buffer[1] = (value>>8) &0xFF;
    (void) CraWriteBlockI2c(pDrvAdapt->instCra, pDrvAdapt->config.i2cSlaveAddr, virtualAddr, buffer, 2);
}


/*****************************************************************************/
/**
* @brief Write bytes to the virtual page of i2c device.
*
* @param[in] pDrvAdapt      the pointer to the Adapter Instance
* @param[in] virtualAddr    starting register address
* @param[in] value          source data buffer
* @param[in] count          the number of bytes to write
* @return
*
*****************************************************************************/
void SiiDrvVirtualPageWriteBlock(DrvAdaptInst_t *pDrvAdapt, uint8_t virtualAddr, const uint8_t *pBuffer, uint16_t count)
{
    (void) CraWriteBlockI2c(pDrvAdapt->instCra, pDrvAdapt->config.i2cSlaveAddr, virtualAddr, pBuffer, count);
}

/*****************************************************************************/
/**
* @brief Read bytes from the virtual page of i2c device.
*
* @param[in] pDrvAdapt      the pointer to the Adapter Instance
* @param[in] virtualAddr    starting register address
* @param[out] pBuffer       destination data buffer
*
* @return
*
*****************************************************************************/
void SiiDrvVirtualPageReadBlock(DrvAdaptInst_t *pDrvAdapt, uint8_t virtualAddr, uint8_t *pBuffer, uint16_t count)
{
    (void) CraReadBlockI2c(pDrvAdapt->instCra, pDrvAdapt->config.i2cSlaveAddr, virtualAddr, pBuffer, count );
}

void SiiDrvVirtualPageReadWord(DrvAdaptInst_t *pDrvAdapt, uint8_t virtualAddr, uint16_t *value)
{
    uint8_t buffer[2] = {0,0};
    (void) CraReadBlockI2c(pDrvAdapt->instCra, pDrvAdapt->config.i2cSlaveAddr, virtualAddr, buffer, 2);
    *value = buffer[1];
    *value <<=8;
    *value += buffer[0];
}

/*****************************************************************************/
/**
* @brief modify the value on the virtual page of i2c device.
*
* @param[in] pDrvAdapt      the pointer to the Adapter Instance
* @param[in] virtualAddr    starting register address
* @param[in] mask           mask for the parameter of value
* @param[in] value          source data
*
* @return
*
*****************************************************************************/
void SiiDrvVirtualRegModify(DrvAdaptInst_t *pDrvAdapt, uint8_t virtualAddr, uint8_t mask, uint8_t value)
{
    uint8_t aByte;

    aByte = SiiDrvVirtualPageRead(pDrvAdapt, virtualAddr);
    aByte &= (~mask);                       // first clear all bits in mask
    aByte |= (mask & value);                // then set bits from value
    SiiDrvVirtualPageWrite(pDrvAdapt, virtualAddr, aByte );
}

/*****************************************************************************/
/**
* @brief set the related bits on the virtual page of i2c device.
*
* @param[in] pDrvAdapt      the pointer to the Adapter Instance
* @param[in] virtualAddr    starting register address
* @param[in] bitMask        mask value
* @param[in] setBits        whether to set or clean these bits
*
* @return
*
*****************************************************************************/
void SiiDrvVirtualRegBitsSet(DrvAdaptInst_t *pDrvAdapt, uint8_t virtualAddr, uint8_t bitMask, bool_t setBits)
{
    uint8_t aByte;

    aByte = SiiDrvVirtualPageRead(pDrvAdapt, virtualAddr);
    aByte = (setBits) ? (aByte | bitMask) : (aByte & ~bitMask);
    SiiDrvVirtualPageWrite(pDrvAdapt, virtualAddr, aByte);
}

/*****************************************************************************/
/**
* @brief Read a byte from the chip hardware page of i2c device.
*
* @param[in] pDrvAdapt      the pointer to the Adapter Instance
* @param[in] pageAddr       I2C slave address
* @param[in] pageoffset     starting register address
*
* @return the got data from the virtual page
*
*****************************************************************************/
uint8_t SiiDrvRegReadByte(DrvAdaptInst_t *pDrvAdapt,  uint8_t pageAddr, uint8_t pageoffset)
{
    uint8_t             value = 0;
    (void)CraReadBlockI2c (pDrvAdapt->instCra, pageAddr, pageoffset, &value, 1 );
    return value;
}

/*****************************************************************************/
/**
* @brief Write a byte to the chip hardware page of i2c device.
*
* @param[in] pDrvAdapt      the pointer to the Adapter Instance
* @param[in] pageAddr       I2C slave address
* @param[in] pageoffset     starting register address
* @param[in] value          destination data value.
* @return
*
*****************************************************************************/
void SiiDrvRegWriteByte(DrvAdaptInst_t *pDrvAdapt,  uint8_t pageAddr, uint8_t pageoffset, uint8_t value)
{
#if 0
    (void)CraWriteBlockI2c(pDrvAdapt->instCra, pageAddr, pageoffset, &value, 1);
#else
    (void)CraWriteByteI2c(pDrvAdapt->instCra, pageAddr, pageoffset, &value);
#endif
}

/*****************************************************************************/
/**
* @brief Read bytes from the chip hardware page of i2c device.
*
* @param[in] pDrvAdapt      the pointer to the Adapter Instance
* @param[in] pageAddr       I2C slave address
* @param[in] pageoffset     starting register address
* @param[in] pBuffer        destination data buffer
* @param[in] count          the number of bytes to write
*
*****************************************************************************/
void SiiDrvRegReadBlock(DrvAdaptInst_t *pDrvAdapt,  uint8_t pageAddr, uint8_t pageoffset, uint8_t *pBuffer, uint16_t count)
{
    (void)CraReadBlockI2c(pDrvAdapt->instCra, pageAddr, pageoffset, pBuffer, count );
}

/*****************************************************************************/
/**
* @brief write bytes from the chip hardware page of i2c device.
*
* @param[in] pDrvAdapt      the pointer to the Adapter Instance
* @param[in] pageAddr       I2C slave address
* @param[in] pageoffset     starting register address
* @param[in] pBuffer        source data buffer
* @param[in] count          the number of bytes to write
*
*****************************************************************************/
#if 0
void SiiDrvRegWriteBlock(DrvAdaptInst_t *pDrvAdapt,  uint8_t pageAddr, uint8_t pageoffset, uint8_t *pBuffer, uint16_t count)
#else
void SiiDrvRegWriteBlock(DrvAdaptInst_t *pDrvAdapt,  uint8_t pageAddr, uint8_t pageoffset, uint8_t *pBuffer, uint32_t count)
#endif
{
    (void)CraWriteBlockI2c(pDrvAdapt->instCra, pageAddr, pageoffset, pBuffer, count);
}

#if AST_HDMITX
extern void wdt_kick(void);

void kick_wdt(void)
{
#ifdef CONFIG_AST1500_WATCHDOG
	static unsigned long print_now = 0;

	/* Print '#' once per 2 seconds. */
	if ((print_now == 0) || time_after(jiffies, print_now)) {
		printk("#");
		print_now = jiffies + msecs_to_jiffies(2000);
	}

	wdt_kick();
#endif
}
#endif


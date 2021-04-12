/*
 *****************************************************************************
 *
 * Copyright 2010, Silicon Image, Inc.  All rights reserved.
 * No part of this work may be reproduced, modified, distributed, transmitted,
 * transcribed, or translated into any language or computer format, in any form
 * or by any means without written permission of: Silicon Image, Inc., 1060
 * East Arques Avenue, Sunnyvale, California 94085
 *****************************************************************************
 */
/*
 *****************************************************************************
 * @file  si_apiCpi.c
 *
 * @brief Implementation of the CP 9387 Starter Kit CPI functions.
 *
 *****************************************************************************
*/
#include "defs.h"
#if !AST_HDMITX
#include <stdio.h>
#endif
#include "si_basetypes.h"
#include "si_datatypes.h"
#if !AST_HDMITX
#include "at89c51xd2.h"
#endif
#include "constants.h"
#include "defs.h"
#if !AST_HDMITX
#include "gpio.h"
#endif

#if !AST_HDMITX
#include <string.h>
#endif
#include "i2c_slave_addrs.h"
#include "si_apiCpi.h"
#include "si_cpi_regs.h"
#include "si_cec_enums.h"
#if AST_HDMITX
#include <linux/platform_device.h>
#include "SiI9136_main.h"
#include "ast_i2c.h"
#include "Externals.h" /* for CableConnected */
#else
#include "i2c_master_sw.h"
#endif

#if defined(CONFIG_ARCH_AST1500_CLIENT) && (IS_CEC == 1)
#if AST_HDMITX
#include "ast_utilities.h"
#define printf uinfo
#endif

static uint8_t cpi_write_cntr = 0;

static void SI_TxCountCpiWrites(uint8_t length)
{
	cpi_write_cntr += length;
}

//------------------------------------------------------------------------------
// Function:    SiIRegioReadBlock
// Description: Read a block of registers starting with the specified register.
//              The register address parameter is translated into an I2C
//              slave address and offset.
//              The block of data bytes is read from the I2C slave address
//              and offset.
//------------------------------------------------------------------------------

void SiIRegioReadBlock ( uint16_t regAddr, uint8_t* buffer, uint16_t length)
{
    I2C_ReadBlock(SA_TX_CPI_Primary, (uint8_t)regAddr, buffer, length);
//    HalI2cBus0ReadBlock( l_regioDecodePage[ regAddr >> 8], (uint8_t)regAddr, buffer, length);
}
//------------------------------------------------------------------------------
// Function:    SiIRegioWriteBlock
// Description: Write a block of registers starting with the specified register.
//              The register address parameter is translated into an I2C slave
//              address and offset.
//              The block of data bytes is written to the I2C slave address
//              and offset.
//------------------------------------------------------------------------------

void SiIRegioWriteBlock ( uint16_t regAddr, uint8_t* buffer, uint16_t length)
{
	I2C_WriteBlock(SA_TX_CPI_Primary, (uint8_t)regAddr, buffer, length);
	/* for software reset workaroud, record how many times we write CEC register */
	SI_TxCountCpiWrites(length);
}

//------------------------------------------------------------------------------
// Function:    SiIRegioRead
// Description: Read a one byte register.
//              The register address parameter is translated into an I2C slave
//              address and offset. The I2C slave address and offset are used
//              to perform an I2C read operation.
//------------------------------------------------------------------------------

uint8_t SiIRegioRead ( uint16_t regAddr )
{
    return (I2C_ReadByte(SA_TX_CPI_Primary, (uint8_t)regAddr));
}

//------------------------------------------------------------------------------
// Function:    SiIRegioWrite
// Description: Write a one byte register.
//              The register address parameter is translated into an I2C
//              slave address and offset. The I2C slave address and offset
//              are used to perform an I2C write operation.
//------------------------------------------------------------------------------

void SiIRegioWrite ( uint16_t regAddr, uint8_t value )
{
	SiIRegioWriteBlock(regAddr, &value, 1);
}


//------------------------------------------------------------------------------
// Function:    SiIRegioModify
// Description: Modify a one byte register under mask.
//              The register address parameter is translated into an I2C
//              slave address and offset. The I2C slave address and offset are
//              used to perform I2C read and write operations.
//
//              All bits specified in the mask are set in the register
//              according to the value specified.
//              A mask of 0x00 does not change any bits.
//              A mask of 0xFF is the same a writing a byte - all bits
//              are set to the value given.
//              When only some bits in the mask are set, only those bits are
//              changed to the values given.
//------------------------------------------------------------------------------

void SiIRegioModify ( uint16_t regAddr, uint8_t mask, uint8_t value)
{
	uint8_t abyte;

	abyte = SiIRegioRead(regAddr);
	abyte &= (~mask);                                       //first clear all bits in mask
	abyte |= (mask & value);                                //then set bits from value
	SiIRegioWrite(regAddr, abyte);
}

/* SI_TxPreSwReset() is from SiI-SW-1090-SRC-v.3.5.10 */
void SI_TxPreSwReset(uint8_t fix)
{
	uint8_t just_zeros[8] = {0, 0, 0, 0, 0, 0, 0, 0};
	uint8_t wr_cntr;

	wr_cntr = (cpi_write_cntr + fix) & 0x7; /* %8 */

	if (wr_cntr)
		SiIRegioWriteBlock(0, just_zeros, 8 - wr_cntr);

	cpi_write_cntr = 0;
}

int cec_ok_after_reset(void)
{
	int backup, test, offset, cnt = 0;

	offset = REG_CEC_CAPTURE_ID0;
	backup = SiIRegioRead(offset);

	if (backup)
		test = 0;
	else
		test = 0xff;

	/* write and read back to make sure CEC is OK after software reset */
	SiIRegioWrite(offset, test);

	if (SiIRegioRead(offset) == test) {
		SiIRegioWrite(offset, backup);
		return 1;
	}

	while (SiIRegioRead(offset) != backup) {
		SiIRegioWrite(offset, backup);
		cnt++;

		if (cnt > 8)
			break;
	}

	return 0;
}

//------------------------------------------------------------------------------
// Function:    SI_CpiSetLogicalAddr
// Description: Configure the CPI subsystem to respond to a specific CEC
//              logical address.
//------------------------------------------------------------------------------

uint8_t SI_CpiSetLogicalAddr ( uint8_t logicalAddress )
{
    uint8_t capture_address[2];
    uint8_t capture_addr_sel = 0x01;

    capture_address[0] = 0;
    capture_address[1] = 0;
    if( logicalAddress < 8 )
    {
        capture_addr_sel = capture_addr_sel << logicalAddress;
        capture_address[0] = capture_addr_sel;
    }
    else
    {
        capture_addr_sel   = capture_addr_sel << ( logicalAddress - 8 );
        capture_address[1] = capture_addr_sel;
    }

        // Set Capture Address
#if AST_HDMITX
	/* FIXME : capture all frame?  */
#if 0
	capture_address[0] = 0xff;
	capture_address[1] = 0xff;
#endif
#endif

    SiIRegioWriteBlock(REG_CEC_CAPTURE_ID0, capture_address, 2 );
    SiIRegioWrite( REG_CEC_TX_INIT, logicalAddress );
	DEBUG_PRINT(MSG_STAT,("CEC: logicalAddress: 0x%x\n", (int)logicalAddress));

    return( true );
}

//------------------------------------------------------------------------------
// Function:    SI_CpiSendPing
// Description: Initiate sending a ping, and used for checking available
//                       CEC devices
//------------------------------------------------------------------------------

void SI_CpiSendPing ( uint8_t bCECLogAddr )
{
    SiIRegioWrite( REG_CEC_TX_DEST, BIT_SEND_POLL | bCECLogAddr );
}

//------------------------------------------------------------------------------
// Function:    SI_CpiWrite
// Description: Send CEC command via CPI register set
//------------------------------------------------------------------------------

uint8_t SI_CpiWrite( SI_CpiData_t *pCpi )
{
	uint8_t cec_int_status_reg[2];

#if AST_HDMITX
	if (!CableConnected) {
		int need_to_send = 0;
		/*
		 * HPD is low
		 * some TVs (Panasonic) put HPD at low at Standby state even if it is plugged into wall outlet
		 *
		 * for waking it up, we have to trasmit this kind of frame.
		 */
		if ((pCpi->srcDestAddr & 0x0F) == 0) { /* Dest is TV */
			if ((pCpi->opcode == 0x04) || (pCpi->opcode == 0x0D)) {
				/*
				 * 0x04:Image View On
				 * 0x0D:Text View On
				 */
				need_to_send = 1;
			}
		}

		if (need_to_send == 0)
			goto si_cpi_write_exit;
	}
#endif

#if (INCLUDE_CEC_NAMES > CEC_NO_TEXT)
	CpCecPrintCommand(pCpi, true);
#endif
#if AST_HDMITX
	/*
	 * DO NOT clear Tx buffer
	 * rapidly send N CEC frames, always get (N - 1) at CEC bus unless we disable this instruction
	 */
#else
	SiIRegioModify(REG_CEC_DEBUG_3, BIT_FLUSH_TX_FIFO, BIT_FLUSH_TX_FIFO);  // Clear Tx Buffer
#endif

        /* Clear Tx-related buffers; write 1 to bits to be cleared. */

	cec_int_status_reg[0] = 0x64 ; // Clear Tx Transmit Buffer Full Bit, Tx msg Sent Event Bit, and Tx FIFO Empty Event Bit
	cec_int_status_reg[1] = 0x02 ; // Clear Tx Frame Retranmit Count Exceeded Bit.
	SiIRegioWriteBlock(REG_CEC_INT_STATUS_0, cec_int_status_reg, 2);

        /* Send the command */
#if AST_HDMITX
	SiIRegioWrite(REG_CEC_TX_INIT, (pCpi->srcDestAddr >> 4) & 0x0F); /* Initiator */
#endif
	SiIRegioWrite(REG_CEC_TX_DEST, pCpi->srcDestAddr & 0x0F); /* Destination */
	SiIRegioWrite(REG_CEC_TX_COMMAND, pCpi->opcode);
	SiIRegioWriteBlock(REG_CEC_TX_OPERAND_0, pCpi->args, pCpi->argCount);
	SiIRegioWrite(REG_CEC_TRANSMIT_DATA, BIT_TRANSMIT_CMD | pCpi->argCount);

#if AST_HDMITX
si_cpi_write_exit:
#endif

	return (true);
}

//------------------------------------------------------------------------------
// Function:    SI_CpiRead
// Description: Reads a CEC message from the CPI read FIFO, if present.
//------------------------------------------------------------------------------

uint8_t SI_CpiRead( SI_CpiData_t *pCpi )
{
    uint8_t    error = false;
    uint8_t argCount;

    argCount = SiIRegioRead( REG_CEC_RX_COUNT );

    if ( argCount & BIT_MSG_ERROR )
    {
        error = true;
    }
    else
    {
        pCpi->argCount = argCount & 0x0F;
        pCpi->srcDestAddr = SiIRegioRead( REG_CEC_RX_CMD_HEADER );
        pCpi->opcode = SiIRegioRead( REG_CEC_RX_OPCODE );
        if ( pCpi->argCount )
        {
            SiIRegioReadBlock( REG_CEC_RX_OPERAND_0, pCpi->args, pCpi->argCount );
        }
    }

        // Clear CLR_RX_FIFO_CUR;
        // Clear current frame from Rx FIFO

    SiIRegioModify( REG_CEC_RX_CONTROL, BIT_CLR_RX_FIFO_CUR, BIT_CLR_RX_FIFO_CUR );

#if (INCLUDE_CEC_NAMES > CEC_NO_TEXT)
    if ( !error )
    {
        CpCecPrintCommand( pCpi, false );
    }
#endif
    return( error );
}

#if AST_HDMITX
extern struct sii9136_drv_data *drv_data;
#endif
//------------------------------------------------------------------------------
// Function:    SI_CpiStatus
// Description: Check CPI registers for a CEC event
//------------------------------------------------------------------------------

uint8_t SI_CpiStatus( SI_CpiStatus_t *pStatus )
{
    uint8_t cecStatus[2];

#if AST_HDMITX
	int tx_ready = 0;
#endif
    pStatus->txState    = 0;
    pStatus->cecError   = 0;
    pStatus->rxState    = 0;

    SiIRegioReadBlock( REG_CEC_INT_STATUS_0, cecStatus, 2);

    if ( (cecStatus[0] & 0x7F) || cecStatus[1] )
    {
        DEBUG_PRINT(MSG_STAT,("\nCEC Status: %02X %02X\n", (int) cecStatus[0], (int) cecStatus[1]));

            // Clear interrupts

        if ( cecStatus[1] & BIT_FRAME_RETRANSM_OV )
        {
            DEBUG_PRINT(MSG_DBG,("\n!TX retry count exceeded! [%02X][%02X]\n",(int) cecStatus[0], (int) cecStatus[1]));

                /* Flush TX, otherwise after clearing the BIT_FRAME_RETRANSM_OV */
                /* interrupt, the TX command will be re-sent.                   */

            SiIRegioModify( REG_CEC_DEBUG_3,BIT_FLUSH_TX_FIFO, BIT_FLUSH_TX_FIFO );
        }

            // Clear set bits that are set

        SiIRegioWriteBlock( REG_CEC_INT_STATUS_0, cecStatus, 2 );

            // RX Processing

        if ( cecStatus[0] & BIT_RX_MSG_RECEIVED )
        {
            pStatus->rxState = 1;   // Flag caller that CEC frames are present in RX FIFO
        }

            // RX Errors processing

        if ( cecStatus[1] & BIT_SHORT_PULSE_DET )
        {
            pStatus->cecError |= SI_CEC_SHORTPULSE;
        }

        if ( cecStatus[1] & BIT_START_IRREGULAR )
        {
            pStatus->cecError |= SI_CEC_BADSTART;
        }

        if ( cecStatus[1] & BIT_RX_FIFO_OVERRUN )
        {
            pStatus->cecError |= SI_CEC_RXOVERFLOW;
        }

            // TX Processing

        if ( cecStatus[0] & BIT_TX_FIFO_EMPTY )
        {
            pStatus->txState = SI_TX_WAITCMD;
        }
        if ( cecStatus[0] & BIT_TX_MESSAGE_SENT )
        {
            pStatus->txState = SI_TX_SENDACKED;
#if AST_HDMITX
		tx_ready = 1;
#endif
        }
        if ( cecStatus[1] & BIT_FRAME_RETRANSM_OV )
        {
            pStatus->txState = SI_TX_SENDFAILED;
#if AST_HDMITX
		tx_ready = 1;
#endif
        }
#if AST_HDMITX
	if (tx_ready)
		queue_work(drv_data->wq, &drv_data->work_cec_tx);
#endif
    }

    return( true );
}

//------------------------------------------------------------------------------
// Function:    SI_CpiGetLogicalAddr
// Description: Get Logical Address
//------------------------------------------------------------------------------

uint8_t SI_CpiGetLogicalAddr( void )
{
    return SiIRegioRead( REG_CEC_TX_INIT);
}

#if AST_HDMITX
static int topology_status_poll = 0;
static int topology_status = 0;

void si_cpi_topology_poll_cfg(u32 en)
{
	topology_status_poll = en;
}

void update_topology_status(u16 src)
{
	if (src > 14)
		return;

	topology_status |= (0x1 << src);
}

static int discovery_timeout = 160; /* 16 second (160 * 100ms), no idea why we need more than 10 seconds to get discovery completed in RCT sometimes */

void si_cpi_discovery_timeout_cfg(u32 cfg)
{
	discovery_timeout = cfg;
}

int si_cpi_discovery_timeout(void)
{
	return discovery_timeout;
}

u16 si_cpi_discovery_result(void)
{
	u16 result, cnt = 0, retry = 0;

start:
	if (retry > 2) {
		/* give up, return 0xFFFF let CEC driver decide what to do */
		result = 0xFFFF;
		goto exit;
	}

	/* clear discovery status */
	SiIRegioModify(REG_CEC_DISCOVERY_CTRL, DISCOVERY_STATUS_CLR, DISCOVERY_STATUS_CLR);
	/* start discovery */
	SiIRegioModify(REG_CEC_DISCOVERY_CTRL, DISCOVERY_START, DISCOVERY_START);

	while (1) {
		if (!CableConnected) {
			SiIRegioModify(REG_CEC_DISCOVERY_CTRL, DISCOVERY_START, 0);
			result = 0;
			goto exit;
		}

		msleep(100);
		if (SiIRegioRead(REG_CEC_DISCOVERY_CTRL) & DISCOVERY_DONE)
			break;

		cnt++;

		if (cnt > discovery_timeout) {
			/* too much time, retry */
			SiIRegioModify(REG_CEC_DISCOVERY_CTRL, DISCOVERY_START, 0);
			retry++;
			goto start;
		}
	}

	result = (SiIRegioRead(REG_CEC_DISCOVERY_STATUS_1) << 8) | SiIRegioRead(REG_CEC_DISCOVERY_STATUS_0);
exit:
	return result;
}

u16 si_cec_topology_status(void)
{
	if (topology_status_poll) {
		topology_status_poll = 0;
		topology_status = si_cpi_discovery_result();
	}

	return topology_status;
}
#endif
//------------------------------------------------------------------------------
// Function:    SI_CpiInit
// Description: Initialize the CPI subsystem for communicating via CEC
//------------------------------------------------------------------------------

uint8_t SI_CpiInit( void )
{

#ifdef DEV_SUPPORT_CEC_FEATURE_ABORT
    // Turn on CEC auto response to <Abort> command.
    SiIRegioWrite( CEC_OP_ABORT_31, BIT7 );
#else
    // Turn off CEC auto response to <Abort> command.
    SiIRegioWrite( CEC_OP_ABORT_31, CLEAR_BITS );
#endif

#ifdef DEV_SUPPORT_CEC_CONFIG_CPI_0
    // Bit 4 of the CC Config reister must be cleared to enable CEC
	SiIRegioModify (REG_CEC_CONFIG_CPI, 0x10, 0x00);
#endif

#if AST_HDMITX
	/* set to unregistered, will take new LA once get topology status from host */
	if (!SI_CpiSetLogicalAddr(CEC_LOGADDR_UNREGORBC)) {
		DEBUG_PRINT( MSG_ALWAYS, ("\n Cannot init CPI/CEC"));
		return false;
	}

	SiIRegioWrite(REG_CEC_INT_ENABLE_0, 0x22); /* [5]: message sent event, [1]: Rx msg available */
	SiIRegioWrite(REG_CEC_INT_ENABLE_1, 0x0F); /* */

	tx_q_init();
#else
    // initialized he CEC CEC_LOGADDR_TV logical address
    if ( !SI_CpiSetLogicalAddr( CEC_LOGADDR_PLAYBACK1 ))
    {
        DEBUG_PRINT( MSG_ALWAYS, ("\n Cannot init CPI/CEC"));
        return( false );
    }

#endif

    return( true );
}
#endif /* #if defined(CONFIG_ARCH_AST1500_CLIENT) && (IS_CEC == 1) */

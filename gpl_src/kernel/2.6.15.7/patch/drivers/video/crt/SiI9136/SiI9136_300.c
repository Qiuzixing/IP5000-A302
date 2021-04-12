/******************************************************************************
 * Copyright 2009-2011 Silicon Image, Inc.  All rights reserved.
 * No part of this work may be reproduced, modified, distributed, transmitted,
 * transcribed, or translated into any language or computer format, in any form
 * or by any means without written permission of:
 *   Silicon Image, Inc.
 *   1060 East Arques Avenue, Sunnyvale, California 94085
 ******************************************************************************/
/******************************************************************************
 *
 * @file sii9136_300.c
 *
 * @brief Implementation of the sii9136_300 platform.
 *
 ******************************************************************************/
//#include <stdio.h>
#include "defs.h"
#include "ast_utilities.h"
#include "si_basetypes.h"
#include "si_datatypes.h"
//#include "at89c51xd2.h"
#include "constants.h"
//#include "gpio.h"

#include "videomodedesc.h"
#include "i2c_slave_addrs.h"
#include "serialpacket.h"
#include "TPI_generic.h"
//#include "SiI92_56_1.h"
//#include "i2c_master_sw.h"
//#include "delay.h"
//#include "AMF_Lib.h"
#include "TPI_Access.h"
#include "TPI_Regs.h"
#include "Externals.h"
#include "tpidebug.h"
//#include "registers.h"

void siInputRxAutoVideoSetup (void);
void siInputRxAutoAudioSetup (void);

void TXHAL_InitMicroGpios (void)
{
}


void TXHAL_InitPreReset (void)
{
}


void TXHAL_InitPostReset (void)
{
#if !(AST_HDMITX)
	PinRxHwReset = LOW;
	DelayMS(RX_HW_RESET_PERIOD); // leave this one
	PinRxHwReset = HIGH;
#endif
	// PLL Bandwidth set to 0.75x [Bits 7:6 = 00]
	ReadModifyWriteTxPage0Register( TXL_PAGE_0_TMDS_CCTRL_ADDR, TXL_PAGE_0_REG_BWCTL_MASK, 0x00);
	WriteByteTPI(TXL_PAGE_0_HTPLG_T2_ADDR, 0x14);		// HW debounce to 64ms (0x14)
	WriteByteTPI(TXL_PAGE_0_TMDS_CTRL_ADDR, 0xA5);
}

void TXHAL_Init (void)
{
#if !(AST_HDMITX)
	AMF_SetOutColorSpace(OCS_RGB);
	AMF_Init(!DEBUG);
#endif
}


void TXHAL_Delays (void)
{
}


void TXHAL_PowerStateD0(void)
{
}

void TXHAL_PowerStateD3(void)
{
	ReadModifyWriteTPI(PAGE_0_TPI_SW_RST_EN_ADDR, BIT_0, ENABLE);
	DelayMS(10); //leave this one
	ReadModifyWriteTPI(PAGE_0_TPI_SW_RST_EN_ADDR, BIT_0, DISABLE);

	ReadModifyWriteTPI(PAGE_0_TPI_PWR_ST_ADDR, TX_POWER_STATE_MASK, TX_POWER_STATE_D3);
    ClrInTpiMode
}

void TXHAL_EnableInterrupts(uint8_t Interrupt_Pattern)
{
    ReadSetWriteTPI(PAGE_0_TPI_INTR_EN_ADDR, Interrupt_Pattern);
}

void TXHAL_DisableInterrupts(void)
{
}

//--------------------------------------------------------------------------
//  This is called when Audio Setup sees HBR Audio is selected.  It does the
//  special HBR configuration changed on the Samabeth and some audio change
//  stuff for Evita.
//--------------------------------------------------------------------------
void TXHAL_HBR_On (int Arg1)
{
#if !(AST_HDMITX)
    // Setup Samabeth
    I2C_WriteByte (SA_RX_Page1_Primary, RX_SMB_PAGE_1_AUDP0_MUTE_ADDR, 0x02);   //mute audio
	I2C_WriteByte (SA_RX_Page0_Primary, RX_SMB_PAGE_0_AEC0_CTRL_ADDR, 0x00);   	//disable auto control
    I2C_WriteByte (SA_RX_Page1_Primary, RX_SMB_PAGE_1_I2S_CTRL1_ADDR, 0x40);   	//16bit and word change on negative edge
    I2C_WriteByte (SA_RX_Page1_Primary, RX_SMB_PAGE_1_AUDRX_CTRL_ADDR, 0x05);   //SPDIF out enable and SCK and WS toggle
    I2C_WriteByte (SA_RX_Page1_Primary, RX_SMB_PAGE_1_FREQ_SVAL_RX_ADDR, 0x0E); //FREQ_SVAL  128 * Fs
    I2C_WriteByte (SA_RX_Page1_Primary, RX_SMB_PAGE_1_ACR_CTRL1_ADDR, 0x03);   	//overwrite incoming Fs with value in FREQ_SVAL&ACRINIT
    //I2C_WriteByte (SA_RX_Page0_Primary, RX_SMB_PAGE_0_SRST_RX_ADDR, 0x01);   	//SW Reset
    //I2C_WriteByte (SA_RX_Page0_Primary, RX_SMB_PAGE_0_SRST_RX_ADDR, 0x00);   	//clear SW Reset
    I2C_WriteByte (SA_RX_Page1_Primary, RX_SMB_PAGE_1_AUDP0_MUTE_ADDR, 0x00);   //un-mute audio
#endif
    // Setup Evita 300
    WriteByteTPI( PAGE_0_TPI_CONFIG3_ADDR, 0x90); 		//I2S, Layout0, Mute
    WriteByteTPI( PAGE_0_TPI_I2S_EN_ADDR, 0x80);
    WriteByteTPI( PAGE_0_TPI_I2S_EN_ADDR, 0x91);
    WriteByteTPI( PAGE_0_TPI_I2S_EN_ADDR, 0xA2);
    WriteByteTPI( PAGE_0_TPI_I2S_EN_ADDR, 0xB3);
	WriteByteTPI( PAGE_0_TPI_CONFIG4_ADDR, Arg1);	  	// Arg1: SS=bits7-6, Fs=bits5-3, HBRA=bit2,
    WriteByteTPI( PAGE_0_TPI_CONFIG3_ADDR, 0x80);  		//I2S, Layout0, Unmute
}

void TXHAL_HBR_Off (void)
{
}

#if !(AST_HDMITX)
//----------------------------------------------------------------------------------
//  This is called when the Color Depth is changed.  Since there is no AMF library
//  for Samabeth, and Samabeth must be re-configured when you go to Deep Color,
//  this HAL API is added so Evita 300 can re-configure Color Depth on the
//  Samabeth.
//
// Description: Configure the deep color clock based on input video format.
//              Change is made only when input color depth changes.
//-----------------------------------------------------------------------------------

void TXHAL_ColorDepthConfig (void)
{
    uint8_t bDcCtlValue;
    uint8_t bDcOutputVal;
	uint8_t bRegVal;

	// Video Mute ON
	bRegVal = I2C_ReadByte(SA_RX_Page1_Primary, 0x37);
	bRegVal |= 0x01;
	I2C_WriteByte(SA_RX_Page1_Primary, 0x37, bRegVal);

	/////////////////////////////////////////////////////////////////////////////////////
	// Select new TMDS Clock setting.  The setting depends on BOTH the Color Depth
	// we want as input AND if the muxYC output is enabled.
	/////////////////////////////////////////////////////////////////////////////////////

	// We get muxYC from the receiver's TMDS CCTRL2 register
	// All multiplexed modes will have bit 5 set.
    if (bRegVal = I2C_ReadByte(SA_RX_Page0_Primary, RX_SMB_PAGE_0_VIDA_AOF_ADDR) & BIT__MUXYC)
    {
      	// value is 2X if muxYC output is enabled
		switch(VideoModeDescription.InputColorDepth)
		{
			case VMD_COLOR_DEPTH_8BIT:
           		bDcCtlValue = VAL__DC_CTL_8BPP_2X;
    			bDcOutputVal = VAL__DITHER_8BITS;
				break;
			case VMD_COLOR_DEPTH_10BIT:
           		bDcCtlValue = VAL__DC_CTL_10BPP_2X;
    			bDcOutputVal = VAL__DITHER_10BITS;
				break;
			case VMD_COLOR_DEPTH_12BIT:
           		bDcCtlValue = VAL__DC_CTL_12BPP_2X;
    			bDcOutputVal = VAL__DITHER_12BITS;
				break;
			case VMD_COLOR_DEPTH_16BIT:
			default:
				TPI_DEBUG_PRINT((TPI_DEBUG_CHANNEL, "TXHAL_ColorDepthConfig: Unsupported Receiver Color Depth\n"));
				return;;
		}
    }
    else
    {
		// No muxYC so it is 1X
		switch(VideoModeDescription.InputColorDepth)
		{
			case VMD_COLOR_DEPTH_8BIT:
           		bDcCtlValue = VAL__DC_CTL_8BPP_1X;
    			bDcOutputVal = VAL__DITHER_8BITS;
				break;
			case VMD_COLOR_DEPTH_10BIT:
           		bDcCtlValue = VAL__DC_CTL_10BPP_1X;
    			bDcOutputVal = VAL__DITHER_10BITS;
				break;
			case VMD_COLOR_DEPTH_12BIT:
           		bDcCtlValue = VAL__DC_CTL_12BPP_1X;
    			bDcOutputVal = VAL__DITHER_12BITS;
				break;
			case VMD_COLOR_DEPTH_16BIT:
			default:
				TPI_DEBUG_PRINT((TPI_DEBUG_CHANNEL, "TXHAL_ColorDepthConfig: Unsupported Receiver Color Depth\n"));
				return;;
		}
	}

	// Turn OFF AVC so WE can control things
	bRegVal = I2C_ReadByte(SA_RX_Page0_Primary, 0xB5);
	bRegVal &= ~0x4;
	I2C_WriteByte(SA_RX_Page0_Primary, 0xB5, bRegVal);

	// Update the TMDS Clock on the receiver
	bRegVal = I2C_ReadByte(SA_RX_Page0_Primary, RX_SMB_PAGE_0_TMDS0_CCTRL2_ADDR);
	bRegVal &= ~MSK__DC_CTL;
	bRegVal |= bDcCtlValue;
	I2C_WriteByte(SA_RX_Page0_Primary, RX_SMB_PAGE_0_TMDS0_CCTRL2_ADDR, bRegVal);

	// reset the deep color FIFO by toggling it's reset bit
   	bRegVal = I2C_ReadByte(SA_RX_Page0_Primary, RX_SMB_PAGE_0_C0_HDCP_RST_ADDR);
   	bRegVal |=  BIT__DCFIFO_RST;
   	I2C_WriteByte(SA_RX_Page0_Primary, RX_SMB_PAGE_0_C0_HDCP_RST_ADDR, bRegVal);
   	bRegVal &= ~BIT__DCFIFO_RST;
   	I2C_WriteByte(SA_RX_Page0_Primary, RX_SMB_PAGE_0_C0_HDCP_RST_ADDR, bRegVal);

	// Change the receiver's Output Video Mode to use the new Color Depth
	bRegVal = I2C_ReadByte(SA_RX_Page0_Primary, RX_SMB_PAGE_0_VIDA_MODE2_ADDR);
	//bRegVal &= ~MSK__DITHER_MODE;
	bRegVal |= bDcOutputVal;
	I2C_WriteByte(SA_RX_Page0_Primary, RX_SMB_PAGE_0_VIDA_MODE2_ADDR, bRegVal);

	TPI_DEBUG_PRINT((TPI_DEBUG_CHANNEL, "Set receiver's Deep Color: Depth=0x%x ctl=0x%x\n", \
							(int)VideoModeDescription.InputColorDepth, (int)bDcCtlValue));

	// Video Mute OFF
	bRegVal = I2C_ReadByte(SA_RX_Page1_Primary, 0x37);
	bRegVal &= ~(0x01);
	I2C_WriteByte(SA_RX_Page1_Primary, 0x37, bRegVal);
}
#endif


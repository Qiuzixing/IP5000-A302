/*
 *****************************************************************************
 *
 * Copyright 2002-2009,2010, Silicon Image, Inc.  All rights reserved.
 * No part of this work may be reproduced, modified, distributed, transmitted,
 * transcribed, or translated into any language or computer format, in any form
 * or by any means without written permission of: Silicon Image, Inc., 1060
 * East Arques Avenue, Sunnyvale, California 94085
 *****************************************************************************
 */
/*
 *****************************************************************************
 * @file  AV_Config.c
 *
 * @brief Implementation of the audio and video API.
 *
 *****************************************************************************
*/
//#include <stdio.h>
#include "defs.h"
#include "ast_utilities.h"
#include "si_basetypes.h"
#include "si_datatypes.h"
//#include "at89c51xd2.h"
#include "constants.h"
//#include "gpio.h"


#include "i2c_slave_addrs.h"
#include "TPI_generic.h"
#include "TPI_Regs.h"
#include "Macros.h"
#include "TPI_Access.h"
//#include "delay.h"
//#include "i2c_master_sw.h"
#include "tpidebug.h"
#include "VideoModeTable.h"
//#include "AMF_Lib.h"
#include "videomodedesc.h"
#include "serialpacket.h"
#include "AV_Config.h"
#include "Externals.h"
//#include "SerialPort.h"
//#include "SimConfig.h"
#include "edid.h"
#include "HDCP.h"
#include "tpidebug.h"
#include "TPI.h"
//#include "si_cbus_tx.h"
#include "tpidrv.h"
#include "txhal.h"
#if (AST_HDMITX)
#include "SiI9136_main.h"
#endif

extern	void TXHAL_ColorDepthConfig(void);


// VSIF Constants
//===============
#define VSIF_TYPE  						0x81
#define VSIF_VERSION					0x01
#define VSIF_LEN						0x06


#define	PRINT_COLOR_SPACE(x)            \
	switch(x)                           \
	{                                   \
		case VMD_COLOR_SPACE_RGB:       \
			printk(" (RGB)");           \
			break;                      \
		case VMD_COLOR_SPACE_YCBCR422:  \
			printk(" (YCbCr4:2:2)");    \
			break;                      \
		case VMD_COLOR_SPACE_YCBCR444:  \
			printk(" (YCbCr4:4:4)");    \
			break;                      \
		default:                        \
			printk(" (Illegal)");       \
			break;                      \
	}

#define	PRINT_COLOR_DEPTH(x)            \
	switch(x)                           \
	{                                   \
		case VMD_COLOR_DEPTH_8BIT:      \
			printk(" (8bit)");          \
			break;                      \
		case VMD_COLOR_DEPTH_10BIT:     \
			printk(" (10bit)");         \
			break;                      \
		case VMD_COLOR_DEPTH_12BIT:     \
			printk(" (12bit)");         \
			break;                      \
		case VMD_COLOR_DEPTH_16BIT:     \
			printk(" (16bit)");         \
			break;                      \
		default:                        \
			printk(" (Illegal)");       \
			break;                      \
	}

#define	PRINT_RANGE(x)            		\
	switch(x)                           \
	{                                   \
		case VMD_VIDEO_RANGE_AUTO:      \
			printk(" (Auto)");          \
			break;                      \
		case VMD_VIDEO_RANGE_FULL:      \
			printk(" (Full)");          \
			break;                      \
		case VMD_VIDEO_RANGE_LIMITED:   \
			printk(" (Limited)");       \
			break;                      \
		default:                        \
			printk(" (Illegal)");       \
			break;                      \
	}



///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION         :   videoModeReport ()
//
// PURPOSE          :
//
// CALLED BY        :
//
// INPUT PARAMS     :
//
// OUTPUT PARAMS    :   None
//
// GLOBALS USED     :
//
// RETURNED VALUE   :   None
//
///////////////////////////////////////////////////////////////////////////////

void videoModeReport(char *pszFile, int iLine, VideoModeDescription_t *pVideoModeDescription)
{

    {
        printk("%s:%d HF:%d",pszFile,iLine, (int) pVideoModeDescription->HDMIVideoFormat);
    	switch(pVideoModeDescription->HDMIVideoFormat)
    	{
    		case VMD_HDMIFORMAT_CEA_VIC:
    			printk(" (CEA VIC)");
    			break;
    		case VMD_HDMIFORMAT_HDMI_VIC:
    			printk(" (HDMI VIC)");
    			break;
    		case VMD_HDMIFORMAT_3D:
    			printk(" (3D)");
    			break;
    		case VMD_HDMIFORMAT_PC:
    			printk(" (PC)");
    			break;
    		default:
    			printk(" (Illegal)");
    			break;
    	}
        printk(" VIC:%d", (int) pVideoModeDescription->VIC);
        printk(" A:%x", (int) pVideoModeDescription->AspectRatio);
    	switch(pVideoModeDescription->AspectRatio)
    	{
    		case VMD_ASPECT_RATIO_4x3:
    			printk(" (4x3)");
    			break;
    		case VMD_ASPECT_RATIO_16x9:
    			printk(" (16x9)");
    			break;
    		default:
    			printk(" (Illegal)");
    			break;
    	}
        printk(" ICS:%d", (int) pVideoModeDescription->InputColorSpace);
    	PRINT_COLOR_SPACE(pVideoModeDescription->InputColorSpace);
        printk(" ICD:%d", (int) pVideoModeDescription->InputColorDepth);
    	PRINT_COLOR_DEPTH(pVideoModeDescription->InputColorDepth);
        printk(" IRX:%d", (int) pVideoModeDescription->InputRangeExpansion);
    	PRINT_RANGE(pVideoModeDescription->InputRangeExpansion);
        printk(" OCS:%x", (int) pVideoModeDescription->OutputColorSpace);
    	PRINT_COLOR_SPACE(pVideoModeDescription->OutputColorSpace);
        printk(" OCD:%x", (int) pVideoModeDescription->OutputColorDepth);
    	PRINT_COLOR_DEPTH(pVideoModeDescription->OutputColorDepth);
        printk(" ORC:%x", (int) pVideoModeDescription->OutputRangeCompression);
    	PRINT_RANGE(pVideoModeDescription->OutputRangeCompression);
        printk(" HA:%d", (int) pVideoModeDescription->HDCPAuthenticated);
    	if (pVideoModeDescription->HDCPAuthenticated == VMD_HDCP_AUTHENTICATED)
    	{
    		printk(" (Authenticated)\n");
    	}
    	else
    	{
    		printk(" (NOT Authenticated)\n");
    	}
    	if (pVideoModeDescription->HDMIVideoFormat == VMD_HDMIFORMAT_3D)
    	{
        	printk(" 3D:%d", (int) pVideoModeDescription->ThreeDStructure);
    		switch(pVideoModeDescription->ThreeDStructure)
    		{
    			case VMD_3D_EXT_DATA_MAP:
    				printk(" (Ext Data Map)\n");
    				break;
    			case VMD_3D_FRAMEPACKING:
    				printk(" (Frame Packing)\n");
    				break;
    			case VMD_3D_FIELDALTERNATIVE:
    				printk(" (Field Alternating)\n");
    				break;
    			case VMD_3D_LINEALTERNATIVE:
    				printk("(Line Alternating)\n");
    				break;
    			case VMD_3D_SIDEBYSIDEFULL:
    				printk(" (Side-by-side Full)\n");
    				break;
    			case VMD_3D_LDEPTH:
    				printk(" (L Depth)\n");
    				break;
    			case VMD_3D_LDEPTHGRAPHICS:
    				printk(" (L Depth Graphics)\n");
    				break;
    			case VMD_3D_SIDEBYSIDEHALF:
    				printk(" (Side-by-side half)");
        			printk(" 3Dx:%d\n", (int) pVideoModeDescription->ThreeDExtData);
    				break;
    			default:
    				printk(" (Illegal)");
    				break;
    		}
    		printk("\n");
    	}
    }
}

//////////////////////////////////////////////////////////////////////////////
//
// FUNCTION      :  InitVideo()
//
// PURPOSE       :  Set the 9022/4 to the video mode determined by GetVideoMode()
//
// INPUT PARAMS  :  Index of video mode to set; Flag that distinguishes between
//                  calling this function after power up and after input
//                  resolution change
//
// OUTPUT PARAMS :  None
//
// GLOBALS USED  :  VModesTable, VideoCommandImage
//
// RETURNS       :  TRUE
//
//////////////////////////////////////////////////////////////////////////////

uint8_t InitVideo(uint8_t TclkSel)
{
	int	ModeTblIndex;
#ifdef DEEP_COLOR
	uint8_t temp;
#endif
	uint8_t B_Data[8];

#ifdef DEV_EMBEDDED
	uint8_t EMB_Status;
#endif
#ifdef USE_DE_GENERATOR
	uint8_t DE_Status;
#endif

#ifndef DEV_INDEXED_PLL
	uint8_t Pattern;
#endif

#if (AST_HDMITX)
	return TRUE;
#endif
	TPI_ENTRY_PRINT((TPI_ENTRY_EXIT_CHANNEL,"HA:%X\n",(int)VideoModeDescription.HDCPAuthenticated));
	TPI_TRACE_PRINT((TPI_TRACE_CHANNEL,">>InitVideo()\n"));

	VideoModeReport(&VideoModeDescription);

	ModeTblIndex = (uint8_t)ConvertVIC_To_VM_Index();

#ifdef DEV_INDEXED_PLL
	SetPLL(TclkSel);
#else
	Pattern = (TclkSel << 6) & TWO_MSBITS;								// Use TPI 0x08[7:6] for 9022A/24A video clock multiplier
	ReadSetWriteTPI(PAGE_0_TPI_PRD_ADDR, Pattern);
#endif

	////////////////////////////////////////////////////////////////
	//  Set Pixel Repetition
	////////////////////////////////////////////////////////////////

	// Take values from VModesTable[]:

	B_Data[0] = VModesTable[ModeTblIndex].PixClk & 0x00FF;			// write Pixel clock to TPI registers 0x00, 0x01
	B_Data[1] = (VModesTable[ModeTblIndex].PixClk >> 8) & 0xFF;

	B_Data[2] = VModesTable[ModeTblIndex].Tag.VFreq & 0x00FF;			// write Vertical Frequency to TPI registers 0x02, 0x03
	B_Data[3] = (VModesTable[ModeTblIndex].Tag.VFreq >> 8) & 0xFF;

	B_Data[4] = VModesTable[ModeTblIndex].Tag.Total.Pixels & 0x00FF;	// write total number of pixels to TPI registers 0x04, 0x05
	B_Data[5] = (VModesTable[ModeTblIndex].Tag.Total.Pixels >> 8) & 0xFF;

	B_Data[6] = VModesTable[ModeTblIndex].Tag.Total.Lines & 0x00FF;	// write total number of lines to TPI registers 0x06, 0x07
	B_Data[7] = (VModesTable[ModeTblIndex].Tag.Total.Lines >> 8) & 0xFF;

	WriteBlockTPI(PAGE_0_TPI_PIX_CLK_LSB_ADDR, 8, B_Data);					// Write TPI Mode data.

	B_Data[0] = (VModesTable[ModeTblIndex].PixRep) & LOW_BYTE;		// Set pixel replication field of 0x08
	B_Data[0] |= BIT_BUS_24;									// Set 24 bit bus

#ifndef DEV_INDEXED_PLL
	B_Data[0] |= (TclkSel << 6) & TWO_MSBITS;
#endif
#ifdef CLOCK_EDGE_FALLING
	B_Data[0] &= ~BIT_EDGE_RISE;								// Set to falling edge
#endif
#ifdef CLOCK_EDGE_RISING
	B_Data[0] |= BIT_EDGE_RISE;									// Set to rising edge
#endif
	WriteByteTPI(PAGE_0_TPI_PRD_ADDR, B_Data[0]);				// 0x08

#ifdef DEV_EMBEDDED
	EMB_Status = SetEmbeddedSync();
	EnableEmbeddedSync();
#endif


	////////////////////////////////////////////////////////////////
	//  Set Input and Output Color Space & Color Depth
	//  B_Data[0] is Input Settings
	//  B_Data[1] is Output Settings
	////////////////////////////////////////////////////////////////
#ifdef USE_DE_GENERATOR
	ReadClearWriteTPI(PAGE_0_TPI_SYNC_GEN_ADDR, MSBIT);				// set 0x60[7] = 0 for External Sync
	DE_Status = SetDE();									// Call SetDE() with Video Mode as a parameter
	B_Data[0] = (((BITS_IN_YCBCR422 | BITS_IN_AUTO_RANGE) & ~BIT_EN_DITHER_10_8) & ~BIT_EXTENDED_MODE);	// 0x09
#endif
//#else
#ifdef DEEP_COLOR
	B_Data[0] = 0;

	// Set Input Color Space; bits[1:0]
	switch (VideoModeDescription.InputColorSpace)
	{
		case VMD_COLOR_SPACE_YCBCR422:
			temp = ICS_YCBCR422;
			break;
		case VMD_COLOR_SPACE_YCBCR444:
			temp = ICS_YCBCR444;
			break;
		case VMD_COLOR_SPACE_RGB:
		default:
			temp = ICS_RGB;
			break;
	}
	B_Data[0] = ((B_Data[0] & ~VMD_COLOR_SPACE_MAP) | temp);

	// Set Input Range Expansion; bits[3:2]
	switch (VideoModeDescription.InputRangeExpansion)
	{
		case VMD_VIDEO_RANGE_AUTO:
			temp = IVRX_AUTO;
			break;
		case VMD_VIDEO_RANGE_FULL:
			temp = IVRX_ON;
			break;
		case VMD_VIDEO_RANGE_LIMITED:
			temp = IVRX_OFF;
			break;
		default:
			temp = IVRX_AUTO;
			break;
	}
	B_Data[0] = ((B_Data[0] & ~VMD_VIDEO_RANGE_MAP) | temp);
//#endif

//#ifdef DEEP_COLOR
	// Set Input Color Depth; bits[7:6]
	switch (VideoModeDescription.InputColorDepth)
	{
		case VMD_COLOR_DEPTH_16BIT:
			temp = 0x40;
			break;
		case VMD_COLOR_DEPTH_10BIT:
		   	temp = 0x80;
			break;
		case VMD_COLOR_DEPTH_12BIT:
			temp = 0xC0;
			break;
		case VMD_COLOR_DEPTH_8BIT:
		default:
			temp = 0x00;
			break;
	}
	B_Data[0] = ((B_Data[0] & ~VMD_COLOR_DEPTH_MAP) | temp);	// Set Color Depth bits
#endif

#ifdef DEV_EMBEDDED
	B_Data[0] = ((B_Data[0] & 0xFC) | 0x02A);	// Force YCbCr4:2:2 and Video Range Expansion OFF
#endif

	//**************************************
	//  Do Output Format Register contents.
	//**************************************

	//Video Range Compression to AUTO.  Bits[3:2].  Also clears Dither; Bit[5]
	//and Color Space Standard; bit[4]
	B_Data[1] = BITS_OUT_AUTO_RANGE;

#ifdef DEEP_COLOR
	// Setup Output Color Depth bits [7:6]
	switch (VideoModeDescription.OutputColorDepth)
	{
		case VMD_COLOR_DEPTH_16BIT:
			temp = 0x40;
			ReadModifyWriteTPI(PAGE_0_TPI_SW_RST_EN_ADDR, TPI_SW_RST_EN_REG_DC_PKT_EN_MASK, TPI_SW_RST_EN_REG_DC_PKT_EN_ENABLE);	// deep color packet enable
			break;
		case VMD_COLOR_DEPTH_10BIT:
			temp = 0x80;
			ReadModifyWriteTPI(PAGE_0_TPI_SW_RST_EN_ADDR, TPI_SW_RST_EN_REG_DC_PKT_EN_MASK, TPI_SW_RST_EN_REG_DC_PKT_EN_ENABLE);	// deep color packet enable
			break;
		case VMD_COLOR_DEPTH_12BIT:
			temp = 0xC0;
			ReadModifyWriteTPI(PAGE_0_TPI_SW_RST_EN_ADDR, TPI_SW_RST_EN_REG_DC_PKT_EN_MASK, TPI_SW_RST_EN_REG_DC_PKT_EN_ENABLE);	// deep color packet enable
			break;
		case VMD_COLOR_DEPTH_8BIT:
		default:
			temp = 0x00;
			ReadModifyWriteTPI(PAGE_0_TPI_SW_RST_EN_ADDR, TPI_SW_RST_EN_REG_DC_PKT_EN_MASK, TPI_SW_RST_EN_REG_DC_PKT_EN_DISABLE);	// disable deep color packet
			break;
	}
	B_Data[1] = ((B_Data[1] & ~VMD_COLOR_DEPTH_MAP) | temp);
//#endif

	// Setup Output Color Space.  Bits[1:0]
	switch (VideoModeDescription.OutputColorSpace)
	{
		case VMD_COLOR_SPACE_YCBCR422:
            temp = 0x02;
			break;
		case VMD_COLOR_SPACE_YCBCR444:
            temp = 0x01;
			break;
		case VMD_COLOR_SPACE_RGB:
		default:
        	temp = 0x00;
			break;
	}
	B_Data[1] = ((B_Data[1] & ~VMD_COLOR_SPACE_MAP) | temp);
#endif

	SetFormat(B_Data);

	ReadClearWriteTPI(PAGE_0_TPI_SYNC_GEN_ADDR, BIT_2);	// Number HSync pulses from VSync active edge to Video Data Period should be 20 (VS_TO_VIDEO)

#ifdef SOURCE_TERMINATION_ON
	{
		uint8_t regData;
		regData = ReadIndexedRegister(INDEXED_PAGE_0, TMDS_CONT_REG);
		regData = (regData & 0x3F) | 0x25;
		WriteIndexedRegister(INDEXED_PAGE_0, TMDS_CONT_REG, regData);
	}
#endif

	TPI_EXIT_PRINT((TPI_ENTRY_EXIT_CHANNEL,"\n"));
	return TRUE;
}


#if (AST_HDMITX)
static u32 output_hdmi = 0;
/*
 * [1:0]: clock edge select: 1: rising, 0: falling
 * [1]: 4K clock edge select, default: falling
 * [0]: 1080 clock edge select, default:rising
 */
static u32 clk_edge_select = 0x1;

/*
 * sii9136_video_mode - set video mode
 *
 * @pixel_clock: pixel clock /10000
 * @vfreq: vertical frequency in Hz
 * @pixels: total pixels pre line
 * @lines: total lines
 *
 */
void sii9136_video_mode(u16 pixel_clock, u16 vfreq, u16 pixels, u16 lines)
{
	u8 B_Data[8];

	P_TRACE("[%s]pixel_clock %d, vfreq %d, pixels %d, lines %d\n",
		__func__, pixel_clock, vfreq, pixels, lines);

	B_Data[0] = pixel_clock & 0x00FF;
	B_Data[1] = (pixel_clock >> 8) & 0xFF;

	B_Data[2] = vfreq & 0x00FF;
	B_Data[3] = (vfreq >> 8) & 0xFF;

	B_Data[4] = pixels & 0x00FF;
	B_Data[5] = (pixels >> 8) & 0xFF;

	B_Data[6] = lines & 0x00FF;
	B_Data[7] = (lines >> 8) & 0xFF;

	WriteBlockTPI(PAGE_0_TPI_PIX_CLK_LSB_ADDR, 8, B_Data);
}

static u8 video_input_format(u8 color_depth, u8 range_expansion, u8 color_space)
{
	u8 data;

	P_TRACE("[%s] color_depth %d, range_expansion %d, color_space %d\n",
		__func__, color_depth, range_expansion, color_space);

	data = ((color_depth & 0x3) << 6)
		| ((range_expansion & 0x3) << 2)
		| (color_space & 0x3);

	return data;
}

static u8 video_output_format(u8 color_depth, u8 dither, u8 color_space_std, u8 range_compression, u8 format)
{
	u8 data;
	P_TRACE("[%s] color_depth %d, dither %d, color_space_std %d, range_compression %d, format %d\n",
		__func__, color_depth, dither, color_space_std, range_compression, format);

	data = ((color_depth & 0x3) << 6)
		| ((dither & 0x1) << 5)
		| ((color_space_std & 0x1) << 4)
		| ((range_compression & 0x3) << 2)
		| (format & 0x3);
	return data;
}

void sii9136_video_mode_input_bus_pixel_rep(u8 tclk_sel, u8 input_bus_sel, u8 edge_select, u8 pixel_repetition)
{
	u8 data;

	data = ((tclk_sel & 0x3) << 6)
		| ((input_bus_sel & 0x1) << 5)
		| (pixel_repetition & 0xF);

	if ((clk_edge_select >> edge_select) & 0x1)
		data |= BIT_EDGE_RISE;
	else
		data &= ~BIT_EDGE_RISE; /* Set to falling edge */


	WriteByteTPI(PAGE_0_TPI_PRD_ADDR, data);
}

void sii9136_output_hdmi_cfg(u32 enable)
{
	if (enable) {
		ReadModifyWriteTPI(PAGE_0_TPI_SC_ADDR, OUTPUT_MODE_MASK, OUTPUT_MODE_HDMI);
		output_hdmi = 1;
	} else {
		ReadModifyWriteTPI(PAGE_0_TPI_SC_ADDR, OUTPUT_MODE_MASK, OUTPUT_MODE_DVI);
		output_hdmi = 0;
	}
}

u32 sii9136_output_hdmi(void)
{
#if 1
	return output_hdmi;
#else
	return TPI_PAGE_0_REG_TPI_OUTPUT_MODE_B0_RD(ReadByteTPI(PAGE_0_TPI_SC_ADDR));
#endif
}

void sii9136_clk_edge_cfg(u32 cfg)
{
	clk_edge_select = cfg;
}

u32 sii9136_clk_edge(void)
{
	return clk_edge_select;
}

void sii9136_video_format(u32 input_color_space, u32 output_format, u32 color_space_std, u32 color_depth, u32 hdmi, u32 input_range_expansion, u32 output_range_compression)
{
	u8 B_Data[2], depth, enable_gcp;

	switch (color_depth) {
	case 24:
		depth = VMD_COLOR_DEPTH_8BIT;
		break;
	case 30:
		depth = VMD_COLOR_DEPTH_10BIT;
		break;
	case 36:
		depth = VMD_COLOR_DEPTH_12BIT;
		break;
	case 48:
		depth = VMD_COLOR_DEPTH_16BIT;
		break;
	default:
		depth = VMD_COLOR_DEPTH_8BIT;
		break;
	}


	/* deep color setting require general control packet to be sent with color depth information */
	if (VMD_COLOR_DEPTH_8BIT == depth)
		enable_gcp = 0;
	else
		enable_gcp = 1;

	ReadModifyWriteTPI(PAGE_0_TPI_SW_RST_EN_ADDR,
				TPI_PAGE_0_REG_DC_PKT_EN_MASK,
				TPI_PAGE_0_REG_DC_PKT_EN_WR(enable_gcp));

	B_Data[0] = video_input_format(VMD_COLOR_DEPTH_8BIT, input_range_expansion, input_color_space);
	B_Data[1] = video_output_format(depth, 1, color_space_std, output_range_compression, output_format);

	/*
	 * from SiI-PR-1060-C, p15
	 * switch to 8-bit color depth: switch to 16-bit first and then to 8-bit
	 */
	if (VMD_COLOR_DEPTH_8BIT == depth) {
		WriteByteTPI(PAGE_0_TPI_OUTPUT_ADDR,
			video_output_format(VMD_COLOR_DEPTH_16BIT, 1, color_space_std, output_range_compression, output_format));
	}

	sii9136_output_hdmi_cfg(1); /* Set HDMI mode to allow color space conversion */
	WriteBlockTPI(PAGE_0_TPI_INPUT_ADDR, 2, B_Data); /* Program TPI AVI Input and Output Format */

	if (!hdmi) { /* clear AVI infoframe field when DVI */
		int i;

		for (i = TPI_AVI_BYTE_0; i < PAGE_0_TPI_END_RIGHT_BAR_MSB_ADDR; i++)
			WriteByteTPI(i, 0x00);
	}

	WriteByteTPI(PAGE_0_TPI_END_RIGHT_BAR_MSB_ADDR, 0x00); /* Set last byte of TPI AVI InfoFrame for TPI AVI I/O Format to take effect */

	sii9136_output_hdmi_cfg(hdmi);

	/* SiI-PR-1060-C p17, recommend to clear bit 2 for normal operation */
	ReadClearWriteTPI(PAGE_0_TPI_SYNC_GEN_ADDR, BIT_2);
}
#endif /* #if (AST_HDMITX) */


//////////////////////////////////////////////////////////////////////////////
//
// FUNCTION     :   SetFormat(uint8_t * Data)
//
// PURPOSE      :
//
// INPUT PARAMS :
//
// OUTPUT PARAMS:
//
// GLOBALS USED :
//
// RETURNS      :
//
//////////////////////////////////////////////////////////////////////////////
void SetFormat(uint8_t *Data)
{
	if (!IsHDMI_Sink())
		ReadModifyWriteTPI(PAGE_0_TPI_SC_ADDR, OUTPUT_MODE_MASK, OUTPUT_MODE_HDMI); /* Set HDMI mode to allow color space conversion */

	WriteBlockTPI(PAGE_0_TPI_INPUT_ADDR, 2, Data); /* Program TPI AVI Input and Output Format */
	WriteByteTPI(PAGE_0_TPI_END_RIGHT_BAR_MSB_ADDR, 0x00); /* Set last byte of TPI AVI InfoFrame for TPI AVI I/O Format to take effect */

	if (!IsHDMI_Sink())
		ReadModifyWriteTPI(PAGE_0_TPI_SC_ADDR, OUTPUT_MODE_MASK, OUTPUT_MODE_DVI);

#ifdef DEV_EMBEDDED
	EnableEmbeddedSync(); /* Last byte of TPI AVI InfoFrame resets Embedded Sync Extraction */
#endif
}

//
// FUNCTION     :   SetEmbeddedSync()
//
// PURPOSE      :   Set the 9022/4 registers to extract embedded sync.
//
// INPUT PARAMS :   Index of video mode to set
//
// OUTPUT PARAMS:   None
//
// GLOBALS USED :   VModesTable[]
//
// RETURNS      :   TRUE
//
//////////////////////////////////////////////////////////////////////////////
uint8_t SetEmbeddedSync()
{
    int	ModeTblIndex;
    uint16_t H_Bit_2_H_Sync;
    uint16_t Field2Offset;
    uint16_t H_SyncWidth;

    uint8_t V_Bit_2_V_Sync;
    uint8_t V_SyncWidth;
    uint8_t B_Data[8];

    TPI_TRACE_PRINT((TPI_TRACE_CHANNEL,">>SetEmbeddedSync()\n"));

	ReadModifyWriteTxPage0Register( TXL_PAGE_0_SYS_CTRL3_ADDR, 0x01, 0x01);

	ReadClearWriteTPI(PAGE_0_TPI_SYNC_GEN_ADDR, MSBIT);	 // set 0x60[7] = 0 for DE mode
	WriteByteTPI(PAGE_0_TPI_HBIT_2HSYNC2_ADDR, 0x30);
    ReadSetWriteTPI(PAGE_0_TPI_SYNC_GEN_ADDR, MSBIT);       // set 0x60[7] = 1 for Embedded Sync

    ModeTblIndex = ConvertVIC_To_VM_Index();

    H_Bit_2_H_Sync = VModesTable[ModeTblIndex]._656.HBit2HSync;
    Field2Offset = VModesTable[ModeTblIndex]._656.Field2Offset;
    H_SyncWidth = VModesTable[ModeTblIndex]._656.HLength;
    V_Bit_2_V_Sync = VModesTable[ModeTblIndex]._656.VBit2VSync;
    V_SyncWidth = VModesTable[ModeTblIndex]._656.VLength;

    B_Data[0] = H_Bit_2_H_Sync & LOW_BYTE;                  // Setup HBIT_TO_HSYNC 8 LSBits (0x62)

    B_Data[1] = (H_Bit_2_H_Sync >> 8) & TWO_LSBITS;         // HBIT_TO_HSYNC 2 MSBits
    //B_Data[1] |= BIT_EN_SYNC_EXTRACT;                     // and Enable Embedded Sync to 0x63

    EmbeddedSynPATCH = B_Data[1];

    B_Data[2] = Field2Offset & LOW_BYTE;                    // 8 LSBits of "Field2 Offset" to 0x64
    B_Data[3] = (Field2Offset >> 8) & LOW_NIBBLE;           // 2 MSBits of "Field2 Offset" to 0x65

    B_Data[4] = H_SyncWidth & LOW_BYTE;
    B_Data[5] = (H_SyncWidth >> 8) & TWO_LSBITS;                    // HWIDTH to 0x66, 0x67
    B_Data[6] = V_Bit_2_V_Sync;                                     // VBIT_TO_VSYNC to 0x68
    B_Data[7] = V_SyncWidth;                                        // VWIDTH to 0x69

	WriteBlockTPI(PAGE_0_TPI_HBIT_2HSYNC1_ADDR, 8, &B_Data[0]);
    return TRUE;
}

//////////////////////////////////////////////////////////////////////////////
//
// FUNCTION     :   EnableEmbeddedSync
//
// PURPOSE      :
//
// INPUT PARAMS :
//
// OUTPUT PARAMS:
//
// GLOBALS USED :
//
// RETURNS      :
//
//////////////////////////////////////////////////////////////////////////////
void EnableEmbeddedSync()
{
    TPI_TRACE_PRINT((TPI_TRACE_CHANNEL,">>EnableEmbeddedSync()\n"));

	ReadClearWriteTPI(PAGE_0_TPI_SYNC_GEN_ADDR, MSBIT);	 // set 0x60[7] = 0 for DE mode
	WriteByteTPI(PAGE_0_TPI_HBIT_2HSYNC2_ADDR, 0x30);
    ReadSetWriteTPI(PAGE_0_TPI_SYNC_GEN_ADDR, MSBIT);       // set 0x60[7] = 1 for Embedded Sync
	ReadSetWriteTPI(PAGE_0_TPI_HBIT_2HSYNC2_ADDR, BIT_6);
}

//////////////////////////////////////////////////////////////////////////////
//
// FUNCTION     :   SetDE()
//
// PURPOSE      :   Set the 9022/4 internal DE generator parameters
//
// INPUT PARAMS :
//
// OUTPUT PARAMS:   None
//
// GLOBALS USED :   None
//
// RETURNS      :   DE_SET_OK
//
// NOTE         :   0x60[7] must be set to "0" for the follwing settings to
//                  take effect
//
//////////////////////////////////////////////////////////////////////////////
uint8_t SetDE()
{
    uint8_t RegValue;
	int	ModeTblIndex;

    uint16_t H_StartPos;
    uint16_t V_StartPos;
	uint16_t Htotal;
	uint16_t Vtotal;
    uint16_t H_Res;
    uint16_t V_Res;

    uint8_t Polarity;
    uint8_t B_Data[12];

    TPI_TRACE_PRINT((TPI_TRACE_CHANNEL,">>SetDE()\n"));

    ModeTblIndex = ConvertVIC_To_VM_Index();

     // Make sure that External Sync method is set before enableing the DE Generator:
    RegValue = ReadByteTPI(PAGE_0_TPI_SYNC_GEN_ADDR);

    if (RegValue & BIT_7)
    {
        return DE_CANNOT_BE_SET_WITH_EMBEDDED_SYNC;
    }

    H_StartPos = VModesTable[ModeTblIndex].Pos.H;
    V_StartPos = VModesTable[ModeTblIndex].Pos.V;

   	Htotal = VModesTable[ModeTblIndex].Tag.Total.Pixels;
	Vtotal = VModesTable[ModeTblIndex].Tag.Total.Lines;

    Polarity = (~VModesTable[ModeTblIndex].Tag.RefrTypeVHPol) & TWO_LSBITS;

    H_Res = VModesTable[ModeTblIndex].Res.H;

        if ((VModesTable[ModeTblIndex].Tag.RefrTypeVHPol & 0x04))
        {
        	V_Res = (VModesTable[ModeTblIndex].Res.V) >> 1;
        }
        else
        {
            V_Res = (VModesTable[ModeTblIndex].Res.V);
        }

    B_Data[0] = H_StartPos & LOW_BYTE;              // 8 LSB of DE DLY in 0x62

    B_Data[1] = (H_StartPos >> 8) & TWO_LSBITS;     // 2 MSBits of DE DLY to 0x63
    B_Data[1] |= (Polarity << 4);                   // V and H polarity
    B_Data[1] |= BIT_EN_DE_GEN;                     // enable DE generator

    B_Data[2] = V_StartPos & SEVEN_LSBITS;      // DE_TOP in 0x64
    B_Data[3] = 0x00;                           // 0x65 is reserved
    B_Data[4] = H_Res & LOW_BYTE;               // 8 LSBits of DE_CNT in 0x66
    B_Data[5] = (H_Res >> 8) & LOW_NIBBLE;      // 4 MSBits of DE_CNT in 0x67
    B_Data[6] = V_Res & LOW_BYTE;               // 8 LSBits of DE_LIN in 0x68
    B_Data[7] = (V_Res >> 8) & THREE_LSBITS;    // 3 MSBits of DE_LIN in 0x69
	// H_RES and V_RES measure the time between two sync active edges and may vary
	// slightly from one reading to the next. The values in these registers are
	// accurate only when there are active HSYNC and VSYNC controls arriving on the
	// video input. These are read only registers however the block of TPI registers
	// in the Explicit Sync DE Generator Register block do not take affect until the
	// entire block is written.
	B_Data[8] = Htotal & LOW_BYTE;				// 8 LSBits of H_RES in 0x6A
	B_Data[9] =	(Htotal >> 8) & LOW_NIBBLE;		// 4 MSBITS of H_RES in 0x6B
	B_Data[10] = Vtotal & LOW_BYTE;				// 8 LSBits of V_RES in 0x6C
	B_Data[11] = (Vtotal >> 8) & BITS_2_1_0;	// 3 MSBITS of V_RES in 0x6D

    WriteBlockTPI(PAGE_0_TPI_HBIT_2HSYNC1_ADDR, 12, &B_Data[0]);

    return DE_SET_OK;                               // Write completed successfully
}



//////////////////////////////////////////////////////////////////////////////
//
// FUNCTION      : SetBasicAudio()
//
// PURPOSE       : Set the 9022/4 audio interface to basic audio.
//
// INPUT PARAMS  : None
//
// OUTPUT PARAMS : None
//
// GLOBALS USED  : None
//
// RETURNS       : void.
//
//////////////////////////////////////////////////////////////////////////////
void SetBasicAudio(void)
{
   uint8_t CodingType;
   uint8_t ChannelCount;
   uint8_t SS;
   uint8_t Fs;
   uint8_t SpkConfig;

#ifdef HBR_AUDIO
   unsigned char hbrAudio;
   int AudioConfigSource;
#endif

	TPI_TRACE_PRINT((TPI_TRACE_CHANNEL,">>SetBasicAudio()\n"));

#ifdef I2S_AUDIO
    WriteByteTPI(PAGE_0_TPI_CONFIG3_ADDR,  AUD_IF_I2S);                             // 0x26
    WriteByteTPI(PAGE_0_TPI_AUDIO_HANDLING_ADDR, 0x08 | AUD_DO_NOT_CHECK);          // 0x25
#else
    WriteByteTPI(PAGE_0_TPI_CONFIG3_ADDR, AUD_IF_SPDIF);                    // 0x26 = 0x40
    WriteByteTPI(PAGE_0_TPI_AUDIO_HANDLING_ADDR, AUD_PASS_BASIC);                   // 0x25 = 0x00
#endif

    ReadClearWriteTPI(PAGE_0_TPI_CONFIG3_ADDR, BIT_5); //channel layout

#ifdef I2S_AUDIO
        // I2S - Map channels - replace with call to API MAPI2S
        WriteByteTPI(PAGE_0_TPI_I2S_EN_ADDR, 0x80); // 0x1F
//        WriteByteTPI(PAGE_0_TPI_I2S_EN_ADDR, 0x91);
//        WriteByteTPI(PAGE_0_TPI_I2S_EN_ADDR, 0xA2);
//        WriteByteTPI(PAGE_0_TPI_I2S_EN_ADDR, 0xB3);

        // I2S - Stream Header Settings - replace with call to API SetI2S_StreamHeader
        WriteByteTPI(TPI_I2S_CHST_0, 0x00); // 0x21
        WriteByteTPI(TPI_I2S_CHST_1, 0x00);
        WriteByteTPI(TPI_I2S_CHST_2, 0x00);
        WriteByteTPI(TPI_I2S_CHST_3, 0x02);
        WriteByteTPI(TPI_I2S_CHST_4, 0x02);
        // I2S - Input Configuration - replace with call to API ConfigI2SInput
        WriteByteTPI(TPI_I2S_IN_CFG, (0x10 | SCK_SAMPLE_EDGE)); //0x20
#endif

    WriteByteTPI(TPI_AUDIO_SAMPLE_CTRL, TWO_CHANNELS);  // 0x27 = 0x01

	ChannelCount = VideoModeDescription.CTypeChannelCount & THREE_LSBITS;
	CodingType = (VideoModeDescription.CTypeChannelCount >> 4) & FOUR_LSBITS;
	SS = VideoModeDescription.SampleSizeFrequency & TWO_LSBITS;
	Fs = (VideoModeDescription.SampleSizeFrequency >> 2) & THREE_LSBITS;
	SpkConfig = VideoModeDescription.ChannelSpeakerAlloc;

#ifdef HBR_AUDIO
	hbrAudio = VideoModeDescription.HBRAStatus;

//	printf("\nChannelCount = 0x%2X\n", (int)ChannelCount);
//	printf("Coding Type = 0x%2X\n", (int)CodingType);
//	printf("SS = 0x%2X\n", (int)SS);
//	printf("FS = 0x%2X\n", (int)Fs);
//	printf("Speaker = 0x%2X\n", (int)SpkConfig);
//	printf("HBRA Status = 0x%2X\n", (int)hbrAudio);

   	if (hbrAudio)
	{
//		printf("TPI FW --- HBR audio is ON\n");
		AudioConfigSource = (int)(hbrAudio << 2 | (Fs << 3) | (SS << 6));
//		printf("AudioConfigSource 0x%2X\n\n", AudioConfigSource);
		TXHAL_HBR_On(AudioConfigSource);
	}
//	else
//	{
//		printf("TPI FW ---  HBR audio is OFF\n");
//	}

	//SetAudioInfoFrames(TWO_CHANNELS, 0x00, 0x00, 0x00, 0x00);
	SetAudioInfoFrames(ChannelCount, CodingType, SS, Fs, SpkConfig);
#else
	SetAudioInfoFrames(TWO_CHANNELS, 0x00, 0x00, 0x00, 0x00);
#endif

}

#if (AST_HDMITX)
/*
 * sii9136_set_audio_i2s - set audio subsystem in I2S
 *
 * @d: pointer to audio data structure
 *
 */
void sii9136_set_audio_i2s(struct sii9136_audio_data *d)
{
	u8 sf = 0, ss = 0, hbr = 0;
	u8 channel_num = 2;

	/* Select I2S input with Mute enable(bit[4]) */
	WriteByteTPI(PAGE_0_TPI_CONFIG3_ADDR,  AUD_IF_I2S | BIT_4); /* 0x26 */
	WriteByteTPI(PAGE_0_TPI_AUDIO_HANDLING_ADDR, AUD_PASS_ALL); /* 0x25 */

	/* sample size and sample frequency both refer to stream header */
	WriteByteTPI(TPI_AUDIO_SAMPLE_CTRL, 0);

	hbr = (d->audio_type & AST_AUDIO_HBR)?AUDIO_HBR_ENABLE:0;

	/* check sample frequency */
	/* definition in I2S, need to sync with I2S driver
	 *  3:  32 khz
	 *  0:  44.1 khz
	 *  2:  48 khz
	 *  8:  88.2 khz
	 * 10:  96k hz
	 * 12: 176.4 khz
	 * 14: 192 khz
	 */
	switch (d->sample_frequency) {
	case 0:
		sf = AUDIO_SAMPLE_FREQNENCY_44100;
		break;
	case 2:
		sf = AUDIO_SAMPLE_FREQNENCY_48000;
		break;
	case 8:
		sf = AUDIO_SAMPLE_FREQNENCY_88200;
		break;
	case 10:
		sf = AUDIO_SAMPLE_FREQNENCY_96000;
		break;
	case 12:
		sf = AUDIO_SAMPLE_FREQNENCY_176400;
		break;
	case 14:
		sf = AUDIO_SAMPLE_FREQNENCY_192000;
		break;
	default:
		sf = AUDIO_SAMPLE_FREQNENCY_REFER_TO_HEADER;
	}

	switch (d->sample_size) {
	case 16:
		ss = AUDIO_SAMPLE_SIZE_16;
		break;
	case 20:
		ss = AUDIO_SAMPLE_SIZE_20;
		break;
	case 24:
		ss = AUDIO_SAMPLE_SIZE_24;
		break;
	default:
		ss = AUDIO_SAMPLE_SIZE_REFER_TO_HEADER;
	}

	channel_num = d->valid_ch;

	/* HBR format */
	if (hbr) {
		u32 val;
		/* Sii9136 programming guide p33, important note #1
		 * To clear TPI 0x25[5]
		 * So set channel_num to 2 to clear 0x25[5]
		 */
		channel_num = 2;
		/* Sii9136 programming guide p33, important note #2
		 * page 1: 0x1D[5] = 1
		 */
		val = ReadIndexedRegister(INDEXED_PAGE_1, 0x1D);
		val |= (0x1 << 5);
		WriteIndexedRegister(INDEXED_PAGE_1, 0x1D, val);

		/* HBR is 192K, 24 bit */
		ss = AUDIO_SAMPLE_SIZE_24;
		sf = AUDIO_SAMPLE_FREQNENCY_192000;
	}

	WriteByteTPI(TPI_AUDIO_SAMPLE_CTRL, (ss | sf | hbr));

	/*
	 * Channel layout
	 * 0 => 2 channels
	 * 1 => up to 8 channels
	 */
	if (2 < channel_num)
		ReadSetWriteTPI(PAGE_0_TPI_CONFIG3_ADDR, BIT_5);
	else
		ReadClearWriteTPI(PAGE_0_TPI_CONFIG3_ADDR, BIT_5);

	/* I2S - Map channels */
	WriteByteTPI(PAGE_0_TPI_I2S_EN_ADDR, 0x80);
	WriteByteTPI(PAGE_0_TPI_I2S_EN_ADDR, 0x91);
	WriteByteTPI(PAGE_0_TPI_I2S_EN_ADDR, 0xA2);
	WriteByteTPI(PAGE_0_TPI_I2S_EN_ADDR, 0xB3);

	/* I2S - Stream Header Settings */
	WriteByteTPI(TPI_I2S_CHST_0, d->iec60958_channel_status[0]);
	WriteByteTPI(TPI_I2S_CHST_1, d->iec60958_channel_status[1]);
	WriteByteTPI(TPI_I2S_CHST_2, d->iec60958_channel_status[2]);
	WriteByteTPI(TPI_I2S_CHST_3, d->iec60958_channel_status[3]);
	WriteByteTPI(TPI_I2S_CHST_4, d->iec60958_channel_status[4]);

	/* I2S - Input Configuration */
	WriteByteTPI(TPI_I2S_IN_CFG, (0x10 | SCK_SAMPLE_EDGE));	//0x20


	/* Mute disable */
	ReadClearWriteTPI(PAGE_0_TPI_CONFIG3_ADDR, BIT_4);
}
#endif

#if !(AST_HDMITX)
//////////////////////////////////////////////////////////////////////////////
//
// FUNCTION      :  ChangeVideoMode()
//
// PURPOSE       :  Changes the 9022/4 video resolution following a command from
//                  System Control
//
// INPUT PARAMS  :  API_Cmd type structure with command parameters, sent from
//                  the system controller
//
// OUTPUT PARAMS :  None
//
// GLOBALS USED  :  RxCommand
//
// RETURNS       :  Success message if video resolution changed successfully.
//                  Error Code if resolution change failed
//
// NOTE         :   Function calls InitVideo() with the 2nd parameter set to
//                  MODE_CHANGE (==0). That will initialized only the basic
//                  parameters (Pix. Clk; VFreq; H. # of pixels;...). Other
//                  parmeters will not be set by InitVideo(), and will be set
//                  by this function, based on the values passed in the
//                  API_Cmd typed parameter
//
//////////////////////////////////////////////////////////////////////////////
uint8_t ChangeVideoMode()
{
	uint8_t	oldInputColorDepth = VideoModeDescription.InputColorDepth;

  	TPI_TRACE_PRINT((TPI_TRACE_CHANNEL,">>converts to tabel indexChangeVideoMode()\n"));

    HDCP_Off();
    SiiMhlTxDrvTmdsControl( false,HDCPIsAuthenticated );

    DelayMS(T_RES_CHANGE_DELAY);    // allow control InfoFrames to pass through to the sink device.

	//
	//  We have received a Video Mode Setup message.
	//  First thing is to parse out some information to
	//  same in the VideoModeDescription block.
	//
	//	See Video Mode Setup spec in FirmwareCommunicationsPackets.doc
	//	for fields used below to set VideoModeDescription block.
	VideoModeDescription.VIC                    =   g_ScratchPad.CommData.payload.vidModeChange.VIC;
	VideoModeDescription.HDMIVideoFormat        =   g_ScratchPad.CommData.payload.vidModeChange.HDMIVideoFormat;
	VideoModeDescription.AspectRatio            =   g_ScratchPad.CommData.payload.vidModeChange.AspectRatio;
	VideoModeDescription.InputColorSpace        =  (g_ScratchPad.CommData.payload.vidModeChange.InputColorSpaceDepthRangeExpansion     & VMD_COLOR_SPACE_MAP);
	VideoModeDescription.InputColorDepth        =   g_ScratchPad.CommData.payload.vidModeChange.InputColorSpaceDepthRangeExpansion     >> 6;
	VideoModeDescription.InputRangeExpansion    =  (g_ScratchPad.CommData.payload.vidModeChange.InputColorSpaceDepthRangeExpansion     & VMD_VIDEO_RANGE_MAP) >> 2;
	VideoModeDescription.OutputColorSpace       =  (g_ScratchPad.CommData.payload.vidModeChange.OutputColorSpaceDepthRangeCompression  & VMD_COLOR_SPACE_MAP);
	VideoModeDescription.OutputColorDepth       =   g_ScratchPad.CommData.payload.vidModeChange.OutputColorSpaceDepthRangeCompression  >> 6;
	VideoModeDescription.OutputRangeCompression =  (g_ScratchPad.CommData.payload.vidModeChange.OutputColorSpaceDepthRangeCompression  & VMD_VIDEO_RANGE_MAP) >> 2;
	VideoModeDescription.HDCPAuthenticated      =   g_ScratchPad.CommData.payload.vidModeChange.HDCPAuthenticated;
	VideoModeDescription.ThreeDStructure        =  (g_ScratchPad.CommData.payload.vidModeChange.ThreeDStructure_ExtData                & VMD_3D_FRAMEPACKING_MAP) ;
	VideoModeDescription.ThreeDExtData          = ((g_ScratchPad.CommData.payload.vidModeChange.ThreeDStructure_ExtData                & VMD_3D_EXT_DATA_MAP) >> 4);

#ifndef NEVER	// Replace with test of JP18 %%%
	{
		int	i;
		printf("\nRaw Video Mode Set Packet:\n  ");
		for (i=0; i<15 ; i++)
		{
			printf("0x%x ", (int)g_ScratchPad.printfData[i]);
		}
		printf("\n");
	}
	printf("Change Video Mode:\n");
	VideoModeReport(&VideoModeDescription);
#endif

	// IF we are changing the Input Color Depth, and IF needed on this platform,
	// set the receiver's output color depth to the input color depth we expect.
	if (oldInputColorDepth != VideoModeDescription.InputColorDepth)
	{
		TXHAL_ColorDepthConfig();
	}

	if (VideoModeDescription.OutputColorDepth == VMD_COLOR_DEPTH_8BIT)
	{
		// Yes it is, so force 16bpps first!
		VideoModeDescription.OutputColorDepth = VMD_COLOR_DEPTH_16BIT;
		// Initialize mode with IDCLK set the multiplier
        InitVideo(((g_ScratchPad.CommData.payload.vidModeChange.PRD >> 6) & TWO_LSBITS));
		// Now put it back to 8bit and go do the expected InitVideo() call
		VideoModeDescription.OutputColorDepth = VMD_COLOR_DEPTH_8BIT;
	}

    InitVideo(((g_ScratchPad.CommData.payload.vidModeChange.PRD >> 6) & TWO_LSBITS));  // Will set values based on VModesTable[Arg[0])

#ifdef DEEP_COLOR
    if (((g_ScratchPad.CommData.payload.vidModeChange.InputColorSpaceDepthRangeExpansion & BITS_7_6) >> 6) == DC_48)         // 16 bit Deep Color. Forcer 0x08[5] to 0 for half pixel mode
    {
        g_ScratchPad.CommData.payload.vidModeChange.PRD &= ~BIT_5;
    }
#endif

    WriteByteTPI(PAGE_0_TPI_INPUT_ADDR, g_ScratchPad.CommData.payload.vidModeChange.InputColorSpaceDepthRangeExpansion);     // Write input formats to register 0x09

#ifdef DEEP_COLOR
	if (((g_ScratchPad.CommData.payload.vidModeChange.InputColorSpaceDepthRangeExpansion & BITS_7_6) >> 6) >= DC_48)         // 16 bit Deep Color. Forcer 0x08[5] to 0 for half pixel mode
    {
        ReadModifyWriteTPI(PAGE_0_TPI_SW_RST_EN_ADDR, TPI_SW_RST_EN_REG_DC_PKT_EN_MASK, TPI_SW_RST_EN_REG_DC_PKT_EN_ENABLE);
    }
	else
    {
		ReadModifyWriteTPI(PAGE_0_TPI_SW_RST_EN_ADDR, TPI_SW_RST_EN_REG_DC_PKT_EN_MASK, TPI_SW_RST_EN_REG_DC_PKT_EN_DISABLE);
    }
#endif

 	if (IsHDMI_Sink())                                                  // sink suppurts HDMI
    {
    	ReadSetWriteTPI(PAGE_0_TPI_SC_ADDR, OUTPT_MODE_HDMI);  // Set 0x1A[0] to HDMI
    }

	WriteByteTPI(PAGE_0_TPI_PRD_ADDR, g_ScratchPad.CommData.payload.vidModeChange.PRD);      // 0x08

    WriteByteTPI(PAGE_0_TPI_OUTPUT_ADDR, g_ScratchPad.CommData.payload.vidModeChange.OutputColorSpaceDepthRangeCompression );   // Write output color depth to registers 0x0A

	if (IsHDMI_Sink())
    {
		SetAVI_InfoFrames();                       // InfoFrames - only if output mode is HDMI

		// Setup the Vendor Specific Info Frame
		Set_VSIF();
	}

//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// PATCH!!!PATCH!!!PATCH!!!PATCH!!!PATCH!!!PATCH!!!PATCH!!!PATCH!!!PATCH!!!PATCH!!!PATCH!!!PATCH!!!
//

// THIS PATCH IS NEEDED BECAUSE SETTING UP AVI InfoFrames CLEARS 0x63 and 0x60[5]

 	WriteByteTPI(PAGE_0_TPI_SYNC_GEN_ADDR, g_ScratchPad.CommData.payload.vidModeChange.SYNC_GEN);   // Set 0x60 according to Command.Arg[4]
                                               // THIS PATCH IS NEEDED BECAUSE SETTING UP AVI InfoFrames CLEARS 0x63 and 0x60[5]

    if(g_ScratchPad.CommData.payload.vidModeChange.SYNC_GEN & MSBIT)                         // THIS PATCH IS NEEDED BECAUSE SETTING UP AVI InfoFrames CLEARS 0x63
	{
    	WriteByteTPI(PAGE_0_TPI_HBIT_2HSYNC2_ADDR, EmbeddedSynPATCH); // (set in function SetEmbeddedSync())TO 0.
	}
//
// PATCH!!!PATCH!!!PATCH!!!PATCH!!!PATCH!!!PATCH!!!PATCH!!!PATCH!!!PATCH!!!PATCH!!!PATCH!!!PATCH!!!
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!


 	// Command.Arg[7][3:2] -> 0x0E[7:6] Colorimetry
    ReadModifyWriteTPI(TPI_AVI_BYTE_2, BITS_7_6, g_ScratchPad.CommData.payload.vidModeChange.Colorimetry << 4);

    // Command.Arg[7][6:4] -> 0x0F[6:4] Extended Colorimetry
    if ((g_ScratchPad.CommData.payload.vidModeChange.Colorimetry & BITS_3_2) == SET_EX_COLORIMETRY)
    {
		ReadModifyWriteTPI(TPI_AVI_BYTE_3, BITS_6_5_4, g_ScratchPad.CommData.payload.vidModeChange.Colorimetry);
    }

    //==========================================================

 	// YC Input Mode Select - Command.Arg[6] - offset 0x0B
    WriteByteTPI(TPI_YC_Input_Mode, g_ScratchPad.CommData.payload.vidModeChange.YC_Input_Mode);

	WriteByteTPI(PAGE_0_TPI_SYNC_GEN_ADDR, g_ScratchPad.CommData.payload.vidModeChange.SYNC_GEN);

#ifdef MHL_CONNECTION_STATE_MACHINE
	SiiMhlTxTmdsEnable(false);
#else
	SiiMhlTxTmdsEnable(true);
#endif


    return VIDEO_MODE_SET_OK;
}
#endif //#if !(AST_HDMITX)
#if !(AST_HDMITX)
//////////////////////////////////////////////////////////////////////////////
//
// FUNCTION      :  MapI2S()
//
// PURPOSE       :  Changes the 9022/4 I2S channel mapping as defined by a
//                  command sent from the System Controller
//
// INPUT PARAMS  :  API_Cmd type structure with command parameters
//
// OUTPUT PARAMS :  None
//
// GLOBALS USED  :  None
//
// RETURNS       :  Success message if channel mapping successful. Error
//                  Code if failed
//
//////////////////////////////////////////////////////////////////////////////
uint8_t MapI2S()
{
    uint8_t B_Data;
	int i;
	// uint8_t temp;

        TPI_TRACE_PRINT((TPI_TRACE_CHANNEL,">>MapI2S()\n"));

    B_Data = ReadByteTPI(PAGE_0_TPI_CONFIG3_ADDR);

    if ((B_Data & TWO_MSBITS) != AUD_IF_I2S)    // 0x26 not set to I2S interface
    {
        return I2S_NOT_SET;
    }

//      WriteByteTPI(PAGE_0_TPI_I2S_EN_ADDR, Command.Arg[0]);

//      printf("I2sMapping:\n");

		for (i = 0; i < I2S_MAPPING_CMD_LEN; i++)
        {
			WriteByteTPI(PAGE_0_TPI_I2S_EN_ADDR, g_ScratchPad.CommData.payload.bytes[i]);

//			temp = ReadByteTPI(PAGE_0_TPI_I2S_EN_ADDR);
//         	printf("    FIFO#%d = 0x%2X\n", i, (int)temp);

			if ((g_ScratchPad.CommData.payload.bytes[i+1] & BITS_1_0) == 0)
            {
				return DE_CANNOT_BE_SET_WITH_EMBEDDED_SYNC;
            }
        }
    return I2S_MAPPING_SUCCESSFUL;
}
#endif //#if !(AST_HDMITX)
#if !(AST_HDMITX)
//////////////////////////////////////////////////////////////////////////////
//
// FUNCTION      :  ConfigI2SInput()
//
// PURPOSE       :  Sets the 9022/4 I2S channel bit direction, justification
//                  and polarity as defined by acommand sent from the System
//                  Controller
//
// INPUT PARAMS  :  API_Cmd type structure with command parameters
//
// OUTPUT PARAMS :  None
//
// GLOBALS USED  :  None
//
// RETURNS       :  Success message if video I2S channels configuredsuccessfully.
//                  Error Code if setting failed
//
//////////////////////////////////////////////////////////////////////////////
uint8_t ConfigI2SInput()
{
    uint8_t B_Data;

        TPI_TRACE_PRINT((TPI_TRACE_CHANNEL,">>ConfigI2SInput()\n"));

    B_Data = ReadByteTPI(PAGE_0_TPI_CONFIG3_ADDR);

    if ((B_Data & TWO_MSBITS) != AUD_IF_I2S)    // 0x26 not set to I2S interface
    {
        return I2S_NOT_SET;
    }

    WriteByteTPI(TPI_I2S_IN_CFG, g_ScratchPad.CommData.payload.configI2SInput.I2S_InputConfig);

    return I2S_INPUT_CONFIG_SUCCESSFUL;
}
#endif //#if !(AST_HDMITX)
#if !(AST_HDMITX)
//////////////////////////////////////////////////////////////////////////////
//
// FUNCTION      :  SetI2S_StreamHeader()
//
// PURPOSE       :  Sets the 9022/4 I2S Channel Status bytes, as defined by
//                  a command sent from the System Controller
//
// INPUT PARAMS  :  API_Cmd type structure with command parameters
//
// OUTPUT PARAMS :  None
//
// GLOBALS USED  :  None
//
// RETURNS       :  Success message if stream header set successfully. Error
//                  Code if failed
//
//////////////////////////////////////////////////////////////////////////////
uint8_t SetI2S_StreamHeader()
{
    uint8_t B_Data;
    int i;

        TPI_TRACE_PRINT((TPI_TRACE_CHANNEL,">>SetI2S_StreamHeader()\n"));

    B_Data = ReadByteTPI(PAGE_0_TPI_CONFIG3_ADDR);

    if ((B_Data & TWO_MSBITS) != AUD_IF_I2S)    // 0x26 not set to I2S interface
    {
        return I2S_NOT_SET;
    }

    for (i = 0; i < g_ScratchPad.CommData.header.PayloadSizeInBytes; i++)
    {
        WriteByteTPI(TPI_I2S_CHST_0 + i, g_ScratchPad.CommData.payload.bytes[i]);
    }

    return I2S_HEADER_SET_SUCCESSFUL;
}
#endif //#if !(AST_HDMITX)

//////////////////////////////////////////////////////////////////////////////
//
// FUNCTION     :   SetGBD_InfoFrame(()
//
// PURPOSE      :   Sets and sends the the 9022A/4A GBD InfoFrames according
//                  to data sent from the System Controller
//
// INPUT PARAMS :   None
//
// OUTPUT PARAMS:   None
//
// GLOBALS USED :   None
//
// RETURNS      :   Success message if GBD packet set successfully. Error
//                  Code if failed
//
// NOTE         :   Currently this function is a place holder. It always
//                  returns a Success message
//
//////////////////////////////////////////////////////////////////////////////
uint8_t SetGBD_InfoFrame()
{
        uint8_t CheckSum;

        TPI_TRACE_PRINT((TPI_TRACE_CHANNEL,">>SetGBD_InfoFrame()\n"));

        // Set MPEG InfoFrame Header to GBD InfoFrame Header values:
        WriteByteTPI(MISC_INFO_FRAMES_CTRL, DISABLE_MPEG);             // 0xBF = Use MPEG      InfoFrame for GBD - 0x03
        WriteByteTPI(MISC_INFO_FRAMES_TYPE, TYPE_GBD_INFOFRAMES);                               // 0xC0 = 0x0A
        WriteByteTPI(MISC_INFO_FRAMES_VER, NEXT_FIELD | GBD_PROFILE | AFFECTED_GAMUT_SEQ_NUM);  // 0x0C1 = 0x81
        WriteByteTPI(MISC_INFO_FRAMES_LEN, ONLY_PACKET | CURRENT_GAMUT_SEQ_NUM);                // 0x0C2 = 0x31

        CheckSum = TYPE_GBD_INFOFRAMES +
                           NEXT_FIELD +
                           GBD_PROFILE +
                           AFFECTED_GAMUT_SEQ_NUM +
                           ONLY_PACKET +
                           CURRENT_GAMUT_SEQ_NUM;

        CheckSum = 0x100 - CheckSum;

        WriteByteTPI(MISC_INFO_FRAMES_CTRL, EN_AND_RPT_MPEG);  // Enable and Repeat MPEG InfoFrames
        WriteByteTPI(MISC_INFO_FRAMES_CHKSUM, CheckSum);                        // 0X00 - Send header only

        return GBD_SET_SUCCESSFULLY;
}

//////////////////////////////////////////////////////////////////////////////
//
// FUNCTION      :  SetAVI_InfoFrames()
//
// PURPOSE       :  Load AVI InfoFrame data into registers and send to sink
//
// INPUT PARAMS  :  An API_Cmd parameter that holds the data to be sent
//                  in the InfoFrames
//
// OUTPUT PARAMS :  None
//
// GLOBALS USED  :  None
//
// RETURNS       :  TRUE
//
// Note:          : Infoframe contents are from spec CEA-861-D
//
//////////////////////////////////////////////////////////////////////////////
uint8_t SetAVI_InfoFrames(AVIInfoFrameData_t *aviInfoFrame)
{
    uint8_t B_Data[SIZE_AVI_INFOFRAME];
    uint8_t i;
    uint8_t TmpVal;

    TPI_TRACE_PRINT((TPI_TRACE_CHANNEL,">>SetAVI_InfoFrames()\n"));
    //
    // Disable transmission of AVI InfoFrames during re-configuration
    WriteByteTPI(MISC_INFO_FRAMES_CTRL, DISABLE_AVI);         //  Disbale MPEG/Vendor Specific InfoFrames

    for (i = 0; i < SIZE_AVI_INFOFRAME; i++)
    {
        B_Data[i] = 0;
    }

    //////////////////////////////////////////////////////////////////////////////////////////
    //  AVI InfoFrame is set by the FW, so we have some work to do to put it all in the proper
    //  format for the TPI 0x0C through 0x19 registers.:sed
    //////////////////////////////////////////////////////////////////////////////////////////

    // AVI InfoFrame DByte1 contains Output ColorSpace
    if ((aviInfoFrame->DataByte1_RV_Y1_Y0_A0_B1_B0_S1_S0 & TWO_LSBITS) == 1)
    {
        TmpVal = 2;     // 2 = YCbCr 4:4:4 in inforframe Byte 1 bits 6:5
    }
    else if ((aviInfoFrame->DataByte1_RV_Y1_Y0_A0_B1_B0_S1_S0 & TWO_LSBITS) == 2)
    {
        TmpVal = 1;     // 1 = YCbCr 4:2:2
    }
    else
    {
        TmpVal = 0;     // 0 = RGB
    }
    B_Data[1] = (TmpVal << 5) & BITS_OUT_FORMAT;
	B_Data[1] |= ACT_FORMAT_INFO_VALID;   	// Set Active Format Information Valid bit in A0 - AVI Infoframe DByte 1

    // Extended colorimetry - xvYCC
    if (((aviInfoFrame->DataByte5_YQ1_YQ0_CN1_CN0_PR3_PR2_PR1_PR0 >> 2) & TWO_LSBITS) == 3)
    {
       B_Data[2] = 0xC0;                                   // Extended colorimetry info (B_Data[3] valid (CEA-861D, Table 11)

       if (((aviInfoFrame->DataByte5_YQ1_YQ0_CN1_CN0_PR3_PR2_PR1_PR0 >> 4) & THREE_LSBITS) == 0)            // xvYCC601
       {
           B_Data[3] &= ~BITS_6_5_4;   	// EC0..EC2
       }
       else if (((aviInfoFrame->DataByte5_YQ1_YQ0_CN1_CN0_PR3_PR2_PR1_PR0 >> 4) & THREE_LSBITS) == 1)
       {
           // xvYCC709
           B_Data[3] = (B_Data[3] & ~BITS_6_5_4) | BIT_4;
       }
	}
  	else if (((aviInfoFrame->DataByte5_YQ1_YQ0_CN1_CN0_PR3_PR2_PR1_PR0 >> 2) & TWO_LSBITS) == 2)
  	{
   		// BT.709
     	B_Data[2] = BT_709;
   	}
  	else if (((aviInfoFrame->DataByte5_YQ1_YQ0_CN1_CN0_PR3_PR2_PR1_PR0 >> 2) & TWO_LSBITS) == 1)
   	{
       	// BT.601
      	B_Data[2] = BT_601;
   	}
  	else
 	{
     	// Carries no data
     	B_Data[2] &= ~BITS_7_6;     // colorimetry = 0
      	B_Data[3] &= ~BITS_6_5_4;   // Extended colorimetry = 0
    }

    //////////////////////////////////////////////////////////////////////////////////////////
    // Find out what the line is in the Video Mode Table for this Video Setup.  Also set
    // Aspect Ratio and Pixel Replication now because we sometimes need to know the
    // table entry before we can set them.
    //////////////////////////////////////////////////////////////////////////////////////////
    VModeTblIndex = ConvertVIC_To_VM_Index();
    B_Data[4] = VideoModeDescription.VIC;

    if (VideoModeDescription.AspectRatio == VMD_ASPECT_RATIO_16x9)
    {
        B_Data[2] |= _16_To_9;                          // AVI Byte2: M1M0
        // If the Video Mode table says this mode can be 4x3 OR 16x9, and we are pointing to the
        // table entry that is 4x3, then we bump to the next Video Table entry which will be for
        // 16x9.
        if ((VModesTable[VModeTblIndex].AspectRatio == R_4or16)
            && (AspectRatioTable[VideoModeDescription.VIC - 1] == R_4))
        {
            VideoModeDescription.VIC++;
            B_Data[4]++;
        }

    }
    else
    {
        B_Data[2] |= _4_To_3;      // AVI Byte2: M1M0
    }

	B_Data[2] |= SAME_AS_AR;  	   // AVI Byte2: R0..R3

    // Use table rep value
    B_Data[5] = VModesTable[VModeTblIndex].PixRep;      // AVI Byte5: Pixel Replication - PR3..PR0

    //////////////////////////////////////////////////////////////////////////////////////////
    // Calculate AVI InfoFrame ChecKsum
    //////////////////////////////////////////////////////////////////////////////////////////
    B_Data[0] = 0x82 + 0x02 + 0x0D;
    for (i = 1; i < SIZE_AVI_INFOFRAME; i++)
    {
        B_Data[0] += B_Data[i];
    }
    B_Data[0] = 0x100 - B_Data[0];

    //////////////////////////////////////////////////////////////////////////////////////////
    // Write the Inforframe data to the TPI Infoframe registers.  This automatically
    // Enables InfoFrame transmission and repeat by over-writing Register 0x19.
    //////////////////////////////////////////////////////////////////////////////////////////
    WriteBlockTPI(TPI_AVI_BYTE_0, SIZE_AVI_INFOFRAME, B_Data);

#ifdef DEV_EMBEDDED
    EnableEmbeddedSync();
#endif

    return TRUE;
}

#if (AST_HDMITX)
/*
 * sii9136_set_avi_info_frame - set avi info frame
 *
 * @content: pointer to the content of AVI info frame packet
 *
 */
void sii9136_set_avi_info_frame(u8 *content)
{
	u8 B_Data[SIZE_AVI_INFOFRAME];
	int i;

	P_TRACE("HDMI AVI info\n");
	WriteByteTPI(MISC_INFO_FRAMES_CTRL, DISABLE_AVI); /* Disbale MPEG/Vendor Specific InfoFrames */

	for (i = 0; i < SIZE_AVI_INFOFRAME; i++)
		B_Data[i] = content[i];

	/* Calculate AVI InfoFrame ChecKsum */
	B_Data[0] = 0x82 + 0x02 + 0x0D;
	for (i = 1; i < SIZE_AVI_INFOFRAME; i++)
		B_Data[0] += B_Data[i];

	B_Data[0] = 0x100 - B_Data[0];

#if 1
	WriteBlockTPI(TPI_AVI_BYTE_0, SIZE_AVI_INFOFRAME - 1 , B_Data);
	WriteByteTPI(PAGE_0_TPI_END_RIGHT_BAR_MSB_ADDR, B_Data[SIZE_AVI_INFOFRAME - 1]); /* Set last byte of TPI AVI InfoFrame for TPI AVI I/O Format to take effect */
#else
	WriteBlockTPI(TPI_AVI_BYTE_0, SIZE_AVI_INFOFRAME, B_Data);
#endif
}
#endif /* #if (AST_HDMITX) */

//////////////////////////////////////////////////////////////////////////////
//
// FUNCTION      :  SetAudioInfoFrames()
//
// PURPOSE       :  Load Audio InfoFrame data into registers and send to sink
//
// INPUT PARAMS  :  (1) Channel count (2) speaker configuration per CEA-861D
//                  Tables 19, 20 (3) Coding type: 0x09 for DSD Audio. 0 (refer
//                                      to stream header) for all the rest (4) Sample Frequency. Non
//                                      zero for HBR only (5) Audio Sample Length. Non zero for HBR
//                                      only.
//
// OUTPUT PARAMS :  None
//
// GLOBALS USED  :  None
//
// RETURNS       :  TRUE
//
//////////////////////////////////////////////////////////////////////////////
uint8_t SetAudioInfoFrames(uint8_t ChannelCount, uint8_t CodingType, uint8_t SS, uint8_t Fs, uint8_t SpeakerConfig)
{
    uint8_t B_Data[SIZE_AUDIO_INFOFRAME];  // 15
    uint8_t i;

	TPI_TRACE_PRINT((TPI_TRACE_CHANNEL,">>SetAudioInfoFrames()\n"));

    for (i = 0; i < SIZE_AUDIO_INFOFRAME; i++)
        B_Data[i] = 0;

    WriteByteTPI(MISC_INFO_FRAMES_CTRL, DISABLE_AUDIO);         //  Disbale MPEG/Vendor Specific InfoFrames

    B_Data[0] = TYPE_AUDIO_INFOFRAMES;      // 0x84
    B_Data[1] = MISC_INFOFRAMES_VERSION;    // 0x01
    B_Data[2] = AUDIO_INFOFRAMES_LENGTH;    // 0x0A
    B_Data[3] = TYPE_AUDIO_INFOFRAMES+ 		// Calculate checksum - 0x84 + 0x01 + 0x0A
				MISC_INFOFRAMES_VERSION+
				AUDIO_INFOFRAMES_LENGTH;

    B_Data[4] = ChannelCount;               // 0 for "Refer to Stream Header" or for 2 Channels. 0x07 for 8 Channels
	B_Data[4] |= (CodingType << 4);        	// 0xC7[7:4] == 0b1001 for DSD Audio

	B_Data[5] = (Fs >> 1) | (SS >> 6);

	B_Data[7] = SpeakerConfig;

    for (i = 4; i < SIZE_AUDIO_INFOFRAME; i++)
        B_Data[3] += B_Data[i];

    B_Data[3] = 0x100 - B_Data[3];
	g_audio_Checksum = B_Data[3];	// Audio checksum for global use

	WriteByteTPI(MISC_INFO_FRAMES_CTRL, EN_AND_RPT_AUDIO);         // Re-enable Audio InfoFrame transmission and repeat

    WriteBlockTPI(MISC_INFO_FRAMES_TYPE, SIZE_AUDIO_INFOFRAME, B_Data);

	#ifdef DEV_EMBEDDED
    EnableEmbeddedSync();
	#endif

    return TRUE;
}

#if (AST_HDMITX)
/*
 * sii9136_set_audio_info_frame - set audio info frame
 *
 * @content: pointer to the content of audio info frame packet
 *
 */
void sii9136_set_audio_info_frame(u8 *content)
{
	u8 B_Data[SIZE_AUDIO_INFOFRAME];
	int i;

	for (i = 0; i < SIZE_AUDIO_INFOFRAME; i++)
		B_Data[i] = 0;

	WriteByteTPI(MISC_INFO_FRAMES_CTRL, DISABLE_AUDIO);	// Disbale MPEG/Vendor Specific InfoFrames

	B_Data[0] = TYPE_AUDIO_INFOFRAMES;	// 0x84
	B_Data[1] = MISC_INFOFRAMES_VERSION;	// 0x01
	B_Data[2] = AUDIO_INFOFRAMES_LENGTH;	// 0x0A
	B_Data[3] = TYPE_AUDIO_INFOFRAMES +	// Calculate checksum - 0x84 + 0x01 + 0x0A
				MISC_INFOFRAMES_VERSION+
				AUDIO_INFOFRAMES_LENGTH;

	B_Data[4] = content[0];
	B_Data[5] = content[1];
	B_Data[7] = content[3];
	B_Data[8] = content[4];

	/* Checksum */
	for (i = 4; i < SIZE_AUDIO_INFOFRAME; i++)
		B_Data[3] += B_Data[i];

	B_Data[3] = 0x100 - B_Data[3];

	WriteByteTPI(MISC_INFO_FRAMES_CTRL, EN_AND_RPT_AUDIO);	// Re-enable Audio InfoFrame transmission and repeat

	WriteBlockTPI(MISC_INFO_FRAMES_TYPE, SIZE_AUDIO_INFOFRAME, B_Data);
}
#endif

//////////////////////////////////////////////////////////////////////////////
//
// FUNCTION      :  Set_VSIF()
//
// PURPOSE       :  Construct Vendor Specific InfoFrame for3D support.
//
// INPUT PARAMS  :
//
// OUTPUT PARAMS :  None
//
// GLOBALS USED  :  VideoModeDescription
//
// RETURNS       :  void
//
//////////////////////////////////////////////////////////////////////////////

void Set_VSIF(void)
{
    uint8_t i;
    uint8_t Data[INFOFRAMES_BUFF_LENGTH];

	// Disable transmission of VSIF during re-configuration
    WriteByteTPI(MISC_INFO_FRAMES_CTRL, DISABLE_HDMI_VSIF);

	if (VideoModeDescription.HDMIVideoFormat == 0)	// if no HDMI video format is presented in the packet,
	{												// clear the VSIF buffer.
		for (i = 0; i < INFOFRAMES_BUFF_LENGTH; i++)
		{
			Data[i] = 0;
		}
	}
	else
	{
		for (i = 0; i < INFOFRAMES_BUFF_LENGTH; i++)
		{
			Data[i] = 0;
		}

		// Header Bytes
		Data[0] = VSIF_TYPE;            // HB0 Packet Type 0x81
		Data[1] = VSIF_VERSION; 		// HB1 Version = 0x01

		// PB1 - PB3 contain the 24bit IEEE Registration Identifier
    	Data[4] = 0x03;                     // HDMI Signature LS Byte
    	Data[5] = 0x0C;                     // HDMI Signature middle byte
    	Data[6] = 0x00;                     // HDMI Signature MS Byte

		// PB4 - HDMI_Video_Format into bits 7:5
    	Data[7] = VideoModeDescription.HDMIVideoFormat << 5;

		// PB5 - Depending on the video format, this byte will contain either the HDMI_VIC
		// code in buts 7:0, OR the 3D_Structure in bits 7:4.
		switch(VideoModeDescription.HDMIVideoFormat)
		{
			case VMD_HDMIFORMAT_HDMI_VIC:
				// This is a 2x4K mode, set the HDMI_VIC in buts 7:0.  Values
				// are from HDMI 1.4 Spec, 8.2.3.1 (Table 8-13).
				Data[8] = VideoModeDescription.VIC;
				Data[9] = 0;
				break;

			case VMD_HDMIFORMAT_3D:
				// This is a 3D mode, set the 3D_Structure in buts 7:4
				// Bits 3:0 are reseved so set to 0.  Values are from HDMI 1.4
				// Spec, Appendix H (Table H-2).
				Data[8] = VideoModeDescription.ThreeDStructure << 4;
				// Add the Extended data field when the 3D format is Side-by-Side(Half).
				// See Spec Table H-3 for details.
				if ((Data[8] >> 4) == VMD_3D_SIDEBYSIDEHALF)
				{
					Data[2] = VSIF_LEN;
					Data[9] = VideoModeDescription.ThreeDExtData << 4;;
				}
				else
				{
			   		Data[2] = VSIF_LEN-1;
				}
				break;

			case VMD_HDMIFORMAT_CEA_VIC:
			default:
				Data[8] = 0;
				Data[9] = 0;
				break;
		}

		// Packet Bytes
    	Data[3] = 	VSIF_TYPE+                // PB0 partial checksum
    				VSIF_VERSION+
              		Data[2];

		// Complete the checksum
    	for (i = 4; i < INFOFRAMES_BUFF_LENGTH; i++)
		{
        	Data[3] += Data[i];
		}

		// Data[3] %= 0x100;
    	Data[3] = 0x100 - Data[3];            // Final checksum
	}

	WriteByteTPI(MISC_INFO_FRAMES_CTRL, EN_AND_RPT_HDMI_VSIF);         		// Enable and Repeat Vendor Specific InfoFrames

    WriteBlockTPI(MISC_INFO_FRAMES_TYPE, INFOFRAMES_BUFF_LENGTH, Data);   	// Write data to VSIF registers and start transmission
#ifdef NEVER	// Replace with test of JP18 %%%
	{
		int	i;
		printf("VSIF settings:\n");
		for (i=0; i<SIZE_AUDIO_INFOFRAME ; i++)
		{
			printf("0x%x ", (int)Data[i]);
		}
		printf("\n");
	}
#endif

	WriteByteTPI(PAGE_0_TPI_INFO_B30_ADDR, 0x00);		// Set last byte of VSIF for data to be sent to the sink.
}

#if (AST_HDMITX)
/*
 * sii9136_set_vendor_specific_info_frame - set vendor specific info frame
 *
 * @content: pointer to the header of vendor specific info frame packet
 *
 */
void sii9136_set_vendor_specific_info_frame(u8 *frame)
{
	uint8_t i;
	uint8_t Data[INFOFRAMES_BUFF_LENGTH];
	P_TRACE("HDMI VS info\n");

	// Disable transmission of VSIF during re-configuration
	WriteByteTPI(MISC_INFO_FRAMES_CTRL, DISABLE_HDMI_VSIF);

	for (i = 0; i < INFOFRAMES_BUFF_LENGTH; i++) {
		Data[i] = frame[i];
	}

	WriteByteTPI(MISC_INFO_FRAMES_CTRL, EN_AND_RPT_HDMI_VSIF);		// Enable and Repeat Vendor Specific InfoFrames

	WriteBlockTPI(MISC_INFO_FRAMES_TYPE, INFOFRAMES_BUFF_LENGTH, Data);	// Write data to VSIF registers and start transmission
	WriteByteTPI(PAGE_0_TPI_INFO_B30_ADDR, 0x00);				// Set last byte of VSIF for data to be sent to the sink.
}

/*
 * sii9136_set_hdr_info_frame - set HDR info frame
 *
 * @content: pointer to the header of HDR info frame packet
 *
 */
void sii9136_set_hdr_info_frame(u8 *frame)
{
	int i;
	u8 Data[SIZE_HDR_INFOFRAME];

	P_TRACE("HDMI HDR info\n");

	for (i = 0; i < SIZE_HDR_INFOFRAME; i++)
		Data[i] = frame[i];

	/* Calculate InfoFrame ChecKsum */
	Data[3] = Data[0] + Data[1] + Data[2];

	for (i = 4; i < SIZE_HDR_INFOFRAME; i++)
		Data[3] += Data[i];

	Data[3] = 0x100 - Data[3];

	/* we use category 3 for HDR InfoFrmae delevery */
	WriteByteTPI(MISC_INFO_FRAMES_CTRL, DISABLE_MPEG);
	WriteByteTPI(MISC_INFO_FRAMES_CTRL, EN_AND_RPT_MPEG);

	WriteBlockTPI(MISC_INFO_FRAMES_TYPE, SIZE_HDR_INFOFRAME, Data);
	WriteByteTPI(PAGE_0_TPI_INFO_B30_ADDR, 0x00); /* Set last byte of VSIF for data to be sent to the sink */
}

void sii9136_disable_avi_if(void)
{
	WriteByteTPI(MISC_INFO_FRAMES_CTRL, DISABLE_AVI);
}

void sii9136_disable_vendor_specific_if(void)
{
	WriteByteTPI(MISC_INFO_FRAMES_CTRL, DISABLE_HDMI_VSIF);
};

void sii9136_disable_hdr_if(void)
{
	/* we use category 3 for HDR InfoFrmae delevery */
	WriteByteTPI(MISC_INFO_FRAMES_CTRL, DISABLE_MPEG);
};

void sii9136_disable_audio_if(void)
{
	WriteByteTPI(MISC_INFO_FRAMES_CTRL, DISABLE_AUDIO);
}

#endif

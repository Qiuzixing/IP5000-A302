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
 * @file  EDID.c
 *
 * @brief Implementation of the Extended Display Identification Data (EDID).
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
#include "videomodedesc.h"
//#include "AMF_Lib.h"
#include "serialpacket.h"
//#include "i2c_master_sw.h"
#include "TPI_Regs.h"
#include "TPI_Access.h"
#include "edid.h"
#include "tpidebug.h"
#include "si_apiConfigure.h"
#include "Externals.h"
#include "tpidebug.h"


#if (IS_CEC == 1) //(
extern	uint16_t SI_CecGetDevicePA(void);
#endif //)

#if (IS_CDC) //(
extern	bool_t CpCdcInit(void);
#endif //)

// Constants
//==========

#define EDID_BLOCK_0_OFFSET 0x00
#define EDID_BLOCK_1_OFFSET 0x80

#define EDID_BLOCK_SIZE      128
#define EDID_HDR_NO_OF_FF   0x06
#define NUM_OF_EXTEN_ADDR   0x7E

#define EDID_TAG_ADDR       0x00
#define EDID_REV_ADDR       0x01
#define EDID_TAG_IDX        0x02
#define LONG_DESCR_PTR_IDX  0x02
#define MISC_SUPPORT_IDX    0x03

#define ESTABLISHED_TIMING_INDEX        35      // Offset of Established Timing in EDID block
#define NUM_OF_STANDARD_TIMINGS          8
#define STANDARD_TIMING_OFFSET          38
#define LONG_DESCR_LEN                  18
#define NUM_OF_DETAILED_DESCRIPTORS      4

#define DETAILED_TIMING_OFFSET        0x36

// Offsets within a Long Descriptors Block
//========================================
#define PIX_CLK_OFFSET                   0
#define H_ACTIVE_OFFSET                  2
#define H_BLANKING_OFFSET                3
#define V_ACTIVE_OFFSET                  5
#define V_BLANKING_OFFSET                6
#define H_SYNC_OFFSET                    8
#define H_SYNC_PW_OFFSET                 9
#define V_SYNC_OFFSET                   10
#define V_SYNC_PW_OFFSET                10
#define H_IMAGE_SIZE_OFFSET             12
#define V_IMAGE_SIZE_OFFSET             13
#define H_BORDER_OFFSET                 15
#define V_BORDER_OFFSET                 16
#define FLAGS_OFFSET                    17

#define AR16_10                          0
#define AR4_3                            1
#define AR5_4                            2
#define AR16_9                           3

// Data Block Tag Codes
//=====================
#define AUDIO_D_BLOCK       0x01
#define VIDEO_D_BLOCK       0x02
#define VENDOR_SPEC_D_BLOCK 0x03
#define SPKR_ALLOC_D_BLOCK  0x04
#define USE_EXTENDED_TAG    0x07

// Extended Data Block Tag Codes
//==============================
#define COLORIMETRY_D_BLOCK 0x05

#define HDMI_SIGNATURE_LEN  0x03

#define CEC_PHYS_ADDR_LEN   0x02
#define EDID_EXTENSION_TAG  0x02
#define EDID_REV_THREE      0x03
#define EDID_DATA_START     0x04

#define EDID_BLOCK_0        0x00
#define EDID_BLOCK_2_3      0x01

#define VIDEO_CAPABILITY_D_BLOCK 0x00
#if !(AST_HDMITX)
#define ReadSegmentBlockEDID(a,b,c,d)   I2C_ReadSegmentBlock(SA_EDID, a, b, c, d)
#endif
extern	void SI_CecSetDevicePA (uint16_t);

Type_EDID_Descriptors EDID_Data;        // holds parsed EDID data needed by the FW

//uint8_t EDID_TempData[EDID_BLOCK_SIZE + 3];


//////////////////////////////////////////////////////////////////////////////
//
// FUNCTION     :   uint8_t DoEDID_Checksum()
//
// PURPOSE      :   Calculte checksum of the 128 byte block pointed to by the
//                  pointer passed as parameter
//
// INPUT PARAMS :   Pointer to a 128 byte block whose checksum needs to be
//                  calculated
//
// OUTPUT PARAMS:   None
//
// GLOBALS USED :   None
//
// RETURNS      :   TRUE if chcksum is 0. FALSE if not.
//
//////////////////////////////////////////////////////////////////////////////

uint8_t DoEDID_Checksum(uint8_t *Block)
{
    uint8_t i;
    uint8_t CheckSum = 0;

    for (i = 0; i < EDID_BLOCK_SIZE; i++)
        CheckSum += Block[i];

    if (CheckSum)
        return FALSE;

    return TRUE;
}

//////////////////////////////////////////////////////////////////////////////
//
// FUNCTION     :   uint8_t CheckEDID_Header()
//
// PURPOSE      :   Checks if EDID header is correct per VESA E-EDID standard
//
// INPUT PARAMS :   Pointer to 1st EDID block
//
// OUTPUT PARAMS:   None
//
// GLOBALS USED :   None
//
// RETURNS      :   TRUE if Header is correct. FALSE if not.
//
//////////////////////////////////////////////////////////////////////////////

static uint8_t CheckEDID_Header(uint8_t *Block)
{
    uint8_t i = 0;

    if (Block[i])               // byte 0 must be 0
        return FALSE;

    for (i = 1; i <= EDID_HDR_NO_OF_FF; i++)
    {
        if(Block[i] != 0xFF)    // bytes [1..6] must be 0xFF
            return FALSE;
    }

    if (Block[i])               // byte 7 must be 0
        return FALSE;

    return TRUE;
}

//////////////////////////////////////////////////////////////////////////////
//
// FUNCTION     :   ParseEstablishedTiming()
//
// PURPOSE      :   Parse the established timing section of EDID Block 0 and
//                  print their decoded meaning to the screen.
//
// INPUT PARAMS :   Pointer to the array where the data read from EDID
//                  Block0 is stored.
//
// OUTPUT PARAMS:   None
//
// GLOBALS USED :   None
//
// RETURNS      :   Void
//
//////////////////////////////////////////////////////////////////////////////

#ifdef CONF__TPI_EDID_PRINT //(
static void ParseEstablishedTiming(uint8_t *Data)
{
    TPI_EDID_PRINT((TPI_EDID_CHANNEL,"Parsing Established Timing:\n"));
    TPI_EDID_PRINT((TPI_EDID_CHANNEL,"===========================\n"));


    // Parse Established Timing Byte #0

    if(Data[ESTABLISHED_TIMING_INDEX] & BIT_7)
        TPI_EDID_PRINT((TPI_EDID_CHANNEL,"720 x 400 @ 70Hz\n"));
    if(Data[ESTABLISHED_TIMING_INDEX] & BIT_6)
        TPI_EDID_PRINT((TPI_EDID_CHANNEL,"720 x 400 @ 88Hz\n"));
    if(Data[ESTABLISHED_TIMING_INDEX] & BIT_5)
        TPI_EDID_PRINT((TPI_EDID_CHANNEL,"640 x 480 @ 60Hz\n"));
    if(Data[ESTABLISHED_TIMING_INDEX] & BIT_4)
        TPI_EDID_PRINT((TPI_EDID_CHANNEL,"640 x 480 @ 67Hz\n"));
    if(Data[ESTABLISHED_TIMING_INDEX] & BIT_3)
        TPI_EDID_PRINT((TPI_EDID_CHANNEL,"640 x 480 @ 72Hz\n"));
    if(Data[ESTABLISHED_TIMING_INDEX] & BIT_2)
        TPI_EDID_PRINT((TPI_EDID_CHANNEL,"640 x 480 @ 75Hz\n"));
    if(Data[ESTABLISHED_TIMING_INDEX] & BIT_1)
        TPI_EDID_PRINT((TPI_EDID_CHANNEL,"800 x 600 @ 56Hz\n"));
    if(Data[ESTABLISHED_TIMING_INDEX] & BIT_0)
        TPI_EDID_PRINT((TPI_EDID_CHANNEL,"800 x 400 @ 60Hz\n"));

    // Parse Established Timing Byte #1:

    if(Data[ESTABLISHED_TIMING_INDEX + 1]  & BIT_7)
        TPI_EDID_PRINT((TPI_EDID_CHANNEL,"800 x 600 @ 72Hz\n"));
    if(Data[ESTABLISHED_TIMING_INDEX + 1]  & BIT_6)
        TPI_EDID_PRINT((TPI_EDID_CHANNEL,"800 x 600 @ 75Hz\n"));
    if(Data[ESTABLISHED_TIMING_INDEX + 1]  & BIT_5)
        TPI_EDID_PRINT((TPI_EDID_CHANNEL,"832 x 624 @ 75Hz\n"));
    if(Data[ESTABLISHED_TIMING_INDEX + 1]  & BIT_4)
        TPI_EDID_PRINT((TPI_EDID_CHANNEL,"1024 x 768 @ 87Hz\n"));
    if(Data[ESTABLISHED_TIMING_INDEX + 1]  & BIT_3)
        TPI_EDID_PRINT((TPI_EDID_CHANNEL,"1024 x 768 @ 60Hz\n"));
    if(Data[ESTABLISHED_TIMING_INDEX + 1]  & BIT_2)
        TPI_EDID_PRINT((TPI_EDID_CHANNEL,"1024 x 768 @ 70Hz\n"));
    if(Data[ESTABLISHED_TIMING_INDEX + 1]  & BIT_1)
        TPI_EDID_PRINT((TPI_EDID_CHANNEL,"1024 x 768 @ 75Hz\n"));
    if(Data[ESTABLISHED_TIMING_INDEX + 1]  & BIT_0)
        TPI_EDID_PRINT((TPI_EDID_CHANNEL,"1280 x 1024 @ 75Hz\n"));

    // Parse Established Timing Byte #2:

    if(Data[ESTABLISHED_TIMING_INDEX + 2] & 0x80)
        TPI_EDID_PRINT((TPI_EDID_CHANNEL,"1152 x 870 @ 75Hz\n"));

    if((!Data[0])&&(!Data[ESTABLISHED_TIMING_INDEX + 1]  )&&(!Data[2]))
        TPI_EDID_PRINT((TPI_EDID_CHANNEL,"No established video modes\n"));
}

//////////////////////////////////////////////////////////////////////////////
//
// FUNCTION     :   ParseStandardTiming()
//
// PURPOSE      :   Parse the standard timing section of EDID Block 0 and
//                  print their decoded meaning to the screen.
//
// INPUT PARAMS :   Pointer to the array where the data read from EDID
//                  Block0 is stored.
//
// OUTPUT PARAMS:   None
//
// GLOBALS USED :   None
//
// RETURNS      :   Void
//
//////////////////////////////////////////////////////////////////////////////

static void ParseStandardTiming(uint8_t *Data)
{
    uint8_t i;
    uint8_t AR_Code;

    TPI_EDID_PRINT((TPI_EDID_CHANNEL,"Parsing Standard Timing:\n"));
    TPI_EDID_PRINT((TPI_EDID_CHANNEL,"========================\n"));

    for (i = 0; i < NUM_OF_STANDARD_TIMINGS; i += 2)
    {
        if ((Data[STANDARD_TIMING_OFFSET + i] == 0x01) && ((Data[STANDARD_TIMING_OFFSET + i +1]) == 1))
        {
            TPI_EDID_PRINT((TPI_EDID_CHANNEL,"Standard Timing Undefined\n")); // per VESA EDID standard, Release A, Revision 1, February 9, 2000, Sec. 3.9
                }
        else
        {
            TPI_EDID_PRINT((TPI_EDID_CHANNEL,"Horizontal Active pixels: %i\n", (int)((Data[STANDARD_TIMING_OFFSET + i] + 31)*8)));    // per VESA EDID standard, Release A, Revision 1, February 9, 2000, Table 3.15

            AR_Code = (Data[STANDARD_TIMING_OFFSET + i +1] & TWO_MSBITS) >> 6;
            TPI_EDID_PRINT((TPI_EDID_CHANNEL,"Aspect Ratio: "));

            switch(AR_Code)
            {
                case AR16_10:
                    TPI_EDID_PRINT((TPI_EDID_CHANNEL,"16:10\n"));
                    break;

                case AR4_3:
                    TPI_EDID_PRINT((TPI_EDID_CHANNEL,"4:3\n"));
                    break;

                case AR5_4:
                    TPI_EDID_PRINT((TPI_EDID_CHANNEL,"5:4\n"));
                    break;

                case AR16_9:
                    TPI_EDID_PRINT((TPI_EDID_CHANNEL,"16:9\n"));
                    break;
            }
        }
    }
}

//////////////////////////////////////////////////////////////////////////////
//
// FUNCTION     :   ParseDetailedTiming()
//
// PURPOSE      :   Parse the detailed timing section of EDID Block 0 and
//                  print their decoded meaning to the screen.
//
// INPUT PARAMS :   Pointer to the array where the data read from EDID
//                  Block0 is stored.
//
//                  Offset to the beginning of the Detailed Timing Descriptor
//                  data.
//
//                                      Block indicator to distinguish between block #0 and blocks
//                                      #2, #3
//
// OUTPUT PARAMS:   None
//
// GLOBALS USED :   None
//
// RETURNS      :   Void
//
//////////////////////////////////////////////////////////////////////////////

static uint8_t ParseDetailedTiming(uint8_t *Data, uint8_t DetailedTimingOffset, uint8_t Block)
{
    uint8_t TmpByte;
    uint8_t i;
    uint16_t TmpWord;

    TmpWord = Data[DetailedTimingOffset + PIX_CLK_OFFSET] +
                256 * Data[DetailedTimingOffset + PIX_CLK_OFFSET + 1];

    if (TmpWord == 0x00)            // 18 byte partition is used as either for Monitor Name or for Monitor Range Limits or it is unused
    {
        if (Block == EDID_BLOCK_0)      // if called from Block #0 and first 2 bytes are 0 => either Monitor Name or for Monitor Range Limits
        {
            if (Data[DetailedTimingOffset + 3] == 0xFC) // these 13 bytes are ASCII coded monitor name
            {
                TPI_EDID_PRINT((TPI_EDID_CHANNEL,"Monitor Name: "));

                for (i = 0; i < 13; i++)
                {
                    TPI_EDID_PRINT((TPI_EDID_CHANNEL,"%c", Data[DetailedTimingOffset + 5 + i])); // Display monitor name on SiIMon
                }
                TPI_EDID_PRINT((TPI_EDID_CHANNEL,"\n"));
            }

            else if (Data[DetailedTimingOffset + 3] == 0xFD) // these 13 bytes contain Monitor Range limits, binary coded
            {
                TPI_EDID_PRINT((TPI_EDID_CHANNEL,"Monitor Range Limits:\n\n"));

                i = 0;
                TPI_EDID_PRINT((TPI_EDID_CHANNEL,"Min Vertical Rate in Hz: %d\n", (int) Data[DetailedTimingOffset + 5 + i++])); //
                TPI_EDID_PRINT((TPI_EDID_CHANNEL,"Max Vertical Rate in Hz: %d\n", (int) Data[DetailedTimingOffset + 5 + i++])); //
                TPI_EDID_PRINT((TPI_EDID_CHANNEL,"Min Horizontal Rate in Hz: %d\n", (int) Data[DetailedTimingOffset + 5 + i++])); //
                TPI_EDID_PRINT((TPI_EDID_CHANNEL,"Max Horizontal Rate in Hz: %d\n", (int) Data[DetailedTimingOffset + 5 + i++])); //
                TPI_EDID_PRINT((TPI_EDID_CHANNEL,"Max Supported pixel clock rate in MHz/10: %d\n", (int) Data[DetailedTimingOffset + 5 + i++])); //
                TPI_EDID_PRINT((TPI_EDID_CHANNEL,"Tag for secondary timing formula (00h=not used): %d\n", (int) Data[DetailedTimingOffset + 5 + i++])); //
                TPI_EDID_PRINT((TPI_EDID_CHANNEL,"Min Vertical Rate in Hz %d\n", (int) Data[DetailedTimingOffset + 5 + i])); //
                TPI_EDID_PRINT((TPI_EDID_CHANNEL,"\n"));
            }
        }

        else if (Block == EDID_BLOCK_2_3)                          // if called from block #2 or #3 and first 2 bytes are 0x00 (padding) then this
        {                                                                                          // descriptor partition is not used and parsing should be stopped
            TPI_EDID_PRINT((TPI_EDID_CHANNEL,"No More Detailed descriptors in this block\n"));
            TPI_EDID_PRINT((TPI_EDID_CHANNEL,"\n"));
            return FALSE;
        }
    }
    else                                            // first 2 bytes are not 0 => this is a detailed timing descriptor from either block
    {
        if((Block == EDID_BLOCK_0) && (DetailedTimingOffset == 0x36))
        {
            TPI_EDID_PRINT((TPI_EDID_CHANNEL,"\n\n\nParse Results, EDID Block #0, Detailed Descriptor Number 1:\n"));
            TPI_EDID_PRINT((TPI_EDID_CHANNEL,"===========================================================\n\n"));
        }
        else if((Block == EDID_BLOCK_0) && (DetailedTimingOffset == 0x48))
        {
            TPI_EDID_PRINT((TPI_EDID_CHANNEL,"\n\n\nParse Results, EDID Block #0, Detailed Descriptor Number 2:\n"));
            TPI_EDID_PRINT((TPI_EDID_CHANNEL,"===========================================================\n\n"));
        }

        TPI_EDID_PRINT((TPI_EDID_CHANNEL,"Pixel Clock (MHz * 100): %d\n", (int)TmpWord));

        TmpWord = Data[DetailedTimingOffset + H_ACTIVE_OFFSET] +
                256 * ((Data[DetailedTimingOffset + H_ACTIVE_OFFSET + 2] >> 4) & FOUR_LSBITS);
        TPI_EDID_PRINT((TPI_EDID_CHANNEL,"Horizontal Active Pixels: %d\n", (int)TmpWord));

        TmpWord = Data[DetailedTimingOffset + H_BLANKING_OFFSET] +
                256 * (Data[DetailedTimingOffset + H_BLANKING_OFFSET + 1] & FOUR_LSBITS);
        TPI_EDID_PRINT((TPI_EDID_CHANNEL,"Horizontal Blanking (Pixels): %d\n", (int)TmpWord));

        TmpWord = (Data[DetailedTimingOffset + V_ACTIVE_OFFSET] )+
                256 * ((Data[DetailedTimingOffset + (V_ACTIVE_OFFSET) + 2] >> 4) & FOUR_LSBITS);
        TPI_EDID_PRINT((TPI_EDID_CHANNEL,"Vertical Active (Lines): %d\n", (int)TmpWord));

        TmpWord = Data[DetailedTimingOffset + V_BLANKING_OFFSET] +
                256 * (Data[DetailedTimingOffset + V_BLANKING_OFFSET + 1] & LOW_NIBBLE);
        TPI_EDID_PRINT((TPI_EDID_CHANNEL,"Vertical Blanking (Lines): %d\n", (int)TmpWord));

        TmpWord = Data[DetailedTimingOffset + H_SYNC_OFFSET] +
                256 * ((Data[DetailedTimingOffset + (H_SYNC_OFFSET + 3)] >> 6) & TWO_LSBITS);
        TPI_EDID_PRINT((TPI_EDID_CHANNEL,"Horizontal Sync Offset (Pixels): %d\n", (int)TmpWord));

        TmpWord = Data[DetailedTimingOffset + H_SYNC_PW_OFFSET] +
                256 * ((Data[DetailedTimingOffset + (H_SYNC_PW_OFFSET + 2)] >> 4) & TWO_LSBITS);
        TPI_EDID_PRINT((TPI_EDID_CHANNEL,"Horizontal Sync Pulse Width (Pixels): %d\n", (int)TmpWord));

        TmpWord = ((Data[DetailedTimingOffset + V_SYNC_OFFSET] >> 4) & FOUR_LSBITS) +
                256 * ((Data[DetailedTimingOffset + (V_SYNC_OFFSET + 1)] >> 2) & TWO_LSBITS);
        TPI_EDID_PRINT((TPI_EDID_CHANNEL,"Vertical Sync Offset (Lines): %d\n", (int)TmpWord));

        TmpWord = ((Data[DetailedTimingOffset + V_SYNC_PW_OFFSET]) & FOUR_LSBITS) +
                256 * (Data[DetailedTimingOffset + (V_SYNC_PW_OFFSET + 1)] & TWO_LSBITS);
        TPI_EDID_PRINT((TPI_EDID_CHANNEL,"Vertical Sync Pulse Width (Lines): %d\n", (int)TmpWord));

        TmpWord = Data[DetailedTimingOffset + H_IMAGE_SIZE_OFFSET] +
                256 * (((Data[DetailedTimingOffset + (H_IMAGE_SIZE_OFFSET + 2)]) >> 4) & FOUR_LSBITS);
        TPI_EDID_PRINT((TPI_EDID_CHANNEL,"Horizontal Image Size (mm): %d\n", (int)TmpWord));

        TmpWord = Data[DetailedTimingOffset + V_IMAGE_SIZE_OFFSET] +
                256 * (Data[DetailedTimingOffset + (V_IMAGE_SIZE_OFFSET + 1)] & FOUR_LSBITS);
        TPI_EDID_PRINT((TPI_EDID_CHANNEL,"Vertical Image Size (mm): %d\n", (int)TmpWord));

        TmpByte = Data[DetailedTimingOffset + H_BORDER_OFFSET];
        TPI_EDID_PRINT((TPI_EDID_CHANNEL,"Horizontal Border (Pixels): %d\n", (int)TmpByte));

        TmpByte = Data[DetailedTimingOffset + V_BORDER_OFFSET];
        TPI_EDID_PRINT((TPI_EDID_CHANNEL,"Vertical Border (Lines): %d\n", (int)TmpByte));

        TmpByte = Data[DetailedTimingOffset + FLAGS_OFFSET];
        if (TmpByte & BIT_7)
        {
            TPI_EDID_PRINT((TPI_EDID_CHANNEL,"Interlaced\n"));
        }
        else
        {
            TPI_EDID_PRINT((TPI_EDID_CHANNEL,"Non-Interlaced\n"));
        }

        if (!(TmpByte & BIT_5) && !(TmpByte & BIT_6))
        {
            TPI_EDID_PRINT((TPI_EDID_CHANNEL,"Normal Display, No Stereo\n"));
        }
        else
        {
            TPI_EDID_PRINT((TPI_EDID_CHANNEL,"Refer to VESA E-EDID Release A, Revision 1, table 3.17\n"));
        }

        if (!(TmpByte & BIT_3) && !(TmpByte & BIT_4))
        {
            TPI_EDID_PRINT((TPI_EDID_CHANNEL,"Analog Composite\n"));
        }
        if ((TmpByte & BIT_3) && !(TmpByte & BIT_4))
        {
            TPI_EDID_PRINT((TPI_EDID_CHANNEL,"Bipolar Analog Composite\n"));
        }
        else if (!(TmpByte & BIT_3) && (TmpByte & BIT_4))
        {
            TPI_EDID_PRINT((TPI_EDID_CHANNEL,"Digital Composite\n"));
        }
        else if ((TmpByte & BIT_3) && (TmpByte & BIT_4))
        {
            TPI_EDID_PRINT((TPI_EDID_CHANNEL,"Digital Separate\n"));
        }

        TPI_EDID_PRINT((TPI_EDID_CHANNEL,"\n"));
    }
    return TRUE;
}
//////////////////////////////////////////////////////////////////////////////
//
// FUNCTION     :   ParseBlock_0_TimingDescriptors()
//
// PURPOSE      :   Parse EDID Block 0 timing descriptors per EEDID 1.3
//                  standard. printf() values to screen.
//
// INPUT PARAMS :   Pointer to the 128 byte array where the data read from EDID
//                  Block0 is stored.
//
// OUTPUT PARAMS:   None
//
// GLOBALS USED :   None
//
// RETURNS      :   Void
//
//////////////////////////////////////////////////////////////////////////////

static void ParseBlock_0_TimingDescriptors(uint8_t *Data)
{
#ifdef CONF__TPI_EDID_PRINT //(
    uint8_t i;
    uint8_t Offset;

    ParseEstablishedTiming(Data);
    ParseStandardTiming(Data);


    for (i = 0; i < NUM_OF_DETAILED_DESCRIPTORS; i++)
    {
        Offset = DETAILED_TIMING_OFFSET + (LONG_DESCR_LEN * i);
        ParseDetailedTiming(Data, Offset, EDID_BLOCK_0);
    }
#else //)(
        Data = Data;    // Dummy usage to suppress warning
#endif //)
}

//////////////////////////////////////////////////////////////////////////////
//
// FUNCTION     :   uint8_t Parse861ShortDescriptors()
//
// PURPOSE      :   Parse CEA-861 extension short descriptors of the EDID block
//                  passed as a parameter and save them in global structure
//                  EDID_Data.
//
// INPUT PARAMS :   A pointer to the EDID 861 Extension block being parsed.
//
// OUTPUT PARAMS:   None
//
// GLOBALS USED :   EDID_Data
//
// RETURNS      :   EDID_PARSED_OK if EDID parsed correctly. Error code if failed.
//
// NOTE         :   Fields that are not supported by the 9022/4 (such as deep
//                  color) are not parsed.
//
//////////////////////////////////////////////////////////////////////////////

uint8_t Parse861ShortDescriptors(uint8_t *Data)
{
    uint8_t LongDescriptorOffset;
    uint8_t DataBlockLength;
    uint8_t DataIndex;
    uint8_t ExtendedTagCode;
	uint8_t VSDB_BaseOffset = 0;

    uint8_t V_DescriptorIndex = 0;  // static to support more than one extension
    uint8_t A_DescriptorIndex = 0;  // static to support more than one extension

    uint8_t TagCode;

    uint8_t i;
    uint8_t j;

    if (Data[EDID_TAG_ADDR] != EDID_EXTENSION_TAG)
    {
        TPI_EDID_PRINT((TPI_EDID_CHANNEL,"EDID -> Extension Tag Error\n"));
        return EDID_EXT_TAG_ERROR;
    }

    if (Data[EDID_REV_ADDR] != EDID_REV_THREE)
    {
        TPI_EDID_PRINT((TPI_EDID_CHANNEL,"EDID -> Revision Error\n"));
        return EDID_REV_ADDR_ERROR;
    }

    LongDescriptorOffset = Data[LONG_DESCR_PTR_IDX];    // block offset where long descriptors start

    EDID_Data.UnderScan = ((Data[MISC_SUPPORT_IDX]) >> 7) & LSBIT;  // byte #3 of CEA extension version 3
    EDID_Data.BasicAudio = ((Data[MISC_SUPPORT_IDX]) >> 6) & LSBIT;
    EDID_Data.YCbCr_4_4_4 = ((Data[MISC_SUPPORT_IDX]) >> 5) & LSBIT;
    EDID_Data.YCbCr_4_2_2 = ((Data[MISC_SUPPORT_IDX]) >> 4) & LSBIT;

    DataIndex = EDID_DATA_START;            // 4

    while (DataIndex < LongDescriptorOffset)
    {
        TagCode = (Data[DataIndex] >> 5) & THREE_LSBITS;
        DataBlockLength = Data[DataIndex++] & FIVE_LSBITS;
        if ((DataIndex + DataBlockLength) > LongDescriptorOffset)
        {
            TPI_EDID_PRINT((TPI_EDID_CHANNEL,"EDID -> V Descriptor Overflow\n"));
            return EDID_V_DESCR_OVERFLOW;
        }

        i = 0;                                  // num of short video descriptors in current data block

        switch (TagCode)
        {
            case VIDEO_D_BLOCK:
                while ((i < DataBlockLength) && (i < MAX_V_DESCRIPTORS))        // each SVD is 1 byte long
                {
                    EDID_Data.VideoDescriptor[V_DescriptorIndex++] = Data[DataIndex++];
                    i++;
                }
                DataIndex += DataBlockLength - i;   // if there are more STDs than MAX_V_DESCRIPTORS, skip the last ones. Update DataIndex

                TPI_EDID_PRINT((TPI_EDID_CHANNEL,"EDID -> Short Descriptor Video Block\n"));
                break;

            case AUDIO_D_BLOCK:
                while (i < DataBlockLength/3)       // each SAD is 3 bytes long
                {
                    j = 0;
                    while (j < AUDIO_DESCR_SIZE)    // 3
                    {
                        EDID_Data.AudioDescriptor[A_DescriptorIndex][j++] = Data[DataIndex++];
                    }
                    A_DescriptorIndex++;
                    i++;
                }
                TPI_EDID_PRINT((TPI_EDID_CHANNEL,"EDID -> Short Descriptor Audio Block\n"));
                break;

            case  SPKR_ALLOC_D_BLOCK:
                EDID_Data.SpkrAlloc[i++] = Data[DataIndex++];       // although 3 bytes are assigned to Speaker Allocation, only
                DataIndex += 2;                                     // the first one carries information, so the next two are ignored by this code.
                TPI_EDID_PRINT((TPI_EDID_CHANNEL,"EDID -> Short Descriptor Speaker Allocation Block\n"));
                break;

            case USE_EXTENDED_TAG:
                ExtendedTagCode = Data[DataIndex++];

                switch (ExtendedTagCode)
                {
                    case VIDEO_CAPABILITY_D_BLOCK:

                                TPI_EDID_PRINT((TPI_EDID_CHANNEL,"EDID -> Short Descriptor Video Capability Block\n"));

                                                // TO BE ADDED HERE: Save "video capability" parameters in EDID_Data data structure
                                                // Need to modify that structure definition
                                                // In the meantime: just increment DataIndex by 1

                                                DataIndex += 1;    // replace with reading and saving the proper data per CEA-861 sec. 7.5.6 while incrementing DataIndex

                                                break;


                    case COLORIMETRY_D_BLOCK:
                        EDID_Data.ColorimetrySupportFlags = Data[DataIndex++] & BITS_1_0;
                        EDID_Data.MetadataProfile = Data[DataIndex++] & BITS_2_1_0;

                                TPI_EDID_PRINT((TPI_EDID_CHANNEL,"EDID -> Short Descriptor Colorimetry Block\n"));

                                                break;
#if (AST_HDMITX)
					default:
						/*
						 * current index is at (extended tag code + 1)
						 * -2 to move to 'tag code + length'
						 */
						DataIndex -= 2;
						DataIndex += ((Data[DataIndex] & 0x1F) + 1);
						break;
#endif
                }
                                break;

            case VENDOR_SPEC_D_BLOCK:
                VSDB_BaseOffset = DataIndex - 1;

                if ((Data[DataIndex++] == 0x03) &&    // check if sink is HDMI compatible
                    (Data[DataIndex++] == 0x0C) &&
                    (Data[DataIndex++] == 0x00))

                    EDID_Data.HDMI_Sink = TRUE;
                else
                    EDID_Data.HDMI_Sink = FALSE;

                EDID_Data.CEC_A_B = Data[DataIndex++];  // CEC Physical address
                EDID_Data.CEC_C_D = Data[DataIndex++];

#if (IS_CEC == 1) //(
				// Take the Address that was passed in the EDID and use this API
				// to set the physical address for CEC.
				{
					uint16_t	phyAddr;
					phyAddr = (uint16_t)EDID_Data.CEC_C_D;	 // Low-order nibbles
					phyAddr |= ((uint16_t)EDID_Data.CEC_A_B << 8); // Hi-order nibbles
					// Is the new PA different from the current PA?
					if (phyAddr != SI_CecGetDevicePA ())
					{
						// Yes!  So change the PA
						SI_CecSetDevicePA (phyAddr);
#if (IS_CDC == 1) //(
						// If we have CDC, it needs to be re-inited because the PA has changed
						// NOTE: This is a callback into device specific app code in Evita.  Maybe
						// later boards with have a different init func to call.  You are warned.
						CpCdcInit();
#endif //) IS_CDC
					}
				}
#endif //) IS_CEC

                if ((DataIndex + 7) > VSDB_BaseOffset + DataBlockLength)        // Offset of 3D_Present bit in VSDB
                        EDID_Data._3D_Supported = FALSE;
                else if (Data[DataIndex + 7] >> 7)
                        EDID_Data._3D_Supported = TRUE;
                else
                        EDID_Data._3D_Supported = FALSE;

                DataIndex += DataBlockLength - HDMI_SIGNATURE_LEN - CEC_PHYS_ADDR_LEN; // Point to start of next block
                TPI_EDID_PRINT((TPI_EDID_CHANNEL,"EDID -> Short Descriptor Vendor Block\n"));
                TPI_EDID_PRINT((TPI_EDID_CHANNEL,"\n"));
                break;

            default:
                TPI_EDID_PRINT((TPI_EDID_CHANNEL,"EDID -> Unknown Tag Code\n"));
                return EDID_UNKNOWN_TAG_CODE;

        }                   // End, Switch statement
    }                       // End, while (DataIndex < LongDescriptorOffset) statement

    return EDID_SHORT_DESCRIPTORS_OK;
}

//////////////////////////////////////////////////////////////////////////////
//
// FUNCTION     :   uint8_t Parse861LongDescriptors()
//
// PURPOSE      :   Parse CEA-861 extension long descriptors of the EDID block
//                  passed as a parameter and printf() them to the screen.
//                  EDID_Data.
//
// INPUT PARAMS :   A pointer to the EDID block being parsed
//
// OUTPUT PARAMS:   None
//
// GLOBALS USED :   None
//
// RETURNS      :   An error code if no long descriptors found; EDID_PARSED_OK
//                  if descriptors found.
//
//////////////////////////////////////////////////////////////////////////////
uint8_t Parse861LongDescriptors(uint8_t *Data)
{
    uint8_t LongDescriptorsOffset;
    uint8_t DescriptorNum = 1;

    LongDescriptorsOffset = Data[LONG_DESCR_PTR_IDX];   // EDID block offset 2 holds the offset

    if (!LongDescriptorsOffset)                         // per CEA-861-D, table 27
    {
        TPI_EDID_PRINT((TPI_EDID_CHANNEL,"EDID -> No Detailed Descriptors\n"));
        return EDID_NO_DETAILED_DESCRIPTORS;
    }

    // of the 1st 18-byte descriptor
    while (LongDescriptorsOffset + LONG_DESCR_LEN < EDID_BLOCK_SIZE)
    {
        TPI_EDID_PRINT((TPI_EDID_CHANNEL,"Parse Results - CEA-861 Long Descriptor #%d:\n", (int) DescriptorNum));
        TPI_EDID_PRINT((TPI_EDID_CHANNEL,"===============================================================\n"));

#ifdef CONF__TPI_EDID_PRINT //(
        if (!ParseDetailedTiming(Data, LongDescriptorsOffset, EDID_BLOCK_2_3))
                        break;
#endif //)
        LongDescriptorsOffset +=  LONG_DESCR_LEN;
        DescriptorNum++;
    }

    return EDID_LONG_DESCRIPTORS_OK;
}

//////////////////////////////////////////////////////////////////////////////
//
// FUNCTION     :   uint8_t Parse861Extensions()
//
// PURPOSE      :   Parse CEA-861 extensions from EDID ROM (EDID blocks beyond
//                  block #0). Save short descriptors in global structure
//                  EDID_Data. printf() long descriptors to the screen.
//
// INPUT PARAMS :   The number of extensions in the EDID being parsed
//
// OUTPUT PARAMS:   None
//
// GLOBALS USED :   None
//
// RETURNS      :   EDID_PARSED_OK if EDID parsed correctly. Error code if failed.
//
// NOTE         :   Fields that are not supported by the 9022/4 (such as deep
//                  color) were not parsed.
//
//////////////////////////////////////////////////////////////////////////////

static uint8_t Parse861Extensions(uint8_t NumOfExtensions)
{
	uint8_t ErrCode;
	uint8_t Segment = 0;
	uint8_t Block = 0;
	uint8_t Offset = 0;

	uint8_t tEdid[128];

	EDID_Data.HDMI_Sink = FALSE;

	do {
		Block++;

		Offset = 0;

		if ((Block % 2) > 0)
			Offset = EDID_BLOCK_SIZE;

		Segment = (uint8_t) (Block / 2);

		if (Block == 1) {
			ReadBlockEDID(EDID_BLOCK_1_OFFSET, tEdid, EDID_BLOCK_SIZE);  /* read first 128 bytes of EDID ROM */
#if (AST_HDMITX)
			memcpy(global_sink_edid + EDID_BLOCK_SIZE, tEdid, EDID_BLOCK_SIZE);
#endif
		} else {
			ReadSegmentBlockEDID(Segment, Offset, tEdid, EDID_BLOCK_SIZE); /* read next 128 bytes of EDID ROM */
		}

		TPI_EDID_PRINT((TPI_EDID_CHANNEL, "\n"));
		TPI_EDID_PRINT((TPI_EDID_CHANNEL, "EDID DATA (Segment = %d Block = %d Offset = %d):\n", (int) Segment, (int) Block, (int) Offset));

		if ((NumOfExtensions > 1) && (Block == 1))
			continue;

		/* do not parse EDID, disable parsing when AST_HDMITX is true */
#if (!AST_HDMITX)
		ErrCode = Parse861ShortDescriptors(tEdid);
		if (ErrCode != EDID_SHORT_DESCRIPTORS_OK)
			return ErrCode;

		ErrCode = Parse861LongDescriptors(tEdid);
		if (ErrCode != EDID_LONG_DESCRIPTORS_OK)
			return ErrCode;
#else
		/* have extension block and its */
		if (DoEDID_Checksum(tEdid))
			EDID_Data.HDMI_Sink = TRUE;
		else
			return EDID_EXT_CHECKSUM_ERROR;
#endif
	} while (Block < NumOfExtensions);

	return EDID_OK;
}

//////////////////////////////////////////////////////////////////////////////
//
// FUNCTION     :   uint8_t ParseEDID()
//
// PURPOSE      :   Extract sink properties from its EDID file and save them in
//                  global structure EDID_Data.
//
// INPUT PARAMS :   Pointer to start of EDID 0
//
// OUTPUT PARAMS:   None
//
// GLOBALS USED :   EDID_Data
//
// RETURNS      :   A uint8_t error code to indicates success or error type.
//
// NOTE         :   Fields that are not supported by the 9022/4 (such as deep
//                  color) were not parsed.
//
//////////////////////////////////////////////////////////////////////////////

uint8_t SiIParseEDID (uint8_t *pEdid, uint8_t *numExt)
{
uint8_t retVal = EDID_OK;

	if (!CheckEDID_Header(pEdid))
	{
		// first 8 bytes of EDID must be {0, FF, FF, FF, FF, FF, FF, 0}
		TPI_EDID_PRINT((TPI_EDID_CHANNEL,"EDID -> Incorrect Header\n"));
		retVal = EDID_INCORRECT_HEADER;
	}
    else if (!DoEDID_Checksum(pEdid))
	{
		// non-zero EDID checksum
		TPI_EDID_PRINT((TPI_EDID_CHANNEL,"EDID -> Checksum Error\n"));
		retVal = EDID_CHECKSUM_ERROR;
	}
    else
    {
    	ParseBlock_0_TimingDescriptors(pEdid);			// Parse EDID Block #0 Desctiptors

    	*numExt = pEdid[NUM_OF_EXTEN_ADDR];	// read # of extensions from offset 0x7E of block 0
    	TPI_EDID_PRINT((TPI_EDID_CHANNEL,"EDID -> Num 861 Extensions = %d\n", (int) *numExt));

    	if (!(*numExt))
    	{
    		// No extensions to worry about
        	TPI_EDID_PRINT((TPI_EDID_CHANNEL,"EDID_NO_861_EXTENSIONS\n"));
    		retVal = EDID_NO_861_EXTENSIONS;
    	}

    	//retVal = Parse861Extensions(NumOfExtensions);			// Parse 861 Extensions (short and long descriptors);
    }
    if (EDID_OK != retVal)
    {
    	TPI_EDID_PRINT((TPI_EDID_CHANNEL,"EDID DATA (Segment = 0 Block = 0 Offset = %d):\n", (int) EDID_BLOCK_0_OFFSET));
        TPI_EDID(
        uint8_t i;
        uint8_t j;
        uint8_t k;
        	for (j = 0, i = 0; j < 128; j++)
        	{
        		k = pEdid[j];
        		printf("%02X ", (int) k);
        		i++;

        		if (i == 0x10)
        		{
        			printf("\n");
        			i = 0;
        		}
        	}
        	printf("\n");
        )
    }
    return retVal;
}


uint8_t DoEdidRead (void)
{
	uint8_t SysCtrlReg;
	uint8_t Result;
	uint8_t NumOfExtensions;
	uint8_t tEdid[128];

	// If we already have valid EDID data, skip this whole thing
	if (!edidDataValid)
	{
		// Request access to DDC bus from the receiver
		if (GetDDC_Access(&SysCtrlReg))
		{
			TPI_EDID_PRINT((TPI_EDID_CHANNEL,"Have DDC Access\n"));
			ReadBlockEDID(EDID_BLOCK_0_OFFSET, tEdid, EDID_BLOCK_SIZE); /* read first 128 bytes of EDID ROM */ 
			Result = SiIParseEDID(tEdid, &NumOfExtensions);

			TPI_EDID_PRINT((TPI_EDID_CHANNEL,"Parsed EDID\n"));
			if (Result != EDID_OK)
			{
				if (Result == EDID_NO_861_EXTENSIONS)
				{
					EDID_Data.HDMI_Sink = FALSE;
					TPI_EDID_PRINT((TPI_EDID_CHANNEL,"EDID -> No 861 Extensions\n"));
				}
				else
				{
					TPI_EDID_PRINT((TPI_EDID_CHANNEL,"EDID -> Parse FAILED\n"));
#if (AST_HDMITX)
					return Result;
#endif
				}
			}
			else
			{
				TPI_EDID_PRINT((TPI_EDID_CHANNEL,"EDID -> Parse OK\n"));
				Result = Parse861Extensions(NumOfExtensions);		// Parse 861 Extensions (short and long descriptors);
				if (Result != EDID_OK)
				{
					TPI_EDID_PRINT((TPI_EDID_CHANNEL,"EDID -> Extension Parse FAILED\n"));
#if (AST_HDMITX)
					return Result;
#endif
				}
			}

			if (!ReleaseDDC(SysCtrlReg))				// Host must release DDC bus once it is done reading EDID
			{
				TPI_EDID_PRINT((TPI_EDID_CHANNEL,"EDID -> DDC bus release failed\n"));
				return EDID_DDC_BUS_RELEASE_FAILURE;
			}
		}
		else
		{
			TPI_EDID_PRINT((TPI_EDID_CHANNEL,"EDID -> DDC bus request failed\n"));
			return EDID_DDC_BUS_REQ_FAILURE;
		}

#if (AST_HDMITX)
		memcpy(global_sink_edid, tEdid, 128);
#endif
		SetEdidDataValid;
	}
	TPI_EDID_PRINT((TPI_EDID_CHANNEL,"edidDataValid\n"));

	return EDID_OK;
}

#endif //)



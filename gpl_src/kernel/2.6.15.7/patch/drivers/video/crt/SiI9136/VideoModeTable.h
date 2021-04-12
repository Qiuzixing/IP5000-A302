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
 * @file  VideoModeTable.h
 *
 * @brief Definitions of the Video Mode Table.
 *
 *****************************************************************************
*/

typedef struct
{
    uint8_t Mode_C1;
    uint8_t Mode_C2;
    uint8_t SubMode;
} ModeIdType;

typedef struct
{
    uint16_t Pixels;
    uint16_t Lines;
} PxlLnTotalType;

typedef struct
{
    uint16_t H;
    uint16_t V;
} HVPositionType;

typedef struct
{
    uint16_t H;
    uint16_t V;
} HVResolutionType;

typedef struct
{
    uint8_t           RefrTypeVHPol;
    uint16_t           VFreq;
    PxlLnTotalType Total;
} TagType;

typedef struct
{
    uint8_t IntAdjMode;
    uint16_t HLength;
    uint8_t VLength;
    uint16_t Top;
    uint16_t Dly;
    uint16_t HBit2HSync;
    uint8_t VBit2VSync;
    uint16_t Field2Offset;
}  _656Type;

typedef struct
{
    uint8_t VactSpace1;
    uint8_t VactSpace2;
    uint8_t Vblank1;
    uint8_t Vblank2;
    uint8_t Vblank3;
} Vspace_Vblank;

//
// WARNING!  The entries in this enum must remian in the samre order as the PC Codes part
// of the VideoModeTable[].
//
typedef	enum
{
    PC_640x350_85_08 = 0,
    PC_640x400_85_08,
    PC_720x400_70_08,
    PC_720x400_85_04,
    PC_640x480_59_94,
    PC_640x480_72_80,
    PC_640x480_75_00,
    PC_640x480_85_00,
    PC_800x600_56_25,
    PC_800x600_60_317,
    PC_800x600_72_19,
    PC_800x600_75,
    PC_800x600_85_06,
    PC_1024x768_60,
    PC_1024x768_70_07,
    PC_1024x768_75_03,
    PC_1024x768_85,
    PC_1152x864_75,
    PC_1600x1200_60,
    PC_1280x768_59_95,
    PC_1280x768_59_87,
    PC_280x768_74_89,
    PC_1280x768_85,
    PC_1280x960_60,
    PC_1280x960_85,
    PC_1280x1024_60,
    PC_1280x1024_75,
    PC_1280x1024_85,
    PC_1360x768_60,
    PC_1400x105_59_95,
    PC_1400x105_59_98,
    PC_1400x105_74_87,
    PC_1400x105_84_96,
    PC_1600x1200_65,
    PC_1600x1200_70,
    PC_1600x1200_75,
    PC_1600x1200_85,
    PC_1792x1344_60,
    PC_1792x1344_74_997,
    PC_1856x1392_60,
    PC_1856x1392_75,
    PC_1920x1200_59_95,
    PC_1920x1200_59_88,
    PC_1920x1200_74_93,
    PC_1920x1200_84_93,
    PC_1920x1440_60,
    PC_1920x1440_75,
    PC_12560x1440_60,
    PC_SIZE			// Must be last
} PcModeCode_t;

typedef struct
{
    ModeIdType       ModeId;
    uint16_t             PixClk;
    TagType          Tag;
    HVPositionType   Pos;
    HVResolutionType Res;
    uint8_t             AspectRatio;
    _656Type         _656;
    uint8_t             PixRep;
	Vspace_Vblank    VsVb;
    uint8_t             _3D_Struct;
} VModeInfoType;

#define NSM                     0   // No Sub-Mode

#define	DEFAULT_VIDEO_MODE		0	// 640  x 480p @ 60 VGA

#define ProgrVNegHNeg           0x00
#define ProgrVNegHPos           0x01
#define ProgrVPosHNeg           0x02
#define ProgrVPosHPos           0x03

#define InterlaceVNegHNeg       0x04
#define InterlaceVPosHNeg       0x05
#define InterlaceVNgeHPos       0x06
#define InterlaceVPosHPos       0x07

#define VIC_BASE                0
#define HDMI_VIC_BASE           43
#define VIC_3D_BASE             47
#define PC_BASE                 64

// Aspect ratio
//=============
#define R_4                      0   // 4:3
#define R_4or16                  1   // 4:3 or 16:9
#define R_16                     2   // 16:9

extern const VModeInfoType VModesTable[];
extern const uint8_t AspectRatioTable[];

//uint8_t ConvertVIC_To_VM_Index(uint8_t, uint8_t);
int ConvertVIC_To_VM_Index(void);

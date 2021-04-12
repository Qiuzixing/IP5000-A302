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
 * @file  edid.h
 *
 * @brief EDID definitions.
 *
 *****************************************************************************
*/


#define MAX_V_DESCRIPTORS			20
#define MAX_A_DESCRIPTORS			10
#define MAX_SPEAKER_CONFIGURATIONS	 4
#define AUDIO_DESCR_SIZE			 3

typedef struct
{								// for storing EDID parsed data
	uint8_t VideoDescriptor[MAX_V_DESCRIPTORS];	// maximum number of video descriptors
	uint8_t AudioDescriptor[MAX_A_DESCRIPTORS][3];	// maximum number of audio descriptors
	uint8_t SpkrAlloc[MAX_SPEAKER_CONFIGURATIONS];	// maximum number of speaker configurations
	uint8_t UnderScan;								// "1" if DTV monitor underscans IT video formats by default
	uint8_t BasicAudio;							// Sink supports Basic Audio
	uint8_t YCbCr_4_4_4;							// Sink supports YCbCr 4:4:4
	uint8_t YCbCr_4_2_2;							// Sink supports YCbCr 4:2:2
	uint8_t HDMI_Sink;								// "1" if HDMI signature found
	uint8_t CEC_A_B;								// CEC Physical address. See HDMI 1.3 Table 8-6
	uint8_t CEC_C_D;
	uint8_t ColorimetrySupportFlags;				// IEC 61966-2-4 colorimetry support: 1 - xvYCC601; 2 - xvYCC709
	uint8_t MetadataProfile;
	uint8_t _3D_Supported;
} Type_EDID_Descriptors;

enum EDID_ErrorCodes
{
	EDID_OK,
	EDID_INCORRECT_HEADER,
	EDID_CHECKSUM_ERROR,
	EDID_NO_861_EXTENSIONS,
#if AST_HDMITX
	EDID_EXT_CHECKSUM_ERROR,
#endif
	EDID_SHORT_DESCRIPTORS_OK,
	EDID_LONG_DESCRIPTORS_OK,
	EDID_EXT_TAG_ERROR,
	EDID_REV_ADDR_ERROR,
	EDID_V_DESCR_OVERFLOW,
	EDID_UNKNOWN_TAG_CODE,
	EDID_NO_DETAILED_DESCRIPTORS,
	EDID_DDC_BUS_REQ_FAILURE,
	EDID_DDC_BUS_RELEASE_FAILURE
};

extern uint8_t DoEdidRead(void);
extern uint8_t SiIParseEDID(uint8_t *pEdid, uint8_t *numExt);
extern uint8_t Parse861ShortDescriptors(uint8_t *Data);
extern uint8_t Parse861LongDescriptors(uint8_t *Data);


#if 0
#define ReadBlockEDID(a,b,c)	I2C_ReadBlock(SA_EDID, a, b, c)
#else

#if !(AST_HDMITX)
#define ReadBlockEDID(a,b,c)                                        \
    TPI_DEBUG_PRINT((TPI_DEBUG_CHANNEL,"Read Block EDID Entry\n")); \
    I2C_ReadBlock(SA_EDID, a, b, c);                                \
    TPI_DEBUG_PRINT((TPI_DEBUG_CHANNEL,"Read Block EDID Exit\n"));
#endif

#endif

#define IsHDMI_Sink()			(EDID_Data.HDMI_Sink)
#define IsCEC_DEVICE()			(((EDID_Data.CEC_A_B != 0xFF) && (EDID_Data.CEC_C_D != 0xFF)) ? TRUE : FALSE)

extern Type_EDID_Descriptors EDID_Data;		// holds parsed EDID data needed by the FW

extern uint8_t EDID_TempData[];

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
 * @file videomodedesc.h
 *
 * @brief Communications structure shared between SiIMon, TPI_FW and AMF_LIB
 *
 *****************************************************************************
*/

#ifndef _VIDEOMODEDESC_H_ //{
#define _VIDEOMODEDESC_H_
//------------------------------------------------------------------------------
// Video Mode Description
// This is the common API that passes Video Mode information to the firmware,
// whether the source is the AMF libray or Simon.
//------------------------------------------------------------------------------
typedef struct
{
	unsigned char  VIC;					// VIC OR the HDMI_VIC
	unsigned char  AspectRatio;			// 4x3 or 16x9
	unsigned char  InputColorSpace;
	unsigned char  InputColorDepth;
	unsigned char  HDCPAuthenticated;
	unsigned char  HDMIVideoFormat;		// VIC, HDMI_VIC or 3D
	unsigned char  ThreeDStructure;		// Valid when (HDMIVideoFormat == VMD_HDMIFORMAT_3D)
	unsigned char  ThreeDExtData;		// Valid when (HDMIVideoFormat == VMD_HDMIFORMAT_3D) && (ThreeDStructure == VMD_3D_SIDEBYSIDEHALF)
	unsigned char  InputRangeExpansion;
	unsigned char  OutputColorSpace;
	unsigned char  OutputColorDepth;
	unsigned char  OutputRangeCompression;

	unsigned char  CTypeChannelCount;
	unsigned char  SampleSizeFrequency;
	unsigned char  ChannelSpeakerAlloc;

	unsigned char  HBRAStatus;

} VideoModeDescription_t,*PVideoModeDescription_t;

void videoModeReport(char *pszFile,int iLine,VideoModeDescription_t *pVideoModeDescription);
#define VideoModeReport(desc) videoModeReport(__FILE__,__LINE__,desc)
#endif //}

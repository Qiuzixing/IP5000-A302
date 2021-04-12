///*****************************************
//  Copyright (C) 2009-2014
//  ITE Tech. Inc. All Rights Reserved
//  Proprietary and Confidential
///*****************************************
//   @file   <debug.h>
//   @author Jau-Chih.Tseng@ite.com.tw
//   @date   2014/11/21
//   @fileversion: ITE_MHLRX_SAMPLE_V1.13
//******************************************/

#ifndef _DEBUG_H_
#define _DEBUG_H_

#include "ast_utilities.h"

//#define Debug_message 0

#ifndef Debug_message
#define Debug_message 1
#endif


#if Debug_message
	#define MHLRX_DEBUG_PRINTF(x) //uinfo x
	#define EQ_DEBUG_PRINTF(x) //uinfo x
	#define EQ_PORT0_PRINTF(x) //uinfo x
	#define EQ_PORT1_PRINTF(x) //uinfo x
	#define VIDEOTIMNG_DEBUG_PRINTF(x) uinfo x
	#define IT6802_DEBUG_INT_PRINTF(x)   uinfo x
	#define IT6802_MHL_DEBUG_PRINTF(x) //uinfo x
	#define MHL_MSC_DEBUG_PRINTF(x) //uinfo x
	#define HDMIRX_VIDEO_PRINTF(x)   //uinfo x
	#define HDMIRX_AUDIO_PRINTF(x) //uinfo x
	#define HDMIRX_DEBUG_PRINT(x) //uinfo x
	#define CEC_DEBUG_PRINTF(x) //uinfo x
	#define EDID_DEBUG_PRINTF(x) //uinfo x
	#define IT680X_DEBUG_PRINTF(x) uinfo x
	#define VSDB_DEBUG_PRINTF(x) //uinfo x
	#define RCP_DEBUG_PRINTF(x) //uinfo x
	#define MHL3D_DEBUG_PRINTF(x) //uinfo x
	#define CBUS_CAL_PRINTF(x) //uinfo x
	#define MHL_INT_PRINTF(x) //uinfo x
#else
	#define MHLRX_DEBUG_PRINTF(x)
	#define EQ_DEBUG_PRINTF(x)
	#define EQ_PORT0_PRINTF(x)
	#define EQ_PORT1_PRINTF(x)
	#define VIDEOTIMNG_DEBUG_PRINTF(x)
	#define IT6802_DEBUG_INT_PRINTF(x)
	#define IT6802_MHL_DEBUG_PRINTF(x)
	#define MHL_MSC_DEBUG_PRINTF(x)
	#define HDMIRX_VIDEO_PRINTF(x)
	#define HDMIRX_AUDIO_PRINTF(x)
	#define HDMIRX_DEBUG_PRINT(x)
	#define CEC_DEBUG_PRINTF(x)
	#define EDID_DEBUG_PRINTF(x)
	#define IT680X_DEBUG_PRINTF(x)
	#define VSDB_DEBUG_PRINTF(x)
	#define RCP_DEBUG_PRINTF(x)
	#define MHL3D_DEBUG_PRINTF(x)
	#define CBUS_CAL_PRINTF(x)
	#define MHL_INT_PRINTF(x)
#endif




#endif

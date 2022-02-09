///*****************************************
//  Copyright (C) 2009-2014
//  ITE Tech. Inc. All Rights Reserved
//  Proprietary and Confidential
///*****************************************
//   @file   >hdmitx.h<
//   @author Jau-Chih.Tseng@ite.com.tw
//   @date   2009/12/09
//   @fileversion: CAT6613_SAMPLEINTERFACE_1.09
//******************************************/

#ifndef _HDMITX_H_
#define _HDMITX_H_

#if !defined(CODE202)

#ifdef EXTERN_HDCPROM
#pragma message("Defined EXTERN_HDCPROM")
#endif // EXTERN_HDCPROM

#define SUPPORT_EDID
#define SUPPORT_HDCP
#define SUPPORT_SHA
// #define DISABLE_TX_CSC

#define SUPPORT_INPUTRGB
#define SUPPORT_INPUTYUV444
#define SUPPORT_INPUTYUV422
// #define SUPPORT_SYNCEMBEDDED
// #define SUPPORT_DEGEN

#ifdef SUPPORT_SYNCEMBEDDED
#pragma message("defined SUPPORT_SYNCEMBEDDED for Sync Embedded timing input or CCIR656 input.")
#endif

#ifndef _MCU_8051_ // DSSSHA need large computation data rather than 8051 supported.
#define SUPPORT_DSSSHA
#endif

#if defined(SUPPORT_INPUTYUV444) || defined(SUPPORT_INPUTYUV422)
#define SUPPORT_INPUTYUV
#endif

#endif//#if !defined(CODE202)

#ifdef CONFIG_AST1500_CAT6613
#include "IO.h"
#else
#ifdef _MCU_8051_
    #include "mcu.h"
    #include "io.h"
    #include "utility.h"
	
#else // not MCU
    #include <windows.h>
    #include <winioctl.h>
    #include <stdio.h>
    #include <stdlib.h>
    #include <string.h>
    #include <stdarg.h>
    #include "ioaccess.h"
    #include "install.h"
    #include "pc.h"
#endif // MCU
#endif

#if defined(CODE202)
#include "config.h"
#endif
#include "typedef_hdmitx.h"
// #include "edid.h"
#include "cat6613_drv.h"

#define HDMITX_INSTANCE_MAX 1

#define SIZEOF_CSCMTX 18
#define SIZEOF_CSCGAIN 6
#define SIZEOF_CSCOFFSET 3

///////////////////////////////////////////////////////////////////////
// Output Mode Type
///////////////////////////////////////////////////////////////////////

#define RES_ASPEC_4x3 0
#define RES_ASPEC_16x9 1
#define F_MODE_REPT_NO 0
#define F_MODE_REPT_TWICE 1
#define F_MODE_REPT_QUATRO 3
#define F_MODE_CSC_ITU601 0
#define F_MODE_CSC_ITU709 1

///////////////////////////////////////////////////////////////////////
// ROM OFFSET
///////////////////////////////////////////////////////////////////////
#define ROMOFF_INT_TYPE 0
#define ROMOFF_INPUT_VIDEO_TYPE 1
#define ROMOFF_OUTPUT_AUDIO_MODE 8
#define ROMOFF_AUDIO_CH_SWAP 9



#ifdef CONFIG_AST1500_CAT6613

#ifndef DelayMS
#if (HANDLE_INT_IN_THREAD && USE_THREAD)
#define DelayMS(x) msleep(x)
#else
#define DelayMS(x) mdelay(x)
#endif
#endif

#else
#define TIMER_LOOP_LEN 10
#define MS(x) (((x)+(TIMER_LOOP_LEN-1))/TIMER_LOOP_LEN); // for timer loop


#ifdef _MCU_8051_ // DSSSHA need large computation data rather than 8051 supported.
#ifndef DelayMS
#define DelayMS(x) delay1ms(x)
#endif
#endif

#endif

//#define HDMITX_DEBUG_PRINTF(x) printf x

#ifdef CONFIG_AST1500_CAT6613
#define SUPPORT_AUDI_AudSWL 24//to do:confirm this value
#else
#define SUPPORT_AUDI_AudSWL 16
#endif
#endif // _HDMITX_H_


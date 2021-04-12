///*****************************************
//  Copyright (C) 2009-2014
//  ITE Tech. Inc. All Rights Reserved
//  Proprietary and Confidential
///*****************************************
//   @file   <config.h>
//   @author Jau-Chih.Tseng@ite.com.tw
//   @date   2014/11/21
//   @fileversion: ITE_MHLRX_SAMPLE_V1.13
//******************************************/
#ifndef _CONFIG_H_
#define _CONFIG_H_

#define AST_HDMIRX 1 //This flag is used to identify those code sections modified by aspeed. CONFIG_AST1500_CAT6023 might be used instead.

#if AST_HDMIRX
#define AST_CSC_LINEAR
#endif

#if !(AST_HDMIRX)
#define _MCU_8051_

#ifndef _MCU_8051_
#include "platform.h"
#endif
#endif //#if !(AST_HDMIRX)

#ifndef _IT6803_
#ifndef _IT6802_
#ifndef _IT6801_
#define _IT6802_
#endif
#endif
#endif

#if !(AST_HDMIRX)
#ifdef _IT6803_
#pragma message("defined _IT6803_")
//#define ENABLE_IT6803	// only for IT6803 Usage
#endif

#ifdef _IT6802_
#pragma message("defined _IT6802_")
#endif

#ifdef _IT6801_
#pragma message("defined _IT6801_")
#endif
#endif //#if !(AST_HDMIRX)

//#define SUPPORT_I2C_SLAVE
#ifdef SUPPORT_I2C_SLAVE
#pragma message ("SUPPORT_I2C_SLAVE defined")
#endif

#define _EN_DUAL_PIXEL_CTRL_

#define _EN_BLOCK_PWRDN_

#define SUPPORT_OUTPUTRGB

#define SUPPORT_INPUTYUV

#define _HBR_I2S_

#define Enable_IT6802_CEC
#if defined(Enable_IT6802_CEC)
#define INTR_MODE 1
#endif
#endif // _CONFIG_H_

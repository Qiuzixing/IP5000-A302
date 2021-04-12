///*****************************************
//  Copyright (C) 2009-2019
//  ITE Tech. Inc. All Rights Reserved
//  Proprietary and Confidential
///*****************************************
//   @file   <sha1.h>
//   @author Kuro.Chung@ite.com.tw
//   @date   2019/03/05
//   @fileversion: iTE6805_MCUSRC_1.31
//******************************************/


#ifndef _SHA_1_H_
#define _SHA_1_H_

#ifdef _MCU_8051_
    #include "Mcu.h"
#endif

#include <string.h>
#include "debug.h"
#include "config.h"
#include "typedef.h"
#if Debug_message
    #include <stdio.h>
#endif

#ifndef HDCP_DEBUG_PRINTF
    #define HDCP_DEBUG_PRINTF(x)
#endif //HDCP_DEBUG_PRINTF

#ifndef HDCP_DEBUG_PRINTF1
    #define HDCP_DEBUG_PRINTF1(x)
#endif //HDCP_DEBUG_PRINTF1

#ifndef HDCP_DEBUG_PRINTF2
    #define HDCP_DEBUG_PRINTF2(x)
#endif //HDCP_DEBUG_PRINTF2


#ifndef DISABLE_HDCP
void SHA_Simple(void *p,iTE_u32 len,iTE_u8 *output);
void SHATransform(iTE_u32 * h);
#endif

#endif // _SHA_1_H_

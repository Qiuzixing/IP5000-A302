///*****************************************
//  Copyright (C) 2009-2014
//  ITE Tech. Inc. All Rights Reserved
//  Proprietary and Confidential
///*****************************************
//   @file   <mcu.h>
//   @author Jau-Chih.Tseng@ite.com.tw
//   @date   2011/06/23
//   @fileversion: HDMIRX_SAMPLE_2.13
//******************************************/

///////////////////////////////////////////////////////////////////////////////
// Include file
///////////////////////////////////////////////////////////////////////////////
#ifndef _MCU_H_
#define _MCU_H_

#ifndef AST_HDMIRX
#include <stdio.h>
#include <stdlib.h>
#include "config.h"
#include "Reg_c51.h"
#endif
///////////////////////////////////////////////////////////////////////////////
// Global Definition
///////////////////////////////////////////////////////////////////////////////
//#define DIVA
///////////////////////////////////////////////////////////////////////////////
// Type Definition
///////////////////////////////////////////////////////////////////////////////
#include "typedef.h"
#include "debug.h"
#include "io.h"
#ifndef AST_HDMIRX
#include "TimerProcess.h"
#endif
///////////////////////////////////////////////////////////////////////////////
// Constant Definition
///////////////////////////////////////////////////////////////////////////////
#define HDMIRXADR       0x90
#define RXDEV           0
#define ExtEDID         1
#define IntEDID         2


#define RXADR           0xB0

#define DELAY_TIME      1           // unit = 1 us;
#define IDLE_TIME       100         // unit = 1 ms;

#define HIGH            1
#define LOW             0

#define ACTIVE          1
#define DISABLE         0

#define DevNum          1
#define LOOPMS          20          // 20-> 5 ,  030408, Clive
///////////////////////////////////////////////////////////////////////////////
// 8051 Definition
///////////////////////////////////////////////////////////////////////////////

#ifndef AST_HDMIRX

#ifndef DISABLE_EDID_PARSING
#pragma message ("Defined EDID Parsing")
#define _EDID_Parsing_
#endif


    sbit HW_RSTN=P3^5;
    sbit SCL_PORT           = P1^0;
    sbit SDA_PORT           = P1^1;
    sbit EDID_SCL=P1^3;
    sbit EDID_SDA=P1^2;
    sbit EDID_WP           = P4^0;     // inverse by MOS
#ifndef OLD_VGA_DDC
    sbit VGA_SCL            = P1^4;     // VGA DDC control
    sbit VGA_SDA            = P1^7;
#else
    sbit VGA_SCL            = P0^0;     // VGA DDC control
    sbit VGA_SDA            = P0^1;
#endif
	sbit Hold_Pin           = P1^5 ;

    sbit RX_HPD=P4^2;
    sbit RXINT=P3^2;
    sbit REPEATER_SET=P1^6 ;

#endif//#ifndef AST_HDMIRX
/////////////////////////////////////////////////////////////////
// CHIP DEFINE
/////////////////////////////////////////////////////////////////

#endif      // _MCU_H_

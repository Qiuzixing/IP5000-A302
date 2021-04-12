///*****************************************
//  Copyright (C) 2009-2019
//  ITE Tech. Inc. All Rights Reserved
//  Proprietary and Confidential
///*****************************************
//   @file   <mcu.h>
//   @author Kuro.Chung@ite.com.tw
//   @date   2019/03/05
//   @fileversion: iTE6805_MCUSRC_1.31
//******************************************/


#ifndef _MCU_H_
#define _MCU_H_

#include "typedef.h"
#include "Reg_c51.h"


// iTE6805 MCU config
sbit Hold_Pin		= P2^0;	// kuro fix from P1^5 to P2^0
sbit SEL_PORT_1		= P1^6;

sbit  EDID_WP0	= P2^6; // kuro fix
sbit  EDID_WP1	= P2^4; // kuro fix

sbit  gpHPD1 	= P2^5; // kuro fix

//#if (iTE68051==TRUE)
sbit  gpPORT1_5V_STATE = P3^3; // 6805A0 port1 HPD detect can't detect in REG, so use it
sbit  gp6028 = P3^6; // 1 = default 6028out, 0 = another one
//#endif

sbit  PORT_SWITCH = P3^0; // for switch PORT

//sbit EVB_CONTROL_4K_DOWNSCALE = P0^0;
//define for TI
//sbit  gpPORT1_5V_STATE = P3^6; // 6805A0 port1 HPD detect can't detect in REG, so use it
//define for TI

sbit NEAREST_P1 = P0^1;
sbit MIDDLE_P1 = P0^0;
sbit FAREST_P1 = P0^2;

sbit  DEV0_SCL_PORT = P1^0;
sbit  DEV0_SDA_PORT = P1^1;

sbit  DEV1_SCL_PORT = P1^2;
sbit  DEV1_SDA_PORT = P1^3;

sbit  DEV2_SCL_PORT = P1^5;
sbit  DEV2_SDA_PORT = P1^6;

sbit  DEV3_SCL_PORT = P1^0;
sbit  DEV3_SDA_PORT = P1^1;

sbit  DEV4_SCL_PORT = P1^2;
sbit  DEV4_SDA_PORT = P1^3;

sbit VsyncToggle = P0^2;
sbit AFE_RESET_PIN = P3^5;
#endif


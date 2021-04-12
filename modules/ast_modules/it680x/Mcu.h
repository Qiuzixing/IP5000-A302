///*****************************************
//  Copyright (C) 2009-2014
//  ITE Tech. Inc. All Rights Reserved
//  Proprietary and Confidential
///*****************************************
//   @file   <mcu.h>
//   @author Jau-Chih.Tseng@ite.com.tw
//   @date   2014/11/21
//   @fileversion: ITE_MHLRX_SAMPLE_V1.13
//******************************************/

#ifndef _MCU_H_
#define _MCU_H_

#include "typedef.h"

#define _IT6802_BOARD_

//#define SUPPORT_UART_CMD
//#define 		Enable_IT6802_CEC		//for ARC and HEAC Function

#ifdef Enable_IT6802_CEC

	#ifdef Enable_IT6802_CEC
		#include "it680x_cec.h"
	#endif

	#define	DEBUG_IT6802_CEC
	#define IT6802CECGPIOid		0		// for identife IT6802_CEC use Pin 1.0 & 1.1
#endif


/****************************************************************************/

#define IT6802A0_HDMI_ADDR 0x94	//Hardware Fixed I2C address of IT6802 HDMI
#define IT6802B0_HDMI_ADDR 0x90	//Hardware Fixed I2C address of IT6802 HDMI
#define MHL_ADDR 0xE0	//Software programmable I2C address of IT6802 MHL
#define EDID_ADDR 0xA8	//Software programmable I2C address of IT6802 EDID RAM
#define CEC_ADDR 0xC8	//Software programmable I2C address of IT6802 CEC


//#define DP_ADDR 0x90
//#define ADC_ADDR 0x90

#define HDMI_DEV  0
//#define DP_DEV	  0

//#define RXDEV           0
//#define MAXRXDEV        1
/****************************************************************************/

#endif

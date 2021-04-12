/******************************************************************************
 * Copyright 2009-2011 Silicon Image, Inc.  All rights reserved.
 * No part of this work may be reproduced, modified, distributed, transmitted,
 * transcribed, or translated into any language or computer format, in any form
 * or by any means without written permission of:
 *   Silicon Image, Inc.
 *   1060 East Arques Avenue, Sunnyvale, California 94085
 ******************************************************************************/
/******************************************************************************
 *
 * @file defs.h
 *
 * @brief Definitions for the sii9136_300 specific library.
 *
 ******************************************************************************/

#include "debug.h"
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Project Definitions
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#define AST_HDMITX 1

#define T_MONITORING_PERIOD		10
#define R_INIT_PERIOD			10
#define R_MONITORING_PERIOD		600

#define TX_HW_RESET_PERIOD      200
#define RX_HW_RESET_PERIOD		600

#define INT_CONTROL 			0x00 // Interrupt pin is push-pull and active high (this is normally 0x06)
#define SiI_DEVICE_ID           0xB4
#define SiI_DEVICE_STRING       "SiI 9136\n"

#define SCK_SAMPLE_EDGE			0x80
/*
 * change to rising (previous configuration)
 * for this setting, we'll get it done later
 */
#if 0
#define CLOCK_EDGE_FALLING
#endif
#define DEEP_COLOR
#define I2S_AUDIO

#define HBR_AUDIO

//#define USE_DE_GENERATOR

#define DEV_SUPPORT_CEC_FEATURE_ABORT
#define DEV_SUPPORT_CEC_CONFIG_CPI_0

#define HDCP_DONT_CLEAR_BSTATUS
#define CALL_CBUS_WAKEUP_GENERATOR()  		/* do nothing */
#define CLEAR_CBUS_TOGGLE()           		/* do nothing */

//#define KSVFORWARD

/*\
| | TPI API Version
\*/

#define TPI_PRODUCT_CODE		1

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Debug Definitions
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Compile debug prints inline or not
#define CONF__TPI_DEBUG_PRINT   	(ENABLE)
#define CONF__TPI_EDID_PRINT    	(DISABLE)
#define CONF__TPI_TRACE_PRINT		(DISABLE)
#define CONF__TPI_CDC_PRINT			(DISABLE)
#define CONF__TPI_TIME_PRINT		(DISABLE)
#define CONF__TPI_ENTRY_EXIT_PRINT	(DISABLE)
#define CONF__CBUS_ALWAYS_PRINT  	(DISABLE)
#define CONF__CBUS_DEBUG_PRINT  	(DISABLE)
#define CONF__SCR_PAD_DEBUG_PRINT   (DISABLE)
#define CONF__HDCP_DEBUG_PRINT      (ENABLE)
#define CONF__HDCP_TRACE_PRINT      (ENABLE)

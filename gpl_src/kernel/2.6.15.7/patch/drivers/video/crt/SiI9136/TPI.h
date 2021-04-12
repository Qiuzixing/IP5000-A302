/*
 *****************************************************************************
 *
 * Copyright (c) 2002-2009, 2010, 2011, Silicon Image, Inc.  All rights reserved.
 * No part of this work may be reproduced, modified, distributed, transmitted,
 * transcribed, or translated into any language or computer format, in any form
 * or by any means without written permission of: Silicon Image, Inc., 1060
 * East Arques Avenue, Sunnyvale, California 94085
 *****************************************************************************
 */
/*
 *****************************************************************************
 * @file  TPI.h
 *
 * @brief Interface definitions for the TPI.
 *
 *****************************************************************************
*/
#ifndef __TPI_H__
#define __TPI_H__

#include "si_basetypes.h"


void TxHW_Reset (void);

uint8_t TPI_Init (void);			// Document purpose, usage
void TPI_Poll (void);			// Document purpose, usage, rename

void RestartHDCP (void);		// Document purpose, usage

void SetAudioMute (uint8_t audioMute);

#if (AST_HDMITX)
void av_mute(void);
void av_unmute(void);
int sii9136_software_reset(void);
void reset_test_cfg(int cfg);
int reset_test(void);
u32 tmds_clock_stable(void);
void SiiMhlTxDrvTmdsControl(bool_t enable, bool_t UnMute);
void no_tmds_disable_cfg(u32 cfg);
u32 no_tmds_disable(void);
u32 info_only(void);
void info_only_reset(u32 cfg);

#undef RESET_TEST
#endif

#define T_EN_TPI       	10
#define T_HPD_DELAY    	10
#define INTERVAL_RSEN_DEGLITCH 150
#define ENABLE_EDID_CHANGE_CHECK 1

#ifdef MHL_CONNECTION_STATE_MACHINE //{
#define ENABLE_AUTO_SOFT_RESET			0x04
#define DISABLE_AUTO_SOFT_RESET			0x00
#define ASR_VALUE						ENABLE_AUTO_SOFT_RESET
#define DDC_XLTN_TIMEOUT_MAX_VAL		0x30
//	static uint8_t cbusRevID;
#endif //}

#if !(AST_HDMITX)
#define CONTROL_COMMAND_VERSION (8)
// TPI Firmware Version
//=====================
#define TPI_FW_VERSION_DEF {CONTROL_COMMAND_VERSION + '0', '.', '0', '4',' '}
extern code const TpiFwVersion_t TPI_FW_VERSION; //see globals.h
#endif

typedef enum
{
    MHL_TX_EVENT_NONE           = 0x00    /* No event worth reporting.  */
   ,MHL_TX_EVENT_DISCONNECTION  = 0x01    /* MHL connection has been lost */
   ,MHL_TX_EVENT_CONNECTION     = 0x02    /* MHL connection has been established */
   ,MHL_TX_EVENT_RCP_READY      = 0x03    /* MHL connection is ready for RCP */
   ,MHL_TX_EVENT_RCP_RECEIVED   = 0x04    /* Received an RCP. Key Code in "eventParameter" */
   ,MHL_TX_EVENT_RCPK_RECEIVED  = 0x05    /* Received an RCPK message */
   ,MHL_TX_EVENT_RCPE_RECEIVED  = 0x06    /* Received an RCPE message .*/
}MhlTxEvent_e;
///////////////////////////////////////////////////////////////////////////////
//
// AppNotifyMhlDownStreamHPDStatusChange
//
//  This function is invoked from the MhlTx component to notify the application about
//  changes to the Downstream HPD state of the MHL subsystem.
//
// Application module must provide this function.
//
void  AppNotifyMhlDownStreamHPDStatusChange(bool_t connected);
void  PopulateVMDfromAMF(void);

#endif //#ifndef __TPI_H__

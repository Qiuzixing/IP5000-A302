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
 * @file  Globals.c
 *
 * @brief Definitions of the global functions and variables.
 *
 *****************************************************************************
*/

//#include <stdio.h>
#include "defs.h"
#include "si_basetypes.h"
#include "si_datatypes.h"
//#include "at89c51xd2.h"
#include "constants.h"
//#include "gpio.h"

#include "si_apiConfigure.h"
#include "videomodedesc.h"
//#include "AMF_Lib.h"
//#include "serialpacket.h"
#include "TPI.h"
#include "Externals.h"
VideoModeDescription_t VideoModeDescription;		// Video Mode description block
#if !(AST_HDMITX)
AMFVideoAudioModeDescription_t AMFVideoAudioModeDescription;		// AMF Video Mode description block
#endif
int VModeTblIndex;					// Index to Video Mode Table row for our current mode


// Patches
//========
uint8_t EmbeddedSynPATCH;

#if !(AST_HDMITX)
//UART
//====
uint8_t CommModeState;
uint8_t TXBusy;
uint8_t IDX_InChar;
uint8_t NumOfArgs;
uint8_t MessageType;
uint8_t serialFlags=0;
#endif

uint16_t globalFlags=0;
#if (AST_HDMITX)
u8 global_sink_edid[256];
u8 hdcp_retry_counter = 0;
#endif
// Checksums
uint8_t g_audio_Checksum;	// Audio checksum

#if !(AST_HDMITX)
// Serial Communication Buffer
FirmwareScratchPad_u g_ScratchPad;
#endif

#if !(AST_HDMITX)
code const TpiFwVersion_t TPI_FW_VERSION = TPI_FW_VERSION_DEF ;
#endif


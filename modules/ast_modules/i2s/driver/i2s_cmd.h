/*
 * Copyright (c) 2004-2012
 * ASPEED Technology Inc. All Rights Reserved
 * Proprietary and Confidential
 *
 * By using this code you acknowledge that you have signed and accepted
 * the terms of the ASPEED SDK license agreement.
 */

#ifndef _I2S_CMD_
#define _I2S_CMD_

#define I2S_MAGIC 0x49325344

#define CMD_CHANGE_AUDIO_SETTINGS 0x00000000

typedef struct _Ctrl_Header
{
    ULONG	Cmd;
	ULONG	DataLength;
} Ctrl_Header, *pCtrl_Header;

#endif

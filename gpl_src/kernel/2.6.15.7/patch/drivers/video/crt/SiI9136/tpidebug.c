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
 * @file  tpidebug.c
 *
 * @brief Implementation of the TPI Debug APIs.
 *
 *****************************************************************************
*/
#include "defs.h"
#include "si_basetypes.h"
#include "si_datatypes.h"
#include "constants.h"

#if !(AST_HDMITX)
#include <stdio.h>
#include <stdarg.h>
#include "at89c51xd2.h"
#include "gpio.h"
#endif



unsigned short usDebugChannels=0xFFFF;

#if (AST_HDMITX)
int DebugChannelEnabled(unsigned short channel)
{
	return (channel & usDebugChannels)?1:0;
}


void ReadModWriteTpiDebugChannelMask(unsigned short AndMask,unsigned short XorMask)
{
	usDebugChannels &= AndMask;
	usDebugChannels ^= XorMask;
}

#else //#if (AST_HDMITX)

int DebugChannelEnabled(unsigned short channel)
{
	return ((0 == DEBUG)&&(channel & usDebugChannels))?1:0;
}

void TpiDebugPrint(unsigned short channel,char *pszFormat,...)
{
	if (0 == DEBUG)
	{
		if (channel & usDebugChannels)
		{
		va_list pArgs;
			va_start(pArgs,pszFormat);
			vprintf(pszFormat,pArgs);
			va_end(pArgs);
		}
	}
}

void ReadModWriteTpiDebugChannelMask(unsigned short AndMask,unsigned short XorMask)
{
	usDebugChannels &= AndMask;
	usDebugChannels ^= XorMask;
}

#endif //#if (AST_HDMITX)



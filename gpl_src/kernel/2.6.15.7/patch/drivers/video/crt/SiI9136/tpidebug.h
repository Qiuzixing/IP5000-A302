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
 * @file  tpidebug.h
 *
 * @brief Definitions of the TPI Debug APIs.
 *
 *****************************************************************************
*/

#ifndef _TPI_DEBUG_H_
#define _TPI_DEBUG_H_
//#include <stdarg.h>

#if (AST_HDMITX)
extern unsigned short usDebugChannels;
#define TpiDebugPrint(c, args...) do {} while (0)
#else
void TpiDebugPrint(unsigned short channel,char *pszFormat,...);
#endif

void ReadModWriteTpiDebugChannelMask(unsigned short AndMask,unsigned short XorMask);
#define RMW_TPI_DEBUG_CHANNEL_MASK(andmask,xormask) ReadModWriteTpiDebugChannelMask(unsigned char AndMask,unsigned char XorMask)

int DebugChannelEnabled(unsigned short channel);

#define FOO_DEBUG_PRINT(x, fmt, args...) do { if (x & usDebugChannels) uinfo(fmt, ##args); } while (0)
#define FOO_DEBUG_IF(channel,x,y) if (DebugChannelEnabled(channel)&&(x)) y
#define FOO_DEBUG(channel,x) if (DebugChannelEnabled(channel)) {x}
/*\
| | Trace Print Macro
| |
| | Note: TPI_DEBUG_PRINT Requires double parenthesis
| | Example:  TPI_DEBUG_PRINT(("hello, world!\n"));
\*/
typedef enum
{
	 TPI_TRACE_CHANNEL      =   0x01
	,TPI_DEBUG_CHANNEL      =   0x02
	,TPI_EDID_CHANNEL       =   0x04
	,TPI_CDC_CHANNEL        =   0x08
	,TPI_TIME_CHANNEL       =   0x10
	,TPI_ENTRY_EXIT_CHANNEL =   0x20
    ,CBUS_DEBUG_CHANNEL     =   0x40
    ,CBUS_ALWAYS_CHANNEL    =   0x80
    ,HDCP_DEBUG_CHANNEL     = 0x0100
    ,HDCP_TRACE_CHANNEL     = 0x0200
    ,SCR_PAD_DEBUG_CHANNEL  = 0x0400
}TpiDebugChannels_e;


#if (CONF__TPI_TRACE_PRINT == ENABLE)
    #define TPI_TRACE_PRINT(x) FOO_DEBUG_PRINT(TPI_TRACE_CHANNEL,x)
    #define TPI_TRACE_IF(x,y)  FOO_DEBUG_IF(TPI_TRACE_CHANNEL,x,y)
    #define TPI_TRACE(x)       FOO_DEBUG(TPI_TRACE_CHANNEL,x)
#else
    #define TPI_TRACE_PRINT(x) /*nothing*/
    #define TPI_TRACE_IF(x,y)  /*nothing*/
    #define TPI_TRACE(x)       /*nothing*/
#endif

/*\
| | Debug Print Macro
| |
| | Note: TPI_DEBUG_PRINT Requires double parenthesis
| | Example:  TPI_DEBUG_PRINT(("hello, world!\n"));
\*/

#if (CONF__TPI_DEBUG_PRINT == ENABLE)
    #define TPI_DEBUG_PRINT(x) FOO_DEBUG_PRINT x
    #define TPI_DEBUG_IF(x,y)  FOO_DEBUG_IF(TPI_DEBUG_CHANNEL,x,y)
    #define TPI_DEBUG(x)       FOO_DEBUG(TPI_DEBUG_CHANNEL,x)
#else
    #define TPI_DEBUG_PRINT(x) /*nothing*/
    #define TPI_DEBUG_IF(x,y) /*nothing*/
    #define TPI_DEBUG(x) /*nothing*/
#endif

/*\
| | EDID Print Macro
| |
| | Note: To enable EDID description printing, both CONF__TPI_EDID_PRINT and CONF__TPI_DEBUG_PRINT must be enabled
| |
| | Note: TPI_EDID_PRINT Requires double parenthesis
| | Example:  TPI_EDID_PRINT(("hello, world!\n"));
\*/

#if (CONF__TPI_EDID_PRINT == ENABLE)
    #define TPI_EDID_PRINT(x) FOO_DEBUG_PRINT x
    #define TPI_EDID_IF(x,y)  FOO_DEBUG_IF(TPI_EDID_CHANNEL,x,y)
    #define TPI_EDID(x)       /*FOO_DEBUG(TPI_EDID_CHANNEL,x) */
#else
    #define TPI_EDID_PRINT(x) /*nothing*/
    #define TPI_EDID_IF(x,y) /*nothing*/
    #define TPI_EDID(x) /*nothing*/
#endif


/*\
| | CDC Print Macro
| |
| | Note: To enable CDC description printing, both CONF__TPI_CDC_PRINT and CONF__TPI_DEBUG_PRINT must be enabled
| |
| | Note: TPI_CDC_PRINT Requires double parenthesis
| | Example:  TPI_CDC_PRINT(("hello, world!\n"));
\*/

#if (CONF__TPI_CDC_PRINT == ENABLE)
    #define TPI_CDC_PRINT(x) FOO_DEBUG_PRINT(TPI_CDC_CHANNEL,x)
    #define TPI_CDC_IF(x,y)  FOO_DEBUG_IF(TPI_CDC_CHANNEL,x,y)
    #define TPI_CDC          FOO_DEBUG(TPI_CDC_CHANNEL,x)
#else
    #define TPI_CDC_PRINT(x) /*nothing*/
    #define TPI_CDC_IF(x,y)  /*nothing*/
    #define TPI_CDC          /*nothing*/
#endif


/*\
| | Timer Print Macro
| |
| | Note: To enable Timer description printing, both CONF__TPI_TIME_PRINT and CONF__TPI_DEBUG_PRINT must be enabled
| |
| | Note: TPI_CDC_PRINT Requires double parenthesis
| | Example:  TPI_CDC_PRINT(("hello, world!\n"));
\*/

#if (CONF__TPI_TIME_PRINT == ENABLE)
    #define TPI_TIME_PRINT(x) FOO_DEBUG_PRINT(TPI_TIME_CHANNEL,x)
    #define TPI_TIME_IF(x,y)  FOO_DEBUG_IF(TPI_TIME_CHANNEL,x,y)
    #define TPI_TIME(x,y)     FOO_DEBUG(TPI_TIME_CHANNEL,x)
#else
    #define TPI_TIME_PRINT(x) /*nothing*/
    #define TPI_TIME_IF(x,y)  /*nothing*/
    #define TPI_TIME(x,y)     /*nothing*/
#endif

/*\
| | Entry/Exit Print Macro
| |
| | Note: To enable Function Entry/Exit printing, both CONF__TPI_ENTRY_EXIT_PRINT and CONF__TPI_DEBUG_PRINT must be enabled
| |
| | Note: TPI_ENTRY_EXIT_PRINT Requires double parenthesis
| | Example:  TPI_ENTRY_EXIT_PRINT(("hello, world!\n"));
\*/

#if (CONF__TPI_ENTRY_EXIT_PRINT == ENABLE)
    #define TPI_ENTRY_PRINT(x)	FOO_DEBUG_PRINT(TPI_ENTRY_EXIT_CHANNEL,x)
    #define TPI_ENTRY_IF(x,y)   FOO_DEBUG_IF(TPI_ENTRY_EXIT_CHANNEL,x,y)
    #define TPI_ENTRY(x,y)      FOO_DEBUG(TPI_ENTRY_EXIT_CHANNEL,x)

    #define TPI_EXIT_PRINT(x)	FOO_DEBUG_PRINT(TPI_ENTRY_EXIT_CHANNEL,x)
    #define TPI_EXIT_IF(x,y)    FOO_DEBUG_IF(TPI_ENTRY_EXIT_CHANNEL,x,y)
    #define TPI_EXIT(x,y)       FOO_DEBUG(TPI_ENTRY_EXIT_CHANNEL,x)
#else
    #define TPI_ENTRY_PRINT(x)  /*nothing*/
    #define TPI_ENTRY_IF(x,y)   /*nothing*/
    #define TPI_ENTRY(x,y)      /*nothing*/

    #define TPI_EXIT_PRINT(x)   /*nothing*/
    #define TPI_EXIT_IF(x,y)    /*nothing*/
    #define TPI_EXIT(x,y)       /*nothing*/
#endif

/*\
| | CBUS Debug Print Macro
| |
| | Note: To enable, CONF__CBUS_DEBUG_PRINT must be enabled
| |
| | Note: CBUS_DEBUG_PRINT Requires double parenthesis
| | Example:  CBUS_DEBUG_PRINT(("hello, world!\n"));
\*/

#if (CONF__CBUS_DEBUG_PRINT == ENABLE)
    #define CBUS_DEBUG_PRINT(x) FOO_DEBUG_PRINT(CBUS_DEBUG_CHANNEL,x)
    #define CBUS_DEBUG_IF(x,y)  FOO_DEBUG_IF(CBUS_DEBUG_CHANNEL,x,y)
    #define CBUS_DEBUG(x)       FOO_DEBUG(CBUS_DEBUG_CHANNEL,x)
#else
    #define CBUS_DEBUG_PRINT(x) /*nothing*/
    #define CBUS_DEBUG_IF(x,y)  /*nothing*/
    #define CBUS_DEBUG(x)       /*nothing*/
#endif


/*\
| | CBus Always Print Macro
| |
| | Note: To enable, CONF__CBUS_ALWAYS_PRINT must be enabled
| |
| | Note: CBUS_ALWAYS_PRINT Requires double parenthesis
| | Example:  CBUS_ALWAYS_PRINT(("hello, world!\n"));
\*/

#if (CONF__CBUS_ALWAYS_PRINT == ENABLE)
    #define CBUS_ALWAYS_PRINT(x) FOO_DEBUG_PRINT(CBUS_ALWAYS_CHANNEL,x)
    #define CBUS_ALWAYS_IF(x,y)  FOO_DEBUG_IF(CBUS_ALWAYS_CHANNEL,x,y)
    #define CBUS_ALWAYS(x)       FOO_DEBUG(CBUS_ALWAYS_CHANNEL,x)
#else
    #define CBUS_ALWAYS_PRINT(x) /*nothing*/
    #define CBUS_ALWAYS_IF(x,y)  /*nothing*/
    #define CBUS_ALWAYS(x)       /*nothing*/
#endif

/*\
| | SerComm Always Print Macro
| |
| | Note: To enable, CONF__SCR_PAD_DEBUG_PRINT must be enabled
| |
| | Note: SCR_PAD_DEBUG_PRINT Requires double parenthesis
| | Example:  SCR_PAD_DEBUG_PRINT(("hello, world!\n"));
\*/

#ifdef CONF__SCR_PAD_DEBUG_PRINT //{
#if (CONF__SCR_PAD_DEBUG_PRINT == ENABLE)
    #define SCR_PAD_DEBUG_PRINT(x) TpiDebugPrint(SCR_PAD_DEBUG_CHANNEL,"%s Arg(%d)\n",#x,(int)((uint8_t *)&x-(uint8_t *)&g_ScratchPad))
    #define SCR_PAD_DEBUG_IF(x,y)  FOO_DEBUG_IF(SCR_PAD_DEBUG_CHANNEL,x,y)
    #define SCR_PAD_DEBUG(x)       FOO_DEBUG(SCR_PAD_DEBUG_CHANNEL,x)
#else
    #define SCR_PAD_DEBUG_PRINT(x) /*nothing*/
    #define SCR_PAD_DEBUG_IF(x,y)  /*nothing*/
    #define SCR_PAD_DEBUG(x)       /*nothing*/
#endif
#else    //}{
    #define SCR_PAD_DEBUG_PRINT(x) /*nothing*/
    #define SCR_PAD_DEBUG_IF(x,y)  /*nothing*/
    #define SCR_PAD_DEBUG(x)       /*nothing*/
#endif //}

#ifdef CONF__HDCP_DEBUG_PRINT //{
#if (CONF__HDCP_DEBUG_PRINT == ENABLE)
    #define HDCP_DEBUG_PRINT(x)	FOO_DEBUG_PRINT x
    #define HDCP_DEBUG_IF(x,y)  FOO_DEBUG_IF(HDCP_DEBUG_CHANNEL,x,y)
    #define HDCP_DEBUG(x)       FOO_DEBUG(HDCP_DEBUG_CHANNEL,x)
#else
    #define HDCP_DEBUG_PRINT(x) /*nothing*/
    #define HDCP_DEBUG_IF(x,y)  /*nothing*/
    #define HDCP_DEBUG(x)       /*nothing*/
#endif
#else    //}{
    #define HDCP_DEBUG_PRINT(x) /*nothing*/
    #define HDCP_DEBUG_IF(x,y)  /*nothing*/
    #define HDCP_DEBUG(x)       /*nothing*/
#endif //}

#ifdef CONF__HDCP_TRACE_PRINT //{
#if (CONF__HDCP_TRACE_PRINT == ENABLE)
    #define HDCP_TRACE_PRINT(x)	FOO_DEBUG_PRINT x
    #define HDCP_TRACE_IF(x,y)  FOO_DEBUG_IF(HDCP_TRACE_CHANNEL,x,y)
    #define HDCP_TRACE(x)       FOO_DEBUG(HDCP_TRACE_CHANNEL,x)
#else
    #define HDCP_TRACE_PRINT(x) /*nothing*/
    #define HDCP_TRACE_IF(x,y)  /*nothing*/
    #define HDCP_TRACE(x)       /*nothing*/
#endif
#else    //}{
    #define HDCP_TRACE_PRINT(x) /*nothing*/
    #define HDCP_TRACE_IF(x,y)  /*nothing*/
    #define HDCP_TRACE(x)       /*nothing*/
#endif //}


#define BOOL_TO_STRING(x) (x ? STRINGIZE(x):"NOT"STRINGIZE(x))

#endif // _TPI_DEBUG_H_

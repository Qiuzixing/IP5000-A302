/*
 *****************************************************************************
 *
 * Copyright 2008-2009, 2010, 2011, Silicon Image, Inc.  All rights reserved.
 * No part of this work may be reproduced, modified, distributed, transmitted,
 * transcribed, or translated into any language or computer format, in any form
 * or by any means without written permission of: Silicon Image, Inc., 1060
 * East Arques Avenue, Sunnyvale, California 94085
 *****************************************************************************
 */
/*
 *****************************************************************************
 * @file  si_datatypes.h
 *
 * @brief Silicon Image data type header (conforms to C99).
 *
 *****************************************************************************
*/

#ifndef __SI_DATATYPES_H__
#define __SI_DATATYPES_H__

#if AST_HDMITX
#define ROM
#define XDATA
#else
#define ROM     code        // 8051 type of ROM memory
#define XDATA   xdata       // 8051 type of external memory
#endif

//------------------------------------------------------------------------------
// Configuration defines used by hal_config.h
//------------------------------------------------------------------------------

//#define ENABLE      (0xFF)
//#define DISABLE     (0x00)

#define BIT0                    0x01
#define BIT1                    0x02
#define BIT2                    0x04
#define BIT3                    0x08
#define BIT4                    0x10
#define BIT5                    0x20
#define BIT6                    0x40
#define BIT7                    0x80

#define CEC_NO_TEXT         0
#define CEC_NO_NAMES        1
#define CEC_ALL_TEXT        2
#define INCLUDE_CEC_NAMES   CEC_NO_TEXT

#define MSG_ALWAYS              0x00
#define MSG_STAT                0x01
#define MSG_DBG                 0x02
#if AST_HDMITX
#define DEBUG_PRINT(l, x)      if (l <= MSG_ALWAYS) printf x
#else
#define DEBUG_PRINT(l,x)      if (l<=5) printf x
#endif

// see include/i2c_slave_addrs.h

#define SET_BITS    0xFF
#define CLEAR_BITS  0x00

uint8_t SiIRegioRead ( uint16_t regAddr );
void SiIRegioWrite ( uint16_t regAddr, uint8_t value );
void SiIRegioModify ( uint16_t regAddr, uint8_t mask, uint8_t value);
void SiIRegioWriteBlock ( uint16_t regAddr, uint8_t* buffer, uint16_t length);

#endif  // __SI_DATATYPES_H__


/*
 *****************************************************************************
 *
 * Copyright 2010, 2011, Silicon Image, Inc.  All rights reserved.
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
 * @brief base level data type definitions
 *
 *****************************************************************************
*/

#ifndef _SI_BASETYPES_H_
#define _SI_BASETYPES_H_

#if (AST_HDMITX)
#include <linux/types.h>
#else
    /* C99 defined data types.  */

typedef unsigned char  uint8_t;
typedef unsigned short uint16_t;
typedef unsigned long  uint32_t;

typedef signed char    int8_t;
typedef signed short   int16_t;
typedef signed long    int32_t;
#endif

    /* Emulate C99/C++ bool type    */
#ifdef __cplusplus
typedef bool bool_t;
#else
typedef enum
{
    false   = 0,
    true
} bool_t;

//typedef char uint8_t;
#endif // __cplusplus

#endif

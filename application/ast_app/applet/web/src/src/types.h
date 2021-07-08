/*
 * types.h
 *
 *  Created on: 2013年11月20日
 *      Author: sid
 */


#ifndef __TYPES_H__
#define __TYPES_H__

#include <sys/types.h>

typedef u_int8_t            UInt8;
typedef int8_t              SInt8;
typedef int16_t             SInt16;
typedef u_int16_t           UInt16;
typedef int32_t             SInt32;
typedef u_int32_t           UInt32;
typedef bool                Bool;
#if __WORDSIZE == 64
typedef int64_t             SInt64;
typedef u_int64_t           UInt64;
#else
typedef signed long long    SInt64;
typedef unsigned long long  UInt64;
#endif
typedef float               Float32;
typedef double              Float64;
typedef UInt16              Bool16;
typedef UInt8               Bool8;

typedef SInt32              OS_Error;
typedef unsigned long       PointerSizedInt;

enum
{
    OS_NoErr = (OS_Error) 0,
    OS_BadURLFormat = (OS_Error) - 100,
    OS_NotEnoughSpace = (OS_Error) - 101
};

#define TRUE                true
#define FALSE               false

// for tsmuxer and tsdemuxer blow
#define L_DEBUG     10000
#define L_INFO      20000
#define L_WARN      30000
#define L_ERROR     40000
#define L_FATAL     50000
#define L_SYSTEM    L_FATAL

#include <stdarg.h>


#endif /* __TYPES_H__ */

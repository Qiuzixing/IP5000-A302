/*
 * types.h
 *
 *  Created on: 2013锟斤拷11锟斤拷20锟斤拷
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
typedef int                 Bool;
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

#endif /* __TYPES_H__ */


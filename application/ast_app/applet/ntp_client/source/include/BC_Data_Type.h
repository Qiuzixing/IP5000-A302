/****************************************************************************
* 版权信息：（C）2013，深圳市拔超科技有限公司版权所有
* 系统名称： 
* 文件名称：BC_Data_Type.h 
* 文件说明：
* 作    者：Heyman 
* 版本信息：1.0 
* 设计日期：2020-08-21
* 修改记录：
* 日    期		版    本		修改人 		修改摘要  
****************************************************************************/

#ifndef __DATA_TYPE_H
#define __DATA_TYPE_H

#include <stdio.h>

typedef unsigned char      BOOLEAN;
typedef unsigned char      BOOL;
typedef signed char        S8;
typedef unsigned char      U8;
typedef signed short       S16;
typedef unsigned short     U16;
typedef signed long        S32;
typedef unsigned long      U32;
typedef signed long long   S64;
typedef unsigned long long U64;

#ifndef NULL
#define NULL            (void *)0
#endif

#define BIT0  0x0001
#define BIT1  0x0002
#define BIT2  0x0004
#define BIT3  0x0008
#define BIT4  0x0010
#define BIT5  0x0020
#define BIT6  0x0040
#define BIT7  0x0080
#define BIT8  0x0100
#define BIT9  0x0200
#define BIT10 0x0400
#define BIT11 0x0800
#define BIT12 0x1000
#define BIT13 0x2000
#define BIT14 0x4000
#define BIT15 0x8000
#define BIT16 0x10000
#define BIT17 0x20000
#define BIT18 0x40000
#define BIT19 0x80000
#define BIT20 0x100000
#define BIT21 0x200000
#define BIT22 0x400000
#define BIT23 0x800000
#define BIT24 0x1000000
#define BIT25 0x2000000
#define BIT26 0x4000000
#define BIT27 0x8000000
#define BIT28 0x10000000
#define BIT29 0x20000000
#define BIT30 0x40000000
#define BIT31 0x80000000

#define SETBIT(REG, BIT)   ((REG) |= (1UL << (BIT)))
#define CLRBIT(REG, BIT)   ((REG) &= ~(1UL << (BIT)))
#define GETBIT(REG, BIT)   (((REG) >> (BIT)) & 0x01UL)
#define COMPLEMENT(a)      (~(a))

#define _bit0_(val)                 ((bit)(val & BIT0))
#define _bit1_(val)                 ((bit)(val & BIT1))
#define _bit2_(val)                 ((bit)(val & BIT2))
#define _bit3_(val)                 ((bit)(val & BIT3))
#define _bit4_(val)                 ((bit)(val & BIT4))
#define _bit5_(val)                 ((bit)(val & BIT5))
#define _bit6_(val)                 ((bit)(val & BIT6))
#define _bit7_(val)                 ((bit)(val & BIT7))
#define _bit8_(val)                 ((bit)(val & BIT8))
#define _bit9_(val)                 ((bit)(val & BIT9))
#define _bit10_(val)                ((bit)(val & BIT10))
#define _bit11_(val)                ((bit)(val & BIT11))
#define _bit12_(val)                ((bit)(val & BIT12))
#define _bit13_(val)                ((bit)(val & BIT13))
#define _bit14_(val)                ((bit)(val & BIT14))
#define _bit15_(val)                ((bit)(val & BIT15))

#ifndef MAX
#define MAX(a, b)        (((a) > (b)) ? (a) : (b))
#endif
#ifndef MIN
#define MIN(a ,b)        (((a) < (b)) ? (a) : (b))
#endif

#define COUNTOF(array)    (sizeof(array) / sizeof((array)[0]))

#ifndef UNUSED 			//to avoid compile warnings...
#define UNUSED(var)     (void)(var)
#endif

#ifndef TRUE
#define TRUE          	1
#endif

#ifndef FALSE
#define FALSE          	0
#endif

#ifndef ENABLE
#define ENABLE          1
#endif

#ifndef DISABLE
#define DISABLE         0
#endif

#define FAILURE            (-1)
#define SUCCESS            (0)

#define CHV(x)  (x / 0x100),((x % 0x100) / 0x10),(x % 0x10) 
#define CHV2(x) (x / 0x1000),((x / 0x100) % 0x10),((x % 0x1000) % 0x100)

#endif//#ifndef __DATA_TYPE_H

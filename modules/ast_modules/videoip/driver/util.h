/*
 * Copyright (c) 2004-2012
 * ASPEED Technology Inc. All Rights Reserved
 * Proprietary and Confidential
 *
 * By using this code you acknowledge that you have signed and accepted
 * the terms of the ASPEED SDK license agreement.
 */


#ifndef _UTIL_H_
#define _UTIL_H_

#include <asm/arch/hardware.h>
#include <linux/types.h>    /* size_t */
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/kernel.h>   /* printk() */
#include <linux/slab.h>   /* kmalloc() */
#include <linux/types.h>    /* size_t */
#include <linux/mm.h>
#include <linux/kdev_t.h>
#include <asm/page.h>
#include <linux/delay.h>
#include <linux/tcp.h>
#include <linux/in.h>
#include <asm/atomic.h>
#include <linux/workqueue.h>

#include <asm/arch/platform.h>

#ifndef BIT0
#define BIT0  (1<<0)
#define BIT1  (1<<1)
#define BIT2  (1<<2)
#define BIT3  (1<<3)
#define BIT4  (1<<4)
#define BIT5  (1<<5)
#define BIT6  (1<<6)
#define BIT7  (1<<7)
#define BIT8  (1<<8)
#define BIT9  (1<<9)
#define BIT10 (1<<10)
#define BIT11 (1<<11)
#define BIT12 (1<<12)
#define BIT13 (1<<13)
#define BIT14 (1<<14)
#define BIT15 (1<<15)
#define BIT16 (1<<16)
#define BIT17 (1<<17)
#define BIT18 (1<<18)
#define BIT19 (1<<19)
#define BIT20 (1<<20)
#define BIT21 (1<<21)
#define BIT22 (1<<22)
#define BIT23 (1<<23)
#define BIT24 (1<<24)
#define BIT25 (1<<25)
#define BIT26 (1<<26)
#define BIT27 (1<<27)
#define BIT28 (1<<28)
#define BIT29 (1<<29)
#define BIT30 (1<<30)
#define BIT31 (1<<31)
#endif



#if 0
#define ReadMemoryLong(baseaddress,offset)        (*(volatile u32 *)(IO_ADDRESS((u32)(baseaddress)+(u32)(offset))))
#define WriteMemoryLong(baseaddress,offset,data)  *(volatile u32 *)(IO_ADDRESS((u32)(baseaddress)+(u32)(offset)))=(u32)(data)
//#define WriteMemoryBYTE(baseaddress,offset,data)  *(volatile u8 *)(IO_ADDRESS((u32)(baseaddress)+(u32)(offset))) = (u8)(data)    
#define WriteMemoryLongWithMASK(baseaddress, offset, data, mask)  *(volatile u32 *)(IO_ADDRESS((u32)(baseaddress)+(u32)(offset))) = (((*(volatile u32 *)(IO_ADDRESS((u32)(baseaddress)+(u32)(offset))))&(~(mask))) | (u32)((data)&(mask)))
#endif

#define __SetReg32(base, offset, data) \
	*(volatile u32 *)(IO_ADDRESS((u32)(base)+(u32)(offset)))=(u32)(data)

#define __GetReg32(base, offset) \
	(*(volatile u32 *)(IO_ADDRESS((u32)(base)+(u32)(offset))))

#define __ModReg32(base, offset, data, mask) \
	*(volatile u32 *)(IO_ADDRESS((u32)(base)+(u32)(offset))) = \
	(((*(volatile u32 *)(IO_ADDRESS((u32)(base)+(u32)(offset))))&(~(mask))) \
	| (u32)((data)&(mask)))

#define SetVideoReg(offset, data) __SetReg32(ASPEED_VIDEO_BASE, offset, data)
#define GetVideoReg(offset) __GetReg32(ASPEED_VIDEO_BASE, offset)
#define ModVideoReg(offset, data, mask) __ModReg32(ASPEED_VIDEO_BASE, offset, data, mask)
#define ClrVideoRegBits(offset, bit_mask) ModVideoReg(offset, 0, bit_mask)
#define SetVideoRegBits(offset, bit_mask) ModVideoReg(offset, bit_mask, bit_mask)

#define SetSCUReg(offset, data) __SetReg32(ASPEED_APB1_BASE, offset, data)
#define GetSCUReg(offset) __GetReg32(ASPEED_APB1_BASE, offset)
#define ModSCUReg(offset, data, mask) __ModReg32(ASPEED_APB1_BASE, offset, data, mask)

#endif /* _UTIL_H_ */


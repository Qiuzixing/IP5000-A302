/*
 * Copyright (c) 2004-2012
 * ASPEED Technology Inc. All Rights Reserved
 * Proprietary and Confidential
 *
 * By using this code you acknowledge that you have signed and accepted
 * the terms of the ASPEED SDK license agreement.
 */

#ifndef _I2S_TYPES_H_
#define _I2S_TYPES_H_

#include <linux/types.h>

#define    ULONG                    unsigned long
#define    USHORT                   unsigned short
#define    UCHAR                    unsigned char
#define    BYTE                     unsigned char
#define    CHAR                     char
#define    LONG                     long
#define    PUCHAR                   UCHAR *
#define    PULONG                   ULONG *

#define I2S_err(fmt, args...)						\
	do {								\
		printk(KERN_ERR "%-10s: ***I2S ERROR*** (%s,%d) %s: " fmt,	\
			(in_interrupt() ? "interrupt" : (current)->comm),\
			__FILE__, __LINE__, __FUNCTION__ , ##args);	\
	} while(0)
	
#define I2S_info(fmt, args...)					\
	do {							\
		printk(KERN_INFO "I2S: " fmt , ## args);	\
	} while(0)
	
#endif

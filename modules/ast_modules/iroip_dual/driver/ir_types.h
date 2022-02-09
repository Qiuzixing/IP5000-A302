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

#include <linux/module.h>
#include <linux/types.h>

#define    ULONG                    unsigned long
#define    USHORT                   unsigned short
#define    UCHAR                    unsigned char
#define    BYTE                     unsigned char
#define    CHAR                     char
#define    LONG                     long
#define    PUCHAR                   UCHAR *
#define    PULONG                   ULONG *

#define IR_err(fmt, args...)						\
	do {								\
		printk(KERN_ERR "%-10s: ***IR ERROR*** %s(%d): " fmt,	\
			(in_interrupt() ? "interrupt" : (current)->comm),\
			__FUNCTION__, __LINE__, ##args);	\
	} while(0)
	
#define IR_info(fmt, args...)					\
	do {							\
		printk(KERN_INFO "IR: " fmt , ## args);	\
	} while(0)


static inline void udump(void *buf, int bufflen)
{
    int i;
    unsigned char *buff= buf;

    printk("dump 0x%08x: %d bytes\n", (u32)(buff), bufflen);

    if (bufflen > 768) {
        for (i = 0; i< 768; i++) {
            if (i%16 == 0)
                printk("   ");
            printk("%02x ", (unsigned char ) buff[i]);
            if (i%4 == 3) printk("| ");
            if (i%16 == 15) printk("\n");
        }
        printk("... (%d byte)\n", bufflen);
        return;
    }

    for (i = 0; i< bufflen; i++) {
        if (i%16 == 0)
            printk("   ");
        printk("%02x ", (unsigned char ) buff[i]);
        if (i%4 == 3)
            printk("| ");
        if (i%16 == 15)
            printk("\n");
    }
    printk("\n");

}



#endif

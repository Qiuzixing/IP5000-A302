/*
 * Copyright (c) 2004-2014
 * ASPEED Technology Inc. All Rights Reserved
 * Proprietary and Confidential
 *
 * By using this code you acknowledge that you have signed and accepted
 * the terms of the ASPEED SDK license agreement.
 */

#ifndef _AST_UTILITIES_H_
#define _AST_UTILITIES_H_

#include <linux/kernel.h> /*printk*/
#include <linux/hardirq.h> /* in_interrupt */
#include <linux/kthread.h> /* current */
#include <linux/delay.h> /* msleep() */

#define DelayMS(ms) do { msleep(ms); } while(0)


/**
 * udbg - print debug messages if CONFIG_VIDEO_DEBUG is defined
 * @fmt:
 * @args:
 */
#if 1

#define udbg(fmt, args...)						\
	do {								\
		printk(KERN_ERR "%-10s: %s(%d): " fmt,		\
			(in_interrupt() ? "interrupt" : (current)->comm),\
			__FUNCTION__ , __LINE__, ##args);	\
	} while(0)

#else

#define udbg(fmt, args...)		do{ }while(0)

#endif

/**
 * uerr - print error messages
 * @fmt:
 * @args:
 */
#define uerr(fmt, args...)						\
	do {								\
		printk(KERN_ERR "%-10s: ***ERROR*** %s(%d): " fmt,	\
			(in_interrupt() ? "interrupt" : (current)->comm),\
			__FUNCTION__, __LINE__,  ##args);	\
	} while(0)

/**
 * uinfo - print information messages
 * @fmt:
 * @args:
 */
#define uinfo(fmt, args...)					\
	do {							\
		printk(KERN_INFO "SiI9136: " fmt , ## args);	\
	} while(0)

static inline void udump(void *buf, int bufflen)
{
    int i;
    unsigned char *buff= buf;

    printk("dump 0x%08x: %d bytes\n", (u32)(buff), bufflen);

    if (0 /*bufflen > 768*/) {
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

static inline int udump_s(void *buf, int bufflen, unsigned char *str)
{
    int i;
    unsigned char *buff= buf;

    //sprintf(str+strlen(str), "dump 0x%08x: %d bytes\n", (u32)(buff), bufflen);

    if (bufflen > 768) {
        for (i = 0; i< 768; i++) {
            if (i%16 == 0)
                sprintf(str+strlen(str), "   ");
            sprintf(str+strlen(str), "%02x ", (unsigned char ) buff[i]);
            if (i%4 == 3) sprintf(str+strlen(str), "| ");
            if (i%16 == 15) sprintf(str+strlen(str), "\n");
        }
        sprintf(str+strlen(str), "... (%d byte)\n", bufflen);
        return strlen(str);
    }

    for (i = 0; i< bufflen; i++) {
        if (i%16 == 0)
            sprintf(str+strlen(str), "   ");
        sprintf(str+strlen(str), "%02x ", (unsigned char ) buff[i]);
        if (i%4 == 3)
            sprintf(str+strlen(str), "| ");
        if (i%16 == 15)
            sprintf(str+strlen(str), "\n");
    }
    sprintf(str+strlen(str), "\n");
    return strlen(str);
}

#endif /* _AST_UTILITIES_H_ */


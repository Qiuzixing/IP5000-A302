/*
 * Copyright (c) 2004-2012
 * ASPEED Technology Inc. All Rights Reserved
 * Proprietary and Confidential
 *
 * By using this code you acknowledge that you have signed and accepted
 * the terms of the ASPEED SDK license agreement.
 */

#ifndef _AST_DEBUG_H_
#define _AST_DEBUG_H_

#include <linux/kernel.h> /*printk*/
#include <linux/hardirq.h> /* in_interrupt */
#include <linux/kthread.h> /* current */

//#include "global.h"

/**
 * udbg - print debug messages if CONFIG_VIDEO_DEBUG is defined
 * @fmt:
 * @args:
 */
#ifdef CONFIG_VIDEO_DEBUG

#define udbg(fmt, args...)						\
	do {								\
		printk(KERN_ERR "%-10s:(%s,%d) %s: " fmt,		\
			(in_interrupt() ? "interrupt" : (current)->comm),\
			__FILE__, __LINE__, __FUNCTION__ , ##args);	\
	} while(0)

#else  /* CONFIG_VIDEO_DEBUG */

#define udbg(fmt, args...)		do{ }while(0)

#endif /* CONFIG_VIDEO_DEBUG */

enum {
	videoip_debug_eh			= (1 << 0),
	videoip_debug_xmit			= (1 << 1),
	videoip_debug_mode			= (1 << 2),
	videoip_debug_v1txhost		= (1 << 3),
	videoip_debug_v1rxclient	= (1 << 4),

	videoip_debug_1080i         = (1 << 5),
	//performance analyze
	videoip_debug_bitrate       = (1 << 16),
	videoip_debug_encoderate    = (1 << 17),
	videoip_debug_poll_hitrate    = (1 << 18),
	videoip_debug_framesize	  = (1 << 19),
	videoip_debug_dynamic_quality   = (1 << 20),
	videoip_debug_run	= (1 << 21),
	videoip_debug_stream_err	= (1 << 22),
	videoip_debug_test	= (1 << 23),
	//HDCP debug
	videoip_debug_hdcp1_mask	= (0x3 << 24),
	videoip_debug_hdcp2_mask	= (0x3 << 26),
	videoip_debug_ignore_hdcp   = (0x1UL << 31),
#define HDCP2_DBG_LEVEL0	(0x0 << 26)
#define HDCP2_DBG_LEVEL1	(0x1 << 26)
#define HDCP2_DBG_LEVEL2	(0x2 << 26)
#define HDCP2_DBG_LEVEL3	(0x3 << 26)
};

extern unsigned long videoip_debug_flag;
//#define dbg_flag_xmit		(videoip_debug_flag & videoip_debug_xmit)
#define dbg_flag_bitrate		(videoip_debug_flag & videoip_debug_bitrate)
#define dbg_flag_encoderate         (videoip_debug_flag & videoip_debug_encoderate)
//enable: will display 1080i in 1920X1080, default: is in 1280X720
#define dbg_flag_1080i              (videoip_debug_flag & videoip_debug_1080i)
#define dbg_flag_poll_hitrate         (videoip_debug_flag & videoip_debug_poll_hitrate)
#define dbg_flag_framesize         (videoip_debug_flag & videoip_debug_framesize)
#define dbg_flag_dynamic_quality         (videoip_debug_flag & videoip_debug_dynamic_quality)
#define dbg_flag_run         (videoip_debug_flag & videoip_debug_run)

#define dbg_flag_xmit		(videoip_debug_flag & 0)
#define dbg_flag_stream_err	 (videoip_debug_flag & videoip_debug_stream_err)
#define dbg_flag_test	 (videoip_debug_flag & videoip_debug_test)
#define dbg_flag_no_hdcp (videoip_debug_flag & videoip_debug_ignore_hdcp)

#define dbg_with_flag(flag, fmt, args...)		\
	do {						\
		if(flag & videoip_debug_flag)		\
			udbg(fmt , ##args);		\
	} while(0)


#define dbg_eh(fmt, args...)			dbg_with_flag(videoip_debug_eh,   fmt , ##args)
#define dbg_xmit(fmt, args...)			dbg_with_flag(videoip_debug_xmit,  fmt , ##args)
//#define dbg_mode(fmt, args...)		dbg_with_flag(videoip_debug_mode,  fmt , ##args)
#define dbg_mode(fmt, args...)			do{ }while(0)
#define dbg_v1txhost(fmt, args...)		dbg_with_flag(videoip_debug_v1txhost,  fmt , ##args)
#define dbg_v1rxclient(fmt, args...)	dbg_with_flag(videoip_debug_v1rxclient,  fmt , ##args)
#define dbg_hdcp1(fmt, args...)		\
	do {						\
		if(flag & videoip_debug_hdcp1)		\
			printk(fmt , ##args);		\
	} while(0)
#define dbg_hdcp2_level1(fmt, args...)		\
	do {						\
		if((videoip_debug_flag & videoip_debug_hdcp2_mask) >= HDCP2_DBG_LEVEL1) 	\
			printk(fmt , ##args);		\
	} while(0)
#define dbg_hdcp2_level2(fmt, args...)		\
	do {						\
		if((videoip_debug_flag & videoip_debug_hdcp2_mask) >= HDCP2_DBG_LEVEL2) 	\
			printk(fmt , ##args);		\
	} while(0)
#define dbg_hdcp2_level3(fmt, args...)		\
	do {						\
		if((videoip_debug_flag & videoip_debug_hdcp2_mask) >= HDCP2_DBG_LEVEL3) 	\
			printk(fmt , ##args);		\
	} while(0)

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
		printk(KERN_INFO "VideoIP: " fmt , ## args);	\
	} while(0)

#define uinfoINT(fmt, args...)					\
	do {							\
		printk(KERN_INFO "INT: " fmt , ## args);	\
	} while(0)

#define uinfoMode(fmt, args...)					\
	do {							\
		printk(KERN_INFO "Mode: " fmt , ## args);	\
	} while(0)

#define uinfo_rl(fmt, args...)					\
	do {							\
		if (printk_ratelimit())				\
			uinfo(fmt, ##args);			\
	} while(0)

#define info_with_flag(flag, fmt, args...)		\
	do {						\
		if(flag & videoip_debug_flag)		\
			uinfo(fmt , ##args);		\
	} while(0)

#define info_eh(fmt, args...)			info_with_flag(videoip_debug_eh,   fmt , ##args)
//#define info_xmit(fmt, args...)			info_with_flag(videoip_debug_xmit,  fmt , ##args)
//#define dbg_mode(fmt, args...)		dbg_with_flag(videoip_debug_mode,  fmt , ##args)
#define info_mode(fmt, args...)			do{ }while(0)
#define info_v1txhost(fmt, args...)		info_with_flag(videoip_debug_v1txhost,  fmt , ##args)
#define info_v1rxclient(fmt, args...)	info_with_flag(videoip_debug_v1rxclient,  fmt , ##args)

#define DEBUGVIDEO1REG \
    PrintVideoReg(0x0, 0xac)

#if (CONFIG_AST1500_SOC_VER == 1)
#define DEBUGVIDEO2REG \
    PrintVideoReg(0x100, 0x1ac)

#define DEBUGVIDEOMREG \
    PrintVideoReg(0x200, 0x2ac)
#endif

#define DEBUGVIDEOGREG \
    PrintVideoReg(0x300, 0x328)

#define dump_ve() \
	do { \
		int __i; \
		printk("\nVE registers:"); \
		for (__i = 0; __i < 0xF0/*0x400*/; __i+=4) \
		{ \
			if ((__i % 16) == 0) \
				printk("\n%08X:", __i); \
			printk(" %08X", GetVideoReg(__i)); \
		} \
	} while (0)

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

#define HDCP2_TEST_VECTORS	0

#endif /* _AST_DEBUG_H_ */


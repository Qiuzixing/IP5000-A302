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

/**
 * udbg - print debug messages if CONFIG_VIDEO_DEBUG is defined
 * @fmt:
 * @args:
 */
#define udbg(fmt, args...)						\
	do {								\
		printk(KERN_ERR "%-10s: %s(%d): " fmt,		\
			(in_interrupt() ? "interrupt" : (current)->comm),\
			__FUNCTION__ , __LINE__, ##args);	\
	} while (0)

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
	} while (0)

/**
 * uinfo - print information messages
 * @fmt:
 * @args:
 */
#define uinfo(fmt, args...) do { printk(KERN_INFO "sii9679: " fmt , ## args); } while (0);
#endif /* _AST_UTILITIES_H_ */


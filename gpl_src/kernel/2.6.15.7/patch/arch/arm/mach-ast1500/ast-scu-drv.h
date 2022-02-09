/*
 *
 * Copyright (C) 2012-2020  ASPEED Technology Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 *   History      :
 *    1. 2013/04/04 Bruce first version
 *
********************************************************************************/

#ifndef _AST_SCU_DRV_H_
#define _AST_SCU_DRV_H_

#include <asm/hardware.h>
#include <asm/io.h>
#include <linux/platform_device.h>

#include <asm/arch/ast-scu.h>


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
		printk(KERN_INFO "SCU: " fmt , ## args);	\
	} while(0)


#define SET_SCU_REG(offset, value)        iowrite32(cpu_to_le32(value), ast_scu.iobase + (offset))
#define GET_SCU_REG(offset)               (le32_to_cpu(ioread32(ast_scu.iobase + (offset))))
#define MOD_SCU_REG(offset, value, mask) \
	do { \
		u32 _r; \
		_r = GET_SCU_REG((offset)); \
		_r &= ~(mask); \
		_r |= ((value) & (mask)); \
		SET_SCU_REG((offset), _r); \
	} while (0)
#define SET_SCU_REG_BITS(offset, mask) MOD_SCU_REG(offset, mask, mask)
#define CLR_SCU_REG_BITS(offset, mask) MOD_SCU_REG(offset, 0, mask)

#define SCU_SW_LOCK() \
	do { \
		spin_lock_irqsave(&ast_scu.lock, ast_scu.lock_flags); \
	} while (0)

#define SCU_SW_UNLOCK() \
	do { \
		spin_unlock_irqrestore(&ast_scu.lock, ast_scu.lock_flags); \
	} while (0)


static inline void SCU_START(void)
{
	SCU_SW_LOCK();
	ast_scu.scu_op(SCUOP_SCU_LOCK, (void*)0);
}

static inline void SCU_END(void)
{
	ast_scu.scu_op(SCUOP_SCU_LOCK, (void*)1);
	SCU_SW_UNLOCK();
}


#endif //#ifndef _AST_SCU_DRV_H_

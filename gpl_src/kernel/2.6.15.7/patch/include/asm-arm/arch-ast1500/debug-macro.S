/* linux/include/asm-arm/arch-gucmvp2000/debug-macro.S
 *
 * Debugging macro include header
 *
 *  Copyright (C) 1994-1999 Russell King
 *  Moved from linux/arch/arm/kernel/debug.S by Ben Dooks
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
*/
		.macro	addruart,rx
		mrc	p15, 0, \rx, c1, c0
		tst	\rx, #1			@ MMU enabled?
		moveq	\rx, #IO_START   	@ physical base address
		movne	\rx, #IO_BASE   	@ virtual base
		addne	\rx, \rx, #IO_START >> 4
		.endm

#include <asm/hardware/mvp2_serial.h>

		.macro	senduart,rd,rx
		strb	\rd, [\rx, #UART_RBR]
		.endm

		.macro	waituart,rd,rx
1001:		ldr	\rd, [\rx, #UART_LSR]
		tst	\rd, #UART_LSR_THRE
		bne	1001b
		.endm

		.macro	busyuart,rd,rx
1001:		ldr	\rd, [\rx, #UART_LSR]
		tst	\rd, #UART_LSR_TEMT
		bne	1001b
		.endm

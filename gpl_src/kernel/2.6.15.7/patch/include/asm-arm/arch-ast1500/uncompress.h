/*
 *  linux/include/asm-arm/arch-aaec2000/uncompress.h
 *
 *  Copyright (c) 2005 Nicolas Bellido Y Ortega
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 */

#ifndef __ASM_ARCH_UNCOMPRESS_H
#define __ASM_ARCH_UNCOMPRESS_H

#include "hardware.h"
#include <asm/io.h>
#include <asm/hardware/aspeed_serial.h>

#define UART_GET_CHAR()		__raw_readl(serial_port + UART_RBR)
#define UART_PUT_CHAR(v)	__raw_writel((v), serial_port + UART_THR)
#define UART_GET_DLL()		__raw_readl(serial_port + UART_DLL)
#define UART_PUT_DLL(v)		__raw_writel((v), serial_port + UART_DLL)
#define UART_GET_DLH()		__raw_readl(serial_port + UART_DLH)
#define UART_PUT_DLH(v)		__raw_writel((v), serial_port + UART_DLH)
#define UART_GET_LCR()		__raw_readl(serial_port + UART_LCR)
#define UART_PUT_LCR(v)		__raw_writel((v), serial_port + UART_LCR)
#define UART_GET_LSR()		__raw_readl(serial_port + UART_LSR)
#define UART_GET_REG(x)		__raw_readl(serial_port + x)

static void putc(int c)
{
	static unsigned long serial_port;
	static int initialed = 0;

	if(!initialed)
	{
		do {
			int idx;
			// MMU is not enable yet
			serial_port = ASPEED_UART_DBG_BASE;
			for(idx=0; idx<32; idx+=4)
				if (UART_GET_REG(idx)!=0x0 && UART_GET_REG(idx)!=0xff)
					goto found;

			return;
		} while (0);
found:

		// Setup console 8,n,1
		{
			unsigned long div = ((TICKS_PER_uSEC*1000000L)/(16*115200));  // nPClk/(16*nBaudRate)
			UART_PUT_LCR(0x9b);
			UART_PUT_DLL(div&0xff);
			UART_PUT_DLH((div>>8)&0xff);
			UART_PUT_LCR(0x03);
		}
		initialed = 1;
	}

	/* wait for space in the UART's transmitter */
	while (!(UART_GET_LSR() & UART_LSR_THRE))
		barrier();

	/* send the character out. */
	UART_PUT_CHAR(c);
}

static inline void flush(void)
{
}

#define arch_decomp_setup()
#define arch_decomp_wdog()

#endif /* __ASM_ARCH_UNCOMPRESS_H */

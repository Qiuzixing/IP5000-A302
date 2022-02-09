/*
 *  linux/include/asm-arm/arch-mvp2000/system.h
 *
 *  Copyright (C) 1999 ARM Limited
 *  Copyright (C) 2000 Deep Blue Solutions Ltd
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#ifndef __ASM_ARCH_SYSTEM_H
#define __ASM_ARCH_SYSTEM_H

#include <asm/hardware.h>
#include <asm/io.h>
#include <asm/arch/platform.h>

static inline void arch_idle(void)
{
	/*
	 * This should do all the clock switching
	 * and wait for interrupt tricks
	 */
	cpu_do_idle();
}

static inline void arch_reset(char mode)
{
	if (mode)//something is wrong
	{
		//clear scratch register Bit 6 to enable ddr initialization during POST
		IO_WRITE((IO_ADDRESS(ASPEED_SCU_BASE))+0x40, IO_READ((IO_ADDRESS(ASPEED_SCU_BASE))+0x40) & 0xFFFFFFBF);
	}

	/*
	 * Use WDT to restart system
	 */
#if 0
	IO_WRITE(ASPEED_WDT_VA_BASE+0x04, 0x10);
#else//restart system ASAP
	//disable WDT first
	IO_WRITE(ASPEED_WDT_VA_BASE+0x0c, IO_READ(ASPEED_WDT_VA_BASE+0x0c) & 0xFFFFFFFE);
	IO_WRITE(ASPEED_WDT_VA_BASE+0x04, 1);
#endif
	IO_WRITE(ASPEED_WDT_VA_BASE+0x08, 0x4755);
#if 0
	IO_WRITE(ASPEED_WDT_VA_BASE+0x0c, 0x03);
#else
	IO_WRITE(ASPEED_WDT_VA_BASE+0x0c, IO_READ(ASPEED_WDT_VA_BASE+0x0c) | 0x03);
#endif
}

#endif

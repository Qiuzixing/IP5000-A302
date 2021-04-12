/*
 *  linux/include/asm-arm/arch-mvp2000/hardware.h
 *
 *  This file contains the hardware definitions of the MVP-2000.
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
#ifndef __ASM_ARCH_HARDWARE_H
#define __ASM_ARCH_HARDWARE_H

#include <asm/sizes.h>
#include <asm/arch/platform.h>

#if (CONFIG_AST1500_SOC_VER >= 2)
	#define PCIBIOS_MIN_IO			0x70010000
	#define PCIBIOS_MIN_MEM			0x70020000
#else
	#define PCIBIOS_MIN_IO			0x60010000
	#define PCIBIOS_MIN_MEM			0x68000000
#endif

#define pcibios_assign_all_busses()	1

/*
 * Where in virtual memory the IO devices (timers, system controllers
 * and so on)
 */
#define IO_BASE			0xF0000000                 // VA of IO
#define IO_BASE2		0xE0000000                 // VA of PCI 
#define IO_SIZE			ASPEED_IO_SIZE             // How much?
#define IO_START		ASPEED_IO_START            // PA of IO

/* macro to get at IO space when running virtually */
//#define IO_ADDRESS(x) (((x) >> 4) + IO_BASE) 
#define IO_ADDRESS(x)  (x - 0x10000000 + IO_BASE) 
#if (CONFIG_AST1500_SOC_VER >= 2)
	#define IO_ADDRESS2(x) (x - 0x70000000 + IO_BASE2) 
#else
	#define IO_ADDRESS2(x) (x - 0x60000000 + IO_BASE2) 
#endif

#ifndef __ASSEMBLY__
/* external functions for GPIO support *
* These allow various different clients to access the same GPIO
* registers without conflicting. If your driver only owns the entire
* GPIO register, then it is safe to ioremap/__raw_{read|write} to it.
*/
/* ast1500_gpio_cfgpin *
* set the configuration of the given pin to the value passed. *
* eg:
*    ast1500_gpio_cfgpin(AST1500_GPA0, AST1500_GPA0_OUT);
*/
extern void ast1500_gpio_cfgpin(unsigned int pin, unsigned int function);
extern unsigned int ast1500_gpio_getcfg(unsigned int pin);
extern void ast1500_gpio_setpin(unsigned int pin, unsigned int to);
extern unsigned int ast1500_gpio_getpin(unsigned int pin);
extern unsigned int ast1500_gpio_get_int_stat(unsigned int pin);
extern void ast1500_gpio_ack_int_stat(unsigned int pin);

#endif /* __ASSEMBLY__ */




#endif


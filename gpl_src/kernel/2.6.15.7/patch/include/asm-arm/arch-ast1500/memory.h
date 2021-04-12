/*
 *  linux/include/asm-arm/arch-mvp2000/memory.h
 *
 *  Copyright (C) 1999 ARM Limited
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
#ifndef __ASM_ARCH_MEMORY_H
#define __ASM_ARCH_MEMORY_H


/*
 * Physical DRAM offset.
 */
#if (CONFIG_AST1500_SOC_DRAM_MAPPING_TYPE == 2)
	#define PHYS_OFFSET	UL(0x80000000)
	#define BUS_OFFSET	UL(0x80000000)
#else
	#define PHYS_OFFSET	UL(0x40000000)
	#define BUS_OFFSET	UL(0x40000000)
#endif

#if defined(CONFIG_ARCH_AST1500_CLIENT) && defined(CONFIG_PCI)
#define arch_adjust_zones(node, size, holes) \
	ast1500_adjust_zones(node, size, holes)
#endif
	
#define ISA_DMA_THRESHOLD UL(0x00ffffff)

/*
 * Virtual view <-> DMA view memory address translations
 * virt_to_bus: Used to translate the virtual address to an
 *              address suitable to be passed to set_dma_addr
 * bus_to_virt: Used to convert an address for DMA operations
 *              to an address that the kernel can use.
 */
#define __virt_to_bus(x)	(x - PAGE_OFFSET + BUS_OFFSET)
#define __bus_to_virt(x)	(x - BUS_OFFSET + PAGE_OFFSET)

#endif

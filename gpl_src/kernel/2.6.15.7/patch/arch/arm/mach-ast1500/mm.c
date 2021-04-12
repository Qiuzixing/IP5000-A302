/*
 *  linux/arch/arm/mach-ast2000/mm.c
 *
 *  Copyright (C) 1999,2000 Arm Limited
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
#include <linux/sched.h>
#include <linux/mm.h>
#include <linux/init.h>

#include <asm/hardware.h>
#include <asm/io.h>
#include <asm/pgtable.h>
#include <asm/page.h>

#include <asm/mach/map.h>

static struct map_desc aspeed_io_desc[] __initdata =
{

 { IO_ADDRESS(ASPEED_AHB_CONTROLLER_BASE),	__phys_to_pfn(ASPEED_AHB_CONTROLLER_BASE),	SZ_64K,		MT_DEVICE},
 { IO_ADDRESS(ASPEED_MAC1_BASE),		__phys_to_pfn(ASPEED_MAC1_BASE),		SZ_64K,		MT_DEVICE},
 { IO_ADDRESS(ASPEED_MAC2_BASE),		__phys_to_pfn(ASPEED_MAC2_BASE),		SZ_64K,		MT_DEVICE},
 { IO_ADDRESS(ASPEED_USB20_BASE),		__phys_to_pfn(ASPEED_USB20_BASE),		SZ_4K,		MT_DEVICE},
#if (CONFIG_AST1500_SOC_VER >= 3)
 { IO_ADDRESS(ASPEED_USB20HC1_BASE),	__phys_to_pfn(ASPEED_USB20HC1_BASE),	SZ_4K,		MT_DEVICE},
 { IO_ADDRESS(ASPEED_USB20HC2_BASE),	__phys_to_pfn(ASPEED_USB20HC2_BASE),	SZ_4K,		MT_DEVICE},
#endif
 { IO_ADDRESS(ASPEED_VIC_BASE),			__phys_to_pfn(ASPEED_VIC_BASE),			SZ_64K,		MT_DEVICE},
// { IO_ADDRESS(ASPEED_APB1_BASE),		__phys_to_pfn(ASPEED_APB1_BASE),		SZ_64K,		MT_DEVICE},
 { IO_ADDRESS(ASPEED_MMC_BASE),			__phys_to_pfn(ASPEED_MMC_BASE),			SZ_4K,		MT_DEVICE},
 { IO_ADDRESS(ASPEED_USB11_BASE),		__phys_to_pfn(ASPEED_USB11_BASE),		SZ_4K,		MT_DEVICE},
 { IO_ADDRESS(ASPEED_SCU_BASE),			__phys_to_pfn(ASPEED_SCU_BASE),			SZ_4K,		MT_DEVICE},
 { IO_ADDRESS(ASPEED_CRYPTO_BASE),		__phys_to_pfn(ASPEED_CRYPTO_BASE),		SZ_4K,		MT_DEVICE},
 { IO_ADDRESS(ASPEED_ADPCM_BASE),		__phys_to_pfn(ASPEED_ADPCM_BASE),		SZ_4K,		MT_DEVICE},
 { IO_ADDRESS(ASPEED_I2S_BASE),			__phys_to_pfn(ASPEED_I2S_BASE),			SZ_4K,		MT_DEVICE},
 { IO_ADDRESS(ASPEED_GRAPHIC_BASE),		__phys_to_pfn(ASPEED_GRAPHIC_BASE),		SZ_4K,		MT_DEVICE},
#if (CONFIG_AST1500_SOC_VER == 2)
 { IO_ADDRESS(ASPEED_ADC_BASE),			__phys_to_pfn(ASPEED_ADC_BASE),			SZ_4K,		MT_DEVICE},
 { IO_ADDRESS(ASPEED_PS2_BASE),			__phys_to_pfn(ASPEED_PS2_BASE),			SZ_4K,		MT_DEVICE},
#endif
 { IO_ADDRESS(ASPEED_VIDEO_BASE),		__phys_to_pfn(ASPEED_VIDEO_BASE),		SZ_64K,		MT_DEVICE},
 { IO_ADDRESS(ASPEED_AHB_TO_PBUS_BASE),		__phys_to_pfn(ASPEED_AHB_TO_PBUS_BASE),		SZ_64K,		MT_DEVICE},
 { IO_ADDRESS(ASPEED_MDMA_BASE),		__phys_to_pfn(ASPEED_MDMA_BASE),		SZ_64K,		MT_DEVICE},
 { IO_ADDRESS(ASPEED_2D_BASE),			__phys_to_pfn(ASPEED_2D_BASE),			SZ_64K,		MT_DEVICE},
// { IO_ADDRESS(ASPEED_APB2_BASE),		__phys_to_pfn(ASPEED_APB2_BASE),		SZ_64K,		MT_DEVICE},
 { IO_ADDRESS(ASPEED_GPIO_BASE),		__phys_to_pfn(ASPEED_GPIO_BASE),		SZ_4K,		MT_DEVICE},
 { IO_ADDRESS(ASPEED_RTC_BASE),			__phys_to_pfn(ASPEED_RTC_BASE),			SZ_4K,		MT_DEVICE},
 { IO_ADDRESS(ASPEED_TIMER_BASE),		__phys_to_pfn(ASPEED_TIMER_BASE),		SZ_4K,		MT_DEVICE},
 { IO_ADDRESS(ASPEED_UART1_BASE),		__phys_to_pfn(ASPEED_UART1_BASE),		SZ_4K,		MT_DEVICE},
 { IO_ADDRESS(ASPEED_UART2_BASE),		__phys_to_pfn(ASPEED_UART2_BASE),		SZ_4K,		MT_DEVICE},
#if (CONFIG_AST1500_SOC_VER >= 2)
 { IO_ADDRESS(ASPEED_UART3_BASE),		__phys_to_pfn(ASPEED_UART3_BASE),		SZ_4K,		MT_DEVICE},
 { IO_ADDRESS(ASPEED_UART4_BASE),		__phys_to_pfn(ASPEED_UART4_BASE),		SZ_4K,		MT_DEVICE},
 { IO_ADDRESS(ASPEED_UART5_BASE),		__phys_to_pfn(ASPEED_UART5_BASE),		SZ_4K,		MT_DEVICE},
#endif
 { IO_ADDRESS(ASPEED_WDT_BASE),			__phys_to_pfn(ASPEED_WDT_BASE),			SZ_4K,		MT_DEVICE},
 { IO_ADDRESS(ASPEED_PWM_BASE),			__phys_to_pfn(ASPEED_PWM_BASE),			SZ_4K,		MT_DEVICE},
 { IO_ADDRESS(ASPEED_VUART0_BASE),		__phys_to_pfn(ASPEED_VUART0_BASE),		SZ_4K,		MT_DEVICE},
 { IO_ADDRESS(ASPEED_VUART1_BASE),		__phys_to_pfn(ASPEED_VUART1_BASE),		SZ_4K,		MT_DEVICE},
 { IO_ADDRESS(ASPEED_LPC_BASE),			__phys_to_pfn(ASPEED_LPC_BASE),			SZ_4K,		MT_DEVICE},
 { IO_ADDRESS(ASPEED_I2C_BASE),			__phys_to_pfn(ASPEED_I2C_BASE),			SZ_4K,		MT_DEVICE},
 { IO_ADDRESS(ASPEED_PECI_BASE),		__phys_to_pfn(ASPEED_PECI_BASE),		SZ_4K,		MT_DEVICE},
#if (CONFIG_AST1500_SOC_VER <= 2)
 { IO_ADDRESS(ASPEED_PCIARBITER_BASE),	__phys_to_pfn(ASPEED_PCIARBITER_BASE),	SZ_4K,		MT_DEVICE},
 { IO_ADDRESS2(ASPEED_PCI_MASTER1_BASE),	__phys_to_pfn(ASPEED_PCI_MASTER1_BASE),		SZ_256M,	MT_DEVICE},
#endif
};

void __init aspeed_map_io(void)
{
	iotable_init(aspeed_io_desc, ARRAY_SIZE(aspeed_io_desc));
}

/*
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

#ifndef __address_h
#define __address_h                     1

#ifndef __ASSEMBLY__
extern void ast_notify_user(const char *name);
#endif //__ASSEMBLY__

/* Base Address */
#define ASPEED_SMC_CONTROLLER_BASE_V1      0x16000000	/* SMC CONTROLLER */
#define ASPEED_SMC_CONTROLLER_IO_RANGE_V1      0x2000000
#define ASPEED_SMC_FLASH_BASE_V1 0x14000000
#define ASPEED_SMC_CONTROLLER_BASE_V2      0x1E620000	/* SMC CONTROLLER */
#define ASPEED_SMC_CONTROLLER_IO_RANGE_V2      0x10000
#define ASPEED_SMC_FLASH_BASE_V2 0x20000000

#if (CONFIG_AST1500_SOC_FLASH_MAPPING_TYPE == 2)
#define ASPEED_SMC_CONTROLLER_BASE      ASPEED_SMC_CONTROLLER_BASE_V2	/* SMC CONTROLLER */
#define ASPEED_SMC_CONTROLLER_IO_RANGE      ASPEED_SMC_CONTROLLER_IO_RANGE_V2
#define ASPEED_SMC_FLASH_BASE ASPEED_SMC_FLASH_BASE_V2
#else
#define ASPEED_SMC_CONTROLLER_BASE      ASPEED_SMC_CONTROLLER_BASE_V1	/* SMC CONTROLLER */
#define ASPEED_SMC_CONTROLLER_IO_RANGE      ASPEED_SMC_CONTROLLER_IO_RANGE_V1
#define ASPEED_SMC_FLASH_BASE ASPEED_SMC_FLASH_BASE_V1
#endif
#define ASPEED_SMC_FLASH_SIZE  0x8000000 /* Mapp 128MB */
#define ASPEED_AHB_CONTROLLER_BASE      0x1E600000	/* AHB CONTROLLER */
#define ASPEED_MAC1_BASE                0x1E660000      /* MAC1 */
#define ASPEED_MAC2_BASE                0x1E680000      /* MAC2 */
#define ASPEED_USB20_BASE               0x1E6A0000	/* USB 2.0 */
#if (CONFIG_AST1500_SOC_VER >= 3)
#define ASPEED_USB20HC1_BASE            0x1E6A1000	/* USB 2.0 host controller #1 */
#define ASPEED_USB20HC2_BASE            0x1E6A3000	/* USB 2.0 host controller #2 */
#endif
#define ASPEED_VIC_BASE                 0x1E6C0000	/* VIC */
#define ASPEED_APB1_BASE                0x1E6E0000	/* APB BRIDGE 1 */
#define ASPEED_MMC_BASE                	0x1E6E0000	/* MMC */
#define ASPEED_USB11_BASE              	0x1E6E1000	/* USB11 */
#define ASPEED_SCU_BASE                	0x1E6E2000	/* SCU */
#define ASPEED_CRYPTO_BASE              0x1E6E3000	/* Crypto */
#define ASPEED_ADPCM_BASE               0x1E6E4000	/* ADPCM */
#define ASPEED_I2S_BASE                	0x1E6E5000	/* I2S */
#define ASPEED_GRAPHIC_BASE             0x1E6E6000	/* Graphics */
#if (CONFIG_AST1500_SOC_VER == 2)
#define ASPEED_ADC_BASE                 0x1E6E9000	/* ADC controller */
#define ASPEED_PS2_BASE                 0x1E6EA000	/* PS2 controller */
#endif
#define ASPEED_VIDEO_BASE               0x1E700000	/* VIDEO ENGINE */
#define ASPEED_AHB_TO_PBUS_BASE         0x1E720000	/* APB -> PBUS */
#define ASPEED_MDMA_BASE                0x1E740000	/* MDMA */
#define ASPEED_2D_BASE                  0x1E760000	/* 2D */
#define ASPEED_APB2_BASE                0x1E780000	/* APB BRIDGE 2 */
#define ASPEED_GPIO_BASE               	0x1E780000	/* MMC */
#define ASPEED_RTC_BASE               	0x1E781000	/* RTC */
#define ASPEED_TIMER_BASE               0x1E782000	/* TIMER */

#if (CONFIG_AST1500_SOC_VER >= 2)
#define ASPEED_UART1_BASE               0x1E783000	/* UART#1 */
#define ASPEED_UART2_BASE               0x1E78D000	/* UART#2 */
#define ASPEED_UART3_BASE               0x1E78E000	/* UART#3 */
#define ASPEED_UART4_BASE               0x1E78F000	/* UART#4 */
#define ASPEED_UART5_BASE               0x1E784000	/* UART#5 */
#define ASPEED_UART_DBG_BASE            ASPEED_UART5_BASE	/* UART#5 */
#else //AST1500
#define ASPEED_UART1_BASE               0x1E783000	/* UART#1 */
#define ASPEED_UART2_BASE               0x1E784000	/* UART#2 */
#define ASPEED_UART_DBG_BASE            ASPEED_UART2_BASE	/* UART#2 */
#endif

#if (CONFIG_AST1500_SOC_VER >= 3) //BruceToDo. Move to board_def.h?
#define ASPEED_UART_SOIP_BASE           ASPEED_UART4_BASE	/* UART#4 */
#else
#define ASPEED_UART_SOIP_BASE           ASPEED_UART1_BASE	/* UART#1 */
#endif

#define ASPEED_WDT_BASE               	0x1E785000	/* WDT */
#define ASPEED_PWM_BASE               	0x1E786000	/* PWM */
#define ASPEED_VUART0_BASE              0x1E787000	/* VUART1 */
#define ASPEED_VUART1_BASE              0x1E788000	/* VUART2 */
#define ASPEED_LPC_BASE               	0x1E789000	/* LPC */
#define ASPEED_I2C_BASE               	0x1E78A000	/* I2C */
#define ASPEED_PECI_BASE               	0x1E78B000	/* PECI */
#define ASPEED_PCIARBITER_BASE          0x1E78C000	/* PCI ARBITER */

#if (CONFIG_AST1500_SOC_VER == 2)
	#define ASPEED_PCI_MASTER1_BASE         0x70000000	/* PCI MASTER 1 */
	#define ASPEED_PCI_MASTER2_BASE         0x80000000	/* PCI MASTER 2 */
#elif (CONFIG_AST1500_SOC_VER == 1)
	#define ASPEED_PCI_MASTER1_BASE         0x60000000	/* PCI MASTER 1 */
	#define ASPEED_PCI_MASTER2_BASE         0x80000000	/* PCI MASTER 2 */
#endif

#if (CONFIG_AST1500_SOC_DRAM_MAPPING_TYPE == 2)
#define ASPEED_SDRAM_BASE               0x80000000	/* SDRAM */
#else
#define ASPEED_SDRAM_BASE               0x40000000	/* SDRAM */
#endif

#define ASPEED_IO_START                 0x1E600000
#define ASPEED_IO_SIZE                  0x00200000

/*
 * VIC
 */
/* Interrupt Controllers */
#if (3 <= CONFIG_AST1500_SOC_VER)
#define ASPEED_VIC_STATUS_OFFSET	0x80
#define ASPEED_VIC_STATUS_OFFSET_H	0x84
#define ASPEED_VIC_RAW_STATUS_OFFSET    0x90
#define ASPEED_VIC_INT_SEL_OFFSET	0x98
#define ASPEED_VIC_ENABLE_SET_OFFSET    0xA0
#define ASPEED_VIC_ENABLE_CLEAR_OFFSET  0xA8
#define ASPEED_VIC_SENSE_OFFSET         0xC0
#define ASPEED_VIC_BOTH_EDGE_OFFSET	0xC8
#define ASPEED_VIC_EVENT_OFFSET         0xD0
#define ASPEED_VIC_EDGE_CLEAR_OFFSET    0xD8
#else
#define ASPEED_VIC_STATUS_OFFSET	0x00
#define ASPEED_VIC_RAW_STATUS_OFFSET    0x08
#define ASPEED_VIC_INT_SEL_OFFSET	0x0C
#define ASPEED_VIC_ENABLE_SET_OFFSET    0x10
#define ASPEED_VIC_ENABLE_CLEAR_OFFSET  0x14
#define ASPEED_VIC_SENSE_OFFSET         0x24
#define ASPEED_VIC_BOTH_EDGE_OFFSET	0x28
#define ASPEED_VIC_EVENT_OFFSET         0x2C
#define ASPEED_VIC_EDGE_CLEAR_OFFSET    0x38
#endif

#define ASPEED_VIC_STATUS               (ASPEED_VIC_BASE+ASPEED_VIC_STATUS_OFFSET)
#define ASPEED_VIC_RAW_STATUS           (ASPEED_VIC_BASE+ASPEED_VIC_RAW_STATUS_OFFSET)
#define ASPEED_VIC_ENABLE_SET           (ASPEED_VIC_BASE+ASPEED_VIC_ENABLE_SET_OFFSET)
#define ASPEED_VIC_ENABLE_CLEAR         (ASPEED_VIC_BASE+ASPEED_VIC_ENABLE_CLEAR_OFFSET)
#define ASPEED_VIC_SENSE                (ASPEED_VIC_BASE+ASPEED_VIC_SENSE_OFFSET)
#define ASPEED_VIC_BOTH_EDGE		(ASPEED_VIC_BASE+ASPEED_VIC_BOTH_EDGE_OFFSET)
#define ASPEED_VIC_EVENT                (ASPEED_VIC_BASE+ASPEED_VIC_EVENT_OFFSET)
#define ASPEED_VIC_EDGE_CLEAR           (ASPEED_VIC_BASE+ASPEED_VIC_EDGE_CLEAR_OFFSET)

/* Interrupt Controllers 2 */
#if	defined(CONFIG_ARCH_AST3000FPGA_CLIENT)
#define ASPEED_VIC2_STATUS_OFFSET	0x38
#define ASPEED_VIC2_RAW_STATUS_OFFSET   0x38		/* no use */
#define ASPEED_VIC2_INT_SEL_OFFSET	0x0C		/* no use */
#define ASPEED_VIC2_ENABLE_SET_OFFSET   0x28
#define ASPEED_VIC2_ENABLE_CLEAR_OFFSET 0x28
#define ASPEED_VIC2_SENSE_OFFSET        0x30
#define ASPEED_VIC2_BOTH_EDGE_OFFSET	0x34
#define ASPEED_VIC2_EVENT_OFFSET        0x2C
#define ASPEED_VIC2_EDGE_CLEAR_OFFSET   0x38
#else
#define ASPEED_VIC2_STATUS_OFFSET	0x18
#define ASPEED_VIC2_RAW_STATUS_OFFSET   0x18		/* no use */
#define ASPEED_VIC2_INT_SEL_OFFSET	0x0C		/* no use */
#define ASPEED_VIC2_ENABLE_SET_OFFSET   0x08
#define ASPEED_VIC2_ENABLE_CLEAR_OFFSET 0x08
#define ASPEED_VIC2_SENSE_OFFSET        0x10
#define ASPEED_VIC2_BOTH_EDGE_OFFSET	0x14
#define ASPEED_VIC2_EVENT_OFFSET        0x0C
#define ASPEED_VIC2_EDGE_CLEAR_OFFSET   0x18
#endif

#define ASPEED_VIC2_STATUS              (ASPEED_GPIO_BASE+ASPEED_VIC2_STATUS_OFFSET)
#define ASPEED_VIC2_RAW_STATUS          (ASPEED_GPIO_BASE+ASPEED_VIC2_RAW_STATUS_OFFSET)
#define ASPEED_VIC2_ENABLE_SET          (ASPEED_GPIO_BASE+ASPEED_VIC2_ENABLE_SET_OFFSET)
#define ASPEED_VIC2_ENABLE_CLEAR        (ASPEED_GPIO_BASE+ASPEED_VIC2_ENABLE_CLEAR_OFFSET)
#define ASPEED_VIC2_SENSE               (ASPEED_GPIO_BASE+ASPEED_VIC2_SENSE_OFFSET)
#define ASPEED_VIC2_BOTH_EDGE		(ASPEED_GPIO_BASE+ASPEED_VIC2_BOTH_EDGE_OFFSET)
#define ASPEED_VIC2_EVENT               (ASPEED_GPIO_BASE+ASPEED_VIC2_EVENT_OFFSET)
#define ASPEED_VIC2_EDGE_CLEAR          (ASPEED_GPIO_BASE+ASPEED_VIC2_EDGE_CLEAR_OFFSET)

/* Interrupt numbers and bit positions */
/* on VIC1 */
#if (3 > CONFIG_AST1500_SOC_VER)
#define INT_ECC				0
#define INTMASK_ECC			(1 << INT_ECC)

#define INT_MIC				1
#define INTMASK_MIC			(1 << INT_MIC)

#define INT_MAC1           		2
#define INTMASK_MAC1       		(1 << INT_MAC1)

#define INT_MAC2           		3
#define INTMASK_MAC2       		(1 << INT_MAC2)

#define INT_HAC           		4
#define INTMASK_HAC       		(1 << INT_HAC)

#define INT_USB20           		5
#define INTMASK_USB20       		(1 << INT_USB20)

#define INT_MDMA           		6
#define INTMASK_MDMA       		(1 << INT_MDMA)

#define INT_VIDEO           		7
#define INTMASK_VIDEO       		(1 << INT_VIDEO)

#define INT_LPC				8
#define INTMASK_LPC			(1 << INT_LPC)

#define INT_UARTINT0            	9
#define INTMASK_UARTINT0        	(1 << INT_UARTINT0)

#define INT_UARTINT1            	10
#define INTMASK_UARTINT1        	(1 << INT_UARTINT1)

#define INT_I2S           	        11
#define INTMASK_I2S       		(1 << INT_I2S)

#define INT_I2C				12
#define INTMASK_I2C			(1 << INT_I2C)

#define INT_USB11           		13
#define INTMASK_USB11       		(1 << INT_USB11)

#define INT_ADPCM           		14
#define INTMASK_ADPDM       		(1 << INT_ADPCM)

#if 0
#define INT_INTA           		15
#define INTMASK_INTA       		(1 << INT_INTA)
#endif

#define INT_TIMERINT0           	16
#define INTMASK_TIMERINT0       	(1 << INT_TIMERINT0)

#define INT_TIMERINT1           	17
#define INTMASK_TIMERINT1       	(1 << INT_TIMERINT1)

#define INT_TIMERINT2           	18
#define INTMASK_TIMERINT2       	(1 << INT_TIMERINT2)

#define INT_FLASH		      	19
#define INTMASK_FLASH       		(1 << INT_FLASH)

#define INT_GPIO           		20
#define INTMASK_GPIO       		(1 << INT_GPIO)

#define INT_SCU          		21
#define INTMASK_SCU      		(1 << INT_SCU)

#define INT_RTC_SEC			22
#define INTMASK_RTC_SEC			(1 << INT_RTC_SEC)

#define INT_RTC_DAY			23
#define INTMASK_RTC_DAY			(1 << INT_RTC_DAY)

#define INT_RTC_HOUR			24
#define INTMASK_RTC_HOUR		(1 << INT_RTC_HOUR)

#define INT_RTC_MIN			25
#define INTMASK_RTC_MIN			(1 << INT_RTC_MIN)

#define INT_RTC_ALARM			26
#define INTMASK_RTC_ALARM		(1 << INT_RTC_ALARM)

#define INT_WDT           		27
#define INTMASK_WDT        		(1 << INT_WDT)

#define INT_TACHO           		28
#define INTMASK_TACHO       		(1 << INT_TACHO)

#define INT_2D           		29
#define INTMASK_2D       		(1 << INT_2D)

#define INT_PCI           		30
#define INTMASK_PCI       		(1 << INT_PCI)

#define INT_AHBC			31
#define INTMASK_AHBC			(1 << INT_AHBC)

/* Extension IRQ */
#if	defined(CONFIG_ARCH_AST3000FPGA_CLIENT)
#define VIC2_IRQ_START			32
#define GPIOH_SHIFT			24
#define VIC2_IRQ_SHIFT			(VIC2_IRQ_START - GPIOH_SHIFT)

#define INT_INTA           		35
#define INTMASK_INTA       		(1 << (INT_INTA - VIC2_IRQ_SHIFT))

#define INT_INTB           		34
#define INTMASK_INTB       		(1 << (INT_INTB - VIC2_IRQ_SHIFT))

#define INT_INTC           		33
#define INTMASK_INTC       		(1 << (INT_INTC - VIC2_IRQ_SHIFT))

#define INT_INTD           		32
#define INTMASK_INTD       		(1 << (INT_INTD - VIC2_IRQ_SHIFT))
#endif
#if (CONFIG_AST1500_SOC_VER >= 2)

#define VIC2_IRQ_START			32
#define GPIOH_SHIFT			4 // Start from GPIOA4
#define VIC2_IRQ_SHIFT			(VIC2_IRQ_START - GPIOH_SHIFT)

#define INT_INTA           		32
#define INTMASK_INTA       		(1 << (INT_INTA - VIC2_IRQ_SHIFT))

#define INT_INTB           		33
#define INTMASK_INTB       		(1 << (INT_INTB - VIC2_IRQ_SHIFT))

#define INT_INTC           		34
#define INTMASK_INTC       		(1 << (INT_INTC - VIC2_IRQ_SHIFT))

#define INT_INTD           		35
#define INTMASK_INTD       		(1 << (INT_INTD - VIC2_IRQ_SHIFT))
#endif
#if (CONFIG_AST1500_SOC_VER == 1)
#define VIC2_IRQ_START			32
#define GPIOH_SHIFT			8
#define VIC2_IRQ_SHIFT			(VIC2_IRQ_START - GPIOH_SHIFT)

#define INT_INTA           		32
#define INTMASK_INTA       		(1 << (INT_INTA - VIC2_IRQ_SHIFT))

#define INT_INTB           		33
#define INTMASK_INTB       		(1 << (INT_INTB - VIC2_IRQ_SHIFT))

#define INT_INTC           		34
#define INTMASK_INTC       		(1 << (INT_INTC - VIC2_IRQ_SHIFT))

#define INT_INTD           		35
#define INTMASK_INTD       		(1 << (INT_INTD - VIC2_IRQ_SHIFT))
#endif
#endif

#if (3 <= CONFIG_AST1500_SOC_VER)
#define INT_SDRAM		0
/* 1 reserved */
#define INT_MAC1		2
#define INT_MAC2		3
#define INT_CRYPTO		4
#define INT_USB20		5
/* 6 reserved */
#define INT_VIDEO		7
/* 8 reserved */
#define INT_UART1		9
#define INT_UART5		10
/* 11 reserved */
#define INT_I2C			12
#define INT_USB20_2		13
#define INT_USB11		14
/* 15 reserved */
#define INT_TIMER1		16
#define INT_TIMER2		17
#define INT_TIMER3		18
#define INT_SMC			19
#define INT_GPIO		20
#define INT_SCU			21
#define INT_RCT_ALARM		22
/* 23 reserved */
#define INT_I2S			24
#define INT_CRT			25
#define INT_SD			26
#define INT_WDT			27
/* 28 reserved */
/* 29 reserved */
#define INT_SYSTEM_WAKE_UP	30
#define INT_ADC			31
#define INT_UART2		32
#define INT_UART3		33
#define INT_UART4		34
#define INT_TIMER4		35

#define INT_TIMER5		36
#define INT_TIMER6		37
#define INT_TIMER7		38
#define INT_TIMER8		39
/* 40~46 reserved */
#define INT_GPIO_L1		47
#define INT_GPIO_L3		48
#define INT_GPIO_M1		49
#define INT_UART_DMA		50
/* 51~63 reserved */

#define VIC2_IRQ_START			32
#define INT_MAX_NUM		INT_UART_DMA
#define VIC2_IRQ_SHIFT		32
#define INTMASK(INTR)	(1 << INT_##INTR)
#define INTMASK_H(INTR)	(1 << (INT_##INTR - VIC2_IRQ_SHIFT))

/* For aspeed-ehci driver */
#define INT_USB20HC1		INT_USB20
#define INTMASK_USB20HC1	INTMASK(USB20)
#define INT_USB20HC2		INT_USB20_2
#define INTMASK_USB20HC2	INTMASK(USB20_2)
#endif

/* Mask of valid system controller interrupts */
/* VIC means VIC1 */
#if (3 <= CONFIG_AST1500_SOC_VER)
#define ASPEED_VIC_VALID_INTMASK	( \
					INTMASK(SDRAM) | \
					INTMASK(MAC1) | INTMASK(MAC2) | \
					INTMASK(CRYPTO) | INTMASK(USB20) | \
					INTMASK(VIDEO) | \
					INTMASK(UART1) | INTMASK(UART5) | \
					INTMASK(I2C) | \
					INTMASK(USB20_2) | INTMASK(USB11) | \
					INTMASK(TIMER1) | INTMASK(TIMER2) | INTMASK(TIMER3) | \
					INTMASK(SMC) | \
					INTMASK(GPIO) | \
					INTMASK(SCU) | \
					INTMASK(RCT_ALARM) | \
					INTMASK(I2S) | \
					INTMASK(CRT) | \
					INTMASK(SD) | \
					INTMASK(WDT) | \
					INTMASK(SYSTEM_WAKE_UP) | \
					INTMASK(ADC) \
					)

#define ASPEED_VIC2_VALID_INTMASK	( \
					INTMASK_H(UART2) | INTMASK_H(UART3) | INTMASK_H(UART4) | \
					INTMASK_H(TIMER4) | INTMASK_H(TIMER5) | \
					INTMASK_H(TIMER6) | INTMASK_H(TIMER7) | \
					INTMASK_H(TIMER8) | \
					INTMASK_H(GPIO_L1) | INTMASK_H(GPIO_L3) | \
					INTMASK_H(GPIO_M1) | \
					INTMASK_H(UART_DMA) \
					)
#else
#define ASPEED_VIC_VALID_INTMASK	( \
					 INTMASK_ECC | \
                                         INTMASK_MIC | \
                                         (INTMASK_MAC1 | INTMASK_MAC2) | \
                                         INTMASK_HAC | \
                                         INTMASK_USB20 | \
                                         INTMASK_MDMA | \
                                         INTMASK_VIDEO | \
                                         INTMASK_LPC | \
                                         (INTMASK_UARTINT0 | INTMASK_UARTINT1) | \
                                         INTMASK_I2S | \
                                         INTMASK_I2C | \
                                         INTMASK_USB11 | \
                                         INTMASK_ADPDM | \
                                         (INTMASK_TIMERINT0 | INTMASK_TIMERINT1 | INTMASK_TIMERINT2) | \
                                         INTMASK_FLASH | \
                                         INTMASK_GPIO | \
                                         INTMASK_SCU | \
					 (INTMASK_RTC_SEC | INTMASK_RTC_DAY | INTMASK_RTC_HOUR | INTMASK_RTC_MIN | INTMASK_RTC_ALARM) | \
                                         INTMASK_WDT | \
                                         INTMASK_TACHO | \
                                         INTMASK_2D | \
                                         INTMASK_PCI | \
                                         INTMASK_AHBC \
					)

#define ASPEED_VIC2_VALID_INTMASK	( \
					 (INTMASK_INTA | INTMASK_INTB | INTMASK_INTC | INTMASK_INTD) \
					)
#endif

#if (CONFIG_AST1500_SOC_VER >= 3)
	#define MAXIRQNUM                       INT_MAX_NUM
#else /* #if (CONFIG_AST1500_SOC_VER >= 3) */
#if defined(CONFIG_ARCH_AST1500_CLIENT)
	#if (CONFIG_AST1500_SOC_VER <= 2)
		#define MAXIRQNUM               35
	#else
		#define MAXIRQNUM               31
	#endif
#else
	#define MAXIRQNUM                       31
#endif
#endif /* #if (CONFIG_AST1500_SOC_VER >= 3) */

#define MAXFIQNUM                       29

/*
 * Timer
 */
#define ASPEED_TIMER0_OFFSET		0x0000		/* Timer0 Offset */
#define ASPEED_TIMER1_OFFSET		0x0010		/* Timer1 Offset */
#define ASPEED_TIMER2_OFFSET		0x0020		/* Timer2 Offset */
#define ASPEED_TIMERRC_OFFSET		0x0030		/* Timer RC Offset */

#define ASPEED_TIMER0_VA_BASE		(IO_ADDRESS(ASPEED_TIMER_BASE)+ASPEED_TIMER0_OFFSET)
#define ASPEED_TIMER1_VA_BASE		(IO_ADDRESS(ASPEED_TIMER_BASE)+ASPEED_TIMER1_OFFSET)
#define ASPEED_TIMER2_VA_BASE		(IO_ADDRESS(ASPEED_TIMER_BASE)+ASPEED_TIMER2_OFFSET)
#define ASPEED_TIMERC_VA_BASE		(IO_ADDRESS(ASPEED_TIMER_BASE)+ASPEED_TIMERRC_OFFSET)

#define ASPEED_TIMER_CLKRATE		(ASPEED_EXTCLK)
#define ASPEED_EXTCLK			(1*1000*1000)	/* 1M */

/*
 * UART
 */
#define ASPEED_UART1_VIRTUAL		    (IO_ADDRESS(ASPEED_UART1_BASE))
#define ASPEED_UART2_VIRTUAL		    (IO_ADDRESS(ASPEED_UART2_BASE))
#if (CONFIG_AST1500_SOC_VER >= 2)
#define ASPEED_UART3_VIRTUAL		    (IO_ADDRESS(ASPEED_UART3_BASE))
#define ASPEED_UART4_VIRTUAL		    (IO_ADDRESS(ASPEED_UART4_BASE))
#define ASPEED_UART5_VIRTUAL		    (IO_ADDRESS(ASPEED_UART5_BASE))
#endif
#define ASPEED_UART_DBG_VIRTUAL         (IO_ADDRESS(ASPEED_UART_DBG_BASE))
#define ASPEED_UART_SOIP_VIRTUAL        (IO_ADDRESS(ASPEED_UART_SOIP_BASE))

/*
 * Ticks
 */
//#define TICKS_PER_uSEC                  40 // IP Cam
//#define TICKS_PER_uSEC                  24 // FPGA
#define TICKS_PER_uSEC                  1		/* ASPEED_EXTCLK / 10 ^ 6 */

#define mSEC_1                          1000
#define mSEC_5                          (mSEC_1 * 5)
#define mSEC_10                         (mSEC_1 * 10)
#define mSEC_25                         (mSEC_1 * 25)
#define SEC_1                           (mSEC_1 * 1000)

/*
 * RTC
 */
#define ASPEED_RTC_VA_BASE		(IO_ADDRESS(ASPEED_RTC_BASE))

/*
 * Watchdog
 */
#define ASPEED_WDT_VA_BASE		(IO_ADDRESS(ASPEED_WDT_BASE))

/*
 * PCI Stuff
 */
#define NP_CMD_IOREAD			0x2
#define NP_CMD_IOWRITE			0x3
#define NP_CMD_CONFIGREAD		0xa
#define NP_CMD_CONFIGWRITE		0xb
#define NP_CMD_MEMREAD			0x6
#define	NP_CMD_MEMWRITE			0x7

#define AST1500_PCI_MEM_BASE0		0x00000000 //TBD
#define AST1500_PCI_MEM_END0		0x0000FFFF //TBD

#if (CONFIG_AST1500_SOC_VER >= 2)
	#define AST1500_PCI_MEM_BASE1		0x70020000 //AST1510 has only 256MB.
	#define AST1500_PCI_MEM_END1		0x7fffffff
#else
	#define AST1500_PCI_MEM_BASE1		0x68000000
	#define AST1500_PCI_MEM_END1		0x7fffffff
#endif

#if (CONFIG_AST1500_SOC_VER >= 2)
	#define PCI_CSR_BASE			0x70000000
#else
	#define PCI_CSR_BASE			0x60000000
#endif
#define PCI_CSR_VA_BASE			IO_ADDRESS2(PCI_CSR_BASE)

#define CSR_CRP_CMD_OFFSET		0x00
#define CSR_CRP_WRITE_OFFSET    	0x04
#define CSR_CRP_READ_OFFSET		0x08
#define CSR_PCI_ADDR_OFFSET		0x0C
#define CSR_PCI_CMD_OFFSET		0x10
#define CSR_PCI_WRITE_OFFSET		0x14
#define CSR_PCI_READ_OFFSET		0x18
#define CSR_PCI_STATUS_OFFSET		0x1C

#define CRP_ADDR_REG			(volatile unsigned long*) (PCI_CSR_VA_BASE + CSR_CRP_CMD_OFFSET)
#define CRP_WRITE_REG			(volatile unsigned long*) (PCI_CSR_VA_BASE + CSR_CRP_WRITE_OFFSET)
#define CRP_READ_REG			(volatile unsigned long*) (PCI_CSR_VA_BASE + CSR_CRP_READ_OFFSET)
#define PCI_ADDR_REG			(volatile unsigned long*) (PCI_CSR_VA_BASE + CSR_PCI_ADDR_OFFSET)
#define PCI_CMD_REG			(volatile unsigned long*) (PCI_CSR_VA_BASE + CSR_PCI_CMD_OFFSET)
#define PCI_WRITE_REG			(volatile unsigned long*) (PCI_CSR_VA_BASE + CSR_PCI_WRITE_OFFSET)
#define PCI_READ_REG			(volatile unsigned long*) (PCI_CSR_VA_BASE + CSR_PCI_READ_OFFSET)

#define RESET_PCI_STATUS        	*(volatile unsigned long*) (PCI_CSR_VA_BASE + CSR_PCI_STATUS_OFFSET) = 0x01
#define CHK_PCI_STATUS          	(*(volatile unsigned long*) (PCI_CSR_VA_BASE + CSR_PCI_STATUS_OFFSET) & 0x03)

#endif

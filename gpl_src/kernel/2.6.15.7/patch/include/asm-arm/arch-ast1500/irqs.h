/*
 *  linux/include/asm-arm/arch-mvp2000/irqs.h
 *
 *  Copyright (C) 1999 ARM Limited
 *  Copyright (C) 2000 Deep Blue Solutions Ltd.
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

/* Use the mvp-2000 definitions */
#include <asm/arch/platform.h>


#if (3 <= CONFIG_AST1500_SOC_VER)
#define IRQ_TIMERINT0                   INT_TIMER1
#define IRQ_TIMERINT1                   INT_TIMER2
#define IRQ_TIMERINT2                   INT_TIMER3
#define IRQ_UARTINT0                    INT_UART4
#define IRQ_UARTINT1                    INT_UART5
#define IRQ_UARTINT2                    INT_UART3
#define IRQ_MAC1                        INT_MAC1
#define IRQ_MAC2                        INT_MAC2

#define IRQMASK_TIMERINT0               INTMASK(TIMER1)
#define IRQMASK_TIMERINT1               INTMASK(TIMER2)
#define IRQMASK_TIMERINT2               INTMASK(TIMER3)
#define IRQMASK_UARTINT0                INTMASK(UART4)
#define IRQMASK_UARTINT1                INTMASK(UART5)
#else
#define IRQ_TIMERINT0                   INT_TIMERINT0
#define IRQ_TIMERINT1                   INT_TIMERINT1
#define IRQ_TIMERINT2                   INT_TIMERINT2
#define IRQ_GUCETHERNET			INT_GUCETHERNET
#define IRQ_IDEINT                      INT_IDEINT
#define IRQ_UARTINT0                    INT_UARTINT0
#define IRQ_UARTINT1                    INT_UARTINT1
#define IRQ_MAC1                        INT_MAC1
#define IRQ_MAC2                        INT_MAC2

#define IRQMASK_TIMERINT0               INTMASK_TIMERINT0
#define IRQMASK_TIMERINT1               INTMASK_TIMERINT1
#define IRQMASK_TIMERINT2               INTMASK_TIMERINT2
#define IRQMASK_GUCETHERNET		INTMASK_GUCETHERNET
#define IRQMASK_IDEINT			INTMASK_IDEINT
#define IRQMASK_UARTINT0                INTMASK_UARTINT0
#define IRQMASK_UARTINT1                INTMASK_UARTINT1
#define IRQMASK_MAC1                    INTMASK_MAC1
#define IRQMASK_MAC2                    INTMASK_MAC2

#define IRQ_VIC2			INT_GPIO
#define IRQMASK_VIC2			INTMASK_GPIO
#endif

/*
 * VIC Register (VA)
 */
#define VIC_SENSE_VA                    (IO_ADDRESS(ASPEED_VIC_BASE)+ASPEED_VIC_SENSE_OFFSET)
#define VIC_EVENT_VA                    (IO_ADDRESS(ASPEED_VIC_BASE)+ASPEED_VIC_EVENT_OFFSET)
#define VIC_EDGE_CLEAR_VA               (IO_ADDRESS(ASPEED_VIC_BASE)+ASPEED_VIC_EDGE_CLEAR_OFFSET)

#define IRQ_SET_LEVEL_TRIGGER(irq_no)   *((volatile unsigned long*)VIC_SENSE_VA) |= 1 << (irq_no)
#define IRQ_SET_EDGE_TRIGGER(irq_no)    *((volatile unsigned long*)VIC_SENSE_VA) &= ~(1 << (irq_no))
#define IRQ_SET_RISING_EDGE(irq_no)     *((volatile unsigned long*)VIC_EVENT_VA) |= 1 << (irq_no)
#define IRQ_SET_FALLING_EDGE(irq_no)    *((volatile unsigned long*)VIC_EVENT_VA) &= ~(1 << (irq_no))
#define IRQ_SET_HIGH_LEVEL(irq_no)      *((volatile unsigned long*)VIC_EVENT_VA) |= 1 << (irq_no)
#define IRQ_SET_LOW_LEVEL(irq_no)       *((volatile unsigned long*)VIC_EVENT_VA) &= ~(1 << (irq_no))
#define IRQ_EDGE_CLEAR(irq_no)          *((volatile unsigned long*)VIC_EDGE_CLEAR_VA) |= 1 << (irq_no)

#define VIC2_SENSE_VA                    	(IO_ADDRESS(ASPEED_GPIO_BASE)+ASPEED_VIC2_SENSE_OFFSET)
#define VIC2_EVENT_VA                    	(IO_ADDRESS(ASPEED_GPIO_BASE)+ASPEED_VIC2_EVENT_OFFSET)
#define VIC2_EDGE_CLEAR_VA               	(IO_ADDRESS(ASPEED_GPIO_BASE)+ASPEED_VIC2_EDGE_CLEAR_OFFSET)

#define VIC2_IRQ_SET_LEVEL_TRIGGER(irq_no)   	*((volatile unsigned long*)VIC2_SENSE_VA) |= 1 << (irq_no - VIC2_IRQ_SHIFT)
#define VIC2_IRQ_SET_EDGE_TRIGGER(irq_no)    	*((volatile unsigned long*)VIC2_SENSE_VA) &= ~(1 << (irq_no - VIC2_IRQ_SHIFT))
#define VIC2_IRQ_SET_RISING_EDGE(irq_no)     	*((volatile unsigned long*)VIC2_EVENT_VA) |= 1 << (irq_no - VIC2_IRQ_SHIFT)
#define VIC2_IRQ_SET_FALLING_EDGE(irq_no)    	*((volatile unsigned long*)VIC2_EVENT_VA) &= ~(1 << (irq_no - VIC2_IRQ_SHIFT))
#define VIC2_IRQ_SET_HIGH_LEVEL(irq_no)      	*((volatile unsigned long*)VIC2_EVENT_VA) |= 1 << (irq_no - VIC2_IRQ_SHIFT)
#define VIC2_IRQ_SET_LOW_LEVEL(irq_no)       	*((volatile unsigned long*)VIC2_EVENT_VA) &= ~(1 << (irq_no - VIC2_IRQ_SHIFT))
#define VIC2_IRQ_EDGE_CLEAR(irq_no)          	*((volatile unsigned long*)VIC_EDGE_CLEAR_VA) |= 1 << (irq_no - VIC2_IRQ_SHIFT)

#define NR_IRQS                         (MAXIRQNUM + 1)

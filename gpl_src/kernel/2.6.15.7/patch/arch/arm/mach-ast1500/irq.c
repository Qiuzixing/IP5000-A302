/*
 *  linux/arch/arm/mach-umvp/irq.c
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
#include <linux/init.h>
#include <linux/stddef.h>
#include <linux/list.h>
#include <linux/timer.h>

#include <asm/hardware.h>
#include <asm/irq.h>
#include <asm/io.h>

#include <asm/mach/irq.h>
#include <asm/arch/vic.h>

#define VIC_BASE_VA		IO_ADDRESS(ASPEED_VIC_BASE)

#if (CONFIG_AST1500_SOC_VER <= 2)
#define VIC2_BASE_VA            IO_ADDRESS(ASPEED_GPIO_BASE) 
#endif

#define UMVP_READ_REG(r)        (*((volatile unsigned int *) (r)))
#define UMVP_WRITE_REG(r,v)     (*((volatile unsigned int *) (r)) = ((unsigned int)   (v)))

#define DEFAULT_VIC_EDGE	0
#define DEFAULT_VIC_LEVEL	1
#define DEFAULT_VIC_LOW		0
#define DEFAULT_VIC_HIGH	1

struct  _DEFAULT_VIC_TABLE {
    unsigned int	vic_type;
    unsigned int	vic_trigger;	
};
	
struct _DEFAULT_VIC_TABLE default_vic[] = {
#if (3 <= CONFIG_AST1500_SOC_VER)
	{DEFAULT_VIC_LEVEL, DEFAULT_VIC_HIGH}, /*  0: SDRAM */
	{DEFAULT_VIC_LEVEL, DEFAULT_VIC_HIGH}, /*  1: reserved */
	{DEFAULT_VIC_LEVEL, DEFAULT_VIC_HIGH}, /*  2: MAC1 */
	{DEFAULT_VIC_LEVEL, DEFAULT_VIC_HIGH}, /*  3: MAC2 */
	{DEFAULT_VIC_LEVEL, DEFAULT_VIC_HIGH}, /*  4: CRYPTO */
	{DEFAULT_VIC_LEVEL, DEFAULT_VIC_HIGH}, /*  5: USB2 */
	{DEFAULT_VIC_LEVEL, DEFAULT_VIC_HIGH}, /*  6: reserved */
	{DEFAULT_VIC_LEVEL, DEFAULT_VIC_HIGH}, /*  7: VIDEO */
	{DEFAULT_VIC_LEVEL, DEFAULT_VIC_HIGH}, /*  8: reserved */
	{DEFAULT_VIC_LEVEL, DEFAULT_VIC_HIGH}, /*  9: UART1 */
	{DEFAULT_VIC_LEVEL, DEFAULT_VIC_HIGH}, /* 10: UART5 */
	{DEFAULT_VIC_LEVEL, DEFAULT_VIC_HIGH}, /* 11: reserved */
	{DEFAULT_VIC_LEVEL, DEFAULT_VIC_HIGH}, /* 12: I2C */
	{DEFAULT_VIC_LEVEL, DEFAULT_VIC_HIGH}, /* 13: USB2.0 Host 2*/
	{DEFAULT_VIC_LEVEL, DEFAULT_VIC_HIGH}, /* 14: USB1.1 */
	{DEFAULT_VIC_LEVEL, DEFAULT_VIC_HIGH}, /* 15: reserved */
	{DEFAULT_VIC_EDGE,  DEFAULT_VIC_HIGH}, /* 16: TIMER 1 */
	{DEFAULT_VIC_EDGE,  DEFAULT_VIC_HIGH}, /* 17: TIMER 2 */
	{DEFAULT_VIC_EDGE,  DEFAULT_VIC_HIGH}, /* 18: TIMER 3 */
	{DEFAULT_VIC_LEVEL, DEFAULT_VIC_HIGH}, /* 19: SMC */
	{DEFAULT_VIC_LEVEL, DEFAULT_VIC_HIGH}, /* 20: GPIO */
	{DEFAULT_VIC_LEVEL, DEFAULT_VIC_HIGH}, /* 21: SCU */
	{DEFAULT_VIC_LEVEL, DEFAULT_VIC_HIGH}, /* 22: RTC alarm */
	{DEFAULT_VIC_LEVEL, DEFAULT_VIC_HIGH}, /* 23: reserved */
	{DEFAULT_VIC_LEVEL, DEFAULT_VIC_HIGH}, /* 24: I2S */
	{DEFAULT_VIC_LEVEL, DEFAULT_VIC_HIGH}, /* 25: Graphics CRT */
	{DEFAULT_VIC_LEVEL, DEFAULT_VIC_HIGH}, /* 26: SD/SDIO */
	{DEFAULT_VIC_LEVEL, DEFAULT_VIC_HIGH}, /* 27: WDT ALARM */
	{DEFAULT_VIC_LEVEL, DEFAULT_VIC_HIGH}, /* 28: reserved */
	{DEFAULT_VIC_LEVEL, DEFAULT_VIC_HIGH}, /* 29: reserved */
	{DEFAULT_VIC_LEVEL, DEFAULT_VIC_HIGH}, /* 30: System wake up */
	{DEFAULT_VIC_LEVEL, DEFAULT_VIC_HIGH}, /* 31: ADC */
	{DEFAULT_VIC_LEVEL, DEFAULT_VIC_HIGH}, /* 32: UART 2 */
	{DEFAULT_VIC_LEVEL, DEFAULT_VIC_HIGH}, /* 33: UART 3 */
	{DEFAULT_VIC_LEVEL, DEFAULT_VIC_HIGH}, /* 34: UART 4 */
	{DEFAULT_VIC_EDGE,  DEFAULT_VIC_HIGH}, /* 35: Timer 4 */
	{DEFAULT_VIC_EDGE,  DEFAULT_VIC_HIGH}, /* 36: Timer 5 */
	{DEFAULT_VIC_EDGE,  DEFAULT_VIC_HIGH}, /* 37: Timer 6 */
	{DEFAULT_VIC_EDGE,  DEFAULT_VIC_HIGH}, /* 38: Timer 7 */
	{DEFAULT_VIC_EDGE,  DEFAULT_VIC_HIGH}, /* 39: Timer 8 */
	{DEFAULT_VIC_LEVEL, DEFAULT_VIC_HIGH}, /* 40: reserved */
	{DEFAULT_VIC_LEVEL, DEFAULT_VIC_HIGH}, /* 41: reserved */
	{DEFAULT_VIC_LEVEL, DEFAULT_VIC_HIGH}, /* 42: reserved */
	{DEFAULT_VIC_LEVEL, DEFAULT_VIC_HIGH}, /* 43: reserved */
	{DEFAULT_VIC_LEVEL, DEFAULT_VIC_HIGH}, /* 44: reserved */
	{DEFAULT_VIC_LEVEL, DEFAULT_VIC_HIGH}, /* 45: reserved */
	{DEFAULT_VIC_LEVEL, DEFAULT_VIC_HIGH}, /* 46: reserved */
	{DEFAULT_VIC_LEVEL, DEFAULT_VIC_HIGH}, /* 47: GPIO L1 */
	{DEFAULT_VIC_LEVEL, DEFAULT_VIC_HIGH}, /* 48: GPIO L3 */
	{DEFAULT_VIC_LEVEL, DEFAULT_VIC_HIGH}, /* 49: GPIO M1 */
	{DEFAULT_VIC_LEVEL, DEFAULT_VIC_HIGH}, /* 50: UART DMA */
	{DEFAULT_VIC_LEVEL, DEFAULT_VIC_HIGH}, /* 51: reserved */
	{DEFAULT_VIC_LEVEL, DEFAULT_VIC_HIGH}, /* 52: reserved */
	{DEFAULT_VIC_LEVEL, DEFAULT_VIC_HIGH}, /* 53: reserved */
	{DEFAULT_VIC_LEVEL, DEFAULT_VIC_HIGH}, /* 54: reserved */
	{DEFAULT_VIC_LEVEL, DEFAULT_VIC_HIGH}, /* 55: reserved */
	{DEFAULT_VIC_LEVEL, DEFAULT_VIC_HIGH}, /* 56: reserved */
	{DEFAULT_VIC_LEVEL, DEFAULT_VIC_HIGH}, /* 57: reserved */
	{DEFAULT_VIC_LEVEL, DEFAULT_VIC_HIGH}, /* 58: reserved */
	{DEFAULT_VIC_LEVEL, DEFAULT_VIC_HIGH}, /* 59: reserved */
	{DEFAULT_VIC_LEVEL, DEFAULT_VIC_HIGH}, /* 60: reserved */
	{DEFAULT_VIC_LEVEL, DEFAULT_VIC_HIGH}, /* 61: reserved */
	{DEFAULT_VIC_LEVEL, DEFAULT_VIC_HIGH}, /* 62: reserved */
	{DEFAULT_VIC_LEVEL, DEFAULT_VIC_HIGH}, /* 63: reserved */
#else
    { DEFAULT_VIC_LEVEL, DEFAULT_VIC_HIGH },		/* INT31: AHBC      */	    	
    { DEFAULT_VIC_LEVEL, DEFAULT_VIC_HIGH },		/* INT30: PCI       */	
    { DEFAULT_VIC_LEVEL, DEFAULT_VIC_HIGH },		/* INT29: 2D        */	
    { DEFAULT_VIC_LEVEL, DEFAULT_VIC_HIGH },		/* INT28: TACHO     */	
    { DEFAULT_VIC_EDGE,  DEFAULT_VIC_HIGH },		/* INT27: WDT ALARM */	
    { DEFAULT_VIC_EDGE,  DEFAULT_VIC_HIGH },		/* INT26: RTC ALARM */	
    { DEFAULT_VIC_EDGE,  DEFAULT_VIC_HIGH },		/* INT25: RTC MIN   */	
    { DEFAULT_VIC_EDGE,  DEFAULT_VIC_HIGH },		/* INT24: RTC HOUR  */	
    { DEFAULT_VIC_EDGE,  DEFAULT_VIC_HIGH },		/* INT23: RTC DAY   */	
    { DEFAULT_VIC_EDGE,  DEFAULT_VIC_HIGH },		/* INT22: RTC SEC   */	
    { DEFAULT_VIC_LEVEL, DEFAULT_VIC_HIGH },		/* INT21: SCU       */	
    { DEFAULT_VIC_LEVEL, DEFAULT_VIC_HIGH },		/* INT20: GPIO      */	
    { DEFAULT_VIC_LEVEL, DEFAULT_VIC_HIGH },		/* INT19: SMC       */	
    { DEFAULT_VIC_EDGE,  DEFAULT_VIC_HIGH },		/* INT18: TIMER3    */	
    { DEFAULT_VIC_EDGE,  DEFAULT_VIC_HIGH },		/* INT17: TIMER2    */	
    { DEFAULT_VIC_EDGE,  DEFAULT_VIC_HIGH },		/* INT16: TIMER1    */	
    { DEFAULT_VIC_LEVEL, DEFAULT_VIC_HIGH },		/* INT15: PECI      */	
    { DEFAULT_VIC_LEVEL, DEFAULT_VIC_HIGH },		/* INT14: TBD       */	
    { DEFAULT_VIC_LEVEL, DEFAULT_VIC_HIGH },		/* INT13: USB11     */	
    { DEFAULT_VIC_LEVEL, DEFAULT_VIC_HIGH },		/* INT12: I2C       */	
    { DEFAULT_VIC_LEVEL, DEFAULT_VIC_HIGH },		/* INT11: I2S       */	
    { DEFAULT_VIC_LEVEL, DEFAULT_VIC_HIGH },		/* INT10: UART2     */	
    { DEFAULT_VIC_LEVEL, DEFAULT_VIC_HIGH },		/* INT 9: UART1     */	
    { DEFAULT_VIC_LEVEL, DEFAULT_VIC_HIGH },		/* INT 8: LPC       */	
    { DEFAULT_VIC_LEVEL, DEFAULT_VIC_HIGH },		/* INT 7: VIDEO     */	
    { DEFAULT_VIC_LEVEL, DEFAULT_VIC_HIGH },		/* INT 6: MDMA      */	
    { DEFAULT_VIC_LEVEL, DEFAULT_VIC_HIGH },		/* INT 5: USB2      */	
    { DEFAULT_VIC_LEVEL, DEFAULT_VIC_HIGH },		/* INT 4: CRYPTO    */	
    { DEFAULT_VIC_LEVEL, DEFAULT_VIC_HIGH },		/* INT 3: MAC2      */	
    { DEFAULT_VIC_LEVEL, DEFAULT_VIC_HIGH },		/* INT 2: MAC1      */	
    { DEFAULT_VIC_LEVEL, DEFAULT_VIC_HIGH },		/* INT 1: MIC       */		
    { DEFAULT_VIC_LEVEL, DEFAULT_VIC_HIGH },		/* INT 0: SDRAM     */
#endif
};


#if (3 <= CONFIG_AST1500_SOC_VER)
#define READ_VIC_REG(a) UMVP_READ_REG(VIC_BASE_VA + a)
#define WRITE_VIC_REG(a, b) UMVP_WRITE_REG(VIC_BASE_VA + a, b)
#endif
#if (3 <= CONFIG_AST1500_SOC_VER)
static void vic_reset_1520(void)
{
	u32 i, j, reg_offset, vic_type, vic_trigger;
	for (i = 0; i < 2; i++) {
		reg_offset = i * 4;
		WRITE_VIC_REG(ASPEED_VIC_INT_SEL_OFFSET + reg_offset, 0x00000000);
		WRITE_VIC_REG(ASPEED_VIC_ENABLE_SET_OFFSET + reg_offset, 0x00000000);
		WRITE_VIC_REG(ASPEED_VIC_ENABLE_CLEAR_OFFSET + reg_offset, 0xFFFFFFFF);
		WRITE_VIC_REG(ASPEED_VIC_EDGE_CLEAR_OFFSET + reg_offset, 0xFFFFFFFF);

		vic_type = 0, vic_trigger = 0;
		for (j = 0; j < 32; j++) {
			vic_type |= (default_vic[j + 32*i].vic_type << j);
			vic_trigger |= (default_vic[j + 32*i].vic_trigger << j);
		}

		WRITE_VIC_REG(ASPEED_VIC_SENSE_OFFSET + reg_offset, vic_type);
		WRITE_VIC_REG(ASPEED_VIC_EVENT_OFFSET + reg_offset, vic_trigger);
	}
}
#endif

static void vic_reset(void)
{
#if (3 <= CONFIG_AST1500_SOC_VER)
	vic_reset_1520();
#else
    unsigned int i, vic_type=0, vic_trigger=0;
	
    UMVP_WRITE_REG(VIC_BASE_VA + ASPEED_VIC_INT_SEL_OFFSET,      0x00000000);
    UMVP_WRITE_REG(VIC_BASE_VA + ASPEED_VIC_ENABLE_SET_OFFSET,   0x00000000);
    UMVP_WRITE_REG(VIC_BASE_VA + ASPEED_VIC_ENABLE_CLEAR_OFFSET, 0xFFFFFFFF);
    UMVP_WRITE_REG(VIC_BASE_VA + ASPEED_VIC_EDGE_CLEAR_OFFSET,   0xFFFFFFFF);

    /* set default VIC settings */
    for (i=0; i<32; i++)
    {
    	vic_type   <<= 1;
    	vic_trigger<<= 1;
        vic_type    |= default_vic[i].vic_type;
        vic_trigger |= default_vic[i].vic_trigger;       
    }
    UMVP_WRITE_REG(VIC_BASE_VA + ASPEED_VIC_SENSE_OFFSET, vic_type);
    UMVP_WRITE_REG(VIC_BASE_VA + ASPEED_VIC_EVENT_OFFSET, vic_trigger);
#endif
}

#if (3 <= CONFIG_AST1500_SOC_VER)
static void vic_set_intr_trigger_1520(int irq, vic_intr_trig_mode_E mode)
{
	u32 val, mask, reg, reg_offset = 0;

	mask = 1 << (irq%32);

	if (31 < irq)
		reg_offset = 4;

	/* edge or level trigger */
	reg = ASPEED_VIC_SENSE_OFFSET + reg_offset;
	val = READ_VIC_REG(reg);
	switch (mode) {
	case vicc_edge_activeFalling:
	case vicc_edge_activeRaising:
	case vicc_edge_activeBoth:
		/* edge trigger : corresponding bit shall be reset as 0 */
		val &= ~mask;
		break;
	default:
		/* level trigger : corresponding bit shall be set as 1 */
		val |= mask;
		break;
	}
	WRITE_VIC_REG(reg, val);

	/* active high or active low */
	reg = ASPEED_VIC_EVENT_OFFSET + reg_offset;
	val = READ_VIC_REG(reg);
	switch (mode) {
	case vicc_edge_activeFalling:
	case vicc_level_activeLow:
		/* active low : corresponding bit shall be reset as 0 */
		val &= ~mask;
		break;
	case vicc_edge_activeRaising:
	case vicc_level_activeHigh:
		/* active high : corresponding bit shall be set as 1 */
		val |= mask;
		break;
	default:
		break;
	}
	WRITE_VIC_REG(reg, val);

	/* both edge? */
	reg = ASPEED_VIC_BOTH_EDGE_OFFSET + reg_offset;
	val = READ_VIC_REG(reg);
	if (mode == vicc_edge_activeBoth) /* both edge : corresponding bit shall be set as 1 */
		val |= mask;
	else /* single edge : corresponding bit shall be reset as 0 */
		val &= ~mask;

	WRITE_VIC_REG(reg, val);
}
#endif

void vic_set_intr_trigger(int irqnr, vic_intr_trig_mode_E mode)
{
#if (3 <= CONFIG_AST1500_SOC_VER)
	vic_set_intr_trigger_1520(irqnr, mode);
#else
    unsigned int nMask;
    unsigned int nRegValue;
    unsigned int nVICVA;
    unsigned int nVIC_SENSE_OFFSET, nVIC_EVENT_OFFSET, nVIC_BOTH_EDGE_OFFSET;

    if (irqnr < VIC2_IRQ_START)                                                                                                                       
    {
        nMask = (1 << irqnr);
	nVICVA = VIC_BASE_VA;		/* switch to VIC1 */
	nVIC_SENSE_OFFSET = ASPEED_VIC_SENSE_OFFSET;
	nVIC_EVENT_OFFSET = ASPEED_VIC_EVENT_OFFSET;
	nVIC_BOTH_EDGE_OFFSET = ASPEED_VIC_BOTH_EDGE_OFFSET;	
    }
#if (CONFIG_AST1500_SOC_VER <= 2)
    else
    {
        nMask = (1 << (irqnr - VIC2_IRQ_SHIFT));
	nVICVA = VIC2_BASE_VA;		/* switch to VIC1 */
	nVIC_SENSE_OFFSET = ASPEED_VIC2_SENSE_OFFSET;
	nVIC_EVENT_OFFSET = ASPEED_VIC2_EVENT_OFFSET;
	nVIC_BOTH_EDGE_OFFSET = ASPEED_VIC2_BOTH_EDGE_OFFSET;	
    }    
#endif

    /* edge or level trigger */
    nRegValue = UMVP_READ_REG(nVICVA + nVIC_SENSE_OFFSET);
    if (mode == vicc_edge_activeFalling || mode == vicc_edge_activeRaising ||
        mode == vicc_edge_activeBoth)
    {
        /* edge trigger : corresponding bit shall be reset as 0 */
        nRegValue &= (~nMask);
    }
    else
    {
        /* level trigger : corresponding bit shall be set as 1 */
        nRegValue |= nMask;
    }
    UMVP_WRITE_REG(nVICVA + nVIC_SENSE_OFFSET, nRegValue);



    /* active high or active low */
    nRegValue = UMVP_READ_REG(nVICVA + nVIC_EVENT_OFFSET);
    if (mode == vicc_edge_activeFalling || mode == vicc_level_activeLow)
    {
        /* active low : corresponding bit shall be reset as 0 */
        nRegValue &= (~nMask);
    }
    else if (mode == vicc_edge_activeRaising || mode == vicc_level_activeHigh)
    {
        /* active high : corresponding bit shall be set as 1 */
        nRegValue |= nMask;
    }
    UMVP_WRITE_REG(nVICVA + nVIC_EVENT_OFFSET, nRegValue);



    /* both edge? */
    nRegValue = UMVP_READ_REG(nVICVA + nVIC_BOTH_EDGE_OFFSET);
    if (mode == vicc_edge_activeBoth)
    {
        /* both edge : corresponding bit shall be set as 1 */
        nRegValue |= nMask;
    }
    else
    {
        /* single edge : corresponding bit shall be reset as 0 */
        nRegValue &= (~nMask);
    }
    UMVP_WRITE_REG(nVICVA + nVIC_BOTH_EDGE_OFFSET, nRegValue);
#endif
}

#if (3 <= CONFIG_AST1500_SOC_VER)
static void vic_enable_intr_1520(int irq)
{
	u32 val, reg, mask;

	reg = ASPEED_VIC_ENABLE_SET_OFFSET;
	mask = 1 << (irq % 32);

	if (irq > 31)
		reg += 4;

	val = READ_VIC_REG(reg);
	val |= mask;
	WRITE_VIC_REG(reg, val);
}
#endif

static void vic_enable_intr(int irq)
{
#if (3 <= CONFIG_AST1500_SOC_VER)
	vic_enable_intr_1520(irq);
#else
	register unsigned int regVal;

	if (irq < VIC2_IRQ_START)
	{
		regVal = UMVP_READ_REG(VIC_BASE_VA + ASPEED_VIC_ENABLE_SET_OFFSET);
		regVal |= (1 << irq);
		UMVP_WRITE_REG(VIC_BASE_VA + ASPEED_VIC_ENABLE_SET_OFFSET, regVal);
	}
#if (CONFIG_AST1500_SOC_VER <= 2)
	else
	{
		regVal = UMVP_READ_REG(VIC2_BASE_VA + ASPEED_VIC2_ENABLE_SET_OFFSET);
		regVal |= (1 << (irq - VIC2_IRQ_SHIFT));
		UMVP_WRITE_REG(VIC2_BASE_VA + ASPEED_VIC2_ENABLE_SET_OFFSET, regVal);
		
	        vic_enable_intr(IRQ_VIC2);		
	}	
#endif
#endif
}

#if (3 <= CONFIG_AST1500_SOC_VER)
static void vic_disable_intr_1520(int irq)
{
	u32 val, reg, mask;

	reg = ASPEED_VIC_ENABLE_CLEAR_OFFSET;
	mask = 1 << (irq % 32);

	if (irq > 31)
		reg += 4;

	val = READ_VIC_REG(reg);
	val |= mask;
	WRITE_VIC_REG(reg, val);
}
#endif

static void vic_disable_intr(int irq)
{
#if (3 <= CONFIG_AST1500_SOC_VER)
	vic_disable_intr_1520(irq);
#else
	register unsigned int regVal;

	if (irq < VIC2_IRQ_START)
	{
		regVal = UMVP_READ_REG(VIC_BASE_VA + ASPEED_VIC_ENABLE_CLEAR_OFFSET);
		regVal |= (1 << irq);
		UMVP_WRITE_REG(VIC_BASE_VA + ASPEED_VIC_ENABLE_CLEAR_OFFSET, regVal);
	}
#if (CONFIG_AST1500_SOC_VER <= 2)
	else
	{
		regVal = UMVP_READ_REG(VIC2_BASE_VA + ASPEED_VIC2_ENABLE_CLEAR_OFFSET);
		regVal &= ~(1 << (irq - VIC2_IRQ_SHIFT));
		UMVP_WRITE_REG(VIC2_BASE_VA + ASPEED_VIC2_ENABLE_CLEAR_OFFSET, regVal);
		
		UMVP_WRITE_REG(VIC2_BASE_VA + ASPEED_VIC2_EDGE_CLEAR_OFFSET, (1 << (irq - VIC2_IRQ_SHIFT)));
		
		vic_disable_intr(IRQ_VIC2);
	}	
#endif
#endif
}

#if (3 <= CONFIG_AST1500_SOC_VER)
static void vic_clear_intr_1520(int irq)
{
	u32 reg, mask;

	reg = ASPEED_VIC_EDGE_CLEAR_OFFSET;
	mask = 1 << (irq % 32);

	if (irq > 31)
		reg += 4;

	WRITE_VIC_REG(reg, mask);
}
#endif

void vic_clear_intr(int irq)
{
#if (3 <= CONFIG_AST1500_SOC_VER)
	vic_clear_intr_1520(irq);
#else
	if (irq < VIC2_IRQ_START)
	{
		UMVP_WRITE_REG(VIC_BASE_VA + ASPEED_VIC_EDGE_CLEAR_OFFSET, (1 << irq));
	}
#if (CONFIG_AST1500_SOC_VER <= 2)
	else
	{
		UMVP_WRITE_REG(VIC2_BASE_VA + ASPEED_VIC2_EDGE_CLEAR_OFFSET, (1 << (irq - VIC2_IRQ_SHIFT)));
	}	
#endif
#endif
}

static void sc_mask_irq(unsigned int irq)
{
	vic_disable_intr(irq);
}

static void sc_unmask_irq(unsigned int irq)
{
	vic_enable_intr(irq);
}
 
static struct irqchip sc_chip = {
	.ack	= sc_mask_irq,
	.mask	= sc_mask_irq,
	.unmask = sc_unmask_irq,
}; 
 
void __init aspeed_init_irq(void)
{
	unsigned int i;

	vic_reset();

	for (i = 0; i < NR_IRQS; i++) {
		if (i < VIC2_IRQ_START) {
			/* apply VIC1 mask (VIC means VIC1) */
			if ((1 << i) & ASPEED_VIC_VALID_INTMASK) {
				set_irq_chip(i, &sc_chip);
				set_irq_handler(i, do_level_IRQ);
				set_irq_flags(i, IRQF_VALID);
			}
		} else {
			/* apply VIC2 mask (VIC means VIC2) */
			if ((1 << (i - VIC2_IRQ_SHIFT))
				& ASPEED_VIC2_VALID_INTMASK) {
				set_irq_chip(i, &sc_chip);
				set_irq_handler(i, do_level_IRQ);
				set_irq_flags(i, IRQF_VALID);
			}			
		}	
	}
#if (CONFIG_AST1500_SOC_VER <= 2)
        /* enable VIC2 */
	vic_set_intr_trigger(IRQ_VIC2, vicc_level_activeHigh);
	vic_enable_intr(IRQ_VIC2);
#endif
	/* enable INT_VIC2IRQ on VIC1 for VIC2 be able to launch interrupt */
	/* vic_set_intr_trigger(INT_VIC2IRQ, vicc_level_activeLow);  */


	/* note that : VIC1 needs to be programmed to launch IRQ to CPU for INT_VIC2IRQ */
	/*  and this has been done in the vic_reset(). */

	/* Enable interrupt on VIC1 for INT_VIC2IRQ. */
	/* Please note that you can't use "enable_irq()" to enable this interrupt */
	/* since the program will not generate this interrupt. Please refer to */
	/* "entry-macro.S". */
	/* vic_enable_intr(INT_VIC2IRQ); */
}


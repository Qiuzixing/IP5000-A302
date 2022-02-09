/*
 * include/asm-arm/arch-umvp/entry-macro.S
 *
 * Low-level IRQ helper macros for GUC-MVP2000 platforms
 *
 * This file is licensed under the terms of the GNU General Public
 * License version 2. This program is licensed "as is" without any
 * warranty of any kind, whether express or implied.
 */
#include <asm/hardware.h>
#include <asm/arch/irqs.h>
#define AST_INTR_RR

		.macro	disable_fiq
		.endm

#if (3 <= CONFIG_AST1500_SOC_VER)
		.macro  get_irqnr_and_base, irqnr, irqstat, base, tmp
#if defined(AST_INTR_RR)
/*

	Pseudo code:
	MAIN() {
		irqnr = last irq
		if (irqnr >= 31) {
		        if (STATUS_H) {
        		        irqstat = STATUS_H
				irq_fix = 32
	        	} else {
                		irqstat = STATUS_L
		                irqnr = 31
				irq_fix = 0
	        	}
		} else {
		        if (STATUS_L) {
        		        irqstat = STATUS_L
				irq_fix = 0
		        } else {
        		        read STATUS_H
                		irqstat = STATUS_H
	                	irqnr = 31	=> pick from beginning
				irq_fix = 32
		        }
		}
		Call irq_rr(irqstat, irqnr, irq_fix)
	}

	irq_rr(state, last_nr, base) is original code for RR intr num.
 */
                ldr     \irqstat, =IO_ADDRESS(ASPEED_SCU_BASE + 0x44)
                ldr     \irqstat, [\irqstat]
                and     \irqnr, \irqnr, #63                             @ get last irq

                cmp     \irqnr, #31
                blt     1001f
                b       1003f
1001:							@ last irq < 31

                ldr     \irqstat, =IO_ADDRESS(ASPEED_VIC_BASE + ASPEED_VIC_STATUS_OFFSET)
                ldr     \irqstat, [\irqstat]            @ get interrupts
                cmp     \irqstat, #0
                beq     1002f                           @ upper IRQ?
                mov     \base, #0
                b       1005f
1002:
                ldr     \irqstat, =IO_ADDRESS(ASPEED_VIC_BASE + ASPEED_VIC_STATUS_OFFSET_H)
                ldr     \irqstat, [\irqstat]            @ get interrupts
                mov     \irqnr, #31
                mov     \base, #32
                b       1005f
1003:							@ lasr irq >= 31
                ldr     \irqstat, =IO_ADDRESS(ASPEED_VIC_BASE + ASPEED_VIC_STATUS_OFFSET_H)
                ldr     \irqstat, [\irqstat]            @ get interrupts
                cmp     \irqstat, #0
                beq     1004f
                mov     \base, #32
                b       1005f
1004:
                ldr     \irqstat, =IO_ADDRESS(ASPEED_VIC_BASE + ASPEED_VIC_STATUS_OFFSET)
                ldr     \irqstat, [\irqstat]            @ get interrupts
                mov     \irqnr, #31
                mov     \base, #0
                b       1005f
1005:
                mov     \irqstat, \irqstat, ror \irqnr
                clz     \tmp, \irqstat
                cmp     \tmp, #32
                beq     1006f

                add     \irqnr, \irqnr, #31
                sub     \irqnr, \irqnr, \tmp
                and     \irqnr, \irqnr, #31
                add     \irqnr, \irqnr, \base
1006:
                ldr     \tmp, =IO_ADDRESS(ASPEED_SCU_BASE + 0x44)
                str     \irqnr, [\tmp]
#else

		ldr     \irqstat, =IO_ADDRESS(ASPEED_VIC_BASE + ASPEED_VIC_STATUS_OFFSET)
		ldr     \irqstat, [\irqstat]            @ get interrupts
		cmp     \irqstat, #0
		beq     1001f                           @ upper IRQ?
		clz     \irqnr, \irqstat
		mov     \base, #31
		sub     \irqnr, \base, \irqnr
		b       1002f                           @ lower IRQ being handled
1001:
		ldr     \irqstat, =IO_ADDRESS(ASPEED_VIC_BASE + ASPEED_VIC_STATUS_OFFSET_H)
		ldr     \irqstat, [\irqstat]            @ get upper interrupts
		mov     \irqnr, #63
		clz     \irqstat, \irqstat
		cmp     \irqstat, #32
		subne   \irqnr, \irqnr, \irqstat
1002:
#endif
		.endm
#else
#if 1

#if defined(CONFIG_ARCH_AST1500_CLIENT)
		.macro	get_irqnr_and_base, irqnr, irqstat, base, tmp
/* FIXME: should not be using soo many LDRs here */
		ldr	\irqnr, =IO_ADDRESS(ASPEED_VIC_BASE)
		ldr	\irqstat, [\irqnr, #ASPEED_VIC_STATUS_OFFSET]		@ get masked status
		
		ldr	\tmp, =IO_ADDRESS(ASPEED_SCU_BASE)
		ldr	\irqnr, [\tmp, #0x44]
                and     \irqnr, \irqnr, #31
                mov     \irqstat, \irqstat, ror \irqnr 
                clz     \tmp, \irqstat
                cmp     \tmp, #32
                beq     1005f
                
                add     \irqnr, \irqnr, #31
                sub     \irqnr, \irqnr, \tmp
		and     \irqnr, \irqnr, #31
		
		ldr	\tmp, =IO_ADDRESS(ASPEED_SCU_BASE)
		str	\irqnr, [\tmp, #0x44]
#if (CONFIG_AST1500_SOC_VER <= 2)
1002:		/* VIC2 */
		cmp	\irqnr, #IRQ_VIC2
		bne	1005f
	
		ldr	\base, =IO_ADDRESS(ASPEED_GPIO_BASE)
		ldr	\irqstat, [\base, #ASPEED_VIC2_STATUS_OFFSET]		@ get masked status
#if		defined(CONFIG_ARCH_AST3000FPGA_CLIENT)
		mov	\irqstat, \irqstat, lsr #24				@ GPIO H
#elif	(CONFIG_AST1500_SOC_VER == 2)
		mov	\irqstat, \irqstat, lsr #4				@ GPIO A4
#else //CONFIG_AST1500_SOC_VER == 1
		mov	\irqstat, \irqstat, lsr #8				@ GPIO B
#endif		
		mov	\tmp, #32						@ IRQ Number
1003:  		tst	\irqstat, #1
		bne	1004f
		add	\tmp, \tmp, #1
		mov	\irqstat, \irqstat, lsr #1
		cmp	\tmp, #36
		blt	1003b
				
		/* Fixed GPIO IRQ can't work porperly in client mode, ycchen@090308 */		
		mov	\irqnr, #IRQ_VIC2
		cmp	\irqnr, #32
		b	1005f
		
1004:  		mov	\irqnr, \tmp

#else //#if (CONFIG_AST1500_SOC_VER <= 2)
		cmp	\irqnr, #32
#endif

1005:		/* done */
		.endm
#else
		.macro	get_irqnr_and_base, irqnr, irqstat, base, tmp
/* FIXME: should not be using soo many LDRs here */
		ldr	\irqnr, =IO_ADDRESS(ASPEED_VIC_BASE)
		ldr	\irqstat, [\irqnr, #ASPEED_VIC_STATUS_OFFSET]		@ get masked status
		
		ldr	\tmp, =IO_ADDRESS(ASPEED_SCU_BASE)
		ldr	\irqnr, [\tmp, #0x44]
                and     \irqnr, \irqnr, #31
                mov     \irqstat, \irqstat, ror \irqnr 
                clz     \tmp, \irqstat
                cmp     \tmp, #32
                beq     1001f
                
                add     \irqnr, \irqnr, #31
                sub     \irqnr, \irqnr, \tmp
		and     \irqnr, \irqnr, #31
		
		ldr	\tmp, =IO_ADDRESS(ASPEED_SCU_BASE)
		str	\irqnr, [\tmp, #0x44]
                cmp     \irqnr, #32

#if 0
                mov    \irqnr, #31
                clz    \tmp, \irqstat
                cmp    \tmp, #32
                beq     1001f 
                sub    \irqnr, \irqnr, \tmp
                cmp    \irqnr, #32
#endif
1001:		/* done */
		.endm
#endif

#else

#if defined(CONFIG_ARCH_AST1500_CLIENT)
		.macro	get_irqnr_and_base, irqnr, irqstat, base, tmp
/* FIXME: should not be using soo many LDRs here */
		ldr	\irqnr, =IO_ADDRESS(ASPEED_VIC_BASE)
		ldr	\irqstat, [\irqnr, #ASPEED_VIC_STATUS_OFFSET]		@ get masked status

		/* VIC1 */
		mov	\irqnr, #0
1001:		tst	\irqstat, #1
		bne	1002f
		add	\irqnr, \irqnr, #1
		mov	\irqstat, \irqstat, lsr #1
		cmp	\irqnr, #31
		bcc	1001b
		b	1005f
		
1002:		/* VIC2 */
		cmp	\irqnr, #IRQ_VIC2
		bne	1005f
	
		ldr	\base, =IO_ADDRESS(ASPEED_GPIO_BASE)
		ldr	\irqstat, [\base, #ASPEED_VIC2_STATUS_OFFSET]		@ get masked status
#if		defined(CONFIG_ARCH_AST3000FPGA_CLIENT)
		mov	\irqstat, \irqstat, lsr #24				@ GPIO H
#else		
		mov	\irqstat, \irqstat, lsr #8				@ GPIO B
#endif		
		mov	\tmp, #32						@ IRQ Number
1003:  		tst	\irqstat, #1
		bne	1004f
		add	\tmp, \tmp, #1
		mov	\irqstat, \irqstat, lsr #1
		cmp	\tmp, #36
		blt	1003b
		b	1005f
1004:  		mov	\irqnr, \tmp
1005:		/* done */
		.endm
#else
		.macro	get_irqnr_and_base, irqnr, irqstat, base, tmp
/* FIXME: should not be using soo many LDRs here */
		ldr	\irqnr, =IO_ADDRESS(ASPEED_VIC_BASE)
		ldr	\irqstat, [\irqnr, #ASPEED_VIC_STATUS_OFFSET]		@ get masked status

		mov	\irqnr, #0
1001:		tst	\irqstat, #1
		bne	1002f
		add	\irqnr, \irqnr, #1
		mov	\irqstat, \irqstat, lsr #1
		cmp	\irqnr, #31
		bcc	1001b
1002:		/* EQ will be set if we reach 31 */
		.endm
#endif

#endif
#endif


		.macro	irq_prio_table
		.endm


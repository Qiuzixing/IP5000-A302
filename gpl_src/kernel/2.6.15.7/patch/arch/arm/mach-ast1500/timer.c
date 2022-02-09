/*
 *  linux/arch/arm/arch-ast2000/timer.c
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
 
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/spinlock.h>
#include <linux/interrupt.h>
#include <linux/sched.h>

#include <asm/system.h>
#include <asm/io.h>
#include <asm/arch/hardware.h>
#include <asm/arch/irqs.h>
#include <asm/arch/time.h>

/*
 * Returns number of ms since last clock interrupt.  Note that interrupts
 * will have been disabled by do_gettimeoffset()
 */
static unsigned long aspeed_gettimeoffset(void)
{
	volatile TimerStruct_t *timer0 = (TimerStruct_t *) ASPEED_TIMER0_VA_BASE;
	unsigned long ticks1, ticks2, status;

	/*
	 * Get the current number of ticks.  Note that there is a race
	 * condition between us reading the timer and checking for
	 * an interrupt.  We get around this by ensuring that the
	 * counter has not reloaded between our two reads.
	 */
	ticks2 = timer0->TimerValue;
	do {
		ticks1 = ticks2;
		status = __raw_readl(IO_ADDRESS(ASPEED_VIC_BASE) + ASPEED_VIC_RAW_STATUS_OFFSET);
		ticks2 = timer0->TimerValue;
	} while (ticks2 > ticks1);

	/*
	 * Number of ticks since last interrupt.
	 */
	ticks1 = TIMER_RELOAD - ticks2;

	/*
	 * Interrupt pending?  If so, we've reloaded once already.
	 */
	if (status & IRQMASK_TIMERINT0)
		ticks1 += TIMER_RELOAD;

	/*
	 * Convert the ticks to usecs
	 */
	return TICKS2USECS(ticks1);
}


/*
 * IRQ handler for the timer
 */
static irqreturn_t 
aspeed_timer_interrupt(int irq, void *dev_id, struct pt_regs *regs)
{

	write_seqlock(&xtime_lock);

	/*
	 * clear the interrupt
	 */
	IRQ_EDGE_CLEAR(IRQ_TIMERINT0);

	/*
	 * the clock tick routines are only processed on the
	 * primary CPU
	 */
	if (hard_smp_processor_id() == 0) {
		timer_tick(regs);
#ifdef CONFIG_SMP
		smp_send_timer();
#endif
	}

#ifdef CONFIG_SMP
	/*
	 * this is the ARM equivalent of the APIC timer interrupt
	 */
	update_process_times(user_mode(regs));
#endif /* CONFIG_SMP */

	write_sequnlock(&xtime_lock);

	return IRQ_HANDLED;	
}

static struct irqaction aspeed_timer_irq = {
	.name		= "ASPEED Timer Tick",
	.flags		= SA_INTERRUPT | SA_TIMER,
	.handler	= aspeed_timer_interrupt,
};


/*
 * Set up timer interrupt, and return the current time in seconds.
 */
static void aspeed_setup_timer(void)
{
	volatile TimerStruct_t *timer0 = (volatile TimerStruct_t *) ASPEED_TIMER0_VA_BASE;
        volatile __u32         *timerc = (volatile __u32*) ASPEED_TIMERC_VA_BASE;

	/*
	 * Initialise to a known state (all timers off)
	 */
        *timerc = 0;

	timer0->TimerLoad    = TIMER_RELOAD - 1;
	timer0->TimerValue   = TIMER_RELOAD - 1;
	*timerc              = TIMER0_ENABLE | TIMER0_RefExt;

	/* 
	 * Make irqs happen for the system timer
	 */
	setup_irq(IRQ_TIMERINT0, &aspeed_timer_irq);
	IRQ_SET_RISING_EDGE(IRQ_TIMERINT0);

}

struct sys_timer aspeed_timer = {
	.init		= aspeed_setup_timer,
	.offset		= aspeed_gettimeoffset,
};

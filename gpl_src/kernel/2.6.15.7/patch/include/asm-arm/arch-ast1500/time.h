/*
 *  linux/include/asm-arm/arch-ast2000/time.h
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
#include <asm/system.h>
#include <asm/mach/time.h>
#include <asm/param.h>

/*
 * How long is the timer interval?
 */
#define TIMER_INTERVAL	(ASPEED_TIMER_CLKRATE / HZ)
#define TIMER_RELOAD	(TIMER_INTERVAL)
#define TICKS2USECS(x)	((x) / TICKS_PER_uSEC)

/*
 * Timer Control
 */
#define TIMER0_ENABLE   0x0001
#define TIMER1_ENABLE   0x0010
#define TIMER2_ENABLE   0x0100

#define TIMER0_RefExt   0x0002
#define TIMER1_RefExt   0x0020
#define TIMER2_RefExt   0x0200

/*
 * What does it look like?
 */
typedef struct TimerStruct {
	unsigned long TimerValue;
	unsigned long TimerLoad;
	unsigned long TimerMatch1;
	unsigned long TimerMatch2;
} TimerStruct_t;


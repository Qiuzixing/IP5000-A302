/* ---for mm.c include -----*/
#include <linux/ioctl.h>
#include <asm/arch/platform.h>

#define UART_CLKSRC_EXT (1)
#define SOCLE_UP_BASE_BAUD (24000000 >> 4)
//#define INO_UART0 1

//#define UP_UART0_REGS_BASE 0xe0040000
//#define UP_UART1_REGS_BASE 0xe0060000
#define UP_UART0_REGS_BASE 0xE0040000
#define UP_UART1_REGS_BASE 0xE0060000

#define UART_CLKSRC_CTRL 0x20
#define UART_TX_FIFO_SIZE 16
#define UART0_IRDA_CTRL 0x1c
/* -- end mm.c ---- */
//#define TIMER0_VA_BASE IO_ADDRESS(0x1E800000)
//#define TIMER1_VA_BASE IO_ADDRESS(0x1E800010)
//#define TIMER2_VA_BASE IO_ADDRESS(0x1E800020)
#define TIMER0_VA_BASE (0xE0000000)
#define TIMER1_VA_BASE (0xE0000010)
#define TIMER2_VA_BASE (0xE0000020)

//#define TIMER_INTERVAL	(240000) 
#define TIMER_INTERVAL	(500000) 
#define TIMER_RELOAD	(TIMER_INTERVAL)		/* Divide by 256 */
#define TICKS2USECS(x)	(x/50)
/*--- kernel/time.c include ---*/
//#define SOCLEMAC_ETH0_ADDR (0xF0080000)
//#define SOCLEMAC_ETH1_ADDR (0xF00A0000)
#define SOCLEMAC_ETH0_ADDR IO_ADDRESS(ASPEED_MAC_BASE)
//#define SOCLEMAC_ETH1_ADDR (0xF00A0000)
/*---- for emac driver include -----*/
//yriver
//#define UP_RTC_REGS_BASE (0xE0020000)
/* ------ +grant ---------*/
//yriver
/*
#define UP_SPI_REGS_BASE	0xf00c0000
#define UP_GPIO_REGS_BASE	0xf00e0000
#define UP_WTDOG_REGS_BASE	0xf0100000
*/
/* ------ +grant ---------*/
#define UP_GPIO_PORTA		(0)
#define UP_GPIO_PORTB		(1)
#define UP_GPIO_PORTS		(2)

#define GPIO_OFF_PADR		(0x0)
#define GPIO_OFF_PADIR		(0x4)
#define GPIO_OFF_PBDR		(0x8)
#define GPIO_OFF_PBDIR		(0xC)
//yriver
/*
#define UP_GPIO_PADR		(UP_GPIO_REGS_BASE+GPIO_OFF_PADR)
#define UP_GPIO_PADIR		(UP_GPIO_REGS_BASE+GPIO_OFF_PADIR)
#define UP_GPIO_PBDR		(UP_GPIO_REGS_BASE+GPIO_OFF_PBDR)
#define UP_GPIO_PBDIR		(UP_GPIO_REGS_BASE+GPIO_OFF_PBDIR)
*/

/*
* Bit definition used in DIR setting.
*/
#define GPIO_IN		(0)
#define GPIO_OUT	(1)
/* +grant temp mark
struct uP_gpio_ioctl_data {
	unsigned int device;
	unsigned int mask;
	unsigned int data;
};
*/
#define	DRV_GPIO		(185)
#define	uP_gpio_IN		_IOW(DRV_GPIO, 0, struct uP_gpio_ioctl_data)
#define	uP_gpio_OUT		_IOR(DRV_GPIO, 1, struct uP_gpio_ioctl_data)
#define	uP_gpio_DIR		_IOR(DRV_GPIO, 2, struct uP_gpio_ioctl_data)

/**********************************************************************
 * register summary
 **********************************************************************/
//yriver
/*
#define RTC_TIME		(UP_RTC_REGS_BASE+0x0)
#define RTC_DATE		(UP_RTC_REGS_BASE+0x4)
#define RTC_TALRM		(UP_RTC_REGS_BASE+0x8)
#define RTC_DALRM		(UP_RTC_REGS_BASE+0xC)
#define RTC_CTRL		(UP_RTC_REGS_BASE+0x10)
*/
#define RTC_CLK_FREQ	(32768)
#define RTC_DIVIDER		((RTC_CLK_FREQ / 2 / 16) -1)
/*
 * Bits to be shifted in date/time register
 */
#define UP_RTC_1_OVER_16_SHIFT		(0)
#define UP_RTC_SEC_SHIFT			(4)
#define UP_RTC_10SEC_SHIFT			(8)
#define UP_RTC_MIN_SHIFT			(11)
#define UP_RTC_10MIN_SHIFT			(15)
#define UP_RTC_HOUR_SHIFT			(18)
#define UP_RTC_10HOUR_SHIFT			(22)
#define UP_RTC_DOW_SHIFT			(24)

#define UP_RTC_DAY_SHIFT			(0)
#define UP_RTC_10DAY_SHIFT			(4)
#define UP_RTC_MON_SHIFT			(6)
#define UP_RTC_10MON_SHIFT			(10)
#define UP_RTC_YEAR_SHIFT			(11)
#define UP_RTC_10YEAR_SHIFT			(15)
#define UP_RTC_100YEAR_SHIFT		(19)
#define UP_RTC_1000YEAR_SHIFT		(23)
/*
 * Bits in the control register
 */
#define RTC_ENABLE		(1 << 31)

#define YEAR_1(reg)		(((reg) >> UP_RTC_YEAR_SHIFT) & 0x0f)
#define YEAR_10(reg)	(((reg) >> UP_RTC_10YEAR_SHIFT) & 0x0f)
#define YEAR_100(reg)	(((reg) >> UP_RTC_100YEAR_SHIFT) & 0x0f)
#define YEAR_1000(reg)	(((reg) >> UP_RTC_1000YEAR_SHIFT) & 0x0f)


#define FEBRUARY		2
#define STARTOFTIME		1970
#define SECDAY			86400L
#define SECYR			(SECDAY * 365)
#define leapyear(year)		((year) % 4 == 0)
#define days_in_year(a)		(leapyear(a) ? 366 : 365)
#define days_in_month(a)	(month_days[(a) - 1])

/*-----+grant end rtc.c include -----------------*/
#define MIN_PRESCALE_SHIFT		(0)		/* 1/1 */
#define MAX_PRESCALE_SHIFT		(8)		/* 1/256 */
#define DISALLOWED_PRESCALE		(1)		/* 1/2 */
#define UP_TMR_MAX_COUNT  		((TMR_REG) (-1))

/*
 * uPlatform WATCHDOG register definition
 */
#define WTDOG_OFF_LR		(0x0)
#define WTDOG_OFF_CVR		(0x4)
#define WTDOG_OFF_CON		(0x8)

#define UP_WTDOG_MAX_LR		(0xffffffff) /* 32-bit load register */
#define	MAX_PRESCALE_SHIFT	(8)	/* Max. prescale 1/256 */
#define	UP_WTDOG_DISALLOWED_PRESCALE		(1)

//yriver
/*
#define UP_WTDOG_LR		(UP_WTDOG_REGS_BASE+WTDOG_OFF_LR)
#define UP_WTDOG_CVR		(UP_WTDOG_REGS_BASE+WTDOG_OFF_CVR)
#define UP_WTDOG_CON		(UP_WTDOG_REGS_BASE+WTDOG_OFF_CON)
*/
/*
 * uPlatform WATCHDOG CON register definition
 */
#define WTDOG_ENABLE		(1<<3)
#define WTDOG_RESET_ENABLE	(1<<4)


/*------ +grant wdt def             ------------- */




//yriver
//#define VA_IC_BASE              0xE19C40000
#define ALLPCI ( (1 << IRQ_PCIINT0) | (1 << IRQ_PCIINT1) | (1 << IRQ_PCIINT2) | (1 << IRQ_PCIINT3) )
/* +grant define for scole lp2*/
#define INTC_MAX_IRQ  NR_IRQS
#define UP_INTC_BASE VA_IC_BASE

#define UP_IRQ_BASE  (0)
#define INTC_REG_SIZE	        (4)
/* value definition of source control register */
#define INTC_TRIG_SHIFT		(6)
#define INTC_TRIG_LOW_LVL	(0)
#define INTC_TRIG_HIGH_LVL	(1)
#define INTC_TRIG_NEG_EDGE	(2)
#define INTC_TRIG_POS_EDGE	(3)

#define INTC_PRIO_MASK		(0x07)
#define INTC_MAX_PRIOIRTY	(7)

#define INO_SPI			(UP_IRQ_BASE+0)
#define INTC_TRIG_0		(INTC_TRIG_HIGH_LVL)
#define INTC_PRIO_0		(3)	/* SPI IRQ */

#define INO_UART0		(UP_IRQ_BASE+1)
#define INTC_TRIG_1		(INTC_TRIG_HIGH_LVL)
#define INTC_PRIO_1		(3)	/* UART 0 IRQ */

#define INO_UART1		(UP_IRQ_BASE+2)
#define INTC_TRIG_2		(INTC_TRIG_HIGH_LVL)
#define INTC_PRIO_2		(3)	/* UART 1 IRQ */

#define INO_TIMER0		(UP_IRQ_BASE+3)
#define INTC_TRIG_3		(INTC_TRIG_HIGH_LVL)
#define INTC_PRIO_3		(4)	/* Timer 0 IRQ */

#define INO_TIMER1		(UP_IRQ_BASE+4)
#define INTC_TRIG_4		(INTC_TRIG_HIGH_LVL)
#define INTC_PRIO_4		(4)	/* Timer 1 IRQ */

#define INO_TIMER2		(UP_IRQ_BASE+5)
#define INTC_TRIG_5		(INTC_TRIG_HIGH_LVL)
#define INTC_PRIO_5		(4)	/* Timer 2 IRQ */

#define INO_RTC			(UP_IRQ_BASE+6)
#define INTC_TRIG_6		(INTC_TRIG_HIGH_LVL)
#define INTC_PRIO_6		(2)	/* RTC IRQ */


#define INO_MAC0		(UP_IRQ_BASE+7)
#define INTC_TRIG_7		(INTC_TRIG_HIGH_LVL)
#define INTC_PRIO_7		(2)	/* MAC0 IRQ */


#define INO_MAC1		(UP_IRQ_BASE+8)
#define INTC_TRIG_8		(INTC_TRIG_HIGH_LVL)
#define INTC_PRIO_8		(2)	/* MAC1 IRQ */

#define INO_UDC			(UP_IRQ_BASE+9)
#define INTC_TRIG_9		(INTC_TRIG_HIGH_LVL)
#define INTC_PRIO_9		(2)	/* UDC IRQ */

#define INO_EXT0		(UP_IRQ_BASE+10)
#define INTC_TRIG_10		(INTC_TRIG_LOW_LVL)
#define INTC_PRIO_10		(3)	/* external IRQ 0 */

#define INO_EXT1		(UP_IRQ_BASE+11)
#define INTC_TRIG_11		(INTC_TRIG_LOW_LVL)
#define INTC_PRIO_11		(3)	/* external IRQ 1 */

#define INO_EXT2		(UP_IRQ_BASE+12)
#define INTC_TRIG_12		(INTC_TRIG_LOW_LVL)
#define INTC_PRIO_12		(3)	/* external IRQ 2 */

#define INO_EXT3		(UP_IRQ_BASE+13)
#define INTC_TRIG_13		(INTC_TRIG_LOW_LVL)
#define INTC_PRIO_13		(3)	/* external IRQ 3 */

#define INO_EXT4		(UP_IRQ_BASE+14)
#define INTC_TRIG_14		(INTC_TRIG_LOW_LVL)
#define INTC_PRIO_14		(3)	/* external IRQ 4*/

#define INO_EXT5		(UP_IRQ_BASE+15)
#define INTC_TRIG_15		(INTC_TRIG_POS_EDGE)
#define INTC_PRIO_15		(3)	/* external IRQ 5*/

#define INO_EXT6		(UP_IRQ_BASE+16)
#define INTC_TRIG_16		(INTC_TRIG_NEG_EDGE)
#define INTC_PRIO_16		(3)	/* external IRQ 6*/

#define INO_EXT7		(UP_IRQ_BASE+17)
#define INTC_TRIG_17		(INTC_TRIG_HIGH_LVL) 
#define INTC_PRIO_17		(3)	/* external IRQ 7 +grant*/

#define INO_EXT8		(UP_IRQ_BASE+18)
#define INTC_TRIG_18		(INTC_TRIG_LOW_LVL)
#define INTC_PRIO_18		(3)	/* external IRQ 8 */

#define INO_EXT9		(UP_IRQ_BASE+19)
#define INTC_TRIG_19		(INTC_TRIG_LOW_LVL)
#define INTC_PRIO_19		(3)	/* external IRQ 9 */

#define INO_EXT10		(UP_IRQ_BASE+20)
#define INTC_TRIG_20		(INTC_TRIG_POS_EDGE)
#define INTC_PRIO_20		(3)	/* external IRQ 10*/

#define INO_EXT11		(UP_IRQ_BASE+21)
#define INTC_TRIG_21		(INTC_TRIG_POS_EDGE)
#define INTC_PRIO_21		(3)	/* external IRQ 11*/

#define INO_EXT12		(UP_IRQ_BASE+22)
#define INTC_TRIG_22		(INTC_TRIG_NEG_EDGE)
#define INTC_PRIO_22		(3)	/* external IRQ 12*/

#define INO_4K_WBERR		(UP_IRQ_BASE+23)
#define INTC_TRIG_23		(INTC_TRIG_POS_EDGE)
#define INTC_PRIO_23		(3)	/* wrapper bus error */



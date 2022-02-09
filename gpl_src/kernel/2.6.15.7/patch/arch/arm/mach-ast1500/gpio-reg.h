#ifndef __GPIO_REG_H__
#define __GPIO_REG_H__

#include <asm/arch/platform.h>
#include <asm/arch/hardware.h>


/* register address for the GPIO registers. */
#define AST1500_GPIOREG(x) ((x) + IO_ADDRESS(ASPEED_GPIO_BASE))

#if (CONFIG_AST1500_SOC_VER >= 3)
#define AST1500_GPIO_BASE(pin) \
	((pin < AST1500_GPE0) ? (AST1500_GPIOREG(0)) : \
	(pin < AST1500_GPI0) ? (AST1500_GPIOREG(0x20)) : \
	(pin < AST1500_GPM0) ? (AST1500_GPIOREG(0x70)) : \
	(pin < AST1500_GPQ0) ? (AST1500_GPIOREG(0x78)) : \
	(pin < AST1500_GPU0) ? (AST1500_GPIOREG(0x80)) : \
	(pin < AST1500_GPY0) ? (AST1500_GPIOREG(0x88)) : \
	(pin < AST1500_GPAC0) ? (AST1500_GPIOREG(0x1E0)) : \
	(AST1500_GPIOREG(0x1E8)))

#elif (CONFIG_AST1500_SOC_VER == 2)
#define AST1500_GPIO_BASE(pin) \
	((pin < AST1500_GPE0) ? (AST1500_GPIOREG(0)) : \
	(pin < AST1500_GPI0) ? (AST1500_GPIOREG(0x20)) : \
	(pin < AST1500_GPM0) ? (AST1500_GPIOREG(0x70)) : \
	(pin < AST1500_GPQ0) ? (AST1500_GPIOREG(0x78)) : \
	(AST1500_GPIOREG(0x80)))

#else
#define AST1500_GPIO_BASE(pin)  \
	(((pin) < AST1500_GPE0)?(AST1500_GPIOREG(0)):(AST1500_GPIOREG(0x20)))

#endif

//bit offset within a 32-bit register
#define AST1500_GPIO_OFFSET(pin) ((pin) & 0x1F) //32 bits = 2^5 bits

#if (CONFIG_AST1500_SOC_VER >= 3)
#define AST1500_GPIO_DEBOUNCE(pin)	 \
	(((pin) < AST1500_GPE0)?(AST1500_GPIOREG(0x40)) : \
	((pin) < AST1500_GPI0) ? (AST1500_GPIOREG(0x48)) : \
	((pin) < AST1500_GPM0) ? (AST1500_GPIOREG(0xB0)) : \
	((pin) < AST1500_GPQ0) ? (AST1500_GPIOREG(0x100)) : \
	((pin) < AST1500_GPU0) ? (AST1500_GPIOREG(0x130)) : \
	((pin) < AST1500_GPY0) ? (AST1500_GPIOREG(0x160)) : \
	((pin) < AST1500_GPAC0) ? (AST1500_GPIOREG(0x190)) : \
	(AST1500_GPIOREG(0x1C0)))
#else
#define AST1500_GPIO_DEBOUNCE(pin)	 \
	(((pin) < AST1500_GPE0)?(AST1500_GPIOREG(0x40)):(AST1500_GPIOREG(0x48)))
#endif

#define AST1500_GPIO_DTIMER1(pin)	   (AST1500_GPIOREG(0x50))
#define AST1500_GPIO_DTIMER2(pin)	   (AST1500_GPIOREG(0x54))
#define AST1500_GPIO_DTIMER3(pin)	   (AST1500_GPIOREG(0x58))

//interrupt enable
#if (CONFIG_AST1500_SOC_VER >= 3)
#define AST1500_GPIO_INT(pin)	\
	((pin < AST1500_GPE0) ? (AST1500_GPIO_BASE(pin) + 0x08) : \
	 (pin < AST1500_GPI0) ? (AST1500_GPIO_BASE(pin) + 0x08) : \
	 (pin < AST1500_GPM0) ? (AST1500_GPIO_BASE(pin) + 0x28) : \
	 (pin < AST1500_GPQ0) ? (AST1500_GPIO_BASE(pin) + 0x70) : \
	 (pin < AST1500_GPU0) ? (AST1500_GPIOREG(0x118)) : \
	 (pin < AST1500_GPY0) ? (AST1500_GPIOREG(0x148)) : \
	 (pin < AST1500_GPAC0) ? (AST1500_GPIOREG(0x178)) : \
	 (AST1500_GPIOREG(0x1A8)))
#elif (CONFIG_AST1500_SOC_VER == 2)
#define AST1500_GPIO_INT(pin)	\
	((pin < AST1500_GPE0) ? (AST1500_GPIO_BASE(pin) + 0x08) : \
	 (pin < AST1500_GPI0) ? (AST1500_GPIO_BASE(pin) + 0x08) : \
	 (pin < AST1500_GPM0) ? (AST1500_GPIO_BASE(pin) + 0x28) : \
	 (pin < AST1500_GPQ0) ? (AST1500_GPIO_BASE(pin) + 0x70) : \
	 (AST1500_GPIO_BASE(pin) + 0x98))
#else
#define AST1500_GPIO_INT(pin)	   (AST1500_GPIO_BASE(pin) + 0x08)
#endif

//interrupt sensitivity type 0
#define AST1500_GPIO_INT_ST0(pin)	   (AST1500_GPIO_INT(pin) + 0x4)
//interrupt sensitivity type 1
#define AST1500_GPIO_INT_ST1(pin)	   (AST1500_GPIO_INT(pin) + 0x8)
//interrupt sensitivity type 2
#define AST1500_GPIO_INT_ST2(pin)	   (AST1500_GPIO_INT(pin) + 0xC)
//interrupt status
#define AST1500_GPIO_INT_STAT(pin)	   (AST1500_GPIO_INT(pin) + 0x10)
//reset tolerance
#define AST1500_GPIO_WDT_RESET(pin) (AST1500_GPIO_INT(pin) + 0x14)

//direction
#define AST1500_GPIO_CON(pin)	   (AST1500_GPIO_BASE(pin) + 0x04)
//data
#define AST1500_GPIO_DAT(pin)	   (AST1500_GPIO_BASE(pin))

#endif

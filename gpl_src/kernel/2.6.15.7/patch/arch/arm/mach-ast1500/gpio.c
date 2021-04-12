
#include <linux/module.h>
#include <linux/interrupt.h>

#include <asm/hardware.h>
#include <asm/irq.h>
#include <asm/io.h>
#include <asm/arch/ast-scu.h>

#include <asm/arch/regs-gpio.h>

#include "gpio-reg.h"

u32 dtimer_1ms = 0;
u32 dtimer_d5ms = 0;
u32 dtimer_2ms = 0;

static void setup_dtimer(unsigned int pin)
{
	//Old code record here for reference.
	//--------------------------------------------------------------------------
	//Debounce timer value are based on PCLK
	//if (CONFIG_AST1500_SOC_VER == 2)
	//0x7000 is the default value I caculate based on debounce time = 1ms and
	//SoC V2 PCLK is around 255MHz.
	//#define AST_GPIO_DTIMER_1MS   (0x7000)
	//#define AST_GPIO_DTIMER_d5MS  (0x3800)
	//#define AST_GPIO_DTIMER_2MS   (0xE000)
	//#else
	//0x9000 is the default value I caculate based on debounce time = 1ms and
	//H-PLL = 266MHz~300MHz with PCLK = H-PLL/8.
	//#define AST_GPIO_DTIMER_1MS   (0x9000)
	//#define AST_GPIO_DTIMER_d5MS  (0x4800)
	//#define AST_GPIO_DTIMER_2MS   (0x12000)
	//#endif
	//--------------------------------------------------------------------------

	//First time init.
	if (dtimer_1ms == 0) {
		//debounce time = PCLK cycle time * dtimer value
		//dtimer_1ms = PCLK_Hz / 1000; about >> 10
		dtimer_1ms = ast_scu.PCLK_Hz >> 10;
		dtimer_d5ms = dtimer_1ms >> 1;
		dtimer_2ms = dtimer_1ms << 2;
	}

	/* Bruce is so lazy and program the debounce timer value here every time. */
	__raw_writel(cpu_to_le32(dtimer_1ms), AST1500_GPIO_DTIMER1(pin));
	__raw_writel(cpu_to_le32(dtimer_d5ms), AST1500_GPIO_DTIMER2(pin));
	__raw_writel(cpu_to_le32(dtimer_2ms), AST1500_GPIO_DTIMER3(pin));
}

static void _gpio_debounce_cfg(unsigned int pin, unsigned int function)
{
    void __iomem *base = (void __iomem *)AST1500_GPIO_DEBOUNCE(pin);
    u32 s1 = 0;
    u32 s2 = 0;
    u32 r = 0;
    u32 mask;
    unsigned long flags;

#if (CONFIG_AST1500_SOC_VER <= 2)
    // (pin > AST1500_GPH7) doesn't support debounce.
    if (pin > AST1500_GPH7) {
        printk("ERROR (pin > AST1500_GPH7) doesn't support debounce!!\n");
        //BUG_ON(pin > AST1500_GPH7);
        return;
    }
#endif

    mask = 1 << AST1500_GPIO_OFFSET(pin);

    switch (function) {
    case AST1500_GPIO_NO_DEBOUNCE:
        break;
    case AST1500_GPIO_DEBOUNCE_T1:
        /*Bruce110928. The define in AST1500 datasheet is incorrect. Reference to AST1510.*/
        s2 = mask;
        break;
    case AST1500_GPIO_DEBOUNCE_T2:
        s1 = mask;
        break;
    case AST1500_GPIO_DEBOUNCE_T3:
        s1 = s2 = mask;
        break;
    }

    /* modify the specified register wwith IRQs off */

    local_irq_save(flags);

    /* Bruce is so lazy and program the debounce timer value here every time. */
    setup_dtimer(pin);

    r  = __raw_readl(base);
    r &= ~mask;
    r |= s1;
    __raw_writel(r, base);

    r  = __raw_readl(base + 0x4);
    r &= ~mask;
    r |= s2;
    __raw_writel(r, base + 0x4);

    local_irq_restore(flags);


}

static void _gpio_int_cfg(unsigned int pin, unsigned int function)
{
    //void __iomem *base = (void __iomem *)AST1500_GPIO_BASE(pin);
    u32 st0 = 0;
    u32 st1 = 0;
    u32 st2 = 0;
    u32 en = 0;
    u32 mask;
    u32 r;
    unsigned long flags;

    if (function >= AST1500_GPIO_NO_DEBOUNCE) {
        _gpio_debounce_cfg(pin, function);
        return;
    }

    en = mask = 1 << AST1500_GPIO_OFFSET(pin);

    switch (function) {
    case AST1500_GPIO_INT_FALLING_EDGE:
        break;
    case AST1500_GPIO_INT_RISING_EDGE:
        st0 = mask;
        break;
    case AST1500_GPIO_INT_LEVEL_LOW:
        st1 = mask;
        break;
    case AST1500_GPIO_INT_LEVEL_HIGH:
        st0 = st1 = mask;
        break;
    case AST1500_GPIO_INT_DUAL_EDGE:
        st2 = mask;
        break;
    case AST1500_GPIO_INT_DISABLE:
        en = 0;
        break;
    }

    /* modify the specified register wwith IRQs off */

    local_irq_save(flags);

    if (!en) { //Disable interrupt first to avoid false int alert
        r  = __raw_readl(AST1500_GPIO_INT(pin));
        r &= ~mask;
        __raw_writel(r, AST1500_GPIO_INT(pin));
    }

    r  = __raw_readl(AST1500_GPIO_INT_ST0(pin));
    r &= ~mask;
    r |= st0;
    __raw_writel(r, AST1500_GPIO_INT_ST0(pin));

    r  = __raw_readl(AST1500_GPIO_INT_ST1(pin));
    r &= ~mask;
    r |= st1;
    __raw_writel(r, AST1500_GPIO_INT_ST1(pin));

    r  = __raw_readl(AST1500_GPIO_INT_ST2(pin));
    r &= ~mask;
    r |= st2;
    __raw_writel(r, AST1500_GPIO_INT_ST2(pin));

    if (en) {
        r  = __raw_readl(AST1500_GPIO_INT(pin));
        r &= ~mask;
        r |= en;
        __raw_writel(r, AST1500_GPIO_INT(pin));
    }

    local_irq_restore(flags);
}


void ast1500_gpio_cfgpin(unsigned int pin, unsigned int function)
{
	void __iomem *base = (void __iomem *)AST1500_GPIO_CON(pin);
	unsigned long mask;
	unsigned long con;
	unsigned long flags;

	/*
	printk("cfgpin: pin(0x%x), CON(%d), OFF(%d)\n",
		pin, AST1500_GPIO_CON(pin), AST1500_GPIO_OFFSET(pin));
	*/

	if (function >= AST1500_GPIO_INT_FALLING_EDGE) {
		_gpio_int_cfg(pin, function);
		return;
	}

	mask = 1 << AST1500_GPIO_OFFSET(pin);

	switch (function) {
	case AST1500_GPIO_INPUT:
		function = 0;
		break;
	case AST1500_GPIO_OUTPUT:
		function = 1 << AST1500_GPIO_OFFSET(pin);
		break;
	}

	/* modify the specified register wwith IRQs off */

	local_irq_save(flags);

	con  = __raw_readl(base);
	//printk("!!cfg:read(0x%08x) ", con);
	con &= ~mask;
	//printk("masked(0x%08x) ", con);
	con |= function;
	//printk("|fun(0x%08x)\n", con);

	__raw_writel(con, base);

	local_irq_restore(flags);
}

EXPORT_SYMBOL(ast1500_gpio_cfgpin);

unsigned int ast1500_gpio_getcfg(unsigned int pin)
{
	void __iomem *base = (void __iomem *)AST1500_GPIO_CON(pin);
	unsigned long val = __raw_readl(base);

	val >>= AST1500_GPIO_OFFSET(pin);

	return (val & 0x1);
}

EXPORT_SYMBOL(ast1500_gpio_getcfg);

void ast1500_gpio_setpin(unsigned int pin, unsigned int to)
{
	void __iomem *base = (void __iomem *)AST1500_GPIO_DAT(pin);
	unsigned long offs = AST1500_GPIO_OFFSET(pin);
	unsigned long flags;
	unsigned long dat;
	u32 r = 0;

	/*
	printk("setpin: pin(0x%x), DAT(%d), OFF(%d), INT(%d), I_STAT(%d)\n",
		pin, AST1500_GPIO_DAT(pin), AST1500_GPIO_OFFSET(pin),
		AST1500_GPIO_INT(pin), AST1500_GPIO_INT_STAT(pin));
	*/
	r = __raw_readl(AST1500_GPIO_INT(pin)) & (1<< offs);
	if (r) { //It is in interrupt mode. Use the int status instead.
		base = (void __iomem *)AST1500_GPIO_INT_STAT(pin);
	}

	local_irq_save(flags);

	if (r) {
		/*
		** If we are in Interrupt mode, we CAN'T set the other int status bit
		** or we will mis-clear other interrupt events.
		** NOTE: This implement is for "write 1 clear" reg only.
		*/
		dat =  to << offs;
	} else {
		dat = __raw_readl(base);
		//printk("!!setpin:read(0x%08x) ", dat);
		dat &= ~(1 << offs);
		//printk("masked(0x%08x) ", dat);
		dat |= to << offs;
	}
	//printk("|to(0x%08x)\n", dat);
	__raw_writel(dat, base);

	local_irq_restore(flags);
}

EXPORT_SYMBOL(ast1500_gpio_setpin);

unsigned int ast1500_gpio_getpin(unsigned int pin)
{
	void __iomem *base = (void __iomem *)AST1500_GPIO_DAT(pin);
	unsigned long offs = AST1500_GPIO_OFFSET(pin);
#if 0 //Use ast1500_gpio_get_int_stat() instead
	u32 r;
	r = __raw_readl(AST1500_GPIO_INT(pin)) & (1<< offs);
	if (r) { //It is in interrupt mode. Read the int status instead.
		return __raw_readl(AST1500_GPIO_INT_STAT(pin)) & (1<< offs);
	}
#endif
	return __raw_readl(base) & (1<< offs);
}

EXPORT_SYMBOL(ast1500_gpio_getpin);

void ast1500_gpio_ack_int_stat(unsigned int pin)
{
	void __iomem *base;
	unsigned long offs = AST1500_GPIO_OFFSET(pin);
	unsigned long flags;
	unsigned long dat;

	base = (void __iomem *)AST1500_GPIO_INT_STAT(pin);

	local_irq_save(flags);
	/*
	** If we are in Interrupt mode, we CAN'T set the other int status bit
	** or we will mis-clear other interrupt events.
	** NOTE: This implement is for "write 1 clear" reg only.
	*/
	dat =  1 << offs;
	//printk("|to(0x%08x)\n", dat);
	__raw_writel(dat, base);

	local_irq_restore(flags);
}

EXPORT_SYMBOL(ast1500_gpio_ack_int_stat);


unsigned int ast1500_gpio_get_int_stat(unsigned int pin)
{
	unsigned long offs = AST1500_GPIO_OFFSET(pin);

	return __raw_readl(AST1500_GPIO_INT_STAT(pin)) & (1<< offs);
}

EXPORT_SYMBOL(ast1500_gpio_get_int_stat);


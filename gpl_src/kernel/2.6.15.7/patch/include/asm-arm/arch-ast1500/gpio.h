
#ifndef __ASM_ARCH_AST1500_GPIO_H
#define __ASM_ARCH_AST1500_GPIO_H

#include <linux/kernel.h>
#include <asm/hardware.h>
#include <asm/arch/regs-gpio.h>

extern void ast1500_gpio_cfgpin(unsigned int pin, unsigned int function);
extern unsigned int ast1500_gpio_getcfg(unsigned int pin);
extern void ast1500_gpio_setpin(unsigned int pin, unsigned int to);
extern unsigned int ast1500_gpio_getpin(unsigned int pin);
extern unsigned int ast1500_gpio_get_int_stat(unsigned int pin);
extern void ast1500_gpio_ack_int_stat(unsigned int pin);

static inline int gpio_request(unsigned gpio, const char *label)
{
	return 0;
}

static inline void gpio_free(unsigned gpio)
{
	return;
}

static inline int gpio_direction_input(unsigned gpio)
{
	ast1500_gpio_cfgpin(gpio, AST1500_GPIO_INPUT);
	return 0;
}

static inline int gpio_direction_output(unsigned gpio, int value)
{
	//printk("!!g:out gpio(%d), off(%d), value(%d)\n", gpio, AST1500_GPIO_OFFSET(gpio), value);
	ast1500_gpio_cfgpin(gpio, AST1500_GPIO_OUTPUT);

	ast1500_gpio_setpin(gpio, value);
	return 0;
}

#define gpio_get_value(gpio)		ast1500_gpio_getpin(gpio)
#define gpio_set_value(gpio,value)	ast1500_gpio_setpin(gpio, value)
#define gpio_cfg(gpio,func)			ast1500_gpio_cfgpin(gpio, func)

#define gpio_get_int_stat(gpio)     ast1500_gpio_get_int_stat(gpio)
#define gpio_ack_int_stat(gpio)     ast1500_gpio_ack_int_stat(gpio)

//#include <asm-generic/gpio.h>			/* cansleep wrappers */
/* platforms that don't directly support access to GPIOs through I2C, SPI,
 * or other blocking infrastructure can use these wrappers.
 */

static inline int gpio_cansleep(unsigned gpio)
{
	return 0;
}

static inline int gpio_get_value_cansleep(unsigned gpio)
{
	might_sleep();
	return gpio_get_value(gpio);
}

static inline void gpio_set_value_cansleep(unsigned gpio, int value)
{
	might_sleep();
	gpio_set_value(gpio, value);
}



#endif


/* linux/include/asm-arm/arch-s3c2410/leds-gpio.h
 *
 * Copyright (c) 2006 Simtec Electronics
 *	http://armlinux.simtec.co.uk/
 *	Ben Dooks <ben@simtec.co.uk>
 *
 * S3C24XX - LEDs GPIO connector
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#ifndef __ASM_ARCH_LEDSGPIO_H
#define __ASM_ARCH_LEDSGPIO_H "leds-gpio.h"

#define AST_LEDF_ACTLOW      	(1<<0)		/* LED is on when GPIO low */
#define AST_LEDF_TRISTATE_OFF	(1<<1)		/* tristate to turn off */
#define AST_LEDF_TRISTATE_ON	(1<<2)		/* tristate to turn off */
#define AST_LEDF_BUTTON             (1<<3)          /* This IO is actually used as a push button */
#define AST_LEDF_SWITCH             (1<<4)          /* This IO is actually used as a switch input */
#define AST_LEDF_DEFAULT_ON         (1<<5)          /* LED's init value is ON instead OFF */


struct ast_led_platdata {
	unsigned int		 gpio;
	unsigned int		 flags;

	char			*name;
	char			*def_trigger;
};

#endif /* __ASM_ARCH_LEDSGPIO_H */

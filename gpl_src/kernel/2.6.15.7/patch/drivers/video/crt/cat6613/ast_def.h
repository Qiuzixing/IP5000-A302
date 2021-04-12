#ifndef _HDMITX_AST_DEF_
#define _HDMITX_AST_DEF_

//#include <linux/module.h>

#define CODE202

#include <aspeed/features.h>

#define Linux_KernelMode

#define DEBUG
#define REG_DBG 0
#include "debug.h"

#define USE_THREAD 1
#define HANDLE_INT_IN_THREAD 1
#define NO_INT_MODE 0

#define NLPCM_THROUGH_I2S


#include <asm/arch/gpio.h>

#define ENABLE_INT() do {gpio_cfg(GPIO_CAT6613_INT, AST1500_GPIO_INT_LEVEL_LOW);} while (0)
#define DISABLE_INT() \
do { \
	gpio_cfg(GPIO_CAT6613_INT, AST1500_GPIO_INT_DISABLE); \
	/* ack interrupt. Disabling GPIO int won't clear its status, \
	** so we need to do it by ourself. */ \
	gpio_ack_int_stat(GPIO_CAT6613_INT); \
} while (0)


#endif

/*
 *
 * Copyright (C) 2012-2020  ASPEED Technology Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 *   History      :
 *    1. 2013/04/04 Bruce first version
 *
********************************************************************************/

#ifndef _AST_SCU_H_
#define _AST_SCU_H_


#include <linux/types.h>
#include <linux/spinlock.h>

#include "ast-scu_ast1500.h"
#include "ast-scu_ast1510.h"
#include "ast-scu_ast1520.h"

/* Define all SCU operation here.
** 0xFF000000   Engine category: VE,CRT,I2C
** 0x00FF0000   Sub category: Init, Pin Mux, clock, reset
**              Init:     0x00000000
**              Pin:      0x00010000
**              Clock:    0x00020000
**              Reset:    0x00040000
*/
enum scu_op_e {
	SCUOP_SCU_LOCK                   = 0x00000000,
		SCUOP_HPLL_SET               = 0x00020000,
		SCUOP_D1PLL_SET              = 0x00020001,
	SCUOP_SPI_INIT                   = 0x01000000,
	SCUOP_MAC_INIT                   = 0x02000000,
	SCUOP_USB_INIT                   = 0x03000000,
		SCUOP_VHUB_INIT                ,
		SCUOP_USB20HC_INIT             ,
		SCUOP_USB11HC_INIT             ,
	SCUOP_CRYPT_INIT                 = 0x04000000,
	SCUOP_I2S_INIT                   = 0x05000000,
	SCUOP_I2S_PIN_INIT               = 0x05010000,
		SCUOP_I2S_PIN_HDMI_H           ,
		SCUOP_I2S_PIN_HDMI_C           ,
		SCUOP_I2S_PIN_CODEC_H          ,
		SCUOP_I2S_PIN_CODEC_C          ,
		SCUOP_I2S_PIN_DUAL_OUTPUT_C    ,
	SCUOP_I2S_CLOCK_INIT             = 0x05020000,
	SCUOP_I2S_STOP                   = 0x05040000,
		SCUOP_I2S_START                ,
	SCUOP_CRT_INIT                   = 0x06000000,
		SCUOP_CRT_CRT1_INIT            ,
		SCUOP_CRT_CRT2_INIT            ,
		SCUOP_CRT_DAC_CTRL_FROM_CRT    ,
		SCUOP_CRT_DAC_CTRL_FROM_DVO    ,
		SCUOP_CRT_DAC_CTRL_DISABLE     ,
	SCUOP_CRT_PIN                    = 0x06010000,
		SCUOP_CRT_CRT1_PIN             ,
		SCUOP_CRT_CRT2_PIN             ,
	SCUOP_VE_INIT_ENCODE             = 0x07000000,
		SCUOP_VE_INIT_DECODE           ,
		SCUOP_VE_D2CLK_SELECTION       ,
	SCUOP_VE_PIN                     = 0x07010000,
		SCUOP_VE_PIN_DIGITAL_IN        ,
		SCUOP_VE_PIN_ANALOG_IN         ,
	SCUOP_VE_RESET                   = 0x07040000,
	SCUOP_I2C_INIT                   = 0x08000000,
	SCUOP_UART_INIT                  = 0x09000000,
	SCUOP_IR_INIT                    = 0x0A000000,
	SCUOP_IR_STOP                    = 0x0A040000,
	SCUOP_DEVICE_RST                 = 0x0B040000,
	SCUOP_GEN_LOCK_INIT              = 0x0C000000,
		SCUOP_GEN_LOCK_ADJUST        = 0x0C000001,
		SCUOP_GEN_LOCK_STOP          = 0x0C000002,
	SCUOP_AUDIO_GEN_LOCK_INIT        = 0x0D000000,
		SCUOP_AUDIO_GEN_LOCK_ADJUST  = 0x0D000001,
		SCUOP_AUDIO_GEN_LOCK_STOP    = 0x0D000002,
		SCUOP_AUDIO_GEN_LOCK_LIMIT   = 0x0D000003,
		SCUOP_AUDIO_GEN_LOCK_SET     = 0x0D000004,
};

enum scu_rst_e {
	SCU_RST_HDMI_TX = 0,
	SCU_RST_HDMI_RX,
	SCU_RST_HDCP22_TX,
	SCU_RST_HDCP22_RX,
	SCU_RST_MAX,
};

typedef int (*pfn_scu_op)(u32 op, void *param);

typedef struct {
	spinlock_t lock;
	unsigned long lock_flags;
	unsigned char __iomem *iobase;
	pfn_scu_op scu_op;
	/* Start of SoC related parameters. */
	u32 revision_id;
	u32 CLKIN_Hz; //Reference input clock. 24MHz or 25MHz.
	u32 MPLL_Hz;  //M-PLL clock for DRAM
	u32 HPLL_Hz;  //H-PLL clock for CPU
	u32 HPLL_Mode;
	u32 PCLK_Hz;  //APB Bus clock
	u32 HCLK_Mhz; //AHB Bus clock
	u32 MCLK_Mhz; //Memory Bus (M Bus) clock
	/* End of SoC related parameters. */
	board_info_t board_info;
	ability_info_t ability;
	astparam_t astparam;
} scu_t;


extern scu_t ast_scu;

#endif //#ifndef _AST_SCU_H_


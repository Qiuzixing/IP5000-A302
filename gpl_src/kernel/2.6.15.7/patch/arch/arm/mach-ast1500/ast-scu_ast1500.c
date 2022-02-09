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

#if (CONFIG_AST1500_SOC_VER == 1)

#include <asm/arch/platform.h>
#include <linux/delay.h>
#include <asm/arch/regs-scu.h>
#include <asm/arch/ast-scu.h>
#include "ast-scu-drv.h"
#include <asm/arch/drivers/board_def.h>
#include <asm/arch/gpio.h>

const char driver_name[] = "scu";

static inline int _is_scu_unlocked(void)
{
	return GET_SCU_REG(RSCU_PRTCT);
}

static inline int _scu_lock(int lock)
{
	if (lock) {
#if 0 //TestOnly. Currently, there are lots of driver code assumes SCU is unlocked. Don't lock SCU until they are all fixed.
		SET_SCU_REG(RSCU_PRTCT, 1);
#endif
	} else {
		SET_SCU_REG(RSCU_PRTCT, PRTCT_UNLOCK);
		while (!_is_scu_unlocked())
			uinfo("Unlocking SCU.");
	}
	return 0;
}

static unsigned int get_HPLL_Hz(void)
{
	unsigned int scu24, hpll_select, postDiv;

	scu24 = GET_SCU_REG(RSCU_HPLL);
	if (scu24 & HPLL_FROMREG_EN) {
		unsigned int OD, num, denum;
		/* H-PLL from SCU24[17:0] */
		if (scu24 & HPLL_BYPASS_EN) {
			//bypass from 24MHz directly.
			return 24000000;
		}
		if (scu24 & HPLL_HPLL_OFF) {
			uerr("H-PLL is off?!\n");
			return 0;
		}
		//OD == SCU24[4]
		OD = HPLL_OD_GET(scu24);
		//Numerator == SCU24[10:5]
		num = HPLL_NMRTR_GET(scu24);
		//Denumerator == SCU24[3:0]
		denum = HPLL_DNMRATR_GET(scu24);

		//Post Div == SCU24[14:12]
		if (scu24 & HPLL_PD_HIGH_BIT) {
			postDiv = 1 << (((scu24 >> 12) & 0x3) + 1);
		} else {
			postDiv = 1;
		}
		//hpll = (24MHz * (2-OD) * ((Numerator+2)/(Denumerator+1)))/postDiv
		return (24000 * (2-OD) * ((num+2)*1000/(denum+1)))/postDiv;
	}

	hpll_select = HWSTRAP1_HPLLSLT_GET(GET_SCU_REG(RSCU_HWSTRAP1));
	switch (hpll_select) {
	case HWSTRAP1_HPLLSLT_266MHZ:
		return 266000000;
	case HWSTRAP1_HPLLSLT_233MHZ:
		return 233000000;
	case HWSTRAP1_HPLLSLT_200MHZ:
		return 200000000;
	case HWSTRAP1_HPLLSLT_166MHZ:
		return 166000000;
	case HWSTRAP1_HPLLSLT_133MHZ:
		return 133000000;
	case HWSTRAP1_HPLLSLT_100MHZ:
		return 100000000;
	case HWSTRAP1_HPLLSLT_300MHZ:
		return 300000000;
	case HWSTRAP1_HPLLSLT_24MHZ:
		return 24000000;
	default:
		BUG();
	};
}

static unsigned int get_HPLL_mode(void)
{
	// Get the HPLL select
	return HWSTRAP1_HPLLSLT_GET(GET_SCU_REG(RSCU_HWSTRAP1));
}

static unsigned int set_HPLL_mode(unsigned int mode)
{
	MOD_SCU_REG(RSCU_RESET, HWSTRAP1_HPLLSLT_SET(mode), HWSTRAP1_HPLLSLT_MASK);

	return 0;
}

static unsigned int get_HCLK_MHz(void)
{
	unsigned int div = 0, hpll;
	unsigned int reg = GET_SCU_REG(RSCU_HWSTRAP1);

	hpll = ast_scu.HPLL_Hz;
	switch (HWSTRAP1_AHBCLKDIV_GET(reg))
	{
	case HWSTRAP1_AHBCLKDIV_DIV1:
		 div = 1;
		 break;
	case HWSTRAP1_AHBCLKDIV_DIV2:
		 div = 2;
		 break;
	case HWSTRAP1_AHBCLKDIV_DIV4:
		 div = 4;
		 break;
	case HWSTRAP1_AHBCLKDIV_DIV3:
		 div = 3;
		 break;
	 default:
		 BUG();
	}
	printk("HPLL=%d, Div=%d, HCLK=%d\n", hpll, div, hpll/div);
	return (hpll/div/1000000);
}

static unsigned int get_PCLK_Hz(void)
{
	unsigned int div, hpll;

	//PCLK == HPLL/div, div is from SCU08[25:23]
	hpll = ast_scu.HPLL_Hz;
	div = (CLKSEL_PCLKD_GET(GET_SCU_REG(RSCU_CLKSEL)) + 1) << 1;
	uinfo("HPLL=%d, Div=%d, PCLK=%d\n", hpll, div, hpll/div);
	return (hpll/div);
}

static unsigned int get_board_type(void)
{
	return GET_SCU_REG(RSCU_HWSTRAP1) & HWSTRAP1_ISCLIENT_MASK;
}

static void get_host_video_type(board_info_t *info)
{
	info->video_type = VT_DISABLE;
	info->is_internal_video_capture = 0;
	//check input select
	gpio_direction_input(GPIO_HOST_VIDEO_INPUT_SELECT);
	if (gpio_get_value(GPIO_HOST_VIDEO_INPUT_SELECT)) {
		info->video_type = VT_ANALOG;
#ifdef CONFIG_AST1500_BOARD_V3
		//V3 supports only digital signal
		info->video_type = VT_HDMI;
#endif
	} else {
#ifdef CONFIG_AST1500_BOARD_V3
		info->is_internal_video_capture = 1;
#endif
		info->video_type = VT_HDMI;
	}
}

/* This is SCU internal function. */
static inline void __scu_crt_set_VPA_clk_delay(vpo_clk_delay delay)
{
	MOD_SCU_REG(RSCU_CLKSEL,
	            CLKSEL_VPACLKDLYB321(VPCLKDLYB321(delay)),
	            CLKSEL_VPACLKDLYB321_MASK);

	MOD_SCU_REG(RSCU_MISC1CTRL,
	            MISC1CTRL_VPACLKDLYB0(VPCLKDLYB0(delay)),
	            MISC1CTRL_VPACLKDLYB0_MASK);
}

/* This is SCU internal function. */
static inline void __scu_crt_set_VPB_clk_delay(vpo_clk_delay delay)
{
	MOD_SCU_REG(RSCU_CLKSEL,
	            CLKSEL_VPBCLKDLYB321(VPCLKDLYB321(delay)),
	            CLKSEL_VPBCLKDLYB321_MASK);

	MOD_SCU_REG(RSCU_MISC1CTRL,
	            MISC1CTRL_VPBCLKDLYB0(VPCLKDLYB0(delay)),
	            MISC1CTRL_VPBCLKDLYB0_MASK);
}

static inline void __scu_crt_pin_en(unsigned int crt_num, unsigned int en)
{
	switch (crt_num) {
	case 1:
		if (en) {
			// Enable PortA,VP1 and VP2. Disable PortB
			MOD_SCU_REG(RSCU_PINCTRL1,
						PINCTRL1_VDOPORTA(1) | PINCTRL1_VDOVP1(1) | PINCTRL1_VDOVP2(1) | PINCTRL1_VGAPIN(0),
						PINCTRL1_VDOPORTA_MASK | PINCTRL1_VDOPORTB_MASK | PINCTRL1_VDOVP1_MASK | PINCTRL1_VDOVP2_MASK | PINCTRL1_VGAPIN_MASK);
		} else {
			/*
			** NOT Disable PortA. Keep VP1 and VP2 pin mux. Disable PortB
			** Note. I2S will use CRT1's clock. So, we should never stop D1CLK and PortA.
			** Since PortA is VGA, turning off HSync should be just fine. So, we keep PortA enabled.
			*/
			MOD_SCU_REG(RSCU_PINCTRL1,
						PINCTRL1_VDOPORTA(1) | PINCTRL1_VDOVP1(1) | PINCTRL1_VDOVP2(1) | PINCTRL1_VGAPIN(0),
						PINCTRL1_VDOPORTA_MASK | PINCTRL1_VDOPORTB_MASK | PINCTRL1_VDOVP1_MASK | PINCTRL1_VDOVP2_MASK | PINCTRL1_VGAPIN_MASK);
		}
		break;
	case 2:
	default:
		if (en) {
			//To enable CRT2, Enable PortB, VP1 and VP2.
			MOD_SCU_REG(RSCU_PINCTRL1,
			            PINCTRL1_VDOPORTB(1) | PINCTRL1_VDOVP1(1) | PINCTRL1_VDOVP2(1) | PINCTRL1_VGAPIN(0),
			            PINCTRL1_VDOPORTB_MASK | PINCTRL1_VDOVP1_MASK | PINCTRL1_VDOVP2_MASK | PINCTRL1_VGAPIN_MASK);
		} else {
			// Disable Port B. Keep VP1 and VP2 pin mux.
			MOD_SCU_REG(RSCU_PINCTRL1,
			            PINCTRL1_VDOPORTB(0) | PINCTRL1_VDOVP1(1) | PINCTRL1_VDOVP2(1) | PINCTRL1_VGAPIN(0),
			            PINCTRL1_VDOPORTB_MASK | PINCTRL1_VDOVP1_MASK | PINCTRL1_VDOVP2_MASK | PINCTRL1_VGAPIN_MASK);
		}
		break;
	}
}

static int _scu_crt_crt2_pin_en(unsigned int en)
{
	SCU_START();

	__scu_crt_pin_en(2, en);

	SCU_END();

	return 0;
}

static int _scu_crt_crt1_pin_en(unsigned int en)
{
	SCU_START();

	__scu_crt_pin_en(1, en);

	SCU_END();

	return 0;
}

static int _scu_crt_crt1_init(void)
{
	SCU_START();

	SET_SCU_REG_BITS(RSCU_RESET, RESET_CRT_RESET);

	//Set PortA clock from D1CLK and PortB clock from D2CLK
	MOD_SCU_REG(RSCU_CLKSEL,
	           CLKSEL_PORTBCLKSEL(1) | CLKSEL_PORTACLKSEL(0),
	           CLKSEL_PORTBCLKSEL_MASK | CLKSEL_PORTACLKSEL_MASK);

	__scu_crt_set_VPA_clk_delay(VPCLKDLY_I1d5NS);
	__scu_crt_set_VPB_clk_delay(VPCLKDLY_I1d5NS);

	MOD_SCU_REG(RSCU_PINCTRL2,
	           PINCTRL2_VDOPORTASIGEDG(1) | PINCTRL2_VDOPORTBSIGEDG(1),
	           PINCTRL2_VDOPORTASIGEDG_MASK | PINCTRL2_VDOPORTBSIGEDG_MASK);

	// Enable PortA,VP1 and VP2. Disable PortB
	__scu_crt_pin_en(1, 1);

	//Note. I2S will use CRT1's clock. So, we should never stop D1CLK and PortA.
	MOD_SCU_REG(RSCU_CLKSTOP,
				CLKSTOP_D1CLK(0) | CLKSTOP_D2CLK(1) | CLKSTOP_DCLK(0),
				CLKSTOP_D1CLK_MASK | CLKSTOP_D2CLK_MASK | CLKSTOP_DCLK_MASK);

	MOD_SCU_REG(RSCU_MISC1CTRL,
				MISC1CTRL_VDODACOFF(0) | MISC1CTRL_D1PLLOFF(0),
				MISC1CTRL_VDODACOFF_MASK | MISC1CTRL_D1PLLOFF_MASK);

	CLR_SCU_REG_BITS(RSCU_RESET, RESET_CRT_RESET);

	SCU_END();

	return 0;
}

static int _scu_crt_crt2_init(void)
{
	SCU_START();

	SET_SCU_REG_BITS(RSCU_RESET, RESET_CRT_RESET);

	//Set PortA clock from D1CLK and PortB clock from D2CLK
	MOD_SCU_REG(RSCU_CLKSEL,
	           CLKSEL_PORTBCLKSEL(1) | CLKSEL_PORTACLKSEL(0),
	           CLKSEL_PORTBCLKSEL_MASK | CLKSEL_PORTACLKSEL_MASK);

	__scu_crt_set_VPA_clk_delay(VPCLKDLY_I1d5NS);
	__scu_crt_set_VPB_clk_delay(VPCLKDLY_I1d5NS);

	MOD_SCU_REG(RSCU_PINCTRL2,
	           PINCTRL2_VDOPORTASIGEDG(1) | PINCTRL2_VDOPORTBSIGEDG(1),
	           PINCTRL2_VDOPORTASIGEDG_MASK | PINCTRL2_VDOPORTBSIGEDG_MASK);

	//To enable CRT2, Enable PortB, VP1 and VP2.
	__scu_crt_pin_en(2, 1);

	//Note. I2S will use CRT1's clock. So, we should never stop D1CLK and PortA.
	MOD_SCU_REG(RSCU_CLKSTOP,
	            CLKSTOP_D1CLK(0) | CLKSTOP_D2CLK(0) | CLKSTOP_DCLK(0),
	            CLKSTOP_D1CLK_MASK | CLKSTOP_D2CLK_MASK | CLKSTOP_DCLK_MASK);

	MOD_SCU_REG(RSCU_MISC1CTRL, MISC1CTRL_D2PLLOFF(0), MISC1CTRL_D2PLLOFF_MASK);

	CLR_SCU_REG_BITS(RSCU_RESET, RESET_CRT_RESET);

	SCU_END();

	return 0;
}

static inline int __scu_ve_reset(void)
{
	SET_SCU_REG_BITS(RSCU_RESET, RESET_VE_RESET);
	mb();
	udelay(100);
	//Clear SCU Reset Register
	CLR_SCU_REG_BITS(RSCU_RESET, RESET_VE_RESET);
	mb();
	while (GET_SCU_REG(RSCU_RESET) & RESET_VE_RESET) {
		//to do: error detection and recovery
	}
	return 0;
}

static int _scu_ve_reset(void)
{
	int ret = 0;

	SCU_START();
	ret = __scu_ve_reset();
	SCU_END();

	return ret;
}

static int _scu_ve_d2clk_selection(int video_num)
{
	SCU_START();

	if(video_num == 1) {
		MOD_SCU_REG(RSCU_CLKSEL, CLKSEL_D2CLKSEL(CLKSEL_D2CLKSEL_V1CLK_VIDEO1), CLKSEL_D2CLKSEL_MASK);
		MOD_SCU_REG(RSCU_CLKSTOP, CLKSTOP_D2CLK(0), CLKSTOP_D2CLK_MASK);
	} else if(video_num == 2){
		MOD_SCU_REG(RSCU_CLKSEL, CLKSEL_D2CLKSEL(CLKSEL_D2CLKSEL_V1CLK_VIDEO2), CLKSEL_D2CLKSEL_MASK);
		MOD_SCU_REG(RSCU_CLKSTOP, CLKSTOP_D2CLK(0), CLKSTOP_D2CLK_MASK);
	} else {
		/* disable */
		MOD_SCU_REG(RSCU_CLKSEL, CLKSEL_D2CLKSEL(CLKSEL_D2CLKSEL_NORMAL_CRT1), CLKSEL_D2CLKSEL_MASK);
		MOD_SCU_REG(RSCU_CLKSTOP, CLKSTOP_D2CLK(1), CLKSTOP_D2CLK_MASK);
	}

	SCU_END();

	return 0;
}

static int _scu_ve_init_encode(void)
{
	SCU_START();

	__scu_ve_reset();

	//open vide capture 1 and capture2
	MOD_SCU_REG(RSCU_CLKSTOP,
				CLKSTOP_ECLK(0) | CLKSTOP_V1CLK(0) | CLKSTOP_V2CLK(0),
				CLKSTOP_ECLK_MASK | CLKSTOP_V1CLK_MASK | CLKSTOP_V2CLK_MASK);

	MOD_SCU_REG(RSCU_PINCTRL1,
				0,
				PINCTRL1_VDOPORTA_MASK | PINCTRL1_VDOPORTB_MASK | PINCTRL1_VDOVP1_MASK | PINCTRL1_VDOVP2_MASK);

	MOD_SCU_REG(RSCU_PINCTRL2,
				PINCTRL2_VDOPORTASIGEDG(1) | PINCTRL2_VDOPORTBSIGEDG(1),
				PINCTRL2_VDOPORTASIGEDG_MASK | PINCTRL2_VDOPORTBSIGEDG_MASK);

	SCU_END();

	return 0;
}

static int _scu_ve_init_decode(void)
{
	SCU_START();

	MOD_SCU_REG(RSCU_CLKSTOP,
				CLKSTOP_ECLK(0) | CLKSTOP_V1CLK(0) | CLKSTOP_V2CLK(0) ,
				CLKSTOP_ECLK_MASK | CLKSTOP_V1CLK_MASK | CLKSTOP_V2CLK_MASK);
	SCU_END();

	return 0;
}

static int _scu_i2s_init(void)
{
	SCU_START();
	/* Enable I2S multi-pin */
	SET_SCU_REG_BITS(RSCU_PINCTRL1, PINCTRL1_I2SPIN_EN);

	SCU_END();

	/* Wait for clock stable */
	msleep(1);

	SCU_START();
	/* Enable I2S clock */
	CLR_SCU_REG_BITS(RSCU_CLKSTOP, CLKSTOP_I2S);
	SCU_END();

	return 0;
}

static int _scu_i2s_stop(void)
{
	SCU_START();

	//To reset state.
	SET_SCU_REG_BITS(RSCU_RESET, RESET_I2S_RESET);
	wmb();

	SCU_END();

	return 0;
}

static int _scu_i2s_start(void)
{
	SCU_START();

	CLR_SCU_REG_BITS(RSCU_RESET, RESET_I2S_RESET);
	mb();
	while (GET_SCU_REG(RSCU_RESET) & RESET_I2S_RESET) {
		//to do: error detection and recovery
		printk("W\n");
	}

	SCU_END();

	return 0;
}

static int _scu_vhub_init(void)
{
	SCU_START();

    /*
    ** Following reset sequence can resolve "vhub dead on first power on" issue on V4 board.
    */
    //reset USB20
    SET_SCU_REG_BITS(RSCU_RESET, RESET_USB20_RESET);
    wmb();

    //enable USB20 clock
    SET_SCU_REG_BITS(RSCU_CLKSTOP, CLKSTOP_USB20_ENCLK);
    mdelay(10);

    CLR_SCU_REG_BITS(RSCU_RESET, RESET_USB20_RESET);
    wmb();
    udelay(500);

	SCU_END();

	BUG_ON(GET_SCU_REG(RSCU_RESET) & RESET_USB20_RESET);
	return 0;
}

inline void _scu_init_board_info(void)
{
	board_info_t *info = &ast_scu.board_info;

	info->soc_ver = 1;
	info->soc_revision = 11;
	info->is_client = get_board_type();
	if (info->is_client) {
		//AST1500 client can't detect video type from traping. Use a default value.
		info->video_type = VT_HDMI;
		info->is_internal_video_capture = 0;
	} else {
		get_host_video_type(info);
	}
	info->video_loopback_type = VT_HDMI; //There is no way for AST1500 board to know. Set default to 'available'.
}

static int _scu_init(void)
{
	ast_scu.revision_id = GET_SCU_REG(RSCU_REVISION);
	ast_scu.MPLL_Hz = 0; //TBD
	ast_scu.HPLL_Hz = get_HPLL_Hz();
	ast_scu.PCLK_Hz = get_PCLK_Hz();
	ast_scu.HCLK_Mhz = get_HCLK_MHz();
	ast_scu.HPLL_Mode = get_HPLL_mode();
}

//int scu_op(u32 op, void *param1, void *param2)
int scu_op(u32 op, void *param)
{
	int rtn = 0;
	//uinfo("scu_op(0x%08X) param(0x%08X)\n", op, (int)param);
	switch (op) {
		case SCUOP_SCU_LOCK:
			rtn = _scu_lock((int)param);
			break;
		case SCUOP_VHUB_INIT:
			rtn = _scu_vhub_init();
			break;
		case SCUOP_HPLL_SET:
			rtn = set_HPLL_mode((int)param);
			break;
		case SCUOP_CRT_CRT1_INIT:
			rtn = _scu_crt_crt1_init();
			break;
		case SCUOP_CRT_CRT2_INIT:
			rtn = _scu_crt_crt2_init();
			break;
		case SCUOP_CRT_CRT1_PIN:
			rtn = _scu_crt_crt1_pin_en((unsigned int)param);
			break;
		case SCUOP_CRT_CRT2_PIN:
			rtn = _scu_crt_crt2_pin_en((unsigned int)param);
			break;
		case SCUOP_VE_RESET:
			rtn = _scu_ve_reset();
			break;
		case SCUOP_VE_INIT_ENCODE:
			rtn = _scu_ve_init_encode();
			break;
		case SCUOP_VE_INIT_DECODE:
			rtn = _scu_ve_init_decode();
			break;
		case SCUOP_VE_D2CLK_SELECTION:
			rtn = _scu_ve_d2clk_selection((int)param);
			break;
		case SCUOP_I2S_INIT:
			rtn = _scu_i2s_init();
			break;
		case SCUOP_I2S_STOP:
			rtn = _scu_i2s_stop();
			break;
		case SCUOP_I2S_START:
			rtn = _scu_i2s_start();
			break;

		default:
			break;
	}
	return rtn;
}

/*
** This init function MUST only be called once. No software lock needed.
** ToDo. Make sure this function will be called before others.
*/
int scu_init(void)
{
	int err = 0;

	/*
	** Register IO
	*/
	if (!request_mem_region(ASPEED_SCU_BASE, 0x1000, driver_name)) {
		uerr("TBD: unable to map IO region\n");
		err = -ENOMEM;
		goto err_out;
	}

	ast_scu.iobase = ioremap_nocache(ASPEED_SCU_BASE, 0x1000);
	if (!ast_scu.iobase) {
		uerr("TBD: unable to remap the IO region\n");
		err = -ENOMEM;
		goto err_out;
	}
	uinfo("iobase=0x%08x\n", (u32)ast_scu.iobase);

	_scu_lock(0);
	_scu_init();
	astparam_init_shadow();
	_scu_init_board_info();
	scu_init_ability(&ast_scu.ability);
	/* Requires soc_op_mode. MUST be called after scu_init_ability() */
	scu_init_astparam(&ast_scu, &ast_scu.astparam);
	astparam_destroy_shadow();
	_scu_lock(1);

err_out:
	return err;
}

scu_t ast_scu =
{
	.scu_op = scu_op,
};
EXPORT_SYMBOL(ast_scu);

/*
** This init function MUST only be called once.
*/
int ast_scu_init(void)
{
	uinfo("ast_scu_init()\n");
	/* driver init */
	spin_lock_init(&ast_scu.lock);
	/* SoC dependent SCU init */
	return scu_init();
}
core_initcall(ast_scu_init);

#endif //#if (CONFIG_AST1500_SOC_VER == 1)

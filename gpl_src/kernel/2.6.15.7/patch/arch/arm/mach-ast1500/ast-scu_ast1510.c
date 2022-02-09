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

#if (CONFIG_AST1500_SOC_VER == 2)

#include <asm/arch/platform.h>
#include <linux/delay.h>
#include <asm/arch/regs-scu.h>
#include <asm/arch/ast-scu.h>
#include "ast-scu-drv.h"
#include <asm/arch/drivers/board_def.h>

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

#define SCU_SYSTEM_RESET_DELAY              100
#define SCU_SYSTEM_CLK_DELAY_MS             10
#define SCU_SYSTEM_CLK_SHORT_DELAY_MS       1
/*
 * scu_system_clk_stop - Gating XX engine's clock.
 * @mask:
 * @stop: stop or start
 */
static void scu_system_clk_stop(u32 mask, u32 stop)
{
	u32 reset_mask = 0;
	u32 clk_stable_time = SCU_SYSTEM_CLK_DELAY_MS; //in ms

	switch (mask) {
	case CLKSTOP_ECLK_MASK:
		reset_mask = RESET_VE_RESET;
		break;
	case CLKSTOP_V1CLK_MASK:
		/* For video capture. Clock from video input port's pixel clock */
		clk_stable_time = SCU_SYSTEM_CLK_SHORT_DELAY_MS;
		break;
	case CLKSTOP_DCLK_MASK:
		/* TODO */
		break;
	case CLKSTOP_D1CLK_MASK:
		reset_mask = RESET_CRT_RESET;
		break;
	case CLKSTOP_MAC1_MASK:
		reset_mask = RESET_MAC1_RESET;
		break;
	case CLKSTOP_MAC2_MASK:
		reset_mask = RESET_MAC2_RESET;
		break;
	case CLKSTOP_I2S_MASK:
		reset_mask = RESET_I2S_RESET;
		break;
	case CLKSTOP_IR_MASK:
		reset_mask = RESET_IR_RESET;
		clk_stable_time = SCU_SYSTEM_CLK_SHORT_DELAY_MS;
		break;
	default:
		/* TODO warning message */
		return;
	}

	if (stop)
		goto exit_off;

	if (!reset_mask)
		goto exit_on_without_reset;
	/*
	 * 1. activate RESET
	 * 2. hold for a shot period of time (100us)
	 * 3. enable clock
	 * 4. delay 10ms
	 * 5. de-activate RESET
	 */
	SET_SCU_REG_BITS(RSCU_RESET, reset_mask);
	mb();
	udelay(SCU_SYSTEM_RESET_DELAY);
	CLR_SCU_REG_BITS(RSCU_CLKSTOP, mask);
	mdelay(clk_stable_time);
	CLR_SCU_REG_BITS(RSCU_RESET, reset_mask);
	return;

exit_on_without_reset:
	CLR_SCU_REG_BITS(RSCU_CLKSTOP, mask);
	mdelay(clk_stable_time);
	return;

exit_off:
	SET_SCU_REG_BITS(RSCU_CLKSTOP, mask);
	return;
}


static unsigned int get_HPLL_Hz(void)
{
	unsigned int scu24, hpll_select;

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

		//hpll = 24MHz * (2-OD) * ((Numerator+2)/(Denumerator+1))
		return (24000 * (2-OD) * ((num+2)*1000/(denum+1)));

	}

	hpll_select = HWSTRAP1_HPLLSLT_GET(GET_SCU_REG(RSCU_HWSTRAP1));
	switch (hpll_select) {
	case HWSTRAP1_HPLLSLT_384MHZ:
		return 384000000;
	case HWSTRAP1_HPLLSLT_360MHZ:
		return 360000000;
	case HWSTRAP1_HPLLSLT_336MHZ:
		return 336000000;
	case HWSTRAP1_HPLLSLT_408MHZ:
		return 408000000;
	default:
		BUG();
	};
}

static unsigned int get_HPLL_mode(void)
{
	//BruceToDo
	return 0;
}

static unsigned int set_HPLL_mode(unsigned int mode)
{
	//BruceToDo
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

static u32 get_board_type(void)
{
	return GET_SCU_REG(RSCU_HWSTRAP1) & HWSTRAP1_ISCLIENT_MASK;
}

static video_type_e get_video_type(void)
{
#if (BOARD_DESIGN_VER_TRAP >= 204)

	return  (video_type_e)(HWSTRAP1_VDOTYPE_GET(GET_SCU_REG(RSCU_HWSTRAP1)));
//	return  (video_type_e)(GET_SCU_REG(RSCU_HWSTRAP1) & HWSTRAP1_VIDEOTYPE_MASK >> HWSTRAP1_VIDEOTYPE_BIT);
#else
	// Just a default value. Can't be VT_DISABLE.
	return VT_HDMI;
#endif
}

static video_type_e get_video_loopback_type(void)
{
#if (BOARD_DESIGN_VER_TRAP >= 204)
	return ((GET_SCU_REG(RSCU_HWSTRAP1) & HWSTRAP1_LBSTUS_MASK)?(VT_HDMI):(VT_DISABLE));
#else
	return VT_HDMI;
#endif
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

static inline void __scu_crt_crt2_pin_en(unsigned int en)
{
	if (en) {
		MOD_SCU_REG(RSCU_PINCTRL5,
					PINCTRL5_DGLVDOPIN(2),
					PINCTRL5_DGLVDOPIN_MASK);

		MOD_SCU_REG(RSCU_PINCTRL2,
					0xFF3E0000,
					0xFF3E0000);

		MOD_SCU_REG(RSCU_PINCTRL3,
					0x000FFFFC,
					0x000FFFFC);
	} else {
		MOD_SCU_REG(RSCU_PINCTRL5,
					PINCTRL5_DGLVDOPIN(0),
					PINCTRL5_DGLVDOPIN_MASK);

		MOD_SCU_REG(RSCU_PINCTRL2,
					0x00000000,
					0xFF3E0000);

		MOD_SCU_REG(RSCU_PINCTRL3,
					0x0000000,
					0x000FFFFC);
	}
}

static int _scu_crt_crt2_pin_en(unsigned int en)
{
	SCU_START();

	__scu_crt_crt2_pin_en(en);

	SCU_END();

	return 0;
}

static int _scu_crt_crt2_init(void)
{
	SCU_START();

	SET_SCU_REG_BITS(RSCU_RESET, RESET_CRT_RESET);

	MOD_SCU_REG(RSCU_CLKSEL,
				CLKSEL_PORTACLKSEL(0),
				CLKSEL_PORTACLKSEL_MASK);

	__scu_crt_set_VPA_clk_delay(VPCLKDLY_I0d5NS);

	MOD_SCU_REG(RSCU_MISC1CTRL,
				MISC1CTRL_SELDACSRC(1) | MISC1CTRL_SELDVOSRC(1),
				MISC1CTRL_SELDACSRC_MASK | MISC1CTRL_SELDVOSRC_MASK);

	__scu_crt_crt2_pin_en(1);

	MOD_SCU_REG(RSCU_CLKSTOP,
				CLKSTOP_D1CLK(0) | CLKSTOP_DCLK(0),
				CLKSTOP_D1CLK_MASK | CLKSTOP_DCLK_MASK);

	MOD_SCU_REG(RSCU_MISC1CTRL, MISC1CTRL_D2PLLOFF(0), MISC1CTRL_D2PLLOFF_MASK);

	CLR_SCU_REG_BITS(RSCU_RESET, RESET_CRT_RESET);

	SCU_END();

	return 0;
}

/* from CRT engine */
static int _scu_crt_dac_ctrl_from_crt(void)
{
	SCU_START();

	/*
	** Bruce150701. AST1510 Fine tune loopback DAC gain value. Value from Johnny.
	*/
	MOD_SCU_REG(RSCU_MISC2CTRL,
	                   MISC2CTRL_DACGAIN(0x2),
	                   MISC2CTRL_DACGAIN_MASK);

	/* Enable dac */
	MOD_SCU_REG(RSCU_PINCTRL2,
				PINCTRL2_ENVGAHS_EN | PINCTRL2_ENVGAVS_EN,
				PINCTRL2_ENVGAHS_MASK | PINCTRL2_ENVGAVS_MASK);

	/* Select the DAC source for display output : Graphics CRT mode */
	MOD_SCU_REG(RSCU_MISC1CTRL,
				MISC1CTRL_SELDACSRC(1),
				MISC1CTRL_SELDACSRC_MASK);

	SCU_END();

	return 0;
}

/* from DVO */
static int _scu_crt_dac_ctrl_from_dvo(void)
{
	SCU_START();

	/*
	** Bruce150701. AST1510 Fine tune loopback DAC gain value. Value from Johnny.
	*/
	MOD_SCU_REG(RSCU_MISC2CTRL,
	                   MISC2CTRL_DACGAIN(0x2),
	                   MISC2CTRL_DACGAIN_MASK);

	/* Set multi-pin. AST1510's sync failed to bypass, so we disable it and use it as GPIO control. */
	MOD_SCU_REG(RSCU_PINCTRL2,
				0,
				PINCTRL2_ENVGAHS_MASK | PINCTRL2_ENVGAVS_MASK);

	/* Enable GPIOJ5, turn on Sync */
	gpio_direction_output(GPIO_HOST_DAC_SWITCH, GPIO_HOST_DAC_SWITCH_ON);

	/* Enable DVO bypass mode */
	MOD_SCU_REG(RSCU_MISC1CTRL,
				MISC1CTRL_SELDACSRC(0x2),
				MISC1CTRL_SELDACSRC_MASK);

	SCU_END();

	return 0;
}

/* disable */
static int _scu_crt_dac_ctrl_disable(void)
{
	SCU_START();

	MOD_SCU_REG(RSCU_PINCTRL2,
				0,
				PINCTRL2_ENVGAHS_MASK | PINCTRL2_ENVGAVS_MASK);

	/* Host need to do it */
	if(ast_scu.board_info.is_client == 0) {
		/* Enable GPIOJ5, turn off Sync */
		gpio_direction_output(GPIO_HOST_DAC_SWITCH, GPIO_HOST_DAC_SWITCH_OFF);
	}

	/* Disable DVO bypass mode */
	MOD_SCU_REG(RSCU_MISC1CTRL,
				MISC1CTRL_SELDACSRC(0),
				MISC1CTRL_SELDACSRC_MASK);

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

static int _scu_ve_init_encode(void)
{
	SCU_START();

	__scu_ve_reset();

	//open vide capture 1 and capture2
	MOD_SCU_REG(RSCU_CLKSTOP,
				CLKSTOP_ECLK(0) | CLKSTOP_V1CLK(0) | CLKSTOP_V2CLK(0),
				CLKSTOP_ECLK_MASK | CLKSTOP_V1CLK_MASK | CLKSTOP_V2CLK_MASK);

	MOD_SCU_REG(RSCU_PINCTRL5,
				PINCTRL5_DGLVDOPIN(0x2),
				PINCTRL5_DGLVDOPIN_MASK);

	MOD_SCU_REG(RSCU_PINCTRL2,
				0xFF3E0000,
				0xFF3E0000);

	MOD_SCU_REG(RSCU_PINCTRL3,
				0x000FFFFC,
				0x000FFFFC);

	SCU_END();

	return 0;
}

static int _scu_ve_init_decode(void)
{
	SCU_START();

	SET_SCU_REG(RSCU_ULCKIP, ULCKIP_VEDCDE);

	MOD_SCU_REG(RSCU_CLKSTOP,
				CLKSTOP_ECLK(0) | CLKSTOP_V1CLK(0) | CLKSTOP_V2CLK(0) ,
				CLKSTOP_ECLK_MASK | CLKSTOP_V1CLK_MASK | CLKSTOP_V2CLK_MASK);
	SCU_END();

	return 0;
}

static int _scu_i2s_init(void)
{
	SCU_START();

	SET_SCU_REG(RSCU_ULCKIP, ULCKIP_I2S);
	mb();

	if (!(GET_SCU_REG(RSCU_ULCKIP) & ULCKIP_I2S_ON))
		uerr("Can't unlock I2S IP\n");

	/* Use D2-PLL as I2S clock source */
	MOD_SCU_REG(RSCU_MISC1CTRL,
#if 0
	            MISC1CTRL_SELPLL(MISC1CTRL_SELPLL_D1VGA_D2I2S) | MISC1CTRL_D2PLLOFF(0),
#else
	            MISC1CTRL_SELPLL(MISC1CTRL_SELPLL_D1I2S_D2GFX) | MISC1CTRL_D2PLLOFF(0),
#endif
	            MISC1CTRL_SELPLL_MASK | MISC1CTRL_D2PLLOFF_MASK);

#if (BOARD_DESIGN_VER_I2S >= 204)
	/*
	** For GPIO_AUDIO_CODEC_IN_HOTPLUG GPIOH hotplug detection.
	** Disable GPIOH's internal pull down resister so that it won't
	** affect board hotplug design.
	** BruceToDo. Shell we move GPIO config to here?
	*/
	SET_SCU_REG_BITS(RSCU_PINCTRL4, PINCTRL4_NOITRNLPULLDWN_GPIOH);
#endif

#if (BOARD_DESIGN_VER_I2S >= 205)
	//GPIOJ6,J7 need mult-pin config. Set SCU84[14:15] to 0
	MOD_SCU_REG(RSCU_PINCTRL2,
	            0,
	            PINCTRL2_ENVGADDCCLK_MASK | PINCTRL2_ENVGADDCDAT_MASK);

#endif

	SCU_END();

	return 0;
}

static int _scu_i2s_pin_hdmi_h(void)
{
	return gpio_direction_output(GPIO_AUDIO_IO_SELECT, 0);
}

static int _scu_i2s_pin_hdmi_c(void)
{
	return gpio_direction_output(GPIO_AUDIO_IO_SELECT, 0);
}

static int _scu_i2s_pin_codec_h(void)
{
	return gpio_direction_output(GPIO_AUDIO_IO_SELECT, 1);
}

static int _scu_i2s_pin_codec_c(void)
{
	return gpio_direction_output(GPIO_AUDIO_IO_SELECT, 1);
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

	mb();
	// Enable I2S clock
	CLR_SCU_REG_BITS(RSCU_CLKSTOP, CLKSTOP_I2S);

	SCU_END();
	//Wait for clock stable.
	msleep(10);
	SCU_START();

	CLR_SCU_REG_BITS(RSCU_RESET, RESET_I2S_RESET);
	mb();

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

static int _scu_usb11hc_init(int num_ports)
{
	SCU_START();

	switch (num_ports) {
	case 1:
		MOD_SCU_REG(RSCU_PINCTRL5,
		            PINCTRL5_USB11P2PIN(PINCTRL5_USB11P2PIN_HID),
		            PINCTRL5_USB11P2PIN_MASK);
		mb();
		break;
	case 2:
		MOD_SCU_REG(RSCU_PINCTRL5,
		            PINCTRL5_USB11P2PIN(PINCTRL5_USB11P2PIN_HC),
		            PINCTRL5_USB11P2PIN_MASK);
		mb();
		break;
	default:
		uerr("unsupported number of USB 1.1 HC ports. (%d)\n", num_ports);
		BUG();
	};

	SET_SCU_REG_BITS(RSCU_RESET, RESET_USB11HC_RESET);
	mb();

	CLR_SCU_REG_BITS(RSCU_CLKSTOP, CLKSTOP_USB11HC);
	mb();
	mdelay(10);

	CLR_SCU_REG_BITS(RSCU_RESET, RESET_USB11HC_RESET);
	mb();
	udelay(500);

	SCU_END();
	return 0;
}

static int _scu_uart_init(u32 base)
{
	switch (base) {
	case ASPEED_UART_SOIP_BASE: //0x1E783000 UART#1
		//init UART#1 for SoIP port

#if (BOARD_DESIGN_VER_I2S >= 205)
		{
			u32 r;
			//AST1510 Use UART#1 engine, but route UART6 pin to UART#1 engine.
			//enable UART6 function pins
			SET_SCU_REG_BITS(RSCU_PINCTRL5, PINCTRL5_UART6_EN);
			//route UART1 to IO6 & disable UART1 reset source from LPC
			r = __raw_readl(IO_ADDRESS(0x1e789098));
			r &= ~0x710;
			__raw_writel(r, IO_ADDRESS(0x1e789098));
			//route IO6 to UART1
			r = __raw_readl(IO_ADDRESS(0x1e78909c));
			r |= 0x70000;
			__raw_writel(r, IO_ADDRESS(0x1e78909c));
		}
#endif

		break;
	case ASPEED_UART_DBG_BASE: //0x1E784000 UART#2
		//Debug console. Do nothing.
		break;
	default:
		break;
	};
	return 0;
}

static int _scu_ir_init(void)
{
	SCU_START();

	SET_SCU_REG(RSCU_ULCKIP, ULCKIP_IR);
	mb();

	if (!(GET_SCU_REG(RSCU_ULCKIP) & ULCKIP_IR_ON))
		uerr("Can't unlock IR IP\n");

	scu_system_clk_stop(CLKSTOP_IR_MASK, 0);

	SCU_END();

	return 0;
}

static int _scu_ir_stop(void)
{
	SCU_START();

	//To reset state.
	SET_SCU_REG_BITS(RSCU_RESET, RESET_IR_RESET);
	wmb();

	//delay 10 us to make sure the DMA of the IR engine is IDLE.
	udelay(10);

	SCU_END();

	return 0;
}

#ifdef FPGA_TEST_ONLY
static int _scu_d1pll_set(int HZx100)
{
	SCU_START();
	/* Enable by set SCU2C[18:16] to 101b  */
	MOD_SCU_REG(RSCU_MISC1CTRL,
	            MISC1CTRL_SELDACSRC(1) | MISC1CTRL_SELDVOSRC(1),
	            MISC1CTRL_SELDACSRC_MASK | MISC1CTRL_SELDVOSRC_MASK);

	SCU_END();
	return 0;
}
#endif

inline void _scu_init_board_info(void)
{
	board_info_t *info = &ast_scu.board_info;

	info->soc_ver = 2;
	info->soc_revision = REVISION_GET(ast_scu.revision_id) + 20;
	info->is_client = get_board_type();
	info->video_type = get_video_type();
	info->video_loopback_type = get_video_loopback_type();
	info->is_internal_video_capture = 0;

#ifdef FPGA_TEST_ONLY
	info->video_type = VT_HDMI;
	info->video_loopback_type = VT_DISABLE;

	#ifdef CONFIG_ARCH_AST1500_HOST
		info->is_client = 0;
	#else
		info->is_client = 1;
	#endif
#endif
}

static int _scu_init(void)
{
	ast_scu.revision_id = GET_SCU_REG(RSCU_REVISION);
	ast_scu.MPLL_Hz = 0; //TBD
	ast_scu.HPLL_Hz = get_HPLL_Hz();
	ast_scu.PCLK_Hz = get_PCLK_Hz();
	ast_scu.HCLK_Mhz = get_HCLK_MHz();
	ast_scu.HPLL_Mode = get_HPLL_mode();

	/* Start of Misc. SCU init. */
	// Reset SDRAM controller on full chip watchdog reset.
	SET_SCU_REG_BITS(RSCU_RESET, RESET_DRAM_WDT);
}

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
		case SCUOP_USB11HC_INIT:
			rtn = _scu_usb11hc_init((int)param);
			break;
		case SCUOP_D1PLL_SET:
#ifdef FPGA_TEST_ONLY
			rtn = _scu_d1pll_set((int)param);
#endif
			break;
		case SCUOP_HPLL_SET:
			rtn = set_HPLL_mode((int)param);
			break;
		case SCUOP_CRT_CRT2_INIT:
			rtn = _scu_crt_crt2_init();
			break;
		case SCUOP_CRT_CRT2_PIN:
			rtn = _scu_crt_crt2_pin_en((unsigned int)param);
			break;
		case SCUOP_CRT_DAC_CTRL_FROM_CRT:
			rtn = _scu_crt_dac_ctrl_from_crt();
			break;
		case SCUOP_CRT_DAC_CTRL_FROM_DVO:
			rtn = _scu_crt_dac_ctrl_from_dvo();
			break;
		case SCUOP_CRT_DAC_CTRL_DISABLE:
			rtn = _scu_crt_dac_ctrl_disable();
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
		case SCUOP_VE_D2CLK_SELECTION: //TBD. According to HJ. AST1510 also NEED to enable CRT clock in order to use line buffer.
			break;
#if 1
		case SCUOP_I2S_INIT:
			rtn = _scu_i2s_init();
			break;
#endif
		case SCUOP_I2S_PIN_HDMI_H:
			rtn = _scu_i2s_pin_hdmi_h();
			break;
		case SCUOP_I2S_PIN_HDMI_C:
			rtn = _scu_i2s_pin_hdmi_c();
			break;
		case SCUOP_I2S_PIN_CODEC_H:
			rtn = _scu_i2s_pin_codec_h();
			break;
		case SCUOP_I2S_PIN_CODEC_C:
			rtn = _scu_i2s_pin_codec_c();
			break;
		case SCUOP_I2S_STOP:
			rtn = _scu_i2s_stop();
			break;
		case SCUOP_I2S_START:
			rtn = _scu_i2s_start();
			break;
		case SCUOP_UART_INIT:
			rtn = _scu_uart_init((u32)param);
			break;
		case SCUOP_IR_INIT:
			rtn = _scu_ir_init();
			break;
		case SCUOP_IR_STOP:
			rtn = _scu_ir_stop();
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

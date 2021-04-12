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

#if (CONFIG_AST1500_SOC_VER == 3)
#include <linux/err.h> //IS_ERR()
#include <linux/module.h>
#include <asm/arch/platform.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <asm/arch/regs-scu.h>
#include <asm/arch/ast-scu.h>
#include "ast-scu-drv.h"
#include <asm/arch/drivers/board_def.h>
#include <asm/arch/gpio.h>

#include "ast-scu_ast1520_d1pll.h"
#include "ast-scu_ast1520_d2pll.h"

#if defined(MAC_RGMII_CLOCK_SELECT_D2_PLL)
#define AST1520_D2_PLL_NO_CHANGE
#endif

#if defined(AST1520_D2_PLL_NO_CHANGE)
static int d2_pll_init_done = 0;
#endif

static const char *video_type_s[] = {
	"VGA(0)",
	"Disable(1)",
	"HDMI(2)",
	"DVI(3)"
};

#define MODULE_NAME "scu"
const char driver_name[] = MODULE_NAME;

static inline int _scu_d1pll_set_ve(int HZx100);

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
	case CLKSTOP_UART_4_MASK:
	case CLKSTOP_UART_3_MASK:
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
	case CLKSTOP_RSA_MASK:
		/* TODO */
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

/*
 * scu_system_reset -
 * @mask:
 */
static inline void scu_system_reset(u32 mask)
{
	SET_SCU_REG_BITS(RSCU_RESET, mask);
	mb();
	udelay(SCU_SYSTEM_RESET_DELAY);
	CLR_SCU_REG_BITS(RSCU_RESET, mask);
	mb();
}

/*
* i2c_mfp_cfg - I2C multiple function pin configuration
*
* @id: I2C ID
*     Counting from 0, 1 to 14. 0 is a special value not used here..
* @enable: enable or disable
*/
static int i2c_mfp_cfg(u8 id, u8 enable)
{
	u32 mask = 0;

	switch (id) {
	case 0: //pass through
	case 1: //pass through
	case 2:
		goto out;
	case 3:
		mask = MFPC5_I2C3_EN;
		break;
	case 4:
		mask = MFPC5_I2C4_EN;
		break;
	case 5:
		mask = MFPC5_I2C5_EN;
		break;
	case 6:
		mask = MFPC5_I2C6_EN;
		break;
	case 7:
		mask = MFPC5_I2C7_EN;
		break;
	case 8:
		mask = MFPC5_I2C8_EN;
		break;
	case 9:
		mask = MFPC5_I2C9_EN;
		break;
	case 10:
		mask = MFPC5_I2C10_EN;
		break;
	case 11:
		mask = MFPC5_I2C11_EN;
		break;
	case 12:
		mask = MFPC5_I2C12_EN;
		break;
	case 13:
		mask = MFPC5_I2C13_EN;
		break;
	case 14:
		mask = MFPC5_I2C14_EN;
		break;
	default:
		BUG();
		break;
	};

	if (enable)
		SET_SCU_REG_BITS(RSCU_MFPC5, mask);
	else
		CLR_SCU_REG_BITS(RSCU_MFPC5, mask);

out:
	return 0;
}

/*
 * mac_mfp_cfg - MAC multiple function pin configuration
 *
 * @id: MAC ID
 * @enable: enable or disable
 */
static int mac_mfp_cfg(u8 id, u8 enable)
{
	if (id == 1) {
		if (enable) {
			/* MDC/MDIO pin */
			SET_SCU_REG_BITS(RSCU_MFPC3, MFPC3_MAC1MDIO_EN);
			/* MII pin */
			/* clear to 0 means MAC function*/
			CLR_SCU_REG_BITS(RSCU_MFPC7, MFPC7_MAC1MII_OFF_MASK);
			/* PHY link interrupt */
			SET_SCU_REG_BITS(RSCU_MFPC1, MFPC1_MAC1PHYINT_EN);
		} else {
			/* MDC/MDIO pin */
			CLR_SCU_REG_BITS(RSCU_MFPC3, MFPC3_MAC1MDIO_EN);
			/* MII pin */
			/* clear to 0 means MAC function*/
			SET_SCU_REG_BITS(RSCU_MFPC7, MFPC7_MAC1MII_OFF_MASK);
			/* PHY link interrupt */
			CLR_SCU_REG_BITS(RSCU_MFPC1, MFPC1_MAC1PHYINT_EN);
		}
		return 0;

	}

	if (id == 2) {
		if (enable) {
			/* MDC/MDIO pin */
			SET_SCU_REG_BITS(RSCU_MFPC5, MFPC5_MAC2MDIO_EN);
			/* MII pin */
			/* clear to 0 means MAC function*/
			CLR_SCU_REG_BITS(RSCU_MFPC7, MFPC7_MAC2MII_OFF_MASK);
			/* PHY link interrupt */
			SET_SCU_REG_BITS(RSCU_MFPC1, MFPC1_MAC2PHYINT_EN);
		} else {
			/* MDC/MDIO pin */
			CLR_SCU_REG_BITS(RSCU_MFPC5, MFPC5_MAC2MDIO_EN);
			/* MII pin */
			SET_SCU_REG_BITS(RSCU_MFPC7, MFPC7_MAC2MII_OFF_MASK);

			/* PHY link interrupt */
			CLR_SCU_REG_BITS(RSCU_MFPC1, MFPC1_MAC2PHYINT_EN);
		}
		return 0;
	}
	return -1;
}

/*
 * get_clk_in_Hz - return the value of CLKIN. 24MHz or 25MHz.
 */
static u32 get_clk_in_Hz(void)
{
	if (HWSTRAP1_CLKIN_25MHZ & GET_SCU_REG(RSCU_HWSTRAP1))
		return 25*1000*1000;

	return 24*1000*1000;
}

/*
 * get_MPLL_Hz - return MPLL frequency
 */
static unsigned int get_MPLL_Hz(void)
{
	u32 reg, clk_in;

	reg = GET_SCU_REG(RSCU_MPLL);

	/* If HPLL is turned off */
	if (MPLL_MPLL_OFF & reg)
		return 0;

	clk_in = get_clk_in_Hz();

	/* If HPLL bypass mode is enabled. */
	if (MPLL_BYPASS_EN & reg)
		return clk_in;

	{
		/* mpll = clk_in * [(M+1) /(N+1)] / (P+1)
		 * P = SCU20[18:13]
		 * M = SCU20[12:5]
		 * N = SCU20[4:0]
		 */
		u32 P, M, N;
		P = MPLL_P_GET(reg);
		M = MPLL_M_GET(reg);
		N = MPLL_N_GET(reg);

		//From Gary.
		//DDR clock is MPLL x 2.
		//The other HW IP which uses M-Bus clock is MPLL / 2.
		return (clk_in / 1000 * ((M + 1) / (N + 1)) / (P + 1) * 1000);
	}
}

/*
 * get_HPLL_Hz - return HPLL frequency
 */
static unsigned int get_HPLL_Hz(void)
{
	u32 reg, clk_in;

	reg = GET_SCU_REG(RSCU_HPLL);

	/* If HPLL is turned off */
	if (HPLL_HPLL_OFF & reg)
		return 0;

	clk_in = get_clk_in_Hz();

	/* If HPLL bypass mode is enabled. */
	if (HPLL_BYPASS_EN & reg)
		return clk_in;

	{
		/* hpll = clk_in * [(M+1) /(N+1)] / (P+1)
		 * P = SCU24[18:13]
		 * M = SCU24[12:5]
		 * N = SCU24[4:0]
		 */
		u32 P, M, N;
		P = HPLL_P_GET(reg);
		M = HPLL_M_GET(reg);
		N = HPLL_N_GET(reg);
		return (clk_in / 1000 * ((M + 1) / (N + 1)) / (P + 1) * 1000);
	}
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
	u32 scu70, hpll, cpu2axi, axi2ahb;

	hpll = get_HPLL_Hz();

	scu70 = GET_SCU_REG(RSCU_HWSTRAP1);
	cpu2axi = 2; /* AST152x is always 2. */
	axi2ahb = HWSTRAP1_AXI2AHB_RATIO_GET(scu70) + 1;

	uinfo("HPLL=%d MHz, AXI=%d MHz, HCLK(AHB)=%d MHz, (%d:%d:1)\n",
	       hpll/1000000,
	       hpll/cpu2axi/1000000,
	       hpll/cpu2axi/axi2ahb/1000000,
	       cpu2axi, axi2ahb);

	return hpll/cpu2axi/axi2ahb/1000000;
}

static unsigned int get_PCLK_Hz(void)
{
	u32 div;
	unsigned int pclk;

	/* PCLK == HPLL/div, div is from SCU08[25:23] */
	div = (CLKSEL_PCLKD_GET(GET_SCU_REG(RSCU_CLKSEL)) + 1) << 2;
	pclk = ast_scu.HPLL_Hz/div;

	uinfo("PCLK(APB)=%d MHz (HPLL/%d)\n", pclk/1000000, div);

	return pclk;
}

static u32 get_board_type(void)
{
#if defined(CONFIG_ARCH_AST1500_CLIENT)
	return 1;
#endif
	return 0;
}

static video_type_e get_video_type(void)
{
	unsigned int result;

	if (astparam_get_hex("v_type", &result)) {
		/* No "v_type" defined. Select from GPIO */
		/* read AST1520 GPIOQ4. 1:HDMI 0:VGA */
		//gpio_direction_input(AST1500_GPQ4); Do we need to cfg before read?
		if (gpio_get_value(GPIO_V_TYPE))
			result = VT_HDMI;
		else
			result = VT_ANALOG;
	}
	return (video_type_e)result;
}

static inline unsigned int is_ast1525(void)
{
	/* AST1520 efuse[4:0] is 0x10, AST1525 is 0x11. */
	return ((GET_SCU_REG(0x0d0) & 0x1FUL) == 0x11);
}

static video_type_e get_video_loopback_type(void)
{
	unsigned int result;

	if (astparam_get_hex("v_loopback_type", &result)) {
		if (ast_scu.board_info.board_revision & BOARD_REV_PATCH_VIDEO_SPLITTER) {
			/*
			 * BOARD_REV_PATCH_VIDEO_SPLITTER is set => a splitter on host
			 * set HDMI by default
			 */
			switch (BOARD_REV_PATCH_MINOR_VERSION(ast_scu.board_info.board_revision)) {
			case 0x00:
				/* TODO is it ok to return VT_HDMI on this? */
				result = VT_HDMI;
				break;
			case 0x10:
				/*
				 * IT6663, one-to-two HDMI 2.0 splitter, it was controlled by MCU
				 * we are unable to control(enable/disable) this loopback interface
				 * return VT_DISABLE to avoid invalid operations
				 */
				result = VT_DISABLE;
				break;
			default:
				/* unknown setting, return VT_DISABLE */
				result = VT_DISABLE;
				break;

			}
		} else {
			/* No "v_loopback_type" defined. Select from GPIO */
			/* Read AST1520 GPIOAA5. 1:Available 0:Disable */
			if (gpio_get_value(AST1500_GPAA5)) {
				if (is_ast1525())
					result = ast_scu.board_info.video_type;
				else /* default board_revision 300 design. Has VGA loopback. */
					result = VT_ANALOG;
			} else {
				result = VT_DISABLE;
			}
		}
	}

	return (video_type_e)result;
}

static u32 get_board_revision(void)
{
	u32 result;

	if (astparam_get_hex("board_revision", &result))
		result = BOARD_REV_PATCH_DEFAULT;

	return result;
}

#ifdef CONFIG_ARCH_AST1500_CLIENT
static u32 _calc_ring(u32 step)
{
	volatile u32 r;

	SET_SCU_REG(RSCU_FCNTCTRL, 0x30);
	mb();
	// Wait until SCU10[29:16] = 0
	do {
		mdelay(1);
		r = GET_SCU_REG(RSCU_FCNTCTRL);
		r = (r >> 16) & 0x3FFF;
	} while(r);
	/*
	** Set SCU10[0] = 1 and SCU10[5:2] = clock for measurement
	** SCU10[5:2] Set to 0011 (0x3) for DLY32 clock.
	** SCU10[5:2] Set to 1000 (0x8) for MCLK clock.
	** SCU10[9:14] Set to delay stage to test. NOTE:SCU10[14] should be ignored and be 0.
	*/
	r = ((step & 0x1F) << 9) | (0x3 << 2) | 0x1;
	SET_SCU_REG(RSCU_FCNTCTRL, r);
	// delay 1ms
	mdelay(1);
	// Set SCU10[1] = 1
	MOD_SCU_REG(RSCU_FCNTCTRL, 0x2, 0x2);
	mb();
	// Wait until SCU10[6] = 1
	do {
		mdelay(1);
		r = GET_SCU_REG(RSCU_FCNTCTRL);
		r = (r >> 6) & 0x1;
	} while(!r);
	// Read SCU10[29:16] for the result. And calculate OSCCLK frequency.
	r = GET_SCU_REG(RSCU_FCNTCTRL);
	r = (r >> 16) & 0x3FFF;
	/*
	** OSCCLK_freq = ( CLK24M / 512 * (SCU10[29:16] + 1) ) * DLY32_Multiply
	** CLK24M = 24000000
	** SCU10[29:16] = $RING
	** DLY32_Multiply = 4
	** ==>
	** OSCCLK_freq = 24000000 * (r + 1) * 4 / 512 Hz
	** ==> OSCCLK_ps = (512 * 10^12) / ((r + 1) * 4 * 24 * 10^6)
	** ==> OSCCLK_ps = 16 * 10^6 / (3 * (r + 1))
	*/
	r = 16 * 1000 * 1000 / (3 * (r + 1));

	//uinfo("ring(0x%x)=%d ps\n", ((step << 6) | 0x3d), r);
	return r;
}

static u32 calc_ring(u32 step)
{
	u32 r = 0, cnt = 4;
	unsigned int i;

	/* average version of _cale_ring(). */
	for (i = 0; i < cnt; i++) {
		r += _calc_ring(step);
	}
	r /= cnt;
	uinfo("ring(0x%x)=%d ps\n", ((step << 6) | 0x3d), r);
	return r;
}

#define my_abs(x, y) (((x) >= (y))?((x)-(y)):((y)-(x)))

static u32 calc_v_clk_delay_4k(unsigned int ps_target)
{
	u32 try_step, next_try_step, low_step, high_step;
	u32 ps_min, ps_max, ps_got;

	low_step = 0;
	high_step = 0x1F;

	uinfo("ring info >>>>>>>>>>\n");
	calc_ring(6);
	calc_ring(7);
	calc_ring(8);
	calc_ring(9);
	calc_ring(0xA);
	calc_ring(0xB);
	calc_ring(0xC);
	calc_ring(0xD);
	calc_ring(0xE);
	calc_ring(0x14);
	calc_ring(0x15);
	calc_ring(0x16);
	calc_ring(0x17);
	calc_ring(0x18);
	calc_ring(0x19);
	calc_ring(0x1A);
	calc_ring(0x1B);
	calc_ring(0x1C);
	calc_ring(0x1D);
	calc_ring(0x1E);
	calc_ring(0x1F);
	uinfo("<<<<<<<<<<<<<<<<<<<<\n");

	ps_min = calc_ring(low_step);
	ps_max = calc_ring(high_step);
#if 0
#ifdef FALLING_EDGE_LATCH
	//6000, 6500 ,7000, 7500, 8000, 8500, 9000
	if (ps_max < 6000) {
		ps_target = 6700;
	} else if (ps_max < 6500) {
		ps_target = 6800;
	} else if (ps_max < 7000) {
		ps_target = 6800;
	} else if (ps_max < 7500) {
		ps_target = 7000;
	} else if (ps_max < 8000) {
		ps_target = 7200;
	} else if (ps_max < 8500) {
		ps_target = 7200;
	} else if (ps_max < 9000) {
		ps_target = 7400;
	} else {
		ps_target = 7400;
	}
#else
	ps_target = 3400;
#endif
#endif
	uinfo("Target %d ps\n", ps_target);
	if (ps_target < ps_min)
		return -1;
	if (ps_target > ps_max)
		return -2;

	next_try_step = ((ps_target - ps_min) * (high_step - low_step)) / (ps_max - ps_min);
	try_step = next_try_step;
	/* linear approaching the target step value. */
	do {
		try_step = next_try_step;
		ps_got = calc_ring(try_step);
		uinfo("Try SCUD8=0x%x, %d ps\n", ((try_step << 6) | 0x3d), ps_got);
		if (ps_got > ps_target) {
			ps_max = ps_got;
			high_step = try_step;
		} else {
			ps_min = ps_got;
			low_step = try_step;
		}
		if (ps_max <= ps_min)
			break;

		next_try_step = ((ps_target - ps_min) * (high_step - low_step)) / (ps_max - ps_min);
	} while (next_try_step);

	/* Find the abs nearest step. */
	if (my_abs(ps_max, ps_target) < my_abs(ps_target, ps_min)) {
		try_step = high_step;
	} else {
		try_step = low_step;
	}

	uinfo("Found SCUD8=0x%x, %d ps\n", ((try_step << 6) | 0x3d), calc_ring(try_step));
	/* "| 0x3d" is just for easily mapping to the value we are familiar with. It is actually not used in the code. */
	return ((try_step << 6) | 0x3d);
}

static u32 get_video_clk_delay_4k(scu_t *scu, unsigned int default_target)
{
	u32 result;

	/* astparam->v_clk_delay_target MUST be ready before calling get_video_clk_delay_4k(). */
	if (astparam_get_hex("v_clk_delay", &result)) {
		if (scu->board_info.soc_revision == 30)
			result = 0x3bd;
		else
			result = calc_v_clk_delay_4k(default_target);
	}

	if ((result < 0x13d) || (result > 0x4fd)) {
		uinfo("WARNING!! Unexpected v_clk_delay (0x%x)\n", result);
	}
	return result;
}

static u32 get_video_clk_delay_4k_target(scu_t *scu)
{
	u32 result;

	if (astparam_get_int("v_clk_delay_target", &result)) {
		/*
		** Bruce170526.
		** From test result. The best clock range is:
		** For SiI9136 falling latch:
		** - clk duty SCU1D8: 0x63
		** - clk delay SCUD8: 0x63d ~ 0x77d. around ring 7000 ps. ==> has higher variation between chips.
		** For SiI9136 rising latch:
		** - clk duty SCU1D8: 0x63. 0x65 failed under high temperature.
		** - clk delay SCUD8: 0x23d ~ 0x37d. around ring 3400 ps
		**
		** We pick rising latch because smaller SCUD8 has better/stable clk SSO result.
		*/
		/* This value is got from measurment. */
		result = 3400; //in ps
	}

	return result;
}

static u32 get_video_clk_duty_4k(scu_t *scu)
{
	u32 result;

	if (astparam_get_hex("v_clk_duty", &result)) {
		/*
		** Bruce170526.
		** From test result. The best clock range is:
		** For SiI9136 falling latch:
		** - clk duty SCU1D8: 0x63
		** - clk delay SCUD8: 0x63d ~ 0x77d. around ring 7000 ps. ==> has higher variation between chips.
		** For SiI9136 rising latch:
		** - clk duty SCU1D8: 0x63. 0x65 failed under high temperature.
		** - clk delay SCUD8: 0x23d ~ 0x37d. around ring 3400 ps
		**
		** We pick rising latch because smaller SCUD8 has better/stable clk SSO result.
		*/
		result = 0x63;
	}

	return result;
}
#endif /* #ifdef CONFIG_ARCH_AST1500_CLIENT */

static u32 get_usb_quirk(scu_t *scu)
{
	u32 result;

	if (astparam_get_hex("usb_quirk", &result))
		result = 0;

	return result;
}

static int mac_clk_divider_selection(void)
{
#if defined(MAC_RGMII_CLOCK_SELECT_D2_PLL)
	__scu_d2pll_set(D2_PLL_DEFAULT);
	MOD_SCU_REG(RSCU_MACCLK, MACCLK_RGMII_125M_CLK_SRC_SELECT_INTERNAL_PLL, MACCLK_RGMII_125M_CLK_SRC_SELECT_MASK);
	MOD_SCU_REG(RSCU_MISC2CTRL, MISC2CTRL_RGMII_125M_CLK_DIV_SRC_SELECT_D2_PLL, MISC2CTRL_RGMII_125M_CLK_DIV_SRC_SELECT_MASK);
	MOD_SCU_REG(RSCU_MISC2CTRL, MISC2CTRL_RGMII_125M_CLK_DIV_RATIO(5), MISC2CTRL_RGMII_125M_CLK_DIV_RATIO_MASK);
#endif
#ifdef PLATFORM_S /* Now handled by platform.S */
	u32 hpll_mhz, div_factor;

	hpll_mhz = ast_scu.HPLL_Hz / 1000000;
	div_factor = 1;

	while ((hpll_mhz/((div_factor+1) << 1)) > 100)
		div_factor++;

	div_factor--;

	{
		u32 d = ((div_factor + 1) << 1);
		uinfo("MAC Bus Clock: %d MHz (%d/%d)\n",
		                                        hpll_mhz / d ,
		                                        hpll_mhz,
		                                        d);
	}

	MOD_SCU_REG(RSCU_CLKSEL, CLKSEL_HPLL2MAC_DIV(div_factor), CLKSEL_HPLL2MAC_DIV_MASK);
#endif
}

static inline void mac_rx_tx_clk_delay(void)
{
	u32 val;

	switch (ast_scu.board_info.soc_revision) {
		case 30: //A0
#if defined(CONFIG_AST1520_BOARD_EVA_V1)
			/*
			 * From U-boot, scu_mac_init,
			 *
			 * we set a number 0x0001A009 to SCU048;
			 * mac1_tx_dly = 0x09;
			 * mac2_tx_dly = 0x00;
			 * mac1_rx_dly = 0x1A;
			 * mac2_rx_dly = 0x00;
			 */
			val = MACCLK_MAC1TXDLY(0x09)
				| MACCLK_MAC2TXDLY(0x00)
				| MACCLK_MAC1RXDLY(0x1A)
				| MACCLK_MAC2RXDLY(0x00);
#else /* if defined(CONFIG_AST1520_BOARD_EVA_V2) */
			val = MACCLK_MAC1TXDLY(0x09)
				| MACCLK_MAC2TXDLY(0x00)
				| MACCLK_MAC1RXDLY(0x1D)
				| MACCLK_MAC2RXDLY(0x00);
#endif
			SET_SCU_REG(RSCU_MACCLK, val);
			break;
		case 31: //A1
		default:
			SET_SCU_REG(RSCU_MACCLK, 0x82208);
			SET_SCU_REG(RSCU_MACCLK_100M, 0x82208);
			SET_SCU_REG(RSCU_MACCLK_10M, 0x82208);
#ifdef PLATFORM_S /* Now handled by platform.S */
			SET_SCU_REG(RSCU_CLK_DUTY_SELECTION_1, 0x006E6E00);
#endif
			break;
	}

}

static int _scu_mac_init(int who)
{
	SCU_START();

	if (mac_mfp_cfg(who, 1))
		return -1;

	mac_rx_tx_clk_delay();

	mac_clk_divider_selection();

	if (1 == who) {
		scu_system_clk_stop(CLKSTOP_MAC1_MASK, 1);
		scu_system_clk_stop(CLKSTOP_MAC1_MASK, 0);
	} else {
		scu_system_clk_stop(CLKSTOP_MAC2_MASK, 1);
		scu_system_clk_stop(CLKSTOP_MAC2_MASK, 0);
	}

	SCU_END();

	return 0;
}

static int _scu_i2c_init(int who)
{
	SCU_START();
#if 0 //For ast2500 ARM11 FPGA board?
#ifdef FPGA_TEST_ONLY
	//From Gary. AST1520 Video module I2C port share the same pin mux setting as Video.
	//We should enable the pin mux here for I2C ports to route to Video module daughter board.
	MOD_SCU_REG(RSCU_MFPC6, MFPC6_DVOPIN(MFPC6_DVOPIN_30BITS), MFPC6_DVOPIN_MASK);
	printk("Enable I2C pin mux (0x%08x)\n", GET_SCU_REG(RSCU_MFPC6));
#endif
#endif

	if (i2c_mfp_cfg(who, 1))
		return -1;

	// 0 means global reset.
	if (0 == who)
		scu_system_reset(RESET_I2C_RESET);

	SCU_END();

	return 0;
}


static int __scu_crt_dvo_timing_ctrl(int HZx100)
{
	if (HZx100 > 1700000) {
		MOD_SCU_REG(RSCU_CLK_DUTY_SELECTION, D1CLK_SEL(ast_scu.astparam.v_clk_duty), D1CLK_SEL_MASK);
#if 1
		MOD_SCU_REG(RSCU_CLKSEL_SET2, ast_scu.astparam.v_clk_delay, OUTPUT_CLK_DELAY_SEL_MASK | OUTPUT_CLK_DELAY_INVERT_MASK);
#else
		if (ast_scu.board_info.soc_revision == 30) {
			MOD_SCU_REG(RSCU_CLKSEL_SET2, OUTPUT_CLK_DELAY_INVERT(0)|OUTPUT_CLK_DELAY_SEL(14),OUTPUT_CLK_DELAY_SEL_MASK | OUTPUT_CLK_DELAY_INVERT_MASK);
		} else {
			/* A1 */
			MOD_SCU_REG(RSCU_CLKSEL_SET2, OUTPUT_CLK_DELAY_INVERT(1)|OUTPUT_CLK_DELAY_SEL(22),OUTPUT_CLK_DELAY_SEL_MASK | OUTPUT_CLK_DELAY_INVERT_MASK);
		}
#endif
	} else {
		MOD_SCU_REG(RSCU_CLK_DUTY_SELECTION, D1CLK_SEL(0x62), D1CLK_SEL_MASK);
		MOD_SCU_REG(RSCU_CLKSEL_SET2, OUTPUT_CLK_DELAY_INVERT(0)|OUTPUT_CLK_DELAY_SEL(0),OUTPUT_CLK_DELAY_SEL_MASK | OUTPUT_CLK_DELAY_INVERT_MASK);
	}
	mb();
	return 0;
}

static inline void __scu_crt_crt2_pin_en(unsigned int en)
{
	if (en) {
		SET_SCU_REG_BITS(RSCU_MFPC2, MFPC2_CRT_ALL_EN);
		SET_SCU_REG_BITS(RSCU_MFPC3, MFPC3_VPOA_ALL_EN);
		SET_SCU_REG_BITS(RSCU_MFPC4, MFPC4_VPOA_ALL_EN);
		MOD_SCU_REG(RSCU_MFPC6, MFPC6_DVOPIN(2), MFPC6_DVOPIN_MASK);
		SET_SCU_REG_BITS(RSCU_MFPC8, MFPC8_VPOA_ALL_EN);
		SET_SCU_REG_BITS(RSCU_MFPC9, MFPC9_VPOA_ALL_EN);
	} else {
		CLR_SCU_REG_BITS(RSCU_MFPC2, MFPC2_CRT_ALL_EN);
		CLR_SCU_REG_BITS(RSCU_MFPC3, MFPC3_VPOA_ALL_EN);
		CLR_SCU_REG_BITS(RSCU_MFPC4, MFPC4_VPOA_ALL_EN);
		MOD_SCU_REG(RSCU_MFPC6, MFPC6_DVOPIN(0), MFPC6_DVOPIN_MASK);
		CLR_SCU_REG_BITS(RSCU_MFPC8, MFPC8_VPOA_ALL_EN);
		CLR_SCU_REG_BITS(RSCU_MFPC9, MFPC9_VPOA_ALL_EN);
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

	/* multiple function pin configuration */
	__scu_crt_crt2_pin_en(1);

	/* clock disable */
	scu_system_clk_stop(CLKSTOP_D1CLK_MASK, 1);
	/* reset */
	SET_SCU_REG_BITS(RSCU_RESET2, RESET2_CRT2_RESET);
	mb();
	SET_SCU_REG_BITS(RSCU_RESET, RESET_CRT_RESET);
	mb();

	/* CRT output pixel clock use D1-PLL. */
	MOD_SCU_REG(RSCU_CLKSEL, 0, CLKSEL_CRTCLKL_MASK);
	MOD_SCU_REG(RSCU_MISC1CTRL, 0, MISC1CTRL_CRTCLKH_40M);

	/* Form Johnny's suggestion, set to 0x62, the duty cycle is 50.2% */
	/* D1 clock duty selection */
	MOD_SCU_REG(RSCU_CLK_DUTY_SELECTION, D1CLK_SEL(0x62), D1CLK_SEL_MASK);

	/* clock enable */
	scu_system_clk_stop(CLKSTOP_D1CLK_MASK, 0);
	/* clear reset */
	CLR_SCU_REG_BITS(RSCU_RESET, RESET_CRT_RESET);
	mb();
	CLR_SCU_REG_BITS(RSCU_RESET2, RESET2_CRT2_RESET);
	mb();
	// select CRT as DVO source
	MOD_SCU_REG(RSCU_MISC1CTRL,
				MISC1CTRL_SELDVOSRC(1) | MISC1CTRL_SELDACSRC(1) ,
				MISC1CTRL_SELDVOSRC_MASK | MISC1CTRL_SELDACSRC_MASK);

#ifdef CONFIG_AST1520_BOARD_FPGA_ARM9 //2400 arm9 FPGA board only.
	MOD_SCU_REG(RSCU_MISC1CTRL,
				MISC1CTRL_SELDACSRC(MISC1CTRL_SELDACSRC_CRT2),
				MISC1CTRL_SELDACSRC_MASK);
#endif

	SCU_END();

	return 0;
}

/* from CRT engine */
static int _scu_crt_dac_ctrl_from_crt(void)
{
	SCU_START();
	/* Enable dac */
	MOD_SCU_REG(RSCU_MFPC2,
			MFPC2_ENVGAHS_EN | MFPC2_ENVGAVS_EN,
			MFPC2_ENVGAHS_MASK | MFPC2_ENVGAVS_MASK);

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
	u32 is_port_b;

	SCU_START();

	/* We choose DVI source port by read MFPC6_DVI_SOURCE_SEL reg. */
	is_port_b = (MFPC6_DVI_SOURCE_SEL_GET(GET_SCU_REG(RSCU_MFPC6)) == MFPC6_DVI_SOURCE_PORT_B);

	/* Enable DVO bypass mode */
	if (is_port_b)
		MOD_SCU_REG(RSCU_MISC1CTRL,
				MISC1CTRL_SELDACSRC(MISC1CTRL_SELDACSRC_PSSTHRGHB),
				MISC1CTRL_SELDACSRC_MASK);
	else
		MOD_SCU_REG(RSCU_MISC1CTRL,
				MISC1CTRL_SELDACSRC(MISC1CTRL_SELDACSRC_PSSTHRGHA),
				MISC1CTRL_SELDACSRC_MASK);

	/* Enable dac HS, VS. */
	MOD_SCU_REG(RSCU_MFPC2,
			MFPC2_ENVGAHS_EN | MFPC2_ENVGAVS_EN,
			MFPC2_ENVGAHS_MASK | MFPC2_ENVGAVS_MASK);

	SCU_END();

	return 0;
}

/* disable */
static int _scu_crt_dac_ctrl_disable(void)
{
	SCU_START();

	/* Disable dac HS, VS. */
	MOD_SCU_REG(RSCU_MFPC2,
			0,
			MFPC2_ENVGAHS_MASK | MFPC2_ENVGAVS_MASK);

	/* Disable DVO bypass mode */
	MOD_SCU_REG(RSCU_MISC1CTRL,
			MISC1CTRL_SELDACSRC(MISC1CTRL_SELDACSRC_VGA),
			MISC1CTRL_SELDACSRC_MASK);
	SCU_END();

	return 0;
}

static int _scu_ve_reset(void)
{
	SCU_START();

	if ((ast_scu.board_info.soc_revision >= 31) && (ast_scu.board_info.ve_encode_clk)) {
		scu_system_clk_stop(CLKSTOP_ECLK_MASK, 1);
		scu_system_clk_stop(CLKSTOP_V1CLK_MASK, 1);

		/* Config VE Clock source and divider */
		if ((GET_SCU_REG(0xD0) & 0xFF) == 0) {
			_scu_d1pll_set_ve(ast_scu.board_info.ve_encode_clk);

			MOD_SCU_REG(RSCU_CLKSEL,
			            VESRC_D1PLL | VECLKDIV_DIV2,
			            CLKSEL_VESRC_MASK | CLKSEL_VECLKDIV_MASK);
		} else {
			MOD_SCU_REG(RSCU_CLKSEL,
			            VESRC_HPLL | VECLKDIV_DIV3,
			            CLKSEL_VESRC_MASK | CLKSEL_VECLKDIV_MASK);
		}

		scu_system_clk_stop(CLKSTOP_ECLK_MASK, 0);
		scu_system_clk_stop(CLKSTOP_V1CLK_MASK, 0);
	}

	//Set SCU0C[0] enable video engine clock (has enabled before)
	SET_SCU_REG_BITS(RSCU_RESET, RESET_VE_RESET);
	mb();
	udelay(100);
	CLR_SCU_REG_BITS(RSCU_RESET, RESET_VE_RESET);
	mb();
	while (GET_SCU_REG(RSCU_RESET) & RESET_VE_RESET)
	{
		//to do: error detection and recovery
	}

	SCU_END();

	return 0;
}

static int _scu_ve_pin_input_port(unsigned int is_port_b)
{
	unsigned int input_30bit = 0;
	unsigned int port = is_port_b ? 1 : 0;

	SCU_START();

	switch (port) {
	case 0: //Port A
		/* video input from port A */
		/* multiple function pins configuration */
		if (input_30bit) {
			MOD_SCU_REG(RSCU_MFPC5,
				MFPC5_DVIAPINS(MFPC5_DVIAPINS_PA30BITS),
				MFPC5_DVIAPINS_MASK | MFPC5_DVIBPINS_MASK);
			MOD_SCU_REG(RSCU_MFPC2,
				MFPC2_DVIA_PINS_30BIT_ALL,
				MFPC2_DVIA_PINS_MASK);
			MOD_SCU_REG(RSCU_MFPC3,
				MFPC3_DVIA_PINS_30BIT_ALL,
				MFPC3_DVIA_PINS_MASK);
		} else {
			MOD_SCU_REG(RSCU_MFPC5,
				MFPC5_DVIAPINS(MFPC5_DVIAPINS_PA24BITS),
				MFPC5_DVIAPINS_MASK | MFPC5_DVIBPINS_MASK);
			MOD_SCU_REG(RSCU_MFPC2,
				MFPC2_DVIA_PINS_24BIT_ALL,
				MFPC2_DVIA_PINS_MASK);
			MOD_SCU_REG(RSCU_MFPC3,
				MFPC3_DVIA_PINS_24BIT_ALL,
				MFPC3_DVIA_PINS_MASK);
		}
		/* input source configuration */
		MOD_SCU_REG(RSCU_MFPC6,
				MFPC6_DVI_SOURCE_SEL(MFPC6_DVI_SOURCE_PORT_A),
				MFPC6_DVI_SOURCE_MASK);

		/*
		** TODO:
		** A new SCU API for controlling input delay according to source timing.
		** Called by video Rx driver?
		**
		** Bruce150410.
		** A1 chip has optimized internal data/clock alignment. There is no need for this.
		*/
		if (ast_scu.board_info.soc_revision == 30) {
#if defined(CONFIG_AST1520_BOARD_EVA_V1)
			/*
			 * HJ suggest to set SCUR0D8 to 0x6100003D to work with ITE6802.
			 * IT6802 output DCLK is inverse and delay 1 ns.
			 */
			/*
			 * For 0x6100003D
			 * SCUD8[30]: 1 => control by [29:24]
			 * SCUD8[29]: 1 => inverse output
			 * SCUD8[28:24]: 1 (line stage)
			 */
			MOD_SCU_REG(RSCU_CLKSEL_SET2,
					INPUT_CLK_DELAY_CTL(1) | INPUT_CLK_DELAY_INVERT(1) | INPUT_CLK_DELAY_SEL(1),
					INPUT_CLK_DELAY_CTL_MASK | INPUT_CLK_DELAY_INVERT_MASK | INPUT_CLK_DELAY_SEL_MASK);
#else /* if defined(CONFIG_AST1520_BOARD_EVA_V2) */
			/* Bruce150127. A0 demo board. Use 0x6300003D is better. */
			MOD_SCU_REG(RSCU_CLKSEL_SET2,
					INPUT_CLK_DELAY_CTL(1) | INPUT_CLK_DELAY_INVERT(1) | INPUT_CLK_DELAY_SEL(3),
					INPUT_CLK_DELAY_CTL_MASK | INPUT_CLK_DELAY_INVERT_MASK | INPUT_CLK_DELAY_SEL_MASK);
#endif
		} else {
			/* Bruce150410. A1 chip has optimized internal data/clock alignment. There is no need for this. */
			MOD_SCU_REG(RSCU_CLKSEL_SET2,
					INPUT_CLK_DELAY_CTL(1) | INPUT_CLK_DELAY_INVERT(0) | INPUT_CLK_DELAY_SEL(0),
					INPUT_CLK_DELAY_CTL_MASK | INPUT_CLK_DELAY_INVERT_MASK | INPUT_CLK_DELAY_SEL_MASK);
		}
		break;
	case 1: // Port B
		/* Bruce is lazy. Assumes port B is always 9883 analog input. */
		/* video input from port B. For analog. */
		MOD_SCU_REG(RSCU_MFPC5,
			MFPC5_DVIAPINS(MFPC5_DVIAPINS_DISABLE) | MFPC5_DVIBPINS_EN,
			MFPC5_DVIAPINS_MASK | MFPC5_DVIBPINS_MASK);

		/* input source configuration */
		MOD_SCU_REG(RSCU_MFPC6,
				MFPC6_DVI_SOURCE_SEL(MFPC6_DVI_SOURCE_PORT_B),
				MFPC6_DVI_SOURCE_MASK);

		/* disable port A pin or we'll get something unexpected from port A if it is connected to video source */
		MOD_SCU_REG(RSCU_MFPC2, MFPC2_DVIA_PINS_OFF, MFPC2_DVIA_PINS_MASK);
		MOD_SCU_REG(RSCU_MFPC3, MFPC3_DVIA_PINS_OFF, MFPC3_DVIA_PINS_MASK);

		if (ast_scu.board_info.soc_revision == 30) {
			/* Chroma pattern gen #375: 1864x1050@60Hz 162.5MHz test. Use 0x4300003D. */
			MOD_SCU_REG(RSCU_CLKSEL_SET2,
					INPUT_CLK_DELAY_CTL(1) | INPUT_CLK_DELAY_INVERT(0) | INPUT_CLK_DELAY_SEL(3),
					INPUT_CLK_DELAY_CTL_MASK | INPUT_CLK_DELAY_INVERT_MASK | INPUT_CLK_DELAY_SEL_MASK);
		} else {
			/* Bruce150410. A1 chip has optimized internal data/clock alignment. There is no need for this. */
			MOD_SCU_REG(RSCU_CLKSEL_SET2,
					INPUT_CLK_DELAY_CTL(1) | INPUT_CLK_DELAY_INVERT(0) | INPUT_CLK_DELAY_SEL(3),
					INPUT_CLK_DELAY_CTL_MASK | INPUT_CLK_DELAY_INVERT_MASK | INPUT_CLK_DELAY_SEL_MASK);
		}
		break;
	default:
		BUG();
	}

	SCU_END();

	return 0;
}

static int _scu_ve_init_encode(void)
{
	SCU_START();

#if (PATCH_CODEV3 & PATCH_AST1520A0_BUG14071002_VE_SLOW)
	//Disable power save.
	CLR_SCU_REG_BITS(RSCU_CLKSEL, CLKSEL_VESLOW_EN);
#endif

	scu_system_clk_stop(CLKSTOP_ECLK_MASK, 1);
	scu_system_clk_stop(CLKSTOP_V1CLK_MASK, 1);

	/* Config VE Clock source and divider */
	if (ast_scu.board_info.soc_revision >= 31) {
		if ((GET_SCU_REG(0xD0) & 0xFF) == 0) {
			_scu_d1pll_set_ve(6600000);

			MOD_SCU_REG(RSCU_CLKSEL,
			            VESRC_D1PLL | VECLKDIV_DIV2,
			            CLKSEL_VESRC_MASK | CLKSEL_VECLKDIV_MASK);
		} else {
			MOD_SCU_REG(RSCU_CLKSEL,
			            VESRC_HPLL | VECLKDIV_DIV3,
			            CLKSEL_VESRC_MASK | CLKSEL_VECLKDIV_MASK);
		}
	}

	scu_system_clk_stop(CLKSTOP_ECLK_MASK, 0);
	scu_system_clk_stop(CLKSTOP_V1CLK_MASK, 0);

	SCU_END();

	return 0;
}

static int _scu_ve_init_decode(void)
{
	SCU_START();

	SET_SCU_REG(RSCU_ULCKIP, ULCKIP_VEDCDE);
	mb();

#if (PATCH_CODEV3 & PATCH_AST1520A0_BUG14071002_VE_SLOW)
	//Disable power save. Must disable for decode.
	CLR_SCU_REG_BITS(RSCU_CLKSEL, CLKSEL_VESLOW_EN);
#endif

	scu_system_clk_stop(CLKSTOP_ECLK_MASK, 1);
	scu_system_clk_stop(CLKSTOP_V1CLK_MASK, 1);

	/* Config VE Clock source and divider */
	if (ast_scu.board_info.soc_revision >= 31) {
		MOD_SCU_REG(RSCU_CLKSEL,
		            VESRC_HPLL | VECLKDIV_DIV3,
		            CLKSEL_VESRC_MASK | CLKSEL_VECLKDIV_MASK);
	}

	scu_system_clk_stop(CLKSTOP_ECLK_MASK, 0);
	scu_system_clk_stop(CLKSTOP_V1CLK_MASK, 0);

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

	SET_SCU_REG_BITS(RSCU_RESET, RESET_I2S_RESET);
	udelay(100);
	SET_SCU_REG_BITS(RSCU_CLKSTOP, CLKSTOP_I2S_MASK);

#if !defined(AST1520_D2_PLL_NO_CHANGE)
	//Stop D2-PLL
	SET_SCU_REG(RSCU_D2PLLEXT0,
	                        (D2PLLEXT0_PARH(0x2C)
	                       | D2PLLEXT0_RESET
	                       | D2PLLEXT0_OFF));
#endif

	//disable old style gen-lock
	CLR_SCU_REG_BITS(RSCU_PLLGENLCKCTRL, PLLGENLCKCTRL_AGENLCK_SRC_I2SCLKO);

#if !defined(AST1520_D2_PLL_NO_CHANGE)
	//Set D2-PLL param from SCU and enable D2-PLL.
	//D2-PLL not started yet due to SCU13C setting
	MOD_SCU_REG(RSCU_MISC1CTRL,
	            MISC1CTRL_D2PLLOFF(0),
	            MISC1CTRL_D2PLLOFF_MASK);
#endif

	SCU_END();

	return 0;
}

static int _scu_i2s_pin_hdmi_h(void)
{
	SCU_START();

	MOD_SCU_REG(RSCU_MISC2CTRL,
				MISC2CTRL_I2SWSMSTRP1EN(0)
			  | MISC2CTRL_I2SWSMSTRP0EN(0)
			  | MISC2CTRL_I2SCLKMSTRP1EN(0)
			  | MISC2CTRL_I2SCLKMSTRP0EN(0)
			  | MISC2CTRL_I2SRXP1EN(0)
			  | MISC2CTRL_I2STXP1EN(0),
				MISC2CTRL_I2SWSMSTRP1EN_MASK
			  | MISC2CTRL_I2SWSMSTRP0EN_MASK
			  | MISC2CTRL_I2SCLKMSTRP1EN_MASK
			  | MISC2CTRL_I2SCLKMSTRP0EN_MASK
			  | MISC2CTRL_I2SRXP1EN_MASK
			  | MISC2CTRL_I2STXP1EN_MASK
				);

	MOD_SCU_REG(RSCU_MFPC6,
				MFPC6_I2SMCLKMSTRP1EN(0)
			  | MFPC6_I2SMCLKMSTRP0EN(0)
			  | MFPC6_I2SP1PINEN(0)
			  | MFPC6_I2SP0PINEN(1),
				MFPC6_I2SMCLKMSTRP1EN_MASK
			  | MFPC6_I2SMCLKMSTRP0EN_MASK
			  | MFPC6_I2SP1PINEN_MASK
			  | MFPC6_I2SP0PINEN_MASK
				);

	SCU_END();

	return 0;
}

static int _scu_i2s_pin_hdmi_c(void)
{
	SCU_START();

	MOD_SCU_REG(RSCU_MISC2CTRL,
				MISC2CTRL_I2SWSMSTRP1EN(0)
			  | MISC2CTRL_I2SWSMSTRP0EN(1)
			  | MISC2CTRL_I2SCLKMSTRP1EN(0)
			  | MISC2CTRL_I2SCLKMSTRP0EN(1)
			  | MISC2CTRL_I2SRXP1EN(0)
			  | MISC2CTRL_I2STXP1EN(0),
				MISC2CTRL_I2SWSMSTRP1EN_MASK
			  | MISC2CTRL_I2SWSMSTRP0EN_MASK
			  | MISC2CTRL_I2SCLKMSTRP1EN_MASK
			  | MISC2CTRL_I2SCLKMSTRP0EN_MASK
			  | MISC2CTRL_I2SRXP1EN_MASK
			  | MISC2CTRL_I2STXP1EN_MASK
				);

	MOD_SCU_REG(RSCU_MFPC6,
				MFPC6_I2SMCLKMSTRP1EN(0)
			  | MFPC6_I2SMCLKMSTRP0EN(1)
			  | MFPC6_I2SP1PINEN(0)
			  | MFPC6_I2SP0PINEN(1),
				MFPC6_I2SMCLKMSTRP1EN_MASK
			  | MFPC6_I2SMCLKMSTRP0EN_MASK
			  | MFPC6_I2SP1PINEN_MASK
			  | MFPC6_I2SP0PINEN_MASK
				);

	SCU_END();

	return 0;
}

static int __scu_i2s_pin_codec(u32 host, u32 port, u32 upstream)
{
	unsigned int p0, p1, val;

	SCU_START();

	if (port) {
		p0 = 0;
		p1 = 1;
	} else {
		p0 = 1;
		p1 = 0;
	}

	val = MISC2CTRL_I2SWSMSTRP1EN(p1)
		  | MISC2CTRL_I2SWSMSTRP0EN(p0)
		  | MISC2CTRL_I2SCLKMSTRP1EN(p1)
		  | MISC2CTRL_I2SCLKMSTRP0EN(p0)
		  | MISC2CTRL_I2SRXP1EN(p1)
		  | MISC2CTRL_I2STXP1EN(p1);


	/*
	 * I2S audio controller bug? (improper behavior)
	 *
	 * SCU4C[10]:I2S Rx in port 1; SCU4C[ 9]:I2S Tx in port 1
	 *	SCU4C[10] is set => I2S need RxWS
	 *	SCU4C[ 9] is set => I2S need TxWS
	 *
	 * When SCU4C[10] and SCU4C[ 9] are both set, I2S makes reference to TxWs and RxWs at the same time
	 *
	 * In original implementation, we set both SCU4C[10] and SCU4C[ 9].
	 * There is not Tx at host side if upstream is disabled (multicast mode will disable upstream),
	 * so no TxWs related configurations get set.
	 * In such case, I2S stop working without TxWS.
	 *
	 * We clear SCU4C[ 9] when upstream is disabled to fixed no sound issue in multicast environment.
	 */

	/* Jazoe fixed this issue on A1 chip, we can remove following code if necessary */
	if (0 == upstream) {
		if (host)
			val &= ~MISC2CTRL_I2STXP1EN(p1);
		else
			val &= ~MISC2CTRL_I2SRXP1EN(p1);
	}

	MOD_SCU_REG(RSCU_MISC2CTRL,
			val,
			MISC2CTRL_I2SWSMSTRP1EN_MASK
			  | MISC2CTRL_I2SWSMSTRP0EN_MASK
			  | MISC2CTRL_I2SCLKMSTRP1EN_MASK
			  | MISC2CTRL_I2SCLKMSTRP0EN_MASK
			  | MISC2CTRL_I2SRXP1EN_MASK
			  | MISC2CTRL_I2STXP1EN_MASK);

	MOD_SCU_REG(RSCU_MFPC6,
				MFPC6_I2SMCLKMSTRP1EN(p1)
			  | MFPC6_I2SMCLKMSTRP0EN(p0)
			  | MFPC6_I2SP1PINEN(p1)
			  | MFPC6_I2SP0PINEN(p0),
				MFPC6_I2SMCLKMSTRP1EN_MASK
			  | MFPC6_I2SMCLKMSTRP0EN_MASK
			  | MFPC6_I2SP1PINEN_MASK
			  | MFPC6_I2SP0PINEN_MASK
				);
#if 0 //This is a workaround for eva board#4-H. Should remove once confirmed.
	/*
	 * IO pin with codec: GPIOE6, GPIOE7
	 */
	/* disable GPIOE pass-through mode, write SCU70 to set, write SCU7C to clear */
	if (GET_SCU_REG(RSCU_HWSTRAP1) & HWSTRAP1_GPIOE_PASSTHROUGH)
		SET_SCU_REG(0x7C, HWSTRAP1_GPIOE_PASSTHROUGH);
	/* disable GPIOE6 pass-through to GPIOE7 */
	MOD_SCU_REG(RSCU_MFPC4, 0, MFPC4_GPIOE_6_PASSTHROUGH_TO_7);
#endif

	SCU_END();

	return 0;
}


static int _scu_i2s_pin_codec_h(u32 upstream)
{
	return __scu_i2s_pin_codec(1, 1, upstream);
}

static int _scu_i2s_pin_codec_c(u32 upstream)
{
	return __scu_i2s_pin_codec(0, 1, upstream);
}

#if (PATCH_CODEV3 & PATCH_AST1520A0_I2S_DUAL_OUTPUT)
static int _scu_i2s_pin_dual_output_c(void)
{
	SCU_START();

	MOD_SCU_REG(RSCU_MISC2CTRL,
			MISC2CTRL_I2SWSMSTRP1EN(1)
			  | MISC2CTRL_I2SWSMSTRP0EN(1)
			  | MISC2CTRL_I2SCLKMSTRP1EN(1)
			  | MISC2CTRL_I2SCLKMSTRP0EN(1)
			  | MISC2CTRL_I2SRXP1EN(1)
			  | MISC2CTRL_I2STXP1EN(1),
			MISC2CTRL_I2SWSMSTRP1EN_MASK
			  | MISC2CTRL_I2SWSMSTRP0EN_MASK
			  | MISC2CTRL_I2SCLKMSTRP1EN_MASK
			  | MISC2CTRL_I2SCLKMSTRP0EN_MASK
			  | MISC2CTRL_I2SRXP1EN_MASK
			  | MISC2CTRL_I2STXP1EN_MASK
				);

	MOD_SCU_REG(RSCU_MFPC6,
			MFPC6_I2SMCLKMSTRP1EN(1)
			  | MFPC6_I2SMCLKMSTRP0EN(1)
			  | MFPC6_I2SP1PINEN(1)
			  | MFPC6_I2SP0PINEN(1),
			MFPC6_I2SMCLKMSTRP1EN_MASK
			  | MFPC6_I2SMCLKMSTRP0EN_MASK
			  | MFPC6_I2SP1PINEN_MASK
			  | MFPC6_I2SP0PINEN_MASK
				);

	/*
	 * IO pin with codec: GPIOE6, GPIOE7
	 */
	/* disable GPIOE pass-through mode, write SCU70 to set, write SCU7C to clear */
	if (GET_SCU_REG(RSCU_HWSTRAP1) & HWSTRAP1_GPIOE_PASSTHROUGH)
		SET_SCU_REG(0x7C, HWSTRAP1_GPIOE_PASSTHROUGH);
	/* disable GPIOE6 pass-through to GPIOE7 */
	MOD_SCU_REG(RSCU_MFPC4, 0, MFPC4_GPIOE_6_PASSTHROUGH_TO_7);

	SCU_END();

	return 0;
}
#endif

/*
** A0 chip's SIP/SIC value is swapped. d1pll_table[] is based on A0 chip.
** So we have to swap it in A1.
*/
static inline u32 SWAP_SIP_SIC(u32 scu028)
{
	u32 sip, sic;

	sip = DPLL_SIP(DPLL_SIC_GET(scu028));
	sic = DPLL_SIC(DPLL_SIP_GET(scu028));
	scu028 &= ~(DPLL_SIC_MASK | DPLL_SIP_MASK);
	scu028 |= (sip | sic);

	return scu028;
}

static int __scu_d2pll_set(u32 HZx100)
{
	u32 r, scu01c;
	struct d2pll_table_t *t = d2pll_table;

#if defined(AST1520_D2_PLL_NO_CHANGE)
	if (d2_pll_init_done == 1)
		goto d2_pll_done;
#endif

	for (r = 0; r < d2pll_table_size; r++) {
		if (t->clk== HZx100)
			break;
		t++;
	}

	if (r == d2pll_table_size) {
		uerr("Can't find D2-PLL clock setting?! (%d Hz)\n", HZx100*100);
		BUG();
	}

	r = t->scu13c | D2PLLEXT0_RESET | D2PLLEXT0_OFF;
	SET_SCU_REG(RSCU_D2PLLEXT0, r);

	if (ast_scu.board_info.soc_revision >= 31)
		scu01c = SWAP_SIP_SIC(t->scu01c);
	else
		scu01c = t->scu01c;

	mb();
	SET_SCU_REG(RSCU_D2PLL, scu01c);
	SET_SCU_REG(RSCU_D2PLLEXT1, t->scu140);
	SET_SCU_REG(RSCU_D2PLLEXT2, t->scu144);
	mb();
	CLR_SCU_REG_BITS(RSCU_D2PLLEXT0, (D2PLLEXT0_RESET|D2PLLEXT0_OFF));

	//Wait for PLL stable.
	mdelay(1);

	MOD_SCU_REG(RSCU_MISC1CTRL, MISC1CTRL_D2PLLOFF(0), MISC1CTRL_D2PLLOFF_MASK);

	uinfo("Set D2-PLL to %d Hz.\n", HZx100*100);

#if defined(AST1520_D2_PLL_NO_CHANGE)
	d2_pll_init_done = 1;
d2_pll_done:
#endif
	return 0;
}

#define SCU_AUDIO_GEN_LOCK

#if defined(CONFIG_ARCH_AST1500_CLIENT) && defined(SCU_AUDIO_GEN_LOCK) || defined(MAC_RGMII_CLOCK_SELECT_D2_PLL)
static int _scu_audio_gen_lock_pll_set(u32 HZx100)
{
	struct audio_gen_lock_pll_table_t *t = audio_gen_lock_pll_table;
	u32 i;

	SCU_START();

	CLR_SCU_REG_BITS(RSCU_PLLGENLCKCTRL, PLLGENLCKCTRL_AGENLCK_EN);

	__scu_d2pll_set(D2_PLL_DEFAULT);

	for (i = 0; i < audio_gen_lock_pll_table_size; i++) {
		if (t->clk == HZx100)
			break;
		t++;
	}

	if (i == audio_gen_lock_pll_table_size) {
		uerr("Can't find audio gen-lock clock setting?! (%d Hz)\n", HZx100*100);
		BUG();
	}

	mb();

	MOD_SCU_REG(RSCU_PLLGENLCKCTRL,
		PLLGENLCKCTRL_AGENLCK_SRC_I2SCLKO | PLLGENLCKCTRL_AGENLCK_H_SRC_D2PLL,
		PLLGENLCKCTRL_AGENLCK_SRC_MASK | PLLGENLCKCTRL_AGENLCK_H_SRC_MASK);

	SET_SCU_REG(RSCU_AGENLCK_A, 0);
	SET_SCU_REG(RSCU_AGENLCK_R, t->r_scu1cc);
	SET_SCU_REG(RSCU_AGENLCK_N, t->n_scu1d0);
	SET_SCU_REG(RSCU_AGENLCK_Q, t->q_scu1d4);

	SET_SCU_REG_BITS(RSCU_PLLGENLCKCTRL, PLLGENLCKCTRL_AGENLCK_EN);
	mb();

	//Wait for PLL stable.
	mdelay(1);
	uinfo("Set audio gen-lock clock to %d Hz.\n", HZx100*100);

	SCU_END();
	return 0;
}

#define AUDIO_GEN_LOCK_NEGATIVE (0x1 << 23)

static u32 audio_gen_lock_cfg_positive = 0, audio_gen_lock_cfg_negative = 0, audio_gen_lock_cfg_zero = 0;
static int audio_gen_lock_cfg_ppm = 0;
static int audio_gen_lock_cfg_max = 100;
/*
 * _scu_audio_gen_lock_adjust
 * @level: audio clock adjust level,
 *	negative value : clock - MAX ppm
 *	positive value : clock + MAX ppm
 *	zero: no adjustment
 */
static int _scu_audio_gen_lock_adjust(int level)
{
	u32 val = 0;

	switch (level) {
	case 1: /* H */
		val = audio_gen_lock_cfg_positive;
		break;
	case -1: /* L */
		val = audio_gen_lock_cfg_negative;
		break;
	default:
		val = audio_gen_lock_cfg_zero;
		break;
	}

	SCU_START();

	SET_SCU_REG(RSCU_AGENLCK_A, val);

	SCU_END();

	return 0;
}

/*
 * According to H.J.
 * original formula in AST1520 SCU register SCU1CC is incorrect,
 * (old: Adjusted I2SCLK = I2SCLK * (1 +/- (A / (0x800000)))
 *
 * new formula for audio genlock:
 *	ppm = (A * reference clock * 10^6) / (2^24 *ARCLK)
 *
 * where reference clock is 24MHz, ARCLK is 800MHz (from D2-PLL)
 * ppm = (A * 24 * 10^6) / (2^24 * 800)
 * A = ppm * (2^24 * 800) / (24 * 10^6)  ~= ppm * 559.2405
 */
#define AUDIO_GEN_LOCK_PPM_PARAM 559
#define AUDIO_GEN_LOCK_PPM_PARAM_PRECISE 55924
#define AUDIO_GEN_LOCK_PPM_PARAM_PRECISE_DIVISOR 100
static int _scu_audio_gen_lock_init(int ppm)
{
	int val;

	/* +PPM, H */
	val = ppm + audio_gen_lock_cfg_max;
	audio_gen_lock_cfg_positive = abs(val) * AUDIO_GEN_LOCK_PPM_PARAM_PRECISE / AUDIO_GEN_LOCK_PPM_PARAM_PRECISE_DIVISOR;
	if (val < 0)
		audio_gen_lock_cfg_positive |= AUDIO_GEN_LOCK_NEGATIVE;

	/* -PPM, L */
	val = ppm - audio_gen_lock_cfg_max;
	audio_gen_lock_cfg_negative = abs(val) * AUDIO_GEN_LOCK_PPM_PARAM_PRECISE / AUDIO_GEN_LOCK_PPM_PARAM_PRECISE_DIVISOR;
	if (val <= -1)
		audio_gen_lock_cfg_negative |= AUDIO_GEN_LOCK_NEGATIVE;

	/* 0PPM, N */
	val = ppm;
	audio_gen_lock_cfg_zero = abs(val) * AUDIO_GEN_LOCK_PPM_PARAM_PRECISE / AUDIO_GEN_LOCK_PPM_PARAM_PRECISE_DIVISOR;
	if (val <= -1)
		audio_gen_lock_cfg_zero |= AUDIO_GEN_LOCK_NEGATIVE;

	SCU_START();

	SET_SCU_REG(RSCU_AGENLCK_A, audio_gen_lock_cfg_zero);

	SCU_END();

	audio_gen_lock_cfg_ppm = ppm;

	return 0;
}

static int _scu_audio_gen_lock_set(int ppm)
{
	audio_gen_lock_cfg_zero = abs(ppm) * AUDIO_GEN_LOCK_PPM_PARAM_PRECISE / AUDIO_GEN_LOCK_PPM_PARAM_PRECISE_DIVISOR;

	if (ppm < 0)
		audio_gen_lock_cfg_zero |= AUDIO_GEN_LOCK_NEGATIVE;

	SCU_START();

	SET_SCU_REG(RSCU_AGENLCK_A, audio_gen_lock_cfg_zero);

	SCU_END();

	audio_gen_lock_cfg_ppm = ppm;

	return 0;
}

static int _scu_audio_gen_lock_limit_cfg(u32 cfg)
{
	/*
	 * remove sanity check,
	 * have checked the input parameter in clock_limit_cfg() of I2S driver already
	 */

	audio_gen_lock_cfg_max = cfg;
}
#else
static int _scu_d2pll_set(u32 HZx100)
{
	int r;

	SCU_START();

	r = __scu_d2pll_set(HZx100);

	SCU_END();
	return r;
}
#endif /* #if defined(CONFIG_ARCH_AST1500_CLIENT) && defined(SCU_AUDIO_GEN_LOCK) */

static int (*i2s_clock_init)(u32 sample_rate_idx);

static int _scu_i2s_clock_init(unsigned int sample_rate_idx)
{
	u32 clock_in_100hz = 0;

#if defined(MAC_RGMII_CLOCK_SELECT_D2_PLL)
	i2s_clock_init = _scu_audio_gen_lock_pll_set;
#else /* #if defined(MAC_RGMII_CLOCK_SELECT_D2_PLL) */
#if defined(CONFIG_ARCH_AST1500_CLIENT) && defined(SCU_AUDIO_GEN_LOCK)
	i2s_clock_init = _scu_audio_gen_lock_pll_set;
#else
	i2s_clock_init = _scu_d2pll_set;
#endif
#endif /* #if defined(MAC_RGMII_CLOCK_SELECT_D2_PLL) */

	switch (sample_rate_idx) {
	case I2S_SAMPLE_RATE_32K:
		clock_in_100hz = 81920;
		break;
	case I2S_SAMPLE_RATE_44d1K:
		clock_in_100hz = 112896;
		break;
	case I2S_SAMPLE_RATE_48K:
		clock_in_100hz = 122880;
		break;
	case I2S_SAMPLE_RATE_88d2K:
		clock_in_100hz = 225792;
		break;
	case I2S_SAMPLE_RATE_96K:
		clock_in_100hz = 245760;
		break;
	case I2S_SAMPLE_RATE_176d4K:
		clock_in_100hz = 451584;
		break;
	case I2S_SAMPLE_RATE_192K:
		clock_in_100hz = 491520;
		break;
	default:
		break;
	}

	if (0 == clock_in_100hz)
		BUG();

	i2s_clock_init(clock_in_100hz);

	return 0;
}

static int _scu_i2s_stop(void)
{
	scu_system_clk_stop(CLKSTOP_I2S_MASK, 1);
	return 0;
}

static int _scu_i2s_start(void)
{
	scu_system_clk_stop(CLKSTOP_I2S_MASK, 0);
	return 0;
}

static int _scu_usb20p1_init(int as_HC)
{
	SCU_START();

	if (as_HC) {
		//Select as USB 2.0 HC. Set SCU90[29] to 1 to Enable EHCI controller
		MOD_SCU_REG(RSCU_MFPC5, MFPC5_USB20(MFPC5_USB20_HC), MFPC5_USB20_MASK);
	} else {
		//Select as VHUB. Set SCU90[29] to 0
		MOD_SCU_REG(RSCU_MFPC5, MFPC5_USB20(MFPC5_USB20_VHUB), MFPC5_USB20_MASK);
	}
	wmb();

	//reset USB20. Takes effect on the selection of SCU90[29].
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

static int _scu_usb20p2_init(void)
{
	SCU_START();

	//Set SCU94 bit[14:13] = 10, select USB2.0 port 2 mode as host
	MOD_SCU_REG(RSCU_MFPC6, MFPC6_USB20P2(MFPC6_USB20P2_USB20HCP2), MFPC6_USB20P2_MASK);
	wmb();

	//reset USB20 port 2. Takes effect on the selection of SCU94[14:13].
	SET_SCU_REG_BITS(RSCU_RESET, RESET_USB20P2_RESET);
	wmb();

	//enable USB20 port 2 PHY clock
	CLR_SCU_REG_BITS(RSCU_CLKSTOP, CLKSTOP_USB20P2);
	mdelay(10);

	CLR_SCU_REG_BITS(RSCU_RESET, RESET_USB20P2_RESET);
	wmb();
	udelay(500);

	SCU_END();

	BUG_ON(GET_SCU_REG(RSCU_RESET) & RESET_USB20_RESET);
	return 0;
}

static int _scu_usb20_init(int as_HC)
{
	//Init as 2 ports HC, otherwise as VHUB
	_scu_usb20p1_init(as_HC);
	if (as_HC)
		_scu_usb20p2_init();

	return 0;
}

static int _scu_usb11hc_init(int num_ports)
{
	//USB 2.0 Port 2 must be init first. USB 1.1 HC uses the same clock.
	//Assuming _scu_usb20p2_init() is always called before _scu_usb11hc_init().
	//_scu_usb20p2_init();

	SCU_START();

	switch (num_ports) {
	case 1:
		//pass through
	case 2:
		break;
	case 3:
		SET_SCU_REG_BITS(RSCU_MFPC5, MFPC5_USB11HCP34_EN);
		SET_SCU_REG_BITS(RSCU_MFPC3, MFPC3_USB11HCP3_EN);
		mb();
		break;
	case 4:
		SET_SCU_REG_BITS(RSCU_MFPC5, MFPC5_USB11HCP34_EN);
		SET_SCU_REG_BITS(RSCU_MFPC3, MFPC3_USB11HCP3_EN);
		SET_SCU_REG_BITS(RSCU_MFPC3, MFPC3_USB11HCP4_EN);
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
	udelay(10);

	CLR_SCU_REG_BITS(RSCU_RESET, RESET_USB11HC_RESET);
	mb();
	udelay(500);

	SCU_END();
	return 0;
}

static int _scu_uart_init(u32 base)
{
	switch (base) {
	case ASPEED_UART_SOIP_BASE: //0x1E78F000 UART#4
		//init UART#4 for SoIP port
		//Pin mux: SCU80[31:24] set to 1
		MOD_SCU_REG(RSCU_MFPC1,
		            MFPC1_UART4PINS(MFPC1_UART4PINS_ALL),
		            MFPC1_UART4PINS_MASK);

		//Clock: SCU0C[26] set to 0 (default is 0)
		scu_system_clk_stop(CLKSTOP_UART_4_MASK, 0);
		break;
	case ASPEED_UART3_BASE: //0x1E78E000	/* UART#3 */
		/* WARNING. UART3 to IO6 is for client board ONLY. Host board's GPIOH is occupied by VIPB. */
		/* init UART#3 and route pin to IO6 (GPIOH[7:0]) */
		{
			u32 r;
			/* route UART3 to IO6 */
			r = __raw_readl(IO_ADDRESS(0x1E789098));
			r &= ~(0xF << 8);
			r |= (0x2 << 8);
			__raw_writel(r, IO_ADDRESS(0x1E789098));
			/* route IO6 to UART3 */
			r = __raw_readl(IO_ADDRESS(0x1E78909C));
			r &= ~(0x7 << 22);
			r |= (0x7 << 22);
			__raw_writel(r, IO_ADDRESS(0x1E78909C));
			/* enable IO6 pins */
			SET_SCU_REG_BITS(RSCU_MFPC5, MFPC5_IO6PINS_EN_MASK);
		}
		scu_system_clk_stop(CLKSTOP_UART_3_MASK, 0);
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

static int _scu_device_rst(u32 device)
{
	u32 id = 0, active = 0, delay = 0;

	if (SCU_RST_MAX <= device)
		return 0;

	SCU_START();


	switch (device) {
	case SCU_RST_HDMI_TX:
		id = GPIO_CAT6613_RST;
		active = 0;
		delay = 5;
		break;
	case SCU_RST_HDMI_RX:
		id = GPIO_CAT6023_RST;
		active = 0;
		delay = 50;
		break;
#if defined(CONFIG_AST1500_SII9678)
	case SCU_RST_HDCP22_TX:
		id = GPIO_HDCP22_TX_RST;
		active = 0;
		delay = 5;
		break;
#endif
#if defined(CONFIG_AST1500_SII9679)
	case SCU_RST_HDCP22_RX:
		id = GPIO_HDCP22_RX_RST;
		active = 0;
		delay = 5;
		break;
#endif
	default:
		BUG();
	}

	gpio_direction_output(id, active);
	mdelay(delay);
	gpio_direction_output(id, 1 - active);

	SCU_END();

	return 0;
}

static int _scu_video_gen_lock_stop(void)
{
	SCU_START();

	CLR_SCU_REG_BITS(RSCU_PLLGENLCKCTRL, PLLGENLCKCTRL_VGENLCK_EN);
	SET_SCU_REG(RSCU_VGENLCKCTRL, VGENLCKCTRL_F_A1(0));

	SCU_END();

	return 0;
}

static int _scu_video_gen_lock_init(void)
{
	unsigned int ratio;

	SCU_START();

	SET_SCU_REG(RSCU_VGENLCKCTRL, VGENLCKCTRL_F_A1(0));

	//PLLGENLCKCTRL_SRC_RATIO(v)
	//v = (CPU CLK / PLL SRC) - 27
	//  = (1008 / 24) - 27
	//  = 42 - 27 = 15
	ratio = (ast_scu.HPLL_Hz / ast_scu.CLKIN_Hz) - 27;
	BUG_ON(ratio > 0xF);

	MOD_SCU_REG(RSCU_PLLGENLCKCTRL,
	            PLLGENLCKCTRL_D1PLLLCK_EN
	           |PLLGENLCKCTRL_SRC_RATIO(ratio),
	            PLLGENLCKCTRL_VGENLCK_EN
	           |PLLGENLCKCTRL_D1PLLLCK_EN
	           |PLLGENLCKCTRL_VGENLCK_FROM_MPLL
	           |PLLGENLCKCTRL_SRC_RATIO_MASK);

	SET_SCU_REG_BITS(RSCU_PLLGENLCKCTRL, PLLGENLCKCTRL_VGENLCK_EN);

	SCU_END();

	return 0;
}

static int _scu_video_gen_lock_adjust(int ppm)
{
	int hclk = ast_scu.HPLL_Hz / 1000000;
	int f;
	unsigned int coef_h = 0x0165, coef_l = 0xE9F8; // 0x0165_E9F8
	int m;
	unsigned int m_h, m_l;
	int ppm_max;

	/*
	** Formula: PPM = (24000000 * f) / (2^25 * HCLK_MHz)
	** Where 24000000 is system 24MHz clock. And HCLK_MHz is 1008MHz.
	**
	** f = ppm * hclk_MHz * coef
	** where coef == 2^25/24000000 == 1.39810133333333 == 0x1.65E9F8 == 0x165E9F8 >> 24
	** f = ppm * hclk * 0x165E9F8 >> 24
	** m = ppm * hclk
	** f = m * 0x0165_E9F8 >> 24
	** f = m_h_m_l * 0x0165_E9F8 >> 24
	*/

	/*
	 * a large ppm may cause overflow, following is to check and fix the ppm
	 * ppm_max = (0x7FFFFFFF / ((coef_h << 16) + coef_l) << 16) / hclk;
	 *
	 * When input clock source is 24MHz:
	 * ppm_max = ((0x7FFFFFFF / 0x0165E9F8) << 16) / HCLK
	 * 	= 0x5B0000 / HCLK
	 * When input clock source is 25MHz:
	 * ppm_max = ((0x7FFFFFFF / 0x015798EE) << 16) / HCLK
	 * 	= 0x5F0000 / HCLK
	 */

	if (ast_scu.CLKIN_Hz == 25000000) {
		//2^25/25000000 == 1.34217728 == 0x1.5798EE == 0x15798EE >> 24
		coef_h = 0x0157;
		coef_l = 0x98EE;
		ppm_max = 0x5F0000 / hclk;
	} else {
		ppm_max = 0x5B0000 / hclk;
	}

	if (ppm > 0) {
		if (ppm > ppm_max) {
			uinfo(" %i ppm exceed adjustment range (%d ~ %d), fix it to %d\n",
				ppm, ppm_max, -ppm_max, ppm_max);
			ppm = ppm_max;
		}
	} else {
		if (ppm < -ppm_max) {
			uinfo(" %i ppm exceed adjustment range (%d ~ %d), fix it to %d\n",
				ppm, ppm_max, -ppm_max, -ppm_max);
			ppm = -ppm_max;
		}
	}

	m = ppm * hclk;
	m_h = (unsigned int)(m >> 16);
	m_l = (unsigned int)(m & 0xFFFF);

	/*
	** To avoid 32 bits overflow, we take only upper 32bits into consideration.
	** The lower 16bits is not necessary, since we will >> 24 after m * coef.
	** So, we break >> 24 bits == (16+8) bits. Let's discard lower 16 bits first. (To avoid overflow)
	*/
	f = ((coef_l * m_l) >> 16)
	  + (coef_l * m_h)
	  + (coef_h * m_l)
	  + ((coef_h * m_h) << 16);
	/*
	** Then, discard lower 8 bits and take sign into consideration.
	** 'right shift >>' gets different result on signed/unsigned data type.
	** So, we should convert the result to signed type before doing right shift.
	** f is declared as signed int.
	*/
	f >>= 8;

	/* disable following message becasue gen-lock call this function frequently */
#if 0
	uinfo("v_genlock adjust %ippm ==> SCU1C4=0x%08X\n", ppm, f);
#endif
	SCU_START();

	if (ast_scu.board_info.soc_revision >= 31)
		SET_SCU_REG(RSCU_VGENLCKCTRL, VGENLCKCTRL_F_A1(f));
	else
		SET_SCU_REG(RSCU_VGENLCKCTRL, VGENLCKCTRL_F_A0(f));

	SCU_END();

	return 0;
}

#ifdef FPGA_TEST_ONLY

#if (CONFIG_AST1500_SOC_DRAM_MAPPING_TYPE == 1) //For 2400 FPGA code base
static int _scu_d1pll_set(int HZx100)
{
	SCU_START();

	MOD_SCU_REG(RSCU_MISC1CTRL, (0x3UL << 20), (0x3UL << 20));
	mb();

	/* FPGA supports 25.175, 28.322, 40, 65, 25.175/2, 28.322/2, 40/2, 65/2 MHz. */
	if (HZx100 < 140000) {
		//13.5 480i
		//28.322/2
		MOD_SCU_REG(RSCU_D2PLL, (0x1UL << 20)|(0x1UL << 13), (0x3UL << 20)|(0x1UL << 13));
	}
	else if (HZx100 < 270000) {
		//25.175. 640x480@60Hz
		MOD_SCU_REG(RSCU_D2PLL, (0x0UL << 20)|(0x0UL << 13), (0x3UL << 20)|(0x1UL << 13));
	}
	else if (HZx100 < 350000) {
		//28.322
		MOD_SCU_REG(RSCU_D2PLL, (0x1UL << 20)|(0x0UL << 13), (0x3UL << 20)|(0x1UL << 13));
	}
	else if (HZx100 < 550000) {
		//40. 800x600@60Hz
		MOD_SCU_REG(RSCU_D2PLL, (0x2UL << 20)|(0x0UL << 13), (0x3UL << 20)|(0x1UL << 13));
	}
	else {
		//65. 1024x768@60Hz
		//74.25. 1080i 60Hz, 1080p 24Hz
		MOD_SCU_REG(RSCU_D2PLL, (0x3UL << 20)|(0x0UL << 13), (0x3UL << 20)|(0x1UL << 13));
	}
	msleep(100);

	SCU_END();
	return 0;
}
#endif
#if (CONFIG_AST1500_SOC_DRAM_MAPPING_TYPE == 2) //For 2500 FPGA code base
static void fpga_set_pll(
	int isD1PLL,
	u32 p0,
	u32 p1,
	u32 p2,
	u32 p3,
	u32 p4,
	u32 p5,
	u32 p6)
{
	volatile u32 done = 0;

	SET_SCU_REG(0x50, p0);
	SET_SCU_REG(0x54, p1);
	SET_SCU_REG(0x58, p2);
	SET_SCU_REG(0x5c, p3);
	SET_SCU_REG(0x60, p4);
	SET_SCU_REG(0x64, p5);
	SET_SCU_REG(0x68, p6);
	mb();
	if(isD1PLL)
		SET_SCU_REG(0x6c, 0xAE010000);
	else
		SET_SCU_REG(0x6c, 0xAE020000);

	//wait for stable.
	do {
		done = GET_SCU_REG(RSCU_MISC2CTRL);

		if(isD1PLL)
			done &= (1 << 25);
		else
			done &= (1 << 26);

	} while(!done);

	if(isD1PLL)
		SET_SCU_REG(0x6c, 0xEA010000);
	else
		SET_SCU_REG(0x6c, 0xEA020000);

}

static int _scu_d1pll_set(int HZx100)
{
	SCU_START();

	uinfo("Set D1PLL to %d 00Hz\n", HZx100);
	//FPGA supports 25.175, 28.322, 40, 65, 15.75, 13.5, 74.25MHz
	if (HZx100 < 140000) {
		//13.5 480i
		fpga_set_pll(1,
			0x00020000,
			0x00200008,
			0x02000080,
			0x03018800,
			0x1D0D8000,
			0x00180004,
			0x00000000);
	}
	else if (HZx100 < 200000) {
		//15.75
		fpga_set_pll(1,
			0x00020000, //p0
			0x00200008, //p1
			0x02000080, //p2
			0x02010800, //p3
			0x170A8000, //p4
			0x00180004, //p5
			0x00000000);
	}
	else if (HZx100 < 260000) {
		//25.175. 640x480@60Hz
		fpga_set_pll(1,
			0x00020000, //p0
			0x00200008, //p1
			0x02000080, //p2
			0x41D0D800, //p3
			0x572A0140, //p4
			0x00100004, //p5
			0x00000000);
	}
	else if (HZx100 < 350000) {
		//28.322
		fpga_set_pll(1,
			0x00020000, //p0
			0x00200008, //p1
			0x02000080, //p2
			0x41B0C800, //p3
			0x3D1D0080, //p4
			0x00130004, //p5
			0x00000000);
	}
	else if (HZx100 < 550000) {
		//40. 800x600@60Hz
		fpga_set_pll(1,
			0x00020000, //p0
			0x00200008, //p1
			0x02000080, //p2
			0x01209800, //p3
			0x1E0F8000, //p4
			0x00180004, //p5
			0x00000000);
	}
	else if (HZx100 < 700000) {
		//65. 1024x768@60Hz
		fpga_set_pll(1,
			0x00020000, //p0
			0x00200008, //p1
			0x02000080, //p2
			0x40C06800, //p3
			0x43200080, //p4
			0x00100004, //p5
			0x00000000);
	}
	else {
		//74.25. 1080i 60Hz, 1080p 24Hz
		fpga_set_pll(1,
			0x00020000, //p0
			0x00200008, //p1
			0x12000080, //p2
			0x80804020, //p3
			0x65310100, //p4
			0x00080004, //p5
			0x00000000);
	}

	SCU_END();
	return 0;
}
#endif

#else //#ifdef FPGA_TEST_ONLY
#if defined(AST1520_D1PLL_FRACTIONAL)
#define PLL_TB_VE   0
#define PLL_TB_CRT  1
static int __scu_d1pll_set(int HZx100, unsigned int table)
{
	u32 r, scu028;
	struct d1pll_table_t *t;
	unsigned int ts;

	SCU_START();

	if (table == PLL_TB_CRT) {
		t = d1pll_table;
		ts = d1pll_table_size;
	} else {
		t = ve_d1pll_table;
		ts = ve_d1pll_table_size;
	}

	for (r = 0; r < ts; r++) {
		if (t->clk == HZx100)
			break;
		t++;
	}

	if (r == ts) {
		uerr("Can't find pixel clock setting?! (%d Hz)\n", HZx100*100);
		BUG();
	}

	SET_SCU_REG(RSCU_DPLLEXT0,
	                        (t->scu130
	                       | DPLLEXT0_RESET
	                       | DPLLEXT0_OFF));

	mb();

	if (table == PLL_TB_CRT)
		__scu_crt_dvo_timing_ctrl(HZx100);

	SET_SCU_REG_BITS(RSCU_MISC1CTRL, MISC1CTRL_DPLLPARAM_FRMSCU);

	if (ast_scu.board_info.soc_revision >= 31)
		scu028 = SWAP_SIP_SIC(t->scu028);
	else
		scu028 = t->scu028;

	SET_SCU_REG(RSCU_DPLL, scu028);
	SET_SCU_REG(RSCU_DPLLEXT1, t->scu134);
	SET_SCU_REG(RSCU_DPLLEXT2, t->scu138);
	mb();

	CLR_SCU_REG_BITS(RSCU_DPLLEXT0, (DPLLEXT0_RESET|DPLLEXT0_OFF));

	//Wait for PLL stable.
	mdelay(1);
	uinfo("Set D1-PLL for pixel clock %d Hz. SCU28:0x%08X SCU130:0x%08X SCU134:0x%08X SCU138:0x%08X\n",
	       HZx100*100, scu028, t->scu130, t->scu134, t->scu138);

	SCU_END();
	return 0;
}

static inline int _scu_d1pll_set_ve(int HZx100)
{
	return __scu_d1pll_set(HZx100, PLL_TB_VE);
}

static inline int _scu_d1pll_set(int HZx100)
{
	return __scu_d1pll_set(HZx100, PLL_TB_CRT);
}
#else /* #if defined(AST1520_D1PLL_FRACTIONAL) */
static int _scu_d1pll_set(int HZx100)
{
	u32 r;
	struct d1pll_table_t *t = d1pll_table;

	SCU_START();

	for (r = 0; r < d1pll_table_size; r++) {
		if (t->pixel_clk == HZx100)
			break;
		t++;
	}

	if (r == d1pll_table_size) {
		uerr("Can't find pixel clock setting?! (%d Hz)\n", HZx100*100);
		BUG();
	}

	SET_SCU_REG(RSCU_DPLLEXT0,
		(DPLLEXT0_PARH(0x2C) | DPLLEXT0_RESET | DPLLEXT0_OFF));

	mb();

	__scu_crt_dvo_timing_ctrl(HZx100);

	SET_SCU_REG_BITS(RSCU_MISC1CTRL, MISC1CTRL_DPLLPARAM_FRMSCU);

	/*
	** A0 chip's SIP/SIC value is swapped. d1pll_table[] is based on A0 chip.
	** So we have to swap it in A1.
	*/
	if (ast_scu.board_info.soc_revision >= 31) {
		r = (DPLL_SIP(t->scu28_26_22)
			| DPLL_SIC(t->scu28_31_27)
			| DPLL_OD(t->scu28_21_19)
			| DPLL_P(t->scu28_18_13)
			| DPLL_N(t->scu28_12_8)
			| DPLL_M(t->scu28_7_0));
	} else {
		r = (DPLL_SIP(t->scu28_31_27)
			| DPLL_SIC(t->scu28_26_22)
			| DPLL_OD(t->scu28_21_19)
			| DPLL_P(t->scu28_18_13)
			| DPLL_N(t->scu28_12_8)
			| DPLL_M(t->scu28_7_0));
	}

	SET_SCU_REG(RSCU_DPLL, r);
	mb();
	CLR_SCU_REG_BITS(RSCU_DPLLEXT0, (DPLLEXT0_RESET|DPLLEXT0_OFF));

	/* Wait for PLL stable. */
	mdelay(1);
	uinfo("Set D1-PLL for pixel clock %d Hz. SCU28:0x%08X\n", HZx100*100, r);

	SCU_END();
	return 0;
}
#endif /* #if defined(AST1520_D1PLL_FRACTIONAL) */
#endif //#ifdef FPGA_TEST_ONLY

inline void _scu_init_board_info(void)
{
	board_info_t *info = &ast_scu.board_info;

	info->soc_ver = 3;
	info->soc_revision = REVISION_GET(ast_scu.revision_id) + 30;
	info->is_client = get_board_type();
	info->video_type = get_video_type();
	info->board_revision = get_board_revision();
	/* NOTE: info->video_type must be valid before calling get_video_loopback_type(). */
	/* NOTE: info->board_revision must be valid before calling get_video_loopback_type(). */
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
	ast_scu.CLKIN_Hz = get_clk_in_Hz();
	ast_scu.MPLL_Hz = get_MPLL_Hz();
	ast_scu.HPLL_Hz = get_HPLL_Hz();
	ast_scu.PCLK_Hz = get_PCLK_Hz();
	ast_scu.HCLK_Mhz = get_HCLK_MHz();
	ast_scu.HPLL_Mode = get_HPLL_mode();
	ast_scu.MCLK_Mhz = (ast_scu.MPLL_Hz / 1000000) >> 1; //M-Bus speed is always half of M-PLL clock

	/* Start of Misc. SCU init. */
	// Reset SDRAM controller on full chip watchdog reset.
	SET_SCU_REG_BITS(RSCU_RESET, RESET_DRAM_WDT);

	// Enable GPIOJ4~7 multi-function pin
	CLR_SCU_REG_BITS(RSCU_MFPC2,
	                 MFPC2_ENVGAHS_MASK
	               | MFPC2_ENVGAVS_MASK
	               | MFPC2_ENVGADDCCLK_MASK
	               | MFPC2_ENVGADDCDAT_MASK);

	/* disable MAC 2 */
	mac_mfp_cfg(2, 0);
}

int scu_op(u32 op, void *param)
{
	int rtn = 0;

	/* disable following message becasue gen-lock call this function frequently */
#if 0
	uinfo("scu_op(0x%08X) param(0x%08X)\n", op, (int)param);
#endif
	switch (op) {
		case SCUOP_SCU_LOCK:
			rtn = _scu_lock((int)param);
			break;
		case SCUOP_VHUB_INIT:
			rtn = _scu_usb20_init(0);
			break;
		case SCUOP_USB20HC_INIT:
			rtn = _scu_usb20_init(1);
			break;
		case SCUOP_USB11HC_INIT:
			rtn = _scu_usb11hc_init((int)param);
			break;
		case SCUOP_D1PLL_SET:
			rtn = _scu_d1pll_set((int)param);
			break;
		case SCUOP_MAC_INIT:
			rtn = _scu_mac_init((int)param);
			break;
		case SCUOP_HPLL_SET:
			rtn = set_HPLL_mode((int)param);
			break;
		case SCUOP_I2C_INIT:
			rtn = _scu_i2c_init((int)param);
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
		case SCUOP_VE_D2CLK_SELECTION: //SoC V1 only
			break;
		case SCUOP_VE_PIN_DIGITAL_IN:
			rtn = _scu_ve_pin_input_port(0);
			break;
		case SCUOP_VE_PIN_ANALOG_IN:
			rtn = _scu_ve_pin_input_port(1);
			break;
		case SCUOP_I2S_INIT:
			rtn = _scu_i2s_init();
			break;
		case SCUOP_I2S_PIN_HDMI_H:
			rtn = _scu_i2s_pin_hdmi_h();
			break;
		case SCUOP_I2S_PIN_HDMI_C:
			rtn = _scu_i2s_pin_hdmi_c();
			break;
		case SCUOP_I2S_PIN_CODEC_H:
			rtn = _scu_i2s_pin_codec_h((unsigned int)param);
			break;
		case SCUOP_I2S_PIN_CODEC_C:
			rtn = _scu_i2s_pin_codec_c((unsigned int)param);
			break;
#if (PATCH_CODEV3 & PATCH_AST1520A0_I2S_DUAL_OUTPUT)
		case SCUOP_I2S_PIN_DUAL_OUTPUT_C:
			rtn = _scu_i2s_pin_dual_output_c();
			break;
#endif
		case SCUOP_I2S_CLOCK_INIT:
			rtn = _scu_i2s_clock_init((unsigned int)param);
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
		case SCUOP_DEVICE_RST:
			rtn = _scu_device_rst((u32) param);
			break;
		case SCUOP_GEN_LOCK_INIT:
			rtn = _scu_video_gen_lock_init();
			break;
		case SCUOP_GEN_LOCK_ADJUST:
			rtn = _scu_video_gen_lock_adjust((u32) param);
			break;
		case SCUOP_GEN_LOCK_STOP:
			rtn = _scu_video_gen_lock_stop();
			break;
#if defined(CONFIG_ARCH_AST1500_CLIENT) && defined(SCU_AUDIO_GEN_LOCK)
		case SCUOP_AUDIO_GEN_LOCK_ADJUST:
			rtn = _scu_audio_gen_lock_adjust((u32) param);
			break;
		case SCUOP_AUDIO_GEN_LOCK_INIT:
			rtn = _scu_audio_gen_lock_init((u32) param);
			break;
		case SCUOP_AUDIO_GEN_LOCK_LIMIT:
			rtn = _scu_audio_gen_lock_limit_cfg((u32) param);
			break;
		case SCUOP_AUDIO_GEN_LOCK_SET:
			rtn = _scu_audio_gen_lock_set((u32) param);
			break;
#endif
		default:
			break;
	}
	return rtn;
}

static void _scu_init_ability(ability_info_t *ab)
{
	u32 efuse;

	scu_init_ability(ab);

	/*
	** Bruce160906. AST1525 EDID customization bug fix.
	** CRT driver will reference to ast_scu.ability.v_support_4k for EDID selection
	** before VE driver init. So, we have to configure v_support_4k right
	** before CRT driver loaded. SCU is the place to do it.
	*/
	efuse = GET_SCU_REG(0x158) & 0x1FUL;

	if (efuse == 0x11)
		ast_scu.ability.v_support_4k = 0;
}

static void scu_init_astparam_v3(void *context, astparam_t *astparam)
{
	scu_t *scu = (scu_t *)context;

#ifdef CONFIG_ARCH_AST1500_CLIENT
	astparam->v_clk_delay_target = get_video_clk_delay_4k_target(scu);
	/* astparam->v_clk_delay_target MUST be ready before calling get_video_clk_delay_4k(). */
	astparam->v_clk_delay = get_video_clk_delay_4k(scu, astparam->v_clk_delay_target);
	astparam->v_clk_duty = get_video_clk_duty_4k(scu);
#endif
	astparam->usb_quirk = get_usb_quirk(scu);
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
	_scu_init_ability(&ast_scu.ability);
	scu_init_astparam(&ast_scu, &ast_scu.astparam);
	scu_init_astparam_v3(&ast_scu, &ast_scu.astparam);
	astparam_destroy_shadow();
	_scu_lock(1);

	{
		unsigned int mpll_MHz = ast_scu.MPLL_Hz / 1000000;

		printk("\n\
Revision:0x%08X\n\
Clock Source: %d MHz\n\
CPU:%d MHz\n\
Memory: %d MHz (M-PLL:%d MHz, M Bus(MCLK):%d MHz, DDR:%d MHz)\n\
APB Bus(PCLK): %d MHz\n\
AHB Bus(HCLK): %d MHz\n\
Video Type:%s \n"
#ifdef CONFIG_ARCH_AST1500_HOST
"Video Loopback Type:%s \n"
#endif
"\n",
		ast_scu.revision_id
		,ast_scu.CLKIN_Hz / 1000000
		,ast_scu.HPLL_Hz / 1000000
		,mpll_MHz << 2, mpll_MHz, ast_scu.MCLK_Mhz, mpll_MHz << 1
		,ast_scu.PCLK_Hz / 1000000
		,ast_scu.HCLK_Mhz
		,video_type_s[ast_scu.board_info.video_type]
#ifdef CONFIG_ARCH_AST1500_HOST
		,video_type_s[ast_scu.board_info.video_loopback_type]
#endif
		);

	}
err_out:
	return err;
}

scu_t ast_scu =
{
	.scu_op = scu_op,
};
EXPORT_SYMBOL(ast_scu);

static struct platform_device *pdev;

static ssize_t show_board_info(struct device *dev, struct device_attribute *attr, char *buf)
{
	unsigned int mpll_MHz = ast_scu.MPLL_Hz / 1000000;

	sprintf(buf+strlen(buf), "\n\
Revision:0x%08X\n\
Clock Source: %d MHz\n\
CPU:%d MHz\n\
Memory: %d MHz (M-PLL:%d MHz, M Bus(MCLK):%d MHz, DDR:%d MHz)\n\
APB Bus(PCLK): %d MHz\n\
AHB Bus(HCLK): %d MHz\n\
Video Type:%s \n"
#ifdef CONFIG_ARCH_AST1500_HOST
"Video Loopback Type:%s \n"
#endif
"\n",
	ast_scu.revision_id
	,ast_scu.CLKIN_Hz / 1000000
	,ast_scu.HPLL_Hz / 1000000
	,mpll_MHz << 2, mpll_MHz, ast_scu.MCLK_Mhz, mpll_MHz << 1
	,ast_scu.PCLK_Hz / 1000000
	,ast_scu.HCLK_Mhz
	,video_type_s[ast_scu.board_info.video_type]
#ifdef CONFIG_ARCH_AST1500_HOST
	,video_type_s[ast_scu.board_info.video_loopback_type]
#endif
	);

	return strlen(buf);
}

static ssize_t store_board_info(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	unsigned int p1, p2;
	unsigned char cmd[16];

	sscanf(buf, "%s %d %d", cmd, &p1, &p2);
	if (!strncmp(cmd, "ve_clk", 6)) {
		uinfo("VE CLK: %d\n", p1);
		ast_scu.board_info.ve_encode_clk = p1;
	}
	return count;
} DEVICE_ATTR(board_info, (S_IRUGO | S_IWUSR), show_board_info, store_board_info);

#ifdef CONFIG_ARCH_AST1500_CLIENT
static ssize_t show_v_clk_delay(struct device *dev, struct device_attribute *attr, char *buf)
{
	sprintf(buf+strlen(buf), "0x%X\n", ast_scu.astparam.v_clk_delay);

	return strlen(buf);
}

static ssize_t store_v_clk_delay(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	u32 r;

	r = (u32)simple_strtoul(buf, NULL, 16);
	/* We only care SCUD8[6:11] bits */
	ast_scu.astparam.v_clk_delay = r;

	return count;
} DEVICE_ATTR(v_clk_delay, (S_IRUGO | S_IWUSR), show_v_clk_delay, store_v_clk_delay);
#endif

static ssize_t show_v_genlock(struct device *dev, struct device_attribute *attr, char *buf)
{
	sprintf(buf+strlen(buf), "\n\
Commands:\n\
    stop\n\
    init\n\
    adjust <ppm>\n\
\n");

	return strlen(buf);
}

static ssize_t store_v_genlock(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	int ppm;
	unsigned char cmd[16];

	sscanf(buf, "%s %i", cmd, &ppm);

	if (!strncmp(cmd, "stop", 4)) {
		_scu_video_gen_lock_stop();
	} else if (!strncmp(cmd, "init", 4)) {
		_scu_video_gen_lock_init();
	} else { //adjust
		_scu_video_gen_lock_adjust(ppm);
	}

	return count;
} DEVICE_ATTR(v_genlock, (S_IRUGO | S_IWUSR), show_v_genlock, store_v_genlock);

static struct attribute *dev_attrs[] = {
	&dev_attr_board_info.attr,
	&dev_attr_v_genlock.attr,
#ifdef CONFIG_ARCH_AST1500_CLIENT
	&dev_attr_v_clk_delay.attr,
#endif
	NULL,
};

static struct attribute_group dev_attr_group = {
	.attrs = dev_attrs,
};

/*
** This init function MUST only be called once.
*/
int ast_scu_init(void)
{
	int ret = 0;

	uinfo("ast_scu_init()\n");
	/* driver init */
	spin_lock_init(&ast_scu.lock);

	/* SoC dependent SCU init */
	ret = scu_init();
	if (ret)
		goto out;

	pdev = platform_device_register_simple(
	                           MODULE_NAME, /* driver name string */
	                           -1,          /* id */
	                           NULL,        /* struct resource to alloc */
	                           0);
	/* resource number */
	if (IS_ERR(pdev)) {
		ret = PTR_ERR(pdev);
		goto out;
	}

	if (sysfs_create_group(&pdev->dev.kobj, &dev_attr_group)) {
		uerr("can't create sysfs files\n");
		BUG();
		goto err_out;
	}

	return ret;

err_out:
	platform_device_unregister(pdev);

out:
	return ret;
}
core_initcall(ast_scu_init);

#endif //#if (CONFIG_AST1500_SOC_VER == 3)

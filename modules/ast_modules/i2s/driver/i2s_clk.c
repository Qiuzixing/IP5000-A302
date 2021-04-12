/*
 * Copyright (c) 2004-2012
 * ASPEED Technology Inc. All Rights Reserved
 * Proprietary and Confidential
 *
 * By using this code you acknowledge that you have signed and accepted
 * the terms of the ASPEED SDK license agreement.
 */

#if (CONFIG_AST1500_SOC_VER >= 2)

#include <asm/arch/drivers/crt.h>
#include <asm/arch/ast-scu.h>
#include "i2s.h"
#include "ast1510i2s.h"
#include "i2s_clk.h"

#if (CONFIG_AST1500_SOC_VER == 2)
static u32 clk_tbl_idx = 0;

#if NEW_FPGA_TX_PLL
static I2S_TABLE I2S_CLOCK_TABLE[] = {
	{0, 4, 1, 112, 0x460014},	/* 44.1KHz */
	{2, 4, 0x1, 100, 0x460014},	/* 48KHz */
	{3, 4, 1, 150, 0x460014},	/* 32KHz */
	{8, 4, 1, 56, 0x200014},	/* 88.2KHz */
	{10, 4, 0x2, 50, 0x200014},	/* 96KHz */
	{12, 4, 1, 28, 0x14000a},	/* 176.4KHz */
	{14, 4, 0x3, 25, 0x14000a}	/* 192KHz */
};

static unsigned short I2S_Table_Count = sizeof(I2S_CLOCK_TABLE) / sizeof(I2S_TABLE);
#else /* #if NEW_FPGA_TX_PLL */
/* 500 ppm */
static SOC_V2_I2S_TABLE I2S_CLOCK_TABLE[] = {
	{3,  22, 4,  OD_DIV2, PD_DIV4, I2SCLK_DIVIDE_16, 0x0097, 0x008c, 0x0083, I2SCLK_DIVIDE_4},/* 32khz */
	{0,  83, 11, OD_DIV2, PD_DIV4, I2SCLK_DIVIDE_16, 0x01eb, 0x0189, 0x0149, I2SCLK_DIVIDE_4},/* 44.1khz */
	{2,  74, 9,  OD_DIV2, PD_DIV4, I2SCLK_DIVIDE_16, 0x0138, 0x010e, 0x00ee, I2SCLK_DIVIDE_4},/* 48khz */
	{8,  83, 11, OD_DIV2, PD_DIV4, I2SCLK_DIVIDE_8,  0x01eb, 0x0189, 0x0149, I2SCLK_DIVIDE_4},/* 88.2khz */
	{10, 74, 9,  OD_DIV2, PD_DIV4, I2SCLK_DIVIDE_8,  0x0138, 0x010e, 0x00ee, I2SCLK_DIVIDE_4},/* 96khz */
	{12, 83, 11, OD_DIV2, PD_DIV4, I2SCLK_DIVIDE_4,  0x01eb, 0x0189, 0x0149, I2SCLK_DIVIDE_4},/* 176.4khz */
	{14, 74, 9,  OD_DIV2, PD_DIV4, I2SCLK_DIVIDE_4,  0x0138, 0x010e, 0x00ee, I2SCLK_DIVIDE_4} /* 192khz */
};

#if NEW_FREQ_ADJ
/* This is the clock table for CLK_LOCK_MODE == OFF */
static SOC_V2_I2S_TABLE I2S_CLOCK_TABLE_STATIC[] = {
	/* idx,num,denum,o_div,p_div,i2sclk_div,adj_H,adj_N,adj_L,mclk_div */
#if 0 /* This is the setting for MCLK(SPDIF) is needed. 24M PLL */
	{3,  60, 11,  OD_DIV2, PD_DIV4, I2SCLK_DIVIDE_16, 0, 0, 0, I2SCLK_DIVIDE_4}, /* 32khz */
	{0,  83, 11, OD_DIV2, PD_DIV4, I2SCLK_DIVIDE_16, 0, 0, 0, I2SCLK_DIVIDE_4}, /* 44.1khz */
	{2,  41, 5,  OD_DIV2, PD_DIV4, I2SCLK_DIVIDE_16, 0, 0, 0, I2SCLK_DIVIDE_4}, /* 48khz */
	{8,  83, 11, OD_DIV1, PD_DIV4, I2SCLK_DIVIDE_16,  0, 0, 0, I2SCLK_DIVIDE_4}, /* 88.2khz */
	{10, 41, 5,  OD_DIV1, PD_DIV4, I2SCLK_DIVIDE_16,  0, 0, 0, I2SCLK_DIVIDE_4}, /* 96khz */
	{12, 83, 11, OD_DIV1, PD_DIV2, I2SCLK_DIVIDE_16,  0, 0, 0, I2SCLK_DIVIDE_4}, /* 176.4khz */
	{14, 41, 5,  OD_DIV1, PD_DIV2, I2SCLK_DIVIDE_16,  0, 0, 0, I2SCLK_DIVIDE_4} /* 192khz */
#elif 1 /* This is the setting for non-MCLK(SPDIF) case. 24MHz PLL */
	{3,  43, 9,  OD_DIV2, PD_DIV4, I2SCLK_DIVIDE_14, 0, 0, 0, I2SCLK_DIVIDE_4}, /* 32khz */
	{0,  79, 12, OD_DIV2, PD_DIV4, I2SCLK_DIVIDE_14, 0, 0, 0, I2SCLK_DIVIDE_4}, /* 44.1khz */
	{2,  43, 6,  OD_DIV2, PD_DIV4, I2SCLK_DIVIDE_14, 0, 0, 0, I2SCLK_DIVIDE_4}, /* 48khz */
	{8,  79, 12, OD_DIV1, PD_DIV4, I2SCLK_DIVIDE_14,  0, 0, 0, I2SCLK_DIVIDE_4}, /* 88.2khz */
	{10, 43, 6,  OD_DIV1, PD_DIV4, I2SCLK_DIVIDE_14,  0, 0, 0, I2SCLK_DIVIDE_4}, /* 96khz */
	{12, 79, 12, OD_DIV1, PD_DIV2, I2SCLK_DIVIDE_14,  0, 0, 0, I2SCLK_DIVIDE_4}, /* 176.4khz */
	{14, 43, 6,  OD_DIV1, PD_DIV2, I2SCLK_DIVIDE_14,  0, 0, 0, I2SCLK_DIVIDE_4} /* 192khz */
#else /* This is the setting for 24.567MHz PLL without the need of MCLK(SPDIF). 24576 patch */
	{3,  32, 4,  OD_DIV4, PD_DIV4, I2SCLK_DIVIDE_12, 0, 0, 0, I2SCLK_DIVIDE_4}, /* 32khz */
	{0,  45, 7, OD_DIV2, PD_DIV4, I2SCLK_DIVIDE_14, 0, 0, 0, I2SCLK_DIVIDE_4}, /* 44.1khz */
	{2,  20, 4,  OD_DIV2, PD_DIV4, I2SCLK_DIVIDE_10, 0, 0, 0, I2SCLK_DIVIDE_4}, /* 48khz */
	{8,  45, 7, OD_DIV1, PD_DIV4, I2SCLK_DIVIDE_14,  0, 0, 0, I2SCLK_DIVIDE_4}, /* 88.2khz */
	{10, 20, 4,  OD_DIV1, PD_DIV4, I2SCLK_DIVIDE_10,  0, 0, 0, I2SCLK_DIVIDE_4}, /* 96khz */
	{12, 45, 7, OD_DIV1, PD_DIV2, I2SCLK_DIVIDE_14,  0, 0, 0, I2SCLK_DIVIDE_4}, /*176.4khz */
	{14, 24, 4,  OD_DIV1, PD_DIV4, I2SCLK_DIVIDE_6,  0, 0, 0, I2SCLK_DIVIDE_4} /*192khz */
#endif
	};
#endif /* #if NEW_FREQ_ADJ */

#if NEW_FREQ_DET
static Freq_Det Freq_Det_Table[] = {
#ifdef AST1510_I2S_A0
#if 0 /* I2SMCLK */
	{195, 3, "32KHz"},
	{142, 0, "44.1KHz"},
	{130, 2, "48KHz"},
	{71, 8, "88.2KHz"},
	{65, 10, "96KHz"},
	{36, 12, "176.4KHz"},
	{32, 14, "192KHz"}
#else /* I2SWS */
	{6250, 3, "32KHz"},
	{4535, 0, "44.1KHz"},
	{4167, 2, "48KHz"},
	{2268, 8, "88.2KHz"},
	{2083, 10, "96KHz"},
	{1134, 12, "176.4KHz"},
	{1042, 14, "192KHz"}
#endif
#else /* #ifdef AST1510_I2S_A0 */
#if (ACLK_SRC_SEL == 2)
#if (FREQ_DET_SRC_SEL == 2)
	{6750, 3, "32KHz"},
	{4898, 0, "44.1KHz"},
	{4500, 2, "48KHz"},
	{2449, 8, "88.2KHz"},
	{2250, 10, "96KHz"},
	{1225, 12, "176.4KHz"},
	{1125, 14, "192KHz"}
#endif
#endif
#endif /* #ifdef AST1510_I2S_A0 */
};
#endif /* #if NEW_FREQ_DET */
static unsigned short	I2S_Table_Count = sizeof(I2S_CLOCK_TABLE) / sizeof(SOC_V2_I2S_TABLE);
#endif /* #if NEW_FPGA_TX_PLL */
#endif /* #if (CONFIG_AST1500_SOC_VER) == 2*/

#ifdef FPGA_TEST_ONLY
static int fpga_set_tx_clock (u8 target_sample_rate)
{
#if (CONFIG_AST1500_SOC_VER >= 3)
	/* --- Start of FPGA Only. For AST1520 FPGA. */
	switch (target_sample_rate) {
	case I2S_SAMPLE_RATE_44d1K:
		write_register_and(I2S_ENG_CLK_CFG, 0, BIT8);
		break;
	case I2S_SAMPLE_RATE_48K:
		write_register_and(I2S_ENG_CLK_CFG, BIT8, BIT8);
		break;
	default:
		printk("I2S: sample rate idx %d not supported!!\n", target_sample_rate);
		break;
	}
	/* --- Endif of FPGA Only */
#elif NEW_FPGA_TX_PLL
	write_register(I2S_ENG_CLK_CFG,
				I2S78_I2SCLKO_DIV_EN(0)
				| I2S78_I2SMCLKO_DIV_EN(0)
				| I2S78_ACLK_DIV_EN(0)
				| I2S78_I2SCLK_DIV_EN(0)
				| I2S78_ACLK_SRC(0)
				| I2S78_I2SCLK_SRC(0)
				| I2S78_FIFO_CTRL(3)
				| I2S78_TEST_MODE(0));
#endif
	/* Success */
	return 1;

	/* BruceToDo. Fail case. */
	printk("I2S ERROR: Can't find sample_rate = %d\n", target_sample_rate);
	return 0; /* Not Found */
}
#endif

#if (CONFIG_AST1500_SOC_VER >= 3)
int set_tx_clock(u8 target_sample_rate)
{
#ifdef FPGA_TEST_ONLY
	return fpga_set_tx_clock(target_sample_rate);
#endif

	ast_scu.scu_op(SCUOP_I2S_CLOCK_INIT, (void *)(int)target_sample_rate);
	write_register_and(I2S_ENG_CLK_CFG,
				I2S78_I2SCLKO_DIV(I2SCLK_DIVIDE_4)
				| I2S78_I2SCLKO_DIV_EN(1)
				| I2S78_I2SMCLKO_DIV(0)
				| I2S78_I2SMCLKO_DIV_EN(0),
				I2S78_I2SCLKO_DIV_SEL_MASK
				| I2S78_I2SCLKO_DIV_EN_MASK
				| I2S78_I2SMCLKO_DIV_SEL_MASK
				| I2S78_I2SMCLKO_DIV_EN_MASK);

	write_register_and(I2S_ENG_CLK_CFG,
				I2S78_I2SCLK_DIV_EN(0) | I2S78_I2SCLK_SRC(0),
				I2S78_I2SCLK_DIV_EN_MASK | I2S78_I2SCLK_SRC_SEL_MASK);

	wmb();

	return 1;
}
#else /*  #if (CONFIG_AST1500_SOC_VER >= 3) */
int set_tx_clock(u8 target_sample_rate)
{
	unsigned long i;
	u32 mclko_div_en = 0, mclko_div_sel = 0;
	SOC_V2_I2S_TABLE *clk_table = I2S_CLOCK_TABLE;

#ifdef FPGA_TEST_ONLY
	return fpga_set_tx_clock(target_sample_rate);
#endif

#if NEW_FREQ_ADJ
	if (CLK_LOCK_MODE_SELECT_GET(clock_lock_mode()) == CLK_LOCK_MODE_OFF)
		clk_table = I2S_CLOCK_TABLE_STATIC;
#endif

	/* scu1c, set clock configs */
	for (i = 0; i < I2S_Table_Count; i++) {
		if (target_sample_rate == clk_table[i].sample_rate_index)
			break;
	}

	if (i == I2S_Table_Count) {
		printk ("I2S ERROR: Can't find sample_rate = %d\n", target_sample_rate);
		return 0; /* Not Found */
	}

	clk_tbl_idx = i;

	/* Start Config Clock */
	{ /* BruceToDo. How to move to SCU driver. WTF. */
		unsigned long flags;
		local_irq_save(flags);
		write_register(SCU_D2PLL_PARAM_REGISTER,
					SCU1C_NUMERATOR(clk_table[i].numerator)
					| SCU1C_DENUMERATOR(clk_table[i].denumerator)
					| SCU1C_OUTPUT_DIVIDER(clk_table[i].output_divider)
					| SCU1C_POST_DIVIDER(clk_table[i].post_divider)
					| SCU1C_TURN_OFF_PLL(0)
					| SCU1C_EN_BYPASS_MODE(0));
		local_irq_restore(flags);
	}

	/* MCLK should be 4 * BCLK */
	switch (clk_table[clk_tbl_idx].i2sclko_divider) {
	case I2SCLK_DIVIDE_16:
		mclko_div_en = 1;
		mclko_div_sel = I2SCLK_DIVIDE_4;
		break;
	case I2SCLK_DIVIDE_8:
		mclko_div_en = 1;
		mclko_div_sel = I2SCLK_DIVIDE_2;
		break;
	case I2SCLK_DIVIDE_4:
		mclko_div_en = 0;
		mclko_div_sel = 0; /* don't divide */
		break;
	default:
		printk("I2S WARNING: Not supported i2sclko_divider (%x)?!. Won't generate MCLK.\n", clk_table[clk_tbl_idx].i2sclko_divider);
		break;
	}

	write_register_and(I2S_ENG_CLK_CFG,
				I2S78_I2SCLKO_DIV(clk_table[clk_tbl_idx].i2sclko_divider)
					| I2S78_I2SCLKO_DIV_EN(1)
					| I2S78_I2SMCLKO_DIV(mclko_div_sel)
					| I2S78_I2SMCLKO_DIV_EN(mclko_div_en),
				I2S78_I2SCLKO_DIV_SEL_MASK
					| I2S78_I2SCLKO_DIV_EN_MASK
					| I2S78_I2SMCLKO_DIV_SEL_MASK
					| I2S78_I2SMCLKO_DIV_EN_MASK);

	write_register_and(I2S_ENG_CLK_CFG,
				I2S78_I2SCLK_DIV_EN(0) | I2S78_I2SCLK_SRC(0),
				I2S78_I2SCLK_DIV_EN_MASK | I2S78_I2SCLK_SRC_SEL_MASK);

	wmb();

	return 1;
}
#endif /* #if (CONFIG_AST1500_SOC_VER >= 3) */

static int i2s_clock_ppm = 0;
#if defined(I2S_CLOCK_ADJUST_NEW)
static int i2s_clock_max_distance = I2S_CLOCK_MAX_DISTANCE;
static int i2s_clock_ppm_adjust_interval_in_ms = I2S_CLOCK_PPM_ADJUST_INTERVEL_IN_MS;
static int i2s_clock_ppm_adjust_step = I2S_CLOCK_PPM_ADJUST_STEP;
static int i2s_clock_ppm_now;
#endif
static u32 i2s_clock_limit_ppm = CLK_LOCK_MODE_PPM_LIMIT_DEFAULT;

int clock_ppm(void)
{
	return i2s_clock_ppm;
}

u32 clock_limit(void)
{
	return i2s_clock_limit_ppm;
}

void clock_limit_cfg(u32 limit)
{
	if (limit > CLK_LOCK_MODE_PPM_LIMIT_MAX)
		limit = CLK_LOCK_MODE_PPM_LIMIT_MAX;

	if (limit == 0) {
		limit = CLK_LOCK_MODE_PPM_LIMIT_DEFAULT;
#if defined(I2S_CLOCK_ADJUST_NEW)
		if (ast_scu.ability.soc_op_mode >= 3)
			limit = CLK_LOCK_MODE_PPM_LIMIT_DEFAULT_NEW;
#endif
	}

	i2s_clock_limit_ppm = limit;
}

#if defined(I2S_CLOCK_ADJUST_NEW)
/*
 * 3 stages
 * tx_filled status
 *      0         under     n_d        n        n_u          over        Max
 *      |----------+---------+---------+---------+---------+----------|
 *                 |-------- L --------|
 *                           |-------- N --------|
 *                                     |-------- H --------|
 *
 * L
 * if (tx > n)
 *	Up
 * if (tx < under)
	under run
 * N
 * if (tx < n_d)
 *	Down
 * if (tx > n_u)
 *	Up
 * H
 * if (tx < n)
 *	Down
 * if (tx > over)
 *	over run
 */
static u32 i2s_new_th_under, i2s_new_th_n_d, i2s_new_th_n, i2s_new_th_n_u, i2s_new_th_over;
#endif /* #if defined(I2S_CLOCK_ADJUST_NEW) */
/*
 * i2s_th_n_d: threshold for N to down (ND)  (N => L)
 * i2s_th_n_u: threshold for N to up (NU)    (N => H)
 * i2s_th_l_u: threshold for L to up (LU)    (L => N(H))
 * i2s_th_h_d: threshold for H to down (HD)  (H => N(L))
 *
 * 2 stages
 * tx_filled status
 *      0                HD                LU                 Max
 *      |-----------------+----------------+------------------|
 *
 * 1. L
 * if (tx > LU)
 *	Up
 * 2. H
 * if (tx < HD)
 *	Down
 *
 * 3 stages
 * tx_filled status
 *      0         ND       LU         HD         NU           Max
 *      |----------+-------+-----------+---------+------------|
 *
 * 1. L
 * if (tx > LU)
 *	Up
 * 2. N
 * if (tx < ND)
 *	Down
 * if (tx > NU)
 *	Up
 * 3. H
 * if (tx < HD)
 *	Down
 */
static u32 i2s_th_n_d, i2s_th_l_u, i2s_th_h_d, i2s_th_n_u;

#if (CONFIG_AST1500_SOC_VER == 2)
static u32 Parameter_H, Parameter_N, Parameter_L;
#endif

enum TX_FREQ_ADJ_STAT i2s_stg;

u32 i2s_stage(void)
{
	return i2s_stg;
}

#if (CONFIG_AST1500_SOC_VER >= 3)
#define CLOCK_STAGE_H	(1)
#define CLOCK_STAGE_N	(0)
#define CLOCK_STAGE_L	(-1)
static inline void audio_clock_adjust(int stage)
{
	ast_scu.scu_op(SCUOP_AUDIO_GEN_LOCK_ADJUST, (void *)stage);
}

static inline void _set_stage_N(void)
{
	audio_clock_adjust(CLOCK_STAGE_N);
}
#else /* #if (CONFIG_AST1500_SOC_VER >= 3) */
#ifdef I2SPLL_EnStage_3
static inline void _set_stage_N(void)
{
#ifdef AST1510_I2S_A0 /* work around A0 bug */
	write_register_and(I2S_ENG_CFG, I2S20_EN_FAC_INC(0), I2S20_EN_FAC_INC(1));
#endif
	write_register_and(I2S_PLL_FREQ_ADJ_PARAM, Parameter_N, 0xFFFF);
#ifdef AST1510_I2S_A0 /* work around A0 bug */
	write_register_and(I2S_ENG_CFG, I2S20_EN_FAC_INC(1), I2S20_EN_FAC_INC(1));
#endif
}
#else /* #ifdef I2SPLL_EnStage_3 */
#ifdef I2SPLL_EnParameter_1
static inline void _set_stage_N(void)
{
	write_register_and(I2S_ENG_CFG, I2S20_EN_FAC_INC(0), I2S20_EN_FAC_INC(1));
}
#else /* #ifdef I2SPLL_EnParameter_1 */
static inline void _set_stage_N(void)
{
#ifdef AST1510_I2S_A0 /* work around A0 bug */
	write_register_and(I2S_ENG_CFG, I2S20_EN_FAC_INC(0), I2S20_EN_FAC_INC(1));
#endif
	write_register_and(I2S_PLL_FREQ_ADJ_PARAM, Parameter_N, 0xFFFF);
#ifdef AST1510_I2S_A0 /* work around A0 bug */
	write_register_and(I2S_ENG_CFG, I2S20_EN_FAC_INC(1), I2S20_EN_FAC_INC(1));
#endif
}
#endif /* #ifdef I2SPLL_EnParameter_1 */
#endif /* #ifdef I2SPLL_EnStage_3 */
#endif /* #if (CONFIG_AST1500_SOC_VER >= 3) */

static inline void set_stage_N(void)
{
	if (i2s_stg != I2SPLL_Stg_N) {
		i2s_stg = I2SPLL_Stg_N;
		_set_stage_N();
		printk("N ");
#if defined(I2S_CLOCK_ADJUST_NEW)
		i2s_clock_ppm_now = i2s_clock_ppm;
#endif
	}
}

void set_stage_reset(void)
{
	i2s_stg = I2SPLL_Stg_N;
	_set_stage_N();
}

#if (CONFIG_AST1500_SOC_VER == 2)
/*
 * i2s_parameter_m - calculate M parameter for 1510 I2S gen-lock by different ppm
 * @entry: pointer to I2S_TABLE
 * @ppm: target ppm
 *
 * source PLL, Fp = Fs * (pll_adj_N + 1) / pll_adj_N = Fs * (1 + PPMmax)
 * target freq, Ft = Fs * (1 + PPMt)
 *
 * Fp * M / (M + 1) = Ft
 * M = Ft / (Fp - Ft)
 *   = (1 + PPMt) / ((1 + PPMmax) - (1 + PPMt))
 *   =  (1 + PPMt) / (PPMmax - PPMt)
 */
static u32 i2s_parameter_m(SOC_V2_I2S_TABLE *entry, int ppm)
{
	u32 max_ppm, result;

	max_ppm = 1000000 / entry->pll_adj_N;

	if (abs(ppm) > max_ppm) {
		ppm = max_ppm;
		if (ppm <= -1)
			ppm = -max_ppm;
	}

	result = (1000000 + ppm) / (max_ppm - ppm);

	return result;
}
#endif

static void i2s_gen_lock_init(int ppm)
{
	int limit = clock_limit();

	/*
	 * From HDMI spec 1.4b, 7.3
	 * audio sample rate within +/- 1000ppm
	 */
	if ((ppm + limit)  > 1000)
		ppm = 1000 - limit;
	if ((ppm - limit) < -1000)
		ppm = -1000 + limit;

	i2s_clock_ppm = ppm;
#if defined(I2S_CLOCK_ADJUST_NEW)
	i2s_clock_ppm_now = i2s_clock_ppm;
	DPRINT("%d (+/-%d)\n", i2s_clock_ppm, limit);
#endif

#if (CONFIG_AST1500_SOC_VER >= 3)
	ast_scu.scu_op(SCUOP_AUDIO_GEN_LOCK_LIMIT, (void *)(int) limit);
	ast_scu.scu_op(SCUOP_AUDIO_GEN_LOCK_INIT, (void *)(int) ppm);
#else
	Parameter_H = i2s_parameter_m(I2S_CLOCK_TABLE + clk_tbl_idx, ppm + limit);
	Parameter_N = i2s_parameter_m(I2S_CLOCK_TABLE + clk_tbl_idx, ppm);
	Parameter_L = i2s_parameter_m(I2S_CLOCK_TABLE + clk_tbl_idx, ppm - limit);
#endif /* #if (CONFIG_AST1500_SOC_VER >= 3) */
}


static void i2s_th_init(u32 tx_buf_num)
{
#if defined(I2S_CLOCK_ADJUST_NEW)
	if (ast_scu.ability.soc_op_mode >= 3) {
		u32 kick_th;

		kick_th = tx_kick_th();

		i2s_new_th_over = kick_th + I2S_TX_BUFFER_TH_OVER_SIZE;
		i2s_new_th_n_u = kick_th + I2S_TX_BUFFER_TH_STABLE_SZIE;
		i2s_new_th_n = kick_th;
		i2s_new_th_n_d = kick_th - I2S_TX_BUFFER_TH_STABLE_SZIE;
		i2s_new_th_under = kick_th - I2S_TX_BUFFER_TH_OVER_SIZE;

		return;
	}
#endif /* #if defined(I2S_CLOCK_ADJUST_NEW) */
	/*
	 * i2s_th_n_d: threshold for N to down (ND)  (N => L)
	 * i2s_th_n_u: threshold for N to up (NU)    (N => H)
	 * i2s_th_l_u: threshold for L to up (LU)    (L => N(H))
	 * i2s_th_h_d: threshold for H to down (HD)  (H => N(L))
	 *
	 * 2 stages
	 * tx_filled status
	 *      0                HD                LU                 Max
	 *      |-----------------+----------------+------------------|
	 *
	 * 1. L
	 * if (tx > LU)
	 *	Up
	 * 2. H
	 * if (tx < HD)
	 *	Down
	 *
	 * 3 stages
	 * tx_filled status
	 *      0         ND       LU         HD         NU           Max
	 *      |----------+-------+-----------+---------+------------|
	 *
	 * 1. L
	 * if (tx > LU)
	 *	Up
	 * 2. N
	 * if (tx < ND)
	 *	Down
	 * if (tx > NU)
	 *	Up
	 * 3. H
	 * if (tx < HD)
	 *	Down
	 */
#ifdef I2SPLL_EnStage_3
	i2s_th_n_d = (tx_buf_num >> 2); /* < 1/4, N => L*/
	i2s_th_l_u = (tx_buf_num / 3); /* > 1/3, L => N */
	i2s_th_h_d = (tx_buf_num << 1) / 3; /* < 2/3, H => N */
	i2s_th_n_u = ((3 * tx_buf_num) >> 2); /* > 3/4, N => H */
#else
	i2s_th_l_u = (tx_buf_num / 3) << 1; /* > 2/3,  L => H */
	i2s_th_h_d = tx_buf_num / 3; /* < 1/3, H => L */
	i2s_th_n_u = i2s_th_l_u; /* > 2/3, N => H */
	i2s_th_n_d = i2s_th_h_d; /* < 1/3, H => L */
#endif
}

void i2s_pll_adjust_init(u32 tx_buf_num, int adjust_ppm)
{
	i2s_th_init(tx_buf_num);
	i2s_gen_lock_init(adjust_ppm);

	set_stage_reset();

	/*
	 * set_stage_reset() already set i2s stage to reset stage (N)
	 * => disable following code
	 */
#if 0
#if NEW_FREQ_ADJ
#if (2 == CONFIG_AST1500_SOC_VER)
#ifdef I2SPLL_EnParameter_1
	write_register_and(I2S_PLL_FREQ_ADJ_PARAM, Parameter_L, 0xFFFF);
#endif
#endif

#else /* #if NEW_FREQ_ADJ */
#if (2 == CONFIG_AST1500_SOC_VER)
	write_register_and(I2S_PLL_FREQ_ADJ_PARAM, I2S_CLOCK_TABLE[clk_tbl_idx].pll_adj_L, 0xFFFF);
#endif
#endif /* #if NEW_FREQ_ADJ */
#endif
}

static u32 _clock_lock_mode = (CLK_LOCK_MODE_PPM_LIMIT_DEFAULT << CLK_LOCK_MODE_PPM_LIMIT_OFFSET) |  CLK_LOCK_MODE_AUTO;

u32 clock_lock_mode(void)
{
	return _clock_lock_mode;
}

void clock_lock_mode_cfg(u32 cfg)
{
	_clock_lock_mode = cfg;

	clock_limit_cfg(cfg >> CLK_LOCK_MODE_PPM_LIMIT_OFFSET);
}

#if defined(I2S_CLOCK_ADJUST)
static u32 last_tx_filled = 0; /* we'll determine whether adjsut central frequency by last_tx_filled when tx empty */
static u32 _clock_adj_test_mode = 0; /* 0:normal; 1:L<->N; 2:N<->H; 3:L<->N<->H */

u32 clock_adj_test_mode(void)
{
	return _clock_adj_test_mode;
}

void clock_adj_test_mode_cfg(u32 cfg)
{
	_clock_adj_test_mode = cfg;
}

#if (3 <= CONFIG_AST1500_SOC_VER)
static inline void _set_stage_H(void)
{
	audio_clock_adjust(CLOCK_STAGE_H);
}

static inline void _set_stage_L(void)
{
	audio_clock_adjust(CLOCK_STAGE_L);
}
#else /* (CONFIG_AST1500_SOC_VER >= 3) */
#if NEW_FREQ_ADJ
#ifdef I2SPLL_EnStage_3
static inline void _set_stage_H(void)
{
#ifdef AST1510_I2S_A0 /* work around A0 bug */
	write_register_and(I2S_ENG_CFG, I2S20_EN_FAC_INC(0), I2S20_EN_FAC_INC(1));
#endif
	write_register_and(I2S_PLL_FREQ_ADJ_PARAM, Parameter_H, 0xFFFF);
#ifdef AST1510_I2S_A0 /* work around A0 bug */
	write_register_and(I2S_ENG_CFG, I2S20_EN_FAC_INC(1), I2S20_EN_FAC_INC(1));
#endif
}

static inline void _set_stage_L(void)
{
#ifdef AST1510_I2S_A0 /* work around A0 bug */
	write_register_and(I2S_ENG_CFG, I2S20_EN_FAC_INC(0), I2S20_EN_FAC_INC(1));
#endif
	write_register_and(I2S_PLL_FREQ_ADJ_PARAM, Parameter_L, 0xFFFF);
#ifdef AST1510_I2S_A0 /* work around A0 bug */
	write_register_and(I2S_ENG_CFG, I2S20_EN_FAC_INC(1), I2S20_EN_FAC_INC(1));
#endif
}
#else /* #ifdef I2SPLL_EnStage_3 */
#ifdef I2SPLL_EnParameter_1
static inline void _set_stage_H(void)
{
	printk("H!!!\n");
}

static inline void _set_stage_L(void)
{
	write_register_and(I2S_ENG_CFG, I2S20_EN_FAC_INC(1), I2S20_EN_FAC_INC(1));
}
#else
static inline void _set_stage_H(void)
{
#ifdef AST1510_I2S_A0 /* work around A0 bug */
	write_register_and(I2S_ENG_CFG, I2S20_EN_FAC_INC(0), I2S20_EN_FAC_INC(1));
#endif
	write_register_and(I2S_PLL_FREQ_ADJ_PARAM, Parameter_H, 0xFFFF);
#ifdef AST1510_I2S_A0 /* work around A0 bug */
	write_register_and(I2S_ENG_CFG, I2S20_EN_FAC_INC(1), I2S20_EN_FAC_INC(1));
#endif
}

static inline void _set_stage_L(void)
{
#ifdef AST1510_I2S_A0 /* work around A0 bug */
	write_register_and(I2S_ENG_CFG, I2S20_EN_FAC_INC(0), I2S20_EN_FAC_INC(1));
#endif
	write_register_and(I2S_PLL_FREQ_ADJ_PARAM, Parameter_L, 0xFFFF);
#ifdef AST1510_I2S_A0 /* work around A0 bug */
	write_register_and(I2S_ENG_CFG, I2S20_EN_FAC_INC(1), I2S20_EN_FAC_INC(1));
#endif
}
#endif
#endif /* #ifdef I2SPLL_EnStage_3 */
#endif /* #if NEW_FREQ_ADJ */
#endif /* (CONFIG_AST1500_SOC_VER >= 3) */

static inline void set_stage_H(void)
{
	if (i2s_stg != I2SPLL_Stg_H) {
		_set_stage_H();
		i2s_stg = I2SPLL_Stg_H;
		printk("H ");
#if defined(I2S_CLOCK_ADJUST_NEW)
		i2s_clock_ppm_now = i2s_clock_ppm + i2s_clock_limit_ppm;
#endif
	}
}

static inline void set_stage_L(void)
{
	if (i2s_stg != I2SPLL_Stg_L) {
		_set_stage_L();
		i2s_stg = I2SPLL_Stg_L;
		printk("L ");
#if defined(I2S_CLOCK_ADJUST_NEW)
		i2s_clock_ppm_now = i2s_clock_ppm - i2s_clock_limit_ppm;
#endif
	}
}

static inline void set_stage_up(void)
{
#ifdef I2SPLL_EnStage_3
	if (I2SPLL_Stg_L == i2s_stg)
		set_stage_N();
	else
		set_stage_H();
#else
	set_stage_H();
#endif
}

static inline void set_stage_down(void)
{
#ifdef I2SPLL_EnStage_3
	if (I2SPLL_Stg_H == i2s_stg)
		set_stage_N();
	else
		set_stage_L();
#else
	set_stage_L();
#endif
}


#if defined(I2S_CLOCK_ADJUST_NEW)
static int i2s_gen_lock_en = 0;
static int i2s_ppm_cal_done;
static u32 i2s_ppm_buffer_num_change_update = 2; /* 2, wait for 2-buffer tx queue length change */
static u32 i2s_ppm_factor_sample_num; /* 640000000, num x10000000 (x1000000x1000/100) */
static u32 i2s_ppm_factor_sample_freq; /* 480, sample frequency in 100Hz */

static u32 avg_cnt, avg, avg_benchmark;
static u32 avg_last_num = 0;
static unsigned long avg_last_jiffies = 0;

#define UPDATE_AVG_BENCHMARK(a) { avg_benchmark = (a); avg_last_jiffies = jiffies; DPRINT("BM 0x%x\n", avg_benchmark); }

void i2s_gen_lock_param_init(u32 sample_num, u32 sample_frequency)
{
	i2s_ppm_factor_sample_num = sample_num * 10000000 ; /* num x10000000 (x1000000x1000/100) */
	i2s_ppm_factor_sample_freq = sample_frequency / 100; /* sample frequency in 100Hz */
}

void i2s_gen_lock_restart(void)
{
	avg_cnt = 0;
	avg = 0;
	i2s_ppm_cal_done = 0;
	i2s_gen_lock_en = 1;
}

void i2s_gen_lock_start(ppm_init)
{
	i2s_gen_lock_init(ppm_init);
	i2s_gen_lock_restart();
}

void i2s_gen_lock_stop(void)
{
	i2s_gen_lock_en = 0;
}

static int do_average(u32 num)
{
	static int update = 0;

	if (avg_cnt < (0x1 << I2S_TX_LEN_AVG_SHIFT)) {
		if (num < 8) { /* wait for tx queue accumulation */
			avg_cnt = 0;
			avg = 0;
		} else {
			avg_cnt++;
			avg += (num << I2S_TX_LEN_SCALE_SHIFT);

			if (avg_cnt == (0x1 << I2S_TX_LEN_AVG_SHIFT)) {
				avg = avg >> I2S_TX_LEN_AVG_SHIFT;
				UPDATE_AVG_BENCHMARK(avg);
			}
		}
		update = 0;
	} else {
		if (num < 8) {
			if (ast_scu.ability.soc_op_mode == 1) {
				if (num < 3)
					DPRINT("! %d\n", num);
			} else
				DPRINT("! %d\n", num);
		}

		avg = ((avg << I2S_TX_LEN_AVG_SHIFT) - avg + (num << I2S_TX_LEN_SCALE_SHIFT)) >> I2S_TX_LEN_AVG_SHIFT;

		if (abs(avg - avg_benchmark) >= (i2s_ppm_buffer_num_change_update << I2S_TX_LEN_SCALE_SHIFT)) {
			if (avg > avg_benchmark) {/* lower clock cause longer queue, result is negative */
				update--;
			} else {
				update++;
			}

		} else {
			update = 0;
		}
	}

	avg_last_num = num;

	return update;
}

static void i2s_gen_lock_stable(u32 num)
{
	u32 len;

	len = avg >> I2S_TX_LEN_SCALE_SHIFT;

	switch (i2s_stg) {
	case I2SPLL_Stg_H:
		if (len > i2s_new_th_over) {
			DPRINT("Ov: %d > %d\n", len, i2s_new_th_over);
			i2s_gen_lock_start(i2s_clock_ppm + clock_limit());
			set_stage_H();
		} else if (len <= i2s_new_th_n) {
			set_stage_N();
			UPDATE_AVG_BENCHMARK(avg);
		}
		break;
	case I2SPLL_Stg_N:
		if (len <= i2s_new_th_n_d) {
			set_stage_L();
			UPDATE_AVG_BENCHMARK(avg);
		} else if (len >= i2s_new_th_n_u) {
			set_stage_H();
			UPDATE_AVG_BENCHMARK(avg);
		}
		break;
	case I2SPLL_Stg_L:
		if (len < i2s_new_th_under) {
			DPRINT("Un: %d < %d\n", len, i2s_new_th_under);
			i2s_gen_lock_start(i2s_clock_ppm - clock_limit());
			set_stage_L();
		} else if (len >= i2s_new_th_n) {
			set_stage_N();
			UPDATE_AVG_BENCHMARK(avg);
		}
		break;
	default:
		printk("invalid PLL stage (%d)\n", i2s_stg);
		BUG();
	}
}

static int get_ppm_distance(int target)
{
	int distance = 0;

	if ((target >= (i2s_clock_ppm + i2s_clock_limit_ppm)) || (target <= (i2s_clock_ppm - i2s_clock_limit_ppm))) {
		distance += (target - i2s_clock_ppm);

		if (abs(distance) > i2s_clock_max_distance) {
			if (distance > 0)
				distance = i2s_clock_max_distance;
			else
				distance = (-1) * i2s_clock_max_distance;

			DPRINT("fix: %d => %d (%d)\n", i2s_clock_ppm, i2s_clock_ppm + distance, target);
		}
	}

	return distance;
}

static void clock_adjust_new(u32 filled)
{
	static int ppm_distance = 0;
	static unsigned long timeout = 0;
	int update;

	if (i2s_gen_lock_en == 0)
		return;

	update = do_average(filled);

	if (abs(update) > 3) { /* 3 successive average results > benchmark */
		int deviation, target;
		unsigned long diff_ms;

		/*
		 * I2S spends T time to make one-buffer difference
		 * sample frequency fs, there are N samples in single one buffer
		 * total sample should be T x sample frequency
		 * the deviation is N * 1000000 / (T * fs) ppm
		 *
		 * 48K fs case: 64 sample/buffer
		 * assume we got one-buffer difference after 100 seconds
		 * the deviation is 64 * 1000000 / (100 * 48000) ppm ~= 13.333 ppm
		 */
		diff_ms = jiffies_to_msecs(jiffies - avg_last_jiffies);
		deviation = i2s_ppm_factor_sample_num * i2s_ppm_buffer_num_change_update / i2s_ppm_factor_sample_freq / diff_ms;

		if (update < 0)
			deviation = deviation * (-1);

		target = i2s_clock_ppm_now - deviation;

		i2s_ppm_cal_done = 1;

		UPDATE_AVG_BENCHMARK(avg);

		ppm_distance += get_ppm_distance(target);

		if (ppm_distance)
			timeout = jiffies;
	}

	if (ppm_distance) {
		if (time_after_eq(jiffies, timeout)) {
			int to_do;

			if (abs(ppm_distance) > i2s_clock_ppm_adjust_step)
				to_do = i2s_clock_ppm_adjust_step;
			else
				to_do = abs(ppm_distance);

			if (ppm_distance < 0)
				to_do = (-1) * to_do;

			ppm_distance = ppm_distance - to_do;

			DPRINT("%d => %d\n", i2s_clock_ppm, i2s_clock_ppm + to_do);
			i2s_gen_lock_init(i2s_clock_ppm + to_do);
			UPDATE_AVG_BENCHMARK(avg);
			set_stage_reset();

			timeout = jiffies + msecs_to_jiffies(i2s_clock_ppm_adjust_interval_in_ms);
		}
	}

	if (i2s_ppm_cal_done)
		i2s_gen_lock_stable(filled);
}

u32 i2s_gen_lock_benchmark(void)
{
	return avg_benchmark;
}

u32 i2s_gen_lock_avg(void)
{
	return avg;
}

u32 i2s_gen_lock_last_num(void)
{
	return avg_last_num;
}
#endif /* #if defined(I2S_CLOCK_ADJUST_NEW) */

static void clock_adjust_auto(u32 filled)
{
#if defined(I2S_CLOCK_ADJUST_NEW)
	if (ast_scu.ability.soc_op_mode >= 3) {
		clock_adjust_new(filled);
		return;
	}
#endif
	switch (i2s_stg) {
	case I2SPLL_Stg_H:
		if (filled <= i2s_th_h_d)
			set_stage_down();
		break;
	case I2SPLL_Stg_N:
		if (filled <= i2s_th_n_d)
			set_stage_down();
		else if (filled >= i2s_th_n_u)
			set_stage_up();
		break;
	case I2SPLL_Stg_L:
		if (filled >= i2s_th_l_u)
			set_stage_up();
		break;
	default:
		printk("invalid PLL stage (%d)\n", i2s_stg);
		BUG();
	}
}

void clk_adj_selection(u32 filled)
{
	switch (CLK_LOCK_MODE_SELECT_GET(clock_lock_mode())) {
	case CLK_LOCK_MODE_AUTO:
		clock_adjust_auto(filled);
		last_tx_filled = filled;
		break;
	case CLK_LOCK_MODE_N:
		set_stage_N();
		break;
	case CLK_LOCK_MODE_L:
		set_stage_L();
		break;
	case CLK_LOCK_MODE_H:
		set_stage_H();
		break;
	default:
		break;
	}
}

void clk_adj_proc(u32 tx_filled)
{
	static unsigned int direct = 0;

	switch (clock_adj_test_mode()) {
	case 0:
		clk_adj_selection(tx_filled);
		break;
	case 1:
		if (i2s_stg != I2SPLL_Stg_L)
			set_stage_L();
		else
			set_stage_N();
		break;
	case 2:
		if (i2s_stg != I2SPLL_Stg_H)
			set_stage_H();
		else
			set_stage_N();
		break;
	case 3:
		if (direct == 0) {
			if (i2s_stg == I2SPLL_Stg_L) {
				set_stage_N();
				direct = 1;
			} else
				set_stage_L();
		} else {
			if (i2s_stg == I2SPLL_Stg_H) {
				set_stage_N();
				direct = 0;
			} else
				set_stage_H();
		}
		break;
	default:
		printk(" incorrect clock_adj_test_mode(%d)\n", clock_adj_test_mode());
		BUG();
		break;
	}
}

static void i2s_clock_sliding_up(void)
{
	printk("U");
	i2s_gen_lock_init(i2s_clock_ppm + clock_limit());

	/*
	 * the step between set_stage_reset() and set_stage_up()
	 * is old H => new H, i.e. new N => new H, clock_limit()
	 */
	set_stage_reset();
	set_stage_up();
}

static void i2s_clock_sliding_down(void)
{
	printk("D");
	i2s_gen_lock_init(i2s_clock_ppm - clock_limit());
	/*
	 * the step between set_stage_reset() and set_stage_down()
	 * is N=>L, i.e. clock_limit()
	 * is old L => new L, i.e. new N => new L, clock_limit()
	 */
	set_stage_reset();
	set_stage_down();
}

#define I2S_TIME_TOLERANCE_MS 2000

void clock_fixup_tx_full(void)
{
	static unsigned long last_jiffies = 0, last_diff_ms = 0;
	unsigned long diff_ms;

	if (clock_lock_mode() & CLK_LOCK_MODE_PPM_FIXUP_OFF)
		return;

	if (i2s_stg == I2SPLL_Stg_H) {

		if (last_jiffies == 0) {
			last_jiffies = jiffies;
			return;
		}

		diff_ms = jiffies_to_msecs(jiffies - last_jiffies);
		last_jiffies = jiffies;

		if (last_diff_ms == 0) {
			last_diff_ms = diff_ms;
			return;
		}

		if (abs(diff_ms - last_diff_ms) < I2S_TIME_TOLERANCE_MS) {
			last_diff_ms = 0; last_jiffies = 0;
			i2s_clock_sliding_up();
			return;
		}
		last_diff_ms = diff_ms;
	} else {
		last_diff_ms = 0; last_jiffies = 0;
	}
}

void clock_fixup_tx_empty(void)
{
	if (clock_lock_mode() & CLK_LOCK_MODE_PPM_FIXUP_OFF)
		goto exit;
	/*
	 * because we kick tx after accumulation of tx length (tx_kick_th())
	 * do not choose the period detection for empty case
	 *
	 * current condition is
	 * stage is L (low frqenency) AND last tx filled length is short
	 *
	 * the value we compare to must < i2s_th_n_d (threshold for N to down (ND), filled < i2s_th_n_d,  N => L)
	 * and + tolerance, so use i2s_th_n_d/2
	 */
	if (i2s_stg == I2SPLL_Stg_L) {
		if (last_tx_filled < (i2s_th_n_d >> 1)) {
			i2s_clock_sliding_down();
			return;
		}
	}
exit:
	set_stage_reset();
}

u32 clk_adjust_length_max(void)
{
#ifdef I2SPLL_EnStage_3
	return i2s_th_n_u;
#else
	return i2s_th_l_u;
#endif
}

#if (CONFIG_AST1500_SOC_VER >= 3)
void clock_gen_lock_cfg(int ppm)
{
	i2s_clock_ppm = ppm;
#if defined(I2S_CLOCK_ADJUST_NEW)
	i2s_clock_ppm_now = ppm;
#endif
	ast_scu.scu_op(SCUOP_AUDIO_GEN_LOCK_SET, (void *)(int) ppm);
}

int clock_gen_lock(void)
{
#if defined(I2S_CLOCK_ADJUST_NEW)
	return i2s_clock_ppm_now;
#else
	return i2s_clock_ppm;
#endif
}
#endif /* #if (CONFIG_AST1500_SOC_VER >= 3) */
#endif /* #if defined(I2S_CLOCK_ADJUST) */
#endif /* #if (CONFIG_AST1500_SOC_VER >= 2) */

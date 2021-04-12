/*
 * Copyright (c) 2004-2012
 * ASPEED Technology Inc. All Rights Reserved
 * Proprietary and Confidential
 *
 * By using this code you acknowledge that you have signed and accepted
 * the terms of the ASPEED SDK license agreement.
 */

#ifndef _I2S_CLK_ADJUST_H
#define _I2S_CLK_ADJUST_H

#if (CONFIG_AST1500_SOC_VER == 2)
#define	OD_DIV1	0x1
#define	OD_DIV2	0x2
#define	OD_DIV4	0x3

#define	PD_DIV1	0x0
#define	PD_DIV2	0x2
#define	PD_DIV4	0x3
#endif /* #if (CONFIG_AST1500_SOC_VER == 2) */

#define	I2SCLK_DIVIDE_2		0x0 /* 000 */
#define	I2SCLK_DIVIDE_4		0x1 /* 001 */
#define	I2SCLK_DIVIDE_6		0x2 /* 010 */
#define	I2SCLK_DIVIDE_8		0x3 /* 011 */
#define	I2SCLK_DIVIDE_10	0x4 /* 100 */
#define	I2SCLK_DIVIDE_12	0x5 /* 101 */
#define	I2SCLK_DIVIDE_14	0x6 /* 110 */
#define	I2SCLK_DIVIDE_16	0x7 /* 111 */

#if defined(CONFIG_ARCH_AST1500_CLIENT)
	#define I2S_CLOCK_ADJUST
#endif

#if (CONFIG_AST1500_SOC_VER >= 3)
	#define NEW_FREQ_ADJ 0
	#define I2S_CLK_INFO_FROM_REMOTE
	#if defined(I2S_CLOCK_ADJUST)
		#define I2S_CLOCK_ADJUST_NEW
		#if defined(I2S_CLOCK_ADJUST_NEW)
			#define I2S_GEN_LOCK_DEBUG
		#endif
	#endif
#else
	#define NEW_FREQ_ADJ 1
	#if defined(I2S_CLOCK_ADJUST)
		#define I2S_CLOCK_ADJUST_IN_THREAD
	#endif
#endif

#define CLK_LOCK_MODE_PPM_LIMIT_MASK 0xFFFF0000 /* bit 31~16 */
#define CLK_LOCK_MODE_PPM_LIMIT_MAX     500 /* choose 500 because previous implementation in ast1510 is +/-500 ppm */
#if defined(I2S_CLOCK_ADJUST_NEW)
#define CLK_LOCK_MODE_PPM_LIMIT_DEFAULT_NEW 10
#endif
#define CLK_LOCK_MODE_PPM_LIMIT_DEFAULT 100
#define CLK_LOCK_MODE_PPM_LIMIT_OFFSET 16
#define CLK_LOCK_MODE_PPM_FROM_CTS_OFF (0x2000) /* bit 13, set to disable target PPM by N&CTS, we do not take this anymore */
#define CLK_LOCK_MODE_PPM_FIXUP_OFF (0x1000) /* bit 12, set to disable target PPM fixup  */
#define CLK_LOCK_MODE_MIC_PATCH (0x100)
#define CLK_LOCK_MODE_SELECT_MASK 0x000000FF /* bit 7~0 */
#define CLK_LOCK_MODE_OFF (0x80)
#define CLK_LOCK_MODE_AUTO 0
#define CLK_LOCK_MODE_N 1
#define CLK_LOCK_MODE_L 2
#define CLK_LOCK_MODE_H 4

#define CLK_LOCK_MODE_PPM_LIMIT_GET(cfg) (((cfg) & CLK_LOCK_MODE_PPM_LIMIT_MASK) >> CLK_LOCK_MODE_PPM_LIMIT_OFFSET)
#define CLK_LOCK_MODE_SELECT_SET(mode)  ((mode) & CLK_LOCK_MODE_SELECT_MASK)
#define CLK_LOCK_MODE_SELECT_GET(cfg) ((cfg) & CLK_LOCK_MODE_SELECT_MASK)

#if defined(I2S_CLOCK_ADJUST_NEW)
#define I2S_CLOCK_MAX_DISTANCE 100
#define I2S_CLOCK_PPM_ADJUST_INTERVEL_IN_MS 500
#define I2S_CLOCK_PPM_ADJUST_STEP 1000
#define I2S_TX_LEN_AVG_SHIFT 7
#define I2S_TX_LEN_SCALE_SHIFT 8
#define I2S_TX_BUFFER_TH_STABLE_SZIE 4
#define I2S_TX_BUFFER_TH_OVER_SIZE (I2S_TX_BUFFER_TH_STABLE_SZIE + 8)
#endif

#if defined(I2S_GEN_LOCK_DEBUG)
#define DPRINT(fmt, args...) { printk("AGL:" fmt, ##args); }

#else
#define DPRINT(fmt, args...) do {} while (0)
#endif /* #if defined(I2S_GEN_LOCK_DEBUG) */

int set_tx_clock(u8 target_sample_rate);
void i2s_pll_adjust_init(u32 tx_buf_num, int ppm);

enum TX_FREQ_ADJ_STAT {I2SPLL_Stg_L = 0, I2SPLL_Stg_N = 1, I2SPLL_Stg_H = 2};

u32 i2s_stage(void);

u32 clock_adj_test_mode(void);
void clock_adj_test_mode_cfg(u32 cfg);

u32 clock_lock_mode(void);
void clock_lock_mode_cfg(u32 cfg);

void clk_adj_selection(u32 filled);
void set_stage_reset(void);

u32 clock_limit(void);
void clock_limit_cfg(u32 ppm);
int clock_ppm(void);

void clock_fixup_tx_full(void);
void clock_fixup_tx_empty(void);
u32 clk_adjust_length_max(void);

#if defined(I2S_CLOCK_ADJUST)
void clk_adj_proc(u32 tx_filled);
#endif

#if (CONFIG_AST1500_SOC_VER >= 3)
void clock_gen_lock_cfg(int ppm);
int clock_gen_lock(void);
#endif

#if defined(I2S_CLOCK_ADJUST_NEW)
void i2s_gen_lock_param_init(u32 sample_num, u32 sample_frequency);
void i2s_gen_lock_start(int ppm_init);
void i2s_gen_lock_restart(void);
void i2s_gen_lock_stop(void);
u32 i2s_gen_lock_benchmark(void);
u32 i2s_gen_lock_avg(void);
u32 i2s_gen_lock_last_num(void);
#endif
#endif /* #ifndef _I2S_CLK_ADJUST_H */

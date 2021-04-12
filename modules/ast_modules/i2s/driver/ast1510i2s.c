/*
 * Copyright (c) 2004-2012
 * ASPEED Technology Inc. All Rights Reserved
 * Proprietary and Confidential
 *
 * By using this code you acknowledge that you have signed and accepted
 * the terms of the ASPEED SDK license agreement.
 */

#if (CONFIG_AST1500_SOC_VER >= 2)

#include <linux/delay.h>
#include <linux/platform_device.h>
#include <asm/arch/drivers/board_def.h>
#include <asm/arch/drivers/crt.h>
#include <asm/arch/ast-scu.h>
#include "i2s.h"
#include "ast1510i2s.h"
#include "i2s_clk.h"

#define NLPCM_THROUGH_I2S

static unsigned int rx_buf_size;
static unsigned int rx_buf_num = 0;
static unsigned int tx_buf_size;
static unsigned int tx_buf_num = 0;
static unsigned int rx_idx, rx_free;
unsigned int kick_tx_idx = 0;
static unsigned int i2s_des_vld = 0;
static volatile TX_DESC *tx_desc;
static volatile RX_DESC *rx_desc;
extern unsigned int rx_ready;
extern unsigned int tx_ready;
extern unsigned int rx_enabled;
extern unsigned int  rx_data_available;

#if defined(CONFIG_ARCH_AST1500_CLIENT) && defined(I2S_CLOCK_ADJUST_IN_THREAD)
extern wait_queue_head_t	clock_adj_wait_queue;
#endif

void reset_audio_engine(void);

static unsigned int get_tx_desc_idx(void)
{
	volatile unsigned long old, value;

	old = read_register(I2S_HW_TX_DESC_PTR) & I2S_HW_TX_DESC_PTR_MASK;
	value = read_register(I2S_HW_TX_DESC_PTR) & I2S_HW_TX_DESC_PTR_MASK;
	while (old != value) {
		old = value;
		value = read_register(I2S_HW_TX_DESC_PTR) & I2S_HW_TX_DESC_PTR_MASK;
		//printk("i2s reg 0x4c read not match!\n");
	}
	return value;
}

unsigned int tx_buffer_filled(void)
{
	return((kick_tx_idx - get_tx_desc_idx()) & MASK_TX_DESC_NUM);
}


#if defined(CONFIG_ARCH_AST1500_CLIENT)

#if (PATCH_CODEV3 & PATCH_AST1520A0_I2S_DUAL_OUTPUT)
/*
 * i2s_dual_output_cfg - I2S dual port output configuration
 *
 * @en: enable/disable dual output
 * @io_in_use: specify current io in use
 *
 */
void i2s_dual_output_cfg(u32 en, u32 io_in_use)
{
	if (en) {
		u32 port1_d0, port1_d1, port2_d0, port2_d1;

		/* Codec always use d0 pin as ADC, d1 pin as DAC. */
		if (IO_IN_USE_HDMI == io_in_use) {
			port1_d0 = 0; /* pin 0 from engine D0 output */
			port1_d1 = 1; /* pin 1 from engine D1 output */
			port2_d0 = 1; /* pin 0 from engine D1 output (dummy) */
			port2_d1 = 0; /* pin 1 from engine D0 output */
		} else {
			port1_d0 = 1; /* pin 0 from engine D1 output */
			port1_d1 = 0; /* pin 1 from engine D0 output (dummy) */
			port2_d0 = 0; /* pin 0 from engine D0 output (dummy) */
			port2_d1 = 1; /* pin 1 from engine D1 output */
		}

		ast_scu.scu_op(SCUOP_I2S_PIN_DUAL_OUTPUT_C, NULL);
		write_register_and(I2S_ENG_CFG, I2S20_OUT_I2SMCLK(1), I2S20_OUTPUT_I2SMCLK_MASK);
		write_register_and(I2S_ENG_CLK_CFG,
				I2S78_DUAL_PORT_OUTPUT_EN(1),
				I2S78_DUAL_PORT_OUTPUT_EN_MASK);

		write_register_and(I2S_IO_DATA_SELECT,
				I2S60_OUTPUT_DATA_SELECT_I2S_2_D3(3)
				| I2S60_OUTPUT_DATA_SELECT_I2S_2_D2(2)
				| I2S60_OUTPUT_DATA_SELECT_I2S_2_D1(port2_d1)
				| I2S60_OUTPUT_DATA_SELECT_I2S_2_D0(port2_d0)
				| I2S60_OUTPUT_DATA_SELECT_I2S_1_D3(3)
				| I2S60_OUTPUT_DATA_SELECT_I2S_1_D2(2)
				| I2S60_OUTPUT_DATA_SELECT_I2S_1_D1(port1_d1)
				| I2S60_OUTPUT_DATA_SELECT_I2S_1_D0(port1_d0),
				I2S60_OUTPUT_DATA_SELECT_MASK);
	} else {
		write_register_and(I2S_ENG_CLK_CFG,
				I2S78_DUAL_PORT_OUTPUT_EN(0),
				I2S78_DUAL_PORT_OUTPUT_EN_MASK);
		write_register_and(I2S_IO_DATA_SELECT,
				I2S60_OUTPUT_DATA_SELECT_I2S_2_D3(3)
				| I2S60_OUTPUT_DATA_SELECT_I2S_2_D2(2)
				| I2S60_OUTPUT_DATA_SELECT_I2S_2_D1(1)
				| I2S60_OUTPUT_DATA_SELECT_I2S_2_D0(0)
				| I2S60_OUTPUT_DATA_SELECT_I2S_1_D3(3)
				| I2S60_OUTPUT_DATA_SELECT_I2S_1_D2(2)
				| I2S60_OUTPUT_DATA_SELECT_I2S_1_D1(1)
				| I2S60_OUTPUT_DATA_SELECT_I2S_1_D0(0),
				I2S60_OUTPUT_DATA_SELECT_MASK);
	}
}

u32 i2s_dual_output(void)
{
	u8 ret = 0;

	if (read_register(I2S_ENG_CLK_CFG) & BIT25)
		ret = 1;

	return ret;
}
#endif /* #if (PATCH_CODEV3 & PATCH_AST1520A0_I2S_DUAL_OUTPUT) */
#endif /* #if defined(CONFIG_ARCH_AST1500_CLIENT) */



static void setup_clock(u8 SampleFrequency, unsigned int i2s_master, unsigned int output_mclk)
{
	//Disable I2S
	ast_scu.scu_op(SCUOP_I2S_STOP, NULL);
	//Setup I2S clock
	set_tx_clock(SampleFrequency);
	//Start I2S clock and enable I2S
	ast_scu.scu_op(SCUOP_I2S_START, NULL);

	write_register_and(I2S_ENG_CFG,
	(i2s_master?(I2S20_OUT_I2SCLK(1) | I2S20_OUT_I2SWS(1)):0) | (output_mclk?I2S20_OUT_I2SMCLK(1):0),
	I2S20_OUTPUT_I2SMCLK_MASK | I2S20_OUT_I2SCLK(1) | I2S20_OUT_I2SWS(1));
}

/* enable_I2SMCLK() is used to enable I2S MCLK which will be used by audio codec */
static void enable_I2SMCLK(void)
{
	setup_clock(2, 0, 1);

	/*
	** Bruce161108. NOTE
	** Calling wm8903_init() will enable WM8903 (in somehow).
	** If I2S data pin has data during WM8903 initialization, DAC will output pop-noise.
	**
	** AST152X's port2 I2S data pin is default internal pull high which means
	** port2's data pin will has data after I2S engine SCU reset.
	**
	** Workaround:
	** Set DAC data pin to 'output' mode. So that data pin will be drived to low (0).
	*/
#if ((CONFIG_AST1500_SOC_VER == 3) && (BOARD_DESIGN_VER_I2S >= 204))
	/* Codec always use d0 pin as ADC, d1 pin as DAC. */
	write_register_and(I2S_TX_CFG,
	                   I2S00_EN_D0(1)|I2S00_EN_D1(1),
	                   I2S00_EN_D0(1)|I2S00_EN_D1(1));
#endif
}

int init_audio_engine(void)
{
	int ret = 0;

	//To reset state.
	ast_scu.scu_op(SCUOP_I2S_INIT, NULL);

	/*
	** On AST1510 platform, the audio codec's master clock source comes from I2S engine's
	** MCLK. The master clock must be ready before accessing audio codec.
	*/
#if (BOARD_DESIGN_VER_I2S >= 204)
#ifdef CONFIG_ARCH_AST1500_CLIENT
	ast_scu.scu_op(SCUOP_I2S_PIN_CODEC_C, NULL);
#else
	ast_scu.scu_op(SCUOP_I2S_PIN_CODEC_H, NULL);
#endif
#endif
	enable_I2SMCLK();

	return ret;
}

static unsigned int calc_DWB(unsigned int SampleSize)
{
	unsigned int dwb = 0;

	if (SRC_WS_SIZE == 32) {
		dwb = SRC_DATA_START_FROM;
		if (!SRC_PADDING_0 && !SRC_LSB_FIRST)
			dwb += 32 + SampleSize;
		else if (SRC_PADDING_0 && SRC_LSB_FIRST)
			dwb += 32 - SampleSize;

		dwb &= 0x3F;
		if (SRC_CH0_ON_WS_HIGH)
			dwb = (dwb + 32) & 0x3F;
	} else { /* SRC_WS_SIZE == 16 bits */
		dwb = SRC_DATA_START_FROM;
		dwb &= 0x1F;
		if (SRC_CH0_ON_WS_HIGH)
			dwb = (dwb + 16) & 0x1F;		
	}
	return dwb;
}

#if TX_WS_INTERNAL
#define TX_DWB_OFFSET	(((read_register(I2S_DWB_OFFSET) & 0x3F) + 4) & 0x3F) /* I2S64[5:0] + 4 */
#else
#define TX_DWB_OFFSET   4
#endif

#if RX_WS_INTERNAL
#define RX_DWB_OFFSET	((((read_register(I2S_DWB_OFFSET) >> 16) & 0x3F) + 3) & 0x3F) /* I2S64[21:16] + 3 */
#else
#define RX_DWB_OFFSET   3
#endif


#if NEW_FREQ_DET
/* assuming frequency is detected from I2SMCLK */
static void enable_FD(void)
{
	/* work around A0's bug */
	write_register(I2S_PLL_FREQ_ADJ_PARAM, 1);
	write_register(I2S_FREQ_DETECT,
			(read_register(I2S_FREQ_DETECT) & 0x0000FFFF)
			| I2S08_MCLK_RATIO(0)
			| I2S08_EN_FD_MIN(1) | I2S08_EN_FD_MAX(1)
			| I2S08_FD_SRC(FREQ_DET_SRC_SEL)
			| I2S08_FD_STABLE_CNT(0));
}
#endif

#if defined(CONFIG_ARCH_AST1500_CLIENT)

/*
 * un-define AUDIO_CTS_PPM,
 * we do not take the result of audio N/CTS, the reason is:
 * 1. expected CTS is uncertain by different TMDS
 * from HDMI Spec1.4b Table 7-1~7-3, some 59/60 cases may have different recommended N and expected CTS value.
 * but it is not mandatory (780A didn't follow the value listed in table)
 * without exactly CTS, we cannot get ppm correctly.
 * 2. audio free routing issue
 * N/CTS calculation must get done at host.
 * we'll have some problmes with getting N/CTS (must waiting for video, or cannot get N/CTS if video/audio is not together)
 *
 * alternative plan:
 * there is a clock accuracy case in HDMI CTS 1.4a, TestID 7-29
 * the pass conditon is +/-50ppm or +/-100ppm (by channel status)
 * we think audio clock should get limited between +/-100ppm
 */
#undef AUDIO_CTS_PPM
#if defined(AUDIO_CTS_PPM)
static u32 tmds_clock_khz(void)
{
	struct s_crt_drv *crt;
	struct s_crt_info *info;
	MODE_ITEM *mode;
	u32 clk_khz;

	crt = crt_connect(OWNER_NONE);
	info = &crt->VE1_mode;

	mode = crt_timing_by_index(info->TimingTableIdx);

	clk_khz = mode->DCLK10000 / 10;

	switch (info->color_depth) {
	case 30:
		clk_khz = clk_khz + (clk_khz >> 2); /* clk_khz * 1.25 */
		break;
	case 36:
		clk_khz = clk_khz + (clk_khz >> 1); /* clk_khz * 1.5 */
		break;
	case 48:
		clk_khz = clk_khz << 1; /* clk_khz * 2 */
		break;
	default:
		break;
	}

	return clk_khz;
}

struct hdmi_n_cts {
	u32 n;
	u32 cts;
};

struct hdmi_n_cts fs_32k[] = {
	{4576,  28125},
	{5824, 421875},
	{3072, 222750},
};

struct hdmi_n_cts fs_44d1k[] = {
	{ 7007, 31250},
	{17836, 234375},
	{8918, 234375},
	{6272, 165000},
	{4459, 234375},
	{4704, 247500},
};

struct hdmi_n_cts fs_48k[] = {
	{6864, 28125},
	{11648, 140625},
	{5120, 247500}
};

static int _cts_ppm(u32 fs, u32 n, u32 cts)
{
	u32 cts_spec;
	u32 shift = 0;
	u32 cts_spec_from_clk = 1;
	int diff, ppm;

	if (cts == 0) {
		/* return 0 if cts from receiver is non-referential */
		ppm = 0;
		goto exit;
	}

	cts_spec = cts;

	/* 128 * fs = TMDS * N / CTS */
	switch (fs) {
	case I2S_SAMPLE_RATE_32K:
		if (n == 11648) {
			/*
			 * 72.25/1.001 => 210937
			 * 148.5/1.001 => 421875
			 */
			cts_spec = 210937;

			if (tmds_clock_khz() > 75000)
				cts_spec = 421875;
		} else {
			int i, size;
			struct hdmi_n_cts *table;

			table = fs_32k;
			size = ARRAY_SIZE(fs_32k);

			for (i = 0; i < size; i++) {
				if (n == table[i].n) {
					cts_spec = table[i].cts;
					cts_spec_from_clk = 0;
					break;
				}
			}

			if (i == size) {
				/*
				 * 4096 = 128 * 32 / 1000
				 * CTS should be TMDS / 1000
				 */
				cts_spec = tmds_clock_khz();
			}

		}
		break;
	case I2S_SAMPLE_RATE_44d1K:
	case I2S_SAMPLE_RATE_88d2K:
	case I2S_SAMPLE_RATE_176d4K:
		{
			int i, size;
			struct hdmi_n_cts *table;

			table = fs_44d1k;
			size = ARRAY_SIZE(fs_44d1k);

			if (fs == I2S_SAMPLE_RATE_88d2K)
				shift = 1;
			else if (fs == I2S_SAMPLE_RATE_176d4K)
				shift = 2;

			for (i = 0; i < size; i++) {
				if (n == table[i].n << shift) {
					cts_spec = table[i].cts;
					cts_spec_from_clk = 0;
					break;
				}
			}

			if (i == size) {
				/* TBD
				 * 6272 = 128*44.1 * 10 / 9 / 1000
				 * CTS should be (TMDS / 1000) * 10 / 9
				 */
				cts_spec = tmds_clock_khz() * 10 / 9;
			}
		}
		break;
	case I2S_SAMPLE_RATE_48K:
	case I2S_SAMPLE_RATE_96K:
	case I2S_SAMPLE_RATE_192K:
		{
			int i, size;
			struct hdmi_n_cts *table;

			table = fs_48k;
			size = ARRAY_SIZE(fs_48k);
			if (fs == I2S_SAMPLE_RATE_96K)
				shift = 1;
			else if (fs == I2S_SAMPLE_RATE_192K)
				shift = 2;

			if (n == (5824 << shift)) {
				/*
				 * 148.5/1.001 => 140625
				 * 297/1.001 => 281250
				 */
				cts_spec = 140625;
				if (tmds_clock_khz() > 160000)
					cts_spec = 281250;
			} else {
				for (i = 0; i < size; i++) {
					if (n == table[i].n << shift) {
						cts_spec = table[i].cts;
						cts_spec_from_clk = 0;
						break;
					}
				}

				if (i == size) {
					/*
					 * 6144 = 128 * 48 / 1000
					 * CTS should be TMDS /1000
					 */
					cts_spec = tmds_clock_khz();
				}
			}
		}
		break;
	default:
		cts_spec = cts;
	}

	/* calculate PPM of CTS */
	diff = (int) cts - (int) cts_spec;

	if (abs(diff) > 4000) {
		/*
		 * may overflow when diff * 1000 * 1000
		 * give up and let ppm be 0
		 */
		ppm = 0;
		goto exit;
	}

	ppm = diff * 1000 * 1000 / (int) cts_spec;

	if (cts_spec_from_clk == 1) {
		/*
		 * cts_spec is decided by tmds clock
		 * we may get ~1000PPM difference when 59.94/60 case
		 *
		 * following is to fix PPM when ABS(PPM) > 500
		 * we choose the one close to our result
		 */
		if (abs(ppm) > 500) {
			if (ppm > 0)
				ppm -= 1000;
			else
				ppm += 1000;
		}
	}

	/* use 0 if final result still > 1000ppm */
	if (abs(ppm) > 1000)
		ppm = 0;
exit:
	return ppm;
}

static int cts_ppm(Audio_Info *info)
{
	if (info->io_in_use == IO_IN_USE_CODEC)
		return 0;

	if (clock_lock_mode() & CLK_LOCK_MODE_PPM_FROM_CTS_OFF)
		return 0;
	/*
	 * 128 * Fs = TMDS * N / CTS, suppose TMDS is ideal
	 * Fs = A / CTS  (A =  TMDS * N / 128)
	 * CTS+ => FS- ; CTS- => Fs+
	 * the difference of Fs will be (-1) * hdmi_cts_ppm
	 */

	return (-1) * _cts_ppm(info->SampleFrequency, info->hdmi_n, info->hdmi_cts);
}

#endif /* #if defined(AUDIO_CTS_PPM) */

static int adjust_ppm(Audio_Info *info)
{
	int ret = 0;

#if defined(AUDIO_CTS_PPM)
	ret += cts_ppm(info);
#endif

	return ret;
}
#endif /* #if defined(CONFIG_ARCH_AST1500_CLIENT) */

//to do: support SPDIF
void setup_audio_engine(Audio_Info *audio_info, Audio_Buf *rx_buffer, Audio_Buf *tx_buffer, unsigned int i2s_master, unsigned int output_mclk)
{
	volatile unsigned int i;
	u8 SampleSize = audio_info->SampleSize;
	u8 SampleFrequency = audio_info->SampleFrequency;
	u8 ValidCh = audio_info->ValidCh;
	u8 bSPDIF = 0;
//	unsigned int is_hbr = 0;
	u32 down_sampling = 0;
	u32 mode_16bits = 0;
	u32 pll_fac_enable = 0; /* pll frequency adjustment control */
#if defined(CONFIG_ARCH_AST1500_CLIENT)
	int ppm_fix;
#endif

	if (!rx_buffer && !tx_buffer) {
		printk("setup_audio_engine for nothing!?\n");
		return;
	}

#if NEW_FREQ_ADJ && !defined(I2SPLL_EnParameter_1)
	if (CLK_LOCK_MODE_SELECT_GET(clock_lock_mode()) != CLK_LOCK_MODE_OFF)
		pll_fac_enable = 1;
#endif

	if (audio_info->Audio_Type & AST_AUDIO_HBR) {
		printk("HBR\n");
//		is_hbr = 1;
	} else if (audio_info->Audio_Type & AST_AUDIO_NLPCM) {
		printk("NLPCM\n");
#ifndef NLPCM_THROUGH_I2S
		bSPDIF = 1;
#endif
	} else {
		printk("LPCM\n");
		if (audio_info->Mode16bits) {
			mode_16bits = 1;
			SampleSize = 16;
		}

		if (audio_info->DownSampling)
			down_sampling = 1;
	}

	printk("ValidCh = %d\n", ValidCh);
	printk("SampleSize = %d\n", SampleSize);
	printk("SampleFrequency = %d\n", SampleFrequency);

	if (rx_buffer && tx_buffer) {
		if (ValidCh != 2) {
			printk("Only 2-CH audio is available when audio is bidirectional.\n");
			BUG();
		}

		if (bSPDIF) {
			printk("Bidirectional audio is not supported with SPDIF.\n");
			BUG();
		}
	}

	//Setup pin mux
	if (audio_info->io_in_use == IO_IN_USE_CODEC) {
#if defined(CONFIG_ARCH_AST1500_CLIENT)
		ast_scu.scu_op(SCUOP_I2S_PIN_CODEC_C, (void *)(int) audio_info->enable_upstream);
#else
		ast_scu.scu_op(SCUOP_I2S_PIN_CODEC_H, (void *)(int) audio_info->enable_upstream);
#endif
	} else {
#if defined(CONFIG_ARCH_AST1500_CLIENT)
		ast_scu.scu_op(SCUOP_I2S_PIN_HDMI_C, NULL);
#else
		ast_scu.scu_op(SCUOP_I2S_PIN_HDMI_H, NULL);
#endif
	}

	setup_clock(SampleFrequency, i2s_master, output_mclk);

	i2s_des_vld  = 0;

	/* Clear Interrupt status */
	write_register(I2S_INT_STAT, 0xFFFFFFFF);
	wmb();
#if NEW_FREQ_DET
	if (rx_buffer && !tx_buffer)
		enable_FD();
#endif
#if defined(CONFIG_ARCH_AST1500_CLIENT)
	ppm_fix = adjust_ppm(audio_info);
#if defined(I2S_CLOCK_ADJUST)
	if (tx_buffer)
		i2s_pll_adjust_init(tx_buffer->buf_num, ppm_fix);
#endif
#else
	if (rx_buffer)
		i2s_pll_adjust_init(rx_buffer->buf_num, 0);
	else
		i2s_pll_adjust_init(tx_buffer->buf_num, 0);
#endif
	wmb();

	if (rx_buffer && tx_buffer) {
		write_register_and(I2S_ENG_CLK_CFG,
			I2S78_FIFO_CTRL(1),
			I2S78_FIFO_CTRL_MASK);
		wmb();
		write_register(I2S_DMA_TH, I2S10_RX_TH(0xF) | I2S10_TX_TH(0xF));
		wmb();

		write_register(I2S_ENG_CFG,
			I2S20_EN_TX(0) |
			I2S20_EN_RX(0) |
			I2S20_OUT_I2SCLK((i2s_master)?(1):(0)) |
#ifndef AST1510_I2S_A0
			/* useful only after A1 */
			I2S20_OUT_I2SMCLK((output_mclk)?(1):(0)) |
#endif
			I2S20_OUT_I2SWS(1) |
			I2S20_32BITS_TXWS((SRC_WS_SIZE == 16)?(0):(1)) |
			I2S20_32BITS_RXWS((SRC_WS_SIZE == 16)?(0):(1)) |
			I2S20_I2STXWS_FALLING_EDGE(1) |
			I2S20_I2SRXWS_FALLING_EDGE(1) |
			I2S20_DATA_FALLING_EDGE(1) |
			I2S20_SPDIF_MODE(0) |
			I2S20_EN_FAC_INC(pll_fac_enable)
			);
	} else if (rx_buffer) {
		write_register_and(I2S_ENG_CLK_CFG,
			I2S78_FIFO_CTRL(0),
			I2S78_FIFO_CTRL_MASK);
		wmb();
		write_register(I2S_DMA_TH, I2S10_RX_TH(0x1F) | I2S10_TX_TH(0));
		wmb();

		if (bSPDIF) {
			write_register(I2S_ENG_CFG,
				I2S20_EN_TX(0) |
				I2S20_EN_RX(0) |
				I2S20_OUT_I2SCLK(0) |
#ifndef AST1510_I2S_A0
				I2S20_OUT_I2SMCLK(0) | /* useful only after A1 */
#endif
				I2S20_32BITS_RXWS(1) | /* SPDIF MUST set this bit to 1 */
				I2S20_SPDIF_MODE(1) |
				I2S20_20BITS_SPDIF(0) |
				I2S20_OUT_SPDIF(0) |
				I2S20_EN_FAC_INC(pll_fac_enable)
				);
		} else {
			write_register(I2S_ENG_CFG,
				I2S20_EN_TX(0) |
				I2S20_EN_RX(0) |
				I2S20_OUT_I2SCLK((i2s_master)?(1):(0)) |
#ifndef AST1510_I2S_A0
				I2S20_OUT_I2SMCLK((output_mclk)?(1):(0)) |
#endif
				I2S20_OUT_I2SWS((i2s_master)?(1):(0)) |
				I2S20_32BITS_RXWS((SRC_WS_SIZE == 16)?(0):(1)) |
				I2S20_I2SRXWS_FALLING_EDGE((i2s_master)?(1):(0)) |
				I2S20_DATA_FALLING_EDGE((i2s_master)?(1):(0)) |
				I2S20_SPDIF_MODE(0) |
				I2S20_EN_FAC_INC(pll_fac_enable)
				);
		}
	} else {
		write_register_and(I2S_ENG_CLK_CFG,
			I2S78_FIFO_CTRL(3),
			I2S78_FIFO_CTRL_MASK);
		wmb();
		write_register(I2S_DMA_TH, I2S10_RX_TH(0x0) | I2S10_TX_TH(0x1F));
		wmb();

		if (bSPDIF) {
			write_register(I2S_ENG_CFG,
				I2S20_EN_TX(0) |
				I2S20_EN_RX(0) |
				I2S20_OUT_I2SCLK(1) |
#ifndef AST1510_I2S_A0
				I2S20_OUT_I2SMCLK(1) | /* useful only after A1 */
#endif
				I2S20_32BITS_TXWS(1) | /* SPDIF MUST set this bit to 1 */
				I2S20_SPDIF_MODE(1) |
				I2S20_20BITS_SPDIF(0) |
				I2S20_OUT_SPDIF(1) |
				I2S20_EN_FAC_INC(pll_fac_enable)
				);
		} else {
			write_register(I2S_ENG_CFG,
				I2S20_EN_TX(0) |
				I2S20_EN_RX(0) |
				I2S20_OUT_I2SCLK((i2s_master)?(1):(0)) |
#ifndef AST1510_I2S_A0
				I2S20_OUT_I2SMCLK((output_mclk)?(1):(0)) |
#endif
				I2S20_OUT_I2SWS(1) |
				I2S20_32BITS_TXWS((SRC_WS_SIZE == 16)?(0):(1)) |
				I2S20_I2STXWS_FALLING_EDGE(1) |
				I2S20_DATA_FALLING_EDGE(1) |
				I2S20_SPDIF_MODE(0) |
				//I2S20_EN_FAC(1) |
			  //I2S20_I2S_TRANSMIT_LDS(1) |
			  //this bit enables frequency adjustment
				I2S20_EN_FAC_INC(pll_fac_enable)
				);
		}
	}

	if (rx_buffer) {
		rx_desc = rx_buffer->desc_va;
		rx_buf_size = rx_buffer->buf_size;
		rx_buf_num = rx_buffer->buf_num;
		for (i = 0; i < rx_buf_num; i++) {
			rx_desc[i].base_address = rx_buffer->buf_pa[i];
			rx_desc[i].owner_bit = ENGINE_OWN;
			rx_desc[i].end_bit = 0;
			rx_desc[i].length = rx_buf_size;
		}
		rx_desc[rx_buf_num - 1].end_bit = 1;
		rx_idx = 0;
		rx_free = rx_buf_num;
		//fill in rx descriptor base register
		write_register (I2S_RX_DESC_BASE, rx_buffer->desc_pa);
		if (tx_buffer) {
			write_register (I2S_RX_CFG,
				I2S04_EN_D0(1) |
				I2S04_EN_D1(0) |
				I2S04_EN_D2(0) |
				I2S04_EN_D3(0) |
				I2S04_DWB(calc_DWB(SampleSize)) | /* Ask Jazoe for this. 49 for 16bits, 57 for 24bits.*/
				I2S04_INTERNAL_WS(RX_WS_INTERNAL) |
//				I2S04_MULTI_CHANNEL_SYNC_EN((is_hbr)?(1):(0)) |
				I2S04_MULTI_CHANNEL_SYNC_EN(1) |
				I2S04_VALID_BITS(SampleSize) |
				I2S04_EN_16BITS((mode_16bits)?(1):(0)) |
				I2S04_LSB_FIRST(SRC_LSB_FIRST) |
				I2S04_DWB_OFFSET(RX_DWB_OFFSET) |
				I2S04_EN_DOWNSAMPLE(down_sampling) |
				I2S04_DISABLE_SPDIF_ERR(0)
				);
		} else {
			if (bSPDIF) {
				write_register (I2S_RX_CFG, 
					I2S04_EN_D0(0) |
					I2S04_EN_D1(0) |
					I2S04_EN_D2(0) |
					I2S04_EN_D3(1) |
					I2S04_VALID_BITS(32) |
					I2S04_EN_16BITS(0) |
					I2S04_LSB_FIRST(1) |
					I2S04_DISABLE_SPDIF_ERR(0)
					);
				write_register(I2S_FREQ_DETECT,
					I2S08_SPDIF_NE3(1) |
					I2S08_SPDIF_PE3(1) |
					I2S08_MCLK_RATIO(0)
					);

				write_register(I2S_SPDIF_EDGE_MASK,
					I2S0C_SPDIF_NE1(1) |
					I2S0C_SPDIF_PE1(1) |
					I2S0C_SPDIF_NE2(1) |
					I2S0C_SPDIF_PE2(1)
					);
			} else {
				write_register (I2S_RX_CFG,
					I2S04_EN_D0((ValidCh >= 2)?(1):(0)) |
					I2S04_EN_D1((ValidCh >= 4)?(1):(0)) |
					I2S04_EN_D2((ValidCh >= 6)?(1):(0)) |
					I2S04_EN_D3((ValidCh >= 8)?(1):(0)) |
					I2S04_DWB(calc_DWB(SampleSize)) | /* Ask Jazoe for this. 49 for 16bits, 57 for 24bits.*/
					I2S04_INTERNAL_WS(RX_WS_INTERNAL) |
	//				I2S04_MULTI_CHANNEL_SYNC_EN((is_hbr)?(1):(0)) |
					I2S04_MULTI_CHANNEL_SYNC_EN(1) |
					I2S04_VALID_BITS(SampleSize) |
					I2S04_EN_16BITS((mode_16bits)?(1):(0)) |
					I2S04_LSB_FIRST(SRC_LSB_FIRST) |
					I2S04_DWB_OFFSET(RX_DWB_OFFSET) |
					I2S04_EN_DOWNSAMPLE(down_sampling) |
					I2S04_DISABLE_SPDIF_ERR(0)
					);
			}
		}
	}

	if (tx_buffer) {
		tx_desc = tx_buffer->desc_va;
		tx_buf_size = tx_buffer->buf_size;
		tx_buf_num = tx_buffer->buf_num;
		for (i = 0; i < tx_buf_num; i++) {
			tx_desc[i].base_address = tx_buffer->buf_pa[i];
			tx_desc[i].owner_bit = SW_OWN;
			tx_desc[i].end_bit = 0;
			tx_desc[i].length = tx_buf_size;
		}
		tx_desc[tx_buf_num - 1].end_bit = 1;
		//fill in tx descriptor base register
		write_register (I2S_TX_DESC_BASE, tx_buffer->desc_pa);
		if (rx_buffer) {
			write_register(I2S_TX_CFG,
				I2S00_EN_D0(0) |
				I2S00_EN_D1(1) |
				I2S00_EN_D2(0) |
				I2S00_EN_D3(0) |
				I2S00_DWB(calc_DWB(SampleSize)) | //From Jazoe
				I2S00_INTERNAL_WS(TX_WS_INTERNAL) | //Feedback from I2SWS IO
//				I2S00_MULTI_CHANNEL_SYNC_EN((is_hbr)?(1):(0)) |
				I2S00_MULTI_CHANNEL_SYNC_EN(((ast_scu.ability.soc_op_mode == 1)?(0):(1))) |
				I2S00_VALID_BITS(SampleSize) |
				I2S00_EN_16BITS((mode_16bits)?(1):(0)) |
				I2S00_LSB_FIRST(SRC_LSB_FIRST) |
				I2S00_EN_UPSAMPLE(down_sampling) |
				I2S00_DWB_OFFSET(TX_DWB_OFFSET)
				);
		} else {
			if (bSPDIF) {
				write_register(I2S_TX_CFG,
					I2S00_EN_D0(0) |
					I2S00_EN_D1(0) |
					I2S00_EN_D2(0) |
					I2S00_EN_D3(1) |
					I2S00_VALID_BITS(32) |
					I2S00_EN_16BITS(0) |
					I2S00_LSB_FIRST(1) |
					I2S00_DISABLE_SPDIF_ERR(0)
					);

				write_register(I2S_FREQ_DETECT,
					I2S08_SPDIF_NE3(1) |
					I2S08_SPDIF_PE3(1) |
					I2S08_MCLK_RATIO(0)
					);

				write_register(I2S_SPDIF_EDGE_MASK,
					I2S0C_SPDIF_NE1(1) |
					I2S0C_SPDIF_PE1(1) |
					I2S0C_SPDIF_NE2(1) |
					I2S0C_SPDIF_PE2(1)
					);
			} else {
				if (audio_info->io_in_use == IO_IN_USE_CODEC) {
					write_register(I2S_TX_CFG,
						I2S00_EN_D0(0) |
						I2S00_EN_D1(1) |
						I2S00_EN_D2(0) |
						I2S00_EN_D3(0) |
						I2S00_DWB(calc_DWB(SampleSize)) | //From Jazoe
						I2S00_INTERNAL_WS(TX_WS_INTERNAL) | //Feedback from I2SWS IO
		//				I2S00_MULTI_CHANNEL_SYNC_EN((is_hbr)?(1):(0)) |
						I2S00_MULTI_CHANNEL_SYNC_EN(((ast_scu.ability.soc_op_mode == 1)?(0):(1))) |
						I2S00_VALID_BITS(SampleSize) |
						I2S00_EN_16BITS((mode_16bits)?(1):(0)) |
						I2S00_LSB_FIRST(SRC_LSB_FIRST) |
						I2S00_EN_UPSAMPLE(down_sampling) |
						I2S00_DWB_OFFSET(TX_DWB_OFFSET)
						);
				} else {
					write_register(I2S_TX_CFG,
						I2S00_EN_D0((ValidCh >= 2)?(1):(0)) |
						I2S00_EN_D1((ValidCh >= 4)?(1):(0)) |
						I2S00_EN_D2((ValidCh >= 6)?(1):(0)) |
						I2S00_EN_D3((ValidCh >= 8)?(1):(0)) |
						I2S00_DWB(calc_DWB(SampleSize)) | //From Jazoe
						I2S00_INTERNAL_WS(TX_WS_INTERNAL) | //Feedback from I2SWS IO
		//				I2S00_MULTI_CHANNEL_SYNC_EN((is_hbr)?(1):(0)) |
						I2S00_MULTI_CHANNEL_SYNC_EN(((ast_scu.ability.soc_op_mode == 1)?(0):(1))) |
						I2S00_VALID_BITS(SampleSize) |
						I2S00_EN_16BITS((mode_16bits)?(1):(0)) |
						I2S00_LSB_FIRST(SRC_LSB_FIRST) |
						I2S00_EN_UPSAMPLE(down_sampling) |
						I2S00_DWB_OFFSET(TX_DWB_OFFSET)
						);
				}
			}
		}
	}

	wmb();

	if (rx_buffer)
		write_register_or(I2S_ENG_CFG, I2S20_EN_RX(1));
	if (tx_buffer)
		write_register_or(I2S_ENG_CFG, I2S20_EN_TX(1));
	
	wmb();
	if (rx_buffer)
		rx_ready = 1;
	if (tx_buffer) {
		tx_ready = 1;

#if defined(CONFIG_ARCH_AST1500_CLIENT) && defined(I2S_CLOCK_ADJUST_IN_THREAD)
		wake_up_interruptible(&clock_adj_wait_queue);
#endif
	}
}

inline void kick_rx_desc(void)
{
	write_register(I2S_RX_DESC_POLL, 1);
	barrier();
}

void enable_rx(void)
{
	kick_rx_desc();
	
	wmb();
	/* enable rx interrupt */
#if NEW_FREQ_DET
	write_register(I2S_INT_EN, INT_FD_MASK | INT_RX_DONE_MASK);
#else
	write_register(I2S_INT_EN, INT_RX_DONE_MASK);
#endif
	rx_enabled = 1;
	printk("audio Rx enabled\n");
}

inline void collect_rx_data(unsigned int max_pkt_num, unsigned int *pkt_num, unsigned int *start_idx)
{
	volatile RX_DESC *cur_desc;

	*start_idx = rx_idx;
	cur_desc = &rx_desc[rx_idx];

	while ((cur_desc->owner_bit == SW_OWN) && (rx_free > 0)) {
		(*pkt_num)++;
		if (!cur_desc->end_bit)
			rx_idx++;
		else
			rx_idx = 0;
		cur_desc = &rx_desc[rx_idx];
		rx_free--;
		if ((max_pkt_num) && (*pkt_num == max_pkt_num))
			break;
	}
}

inline void reclaim_rx_desc(unsigned int start_idx)
{
	unsigned int idx = start_idx;
	volatile RX_DESC *cur_desc;

	while (rx_free < rx_buf_num) {
		cur_desc = &rx_desc[idx];
		cur_desc->length = rx_buf_size;
		cur_desc->owner_bit = ENGINE_OWN;
		if (!cur_desc->end_bit)
			idx++;
		else
			idx = 0;
		rx_free++;
	}
	kick_rx_desc();
}

inline unsigned int poll_rx_data(void)
{
	volatile RX_DESC *cur_desc;

	cur_desc = &rx_desc[rx_idx];
	if (cur_desc->owner_bit == SW_OWN)
		return 1;
	else
		return 0;
}

#if defined(I2S_DATA_DELAY)
/*
 * expect at least 16 ms, 12 ~= 16/1.33, extend to 24 to adapt network variation
 * test result shows that 24 is not enough to adapt the variation caused by host heavy loading
 * change from 24 to 40 for further test
 * when OP mode = 1, need ~5.33 ms to accumulate one buffer,
 * 16/5.33 ~= 3, extend to 6 to adapt network variation
 *
 * test result with I2S_CLOCK_ADJUST_NEW shows that 24 is not enough to adapt the variation caused by host heavy loading
 * change from 24 to 40 for further test
 * when OP mode = 1, extend to 10
 *
 * under heavy load, host may not be able to send audio data immediately for a time
 * so extend TX_QUEUE_LEN_MIN_NEW from 40 to 60 to avoid that client run ot out audio data
 *
 */
#if defined(I2S_CLOCK_ADJUST_NEW)
#define TX_QUEUE_LEN_MIN_NEW 60
#endif

#define TX_QUEUE_LEN_MIN 24
#define TX_QUEUE_LEN_MIN_OP1 6

static u32 _tx_queue_len = TX_QUEUE_LEN_MIN;

void tx_queue_len_cfg(u32 cfg, unsigned int max_buf_num, unsigned int min_buf_num)
{
	u32 len_min;

	if (cfg > max_buf_num)
		_tx_queue_len = max_buf_num;
	else if (cfg < min_buf_num)
		_tx_queue_len = min_buf_num;
	else
		_tx_queue_len = cfg;

	if (ast_scu.ability.soc_op_mode == 1)
		len_min = TX_QUEUE_LEN_MIN_OP1;
	else
		len_min = TX_QUEUE_LEN_MIN;

#if defined(I2S_CLOCK_ADJUST_NEW)
	if (ast_scu.ability.soc_op_mode >= 3)
		len_min = TX_QUEUE_LEN_MIN_NEW;
#endif

	if (len_min > _tx_queue_len)
		_tx_queue_len = len_min;
}

u32 tx_queue_len(void)
{
	return _tx_queue_len;
}
#endif

#ifdef I2SPLL_EnStage_3
#define I2SPLL_Thres_S3Init    (tx_buf_num >> 1)
#define I2SPLL_Thres_S3N2L     (tx_buf_num >> 2)
#define I2SPLL_Thres_S3L2N     (tx_buf_num / 3)
#define I2SPLL_Thres_S3N2H     ((tx_buf_num * 3) >> 2)
#define I2SPLL_Thres_S3H2N     ((tx_buf_num << 1) / 3)
#else
#define I2SPLL_Thres_S2Init    (tx_buf_num / 2)
#define I2SPLL_Thres_S2N2L     (tx_buf_num / 3)
#define I2SPLL_Thres_S2L2N     ((tx_buf_num / 3) * 2)
#endif

u32 tx_kick_th(void)
{
#if defined(CONFIG_ARCH_AST1500_CLIENT)
#if (3 <= CONFIG_AST1500_SOC_VER)
	return tx_queue_len();
#else
#ifdef I2SPLL_EnStage_3
	return I2SPLL_Thres_S3Init;
#else
	return I2SPLL_Thres_S2Init;
#endif
#endif
#else
	return tx_buf_num >> 1;
#endif
}

#if defined(I2S_CLOCK_ADJUST) && !defined(I2S_CLOCK_ADJUST_IN_THREAD)
#if defined(I2S_CLOCK_ADJUST_NEW)
#define I2S_CLK_ADJUST_PERIOD_MS_NEW 10
#endif
#define I2S_CLK_ADJUST_PERIOD_MS 3
static unsigned long timeout = 0;
#endif

#if defined(I2S_SYNC_CHECK_NET_RX)
static int tx_data_check_net_rx;
static int tx_data_check_net_rx_dropped;
static int tx_data_check_net_rx_ok;

#define NET_RX_CHECK_TH (512) /* sometimes got accumulation after 256 stable buffer, so use 512 */

void net_rx_check_start(int type)
{
	tx_data_check_net_rx = type; tx_data_check_net_rx_dropped = 0; tx_data_check_net_rx_ok = 0;
}

int net_rx_check(u32 pkt_num)
{
	int rx_check_th = NET_RX_CHECK_TH;

	if ((ast_scu.ability.soc_op_mode >= 3) && tx_data_check_net_rx) {
		if (tx_data_check_net_rx == 1) {
			if (pkt_num > 1) {
				tx_data_check_net_rx_dropped += pkt_num;
				tx_data_check_net_rx_ok = 0;
				return -1;
			}

			tx_data_check_net_rx_ok++;

			if (tx_data_check_net_rx_ok < rx_check_th)
				goto drop;

			tx_data_check_net_rx = 0;
		} else {
			/*
			 * Tx is empty case
			 * we woulid like to recovery audio ASAP if tx queue got empty when system is under heavy load
			 * (for example: plug/unplug USB devices)
			 *
			 * drop audio data only after audio stream stable and tx queue length >  tx_kick_th()
			 */
			if (tx_data_check_net_rx_ok >= rx_check_th) {
				if (tx_buffer_filled() <= (tx_kick_th() + 2)) {
					tx_data_check_net_rx = 0;
#if defined(I2S_CLOCK_ADJUST_NEW)
					i2s_gen_lock_restart();
#endif
				} else {
					goto drop;
				}
			}

			tx_data_check_net_rx_ok += pkt_num;
		}
	}

	return 0;
drop:
	tx_data_check_net_rx_dropped++;
	return -1;
}
#endif /* #if defined(I2S_SYNC_CHECK_NET_RX) */

void kick_tx_desc(unsigned int idx)
{
	volatile TX_DESC *cur_desc;
#if defined(I2S_CLOCK_ADJUST) && !defined(I2S_CLOCK_ADJUST_IN_THREAD)
	unsigned int clk_adjust_period_in_ms = I2S_CLK_ADJUST_PERIOD_MS;

#if defined(I2S_CLOCK_ADJUST_NEW)
	if (ast_scu.ability.soc_op_mode >= 3)
		clk_adjust_period_in_ms = I2S_CLK_ADJUST_PERIOD_MS_NEW;
#endif
#endif /* #if defined(I2S_CLOCK_ADJUST) && !defined(I2S_CLOCK_ADJUST_IN_THREAD) */

	while (kick_tx_idx != idx) {
		cur_desc = &tx_desc[kick_tx_idx];
		cur_desc->length = tx_buf_size;
		cur_desc->owner_bit = ENGINE_OWN;
		if (cur_desc->end_bit)
			kick_tx_idx = 0;
		else
			kick_tx_idx++;
	}

	if (read_register(I2S_INT_STAT) & BIT5) {
		printk("E\n");
		write_register(I2S_INT_STAT, BIT5);
		i2s_des_vld  = 0;
#if defined(CONFIG_ARCH_AST1500_CLIENT) && defined(I2S_CLOCK_ADJUST)
		/* Bruce150302. Fix (from my understanding)
		** set_stage_N() should be called only when clock_adj is enabled.
		** Which implies tx_ready is true. (NOT rx_ready).
		*/
#if defined(I2S_CLOCK_ADJUST_NEW)
		i2s_gen_lock_stop();
#endif
#if defined(I2S_SYNC_CHECK_NET_RX)
		net_rx_check_start(2);
#endif
		if (tx_ready) {
			/*
			 * check if we need to adjust audio central frequency
			 * set_stage_reset() is included in NOT adjustment case of clock_fixup_tx_empty()
			 */
			clock_fixup_tx_empty();
		}
#endif
	}

	if (!i2s_des_vld) {
		if (tx_buffer_filled() > tx_kick_th()) {
			i2s_des_vld = 1;
#if defined(I2S_CLOCK_ADJUST) && !defined(I2S_CLOCK_ADJUST_IN_THREAD)
			timeout = jiffies + msecs_to_jiffies(clk_adjust_period_in_ms);
#endif
		}
	}

	mb();

	if (i2s_des_vld) {
		write_register(I2S_TX_DESC_POLL, 1);
#if defined(I2S_CLOCK_ADJUST) && !defined(I2S_CLOCK_ADJUST_IN_THREAD)
		if (time_after(jiffies, timeout)) {
			timeout = jiffies + msecs_to_jiffies(clk_adjust_period_in_ms);
			clk_adj_proc(tx_buffer_filled());
		}
#endif
	}
}

void stop_audio_engine(void)
{
	rx_enabled = rx_ready = 0;
	rx_data_available = 0;
	tx_ready = 0;
	ast_scu.scu_op(SCUOP_I2S_STOP, NULL);
	kick_tx_idx = 0;
}

inline int audio_int_handler(int *rx_int)
{
	volatile unsigned int status;

	*rx_int = 0;

	status = read_register(I2S_INT_STAT)/* & int_active*/;
#if NEW_FREQ_DET
	if (status & INT_FD_MASK) {
		unsigned int temp, vmax, vmin, cmax, cmin;
		printk("INT_FD\n");
		//read statistics
		temp = read_register(I2S_FREQ_DETECT_STAT_MAX);
		vmax = (temp >> 16) & 0xFFF;
		cmax = temp & 0xFFF;
		temp = read_register(I2S_FREQ_DETECT_STAT_MIN);
		vmin = (temp >> 16) & 0xFFF;
		cmin = temp & 0xFFF;
		printk("Vmax=%d,Vmin=%d\n", vmax, vmin);
		printk("Cmax=%d,Cmin=%d\n", cmax, cmin)

		if (vmax < vmin)
			printk("Max (%d) < Min (%d)!!!\n", vmax, vmin);
		else if (vmin == (0xFFF - 1))
			printk("mute???\n");
		else { /* look up frequency table */
			unsigned int sum, i, diff = 0, index = 0;
			sum = vmax + vmin + 2;//2 is the difference from ideal value by A0 experience
			printk("sum=%d\n", sum);
			for (i = 0; i < I2S_Table_Count; i++) {
				printk("sum[%d]=%d\n", i, Freq_Det_Table[i].sum);
				if (sum >= Freq_Det_Table[i].sum) {
					if (i == 0)
						index = 0;
					else {
						if ((sum - Freq_Det_Table[i].sum) <= diff)
							index = i;
						else
							index = i - 1;
					}
					break;
				} else {
					diff = Freq_Det_Table[i].sum - sum;
				}
			}

			if (i == I2S_Table_Count)
				index = I2S_Table_Count - 1;
			printk("fs = %d(%s)\n", Freq_Det_Table[index].sample_rate_index, Freq_Det_Table[index].sample_rate);
		}
	}
#endif

	write_register (I2S_INT_STAT, status);

	if (status) {
#if 0
		//check Rx buffer underrun
		if (status & (INT_RX_OVERFLOW_MASK | INT_RX_EMPTY_MASK))
		{
			printk("INT:%08X\n", status);
		}
#endif
		if (status & INT_RX_DONE_MASK)
		{
			*rx_int = 1;
		}
		return 1;
	}

	return 0;
}

void disable_rx_int(void)
{
	write_register(I2S_INT_EN, read_register(I2S_INT_EN) & (~INT_RX_DONE_MASK));
}

void enable_rx_int(void)
{
	write_register(I2S_INT_STAT, INT_RX_DONE_MASK);
	write_register(I2S_INT_EN, read_register(I2S_INT_EN) | INT_RX_DONE_MASK);
}

unsigned int do_tx_buf_overflow_patch(unsigned int reset_engine_on_dead)
{

	if (!(read_register(I2S_TX_DESC1) & 0x80000000)) {
		/* Descriptor is own by SW implies engine is dead */
		printk("D\n");
		if (reset_engine_on_dead) {
			printk("reset audio engine\n");
			reset_audio_engine();
		}
		return 1;
	}
	/* engine is not dead */
	kick_tx_desc(kick_tx_idx);

	/*
	** On client, if i2s rx is not ready, it implies that it is downstream only scenario.
	** We do "dynamic freq. adj" under downstream only case.
	*/
#if defined(CONFIG_ARCH_AST1500_CLIENT)
#if defined(I2S_CLOCK_ADJUST)
	if (!rx_ready)
		clk_adj_selection(tx_buffer_filled());
#endif
#endif
	return 0;
}
#endif /* #if (CONFIG_AST1500_SOC_VER >= 2) */

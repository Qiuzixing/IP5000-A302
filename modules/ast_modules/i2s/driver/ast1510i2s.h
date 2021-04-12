/*
 * Copyright (c) 2004-2012
 * ASPEED Technology Inc. All Rights Reserved
 * Proprietary and Confidential
 *
 * By using this code you acknowledge that you have signed and accepted
 * the terms of the ASPEED SDK license agreement.
 */

#ifndef _AST1510_AUDIO_H_
#define _AST1510_AUDIO_H_

#if defined(CONFIG_ARCH_AST1500_CLIENT)
	#define I2S_CLOCK_ADJUST
#endif

#if (CONFIG_AST1500_SOC_VER >= 3)
	#define I2S_INT 24
	#define I2S_DATA_DELAY
	#define I2S_SYNC_CHECK_NET_RX
#else
	#define I2S_INT 11
	#undef AST1510_I2S_A0
#endif

#define NEW_FPGA_TX_PLL 0
#define NEW_MAX_PKT 0
//NEW_24_TO_16 and MULTI_CHANNEL_SYNC can NOT be enabled simultaneously.
#define NEW_24_TO_16 1
#define NEW_DOWN_SAMPLE 1
#define MULTI_CHANNEL_SYNC 1

#define NEW_FREQ_DET 0

#if NEW_FREQ_DET
#define FREQ_DET_SRC_SEL 2 /* 0:SPDIFDI;1:I2SMCLK;2:I2SWS;3:I2SSDI0 */
#define ACLK_SRC_SEL 2 /* 00:I2SCLKO;01:I2SCLKO;10:H-PLL;11:MCLK */
#else
#define FREQ_DET_SRC_SEL 4
#define ACLK_SRC_SEL 4
#endif

#if NEW_MAX_PKT
	#define    TX_PACKET_SIZE                       0xE000
	#define    RX_PACKET_SIZE						0xE000
	#define    NUM_TX_DESCRIPTOR                4
	#define    NUM_RX_DESCRIPTOR                4
	#define    SIZE_TX_DESCRIPTOR               0x8
	#define    SIZE_RX_DESCRIPTOR               0x8
	#define    MAX_RX_PACKET_NUM               2
	#define    MAX_RX_SIZE               (RX_PACKET_SIZE * MAX_RX_PACKET_NUM)
	#define    MAX_TX_PACKET_NUM               2//MAX_RX_SIZE / TX_PACKET_SIZE
#else
	#define    NUM_RX_DESCRIPTOR				128//<= 1024
	/*
	 * In current implementation, I2S need one about 1.33~1.93 ms to accumulate data for each buffer
	 * For I2S tx delay feature, we define NUM_TX_DESCRIPTOR to 256 and can get max to 340ms delay
	 * But in 192KHz, 8 channels case, we will need 8192 bytes for each tx/rx buffer and need 8192 * 256 = 0x200000 bytes
	 *
	 * Because this size is too big to allocate for dma_pool_alloc()/dma_alloc_coherent()
	 * we rollback NUM_TX_DESCRIPTOR to 128 and the tx delay time is max to ~170ms
	 */
	#define    NUM_TX_DESCRIPTOR				128//<= 1024
#if (CONFIG_AST1500_SOC_VER >= 3)
	#define    NUM_RX_DESCRIPTOR_OP1			32 /* NUM_RX_DESCRIPTOR is 32 in ast1500i2s.h */
	#define    NUM_TX_DESCRIPTOR_OP1			32 /* NUM_TX_DESCRIPTOR is 32 in ast1500i2s.h */
#endif
	#define    SIZE_TX_DESCRIPTOR               0x8
	#define    SIZE_RX_DESCRIPTOR               0x8
#endif /* #if NEW_MAX_PKT */

#if 0
#define I2SPLL_EnParameter_1
#define I2SPLL_EnStage_3
#endif

//#define I2SPLL_EnStage_2

#define BIT0  (1<<0)
#define BIT1  (1<<1)
#define BIT2  (1<<2)
#define BIT3  (1<<3)
#define BIT4  (1<<4)
#define BIT5  (1<<5)
#define BIT6  (1<<6)
#define BIT7  (1<<7)
#define BIT8  (1<<8)
#define BIT9  (1<<9)
#define BIT10 (1<<10)
#define BIT11 (1<<11)
#define BIT12 (1<<12)
#define BIT13 (1<<13)
#define BIT14 (1<<14)
#define BIT15 (1<<15)
#define BIT16 (1<<16)
#define BIT17 (1<<17)
#define BIT18 (1<<18)
#define BIT19 (1<<19)
#define BIT20 (1<<20)
#define BIT21 (1<<21)
#define BIT22 (1<<22)
#define BIT23 (1<<23)
#define BIT24 (1<<24)
#define BIT25 (1<<25)
#define BIT26 (1<<26)
#define BIT27 (1<<27)
#define BIT28 (1<<28)
#define BIT29 (1<<29)
#define BIT30 (1<<30)
#define BIT31 (1<<31)

	//I2S Source Dependent Config
#define SRC_DATA_START_FROM 1 // 1,2,3....Normally, I2S data start after the first clock of WS
#define SRC_CH0_ON_WS_HIGH 0 // 0:on WS low, 1: on WS high
#define SRC_LSB_FIRST 0 // 0: MSB first, 1: LSB first
#define SRC_PADDING_0 0 // 1: Pre-padding 0 before valid bits. 0: no padding
#define SRC_WS_SIZE 32 // 32 or 16 bits
#if 0
#if NEW_DOWN_SAMPLE
#define SRC_DOWN_SAMPLE 1 // I2S00[19] and I2S04[19]. 1: enable 0: disable
#else
#define SRC_DOWN_SAMPLE 0 // I2S00[19] and I2S04[19]. 1: enable 0: disable
#endif
#endif

/* I2S WS configuration */
#define TX_WS_INTERNAL 1
#define RX_WS_INTERNAL 0

// SCU
#define SCU_ENGINE_ADDRESS                     0x1E6E2000
#define SCU_D2PLL_PARAM_REGISTER               0x1c + SCU_ENGINE_ADDRESS
	//SCU1C: D2-PLL Parameter Register
	#define SCU1C_NUMERATOR(v) WBITS(v, 0, 8)
	#define SCU1C_DENUMERATOR(v) WBITS(v, 8, 5)
	#define SCU1C_OUTPUT_DIVIDER(v) WBITS(v, 13, 2)
	#define SCU1C_POST_DIVIDER(v) WBITS(v, 15, 2)
	#define SCU1C_TURN_OFF_PLL(en) WBITS(en, 17, 1)
	#define SCU1C_EN_BYPASS_MODE(en) WBITS(en, 18, 1)


	//AST1510 I2S/SPDIF register define
#define I2S_ENGINE_ADDRESS (0x1E6E5000)
#define I2S_TX_CFG (0x00 + I2S_ENGINE_ADDRESS)
#define I2S_RX_CFG (0x04 + I2S_ENGINE_ADDRESS)
#define I2S_FREQ_DETECT (0x08 + I2S_ENGINE_ADDRESS)
#define I2S_SPDIF_EDGE_MASK (0x0C + I2S_ENGINE_ADDRESS)
#define I2S_DMA_TH (0x10 + I2S_ENGINE_ADDRESS)
#define I2S_FREQ_INC_TH (0x14 + I2S_ENGINE_ADDRESS)
#define I2S_FREQ_DEC_TH (0x18 + I2S_ENGINE_ADDRESS)
#define I2S_FREQ_DETECT_TOL (0x1C + I2S_ENGINE_ADDRESS)
#define I2S_ENG_CFG (0x20 + I2S_ENGINE_ADDRESS)
#define I2S_PLL_FREQ_ADJ_PARAM (0x24 + I2S_ENGINE_ADDRESS)
#define I2S_FREQ_DETECT_STAT_MIN (0x30 + I2S_ENGINE_ADDRESS)
#define I2S_FREQ_DETECT_STAT_MAX (0x38 + I2S_ENGINE_ADDRESS)
#define I2S_TX_DESC0 (0x40 + I2S_ENGINE_ADDRESS)
#define I2S_TX_DESC1 (0x44 + I2S_ENGINE_ADDRESS)
#define I2S_TX_DESC_BASE (0x48 + I2S_ENGINE_ADDRESS)
#define I2S_TX_DESC_POLL (0x4C + I2S_ENGINE_ADDRESS)
#define I2S_HW_TX_DESC_PTR (0x4C + I2S_ENGINE_ADDRESS)
#define I2S_RX_DESC0 (0x50 + I2S_ENGINE_ADDRESS)
#define I2S_RX_DESC1 (0x54 + I2S_ENGINE_ADDRESS)
#define I2S_RX_DESC_BASE (0x58 + I2S_ENGINE_ADDRESS)
#define I2S_RX_DESC_POLL (0x5C + I2S_ENGINE_ADDRESS)
#define I2S_IO_DATA_SELECT (0x60 + I2S_ENGINE_ADDRESS)
#define I2S_DWB_OFFSET (0x64 +  I2S_ENGINE_ADDRESS)
#define I2S_INT_EN (0x68 + I2S_ENGINE_ADDRESS)
#define I2S_INT_STAT (0x6C + I2S_ENGINE_ADDRESS)
#define I2S_TX_TEST (0x70 + I2S_ENGINE_ADDRESS)
#define I2S_RX_TEST (0x74 + I2S_ENGINE_ADDRESS)
#define I2S_ENG_CLK_CFG (0x78 + I2S_ENGINE_ADDRESS)

#define MASK(offset, valid_bits) (((1UL << (valid_bits)) - 1) << (offset))
#define WBITS(value, offset, valid_bits) (((value) & ((1UL << (valid_bits)) - 1)) << (offset))
#define RBITS(rd_value, offset, valid_bits) (((rd_value) & MASK(offset, valid_bits)) >> (offset))

	//I2S00 TX_CFG
#define _I2S00_EN_D0(en) WBITS(en, 0, 1)
#define _I2S00_EN_D1(en) WBITS(en, 1, 1)
#define _I2S00_EN_D2(en) WBITS(en, 2, 1)
#define _I2S00_EN_D3(en) WBITS(en, 3, 1)
#define I2S00_DWB(b) WBITS(b, 4, 6)
#define I2S00_INTERNAL_WS(en) WBITS(en, 10, 1)
#define I2S00_MULTI_CHANNEL_SYNC_EN(en) WBITS(en, 11, 1)
#define I2S00_VALID_BITS(v) WBITS(((v)-1), 12, 5)
#define I2S00_EN_16BITS(en) WBITS(en, 17, 1)
#define I2S00_LSB_FIRST(en) WBITS(en, 18, 1)
#define I2S00_EN_UPSAMPLE(en) WBITS(en, 19, 1)
#define I2S00_DWB_OFFSET(o) WBITS(o, 20, 6)
/*
 * AST1520 A1 I2S00[26]: Enable I2S Transmitter I2SSDO0 pin of dual port to transmit
 * AST1520 A1 I2S00[27]: Enable I2S Transmitter I2SSDO1 pin of dual port to transmit
 * AST1520 A1 I2S00[28]: Enable I2S Transmitter I2SSDO2 pin of dual port to transmit
 * AST1520 A1 I2S00[29]: Enable I2S Transmitter I2SSDO3 pin of dual port to transmit
 */
#define I2S00_EN_D0_DUAL_PORT(en) WBITS(en, 26, 1)
#define I2S00_EN_D1_DUAL_PORT(en) WBITS(en, 27, 1)
#define I2S00_EN_D2_DUAL_PORT(en) WBITS(en, 28, 1)
#define I2S00_EN_D3_DUAL_PORT(en) WBITS(en, 29, 1)
/* I2S00[31:30] Disable SPDIF error handling[1:0] */
#define I2S00_DISABLE_SPDIF_ERR(v) WBITS(v, 30, 1)

#define I2S00_EN_D0(en) (_I2S00_EN_D0(en) | I2S00_EN_D0_DUAL_PORT(en))
#define I2S00_EN_D1(en) (_I2S00_EN_D1(en) | I2S00_EN_D1_DUAL_PORT(en))
#define I2S00_EN_D2(en) (_I2S00_EN_D2(en) | I2S00_EN_D2_DUAL_PORT(en))
#define I2S00_EN_D3(en) (_I2S00_EN_D3(en) | I2S00_EN_D3_DUAL_PORT(en))

	//I2S04 RX_CFG
#define I2S04_EN_D0(en) WBITS(en, 0, 1)
#define I2S04_EN_D1(en) WBITS(en, 1, 1)
#define I2S04_EN_D2(en) WBITS(en, 2, 1)
#define I2S04_EN_D3(en) WBITS(en, 3, 1)
#define I2S04_DWB(b) WBITS(b, 4, 6)
#define I2S04_INTERNAL_WS(en) WBITS(en, 10, 1)
#define I2S04_MULTI_CHANNEL_SYNC_EN(en) WBITS(en, 11, 1)
#define I2S04_VALID_BITS(v) WBITS(((v)-1), 12, 5)
#define I2S04_EN_16BITS(en) WBITS(en, 17, 1)
#define I2S04_LSB_FIRST(en) WBITS(en, 18, 1)
#define I2S04_EN_DOWNSAMPLE(en) WBITS(en, 19, 1)
#define I2S04_DWB_OFFSET(o) WBITS(o, 20, 6)
#define I2S04_DISABLE_SPDIF_ERR(v) WBITS(v, 28, 4)
	//I2S08 F.D. and SPDIF edge mask
#define I2S08_SPDIF_NE3(v) WBITS(v, 0, 8)
#define I2S08_SPDIF_PE3(v) WBITS(v, 8, 8)
#define I2S08_MCLK_RATIO(v) WBITS(v, 16, 3)
#define I2S08_EN_FD_MIN(en) WBITS(en, 20, 1)
#define I2S08_EN_FD_MAX(en) WBITS(en, 22, 1)
#define I2S08_FD_SRC(v) WBITS(v, 24, 2)
#define I2S08_FD_STABLE_CNT(v) WBITS(v, 26, 6)
	//I2S0C SPDIF edge mask
#define I2S0C_SPDIF_NE1(v) WBITS(v, 0, 8)
#define I2S0C_SPDIF_PE1(v) WBITS(v, 8, 8)
#define I2S0C_SPDIF_NE2(v) WBITS(v, 16, 8)
#define I2S0C_SPDIF_PE2(v) WBITS(v, 24, 8)
	//I2S10 DMA FIFO threshold
#define I2S10_TX_TH(v) WBITS(v, 0, 7)
#define I2S10_RX_TH(v) WBITS(v, 7, 7)
	//I2S20 I2S_ENG_CFG
#define I2S20_EN_TX(en) WBITS(en, 0, 1)
#define I2S20_EN_RX(en) WBITS(en, 1, 1)
#define I2S20_OUT_I2SCLK(en) WBITS(en, 2, 1)
#define I2S20_OUT_I2SWS(en) WBITS(en, 3, 1)
//#define I2S20_32BITS_WS(en) WBITS(en, 4, 1)
#define I2S20_32BITS_TXWS(en) WBITS(en, 4, 1)
#if (CONFIG_AST1500_SOC_VER >= 3)
#define I2S20_32BITS_RXWS(en) WBITS(en, 5, 1)
#else
#define I2S20_32BITS_RXWS(en) WBITS(en, 4, 1)
#endif
//#define I2S20_I2SWS_FALLING_EDGE(en) WBITS(en, 10, 1)
#define I2S20_I2SRXWS_FALLING_EDGE(en) WBITS(en, 10, 1)
#if (CONFIG_AST1500_SOC_VER >= 3)
#define I2S20_I2STXWS_FALLING_EDGE(en) WBITS(en, 9, 1)
#else
#define I2S20_I2STXWS_FALLING_EDGE(en) WBITS(en, 10, 1)
#endif
#define I2S20_DATA_FALLING_EDGE(en) WBITS(en, 11, 1)
#define I2S20_SPDIF_MODE(en) WBITS(en, 12, 1)
#define I2S20_20BITS_SPDIF(en) WBITS(en, 13, 1)
#define I2S20_OUT_SPDIF(en) WBITS(en, 15, 1)
#define I2S20_EN_FAC(en) WBITS(en, 16, 1)
#ifdef AST1510_I2S_A0
#define I2S20_EN_FAC_DEC(en) WBITS(en, 17, 1)
#define I2S20_EN_FAC_INC(en) WBITS(en, 18, 1)
#else
#define I2S20_EN_FAC_INC(en) WBITS(en, 16, 1)
#endif
#define I2S20_FAC_2NORMAL(en) WBITS(en, 19, 1)
#define I2S20_I2S_TRANSMIT_LDS(en) WBITS(en, 20, 1)
#define I2S20_OUTPUT_I2SMCLK_MASK	0x00400000
#define I2S20_OUT_I2SMCLK(en) WBITS(en, 22, 1)

/* I2S60: I2S IO data selection */
#define I2S60_OUTPUT_DATA_SELECT_I2S_2_D3(v) WBITS(v, 14, 2)
#define I2S60_OUTPUT_DATA_SELECT_I2S_2_D2(v) WBITS(v, 12, 2)
#define I2S60_OUTPUT_DATA_SELECT_I2S_2_D1(v) WBITS(v, 10, 2)
#define I2S60_OUTPUT_DATA_SELECT_I2S_2_D0(v) WBITS(v, 8, 2)
#define I2S60_OUTPUT_DATA_SELECT_I2S_1_D3(v) WBITS(v, 6, 2)
#define I2S60_OUTPUT_DATA_SELECT_I2S_1_D2(v) WBITS(v, 4, 2)
#define I2S60_OUTPUT_DATA_SELECT_I2S_1_D1(v) WBITS(v, 2, 2)
#define I2S60_OUTPUT_DATA_SELECT_I2S_1_D0(v) WBITS(v, 0, 2)
#define I2S60_OUTPUT_DATA_SELECT_MASK 0x0000FFFF

	//I2S68 I2S_INT_EN
#define I2S68_EN_FD(en) WBITS(en, 0, 1)
#define I2S68_EN_SPDIF_ERR(en) WBITS(en, 1, 1)
#define I2S68_EN_RX_DONE(en) WBITS(en, 2, 1)
#define I2S68_EN_TX_DONE(en) WBITS(en, 3, 1)
#define I2S68_EN_RX_EMPTY(en) WBITS(en, 4, 1)
#define I2S68_EN_TX_EMPTY(en) WBITS(en, 5, 1)
#define I2S68_EN_RX_OVERFLOW(en) WBITS(en, 16, 1)
#define I2S68_EN_TX_UNDERRUN(en) WBITS(en, 17, 1)
#define I2S68_EN_SPDIF_RX_ERR(v) WBITS(v, 18, 4)
#define I2S68_EN_SPDIF_TX_ERR(v) WBITS(v, 22, 2)
#define INT_FD_MASK				0x00000001
#define INT_SPDIF_ERR_MASK		0x00000002
#define INT_RX_DONE_MASK		0x00000004
#define INT_TX_DONE_MASK		0x00000008
#define INT_RX_EMPTY_MASK		0x00000010
#define INT_TX_EMPTY_MASK		0x00000020
#define INT_RX_OVERFLOW_MASK	0x00010000
#define INT_TX_UNDERRUN_MASK	0x00020000
#define I2S_TX_INT_MASK (I2S68_EN_TX_DONE(1) | I2S68_EN_TX_EMPTY(1))


	//I2S78 I2S_ENG_CLK_CFG
#define I2S78_I2SCLKO_DIV_SEL_MASK	0x00000007
#define I2S78_I2SCLKO_DIV(v) WBITS(v, 0, 3)
#define I2S78_I2SCLKO_DIV_EN_MASK	0x00000008
#define I2S78_I2SCLKO_DIV_EN(en) WBITS(en, 3, 1)
#define I2S78_I2SMCLKO_DIV_SEL_MASK	0x00000070
#define I2S78_I2SMCLKO_DIV(v) WBITS(v, 4, 3)
#define I2S78_I2SMCLKO_DIV_EN_MASK	0x00000080
#define I2S78_I2SMCLKO_DIV_EN(en) WBITS(en, 7, 1)
#define I2S78_ACLK_DIV_SEL_MASK	0x00000700
#define I2S78_ACLK_DIV(v) WBITS(v, 8, 3)
#define I2S78_ACLK_DIV_EN_MASK	0x00000800
#if NEW_FPGA_TX_PLL
#define I2S78_ACLK_DIV_EN(en) WBITS(en, 11, 1)
#endif
#define I2S78_I2SCLK_DIV_SEL_MASK	0x00007000
#define I2S78_I2SCLK_DIV(v) WBITS(v, 12, 3)
#define I2S78_I2SCLK_DIV_EN_MASK	0x00008000
#define I2S78_I2SCLK_DIV_EN(en) WBITS(en, 15, 1)
#ifdef AST1510_I2S_A0
#define I2S78_ACLK_SRC(v) WBITS(v, 16, 1)
#else
#define I2S78_ACLK_SRC_SEL_MASK		0x01010000
	#define I2S78_ACLK_SRC_HPLL			0x01000000
#if (ACLK_SRC_SEL == 2)
#define I2S78_ACLK_SRC_SEL	I2S78_ACLK_SRC_HPLL
#define I2S78_ACLK_DIV_EN	0x00000800
#endif
#endif
#define I2S78_I2SCLK_SRC_SEL_MASK	0x00040000
#define I2S78_I2SCLK_SRC(v) WBITS(v, 17, 1)
#define I2S78_FIFO_CTRL_MASK		0x000C0000
#define I2S78_FIFO_CTRL(v) WBITS(v, 18, 2)
#define I2S78_DUAL_PORT_OUTPUT_EN(v)	WBITS(v, 25, 1)
#define I2S78_DUAL_PORT_OUTPUT_EN_MASK	(0x1 << 25)
#define I2S78_TEST_MODE(en) WBITS(en, 31, 1)

#define I2S_HW_TX_DESC_PTR_MASK 0x3ff //mask bits 0-9

typedef struct
{
	// RXDES0
	u32 base_address:29;
	u32 reserved1:3;
	
	// RXDES1
	u32 length:22;
	u32 reserved2:8;
	u32 end_bit:1;
	u32 owner_bit:1;
} __attribute__ ((packed)) RX_DESC;

typedef struct
{
	// TXDES0
	u32 base_address:29;
	u32 reserved1:3;
	
	// TXDES1
	u32 length:22;
	u32 reserved2:8;
	u32 end_bit:1;
	u32 owner_bit:1;
} __attribute__ ((packed)) TX_DESC;

#if NEW_FREQ_DET
typedef struct {
	u32	sum;
	u8	sample_rate_index;
	char	sample_rate[15];
} Freq_Det;
#endif

typedef struct {
	u8	sample_rate_index;
	u8	sample_size;
	u32	PLL_parameter;
	u32	horizontal_total;
	u32	horizontal_retrace;
} I2S_TABLE;

typedef struct
{
	u32 sample_rate_index;
	u32 numerator; //0-7
	u32 denumerator; //8-12
	u32 output_divider; //13-14
	u32 post_divider; //15-16
	u32 i2sclko_divider;
	u32 pll_adj_H;
	u32 pll_adj_N;
	u32 pll_adj_L;
	u32 i2smclko_divider;
} SOC_V2_I2S_TABLE;

int init_audio_engine(void);
void setup_audio_engine(Audio_Info *audio_info, Audio_Buf *rx_buffer, Audio_Buf *tx_buffer, unsigned int i2s_master, unsigned int output_mclk);
void kick_tx_desc(unsigned int idx);
void kick_rx_desc(void);
void enable_rx(void);
void collect_rx_data(unsigned int max_pkt_num, unsigned int *pkt_num, unsigned int *start_idx);
void reclaim_rx_desc(unsigned int start_idx);
unsigned int poll_rx_data(void);
void stop_audio_engine(void);
int audio_int_handler(int *rx_int);
void disable_rx_int(void);
void enable_rx_int(void);
#if defined(CONFIG_ARCH_AST1500_CLIENT)
#if (PATCH_CODEV3 & PATCH_AST1520A0_I2S_DUAL_OUTPUT)
void i2s_dual_output_cfg(u32 en, u32 port_in_use);
u32 i2s_dual_output(void);
#endif
#endif /* #if defined(CONFIG_ARCH_AST1500_CLIENT) */
#if defined(I2S_DATA_DELAY)
#define I2S_TX_DELAY_QUEUE_TOLERANCE (32 + 16)
#define I2S_TX_DELAY_QUEUE_TOLERANCE_1500 (16 + 4)
#define I2S_TX_DELAY_TIME_MIN_MS 0 /* 16ms => 0ms, no need to set minimum delay time if we add drop tolerance for tx queue */
void tx_queue_len_cfg(u32 cfg, unsigned int max_buf_num, unsigned int min_buf_num);
u32 tx_queue_len(void);
#endif
u32 tx_kick_th(void);
unsigned int tx_buffer_filled(void);
unsigned int do_tx_buf_overflow_patch(unsigned int);

#if defined(I2S_SYNC_CHECK_NET_RX)
void net_rx_check_start(int type);
int net_rx_check(u32 pkt_num);
#endif

#endif /* #ifndef _AST1510_AUDIO_H_ */

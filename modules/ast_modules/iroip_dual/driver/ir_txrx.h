/*
 * Copyright (c) 2004-2012
 * ASPEED Technology Inc. All Rights Reserved
 * Proprietary and Confidential
 *
 * By using this code you acknowledge that you have signed and accepted
 * the terms of the ASPEED SDK license agreement.
 */

#ifndef _IR_TXRX_
#define _IR_TXRX_

#include <asm/arch/drivers/board_def.h>

#if (CONFIG_AST1500_SOC_VER >= 2)

#if (CONFIG_AST1500_SOC_VER >= 3)
#define IR_INT 24
#else
#define IR_INT 11
#endif

//#define AST1510_IR_A0

//#define    PACKETSIZE                       0x1000
/*
** Bruce150521. To adapt SoC V3's new timeout design (bad design), we increase
** the buffer size and hope most of short IR button press can fit into one single
** desc buffer.
**
** The same buff/desc size at SoC V2 is tested and verified ok.
*/
#define    SIZE_TX_PACKET		0x8000
#define    SIZE_RX_PACKET		0x8000
#define    NUM_TX_DESCRIPTOR		4
#define    NUM_RX_DESCRIPTOR		4
#define    SIZE_TX_DESCRIPTOR		0x8
#define    SIZE_RX_DESCRIPTOR		0x8
//#define    MAX_TX_PACKET_NUM               15 //MUST < NUM_TX_DESCRIPTOR and == MAX_RX_PACKET_NUM
//#define    MAX_RX_PACKET_NUM               15 //MUST < NUM_RX_DESCRIPTOR and == MAX_TX_PACKET_NUM
//#define    MAX_TX_SIZE               (PACKETSIZE * MAX_TX_PACKET_NUM) // MUST < 64KB
//#define    MAX_RX_SIZE               (PACKETSIZE * MAX_RX_PACKET_NUM) // MUST < 64KB
//#define    MAX_TX_SIZE               (SIZE_TX_PACKET * MAX_TX_PACKET_NUM) // MUST < 64KB
//#define    MAX_RX_SIZE               (SIZE_RX_PACKET * MAX_RX_PACKET_NUM) // MUST < 64KB
//#define SIZE_TO_DESC_CNT(size) (((size)+((PACKETSIZE)-1))/PACKETSIZE)
#define MAX_IR_TX_BUF_SIZE (SIZE_TX_PACKET * NUM_TX_DESCRIPTOR)
#define MAX_UDP_RX_SIZE ((MAX_IR_TX_BUF_SIZE > (64*1024))?(64*1024):(MAX_IR_TX_BUF_SIZE))
#define TX_SIZE_TO_DESC_CNT(size) (((size)+((SIZE_TX_PACKET)-1))/SIZE_TX_PACKET)
#define RX_SIZE_TO_DESC_CNT(size) (((size)+((SIZE_RX_PACKET)-1))/SIZE_RX_PACKET)

#define IR_TX_PORT 1
#define IR_RX_PORT 0

//I2S Descriptor
#define    OWNER                                  (1 << 31)
#define    SW_OWN                                 0
#define    ENGINE_OWN                             1
#define    END_OF_RING                            (1 << 30)

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

#define MASK(offset, valid_bits) (((1UL << (valid_bits)) - 1) << (offset))
#define WBITS(value, offset, valid_bits) (((value) & ((1UL << (valid_bits)) - 1)) << (offset))
#define RBITS(rd_value, offset, valid_bits) (((rd_value) & MASK(offset, valid_bits)) >> (offset))

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
#define I2S_DWB_OFFSET (0x64 +  I2S_ENGINE_ADDRESS)
#define I2S_INT_EN (0x68 + I2S_ENGINE_ADDRESS)
#define I2S_INT_STAT (0x6C + I2S_ENGINE_ADDRESS)
#define I2S_TX_TEST (0x70 + I2S_ENGINE_ADDRESS)
#define I2S_RX_TEST (0x74 + I2S_ENGINE_ADDRESS)
#define I2S_ENG_CLK_CFG (0x78 + I2S_ENGINE_ADDRESS)

#define IR_ENG_CLK_CFG (0x7C + I2S_ENGINE_ADDRESS)
#define IR_DESC0 (0x80 + I2S_ENGINE_ADDRESS)
#define IR_DESC1 (0x84 + I2S_ENGINE_ADDRESS)
#define IR_DESC_BASE (0x88 + I2S_ENGINE_ADDRESS)
#define IR_DESC_POLL (0x8C + I2S_ENGINE_ADDRESS)
#define IR_DMA_TH (0x90 + I2S_ENGINE_ADDRESS)
#define IR_ENG_CFG (0x94 + I2S_ENGINE_ADDRESS)
#define IR_INT_EN (0x98 + I2S_ENGINE_ADDRESS)
#define IR_INT_STAT (0x9C + I2S_ENGINE_ADDRESS)

//IR source dependent config
#if (BOARD_DESIGN_VER_IR < 204) //For CONFIG_AST1510_BOARD_EVA_V2
#define SRC_IR_RX_POLARITY 1 //I2S94[27] 1: low active 0: high active
#define SRC_IR_TX_POLARITY 1 //I2S94[27] 1: low active 0: high active
#else //#if (BOARD_DESIGN_VER_IR < 204)
#define SRC_IR_RX_POLARITY 0 //I2S94[27] 1: low active 0: high active
#define SRC_IR_TX_POLARITY 0 //I2S94[27] 1: low active 0: high active
#endif //#if (BOARD_DESIGN_VER_IR < 204)
#define SRC_IR_TIMEOUT_CTL 1 //I2S94[26] 1: enable 0: disable
/*
** Bruce150520.
** Target silent period is about 50ms.
** Target timeout period is about 500ms.
*/
/*
 * IR clock at AST1510 A5
 * clock source: H-PLL
 * clock divider: 16
 * => 420 / 16 = 26.25MHz
 * silent period configuration: 0x555
 *	(0x555 + 1) << 10 = 0x155800 (1398784)
 *	1398784 / 26.25 MHz ~= 53.287ms
 * timeout period configuration: 0x4000
 *	(0x4000 + 1) << 10 = 0x1000400 (16778240)
 *	16778240 / 26.25 MHz ~= 639.171ms
 *
 * IR clock at A6 is 2MHz
 * silent period configuration: 0x31
 *	(0x31 + 1) << 10 = 0xc800 (51200)
 *	51200 / 2 MHz ~= 25.6ms
 * silent period configuration: 0x61
 *	(0x61 + 1) << 10 = 0x18800 (100352)
 *	100352 / 2 MHz ~= 50.176ms
 * timeout period configuration: 0x1E9
 *	(0x1E9 + 1) << 10 = 0x7A800 (501760)
 *	501760 / 2 MHz ~= 250.88ms
 * timeout period configuration: 0x3D2
 *	(0x3D2 + 1) << 10 = 0xF4C00 (1002496)
 *	1002496 / 2 MHz ~= 501.248ms
 */
#if 0
#if (CONFIG_AST1500_SOC_VER >= 3)
//#define SRC_SILENT_PERIOD 0xFFF //I2S94[24:10]
#define SRC_SILENT_PERIOD 0x31 //I2S94[24:10]
#define SRC_TIMEOUT_PERIOD 0x1E9 //I2S90[25:10]
#else
//#define SRC_SILENT_PERIOD 0xFFF //I2S94[24:10]
#define SRC_SILENT_PERIOD 0x31 //I2S94[24:10]
#define SRC_TIMEOUT_PERIOD 0x1E9 //I2S90[25:10]
#endif
#else
#define SRC_SILENT_PERIOD 0x61 //I2S94[24:10]
#define SRC_TIMEOUT_PERIOD 0x3D2 //I2S90[25:10]
#endif
#define SRC_ENCODE_MODE 0 //I2S94[25]

#ifdef AST1510_IR_A0
#define I2S7C_IRCLK_DIV(v) WBITS(v, 0, 3)
#define I2S7C_IRCLK_DIV_EN(en) WBITS(en, 3, 1)
#define I2S7C_IRCLK_SRC(v) WBITS(v, 4, 1)
#else
#define I2S7C_IRCLK_DIV(v) WBITS(v, 0, 7) /* bit[6:0] => 7 bits */
#define I2S7C_IRCLK_DIV_EN(en) WBITS(en, 7, 1)
#if (CONFIG_AST1500_SOC_VER >= 3) /* Actually it is >= AST1520 A1 */
#define I2S7C_IRCLK_SRC(v) WBITS(v, 8, 2)
#define CURRENT_IRCLK_SRC 0x2 /* AST1520 and above uses 24MHz CLKIN as source. */
#else
#define I2S7C_IRCLK_SRC(v) WBITS(v, 8, 1)
#define CURRENT_IRCLK_SRC 0 /* AST1510 uses HPLL as source. */
#endif
#endif
#define I2S7C_IR_TX_MODE_SEL(en) WBITS(en, 15, 1)

/* Bruce150520.
** We aims to use all the same clock source accross different SoC versions.
** In SoC V3 and future, we will take 24MHz CLKIN. In SoC V2, it is HPLL.
** So, the common clock is 2MHz (Note 1):
** SoC V2: 420/210  = 2MHz
** SoC >= V3: 24/12 = 2MHz
**
** Note 1:
** For AST1510 A1,
**	H-PLL: 420 MHz
**	IR clock divider: 2~256 (A0 is 2~16)
** We cannot get 1MHz IR clock (420/256 > 1), and use 2Mhz for common clock instead.
**/
#if (CONFIG_AST1500_SOC_VER >= 3)
#define IR_CLK_DIVIDER 12
#else
#if defined(AST1510_IR_A0)
	/* TODO */
#else
#define IR_CLK_DIVIDER 210
#endif
#endif

#define IR_CLK_DIV(a) (((a) - 2) >> 1)
#define CURRENT_IR_CLK_DIV IR_CLK_DIV(IR_CLK_DIVIDER)

#define I2S90_TIMEOUT(v) WBITS(v, 10, 16)
#define I2S90_FIFO_TH(v) WBITS(v, 26, 5)

#define I2S94_SILENT_DET(v) WBITS(v, 10, 15)
#define I2S94_ENCODE_MODE(v) WBITS(v, 25, 1)
#define I2S94_TIMEOUT_CTL(en) WBITS(en, 26, 1)
#define I2S94_IR_POLARITY(v) WBITS(v, 27, 1)
#define I2S94_IR_EN(en) WBITS(en, 28, 1)

#define I2S98_EN_DONE(en) WBITS(en, 0, 1) //Interrupt on eacho complete of desc
#define I2S98_EN_EMPTY(en) WBITS(en, 1, 1) //Interrupt when running out of desc
#define I2S98_EN_RX_END(en) WBITS(en, 2, 1) //Interrupt on "rx silent detected" (one cmd completed)
#define I2S98_EN_RX_TIMEOUT(en) WBITS(en, 3, 1) //Interrupt on "rx timeout" happen
#define I2S98_EN_RX_DESC_END(en) WBITS(en, 4, 1) //Interrupt on "rx cmd end desc" updated
#define I2S98_EN_TX_END(en) WBITS(en, 5, 1) //Interrupt when the ir tx dma completed
#define I2S98_EN_RX_OVERFLOW(en) WBITS(en, 16, 1)
#define I2S98_EN_TX_UNDERLOW(en) WBITS(en, 17, 1)

#define IRINTS_ONE_DESC_DONE I2S98_EN_DONE(1)
#define IRINTS_DESC_EMPTY I2S98_EN_EMPTY(1)
#define IRINTS_RX_CMD_END I2S98_EN_RX_END(1)
#define IRINTS_RX_TIMEOUT I2S98_EN_RX_TIMEOUT(1)
#define IRINTS_RX_CMD_END_DESC_DONE I2S98_EN_RX_DESC_END(1)
#define IRINTS_TX_END I2S98_EN_TX_END(1)
#define IRINTS_RX_OVERFLOW I2S98_EN_RX_OVERFLOW(1)
#define IRINTS_TX_UNDERFLOW I2S98_EN_TX_UNDERLOW(1)
#define IRINTS_RX_IDLE WBITS(1, 30, 1)
#define IRINTS_TX_IDLE WBITS(1, 31, 1)


//#define IR_RX_INT_MASK (I2S98_EN_DONE(1)|I2S98_EN_EMPTY(1)|I2S98_EN_RX_DESC_END(1))
#if (CONFIG_AST1500_SOC_VER >= 3)
#define IR_RX_INT_MASK (I2S98_EN_RX_DESC_END(1) | I2S98_EN_DONE(1))
#else
#define IR_RX_INT_MASK (I2S98_EN_RX_DESC_END(1))
#endif
//#define IR_TX_INT_MASK (I2S98_EN_DONE(1)|I2S98_EN_EMPTY(1)|I2S98_EN_TX_END(1))
#define IR_TX_INT_MASK (I2S98_EN_TX_END(1))

#if (CONFIG_AST1500_SOC_VER >= 3)
#define IR_BASE_ADDRESS_MASK 0x3FFFFFFF /* bit[29:3]*/
#else
#define IR_BASE_ADDRESS_MASK 0x1FFFFFFF /* bit[28:3]*/
#endif

#define    u32               unsigned int

typedef struct _RX_DES1 {
	u32 length:22;
	u32 reserved2:5;
	u32 rx_timeout:1;
	u32 reserved3:2;
	u32 end_bit:1;
	u32 owner_bit:1;
} __attribute__ ((packed)) RX_DES1;

typedef union _RX_DES1_UNION {
    u32               value;
    RX_DES1   rx_desc1;
} RX_DES1_UNION;

typedef struct
{
	// RXDES0
	u32 base_address:29;
	u32 reserved1:3;

	// RXDES1
#if 0
	u32 length:22;
	u32 reserved2:5;
	u32 rx_timeout:1;
	u32 reserved3:2;
	u32 end_bit:1;
	u32 owner_bit:1;
#else
	RX_DES1_UNION rx_desc1_u;
#endif
} __attribute__ ((packed)) RX_DESC;

typedef struct _TX_DES1 {
	u32 length:22;
	u32 reserved2:5;
	u32 rx_timeout:1;
	u32 reserved3:2;
	u32 end_bit:1;
	u32 owner_bit:1;
} __attribute__ ((packed)) TX_DES1;

typedef union _TX_DES1_UNION {
    u32               value;
    TX_DES1   tx_desc1;
} TX_DES1_UNION;

typedef struct
{
	// TXDES0
	u32 base_address:29;
	u32 reserved1:3;

	// TXDES1
#if 0
	u32 length:22;
	u32 reserved2:5;
	u32 rx_timeout:1;
	u32 reserved3:2;
	u32 end_bit:1;
	u32 owner_bit:1;
#else
	TX_DES1_UNION tx_desc1_u;
#endif
} __attribute__ ((packed)) TX_DESC;

#define read_register(address)	*(volatile u32 *)(IO_ADDRESS((u32)(address)))
#define write_register(address, data)	*(volatile u32 *)(IO_ADDRESS((u32)(address))) = (u32)(data)
#define write_register_or(address, data)	*(volatile u32 *)(IO_ADDRESS((u32)(address))) = (u32)(read_register(address) | data)
#define write_register_and(address, data, anddata)	*(volatile u32 *)(IO_ADDRESS((u32)(address))) = (u32)((read_register(address) & (~anddata)) | data)

int check_IRRx_Int(void);

#else //#if (CONFIG_AST1500_SOC_VER >= 2)
	#error "SOC version undefined?!"
#endif //#if (CONFIG_AST1500_SOC_VER >= 2)

#endif

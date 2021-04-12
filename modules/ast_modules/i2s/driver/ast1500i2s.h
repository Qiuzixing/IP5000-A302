/*
 * Copyright (c) 2004-2012
 * ASPEED Technology Inc. All Rights Reserved
 * Proprietary and Confidential
 *
 * By using this code you acknowledge that you have signed and accepted
 * the terms of the ASPEED SDK license agreement.
 */

#ifndef _AST1500_AUDIO_H_
#define _AST1500_AUDIO_H_

#define I2S_INT 11

//#define    TX_PACKET_SIZE                       0x1000
//#define    RX_PACKET_SIZE                       0x1000
//#define    TX_SIZE_TO_PACKET_NUM(s)			((s) >> 12)
#if 1
#define    NUM_TX_DESCRIPTOR                32//16 //Bruce120423. The balance of desc# and buf size is tricky. Looks like we need more desc# to have better response time and keep buf size small to avoid lip sync issue.
#define    NUM_RX_DESCRIPTOR                32//16
#else
#define    NUM_TX_DESCRIPTOR                256
#define    NUM_RX_DESCRIPTOR                256
#endif
#define    SIZE_TX_DESCRIPTOR               0x8
#define    SIZE_RX_DESCRIPTOR               0x8
//#define    MAX_RX_PACKET_NUM               6
//#define    MAX_RX_SIZE               (RX_PACKET_SIZE * MAX_RX_PACKET_NUM)
//#define    MAX_TX_PACKET_NUM               6//MAX_RX_SIZE / TX_PACKET_SIZE


//SCU Register
#define    SCU_ENGINE_ADDRESS                     0x1E6E2000
#define    SCU_RESET_CONTROL_REGISTER             0x04 + SCU_ENGINE_ADDRESS
	#define    PWM_RESET_BIT						  (1 << 9)
	#define    CRT1_RESET_BIT                         (1 << 13)
#define    SCU_CLOCK_STOP_CONTROL_REGISTER        0x0c + SCU_ENGINE_ADDRESS
	#define    VGA_DCLK                               (1 << 5)
	#define    CRT1_D1CLK                             (1 << 10)
	#define    CRT1_D2CLK                             (1 << 11)
#define    SCU_MULTIPIN_CONTROL1_REGISTER         0x74 + SCU_ENGINE_ADDRESS
	#define    ENABLE_GNT4_PIN                        (1 << 6)
	#define    ENABLE_PWM4_PIN						  (1 << 11)
	#define    ENABLE_VGA_PIN                         (1 << 15)
	#define    ENABLE_PORTA_OUTPUT_PIN                (1 << 16)
	#define    ENABLE_PORTB_OUTPUT_PIN                (1 << 17)


//CRT1 Register
#define    CRT1_ENGINE_ADDRESS                    0x1E6E6000
#define    CRT1_CONTROL_REGISTER1                 0x0 + CRT1_ENGINE_ADDRESS
#define    CRT1_CONTROL_REGISTER2                 0x04 + CRT1_ENGINE_ADDRESS
#define    CRT1_PLL_SETTING_REGISTER              0x0C + CRT1_ENGINE_ADDRESS
#define    CRT1_HORIZONTAL_TOTAL_REGISTER         0x10 + CRT1_ENGINE_ADDRESS
#define    CRT1_HORIZONTAL_RETRACE_REGISTER       0x14 + CRT1_ENGINE_ADDRESS
#define    ENABLE_GRAPHIC_DISPLAY                 1
#define    CRT1_SCREEN_OFF                        (1 << 20)
#define    ENABLE_CRT1_DVO                        (1 << 7)

#define    PWM_CONTROLLER_BASE_ADDRESS                    0x1E786000
#define    PWM_GENERAL_CONTROL_REGISTER_ADDRESS                    0x00 + PWM_CONTROLLER_BASE_ADDRESS
#define    PWM_CLOCK_CONTROL_REGISTER_ADDRESS                    0x04 + PWM_CONTROLLER_BASE_ADDRESS
#define    PWM_DUTY_CONTROL0_REGISTER_ADDRESS                    0x08 + PWM_CONTROLLER_BASE_ADDRESS
#define    PWM_DUTY_CONTROL1_REGISTER_ADDRESS                    0x0C + PWM_CONTROLLER_BASE_ADDRESS
#define    TYPEM_CONTROL0_REG                    0x10 + PWM_CONTROLLER_BASE_ADDRESS
#define    TYPEM_CONTROL1_REG                    0x14 + PWM_CONTROLLER_BASE_ADDRESS
#define    TYPEN_CONTROL0_REG                    0x18 + PWM_CONTROLLER_BASE_ADDRESS
#define    TYPEN_CONTROL1_REG                    0x1C + PWM_CONTROLLER_BASE_ADDRESS

//I2S Engine Register
#define    PCI_AHB_OFFSET                         0x10000
#define    I2S_ENGINE_ADDRESS                     0x1E6E5000
#define    I2S_OFFSET                             (I2S_ENGINE_ADDRESS & 0xFFFF) + PCI_AHB_OFFSET
#define    I2S_TX_DESCRIPTOR_ADDRESS_REGISTER     0x8 + I2S_ENGINE_ADDRESS
#define    I2S_TX_POLL_DEMAND_REGISTER            0xC + I2S_ENGINE_ADDRESS
#define    I2S_RX_DESCRIPTOR_ADDRESS_REGISTER     0x18 + I2S_ENGINE_ADDRESS
#define    I2S_RX_POLL_DEMAND_REGISTER            0x1C + I2S_ENGINE_ADDRESS
#define    I2S_ENGINE_CONTROL_REGISTER            0x20 + I2S_ENGINE_ADDRESS
	#define    ENABLE_I2S_READ_SUCCESS                (1 << 26)
#define    I2S_ENGINE_STATUS_REGISTER             0x24 + I2S_ENGINE_ADDRESS
#define    I2S_ENGINE_RX_WRITE_REQCNT_REGISTER    0x28 + I2S_ENGINE_ADDRESS

//I2S Engine Control Bit
#define    DISABLE_ENGINE                         0
#define    ENABLE_RX_ENGINE                       (1 << 0)
#define    ENABLE_TX_ENGINE                       (1 << 1)
#define    LEFT_CHANNEL_HIGH                      (1 << 16)
#define    DELAY_CLOCK_CYCLE                      (1 << 17)
#define    TX_PADDING_ZERO                        (1 << 21)
#define    RX_PADDING_ZERO                        (1 << 20)

typedef struct
{
	// RXDES0
	u32 base_address:28;
	u32 reserved1:4;

	// RXDES1
	u32 length:22;
	u32 reserved2:8;
	u32 end_bit:1;
	u32 owner_bit:1;
} __attribute__ ((packed)) RX_DESC;

typedef struct
{
	// TXDES0
	u32 base_address:28;
	u32 reserved1:4;

	// TXDES1
	u32 length:22;
	u32 reserved2:8;
	u32 end_bit:1;
	u32 owner_bit:1;
} __attribute__ ((packed)) TX_DESC;

typedef struct {
	u8	sample_rate_index;
	u8	sample_size;
	u32	PLL_parameter;
	u32	horizontal_total;
	u32	horizontal_retrace;
	u32	M_PWM_clk_ctrl;
	u32	PWM_duty_ctrl;
} I2S_TABLE;

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
int is_desc_own_by_eng(int desc_idx);
#endif /* #ifndef _AST1500_AUDIO_H_ */

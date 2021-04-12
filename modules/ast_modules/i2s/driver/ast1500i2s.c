/*
 * Copyright (c) 2004-2012
 * ASPEED Technology Inc. All Rights Reserved
 * Proprietary and Confidential
 *
 * By using this code you acknowledge that you have signed and accepted
 * the terms of the ASPEED SDK license agreement.
 */

#if (CONFIG_AST1500_SOC_VER == 1)
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <asm/arch/drivers/board_def.h>
#include <asm/arch/drivers/crt.h>
#include "i2s.h"
#include "ast1500i2s.h"
#include "alc5624.h"
#include <asm/arch/ast-scu.h>

static unsigned int rx_buf_size;
static unsigned int rx_buf_num = 0;
static unsigned int tx_buf_size;
static unsigned int tx_buf_num = 0;
static unsigned int rx_idx, rx_free;
unsigned int kick_tx_idx = 0;
#define IO_IN_USE_NONE	0
#define IO_IN_USE_CODEC	1
#define IO_IN_USE_HDMI		2
extern unsigned int IO_in_Use; //I2S BCLK is from PWM and CPLD is not needed when audio CODEC is in use. 
static volatile TX_DESC *tx_desc;
static volatile RX_DESC *rx_desc;
extern unsigned int rx_ready;
extern unsigned int tx_ready;
extern unsigned int rx_enabled;
extern unsigned int  rx_data_available;
//The clock for I2S should be sample_rate * 2(left + right) * select_period(16/32 bits)
static I2S_TABLE I2S_CLOCK_TABLE[] = {
	{0, 4, 0x2c4f, 112, 0x460014, 0x0700, 0x0703},	//44.1KHz 
	{2, 4, 0x2a40, 100, 0x460014, 0x0700, 0x0703},	//48KHz
	{3, 4, 0x2a40, 150, 0x460014, 0x0b00, 0x0b05},	//32KHz
	{8, 4, 0x2c4f, 56, 0x200014, 0x0300, 0x0301},	//88.2KHz
	{10, 4, 0x2a40, 50, 0x200014, 0x0300, 0x0301},	//96KHz
	{12, 4, 0x2c4f, 28, 0x14000a, 0x0100, 0x0100},	//176.4KHz
	{14, 4, 0x2a40, 25, 0x14000a, 0x0100, 0x0100}	//192KHz
};
static unsigned int I2S_Table_Count = sizeof(I2S_CLOCK_TABLE) / sizeof(I2S_TABLE);

int init_audio_engine(void)
{
	int ret = 0;

	//enable i2s engine clock & multipin
	ast_scu.scu_op(SCUOP_I2S_INIT, NULL);

	return ret;
}

static void setup_pwm_clock(unsigned int port_num, unsigned int freq_index)
{
	unsigned long flags;

	printk("set up PWM\n");

	local_irq_save(flags);
	//set up multipin
	write_register (SCU_MULTIPIN_CONTROL1_REGISTER, (read_register(SCU_MULTIPIN_CONTROL1_REGISTER) & ~ENABLE_GNT4_PIN) | ENABLE_PWM4_PIN);
	//clear reset
	write_register (SCU_RESET_CONTROL_REGISTER, read_register(SCU_RESET_CONTROL_REGISTER) & ~PWM_RESET_BIT);
	local_irq_restore(flags);

	//disable fan tach
	write_register (TYPEM_CONTROL0_REG, 0);
	write_register (TYPEN_CONTROL0_REG, 0);
	//set up M-type PWM clock & duty cycle
	write_register (PWM_CLOCK_CONTROL_REGISTER_ADDRESS, (read_register(PWM_CLOCK_CONTROL_REGISTER_ADDRESS) & 0xFFFF0000) | I2S_CLOCK_TABLE[freq_index].M_PWM_clk_ctrl);

	switch (port_num) {
	case 0:
		write_register (PWM_DUTY_CONTROL0_REGISTER_ADDRESS, (read_register(PWM_DUTY_CONTROL0_REGISTER_ADDRESS) & 0xFFFF0000) | I2S_CLOCK_TABLE[freq_index].PWM_duty_ctrl);
		break;
	case 1:
		write_register (PWM_DUTY_CONTROL0_REGISTER_ADDRESS, (read_register(PWM_DUTY_CONTROL0_REGISTER_ADDRESS) & 0x0000FFFF) | (I2S_CLOCK_TABLE[freq_index].PWM_duty_ctrl << 16));
		break;
	case 2:
		write_register (PWM_DUTY_CONTROL1_REGISTER_ADDRESS, (read_register(PWM_DUTY_CONTROL1_REGISTER_ADDRESS) & 0xFFFF0000) | I2S_CLOCK_TABLE[freq_index].PWM_duty_ctrl);
		break;
	case 3:
		write_register (PWM_DUTY_CONTROL1_REGISTER_ADDRESS, (read_register(PWM_DUTY_CONTROL1_REGISTER_ADDRESS) & 0x0000FFFF) | (I2S_CLOCK_TABLE[freq_index].PWM_duty_ctrl << 16));
		break;
	}
	//enable M-type PWM
	write_register (PWM_GENERAL_CONTROL_REGISTER_ADDRESS, 1 | (1 << (8 + port_num)));
}

static void disable_pwm_clock(unsigned int port_num)
{
	//disable PWM port
	write_register (PWM_GENERAL_CONTROL_REGISTER_ADDRESS, read_register(PWM_GENERAL_CONTROL_REGISTER_ADDRESS) & ~(1 << (8 + port_num)));
}

static void enable_crt(unsigned int crt_id, unsigned long i)
{
	unsigned long flags;
	unsigned int offset = 0x60 * crt_id;

	local_irq_save(flags);

	if (crt_id == 0) { /* CRT1 */
		//Enable D1CLK and DCLK	SCU0C, D[10], D[5]
		//Reset CRT	SCU04, D[13]
		write_register (SCU_CLOCK_STOP_CONTROL_REGISTER, read_register(SCU_CLOCK_STOP_CONTROL_REGISTER) & ~(CRT1_D1CLK | VGA_DCLK));
		udelay (1);
		write_register (SCU_RESET_CONTROL_REGISTER, read_register(SCU_RESET_CONTROL_REGISTER) & (~CRT1_RESET_BIT));
		//Multipin : Disable VGA D[15]/Enable PortA D[16] for VPAHSYNC
		write_register (SCU_MULTIPIN_CONTROL1_REGISTER, (read_register(SCU_MULTIPIN_CONTROL1_REGISTER) & (~ENABLE_VGA_PIN)) | (ENABLE_PORTA_OUTPUT_PIN));
	} else {
		//Enable D2CLK and DCLK SCU0C, D[11], D[5]
		//Reset CRT SCU04, D[13]
		write_register (SCU_CLOCK_STOP_CONTROL_REGISTER, read_register(SCU_CLOCK_STOP_CONTROL_REGISTER) & ~(CRT1_D2CLK | VGA_DCLK));
		udelay (1);
		write_register (SCU_RESET_CONTROL_REGISTER, read_register(SCU_RESET_CONTROL_REGISTER) & (~CRT1_RESET_BIT));
		//Multipin : Enable PortB D[17] for VPBHSYNC
		write_register (SCU_MULTIPIN_CONTROL1_REGISTER, read_register(SCU_MULTIPIN_CONTROL1_REGISTER) | ENABLE_PORTB_OUTPUT_PIN);
	}

	local_irq_restore(flags);

	//Need to enable graphic display and hardware suggests to set screen off
	write_register (CRT1_CONTROL_REGISTER1 + offset, CRT1_SCREEN_OFF | ENABLE_GRAPHIC_DISPLAY);
	//Need to enable CRT1 DVO
	write_register (CRT1_CONTROL_REGISTER2 + offset, ENABLE_CRT1_DVO);
	write_register (CRT1_PLL_SETTING_REGISTER + offset, I2S_CLOCK_TABLE[i].PLL_parameter);
	write_register (CRT1_HORIZONTAL_TOTAL_REGISTER + offset, I2S_CLOCK_TABLE[i].horizontal_total - 1);
	write_register (CRT1_HORIZONTAL_RETRACE_REGISTER + offset, I2S_CLOCK_TABLE[i].horizontal_retrace);
}

static int setup_i2s_clock (u8 target_sample_rate)
{
	unsigned int i;

	for (i = 0; i < I2S_Table_Count; i++) {	
		if (target_sample_rate == I2S_CLOCK_TABLE[i].sample_rate_index) {
			if (IO_in_Use == IO_IN_USE_CODEC) {
				printk("set up PWM%d for I2S clock\n", PWM_I2S_CLK);
				setup_pwm_clock(PWM_I2S_CLK, i);
			} else {
				printk("set up CRT%d HSync for I2S clock\n", CRT_I2S_CLK);
				enable_crt(CRT_I2S_CLK, i);
			}
			return 1;	/* Found */
		}
	}

	printk ("I2S ERROR: Can't find sample_rate = %d\n", target_sample_rate); 
	return 0;	/* Not Found */
}

void setup_audio_engine(Audio_Info *audio_info, Audio_Buf *rx_buffer, Audio_Buf *tx_buffer, unsigned int i2s_master, unsigned int output_mclk)
{
	int i;

	if (rx_buffer && tx_buffer) {
		printk("AST1500 supports only 1 I2S channel.\n");
		BUG();
	}

	if (!rx_buffer && !tx_buffer) {
		printk("setup_audio_engine for nothing!?\n");
		return;
	}
	printk ("SampleSize = %d\n", audio_info->SampleSize);
	printk ("SampleFrequency = %d\n", audio_info->SampleFrequency);

	if (tx_buffer) {
		/*
		** Bruce111031. A correct procedure of for clock change:
		** 1. SCU reset and hold
		** 2. Setup I2S clock and wait for clock stable.
		** 3. Release SCU reset
		** 3. Program I2S engine
		** 4. Start I2S engine
		*/
		tx_desc = tx_buffer->desc_va;
		/* i2s engine reset */
		ast_scu.scu_op(SCUOP_I2S_STOP, NULL);

		setup_i2s_clock(audio_info->SampleFrequency);
		msleep(1);

		ast_scu.scu_op(SCUOP_I2S_START, NULL);

		write_register(I2S_ENGINE_CONTROL_REGISTER, 0);

		tx_buf_size = tx_buffer->buf_size;
		tx_buf_num = tx_buffer->buf_num;
		for (i = 0; i < tx_buf_num; i++) {
			tx_desc[i].base_address = tx_buffer->buf_pa[i];
			tx_desc[i].owner_bit = SW_OWN;
			tx_desc[i].end_bit = 0;
			tx_desc[i].length = tx_buf_size;
		}
		tx_desc[tx_buf_num - 1].end_bit = 1;
		kick_tx_idx = 0;

		/* fill in tx descriptor base register */
		write_register(I2S_TX_DESCRIPTOR_ADDRESS_REGISTER, tx_buffer->desc_pa);
		/* enable engine */
		//	write_register (I2S_ENGINE_CONTROL_REGISTER, ENABLE_TX_ENGINE | DELAY_CLOCK_CYCLE | LEFT_CHANNEL_HIGH | 0x08 | TX_PADDING_ZERO);
		write_register(I2S_ENGINE_CONTROL_REGISTER, ENABLE_TX_ENGINE | /*LEFT_CHANNEL_HIGH |*/ 0x08 | ((audio_info->SampleSize - 16) << 8));

		tx_ready = 1;
	}

	if (rx_buffer) {
		unsigned int status = 0;
		rx_desc = rx_buffer->desc_va;

		/* i2s engine reset */
		ast_scu.scu_op(SCUOP_I2S_STOP, NULL);

		if (IO_in_Use == IO_IN_USE_CODEC)//no CPLD
			setup_i2s_clock(audio_info->SampleFrequency);

		msleep(1);
		ast_scu.scu_op(SCUOP_I2S_START, NULL);

		rx_buf_size = rx_buffer->buf_size;
		rx_buf_num = rx_buffer->buf_num;
		for (i = 0; i < rx_buf_num; i++) {
			rx_desc[i].base_address = rx_buffer->buf_pa[i];
			rx_desc[i].owner_bit = ENGINE_OWN;
			rx_desc[i].end_bit = 0;
			rx_desc[i].length = rx_buf_size;
		}
		rx_desc[rx_buf_num - 1].end_bit = 1;
		/* fill in rx descriptor base register */
		write_register (I2S_RX_DESCRIPTOR_ADDRESS_REGISTER, rx_buffer->desc_pa);
		/* enable engine */
		{
			unsigned int r = 0x1;

			write_register (I2S_ENGINE_CONTROL_REGISTER, 0);
			barrier();
#if 0//according to jazoe, this is not necessary
			msleep(100);
#else
			msleep(1);
#endif
			if (audio_info->SampleSize > 16)
				r |= 0x8;

			r |= (audio_info->SampleSize - 16) << 4;
			//r |= LEFT_CHANNEL_HIGH;

			write_register (I2S_ENGINE_CONTROL_REGISTER, r);
			barrier();
		}

		if (IO_in_Use == IO_IN_USE_CODEC)
			status = 1;
		else { /* for CPLD */
			gpio_direction_output(AST1500_GPC2, 1);
			status = 0;
			if (audio_info->SampleSize > 16)
				gpio_set_value(AST1500_GPC2, 1);
			else
				gpio_set_value(AST1500_GPC2, 0);

			/* Bruce140526. Fix GW Audio DSP issue.
			** There is technically 50% possibility that CPLD can't lock input WS signal.
			** We need to reset I2S (change WS output timing) and hopefully
			** WS signal can be locked. The code retry for 30 times.
			** The worse case I saw is 11 times. Typical cases are under 4 times.
			*/
			for (i = 0; i < 30; i++) {
				/*
				** Bruce120523. RctBug#2012052300.
				** CAT6023 may detected wrong frequency using Win7 HDMI audio.
				** CPLD will not stable (GPIOC4) under this case. So, if we keep
				** looping here, we won't be able to unload i2s driver.
				*/
				unsigned int r = 0x1;

				msleep(1 + i);
				status = gpio_get_value(AST1500_GPC4);
				if (status)
					break;

				write_register(I2S_ENGINE_CONTROL_REGISTER, 0);
				barrier();
				msleep(1);

				if (audio_info->SampleSize > 16)
					r |= 0x8;
	
				r |= (audio_info->SampleSize - 16) << 4;
				//r |= LEFT_CHANNEL_HIGH;

				write_register(I2S_ENGINE_CONTROL_REGISTER, r);
				barrier();
			}

			if (!status) {
				/* Failed. */
				printk("CPLD initialized failed.\n");
				printk("CPLD status = %x\n", status);
			} else {
				printk("CPLD retried %d times\n", i);
			}
		}

		rx_idx = 0;
		rx_free = rx_buf_num;
		rx_ready = 1;
	}
}

void kick_tx_desc(unsigned int idx)
{
	volatile TX_DESC *cur_desc;

#if 1 //Bruce120417. Test Only. Make sure pop noise comes from Fifo empty. I never see this msg. Not sure whether this code works?!
	if (read_register(I2S_ENGINE_STATUS_REGISTER) & (1<<28)) {
		printk("I2S Tx Engine Empty!!\n");
		write_register(I2S_ENGINE_STATUS_REGISTER, (1<<28));
	}
#endif

	while (kick_tx_idx != idx) {
		cur_desc = &tx_desc[kick_tx_idx];
		cur_desc->length = tx_buf_size;
		cur_desc->owner_bit = ENGINE_OWN;
		if (cur_desc->end_bit)
			kick_tx_idx = 0;
		else
			kick_tx_idx++;
	}
	write_register (I2S_TX_POLL_DEMAND_REGISTER, 1);
}


inline void kick_rx_desc(void)
{
	write_register (I2S_RX_POLL_DEMAND_REGISTER, 1);
	barrier();
}

void enable_rx(void)
{
	write_register_or (I2S_ENGINE_CONTROL_REGISTER, ENABLE_I2S_READ_SUCCESS);
	kick_rx_desc();
	rx_enabled = 1;
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

	//I2S_info("s:%d, cur:%d, free:%d\n", start_index, cur_index, free);
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

void stop_audio_engine(void)
{
	rx_enabled = rx_ready = 0;
	rx_data_available = 0;
	tx_ready = 0;
	if (IO_in_Use == IO_IN_USE_CODEC)
		disable_pwm_clock(PWM_I2S_CLK);
	//disable I2S
	ast_scu.scu_op(SCUOP_I2S_STOP, NULL);
}

inline int audio_int_handler(int *rx_int)
{
	volatile unsigned int status;

	status = read_register (I2S_ENGINE_STATUS_REGISTER);
	write_register (I2S_ENGINE_STATUS_REGISTER, (status & ENABLE_I2S_READ_SUCCESS));

	if (status & ENABLE_I2S_READ_SUCCESS)
		*rx_int = 1;
	else
		*rx_int = 0;

	return 1;
}

void disable_rx_int(void)
{
	write_register(I2S_ENGINE_CONTROL_REGISTER, read_register(I2S_ENGINE_CONTROL_REGISTER) & (~ENABLE_I2S_READ_SUCCESS));
}

void enable_rx_int(void)
{
	write_register(I2S_ENGINE_STATUS_REGISTER, ENABLE_I2S_READ_SUCCESS);
	write_register(I2S_ENGINE_CONTROL_REGISTER, read_register(I2S_ENGINE_CONTROL_REGISTER) | ENABLE_I2S_READ_SUCCESS);
}

int is_desc_own_by_eng(int desc_idx)
{
	return (tx_desc[desc_idx].owner_bit == ENGINE_OWN)?(1):(0);
}
#endif /* #if (CONFIG_AST1500_SOC_VER == 1) */

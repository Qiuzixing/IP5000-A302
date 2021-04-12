/*
 * Copyright (c) 2004-2012
 * ASPEED Technology Inc. All Rights Reserved
 * Proprietary and Confidential
 *
 * By using this code you acknowledge that you have signed and accepted
 * the terms of the ASPEED SDK license agreement.
 */
#include <linux/kernel.h>
#include <linux/delay.h>
#include <asm/arch/drivers/board_def.h>
#include <asm/arch/drivers/I2C.h>
#include "codec_hal.h"

#define I2C_WM8903_BUS_NUM    I2C_AUDIO_CODEC
#define I2C_WM8903_ADDRESS    0x34
#define I2C_WM8903_SPEED      40000

/*
 * Register values.
 */
#define WM8903_SW_RESET_AND_ID                  0x00
#define WM8903_REVISION_NUMBER                  0x01
#define WM8903_BIAS_CONTROL_0                   0x04
#define WM8903_VMID_CONTROL_0                   0x05
#define WM8903_MIC_BIAS_CONTROL_0               0x06
#define WM8903_ANALOGUE_DAC_0                   0x08
#define WM8903_ANALOGUE_ADC_0                   0x0A
#define WM8903_POWER_MANAGEMENT_0               0x0C
#define WM8903_POWER_MANAGEMENT_1               0x0D
#define WM8903_POWER_MANAGEMENT_2               0x0E
#define WM8903_POWER_MANAGEMENT_3               0x0F
#define WM8903_POWER_MANAGEMENT_4               0x10
#define WM8903_POWER_MANAGEMENT_5               0x11
#define WM8903_POWER_MANAGEMENT_6               0x12
#define WM8903_CLOCK_RATES_0                    0x14
#define WM8903_CLOCK_RATES_1                    0x15
#define WM8903_CLOCK_RATES_2                    0x16
#define WM8903_AUDIO_INTERFACE_0                0x18
#define WM8903_AUDIO_INTERFACE_1                0x19
#define WM8903_AUDIO_INTERFACE_2                0x1A
#define WM8903_AUDIO_INTERFACE_3                0x1B
#define WM8903_DAC_DIGITAL_VOLUME_LEFT          0x1E
#define WM8903_DAC_DIGITAL_VOLUME_RIGHT         0x1F
#define WM8903_DAC_DIGITAL_0                    0x20
#define WM8903_DAC_DIGITAL_1                    0x21
#define WM8903_ADC_DIGITAL_VOLUME_LEFT          0x24
#define WM8903_ADC_DIGITAL_VOLUME_RIGHT         0x25
#define WM8903_ADC_DIGITAL_0                    0x26
#define WM8903_DIGITAL_MICROPHONE_0             0x27
#define WM8903_DRC_0                            0x28
#define WM8903_DRC_1                            0x29
#define WM8903_DRC_2                            0x2A
#define WM8903_DRC_3                            0x2B
#define WM8903_ANALOGUE_LEFT_INPUT_0            0x2C
#define WM8903_ANALOGUE_RIGHT_INPUT_0           0x2D
#define WM8903_ANALOGUE_LEFT_INPUT_1            0x2E
#define WM8903_ANALOGUE_RIGHT_INPUT_1           0x2F
#define WM8903_ANALOGUE_LEFT_MIX_0              0x32
#define WM8903_ANALOGUE_RIGHT_MIX_0             0x33
#define WM8903_ANALOGUE_SPK_MIX_LEFT_0          0x34
#define WM8903_ANALOGUE_SPK_MIX_LEFT_1          0x35
#define WM8903_ANALOGUE_SPK_MIX_RIGHT_0         0x36
#define WM8903_ANALOGUE_SPK_MIX_RIGHT_1         0x37
#define WM8903_ANALOGUE_OUT1_LEFT               0x39
#define WM8903_ANALOGUE_OUT1_RIGHT              0x3A
#define WM8903_ANALOGUE_OUT2_LEFT               0x3B
#define WM8903_ANALOGUE_OUT2_RIGHT              0x3C
#define WM8903_ANALOGUE_OUT3_LEFT               0x3E
#define WM8903_ANALOGUE_OUT3_RIGHT              0x3F
#define WM8903_ANALOGUE_SPK_OUTPUT_CONTROL_0    0x41
#define WM8903_DC_SERVO_0                       0x43
#define WM8903_DC_SERVO_2                       0x45
#define WM8903_DC_SERVO_4                       0x47
#define WM8903_DC_SERVO_5                       0x48
#define WM8903_DC_SERVO_6                       0x49
#define WM8903_DC_SERVO_7                       0x4A
#define WM8903_DC_SERVO_READBACK_1              0x51
#define WM8903_DC_SERVO_READBACK_2              0x52
#define WM8903_DC_SERVO_READBACK_3              0x53
#define WM8903_DC_SERVO_READBACK_4              0x54
#define WM8903_ANALOGUE_HP_0                    0x5A
#define WM8903_ANALOGUE_LINEOUT_0               0x5E
#define WM8903_CHARGE_PUMP_0                    0x62
#define WM8903_CLASS_W_0                        0x68
#define WM8903_WRITE_SEQUENCER_0                0x6C
#define WM8903_WRITE_SEQUENCER_1                0x6D
#define WM8903_WRITE_SEQUENCER_2                0x6E
#define WM8903_WRITE_SEQUENCER_3                0x6F
#define WM8903_WRITE_SEQUENCER_4                0x70
#define WM8903_CONTROL_INTERFACE                0x72
#define WM8903_GPIO_CONTROL_1                   0x74
#define WM8903_GPIO_CONTROL_2                   0x75
#define WM8903_GPIO_CONTROL_3                   0x76
#define WM8903_GPIO_CONTROL_4                   0x77
#define WM8903_GPIO_CONTROL_5                   0x78
#define WM8903_INTERRUPT_STATUS_1               0x79
#define WM8903_INTERRUPT_STATUS_1_MASK          0x7A
#define WM8903_INTERRUPT_POLARITY_1             0x7B
#define WM8903_INTERRUPT_CONTROL                0x7E
#define WM8903_CLOCK_RATE_TEST_4                0xA4
#define WM8903_ANALOGUE_OUTPUT_BIAS_0           0xAC


#define DEVICE_ID     0x8903

static inline int I2sCodecI2cWriteWord(u8 regoffset, unsigned short value)
{
    return SetI2CWord(I2C_WM8903_BUS_NUM, I2C_WM8903_ADDRESS, regoffset, value);
}
static inline unsigned short I2sCodecI2cReadWord(u8 regoffset, unsigned short *value)
{
    return GetI2CWord(I2C_WM8903_BUS_NUM, I2C_WM8903_ADDRESS, regoffset, value);
}

/*
** Power down dac and adc.
*/
static void wm8903_power_down(void)
{
	/* Disable HP output */
	I2sCodecI2cWriteWord(WM8903_ANALOGUE_HP_0, 0x77);

	/* Turn off input PGA */
	I2sCodecI2cWriteWord(WM8903_POWER_MANAGEMENT_0,0);

	/* Start of Table 40. HP disable sequence. */
	//I2sCodecI2cWriteWord(WM8903_ANA_HP_0,0x0077);
	//I2sCodecI2cWriteWord(WM8903_ANA_HP_0,0x0000);
	/* End of Table 40. HP disable sequence. */

	/* Turn off head phone PGA */
	I2sCodecI2cWriteWord(WM8903_POWER_MANAGEMENT_2,0);

	/* Disable Line out AMPs */
	I2sCodecI2cWriteWord(WM8903_ANALOGUE_LINEOUT_0,0);
	/* Disable Line out PGAs */
	I2sCodecI2cWriteWord(WM8903_POWER_MANAGEMENT_3,0);

	/* Turn off DAC and ADC. */
	I2sCodecI2cWriteWord(WM8903_POWER_MANAGEMENT_6,0);
}


static int wm8903_init(void)
{
	u16 r;

	/*
	** Bruce161108. NOTE
	** Calling wm8903_init() will enable WM8903 (in somehow).
	** If I2S data pin has data during initialization, DAC will output pop-noise.
	*/
	/* Clock setting */
	//R15 = 0x0C08 //default value
	I2sCodecI2cWriteWord(WM8903_CLOCK_RATES_1, 0x0C08);

	/* Config I2S interface */
	// ASPEED I2S is: I2S format. //Default value
	I2sCodecI2cWriteWord(WM8903_AUDIO_INTERFACE_0, 0x0050);
	// R19[3:2]=11b word length is 32bits.
	// R19[1:0]=10b I2S format
	// R19[9]=0b LRCLK is input
	// R19[6]=0b BCLK is input
	I2sCodecI2cWriteWord(WM8903_AUDIO_INTERFACE_1, 0x000E);

	// Run default start up. So that "reference voltage VMID and the bias current" will be enabled.
	// R6C[8]=1b. Enable Write Sequencer Clock
	I2sCodecI2cWriteWord(WM8903_WRITE_SEQUENCER_0, 0x0100);
	// R16[2]=1b. Enable system clock
	I2sCodecI2cWriteWord(WM8903_CLOCK_RATES_2, 0x0004);
	// R6F[8]=1b.[5:0]=000000b. Start write sequencer at index 0.
	I2sCodecI2cWriteWord(WM8903_WRITE_SEQUENCER_3, 0x0100);
	// Check R70[0]. Wait for it turn to 0. It will take about 425ms to complete.
	printk("\nInitializing WM8903.");
	while(!I2sCodecI2cReadWord(WM8903_WRITE_SEQUENCER_4, &r) && (r & 0x0001)) {
		msleep(100);
		printk(".");
	}
	printk("\n");

	wm8903_power_down();

	return 1;
}

/*
** Power down dac and adc.
*/
static void wm8903_power_up(unsigned int enable_adc, unsigned int enable_dac)
{
	u16 r12 = 0;

	if (enable_adc) {
		/* Turn on input PGA */
		I2sCodecI2cWriteWord(WM8903_POWER_MANAGEMENT_0,0x0003);
		/* Turn on ADC. */
		r12 |= 0x0003;
	}
	if (enable_dac) {
		/* Turn on head phone PGA */
		I2sCodecI2cWriteWord(WM8903_POWER_MANAGEMENT_2,0x0003);
		r12 |= 0x000C;
	}
	I2sCodecI2cWriteWord(WM8903_POWER_MANAGEMENT_6,r12);

	if (enable_dac) {
		/* Enable HP output */
		I2sCodecI2cWriteWord(WM8903_ANALOGUE_HP_0, 0xFF);
	}
}

int wm8903_CodecExist(void)
{
	u16 r;
	int ret;

	I2CInit(I2C_WM8903_BUS_NUM, I2C_WM8903_SPEED);

	/* TODO: toggle HW reset pin. */
	/* SW reset. */
	I2sCodecI2cWriteWord(WM8903_SW_RESET_AND_ID,0x8903);

	/* Read wm8903 ID reset default value. */
	ret = I2sCodecI2cReadWord(WM8903_SW_RESET_AND_ID, &r);
	if (ret) {
		printk("WM8903 unavailable\n");
		return 0;
	}
	if (r != DEVICE_ID) {
		printk("WM8903 not expected device ID (0x%04X)?!\n", r);
		return 0;
	}

	return wm8903_init();
}

int wm8903_SetupCodec(unsigned int enable_adc, unsigned int enable_dac,
	unsigned int input_paths, unsigned int output_paths)
{
	wm8903_power_down();

	if (enable_adc) {
		switch (input_paths) {
		default:
			/* Passthrough. Use Line in */
		case CODEC_INPUT_LINE:
			I2sCodecI2cWriteWord(WM8903_ANALOGUE_LEFT_INPUT_1,0x0028);
			I2sCodecI2cWriteWord(WM8903_ANALOGUE_RIGHT_INPUT_1,0x0028);
			I2sCodecI2cWriteWord(WM8903_ANALOGUE_LEFT_INPUT_0,0x0005);
			I2sCodecI2cWriteWord(WM8903_ANALOGUE_RIGHT_INPUT_0,0x0005);
			I2sCodecI2cWriteWord(WM8903_AUDIO_INTERFACE_0, 0x0050);
			break;
		case CODEC_INPUT_MIC1:
		case CODEC_INPUT_MIC2: /* TODO */
			/*
			** MIC is typically mono input to MIC_P pin.
			** Typical board design uses "psudo differencial MIC" reference MIC_N
			** to board ground.
			** AST1520 follow above design.
			*/
			I2sCodecI2cWriteWord(WM8903_ANALOGUE_LEFT_INPUT_1,0x0000);
			I2sCodecI2cWriteWord(WM8903_ANALOGUE_RIGHT_INPUT_1,0x0046);
			I2sCodecI2cWriteWord(WM8903_ANALOGUE_LEFT_INPUT_0,0x0005);
			I2sCodecI2cWriteWord(WM8903_ANALOGUE_RIGHT_INPUT_0,0x0005);
			/* MIC is mono input. We route mono R input to both L and R channel. */
			I2sCodecI2cWriteWord(WM8903_AUDIO_INTERFACE_0, 0x00D0);
			I2sCodecI2cWriteWord(WM8903_MIC_BIAS_CONTROL_0,0x0001);
			break;
		}
	}

	if (enable_dac) {
		switch(output_paths) {
		default:
			/* Passthrough. Use HP. */
		case CODEC_OUTPUT_HP:
			/* Enable DCS on HPOUTs ONLY*/
			I2sCodecI2cWriteWord(WM8903_DC_SERVO_0,0x001C);
			/*
			** Bruce160311.
			** Set HPOUTL from 0dB (0x0039) to -12dB (0x002D) (chip default)
			** so that it won't sounds louder on client.
			*/
			/* Set HPOUTL to -12dB */
			I2sCodecI2cWriteWord(WM8903_ANALOGUE_OUT1_LEFT,0x002D);
			/* Set HPOUTR to -12dB and perform volume update */
			I2sCodecI2cWriteWord(WM8903_ANALOGUE_OUT1_RIGHT,0x00AD);
			/* Class W Enable */
			I2sCodecI2cWriteWord(WM8903_CLASS_W_0,0x0001);
			break;
		}
	}

	wm8903_power_up(enable_adc, enable_dac);
	return 1;
}

void wm8903_PowerDownCodec(void)
{
	wm8903_power_down();
}

void wm8903_analog_in_volume_cfg(int cfg)
{
	u16 val, reg, mask, range, update;

	/*
	 * use ADC volume control for line-in and microphone
	 * WM8903_ADC_DIGITAL_VOLUME_LEFT/WM8903_ADC_DIGITAL_VOLUME_RIGHT
	 * [8]: update
	 * [7:0]: 0 => mute, 0x01~0xEF => -71.625dB ~ +17.625dB (0.375dB steps)
	 *        0xF0~0xFF => 17.625dB
	 */
	mask = 0xFF;
	range = 0xEF;
	update = 0x1 << 8;

	if (cfg == -1) { /* default */
		cfg = 0xC0; /* 0xC0: 0 dB*/
		goto cfg_set;
	} else if ((cfg > 100) || (cfg < 0))
		return;

	cfg = range * cfg / 100;

cfg_set:
	reg = WM8903_ADC_DIGITAL_VOLUME_LEFT;
	I2sCodecI2cReadWord(reg, &val);
	val &= ~mask;
	val |= cfg;
	I2sCodecI2cWriteWord(reg, val);

	reg = WM8903_ADC_DIGITAL_VOLUME_RIGHT;
	I2sCodecI2cReadWord(reg, &val);
	val &= ~mask;
	val |= cfg;
	I2sCodecI2cWriteWord(reg, val | update);
}

void wm8903_analog_out_volume_cfg(int cfg)
{
	u16 val, reg, range, mask, mute, update;

	/*
	 * both client and host use HPOUTR/HPOUTL for line out
	 *
	 * WM8903_ANALOGUE_OUT1_LEFT/WM8903_ANALOGUE_OUT1_RIGHT
	 * [8]: mute
	 * [7]: update
	 * [5:0]: -57dB ~ +6dB (1dB steps)
	 */
	mask = (0x1 << 8) | 0x3F;
	range = 0x3F;
	update = 0x1 << 7;
	mute = 0x1 << 8;

	if (cfg == -1) { /* default */
		cfg = 0x39; /* 0x39: 0dB */
		goto cfg_set;
	} else if ((cfg > 100) || (cfg < 0))
		return;

	cfg = range * cfg / 100;

cfg_set:
	reg = WM8903_ANALOGUE_OUT1_LEFT;
	I2sCodecI2cReadWord(reg, &val);
	val &= ~mask;
	val |= cfg;
	if (cfg == 0)
		val |= mute;

	I2sCodecI2cWriteWord(reg, val);

	reg = WM8903_ANALOGUE_OUT1_RIGHT;
	I2sCodecI2cReadWord(reg, &val);
	val &= ~mask;
	val |= cfg;
	if (cfg == 0)
		val |= mute;

	I2sCodecI2cWriteWord(reg, val | update);
}

u32 wm8903_analog_in_volume(void)
{
	u16 val, reg, mask, range, offset;
	u32 ret = 0;

	reg = WM8903_ADC_DIGITAL_VOLUME_LEFT;
	mask = 0xFF;
	range = 0xEF;
	offset = 0;
	if (I2sCodecI2cReadWord(reg, &val) == 0) {

		ret = ((val >> offset) & mask) * 100 / range;

		if (ret > 100)
			ret = 100;
	}

	return ret;
}

u32 wm8903_analog_out_volume(void)
{
	u16 val, reg, range, offset;

	reg = WM8903_ANALOGUE_OUT1_LEFT;
	range = 0x3F;
	offset = 0;

	if (I2sCodecI2cReadWord(reg, &val) == 0)
		return ((val >> offset) & range) * 100 / range;

	return 0;
}

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
#include <asm/arch/drivers/crt.h>
#include <asm/arch/drivers/I2C.h>
#include <asm/arch/gpio.h>
#include "i2s.h"
#include "ast1500i2s.h"
#include "alc5624_reg.h"
#include "alc5624.h"

static inline int I2sCodecI2cWriteWord(u8 regoffset, unsigned short value)
{
    return SetI2CWord(I2C_AUDIO_CODEC, ALC5624_I2C_ADDRESS, regoffset, value);
}

static inline unsigned short I2sCodecI2cReadWord(u8 regoffset, unsigned short *value)
{
    return GetI2CWord(I2C_AUDIO_CODEC, ALC5624_I2C_ADDRESS, regoffset, value);
}

#if 0
//Set up CODEC master clock,. Currently PWM is used.
void setup_codec_clock(void)
{
	unsigned int port_num = PWM_CODEC_CLK;
	unsigned long flags;
	local_irq_save(flags);
	//set up multipin
	write_register (SCU_MULTIPIN_CONTROL1_REGISTER, (read_register(SCU_MULTIPIN_CONTROL1_REGISTER) & ~ENABLE_GNT4_PIN) | ENABLE_PWM4_PIN);
	//clear reset
	write_register (SCU_RESET_CONTROL_REGISTER, read_register(SCU_RESET_CONTROL_REGISTER) & ~PWM_RESET_BIT);
	local_irq_restore(flags);
	//set up M-type PWM clock & duty cycle
	write_register (PWM_CLOCK_CONTROL_REGISTER_ADDRESS, (read_register(PWM_CLOCK_CONTROL_REGISTER_ADDRESS) & 0xFFFF0000) | 0x0700);
	switch (port_num)
	{
	case 0:
		write_register (PWM_DUTY_CONTROL0_REGISTER_ADDRESS, (read_register(PWM_DUTY_CONTROL0_REGISTER_ADDRESS) & 0xFFFF0000) | 0x0703);
		break;
	case 1:
		write_register (PWM_DUTY_CONTROL0_REGISTER_ADDRESS, (read_register(PWM_DUTY_CONTROL0_REGISTER_ADDRESS) & 0x0000FFFF) | (0x0703 << 16));
		break;
	case 2:
		write_register (PWM_DUTY_CONTROL1_REGISTER_ADDRESS, (read_register(PWM_DUTY_CONTROL1_REGISTER_ADDRESS) & 0xFFFF0000) | 0x0703);
		break;
	case 3:
		write_register (PWM_DUTY_CONTROL1_REGISTER_ADDRESS, (read_register(PWM_DUTY_CONTROL1_REGISTER_ADDRESS) & 0x0000FFFF) | (0x0703 << 16));
		break;
	}
	//enable M-type PWM
	write_register (PWM_GENERAL_CONTROL_REGISTER_ADDRESS, read_register(PWM_GENERAL_CONTROL_REGISTER_ADDRESS) | 1 | (1 << (8 + port_num)));
	msleep(1);
}

//disable CODEC master clock
void disable_codec_clock(void)
{
	unsigned int port_num = PWM_CODEC_CLK;
	//disable PWM port
	write_register (PWM_GENERAL_CONTROL_REGISTER_ADDRESS, read_register(PWM_GENERAL_CONTROL_REGISTER_ADDRESS) & ~(1 << (8 + port_num)));
}
#endif

//MCLK must be ready before I2C access.
int alc5624_CodecExist(void)
{
	unsigned short wData;
	int ret;

	I2CInit(I2C_AUDIO_CODEC, I2C_ALC5624_SPEED);
	//reset
#if 0
	ret = I2sCodecI2cWriteWord(ALC5624_RESET_REG,0);
	if (ret)
	{
		printk("ALC5624 unavailable\n");
		return 0;
	}
#else
	gpio_direction_output(GPIO_AUDIO_CODEC_RESET, 0);
	msleep(10);
	gpio_direction_output(GPIO_AUDIO_CODEC_RESET, 1);
#endif
	msleep(1);

	/* read alc reset default value */
	ret = I2sCodecI2cReadWord(ALC5624_RESET_REG, &wData);

	if (ret) {
		printk("ALC5624 unavailable\n");
		return 0;
	}

	if (wData != ALC5624_RESET_VALUE) {
		printk("ALC5624 Read Reset Value Error\n");
		return 0;
	}

	/* Main Serial Data Port Control (bit15: Slave, bits[3:2] 10:24bits) */
	I2sCodecI2cWriteWord(0x34, 0x8008);
	return 1;
}

/*
1. Codec is in I2S slave mode.
2. In order to maintain synchronization between BCLK/WS and I2S master clock, the I2S mater clock is from PLL (Reg40[15] = 1) and
PLL input is BCLK (Reg42[14] = 1).
*/
/* Bruce120924. For AST15xx series project:
** Host: Line Input -> Client: HP(HeadPhone) output
** Client: MIC1 Input -> Host: HP output
** Client board uses MIC1 as MONO differencial input. Use ADC record mixer to mix MIC1 to both Left and Right.
** From Bruce's understanding, program 5624 as single-end input works better under both
** single-end and differencial MIC input.
*/
int alc5624_SetupCodec(unsigned int enable_adc, unsigned int enable_dac,
	unsigned int input_paths, unsigned int output_paths)
{
	unsigned short value;

	printk("set up ALC5624\n");
	/* 48 KHz */
	I2sCodecI2cWriteWord(0x60, 0x3075); /* Stereo DAC Colock Control 1 */
	I2sCodecI2cWriteWord(0x62, 0x1010); /* Stereo DAC Colock Control 2 */
#if 0
	if (enable_dac)
	{
		if (input_paths & (ALC5624_INPUT_MIC1 | ALC5624_INPUT_MIC2))//client
			I2sCodecI2cWriteWord(0x04,0x0f0f);
		else
			I2sCodecI2cWriteWord(0x04,0x0000);
	}
#else
	/* HP output volume */
	I2sCodecI2cWriteWord(0x04, 0x0000);
#endif

	/* ADC record gain  */
	I2sCodecI2cWriteWord(0x12, 0xf58b);

	/* MIC boost +20dB */
	if ((enable_adc) && (input_paths & (ALC5624_INPUT_MIC1 | ALC5624_INPUT_MIC2))) {
		value = 0;
		if (input_paths & ALC5624_INPUT_MIC1)
			value |= 0x0400;
		if (input_paths & ALC5624_INPUT_MIC2)
			value |= 0x0100;
		I2sCodecI2cWriteWord(0x22,value);
	}

	/* stereo dac valume */
	I2sCodecI2cWriteWord(0x0c, 0x6808);

	//Bruce120924. MIC uses differential input.
	// From Bruce's understanding, program 5624 as single-end input works better under both
	// single-end and differencial MIC input.
	I2sCodecI2cWriteWord(0x10, 0xe0e0);

	if (enable_adc) {
		value = 0x7f7f;
		if (input_paths & ALC5624_INPUT_LINE)
			value &= 0x6f6f;
		if (input_paths & ALC5624_INPUT_PHONE)
			value &= 0x7777;
		if (input_paths & ALC5624_INPUT_MIC1)
			value &= 0x3f3f;
		if (input_paths & ALC5624_INPUT_MIC2)
			value &= 0x5f5f;
		I2sCodecI2cWriteWord(0x14, value);
	}

	if (enable_dac)
		I2sCodecI2cWriteWord(0x1c, 0x0300);

	I2sCodecI2cWriteWord(0x26, 0x8000);

	value = 0xc803;
	if (enable_adc) {
		if (input_paths & ALC5624_INPUT_MIC1)
			value |= 0x0008;
		if (input_paths & ALC5624_INPUT_MIC2)
			value |= 0x0004;
	}
	I2sCodecI2cWriteWord(0x3a, value);

//steven	I2sCodecI2cWriteWord(0x3c,0x63f3);
	I2sCodecI2cWriteWord(0x3c, 0x73f3);

	value = 0x0c00;
	if (enable_adc) {
		if (input_paths & ALC5624_INPUT_MIC1)
			value |= 0x0002;
		if (input_paths & ALC5624_INPUT_MIC2)
			value |= 0x0001;
		if (input_paths & ALC5624_INPUT_LINE)
			value |= 0x00c0;
	}
	I2sCodecI2cWriteWord(0x3e, value);

	I2sCodecI2cWriteWord(0x44, 0x1ea0);
	I2sCodecI2cWriteWord(0x42, 0x4000);

//steven	I2sCodecI2cWriteWord(0x40,0x0428);
	I2sCodecI2cWriteWord(0x40, 0x8428);

	return ALC5624_OK;
}

void alc5624_PowerDownCodec(void)
{
	I2sCodecI2cWriteWord(0x26,0xef00);
}

void alc5624_analog_in_volume_cfg(int cfg)
{
	u16 val, reg, mask, range, mute, mute_mask;

	reg = 0x12;
	range = 0x1F;
	mask = 0xF9F;
	mute = 0;

	if (cfg == -1) {
		cfg = 0x0B; /* 0x0B: 0dB*/
		mute = 0;
		goto cfg_set;
	} else if ((cfg > 100) || (cfg < 0))
		return;

	if (cfg == 0)
		mute = 1;

	/* 0x0: -16.5 dB attenuation, 0x0B: 0dB, 0x1F: 30dB */
	cfg = range * cfg / 100;

cfg_set:
	I2sCodecI2cReadWord(reg, &val);
	val &= ~mask;
	/* [11:7]: ADC record gain left channel, [4:0]: ADC record gain right channel*/
	val |= ((cfg << 7) | cfg);

	I2sCodecI2cWriteWord(reg, val);

	/* mute control */
	reg = 0x14;
#if defined(CONFIG_ARCH_AST1500_HOST)
	/* host LINE_IN_L/LINE_IN_R */
	mute_mask = 0x1010; /* [12]: line-in L [4]: line-in R */
#else
	/* client MIC1P/MIC1N */
	mute_mask = 0x4040; /* [14]: MIC1  [6]: MIC1*/
#endif
	I2sCodecI2cReadWord(reg, &val);
	if (mute)
		val |= mute_mask;
	else
		val &= ~mute_mask;

	I2sCodecI2cWriteWord(reg, val);
}

void alc5624_analog_out_volume_cfg(int cfg)
{
	/* both client and host use HP_OUT_R/HP_OUT_L for line out
	 * 0x04
	 *   [15]: mute left control
	 * [12:8]: HP output left volume
	 *    [7]: mute right control
	 *  [4:0]: HP output right volume
	 */
	u16 val, reg, mask, range, mute_mask;

	reg = 0x04;
	range = 0x1F;
	mask = 0x9F9F;
	mute_mask = 0x8080; /* [15]:mute left control, [7]: mute right control */

	if (cfg == -1) { /* default */
		cfg = 0; /* 0 dB attenuation */
		mute_mask = 0;
		goto cfg_set;
	} else if ((cfg > 100) || (cfg < 0))
		return;

	if (cfg != 0)
		mute_mask = 0;

	/* 0x0: 0 dB attenuation, 0x1F: 46.5dB attenuation */
	cfg = 100 - cfg;
	cfg = range * cfg / 100;

cfg_set:
	I2sCodecI2cReadWord(reg, &val);
	val &= ~mask;
	val |= (cfg | cfg << 8);
	val |= mute_mask;
	I2sCodecI2cWriteWord(reg, val);
}

u32 alc5624_analog_in_volume(void)
{
	u16 val, reg, range;

	reg = 0x12; /* ADC record gain */
	range = 0x1F;

	if (I2sCodecI2cReadWord(reg, &val) == 0)
		return (val & range) * 100 / range;

	return 0;
}

u32 alc5624_analog_out_volume(void)
{
	u16 val, reg, range;

	reg = 0x04;
	range = 0x1F;

	if (I2sCodecI2cReadWord(reg, &val) == 0) {
		/* setting is attenuation */
		return 100 - (val & range) * 100 / range;
	}

	return 0;
}

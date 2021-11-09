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
#include "alc5640_reg.h"
#include "alc5640.h"

/*
 * Audio Line-In
 *
 * according to Presto@realtek:
 * there are 2 kind of audio device: analog supply voltage: 1.8V and 3.3V
 * and their line-in characteristics are different
 *	1.8V device: ~ 0.6 Vrms (Vmax: 0.849V)
 *	3.3V device: ~ 1 Vrms (Vmax: 1.414V)
 *
 * if our plan is to support the device with ~0.6 Vrms,
 * users need to adjust the audio volume if they connect with 3.3V devices.
 *
 * comparison of codec AVDD,
 *	ALC5640: 1.8V
 *	ALC5624: 3.3V (1 Vrms)
 *	WM8904: 1.8V
 *	WM8903: 1.8V
 */

static inline int wr16(u8 regoffset, unsigned short value)
{
	return SetI2CWord(I2C_AUDIO_CODEC, ALC5640_I2C_ADDRESS, regoffset, value);
}

static inline unsigned short rd16(u8 regoffset, unsigned short *value)
{
	return GetI2CWord(I2C_AUDIO_CODEC, ALC5640_I2C_ADDRESS, regoffset, value);
}

static void alc5640_init(void)
{
	/* software reset */
	wr16(ALC5640_RESET, 0);

	/*
	 * ALC5640_GEN_CTRL_1, 0xFA
	 * [8]: IN2 single-end input control
	 * [0]: I2S clock gating, 1: enable input clock
	 */
	wr16(ALC5640_GEN_CTRL_1, 0x3701); /* 0x3400 + [8], [0] */
	wr16(ALC5640_MICBIAS, 0x3030); /* 93[5:4] = 11'b */
	wr16(ALC5640_CLS_D_OUT, 0xa000); /* 8d[11] = 0'b */
	wr16(ALC5640_CHARGE_PUMP, 0x0e00);
	/*
	 * PR-3D
	 * [12]: enable ADC clock generator
	 * [9]: enable DAC clock generator
	 */
	wr16(ALC5640_PRIV_INDEX, 0x3D); wr16(ALC5640_PRIV_DATA, 0x3600);
	/*
	 * PR-1C, PR-1B, PR-12, PR-14, PR-20, PR-21 PR-23
	 * no descriptions on datasheet just form linux kernel 4.8 RT5640 driver
	 */
	wr16(ALC5640_PRIV_INDEX, 0x1C); wr16(ALC5640_PRIV_DATA, 0x0D21);
	wr16(ALC5640_PRIV_INDEX, 0x1B); wr16(ALC5640_PRIV_DATA, 0x0D21);
	wr16(ALC5640_PRIV_INDEX, 0x12); wr16(ALC5640_PRIV_DATA, 0x0AA8);
	wr16(ALC5640_PRIV_INDEX, 0x14); wr16(ALC5640_PRIV_DATA, 0x0AAA);
	wr16(ALC5640_PRIV_INDEX, 0x20); wr16(ALC5640_PRIV_DATA, 0x6110);
	wr16(ALC5640_PRIV_INDEX, 0x21); wr16(ALC5640_PRIV_DATA, 0xe0e0);
	wr16(ALC5640_PRIV_INDEX, 0x23); wr16(ALC5640_PRIV_DATA, 0x1804);

	/*
	 * ALC5640_I2S1_SDP, 0x70
	 * Digital interface control, (bit15: Slave, bits[3:2] 10:24bits)
	 */
	wr16(ALC5640_I2S1_SDP, 0x8008);
	wr16(ALC5640_ADDA_CLK1, 0x1114);
}

static void alc5640_multi_func_pin(void)
{
	u16 reg, val;

#if defined(CONFIG_ARCH_AST1500_HOST)
	/*
	 * IN2N/JD2
	 * IN2N is host line-in-R
	 * In IN2N microphone input function,
	 * need to disable JD2 jack detection function – MX-BB[15:13] = 000’b and MX-FB[8] = 0’b.
	 */
	reg = ALC5640_JD_CTRL;

	rd16(reg, &val);
	val &= ~(0x7 << 13); /* [15:13] = 0, disable jack detection */
	wr16(reg, val);

	reg = ALC5640_GEN_CTRL_2;
	rd16(reg, &val);
	val &= ~(0x1 << 8); /* [8]: enable JD2 function, 0: disable */
	wr16(reg, val);
#else
	/*
	 * At client
	 * IN1P: MIC1P
	 * IN1N: MIC1N
	 * In IN1P/IN1N microphone input function, need to power down DMIC interface
	 */
	reg = ALC5640_DMIC;
	rd16(reg, &val);
	val &= ~(0x1 << 15);
	wr16(reg, val);
#endif
}

/*
 * RT5640 PLL input clock range is 2.048MHz~40MHz
 * the MCLK for 176.4KHz/192KHz is 45.158MHz/49.152MHz
 * so define PLL_PRE_DIVIDER to enable PLL pre-divider (divide by 2)
 */
#define PLL_PRE_DIVIDER

static void alc5640_pll(void)
{
	u16 reg, val;

	/*
	 * From ALC5640 datasheet,
	 * For the clock requirement of MCLK must large than 512*FS as SYSCLK that FS is sample rate.
	 * If the MCLK is smaller than 512*FS, that can use internal PLL to generate higher than 512*FS clock.
	 *
	 * => system clock source: PLL, PLL source: MCLK
	 *
	 * The PLL transmit formula as below:
	 *	FOUT = (MCLK * (N+2)) / ((M+2)*(K+2))
	 *
	 * FOUT = = 512 * Fs = MCLK * X
	 * X = 2, MCLK is 256 * Fs
	 *	=> N = 6, k = 2, bypass M
	 * FOUT =  MCLK * (6 + 2) / (2 + 2)
	 */

	/*
	 * ALC5640_GLB_CLK, 0x80
	 * [15:14]: system clock source
	 *	0: MCLK
	 *	1: PLL
	 * [13:12] PLL source
	 *	0: MCLK
	 *	1: BCLK1
	 *	2: BCLK2
	 */
	reg = ALC5640_GLB_CLK;
	val = (0x1 << 14) | (0x0 << 12);
#if defined(PLL_PRE_DIVIDER)
	val |= (0x1 << 3);
#endif
	wr16(ALC5640_GLB_CLK, val);

	/*
	 * FOUT = (MCLK * (N+2)) / ((M+2)*(K+2))
	 * ALC5640_PLL_CTRL1, 0x81
	 *	[15:7]: N
	 *	[4:0] : K
	 * ALC5640_PLL_CTRL2, 0x82
	 *	[15:12]: M
	 *	[11]: bypass M
	 */
	reg = ALC5640_PLL_CTRL1;
#if defined(PLL_PRE_DIVIDER)
	/* MCLK got divided by 2, so make this factor be '4' */
	val = ((6 << 7) | 0); /* N = 6, K = 0, (6+2)/(0+2) = 4 */
#else
	val = ((6 << 7) | 2); /* N = 6, K = 2, (6+2)/(2+2) = 2 */
#endif
	wr16(reg, val);

	reg = ALC5640_PLL_CTRL2;
	val = (0x1 << 11);
	wr16(reg, val);
}

static void alc5640_power(u32 enable_adc, u32 enable_dac)
{
	u16 reg, val;

	/*
	 * ALC5640_PWR_DIG1, 0x61
	 * [15]: I2S1 power control
	 * [12]: DACL1 power control
	 * [11]: DACR1 power control
	 * [2]: ADCL power control
	 * [1]: ADCR power control
	 */
	reg = ALC5640_PWR_DIG1;
	rd16(reg, &val);
	val = (0x1 << 15);

	if (enable_dac)
		val |= ((0x1 << 12) | (0x1 << 11));

	if (enable_adc) {
		/*
		 * at client side, MIC-in only at MICBST1
		 * we route MICBST1 to both RECMIX_L and RECMIX_R,
		 * so need turn on ADCL and ADCR
		 */
		val |= (0x1 << 2);
		val |= (0x1 << 1);
	}

	wr16(reg, val);

	/*
	 * ALC5640_PWR_DIG2, 0x62
	 * [15]: stereo ADC digital filter power control => I2S1
	 * [14]: mono ADC_L digital filter power control => I2S2
	 * [13]: mono ADC_R digital filter power control => I2S2
	 * Host and client both use I2S1 (ADC1)
	 */
	reg = ALC5640_PWR_DIG2;
	rd16(reg, &val);
	val |= (0x1 << 15);
	wr16(reg, val);

	/*
	 * ALC5640_PWR_ANLG1, 0x63
	 */
	reg = ALC5640_PWR_ANLG1;
	val = ((0x1 << 15)
		| (0x1 << 14)
		| (0x1 << 13)
		| (0x1 << 11)
		| (0x1 << 7)
		| (0x1 << 6)
		| (0x1 << 5)
		| (0x1 << 4)
		| (0x1 << 3)
		| (0x1 << 2)
		);
	wr16(reg, val);

	/*
	 * ALC5640_PWR_ANLG2, 0x64
	 * [15]: MIC BIST1 power control
	 * [11]: MICBIAS1 power control
	 * [9]: PLL power control
	 */
	reg = ALC5640_PWR_ANLG2;
	val = (0x1 << 9);
#if defined(CONFIG_ARCH_AST1500_CLIENT)
	/* there is a microphone at client */
	val |= ((0x1 << 15) | (0x1 << 11));
#endif
	wr16(reg, val);
	/*
	 * ALC5640_PWR_MIXER, 0x65
	 * [15]: OUTPUTMIXL power control
	 * [14]: OUTPUTMIXR power control
	 * [11]: RECMIXL power control
	 * [10]: RECMIXR power control
	 */
	reg = ALC5640_PWR_MIXER;
	val = ((0x1 << 15)
		| (0x1 << 14)
		| (0x1 << 11)
		| (0x1 << 10)
		);
	wr16(reg, val);
	/*
	 * ALC5640_PWR_VOL, 0x66
	 * [13]: OUTVOLL power control
	 * [12]: OUTVOLR power control
	 * [11]: HPOVOLL power control
	 * [10]: HPOVOLR power control
	 * [9]: INLVOL power control
	 * [8]: INRVOL power control
	 */
	reg = ALC5640_PWR_VOL;
	val = ((0x1 << 13)
		| (0x1 << 12)
		| (0x1 << 11)
		| (0x1 << 10)
		| (0x1 << 9)
		| (0x1 << 8)
		);
	wr16(reg, val);
}

static void alc5640_mixer(u32 enable_adc, u32 enable_dac)
{
	u16 reg, val;


	if (enable_adc) {
		/*
		 * ALC5640_STO_ADC_MIXER, 0x27
		 * Host and client both use I2S1 (ADC1)
		 */
		reg = ALC5640_STO_ADC_MIXER;
		val = (
			(0x0 << 14) /* mute for ADC1 L */
			| (0x1 << 13) /* mute for ADC2 L */
			| (0x1 << 12) /* ADC1 source, 1:ADCL/ADCR */
			| (0x0 << 6) /* mute for ADC1 R */
			| (0x1 << 5) /* mute for ADC2 R */
		);
		wr16(reg, val);
#if defined(CONFIG_ARCH_AST1500_HOST)
		/*
		 * Host Line-in =>  ALC5640 IN2
		 *
		 * ALC5640_REC_L2_MIXER, 0x3C
		 * [5]: 1=> mute, 0=> unmuate for INL
		 * ALC5640_REC_R2_MIXER, 0x3E
		 * [5]: 1=> mute, 0=> unmuate for INR
		 */
		reg = ALC5640_REC_L2_MIXER;
		rd16(reg, &val);
		val &= ~(0x1 << 5);
		wr16(reg, val);

		reg = ALC5640_REC_R2_MIXER;
		rd16(reg, &val);
		val &= ~(0x1 << 5);
		wr16(reg, val);
#else
		/*
		 * ALC5640_IN1_IN2, 0x0D
		 * [15:12] IN1 boost control
		 * [7]: input mode, 0: single ended, 1: differential
		 */
		reg = ALC5640_IN1_IN2;
		rd16(reg, &val);
		val &= ~(0xf << 12);
		val |= (0x2 << 12); /* +24dB */
		val |= (0x1 << 7); /* differential */
		wr16(reg, val);

		/*
		 * Client mic-in =>  ALC5640 IN1
		 *
		 * ALC5640_REC_L2_MIXER, 0x3C
		 * [1]: 1=> mute, 0=> unmuate for MICBST1
		 * ALC5640_REC_R2_MIXER, 0x3E
		 * [1]: 1=> mute, 0=> unmuate for MICBST1
		 */
		reg = ALC5640_REC_L2_MIXER;
		rd16(reg, &val);
		val &= ~(0x1 << 1);
		wr16(reg, val);

		reg = ALC5640_REC_R2_MIXER;
		rd16(reg, &val);
		val &= ~(0x1 << 1);
		wr16(reg, val);
#endif
	}

	if (enable_dac) {
		/*
		 * ALC5640_STO_DAC_MIXER, 0x2a
		 * [14]: mute control for DACL1 to stereo DAC left mixer
		 * [10]: mute control for SNC function  to stereo DAC left mixer
		 * [6]: mute control for DACR1 to stereo DAC right mixer
		 * [2]: mute control for SNC function to stereo DAC right mixer
		 */
		reg = ALC5640_STO_DAC_MIXER;
		rd16(reg, &val);
		val &= ~((0x1 << 14) | (0x1 << 6));
		wr16(reg, val);

		/*
		 * ALC5640_DIG_MIXER, 0x2c
		 */
		reg = ALC5640_DIG_MIXER;
		val = (
			(0x0 << 15) /* mute for DACL1 to DACMIXL */
			| (0x1 << 13) /* mute for DACL2 to DACMIXL */
			| (0x0 << 11) /* mute for DACR1 to DACMIXR */
			| (0x1 << 9) /* mute for DACR2 to DACMIXR */
			);
		wr16(reg, val);
	}
}

static void alc5640_hp_control(void)
{
	u16 reg, val;

	rd16(ALC5640_PWR_ANLG1, &val);
	val = ((0x1 << 15)
		| (0x1 << 13)
		| (0x1 << 11)
		| (0x1 << 4)
		);
	wr16(ALC5640_PWR_ANLG1, val);

	/*HP power on*/
	rd16(ALC5640_DEPOP_M2, &val);
	val |= (0x1 << 13);
	wr16(ALC5640_DEPOP_M2, val);

	rd16(ALC5640_DEPOP_M1, &val);
		val = ((0x1 << 3)
		| (0x0 << 2)
		| (0x1 << 1)
		);
	wr16(ALC5640_DEPOP_M1, val);

	rd16(ALC5640_PWR_ANLG1, &val);
	val = ((0x0 << 14)
		| (0x0 << 3)
		);
	wr16(ALC5640_PWR_ANLG1, val);

	rd16(ALC5640_PWR_VOL, &val);
	val = ((0x1 << 11)
		| (0x1 << 10)
		);
	wr16(ALC5640_PWR_VOL, val);

	rd16(ALC5640_PWR_ANLG1, &val);
	val = ((0x1 << 7)
		| (0x1 << 6)
		| (0x1 << 5)
		);
	wr16(ALC5640_PWR_ANLG1, val);

	rd16(ALC5640_PWR_ANLG1, &val);
	val = ((0x1 << 14)
		| (0x1 << 3)
		);
	wr16(ALC5640_PWR_ANLG1, val);

	rd16(ALC5640_CHARGE_PUMP, &val);
	val |= ((0x2 << 8)
		);
	wr16(ALC5640_CHARGE_PUMP, val);

	rd16(ALC5640_DEPOP_M1, &val);
	val = ((0x1 << 4)
		| (0x1 << 2)
		);
	wr16(ALC5640_DEPOP_M1, val);

	/* depop parameters */
	wr16(ALC5640_PRIV_INDEX, 0x24);
	rd16(ALC5640_PRIV_DATA, &val);
	val &= ~(0x0700);
	val |= 0x200;
	wr16(ALC5640_PRIV_DATA, val);

	reg = ALC5640_DEPOP_M2;
	rd16(reg, &val);
	val |= (0x1 << 13);
	wr16(reg, val);

	reg = ALC5640_DEPOP_M1;
	rd16(reg, &val);
	val &= ~(0xD);
	val |= 0x9;
	val |= (0x1 << 4);
	wr16(reg, val);

	wr16(ALC5640_PRIV_INDEX, 0x77); wr16(ALC5640_PRIV_DATA, 0x9F00);

	/* headphone amp power on */
	reg = ALC5640_PWR_ANLG1;
	rd16(reg, &val);
	val &= ~((0x1 << 14) | (0x1 << 3));
	wr16(reg, val);
	val |= (0x1 << 5);
	wr16(reg, val);
	msleep(20);
	val |= ((0x1 << 14) | (0x1 << 3));
	val |= ((0x1 << 7) | (0x1 << 6));
	wr16(reg, val);

	/*
	 * ALC5640_HP_VOL, 0x2
	 * [15]: mute for HPOL
	 * [14]: mute for HPOVOLL
	 * [13:8]: volume control for HPOVOLL, 0x08 => 0dB, 0x10 => -12dB, 1.5dB/step
	 * [7]: mute for HPOR
	 * [6]: mute for HPOVOLR
	 * [5:0]: volume control for HPOVOLR, 0x08 => 0dB, 0x10 => -12dB
	 *
	 * set -12dB, be the same as wm8903 setting, but smaller than wm8903 output
	 * set -3dB, we'll get similar sound volume, 0x0a => -3dB
	 */
	reg = ALC5640_HP_VOL;
	val = ((0x0 << 15) | (0x0 << 14) | (0xa << 8)
		| (0x0 << 7) | (0x0 << 6) | (0xa << 0));
	wr16(reg, val);

	/*
	 * ALC5640_HPO_MIXER, 0x45
	 * [15]: mute for DAC2 to HPOMIX
	 * [14]: mute for DAC1 to HPOMIX
	 * [13]: mute for HPOVOL to HPOMIX
	 */
	reg = ALC5640_HPO_MIXER;
	rd16(reg, &val);
	val &= ~((0x1 << 15) | (0x1 << 14) | (0x1 << 13));
	val |= ((0x1 << 15) | (0x1 << 14));
	wr16(reg, val);

	/*
	 * ALC5640_OUT_L3_MIXER, 0x4F
	 * [0] : mute control DACL1 to OUTMIXL
	 */
	reg = ALC5640_OUT_L3_MIXER;
	rd16(reg, &val);
	val &= ~(0x1);
	wr16(reg, val);

	/*
	 * ALC5640_OUT_R3_MIXEL, 0x52
	 * [0] : mute control DACR1 to OUTMIXR
	 */
	reg = ALC5640_OUT_R3_MIXER;
	rd16(reg, &val);
	val &= ~(0x1);
	wr16(reg, val);

}
/* MCLK must be ready before I2C access */
int alc5640_CodecExist(void)
{
	unsigned short data;
	int ret;

	I2CInit(I2C_AUDIO_CODEC, I2C_ALC5640_SPEED);

	/* reset */
	gpio_direction_output(GPIO_AUDIO_CODEC_RESET, 0);
	msleep(10);
	gpio_direction_output(GPIO_AUDIO_CODEC_RESET, 1);
	msleep(1);

	/* read alc reset default value */
	ret = rd16(ALC5640_RESET, &data);

	if (ret) {
		printk("ALC5640 unavailable\n");
		return 0;
	}

	if (data != ALC5640_RESET_VALUE) {
		printk("ALC5640 Read Reset Value Error\n");
		return 0;
	}

	alc5640_init();

	return 1;
}

/*
 * 1. Codec is in I2S slave mode.
 * 2. In order to maintain synchronization between BCLK/WS and I2S master clock, the I2S mater clock is from PLL (Reg40[15] = 1) and
 * PLL input is BCLK (Reg42[14] = 1).
 */
/* Bruce120924. For AST15xx series project:
** Host: Line Input -> Client: HP(HeadPhone) output
** Client: MIC1 Input -> Host: HP output
** Client board uses MIC1 as MONO differencial input. Use ADC record mixer to mix MIC1 to both Left and Right.
** From Bruce's understanding, program 5640 as single-end input works better under both
** single-end and differencial MIC input.
*/
#define A_MAX_PAYLOAD 1024
int alc5640_SetupCodec(unsigned int enable_adc, unsigned int enable_dac,
	unsigned int input_paths, unsigned int output_paths)
{
	char msg[A_MAX_PAYLOAD];
	printk("set up ALC5640\n");
	alc5640_multi_func_pin();
	if (enable_dac)
		alc5640_hp_control();
	alc5640_pll();
	alc5640_power(enable_adc, enable_dac);
	alc5640_mixer(enable_adc, enable_dac);
#if defined(CONFIG_ARCH_AST1500_CLIENT)
	//qzx 2021.11.09:Before mute, the voice has to be untied. Mute has the final say from the application level
	snprintf(msg, A_MAX_PAYLOAD, "e_%s", "set_up_alc5640");
	ast_notify_user(msg);
#endif
	return ALC5640_OK;
}

void alc5640_PowerDownCodec(void)
{
	/*
	 * additional reset/init to avoid noise in unsupported audio format cases
	 * because (from customer) always get -43dBV~-55dBV noise if audio input is NLPCM (already power codec down)
	 */
	alc5640_init();

	wr16(ALC5640_PWR_DIG1, 0);
	wr16(ALC5640_PWR_DIG2, 0);
	wr16(ALC5640_PWR_ANLG1, 0);
	wr16(ALC5640_PWR_ANLG2, 0);
	wr16(ALC5640_PWR_MIXER, 0);
	wr16(ALC5640_PWR_VOL, 0);

	return;
}

void alc5640_analog_in_volume_cfg(int cfg)
{
	u16 val, reg, range, mute, mute_mask, shift;

	mute = 0;

#if defined(CONFIG_ARCH_AST1500_HOST)
	/*
	 * host INL/INR
	 * ALC5640_REC_L1_MIXER, 0x3b
	 * [12:10]: gain control for INL to RECMIXL
	 * ALC5640_REC_R1_MIXER, 0x3d
	 * [12:10]: gain control for INR to RECMIXR
	 */
	reg = ALC5640_REC_L1_MIXER;
	range = 0x7;
	shift = 10;
	mute_mask = (0x1 << 5);
#else
	/* client MIC1P/MIC1N
	 * ALC5640_REC_L2_MIXER, 0x3c
	 * [15:13]: gain control for BST1 to RECMIXL
	 * [5]: mute control for INL to RECMIXL
	 * [1]: mute control for BST1 to RECMIXL
	 * ALC5640_REC_R2_MIXER, 0x3e
	 * [15:13]: gain control for BST1 to RECMIXR
	 * [5]: mute control for INR to RECMIXR
	 * [1]: mute control for BST1 to RECMIXR
	 */
	reg = ALC5640_REC_L2_MIXER;
	range = 0x7;
	shift = 13;
	mute_mask = (0x1 << 1);
#endif
	if (cfg == -1) {
		cfg = 0x0; /* 0x0: 0dB*/
		mute = 0;
		goto cfg_set;
	} else if ((cfg > 100) || (cfg < 0))
		return;

	if (cfg == 0)
		mute = 1;

	/* 0x0: 0dB, 0x6: -18dB, 0x7: not defined */
	cfg = 100 - cfg;
	cfg = range * cfg / 100;
	if (cfg == 0x7)
		cfg = 0x6;
cfg_set:
	/* gain control, L */
	rd16(reg, &val);
	val &= ~(range << shift);
	val |= (cfg << shift);
	wr16(reg, val);

	/* gian control, R */
	reg += 2;
	rd16(reg, &val);
	val &= ~(range << shift);
	val |= (cfg << shift);
	wr16(reg, val);

	/* mute, L */
	reg = ALC5640_REC_L2_MIXER;
	rd16(reg, &val);
	if (mute)
		val |= mute_mask;
	else
		val &= ~mute_mask;
	wr16(reg, val);
	/* mute, R */
	reg += 2;
	rd16(reg, &val);
	if (mute)
		val |= mute_mask;
	else
		val &= ~mute_mask;

	wr16(reg, val);
}

void alc5640_analog_out_volume_cfg(int cfg)
{
	u16 reg, val, mask, range, mute_mask;
	/*
	 * both client and host use HP_OUT_R/HP_OUT_L for line out
	 * ALC5640_HP_VOL, 0x2
	 * [15]: mute for HPOL
	 * [14]: mute for HPOVOLL
	 * [13:8]: volume control for HPOVOLL, 0x08 => 0dB, 0x10 => -12dB, 1.5dB/step
	 * [7]: mute for HPOR
	 * [6]: mute for HPOVOLR
	 * [5:0]: volume control for HPOVOLR, 0x08 => 0dB, 0x10 => -12dB
	 */
	reg = ALC5640_HP_VOL;
	range = 0x3F;
	mask = 0xFFFF;
	mute_mask = 0xC0C0;

	if (cfg == -1) { /* default */
		cfg = 0x8; /* 0dB */
		mute_mask = 0;
		goto cfg_set;
	} else if ((cfg > 100) || (cfg < 0))
		return;

	if (cfg != 0)
		mute_mask = 0;

	// Sid 2021-10-16, For Kramer, requir 80 as 0dB
	if (cfg >= 80)
	{
		// Sid 2021-10-16, when cfg = 80, it should be 0dB as Kramer's requirement, and 100 is full range to 12dB
		cfg = 100 - cfg;	// 0~20
		cfg = (cfg * 8) / 20;	// 8 is 0dB, 0 is 12dB
	}
	else
	{
		// Sid 2021-10-16 when cfg < 80, for smooth matching from 0x3F to 8
		cfg = 80 - cfg; // 1~79
		cfg = ((cfg * (range - 8)) / 80) + 8; // 8 is 0dB, 0x3F is -82.5dB
	}

	/* 0x0: +12dB, 0x8: 0dB,  0x3F: -46.5dB */
	cfg = 100 - cfg;
	cfg = range * cfg / 100;
cfg_set:
	rd16(reg, &val);
	val &= ~mask;
	val |= (cfg | cfg << 8);
	val |= mute_mask;
	wr16(reg, val);
}

u32 alc5640_analog_in_volume(void)
{
	u16 val, reg, range, shift, cfg;

#if defined(CONFIG_ARCH_AST1500_HOST)
	/*
	 * host INL/INR
	 * ALC5640_REC_L1_MIXER, 0x3b
	 * [12:10]: gain control for INL to RECMIXL
	 * ALC5640_REC_R1_MIXER, 0x3d
	 * [12:10]: gain control for INR to RECMIXR
	 */
	reg = ALC5640_REC_L1_MIXER;
	range = 0x7;
	shift = 10;
#else
	/* client MIC1P/MIC1N
	 * ALC5640_REC_L2_MIXER, 0x3c
	 * [15:13]: gain control for BST1 to RECMIXL
	 * [5]: mute control for INL to RECMIXL
	 * [1]: mute control for BST1 to RECMIXL
	 * ALC5640_REC_R2_MIXER, 0x3e
	 * [15:13]: gain control for BST1 to RECMIXR
	 * [5]: mute control for INR to RECMIXR
	 * [1]: mute control for BST1 to RECMIXR
	 */
	reg = ALC5640_REC_L2_MIXER;
	range = 0x7;
	shift = 13;
#endif

	rd16(reg, &val);
	cfg = (val >> shift) & range;
	cfg = (range - cfg) * 100 / range;

	return cfg;
}

u32 alc5640_analog_out_volume(void)
{
	u16 reg, val, range, cfg;
	/*
	 * both client and host use HP_OUT_R/HP_OUT_L for line out
	 * ALC5640_HP_VOL, 0x2
	 * [15]: mute for HPOL
	 * [14]: mute for HPOVOLL
	 * [13:8]: volume control for HPOVOLL, 0x08 => 0dB, 0x10 => -12dB, 1.5dB/step
	 * [7]: mute for HPOR
	 * [6]: mute for HPOVOLR
	 * [5:0]: volume control for HPOVOLR, 0x08 => 0dB, 0x10 => -12dB
	 */
	reg = ALC5640_HP_VOL;
	range = 0x3F;

	rd16(reg, &val);
	cfg = val & range;

	cfg = (range - cfg) * 100 / range;

	return cfg;
}

u32 alc5640_reg_rd(u32 reg)
{
	u16 val;

	rd16(reg, &val);

	return val & 0xFFFF;
}

void alc5640_reg_wr(u32 reg, u32 data)
{
	wr16(reg, data);
}

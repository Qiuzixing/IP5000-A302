/*
 * Copyright (c) 2004-2012
 * ASPEED Technology Inc. All Rights Reserved
 * Proprietary and Confidential
 *
 * By using this code you acknowledge that you have signed and accepted
 * the terms of the ASPEED SDK license agreement.
 */
#include "alc5624.h"
#include "alc5640.h"
#include "wm8903.h"


int (*SetupCodec)(unsigned int enable_adc, unsigned int enable_dac,
	unsigned int input_paths, unsigned int output_paths);

void (*PowerDownCodec)(void);

void (*analog_in_volume_cfg)(int);
void (*analog_out_volume_cfg)(int);
unsigned int (*analog_in_volume)(void);
unsigned int (*analog_out_volume)(void);

unsigned int (*codec_reg_rd)(unsigned int offset);
void (*codec_reg_wr)(unsigned int offset, unsigned int data);

int CodecExist(void)
{
	int exist = 0;

	if (wm8903_CodecExist()) {
		SetupCodec = wm8903_SetupCodec;
		PowerDownCodec = wm8903_PowerDownCodec;
		analog_in_volume_cfg = wm8903_analog_in_volume_cfg;
		analog_out_volume_cfg = wm8903_analog_out_volume_cfg;
		analog_in_volume = wm8903_analog_in_volume;
		analog_out_volume = wm8903_analog_out_volume;
		codec_reg_rd = 0;
		codec_reg_wr = 0;
		exist = 1;
	} else if (alc5624_CodecExist()) {
		SetupCodec = alc5624_SetupCodec;
		PowerDownCodec = alc5624_PowerDownCodec;
		analog_in_volume_cfg = alc5624_analog_in_volume_cfg;
		analog_out_volume_cfg = alc5624_analog_out_volume_cfg;
		analog_in_volume = alc5624_analog_in_volume;
		analog_out_volume = alc5624_analog_out_volume;
		codec_reg_rd = 0;
		codec_reg_wr = 0;
		exist = 1;
	} else if (alc5640_CodecExist()) {
		SetupCodec = alc5640_SetupCodec;
		PowerDownCodec = alc5640_PowerDownCodec;
		analog_in_volume_cfg = alc5640_analog_in_volume_cfg;
		analog_out_volume_cfg = alc5640_analog_out_volume_cfg;
		analog_in_volume = alc5640_analog_in_volume;
		analog_out_volume = alc5640_analog_out_volume;
		codec_reg_rd = alc5640_reg_rd;
		codec_reg_wr = alc5640_reg_wr;
		exist = 1;
	}

	return exist;
}

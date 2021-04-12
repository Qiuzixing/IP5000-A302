/*
 * Copyright (c) 2004-2012
 * ASPEED Technology Inc. All Rights Reserved
 * Proprietary and Confidential
 *
 * By using this code you acknowledge that you have signed and accepted
 * the terms of the ASPEED SDK license agreement.
 */

#ifndef _ALC5640_H_
#define _ALC5640_H_

#include "codec_hal.h"

#define ALC5640_ERROR                           0
#define ALC5640_OK                              1

/* input paths */
#define ALC5640_INPUT_LINE	CODEC_INPUT_LINE
#define ALC5640_INPUT_PHONE	CODEC_INPUT_PHONE
#define ALC5640_INPUT_MIC1	CODEC_INPUT_MIC1
#define ALC5640_INPUT_MIC2	CODEC_INPUT_MIC2
/* output paths */
#define ALC5640_OUTPUT_SPK	CODEC_OUTPUT_SPK
#define ALC5640_OUTPUT_HP	CODEC_OUTPUT_HP
#define ALC5640_OUTPUT_MONO	CODEC_OUTPUT_MONO

int alc5640_CodecExist(void);
int alc5640_SetupCodec(unsigned int enable_adc, unsigned int enable_dac, unsigned int input_path, unsigned int output_path);
void alc5640_PowerDownCodec(void);
void alc5640_analog_in_volume_cfg(int cfg);
void alc5640_analog_out_volume_cfg(int cfg);
unsigned int alc5640_analog_in_volume(void);
unsigned int alc5640_analog_out_volume(void);
unsigned int alc5640_reg_rd(unsigned int offset);
void alc5640_reg_wr(unsigned int offset, unsigned int data);

#endif /* #ifndef _ALC5640_H_ */

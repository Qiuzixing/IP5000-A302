/*
 * Copyright (c) 2004-2012
 * ASPEED Technology Inc. All Rights Reserved
 * Proprietary and Confidential
 *
 * By using this code you acknowledge that you have signed and accepted
 * the terms of the ASPEED SDK license agreement.
 */

#ifndef _ALC5624_H_
#define _ALC5624_H_

#include "codec_hal.h"

#define ALC5624_ERROR                           0
#define ALC5624_OK                              1

/* input paths */
#define ALC5624_INPUT_LINE	CODEC_INPUT_LINE
#define ALC5624_INPUT_PHONE	CODEC_INPUT_PHONE
#define ALC5624_INPUT_MIC1	CODEC_INPUT_MIC1
#define ALC5624_INPUT_MIC2	CODEC_INPUT_MIC2
/* output paths */
#define ALC5624_OUTPUT_SPK	CODEC_OUTPUT_SPK
#define ALC5624_OUTPUT_HP	CODEC_OUTPUT_HP
#define ALC5624_OUTPUT_MONO	CODEC_OUTPUT_MONO

int alc5624_CodecExist(void);
int alc5624_SetupCodec(unsigned int enable_adc, unsigned int enable_dac, unsigned int input_path, unsigned int output_path);
void alc5624_PowerDownCodec(void);
void alc5624_analog_in_volume_cfg(int cfg);
void alc5624_analog_out_volume_cfg(int cfg);
unsigned int alc5624_analog_in_volume(void);
unsigned int alc5624_analog_out_volume(void);

#endif /* #ifndef _ALC5624_H_ */

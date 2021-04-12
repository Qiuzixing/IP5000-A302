/*
 * Copyright (c) 2004-2012
 * ASPEED Technology Inc. All Rights Reserved
 * Proprietary and Confidential
 *
 * By using this code you acknowledge that you have signed and accepted
 * the terms of the ASPEED SDK license agreement.
 */

#ifndef __CODEC_HAL_H__
#define __CODEC_HAL_H__

/*
** Define for input/output path
*/
//input paths
#define CODEC_INPUT_LINE	0x1
#define CODEC_INPUT_PHONE	0x2
#define CODEC_INPUT_MIC1	0x4
#define CODEC_INPUT_MIC2	0x8
//output paths
#define CODEC_OUTPUT_SPK	0x1
#define CODEC_OUTPUT_HP	    0x2
#define CODEC_OUTPUT_MONO	0x4


int CodecExist(void);
extern int (*SetupCodec)(unsigned int enable_adc, unsigned int enable_dac,
	unsigned int input_paths, unsigned int output_paths);

extern void (*PowerDownCodec)(void);
extern void (*analog_in_volume_cfg)(int cfg);
extern void (*analog_out_volume_cfg)(int cfg);
extern unsigned int (*analog_in_volume)(void);
extern unsigned int (*analog_out_volume)(void);
extern unsigned int (*codec_reg_rd)(unsigned int offset);
extern void (*codec_reg_wr)(unsigned int offset, unsigned int data);
#endif /* #ifndef __CODEC_HAL_H__ */


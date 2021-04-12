/*
 * Copyright (c) 2004-2012
 * ASPEED Technology Inc. All Rights Reserved
 * Proprietary and Confidential
 *
 * By using this code you acknowledge that you have signed and accepted
 * the terms of the ASPEED SDK license agreement.
 */
#ifndef __WM8903_H__
#define __WM8903_H__

int wm8903_CodecExist(void);
int wm8903_SetupCodec(unsigned int enable_adc, unsigned int enable_dac, unsigned int input_path, unsigned int output_path);
void wm8903_PowerDownCodec(void);
void wm8903_analog_in_volume_cfg(int cfg);
void wm8903_analog_out_volume_cfg(int cfg);
unsigned int wm8903_analog_in_volume(void);
unsigned int wm8903_analog_out_volume(void);

#endif /* #ifndef __WM8903_H__ */

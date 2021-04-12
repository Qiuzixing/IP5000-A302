/*
 * Copyright (c) 2004-2012
 * ASPEED Technology Inc. All Rights Reserved
 * Proprietary and Confidential
 *
 * By using this code you acknowledge that you have signed and accepted
 * the terms of the ASPEED SDK license agreement.
 */

#ifndef _ALC5624_REG_H_
#define _ALC5624_REG_H_

#define ALC5624_I2C_ADDRESS     0x30
#define I2C_ALC5624_SPEED     40000

#define ALC5624_RESET_REG                       0x00
    #define ALC5624_RESET_VALUE                 0x59B4

#define ALC5624_POWER_DOWN_CONTROL              0x26
    #define ALC5624_PR7_NORMAL                  0 << 15
    #define ALC5624_SPEAKER_AMP_PWD             1 << 15
    #define ALC5624_PR6_NORMAL                  0 << 14
    #define ALC5624_HP_PWD                      1 << 14
    #define ALC5624_PR5_NORMAL                  0 << 13
    #define ALC5624_INT_CLK_OFF                 1 << 13
    #define ALC5624_PR3_NORMAL                  0 << 11
    #define ALC5624_MIXER_PWD_1                 1 << 11
    #define ALC5624_PR2_NORMAL                  0 << 10
    #define ALC5624_MIXER_PWD_2                 1 << 10
    #define ALC5624_PR1_NORMAL                  0 << 9
    #define ALC5624_STEREO_DAC_PWD              1 << 9
    #define ALC5624_PR0_MORMAL                  0 << 8
    #define ALC5624_STEREO_ADC_PWD              1 << 8
    #define ALC5624_VREF_STATUS_NORMAL          1 << 3
    #define ALC5624_ANALOG_MIXER_READY_STATUS   1 << 2
    #define ALC5624_DAC_READY_STATUS            1 << 1
    #define ALC5624_ADC_READY_STATUS            1 << 0
    #define ALC5624_PD_CONTROL_MASK             (ALC5624_SPEAKER_AMP_PWD | ALC5624_HP_PWD | ALC5624_INT_CLK_OFF | ALC5624_MIXER_PWD_1 | \
                                                 ALC5624_MIXER_PWD_2 | ALC5624_STEREO_DAC_PWD | ALC5624_STEREO_ADC_PWD )

#define ALC5624_VENDOR_ID1                      0x7C

#endif

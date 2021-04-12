/*
 * Copyright (c) 2004-2012
 * ASPEED Technology Inc. All Rights Reserved
 * Proprietary and Confidential
 *
 * By using this code you acknowledge that you have signed and accepted
 * the terms of the ASPEED SDK license agreement.
 */

#ifndef _ALC5640_REG_H_
#define _ALC5640_REG_H_

#define ALC5640_I2C_ADDRESS     0x38
#define I2C_ALC5640_SPEED     40000

/* Info */
#define ALC5640_RESET			0x00
	#define ALC5640_RESET_VALUE		0x000C
#define ALC5640_VENDOR_ID		0xfd
#define ALC5640_VENDOR_ID1		0xfe
#define ALC5640_VENDOR_ID2		0xff
/* I/O - Output */
#define ALC5640_SPK_VOL			0x01
#define ALC5640_HP_VOL			0x02
#define ALC5640_OUTPUT			0x03
#define ALC5640_MONO_OUT		0x04
/* Dummy */
#define ALC5640_DUMMY_PR3F		0x05
/* I/O - Input */
#define ALC5640_IN1_IN2			0x0d
#define ALC5640_IN3_IN4			0x0e
#define ALC5640_INL_INR_VOL		0x0f
/* I/O - ADC/DAC/DMIC */
#define ALC5640_DAC1_DIG_VOL		0x19
#define ALC5640_DAC2_DIG_VOL		0x1a
#define ALC5640_DAC2_CTRL		0x1b
#define ALC5640_ADC_DIG_VOL		0x1c
#define ALC5640_ADC_DATA		0x1d
#define ALC5640_ADC_BST_VOL		0x1e
/* Mixer - D-D */
#define ALC5640_STO_ADC_MIXER		0x27
#define ALC5640_MONO_ADC_MIXER		0x28
#define ALC5640_AD_DA_MIXER		0x29
#define ALC5640_STO_DAC_MIXER		0x2a
#define ALC5640_MONO_DAC_MIXER		0x2b
#define ALC5640_DIG_MIXER		0x2c
#define ALC5640_DSP_PATH1		0x2d
#define ALC5640_DSP_PATH2		0x2e
#define ALC5640_DIG_INF_DATA		0x2f
/* Mixer - ADC */
#define ALC5640_REC_L1_MIXER		0x3b
#define ALC5640_REC_L2_MIXER		0x3c
#define ALC5640_REC_R1_MIXER		0x3d
#define ALC5640_REC_R2_MIXER		0x3e

/* Mixer - DAC */
#define ALC5640_HPO_MIXER		0x45
#define ALC5640_SPK_L_MIXER		0x46
#define ALC5640_SPK_R_MIXER		0x47
#define ALC5640_SPO_L_MIXER		0x48
#define ALC5640_SPO_R_MIXER		0x49
#define ALC5640_SPO_CLSD_RATIO		0x4a
#define ALC5640_MONO_MIXER		0x4c
#define ALC5640_OUT_L1_MIXER		0x4d
#define ALC5640_OUT_L2_MIXER		0x4e
#define ALC5640_OUT_L3_MIXER		0x4f
#define ALC5640_OUT_R1_MIXER		0x50
#define ALC5640_OUT_R2_MIXER		0x51
#define ALC5640_OUT_R3_MIXER		0x52
#define ALC5640_LOUT_MIXER		0x53
/* Power */
#define ALC5640_PWR_DIG1		0x61
#define ALC5640_PWR_DIG2		0x62
#define ALC5640_PWR_ANLG1		0x63
#define ALC5640_PWR_ANLG2		0x64
#define ALC5640_PWR_MIXER		0x65
#define ALC5640_PWR_VOL			0x66
/* Private Register Control */
#define ALC5640_PRIV_INDEX		0x6a
#define ALC5640_PRIV_DATA		0x6c
/* Format - ADC/DAC */
#define ALC5640_I2S1_SDP		0x70
#define ALC5640_I2S2_SDP		0x71
#define ALC5640_ADDA_CLK1		0x73
#define ALC5640_ADDA_CLK2		0x74
#define ALC5640_DMIC			0x75
/* Function - Analog */
#define ALC5640_GLB_CLK			0x80
#define ALC5640_PLL_CTRL1		0x81
#define ALC5640_PLL_CTRL2		0x82
#define ALC5640_ASRC_1			0x83
#define ALC5640_ASRC_2			0x84
#define ALC5640_ASRC_3			0x85
#define ALC5640_ASRC_4			0x89
#define ALC5640_ASRC_5			0x8a
#define ALC5640_HP_OVCD			0x8b
#define ALC5640_CLS_D_OVCD		0x8c
#define ALC5640_CLS_D_OUT		0x8d
#define ALC5640_DEPOP_M1		0x8e
#define ALC5640_DEPOP_M2		0x8f
#define ALC5640_DEPOP_M3		0x90
#define ALC5640_CHARGE_PUMP		0x91
#define ALC5640_PV_DET_SPK_G		0x92
#define ALC5640_MICBIAS			0x93
/* Function - Digital */
#define	ALC5640_EQ_CTRL1		0xb0
#define	ALC5640_EQ_CTRL2		0xb1
#define	ALC5640_WIND_FILTER		0xb2
#define	ALC5640_DRC_AGC_1		0xb4
#define	ALC5640_DRC_AGC_2		0xb5
#define	ALC5640_DRC_AGC_3		0xb6
#define	ALC5640_SVOL_ZC			0xb7
#define	ALC5640_ANC_CTRL1		0xb8
#define	ALC5640_ANC_CTRL2		0xb9
#define	ALC5640_ANC_CTRL3		0xba
#define	ALC5640_JD_CTRL			0xbb
#define	ALC5640_ANC_JD			0xbc
#define	ALC5640_IRQ_CTRL1		0xbd
#define	ALC5640_IRQ_CTRL2		0xbe
#define	ALC5640_INT_IRQ_ST		0xbf
#define	ALC5640_GPIO_CTRL1		0xc0
#define	ALC5640_GPIO_CTRL2		0xc1
#define	ALC5640_GPIO_CTRL3		0xc2
#define	ALC5640_DSP_CTRL1		0xc4
#define	ALC5640_DSP_CTRL2		0xc5
#define	ALC5640_DSP_CTRL3		0xc6
#define	ALC5640_DSP_CTRL4		0xc7
#define	ALC5640_PGM_REG_ARR1		0xc8
#define	ALC5640_PGM_REG_ARR2		0xc9
#define	ALC5640_PGM_REG_ARR3		0xca
#define	ALC5640_PGM_REG_ARR4		0xcb
#define	ALC5640_PGM_REG_ARR5		0xcc
#define	ALC5640_SCB_FUNC		0xcd
#define	ALC5640_SCB_CTRL		0xce
#define	ALC5640_BASE_BACK		0xcf
#define	ALC5640_MP3_PLUS1		0xd0
#define	ALC5640_MP3_PLUS2		0xd1
#define	ALC5640_3D_HP			0xd2
#define	ALC5640_ADJ_HPF			0xd3
#define	ALC5640_HP_CALIB_AMP_DET	0xd6
#define	ALC5640_HP_CALIB2		0xd7
#define	ALC5640_SV_ZCD1			0xd9
#define ALC5640_SV_ZCD2			0xda

#define ALC5640_GEN_CTRL_1		0xfa
#define ALC5640_GEN_CTRL_2		0xfb
#define ALC5640_GEN_CTRL_3		0xfc

/* Index of Codec Private Register definition */
#define ALC5640_BIAS_CUR1			0x12
#define ALC5640_BIAS_CUR3			0x14
#define ALC5640_CLSD_INT_REG1			0x1c
#define ALC5640_CHPUMP_INT_REG1			0x24
#define ALC5640_MAMP_INT_REG2			0x37
#define ALC5640_CHOP_DAC_ADC			0x3d
#define ALC5640_MIXER_INT_REG			0x3f
#define ALC5640_3D_SPK				0x63
#define ALC5640_WND_1				0x6c
#define ALC5640_WND_2				0x6d
#define ALC5640_WND_3				0x6e
#define ALC5640_WND_4				0x6f
#define ALC5640_WND_5				0x70
#define ALC5640_WND_8				0x73
#define ALC5640_DIP_SPK_INF			0x75
#define ALC5640_HP_DCC_INT1			0x77
#define ALC5640_EQ_BW_LOP			0xa0
#define ALC5640_EQ_GN_LOP			0xa1
#define ALC5640_EQ_FC_BP1			0xa2
#define ALC5640_EQ_BW_BP1			0xa3
#define ALC5640_EQ_GN_BP1			0xa4
#define ALC5640_EQ_FC_BP2			0xa5
#define ALC5640_EQ_BW_BP2			0xa6
#define ALC5640_EQ_GN_BP2			0xa7
#define ALC5640_EQ_FC_BP3			0xa8
#define ALC5640_EQ_BW_BP3			0xa9
#define ALC5640_EQ_GN_BP3			0xaa
#define ALC5640_EQ_FC_BP4			0xab
#define ALC5640_EQ_BW_BP4			0xac
#define ALC5640_EQ_GN_BP4			0xad
#define ALC5640_EQ_FC_HIP1			0xae
#define ALC5640_EQ_GN_HIP1			0xaf
#define ALC5640_EQ_FC_HIP2			0xb0
#define ALC5640_EQ_BW_HIP2			0xb1
#define ALC5640_EQ_GN_HIP2			0xb2
#define ALC5640_EQ_PRE_VOL			0xb3
#define ALC5640_EQ_PST_VOL			0xb4
#endif /* #ifndef _ALC5640_REG_H_ */

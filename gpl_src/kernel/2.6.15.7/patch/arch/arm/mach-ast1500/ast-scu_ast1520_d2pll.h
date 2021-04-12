/*
 *
 * Copyright (C) 2012-2020  ASPEED Technology Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 *   History      :
 *    1. 2014/07/16 Bruce first version
 *
********************************************************************************/

#ifndef _AST_SCU_AST1520_D2PLL_H_
#define _AST_SCU_AST1520_D2PLL_H_

#define MAC_RGMII_CLOCK_SELECT_D2_PLL

#if defined(MAC_RGMII_CLOCK_SELECT_D2_PLL)
/*
 * MAC RGMII 125MHz clock source selection: internal PLL
 * variable internal PLL source could be D2-PLL or H-PLL (CPU)
 * internal PLL divider ratio: div 2 ~ div 8
 * so choose D2-PLL and set it to 750MHz, divider ratio: 6
 */
#define D2_PLL_DEFAULT 7500000 /* in 100Hz */
#else
#define D2_PLL_DEFAULT 8000000 /* in 100Hz */
#endif
/*
** D2-PLL table used for I2S engine. According to HDMI CTS, clock must < 100ppm in average.
*/
struct d2pll_table_t {
	u32 clk; //in 100Hz
	u32 scu01c;
	u32 scu13c;
	u32 scu140;
	u32 scu144;
} d2pll_table[] =
{
//{clk(100Hz),   SCU1C,     SCU13C,     SCU140,     SCU144 }
#if defined(MAC_RGMII_CLOCK_SELECT_D2_PLL)
{  7500000, 0xC440217C, 0x00000580, 0x00000000, 0x00000000 }, // integer mode, Clock(750000000),M(125),N( 2),P( 2),OD( 1),Fout(750000000),err_int(       0Hz,    0ppm)
#else
{    81920, 0xAC5E6046, 0x00000588, 0x00000000, 0x0007FF52 }, //Clock(  8192000),M( 71),N( 1),P(52),OD( 4),Fout(  8192307),err_int(     307Hz,   37ppm),Fout_frac( 32767998),Frac(524114[0x0007FF52]),err_frac(-0.0510ppm)
{   112896, 0xAC5D204E, 0x00000588, 0x00000000, 0x000006F7 }, //Clock( 11289600),M( 79),N( 1),P(42),OD( 4),Fout( 11285714),err_int(   -3886Hz, -344ppm),Fout_frac( 45158404),Frac(  1783[0x000006F7]),err_frac( 0.0812ppm)
{   122880, 0x938D202A, 0x00000588, 0x00000000, 0x0000020C }, //Clock( 12288000),M( 43),N( 1),P(42),OD( 2),Fout( 12285714),err_int(   -2286Hz, -186ppm),Fout_frac( 24575997),Frac(   524[0x0000020C]),err_frac(-0.1022ppm)
{   225792, 0xAC4D204E, 0x00000588, 0x00000000, 0x000006F7 }, //Clock( 22579200),M( 79),N( 1),P(42),OD( 2),Fout( 22571428),err_int(   -7772Hz, -344ppm),Fout_frac( 45158404),Frac(  1783[0x000006F7]),err_frac( 0.0812ppm)
{   245760, 0x9385202A, 0x00000588, 0x00000000, 0x0000020C }, //Clock( 24576000),M( 43),N( 1),P(42),OD( 1),Fout( 24571428),err_int(   -4572Hz, -186ppm),Fout_frac( 24575997),Frac(   524[0x0000020C]),err_frac(-0.1022ppm)
{   451584, 0xAC45204E, 0x00000588, 0x00000000, 0x000006F7 }, //Clock( 45158400),M( 79),N( 1),P(42),OD( 1),Fout( 45142857),err_int(  -15543Hz, -344ppm),Fout_frac( 45158404),Frac(  1783[0x000006F7]),err_frac( 0.0812ppm)
{   491520, 0xAC44E051, 0x00000588, 0x00000000, 0x0007EB86 }, //Clock( 49152000),M( 82),N( 1),P(40),OD( 1),Fout( 49200000),err_int(   48000Hz,  977ppm),Fout_frac( 49151999),Frac(519046[0x0007EB86]),err_frac(-0.0224ppm)
{  8000000, 0xC44022C7, 0x00000580, 0x00000000, 0x0004AB3C }, //integer-m mode, Clock(800000000),M(200),N( 3),P( 2),OD( 1),Fout(800000000),err_int(       0Hz,    0ppm)
#endif
};

#if 0 //Jazoe version
//   clk        scu01c        scu13c         scu140        scu144
{  81920,   0xac7b2046,   0x00000588,    0x00000000,   0x0007ff52 },
{ 112896,   0xac7a804e,   0x00000588,    0x00000000,   0x000006f7 },
{ 122880,   0xac63003f,   0x00000580,    0x00000000,   0x00000000 },
{ 225792,   0xac71604e,   0x00000588,    0x00000000,   0x000006f7 },
{ 245760,   0x93b0a02a,   0x00000588,    0x00000000,   0x0000020c },
{ 451584,   0xac70a04e,   0x00000588,    0x00000000,   0x000006f7 },
{ 491520,   0x93b0402a,   0x00000588,    0x00000000,   0x0000020c },
#endif

unsigned int d2pll_table_size = sizeof(d2pll_table) / sizeof(struct d2pll_table_t);

struct audio_gen_lock_pll_table_t {
	u32 clk; /* in 100Hz */
	u32 r_scu1cc; /* R */
	u32 n_scu1d0; /* N */
	u32 q_scu1d4; /* Q */
} audio_gen_lock_pll_table[] = {
	/* {clk(100Hz),	SCU1CC,		SCU1D0,		SCU1D4} */
#if defined(MAC_RGMII_CLOCK_SELECT_D2_PLL)
	/* D2-PLL is 750MHz */
	{81920,		0x00000018,	0x0000001F,	0x0000002D},
	{112896,	0x00000008,	0x00000025,	0x00000021},
	{122880,	0x0000000F,	0x0000001D,	0x0000001E},
	{225792,	0x0000001f,	0x00000033,	0x00000010},
	{245760,	0x0000000F,	0x0000003A,	0x0000000F},
	{451584,	0x00000034,	0x000000AB,	0x00000008},
	{491520,	0x0000005A,	0x0000008F,	0x00000007},
#else
	/* D2-PLL is 800MHz */
	{81920,		0x00000035,	0x00000040,	0x00000030},
	{112896,	0x000000BE,	0x000001B9,	0x00000023},
	{122880,	0x00000035,	0x00000060,	0x00000020},
	{225792,	0x00000277,	0x00000372,	0x00000011},
	{245760,	0x00000035,	0x000000C0,	0x00000010},
	{451584,	0x000005E9,	0x000006E4,	0x00000008},
	{491520,	0x00000035,	0x00000180,	0x00000008},
#endif
};

unsigned int audio_gen_lock_pll_table_size = sizeof(audio_gen_lock_pll_table) / sizeof(struct audio_gen_lock_pll_table_t);
#endif

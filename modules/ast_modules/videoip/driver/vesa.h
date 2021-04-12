/*
 * Copyright (c) 2004-2012
 * ASPEED Technology Inc. All Rights Reserved
 * Proprietary and Confidential
 *
 * By using this code you acknowledge that you have signed and accepted
 * the terms of the ASPEED SDK license agreement.
 */

#ifndef _VESA_H_
#define _VESA_H_

#include <asm/arch/drivers/video_hal.h>

#if !defined(_HDMITX_H_) && !defined(_CAT6611_SYS_H_)
typedef enum tagHDMI_Video_Type {
	HDMI_Unkown = 0,
	HDMI_640x480p60 = 1,
	HDMI_480p60,
	HDMI_480p60_16x9,
	HDMI_720p60,
	HDMI_1080i60,
	HDMI_480i60,
	HDMI_480i60_16x9,
	HDMI_1080p60 = 16,
	HDMI_576p50,
	HDMI_576p50_16x9,
	HDMI_720p50,
	HDMI_1080i50,
	HDMI_576i50,
	HDMI_576i50_16x9,
	HDMI_1080p50 = 31,
	HDMI_1080p24,
	HDMI_1080p25,
	HDMI_1080p30,
	HDMI_3840x2160p50_420 = 96,
	HDMI_3840x2160p60_420 = 97,
	HDMI_4096x2160p24 = 98,
	HDMI_4096x2160p25 = 99,
	HDMI_4096x2160p30 = 100,
	HDMI_4096x2160p50_420 = 101,
	HDMI_4096x2160p60_420 = 102,
} HDMI_Video_Type;
#endif

typedef enum
{
	VCLK25_175	= 0x00,
	VCLK28_322	= 0x01,
	VCLK31_5	= 0x02,
	VCLK31_574	= 0x03,
	VCLK32_76	= 0x04,
	VCLK33_154	= 0x05,
	VCLK36		= 0x06,
	VCLK40		= 0x07,
	VCLK45_978	= 0x08,
	VCLK49_5	= 0x09,
	VCLK50		= 0x0A,
	VCLK52_95	= 0x0B,
	VCLK56_25	= 0x0C,
	VCLK65		= 0x0D,
	VCLK74_48	= 0x0E,
	VCLK75		= 0x0F,
	VCLK78_75	= 0x10,
	VCLK79_373	= 0x11,
	VCLK81_624	= 0x12,
	VCLK83_462	= 0x13,
	VCLK84_715	= 0x14,
	VCLK94_5	= 0x15,
	VCLK106_5	= 0x16,
	VCLK108		= 0x17,
	VCLK119		= 0x18,
	VCLK135		= 0x19,
	VCLK136_358 = 0x1A,
	VCLK138_5   = 0x1B,
	VCLK146_25	= 0x1C,
	VCLK148_5   = 0x1D,
	VCLK154		= 0x1E,
	VCLK157_5	= 0x1F,
	VCLK162		= 0x20
} ePIXEL_CLOCK;

typedef enum {
	GenByVESA	= 0,
	GenByGTF,
	GenByCVT,
	GenByCVT_RB,
	GenByHDTV, //4x3
	GenByHDTV_16x9,
	GenByCVT_HDTV,
	GenByCVT_RB_HDTV,
	GenByOther
} eGenClock;

typedef struct {
	unsigned short HActive;
	unsigned short VActive;
	unsigned char RefreshRate;
	unsigned char HorPolarity; //eVPolarity
	unsigned char VerPolarity; //eVPolarity
	unsigned short HTotal;
	unsigned short VTotal;
	unsigned short HSyncTime;
	unsigned short VSyncTime;
	unsigned short HFPorch; /* H Front Porch */
	unsigned short VFPorch; /* V Front Porch */
	unsigned short HBPorch; /* H Back Porch */
	unsigned short VBPorch; /* V Back Porch */
	unsigned long DCLK10000;  /* DCLK*10000, where DCLK in MHz; unit of DCLK10000 is 100Hz */
	unsigned long PixelClock; /* Value from SCU14 */
	e_hdmi_vic HDMIType;
	unsigned char GenClock;
	unsigned char ADC03;	//Ref table1
	unsigned char ScanMode; //eVScanMode
	unsigned short sn; /* serial number */

} MODE_ITEM;

#define P_NEG NegPolarity
#define P_POS PosPolarity

#if (CONFIG_AST1500_SOC_VER >= 2)
#define DCLK9883_LIMIT      200
#else
#define DCLK9883_LIMIT      140
#endif

#define TV_TIMING_TABLE_SIZE(crt) ((crt)->mode_table_vesa_start)  /*(32)*/
#define HDMI_TIMING_START 0
#define VESA_TIMING_START 0x4000 /* 16384 */
#define USER_TIMING_START 0x8000 /* 32768 */
#define HDMI_SN(a) (HDMI_TIMING_START + a)
#define VESA_SN(a) (VESA_TIMING_START + a)
#define USER_SN(a) (USER_TIMING_START + a)
#define TIMING_SN_INVALID 0xFFFF
#define TIMING_INDEX_INVALID 0xFFFF

#define DEFAULT_TIMING_IDX 0  //640x480@60Hz
#define PREFERRED_DEFAULT_TIMING_SN      (HDMI_SN(13)) //1080p60Hz
#define PREFERRED_DEFAULT_4K_TIMING_SN   (HDMI_SN(107)) //2160p30Hz

#endif /*#ifndef _VESA_H_*/

/*
 * timing table includes,
 *	1. HDTV timing, SNs start from 0
 *	2. VESA timing, SNs start from VESA_TIMING_START
 *	3. User-defined timing, SNs start from USER_TIMING_START
 *
 * Note:
 * - New items MUST always append to the end of one of above table. (For backward compatibility).
 * - NEVER remove item listed. (For backward compatibility).
 * - SN MUST be in order and mapping to table index. (In order to speed up sn_to_index()).
 * - Add items to User-defined table if you are not ASPEED developer.
 */

#ifdef VESA_GLOBALS
MODE_ITEM AstModeTable[] = {
/******************************************************************************************************************************************************************/
/* HDTV Timing */
/* -HA  -VA   -Rate  -Hor   -Ver    -HT    -VT  -Hs -Vs  -HFP -VFP -HBP -VBP  -DCLK10000 -PClk       -HDMIType    -GCLK     -ADC03 -ScanMode    -SN */
{640,   480,    60, P_NEG, P_NEG,  800,  525,   96,  2,   16,  10,  48,  33,   251750, 0x00086A2A, VIC_DMT0659,  GenByHDTV,   0x28, Prog,  0}, /*  #1, 640x480p@60Hz */
{720,   480,    60, P_NEG, P_NEG,  858,  525,   62,  6,   16,   9,  60,  30,   270000, 0x00086824, VIC_480p,      GenByHDTV,  0x30, Prog,  1}, /*  #2, 720x480p@60Hz 4:3 */
{720,   480,    60, P_NEG, P_NEG,  858,  525,   62,  6,   16,   9,  60,  30,   270000, 0x00086824, VIC_480pH, GenByHDTV_16x9, 0x30, Prog,  2}, /*  #3, 720x480p@60Hz 16:9 */
{1280,  720,    60, P_POS, P_POS, 1650,  750,   40,  5,  110,   5, 220,  20,   741759, 0x00006B44, VIC_720p,  GenByHDTV_16x9, 0x70, Prog,  3}, /*  #4, 1280x720p@60Hz 16:9 */
{1920, 1080/2,  60, P_POS, P_POS, 2200,  562,   44,  5,   88,   2, 148,  15,   741759, 0x00006B44, VIC_1080i, GenByHDTV_16x9, 0x70, Interl, 4}, /*  # 5, 1920x1080i@60Hz 16:9 */
{1440/2, 480/2, 60, P_NEG, P_NEG, 1716/2, 262, 124/2, 3, 38/2, 4, 114/2, 15, 270000/2, 0x000C6824, VIC_480i,   GenByHDTV,     0x30, Interl, 5}, /*  #6, 720(1440)x480i@60hz 4:3 */
{1440/2, 480/2, 60, P_NEG, P_NEG, 1716/2, 262, 124/2, 3, 38/2, 4, 114/2, 15, 270000/2, 0x000C6824, VIC_480iH, GenByHDTV_16x9, 0x30, Interl, 6}, /*  # 7, 720(1440)x480i@60hz 16:9 */
{1440/2,  240,  60, P_NEG, P_NEG, 1716/2, 262, 124/2, 3, 38/2, 4, 114/2, 15, 270000/2, 0x000C6824, VIC_240p,   GenByHDTV,     0x30, Prog, 7}, /*  # 8&9_mode1 720(1440)x240p@60Hz 4:3 */
{1440/2,  240,  60, P_NEG, P_NEG, 1716/2, 262, 124/2, 3, 38/2, 4, 114/2, 15, 270000/2, 0x000C6824, VIC_240pH, GenByHDTV_16x9, 0x30, Prog, 8},/*  #8&9_mode1 720(1440)x240p@60Hz 16:9 */
{1440/2,  240,  60, P_NEG, P_NEG, 1716/2, 263, 124/2, 3, 38/2, 5, 114/2, 15, 270000/2, 0x000C6824, VIC_240p,    GenByHDTV,    0x30, Prog, 9}, /*  # 8&9_mode2 720(1440)x240p@60Hz 4:3 */
{1440/2,  240,  60, P_NEG, P_NEG, 1716/2, 263, 124/2, 3, 38/2, 5, 114/2, 15, 270000/2, 0x000C6824, VIC_240pH, GenByHDTV_16x9, 0x30, Prog, 10}, /* #8&9_mode2 720(1440)x240p@60Hz 16:9 */
{1440,  480,  60, P_NEG, P_NEG, 1716,  525,  124,  6,   32,   9, 120,   30,   540000, 0x00006824, VIC_480p2x,  GenByHDTV,      0x70, Prog, 11}, /* #14, 1440x480p@60Hz 4:3 */
{1440,  480,  60, P_NEG, P_NEG, 1716,  525,  124,  6,   32,   9, 120,   30,   540000, 0x00006824, VIC_480p2xH, GenByHDTV_16x9, 0x70, Prog, 12}, /* #15, 1440x480p@60Hz 16:9 */
#if 0
{1920, 1080,  60, P_POS, P_POS, 2200, 1125,   44,  5,   88,   4, 148,   36,  1485000, 0x00004A3E, VIC_1080p,  GenByHDTV_16x9,  0xF0,  Prog, 13}, /* #16, 1920x1080p@60Hz 16:9 */
#else
/* pclk change to 0x4b44 for better 59.94Hz compatibility */
{1920, 1080,  60, P_POS, P_POS, 2200, 1125,   44,  5,   88,   4, 148,   36,  1483517, 0x00004B44, VIC_1080p,  GenByHDTV_16x9, 0xF0,   Prog, 13}, /* #16, 1920x1080p@60Hz 16:9 */
#endif
/* -HA  -VA   -Rate  -Hor   -Ver    -HT    -VT  -Hs -Vs  -HFP -VFP -HBP -VBP  -DCLK10000 -PClk       -HDMIType    -GCLK     -ADC03 -ScanMode    -SN */
{720,   576,    50, P_NEG, P_NEG,  864,  625,   64,  5,   12,   5,  68,  39,   270000, 0x00086824, VIC_576p,      GenByHDTV,  0x30,    Prog, 14}, /* #17, 720x576p@50Hz 4:3 */
{720,   576,    50, P_NEG, P_NEG,  864,  625,   64,  5,   12,   5,  68,  39,   270000, 0x00086824, VIC_576pH, GenByHDTV_16x9, 0x30,  Prog, 15}, /* #18, 720x576p@50Hz 16:9 */
{1280,  720,    50, P_POS, P_POS, 1980,  750,   40,  5,  440,   5, 220,  20,   742500, 0x00006B44, VIC_720p50, GenByHDTV_16x9,  0x70,  Prog, 16}, /* #19, 1280x720p@50Hz 16:9 */
{1920, 1080/2,  50, P_POS, P_POS, 2640, 562,   44,  5,  528,   2, 148,   15,   742500, 0x00006B44, VIC_1080i25,  GenByHDTV_16x9, 0x70,  Interl, 17}, /* #20, 1920x1080i@50Hz 16:9 */
{1440/2, 576/2, 50, P_NEG, P_NEG, 1728/2, 312, 126/2, 3, 24/2, 2, 138/2, 19, 270000/2, 0x000C6824, VIC_576i,  GenByHDTV, 0x30,  Interl,  18}, /* #21, 720(1440)x576i@50Hz 4:3 */
{1440/2, 576/2, 50, P_NEG, P_NEG, 1728/2, 312, 126/2, 3, 24/2, 2, 138/2, 19, 270000/2, 0x000C6824, VIC_576iH, GenByHDTV_16x9, 0x30, Interl, 19}, /* #22, 720(1440)x576i@50Hz 16:9 */
{1440/2,  288,  50, P_NEG, P_NEG, 1728/2, 312, 126/2, 3, 24/2, 2, 138/2, 19, 270000/2, 0x000C6824, VIC_288p,  GenByHDTV,     0x30,    Prog, 20}, /* #23&24_mode1 720(1440)x288p@50Hz 4:3 */
{1440/2,  288,  50, P_NEG, P_NEG, 1728/2, 312, 126/2, 3, 24/2, 2, 138/2, 19, 270000/2, 0x000C6824, VIC_288pH, GenByHDTV_16x9, 0x30, Prog,  21}, /* #23&24_mode1 720(1440)x288p@50Hz 16:9 */
{1440/2,  288,  50, P_NEG, P_NEG, 1728/2, 313, 126/2, 3, 24/2, 3, 138/2, 19, 270000/2, 0x000C6824, VIC_288p,  GenByHDTV,      0x30, Prog,  22}, /* #23&24_mode2, 720(1440)x288p@50Hz 4:3 */
{1440/2,  288,  50, P_NEG, P_NEG, 1728/2, 313, 126/2, 3, 24/2, 3, 138/2, 19, 270000/2, 0x000C6824, VIC_288pH, GenByHDTV_16x9, 0x30, Prog, 23}, /* #23&24_mode2, 720(1440)x288p@50Hz 16:9 */
{1440/2,  288,  50, P_NEG, P_NEG, 1728/2, 314, 126/2, 3, 24/2, 4, 138/2, 19, 270000/2, 0x000C6824, VIC_288p,  GenByHDTV,      0x30, Prog,  24}, /* #23&24_mode3, 720(1440)x288p@50Hz 4:3 */
{1440/2,  288,  50, P_NEG, P_NEG, 1728/2, 314, 126/2, 3, 24/2, 4, 138/2, 19, 270000/2, 0x000C6824, VIC_288pH, GenByHDTV_16x9, 0x30, Prog, 25}, /* #23&24_mode3, 720(1440)x288p@50Hz 16:9 */
{1440,  576,  50, P_NEG, P_POS, 1728,  625,  128,  5,   24,   5, 136,   39,   540000, 0x00006824, VIC_576p2x,   GenByHDTV,       0x70,    Prog, 26}, /* #29&30, 1440x576p@50Hz 4:3 */
{1440,  576,  50, P_NEG, P_POS, 1728,  625,  128,  5,   24,   5, 136,   39,   540000, 0x00006824, VIC_576p2xH,  GenByHDTV_16x9,  0x70,    Prog, 27}, /* #29&30, 1440x576p@50Hz 16:9 */
{1920, 1080,  50, P_POS, P_POS, 2640, 1125,   44,  5,  528,   4, 148,   36,  1485000, 0x00004B44, VIC_1080p50,  GenByHDTV_16x9,  0xF0,    Prog, 28}, /* #31, 1920x1080p@50Hz 16:9 */
{1920, 1080,  24, P_POS, P_POS, 2750, 1125,   44,  5,  638,   4, 148,   36,   741759, 0x00006B44, VIC_1080p24,  GenByHDTV_16x9,   0x70, Prog,  29}, /* #32, 1920x1080p@24Hz 16:9 */
{1920, 1080,  25, P_POS, P_POS, 2640, 1125,   44,  5,  528,   4, 148,   36,   742500, 0x00006B44, VIC_1080p25,  GenByHDTV_16x9,   0x70,  Prog,  30}, /* #33, 1920x1080p@25Hz 16:9 */
/* HDTV with wrong polarity */
/* -HA  -VA   -Rate  -Hor   -Ver    -HT    -VT  -Hs -Vs  -HFP -VFP -HBP -VBP  -DCLK10000 -PClk       -HDMIType    -GCLK     -ADC03 -ScanMode    -SN */
{1440/2, 240,   60, P_POS, P_POS, 1716/2, 262, 124/2, 3, 38/2,  4, 114/2, 15, 270000/2, 0x000C6824, VIC_240p, GenByHDTV,     0x30,    Prog,  31}, /* #8&9_mode1 720(1440)x240p@60Hz 4:3 */
{1440/2, 480/2, 60, P_POS, P_POS, 1716/2, 262+1, 124/2, 3, 38/2, 4, 114/2, 15, 270000/2, 0x000C6824, VIC_480i, GenByHDTV,    0x30,  Interl,  32}, /* #6, 720(1440)x480i@60hz 4:3 */
{1440/2, 576/2, 50, P_POS, P_POS, 1728/2, 312+1, 126/2, 3, 24/2, 2, 138/2, 19, 270000/2, 0x000C6824, VIC_576i, GenByHDTV,    0x30,  Interl,  33}, /* #21, 720(1440)x576i@50Hz 4:3 */
{1440/2, 288,   50, P_POS, P_POS, 1728/2, 312, 126/2,  3, 24/2, 2, 138/2, 19, 270000/2, 0x000C6824, VIC_288p, GenByHDTV,     0x30,    Prog,  34},/* #23&24_mode1 720(1440)x288p@50Hz 4:3 */
{720,    480,   60, P_POS, P_POS,  858,  525,   62,  6,   16,   9,  60,   30,   270000, 0x00086824, VIC_480p, GenByHDTV,     0x30,    Prog,  35}, /* #2, 720x480p@60Hz 4:3 */
{720,    576,   50, P_POS, P_POS,  864,  625,   64,  5,   12,   5,  68,   39,   270000, 0x00086824, VIC_576p, GenByHDTV,     0x30,    Prog,  36}, /* #17, 720x576p@50Hz 4:3 */
{1280,   720,   50, P_NEG, P_NEG, 1980,  750,   40,  5,  440,   5, 220,   20,   742500, 0x00006B44, VIC_720p50, GenByHDTV_16x9, 0x70, Prog,  37}, /* #19, 1280x720p@50Hz 16:9 */
{1280,   720,   60, P_NEG, P_NEG, 1650,  750,   40,  5,  110,   5, 220,   20,   741759, 0x00006B44, VIC_720p, GenByHDTV_16x9,  0x70,  Prog,  38}, /* #4, 1280x720p@60Hz 16:9 */
{1440,   480,   60, P_POS, P_POS, 1716,  525,  124,  6,   32,   9, 120,   30,   540000, 0x00006824, VIC_480p2x, GenByHDTV,     0x70,   Prog,  39}, /* #14, 1440x480p@60Hz 4:3 */
{1440,   576,   50, P_NEG, P_NEG, 1728,  625,  128,  5,   24,   5, 136,   39,   540000, 0x00006824, VIC_576p2x, GenByHDTV,     0x70,   Prog,  40}, /* #29&30, 1440x576p@50Hz 4:3 */
{1920, 1080/2,  50, P_NEG, P_NEG, 2640, 562,   44,  5,  528,   2, 148,   15,    742500, 0x00006B44, VIC_1080i25, GenByHDTV_16x9, 0x70, Interl, 41 }, /* #20, 1920x1080i@50Hz 16:9 */
{1920, 1080/2,  60, P_NEG, P_NEG, 2200, 562,   44,  5,   88,   2,  148,  15,    741759, 0x00006B44, VIC_1080i, GenByHDTV_16x9,   0x70, Interl,  42}, /* #5, 1920x1080i@60Hz 16:9 */
{1920, 1080,    24, P_NEG, P_NEG, 2750, 1125,   44,  5,  638,   4,  148,  36,   741759, 0x00006B44, VIC_1080p24, GenByHDTV_16x9, 0x70, Prog,  43}, /* #32, 1920x1080p@24Hz 16:9 */
{1920, 1080,    25, P_NEG, P_NEG, 2640, 1125,   44,  5,  528,   4, 148,   36,   742500, 0x00006B44, VIC_1080p25, GenByHDTV_16x9, 0x70, Prog,  44}, /* #33, 1920x1080p@25Hz 16:9 */
{1920, 1080,    30, P_NEG, P_NEG, 2200, 1125,   44,  5,   88,   4, 148,   36,   741759, 0x00006B44, VIC_1080p30, GenByHDTV_16x9, 0x70, Prog,  45}, /* #34, 1920x1080p@30Hz 16:9 */
{1920, 1080,    50, P_NEG, P_NEG, 2640, 1125,   44,  5,  528,   4, 148,   36,  1485000, 0x00004B44, VIC_1080p50, GenByHDTV_16x9, 0xF0, Prog,  46}, /* #31, 1920x1080p@50Hz 16:9 */
{1920, 1080,    60, P_NEG, P_NEG, 2200, 1125,   44,  5,   88,   4, 148,   36,  1483517, 0x00004B44, VIC_1080p, GenByHDTV_16x9,   0xF0, Prog,  47}, /* #16, 1920x1080p@60Hz 16:9 */

/*CEA-861-D Timing*/
/* -HA  -VA   -Rate  -Hor   -Ver    -HT    -VT  -Hs -Vs  -HFP -VFP -HBP -VBP  -DCLK10000 -PClk       -HDMIType    -GCLK     -ADC03 -ScanMode    -SN */
{1280,  720, 24, P_POS, P_POS, 3300,  750, 40, 5, 1760,  5, 220, 20,  593407, 0x0006C3B, VIC_720p24,  GenByHDTV_16x9, 0x48, Prog, 48},/* 678.CEA-1280X720P-24 16:9 */
{1280,  720, 25, P_POS, P_POS, 3960,  750, 40, 5, 2420,  5, 220, 20,  742500, 0x0006B44, VIC_720p25,  GenByHDTV_16x9, 0x48, Prog, 49},/* 679.CEA-1280X720P-25 16:9 */
{1280,  720, 30, P_POS, P_POS, 3300,  750, 40, 5, 1760,  5, 220, 20,  741759, 0x0006B44, VIC_720p30,  GenByHDTV_16x9, 0x48, Prog, 50},/* 680.CEA-1280X720P-30 16:9 */
{720*2, 240, 60, P_NEG, P_NEG, 858*2, 262, 62*2, 3, 38,  4, 57*2, 15, 270000, 0x00086824, VIC_480i,  GenByHDTV, 0x48, Interl, 51},/* 612.CEA-1440X480i-60 4:3 */
{720*2, 240, 60, P_NEG, P_NEG, 858*2, 262, 62*2, 3, 38,  4, 57*2, 15, 270000, 0x00086824, VIC_480iH, GenByHDTV, 0x48, Interl, 52},/* 614.CEA-1440X480i-60 16:9 */
{720*2, 240, 60, P_NEG, P_NEG, 858*2, 263, 62*2, 3, 38,  5, 57*2, 15, 310260, 0x00086C3E, VIC_240p,  GenByHDTV, 0x48, Prog, 53}, /* 616.CEA-1440X 240P-60 4:3 */
{720*2, 240, 60, P_NEG, P_NEG, 858*2, 263, 62*2, 3, 38,  5, 57*2, 15, 310260, 0x00086C3E, VIC_240pH,  GenByHDTV, 0x48, Prog, 54}, /* 618.CEA-1440X 240P-60 16:9 */
{720*4, 240, 60, P_NEG, P_NEG, 858*4, 262, 62*4, 3, 76,  4, 57*4, 15, 620520, 0x00006C3E, VIC_480i4x,  GenByHDTV, 0x48, Interl, 55},/* 620.CEA-2880X480i-60 4:3 */
{720*4, 240, 60, P_NEG, P_NEG, 858*4, 262, 62*4, 3, 76,  4, 57*4, 15, 620520, 0x00006C2E, VIC_480i4xH,  GenByHDTV, 0x48, Interl, 56},/* 622.CEA-2880X480i-60 16:9 */
{720*4, 240, 60, P_NEG, P_NEG, 858*4, 263, 62*4, 3, 76,  5, 57*4, 15, 620520, 0x00006C2E, VIC_240p4x,  GenByHDTV, 0x48, Prog, 57}, /* 624.CEA-2880X240P-60 4:3 */
{720*4, 240, 60, P_NEG, P_NEG, 858*4, 262, 62*4, 3, 76,  4, 57*4, 15, 620520, 0x00006C2E, VIC_240p4xH,  GenByHDTV, 0x48, Prog, 58}, /* 626.CEA-2880X240P-60 16:9 */
{720*2, 288, 50, P_NEG, P_NEG, 864*2, 312, 63*2, 3, 24,  2, 69*2, 19, 270000, 0x00086824, VIC_576i,  GenByHDTV, 0x48, Interl, 59},/* 637.CEA-1440X576i-50 4:3 */
{720*2, 288, 50, P_NEG, P_NEG, 864*2, 312, 63*2, 3, 24,  2, 69*2, 19, 270000, 0x00086824, VIC_576iH, GenByHDTV, 0x48, Interl, 60},/* 638.CEA-1440X576i-50 16:9 */
{720*2, 288, 50, P_NEG, P_NEG, 864*2, 312, 63*2, 3, 24,  2, 69*2, 19, 270000, 0x00086824, VIC_288p,  GenByHDTV, 0x48, Prog, 61}, /* 639.CEA-1440X288P-50 4:3 */
{720*2, 288, 50, P_NEG, P_NEG, 864*2, 312, 63*2, 3, 24,  2, 69*2, 19, 270000, 0x00086824, VIC_288pH,  GenByHDTV, 0x48, Prog, 62}, /* 640.CEA-1440X288P-50 16:9 */
{720*4, 288, 50, P_NEG, P_NEG, 864*4, 312, 63*4, 3, 48,  2, 69*4, 19, 540000, 0x00006824, VIC_576i4x,  GenByHDTV, 0x48, Interl, 63},/* 641.CEA-2880X576i-50 4:3 */
{720*4, 288, 50, P_NEG, P_NEG, 864*4, 312, 63*4, 3, 48,  2, 69*4, 19, 540000, 0x00006824, VIC_576i4xH,  GenByHDTV, 0x48, Interl, 64},/* 642.CEA-2880X576i-50 16:9 */
{720*4, 288, 50, P_NEG, P_NEG, 864*4, 312, 63*4, 3, 48,  2, 69*4, 19, 540000, 0x00006824, VIC_288p4x,  GenByHDTV, 0x48, Prog, 65}, /* 643.CEA-2880X288P-50 4:3 */
{720*4, 288, 50, P_NEG, P_NEG, 864*4, 312, 63*4, 3, 48,  2, 69*4, 19, 540000, 0x00006824, VIC_288p4xH,  GenByHDTV, 0x48, Prog, 66}, /* 644.CEA-2880X288P-50 16:9 */
{720*2, 576, 50, P_NEG, P_NEG, 864*2, 625, 64*2, 5, 24,  5, 68*2, 39, 540000, 0x00006824, VIC_576p2x,  GenByHDTV, 0x48, Prog, 67}, /* 645.CEA-1440X576P-50 4:3 */
{720*2, 576, 50, P_NEG, P_NEG, 864*2, 625, 64*2, 5, 24,  5, 68*2, 39, 540000, 0x00006824, VIC_576p2xH,  GenByHDTV, 0x48, Prog, 68}, /* 646.CEA-1440X576P-50 16:9 */
{1920, 1080,  30, P_POS, P_POS, 2200, 1125, 44,  5,  88,  4, 148, 36,  741759, 0x00006B44, VIC_1080p30,  GenByHDTV, 0x48, Prog, 69}, /* 652.CEA-1920X1080P-30 16:9 */
{720*4, 480,  60, P_NEG, P_NEG, 858*4, 525, 62*4, 6,  64,  9, 60*4, 30, 1080000, 0x00004824, VIC_480p4x,  GenByHDTV, 0x48, Prog, 70}, /* 653.CEA-2880x480P-60 4:3 */
{720*4, 480,  60, P_NEG, P_NEG, 858*4, 525, 62*4, 6,  64,  9, 60*4, 30, 1080000, 0x00004824, VIC_480p4xH,   GenByHDTV, 0x48, Prog, 71}, /* 654.CEA-2880x480P-60 16:9 */
{720*4, 576,  50, P_NEG, P_NEG, 864*4, 625, 64*4, 5,  48,  5, 68*4, 39, 1081080, 0x00004824, VIC_576p4x,  GenByHDTV, 0x48, Prog, 72}, /* 655.CEA-2880x576P-50 4:3 */
{720*4, 576,  50, P_NEG, P_NEG, 864*4, 625, 64*4, 5,  48,  5, 68*4, 39, 1081080, 0x00004824, VIC_576p4xH,  GenByHDTV, 0x48, Prog, 73}, /* 656.CEA-2880x576P-50 16:9 */
{1920,  540,  50, P_POS, P_NEG, 2304,  625, 168, 5,  32, 22, 184, 57,  720000, 0x0000672A, VIC_1080i25A,  GenByHDTV, 0x48, Interl, 74},/* 657.CEA-1250X1080i-50 16:9 */
/*CEA-861-D Timing with very high refresh rate. */
{1920,  540, 100, P_POS, P_POS, 2640,  562, 44,  5, 528,  2, 148, 15, 1485000, 0x00004B44, VIC_1080i50,  GenByHDTV, 0x48, Interl, 75},/* 658.CEA-1920X1080i-100 16:9 */
{1280,  720, 100, P_POS, P_POS, 1980,  750, 40,  5, 440,  5, 220, 20, 1485000, 0x00004B44, VIC_720p100,  GenByHDTV, 0x48, Prog, 76}, /* 659.CEA-1280X720P-100 16:9 */
{ 720,  576, 100, P_NEG, P_NEG,  864,  625, 64,  5,  12,  5,  68, 39,  540000, 0x00006824, VIC_576p100,  GenByHDTV, 0x48, Prog, 77}, /* 660.CEA-720X576P-100 4:3 */
{ 720,  576, 100, P_NEG, P_NEG,  864,  625, 64,  5,  12,  5,  68, 39,  540000, 0x00006824, VIC_576p100H,  GenByHDTV, 0x48, Prog, 78}, /* 661.CEA-720X576P-100 16:9 */
{ 720,  288, 100, P_NEG, P_NEG,  864,  312, 63,  3,  12,  2,  69, 19,  270000, 0x00086824, VIC_576i50,  GenByHDTV, 0x48, Interl, 79},/* 662.CEA-720X576i-100 4:3 */
{ 720,  288, 100, P_NEG, P_NEG,  864,  312, 63,  3,  12,  2,  69, 19,  270000, 0x00086824, VIC_576i50H,  GenByHDTV, 0x48, Interl, 80},/* 663.CEA-720X576i-100 16:9 */
{1920,  540, 120, P_POS, P_POS, 2200,  562, 44,  5,  88,  2, 148, 15, 1483517, 0x00004B44, VIC_1080i60,  GenByHDTV, 0x48, Interl, 81},/* 664.CEA-1920X1080i-120 16:9 */
{1280,  720, 120, P_POS, P_POS, 1650,  750, 40,  5, 110,  5, 220, 20, 1483517, 0x00004B44, VIC_720p120,  GenByHDTV, 0x48, Prog, 82}, /* 665.CEA-1280X720P-120 16:9 */
{ 720,  480, 120, P_NEG, P_NEG,  858,  525, 62,  6,  16,  9,  60, 30,  540000, 0x00006824, VIC_480p119,  GenByHDTV, 0x48, Prog, 83}, /* 666.CEA-720X480P-120 4:3 */
{ 720,  480, 120, P_NEG, P_NEG,  858,  525, 62,  6,  16,  9,  60, 30,  540000, 0x00006824, VIC_480p119H,  GenByHDTV, 0x48, Prog, 84}, /* 667.CEA-720X480P-120 16:9 */
{ 720,  240, 120, P_NEG, P_NEG,  858,  262, 62,  3,  19,  4,  57, 15,  270000, 0x00086824, VIC_480i59,  GenByHDTV, 0x48, Interl, 85},/* 668.CEA-720X480i-120 4:3 */
{ 720,  240, 120, P_NEG, P_NEG,  858,  262, 62,  3,  19,  4,  57, 15,  270000, 0x00086824, VIC_480i59H,  GenByHDTV, 0x48, Interl, 86},/* 669.CEA-720X480i-120 16:9 */
{ 720,  576, 200, P_NEG, P_NEG,  864,  625, 64,  5,  12,  5,  68, 39, 1080000, 0x00004824, VIC_576p200,  GenByHDTV, 0x48, Prog, 87}, /* 670.CEA-720X576P-200 4:3 */
{ 720,  576, 200, P_NEG, P_NEG,  864,  625, 64,  5,  12,  5,  68, 39, 1080000, 0x00004824, VIC_576p200H,  GenByHDTV, 0x48, Prog, 88}, /* 671.CEA-720X576P-200 16:9 */
{ 720,  288, 200, P_NEG, P_NEG,  864,  312, 63,  3,  12,  2,  69, 19,  540000, 0x00006824, VIC_576i100,  GenByHDTV, 0x48, Interl, 89},/* 672.CEA-720X576i-200 4:3 */
{ 720,  288, 200, P_NEG, P_NEG,  864,  312, 63,  3,  12,  2,  69, 19,  540000, 0x00006824, VIC_576i100H,  GenByHDTV, 0x48, Interl, 90},/* 673.CEA-720X576i-200 16:9 */
{ 720,  480, 240, P_NEG, P_NEG,  858,  525, 62,  6,  16,  9,  60, 30, 1080000, 0x00004824, VIC_480p239,  GenByHDTV, 0x48, Prog, 91}, /* 674.CEA-720X480P-240 4:3 */
{ 720,  480, 240, P_NEG, P_NEG,  858,  525, 62,  6,  16,  9,  60, 30, 1080000, 0x00004824, VIC_480p239H,  GenByHDTV, 0x48, Prog, 92}, /* 675.CEA-720X480P-240 16:9 */
{ 720,  240, 240, P_NEG, P_NEG,  858,  262, 62,  3,  19,  4,  57, 15,  540540, 0x00006824, VIC_480i119,  GenByHDTV, 0x48, Interl, 93},/* 676.CEA-720X480i-240 4:3 */
{ 720,  240, 240, P_NEG, P_NEG,  858,  262, 62,  3,  19,  4,  57, 15,  540540, 0x00006824, VIC_480i119H,  GenByHDTV, 0x48, Interl, 94},/* 677.CEA-720X480i-240 16:9 */
/* frame packing 3D timings */
/* -HA  -VA   -Rate  -Hor   -Ver    -HT    -VT  -Hs -Vs  -HFP -VFP -HBP -VBP  -DCLK10000 -PClk       -HDMIType    -GCLK     -ADC03 -ScanMode    -SN */
{1280, 1470,  60, P_POS, P_POS, 1650, 1500, 40, 5,  110,  5, 220,  20, 1485000,  0x00004B44, VIC_720p,    GenByHDTV,    0x48, Prog, 95},/* 1280x720p @ 59.94 / 60Hz */
{1280, 1470,  50, P_POS, P_POS, 1980, 1500, 40, 5,  440,  5, 220,  20, 1485000,  0x00004B44, VIC_720p50, GenByHDTV,    0x48, Prog, 96},/* 1280x720p @ 50Hz */
{1280, 1470,  24, P_NEG, P_NEG, 1650, 1500, 40*2, 5, 70,  5, 220,  20,  594000,  0x0006C3B,  VIC_720p24, GenByHDTV,    0x48, Prog, 97},/* 1280x720p @ 24Hz */
{1280, 1470,  30, P_NEG, P_NEG, 3300, 1500, 40*2, 5, 1720, 5, 220, 20, 1485000,  0x00004B44, VIC_720p30,    GenByHDTV,    0x48, Prog, 98},/* 1280x720p @ 30Hz */
{1920, 2205,  24, P_POS, P_POS, 2750, 2250, 44, 5,  638,  4, 148,  36, 1485000,  0x00004B44, VIC_1080p24,   GenByHDTV,    0x48, Prog, 99},/* 1920x1080p@23.98/24Hz */
{1920, 2205, 30, P_POS, P_POS, 2200, 2250, 44, 5,  638,  4, 148,  36, 1485000,  0x00004B44, VIC_1080p30,  GenByHDTV,     0x48, Prog, 100},/* 1920x1080p@29.97/30Hz */
{1920, 2205, 50, P_POS, P_POS, 2640, 2250, 44, 5,  528,  4, 148,  36, 2970000,  0x00003063, VIC_1080p50,   GenByHDTV,    0x48, Prog, 101},/* 1920x1080p@50Hz */
{1920, 2205, 60, P_POS, P_POS, 2200, 2250, 44, 5,   88,  4, 148,  36, 2970000,  0x00003063, VIC_1080p,   GenByHDTV,    0x48, Prog, 102},/* 1920x1080p@60Hz */

/* 4K (2160p) */
/* -HA  -VA   -Rate  -Hor   -Ver    -HT    -VT  -Hs -Vs  -HFP -VFP -HBP -VBP  -DCLK10000 -PClk       -HDMIType    -GCLK     -ADC03 -ScanMode    -SN */
{3840, 2160,  24, P_POS, P_POS, 5500, 2250,   88, 10,  1276,  8, 296,  72,   2967033,  0x00003063, VIC_2160p24,  GenByHDTV_16x9,   0x70,   Prog,  103},
{3840, 2160,  50, P_POS, P_POS, 5280, 2250,   88, 10,  1056,  8, 296,  72,   2970000,  0x00003063, VIC_2160p50,  GenByHDTV_16x9,  0x70,  Prog, 104}, /* 3840x2160p@50Hz,4:2:0. VIC:96 */
{3840, 2160,  25, P_POS, P_POS, 5280, 2250,   88, 10,  1056,  8, 296,  72,   2970000,  0x00003063, VIC_2160p25,  GenByHDTV_16x9,   0x70,   Prog,  105},
{3840, 2160,  60, P_POS, P_POS, 4400, 2250,   88, 10,   176,  8, 296,  72,   2970000,  0x00003063, VIC_2160p,  GenByHDTV_16x9, 0x70, Prog,  106}, /* 3840x2160p@60Hz,4:2:0. VIC:97 */
{3840, 2160,  30, P_POS, P_POS, 4400, 2250,   88, 10,   176,  8, 296,  72,   2967033,  0x00003063, VIC_2160p30,  GenByHDTV_16x9, 0x70, Prog,  107},
/* 4K SMPTE */
{4096, 2160,  24, P_POS, P_POS, 5500, 2250,   88, 10,  1020,  8, 296,  72,   2967033,  0x00003063, VIC_2160p24E,  GenByHDTV_16x9, 0x70, Prog,  108}, /* 4096x2160p@23.98Hz/24Hz, VIC:98 */
{4096, 2160,  50, P_POS, P_POS, 5280, 2250,   88, 10,   968,  8, 128,  72,   2970000,  0x00003063, VIC_2160p50E, GenByHDTV_16x9,  0x70, Prog,  109}, /* 4096x2160p@60Hz,4:2:0. VIC:101 */
{4096, 2160,  25, P_POS, P_POS, 5280, 2250,   88, 10,   968,  8, 128,  72,   2970000,  0x00003063, VIC_2160p25E,  GenByHDTV_16x9,  0x70,   Prog,  110}, /* 4096x2160p@25Hz, VIC:99  */
{4096, 2160,  60, P_POS, P_POS, 4400, 2250,   88, 10,    88,  8, 128,  72,   2970000,  0x00003063, VIC_2160pE,  GenByHDTV_16x9,   0x70,  Prog,  111}, /* 4096x2160p@60Hz,4:2:0. VIC:102 */
/* Bruce170317. [AST1520] 4096x2160 30Hz retrace interval is too short for CRT under critical E pattern. It will cause short line noise in left edge of picture. We shift sync restrace start earlier to resolve this problem. */
{4096, 2160,  30, P_POS, P_POS, 4400, 2250,   88,10,(88-64),8,(128+64),72, 2967033,  0x00003063, VIC_2160p30E,  GenByHDTV_16x9,  0x70, Prog,  112}, /* 4096x2160p@30Hz, VIC:100 */

/* -HA -VA -Rate  -Hor   -Ver    -HT  -VT  -Hs -Vs -HFP -VFP -HBP -VBP -DCLK10000 -PClk       -HDMIType      -GCLK     -ADC03 -ScanMode  -SN */
{2560, 1080,  24, P_POS, P_POS, 3750, 1100, 44, 5,  998,  4, 148,  11,  989011,  0x00006842, VIC_1080p24W3,  GenByHDTV,  0x48, Prog,     113}, /* 2560x1080@24Hz */
{2560, 1080,  25, P_POS, P_POS, 3200, 1125, 44, 5,  448,  4, 148,  36,  900000,  0x0000683C, VIC_1080p25W3,  GenByHDTV,  0x48, Prog,     114}, /* 2560x1080@25Hz */
{2560, 1080,  30, P_POS, P_POS, 3520, 1125, 44, 5,  768,  4, 148,  36, 1186813,  0x00004723, VIC_1080p30W3,  GenByHDTV,  0x48, Prog,     115}, /* 2560x1080@30Hz */
{2560, 1080,  50, P_POS, P_POS, 3300, 1125, 44, 5,  548,  4, 148,  36, 1856250,  0x00004831, VIC_1080p50W3,  GenByHDTV,  0x48, Prog,     116}, /* 2560x1080@50Hz */
{2560, 1080,  60, P_POS, P_POS, 3000, 1100, 44, 5,  248,  4, 148,  11, 1978022,  0x00004842, VIC_1080pW3,    GenByHDTV,  0x48, Prog,     117}, /* 2560x1080@60Hz */

/* pixel clock out of spec. */
{2560, 1080, 100, P_POS, P_POS, 2970, 1250, 44, 5,  218,  4, 148, 161, 3712500,  0x0000283E, VIC_1080p100W3,   GenByHDTV,    0x48, Prog, 118}, /* 2560x1080@100Hz */
{2560, 1080, 120, P_POS, P_POS, 3300, 1250, 44, 5,  548,  4, 148, 161, 4945055,  0x00004C49, VIC_1080p120W3,   GenByHDTV,    0x48, Prog, 119}, /* 2560x1080@120Hz */

/* Special items for 2160p60Hz YUV420 timing. Those items are required for mode detection. */
{3840/2, 2160, 50, P_POS, P_POS, 5280/2, 2250, 88/2, 10, 1056/2,  8, 296/2, 72, 2970000,  0x00003063, VIC_2160p50,  GenByHDTV_16x9,  0x70, Prog, 120}, /* 3840x2160p@50Hz,4:2:0. VIC:96 */
{3840/2, 2160, 60, P_POS, P_POS, 4400/2, 2250, 88/2, 10,  176/2,  8, 296/2, 72, 2970000,  0x00003063, VIC_2160p,    GenByHDTV_16x9,  0x70, Prog, 121}, /* 3840x2160p@60Hz,4:2:0. VIC:97 */
{4096/2, 2160, 50, P_POS, P_POS, 5280/2, 2250, 88/2, 10,  968/2,  8, 128/2, 72, 2970000,  0x00003063, VIC_2160p50E, GenByHDTV_16x9,  0x70, Prog, 122}, /* 4096x2160p@60Hz,4:2:0. VIC:101 */
{4096/2, 2160, 60, P_POS, P_POS, 4400/2, 2250, 88/2, 10,   88/2,  8, 128/2, 72, 2970000,  0x00003063, VIC_2160pE,   GenByHDTV_16x9,  0x70, Prog, 123}, /* 4096x2160p@60Hz,4:2:0. VIC:102 */


/* -HA  -VA -Rate -Hor   -Ver   -HT    -VT  -Hs -Vs -HFP -VFP -HBP  -VBP  -DCLK10000   -PClk    -HDMIType     -GCLK     -ADC03 -ScanMode    -SN */
{1680,  720,  24, P_POS, P_POS, 3300,  750, 40,  5, 1360,  5,  220,  20,   593407, 0x00006C3B,  VIC_720p24W2,  GenByHDTV, 0x48, Prog, 124}, /* CEA-16800x720-24 VIC 79 */
{1680,  720,  25, P_POS, P_POS, 3168,  750, 40,  5, 1228,  5,  220,  20,   594000, 0x00006C3B,  VIC_720p25W2,  GenByHDTV, 0x48, Prog, 125}, /* CEA-16800x720-25 VIC 80 */
{1680,  720,  30, P_POS, P_POS, 2640,  750, 40,  5,  700,  5,  220,  20,   593407, 0x00006C3B,  VIC_720p30W2,  GenByHDTV, 0x48, Prog, 126}, /* CEA-16800x720-30 VIC 81 */
{1680,  720,  50, P_POS, P_POS, 2200,  750, 40,  5,  260,  5,  220,  20,   825000, 0x00006837,  VIC_720p50W2,  GenByHDTV, 0x48, Prog, 127}, /* CEA-16800x720-50 VIC 82 */
{1680,  720,  60, P_POS, P_POS, 2200,  750, 40,  5,  260,  5,  220,  20,   989011, 0x00006842,    VIC_720pW2,  GenByHDTV, 0x48, Prog, 128}, /* CEA-16800x720-60 VIC 83 */
{1680,  720, 100, P_POS, P_POS, 2200,  750, 40,  5,   60,  5,  220,  20,  1650000, 0x00004837, VIC_720p100W2,  GenByHDTV, 0x48, Prog, 129}, /* CEA-16800x720-100 VIC 84 */
{1680,  720, 120, P_POS, P_POS, 2200,  750, 40,  5,   60,  5,  220,  20,  1978022, 0x00004842, VIC_720p120W2,  GenByHDTV, 0x48, Prog, 130}, /* CEA-16800x720-120 VIC 85 */

/* Special items for 4K YUV420 timing which not defined in HDMI spec yet. Those items are required for mode detection. */
{3840/2, 2160, 24, P_POS, P_POS, 5500/2, 2250, 88/2, 10, 1276/2,  8, 296/2, 72, 2967033,  0x00003063, VIC_unknown,  GenByHDTV_16x9,  0x70, Prog,  131}, /* 3840x2160p@24Hz,4:2:0 */
{3840/2, 2160, 25, P_POS, P_POS, 5280/2, 2250, 88/2, 10, 1056/2,  8, 296/2, 72, 2970000,  0x00003063, VIC_unknown,  GenByHDTV_16x9,  0x70, Prog,  132}, /* 3840x2160p@25Hz,4:2:0 */
{3840/2, 2160, 30, P_POS, P_POS, 4400/2, 2250, 88/2, 10,  176/2,  8, 296/2, 72, 2967033,  0x00003063, VIC_unknown,  GenByHDTV_16x9,  0x70, Prog,  133}, /* 3840x2160p@30Hz,4:2:0 */
{4096/2, 2160, 24, P_POS, P_POS, 5500/2, 2250, 88/2, 10, 1020/2,  8, 296/2, 72, 2967033,  0x00003063, VIC_unknown,  GenByHDTV_16x9,  0x70, Prog,  134}, /* 4096x2160p@24Hz,4:2:0 */
{4096/2, 2160, 25, P_POS, P_POS, 5280/2, 2250, 88/2, 10,  968/2,  8, 128/2, 72, 2970000,  0x00003063, VIC_unknown,  GenByHDTV_16x9,  0x70, Prog,  135}, /* 4096x2160p@25Hz,4:2:0 */
{4096/2, 2160, 30, P_POS, P_POS, 4400/2, 2250, 88/2, 10,   88/2,  8, 128/2, 72, 2967033,  0x00003063, VIC_unknown,  GenByHDTV_16x9,  0x70, Prog,  136}, /* 4096x2160p@30Hz,4:2:0 */

#if 0
/*
 * Special items for interlaced video with pixel repetition.
 * Those items are required for mode detection if HDMI Rx output is repeated one
 * just IT6805 output interlaced video with repetition by default (configurable)
 */
/* -HA  -VA -Rate -Hor  -Ver   -HT   -VT   -Hs -Vs -HFP -VFP -HBP -VBP  -DCLK10000  -PClk   -HDMIType     -GCLK     -ADC03 -ScanMode  -SN */
{720*2, 240, 120, P_NEG, P_NEG, 858*2, 262, 62*2, 3, 19*2, 4, 57*2, 15,  540000, 0x00086824, VIC_480i59,  GenByHDTV,  0x48,  Interl,  137}, /* 668.CEA-720X480i-120 4:3 */
{720*2, 240, 240, P_NEG, P_NEG, 858*2, 262, 62*2, 3, 19*2, 4, 57*2, 15, 1080000, 0x00004824, VIC_480i119, GenByHDTV,  0x48,  Interl,  138}, /* 676.CEA-720X480i-240 4:3 */
{720*2, 288, 100, P_NEG, P_NEG, 864*2, 312, 63*2, 3, 12*2, 2, 69*2, 19,  540000, 0x00006824, VIC_576i50,  GenByHDTV,  0x48,  Interl,  139}, /* 662.CEA-720X576i-100 4:3 */
{720*2, 288, 200, P_NEG, P_NEG, 864*2, 312, 63*2, 3, 12*2, 2, 69*2, 19, 1080000, 0x00004824, VIC_576i100, GenByHDTV,  0x48,  Interl,  140}, /* 672.CEA-720X576i-200 4:3 */
#endif

#if 0
/* 1080P 120Hz/100Hz not over spec, but got some noise */
/* -HA  -VA -Rate -Hor   -Ver   -HT    -VT    -Hs -Vs  -HFP -VFP -HBP  -VBP  -DCLK10000   -PClk    -HDMIType     -GCLK         -ADC03 -ScanMode -SN */
{1920, 1080, 120, P_POS, P_POS, 2200, 1125,   44,  5,   88,   4, 148,   36,  2967033, 0x00003063, VIC_1080p120,  GenByHDTV_16x9, 0x70,   Prog, 137}, /* #16, 1920x1080p@60Hz 16:9 */
{1920, 1080, 100, P_POS, P_POS, 2640, 1125,   44,  5,  528,   4, 148,   36,  2967033, 0x00003063, VIC_1080p100,  GenByHDTV_16x9, 0xF0,   Prog, 138}, /* #16, 1920x1080p@60Hz 16:9 */
#endif


/******************************************************************************************************************************************************************/
/* VESA Digital Timing */
/* -HA  -VA -Rate  -Hor  -Ver    -HT    -VT  -Hs -Vs  -HFP -VFP  -HBP -VBP -DCLK10000  -PClk       -HDMIType    -GCLK    -ADC03 -ScanMode -SN */
{ 640,  350,  85, P_POS, P_NEG,  832,  445,  64,  3,  32,  32,   96,  60,  315000, 0x0008682A, VIC_unknown, GenByVESA, 0x48, Prog,  VESA_SN(0)}, /* 501.DVI-640X350-85 */
{ 640,  400,  85, P_NEG, P_POS,  832,  445,  64,  3,  32,   1,   96,  41,  315000, 0x0008682A, VIC_unknown, GenByVESA, 0x48, Prog,  VESA_SN(1)}, /* 502.DVI-640X400-85 */
{ 720,  400,  85, P_NEG, P_POS,  936,  446,  72,  3,  36,   1,  108,  42,  355000, 0x00086C47, VIC_unknown, GenByVESA, 0x48, Prog,  VESA_SN(2)}, /* 503.DVI-720X400-85 */
{ 640,  480,  60, P_NEG, P_NEG,  800,  525,  96,  2,  16,  10,   48,  33,  251750, 0x00086A2A, VIC_unknown, GenByVESA, 0x28, Prog,  VESA_SN(3)}, /* 504.DVI-640X480-60 */
{ 640,  480,  72, P_NEG, P_NEG,  832,  520,  40,  3,  24,   9,  128,  28,  315000, 0x0008682A, VIC_unknown, GenByVESA, 0x30, Prog,  VESA_SN(4)}, /* 505.DVI-640X480-72 */
{ 640,  480,  75, P_NEG, P_NEG,  840,  500,  64,  3,  16,   1,  120,  16,  315000, 0x0008682A, VIC_unknown, GenByVESA, 0x30, Prog,  VESA_SN(5)}, /* 506.DVI-640X480-75 */
{ 640,  480,  85, P_NEG, P_NEG,  832,  509,  56,  3,  56,   1,   80,  25,  360000, 0x0008672A, VIC_unknown, GenByVESA, 0x30, Prog,  VESA_SN(6)}, /* 507.DVI-640X480-85 */
{ 800,  600,  56, P_POS, P_POS, 1024,  625,  72,  2,  24,   1,  128,  22,  360000, 0x0008672A, VIC_unknown, GenByVESA, 0x30, Prog,  VESA_SN(7)}, /* 508.DVI-800X600-56 */
{ 800,  600,  60, P_POS, P_POS, 1056,  628, 128,  4,  40,   1,   88,  23,  400000, 0x0008693C, VIC_unknown, GenByVESA, 0x60, Prog,  VESA_SN(8)}, /* 509.DVI-800X600-60 */
{ 800,  600,  72, P_POS, P_POS, 1040,  666, 120,  6,  56,  37,   64,  23,  500000, 0x00006C32, VIC_unknown, GenByVESA, 0x60, Prog,  VESA_SN(9)}, /* 510.DVI-800X600-72 */
{ 800,  600,  75, P_POS, P_POS, 1056,  625,  80,  3,  16,   1,  160,  21,  495000, 0x00086842, VIC_unknown, GenByVESA, 0x60, Prog, VESA_SN(10)}, /* 511.DVI-800X600-75 */
{ 800,  600,  85, P_POS, P_POS, 1048,  631,  64,  3,  32,   1,  152,  27,  562500, 0x00006A2F, VIC_unknown, GenByVESA, 0x68, Prog, VESA_SN(11)}, /* 512.DVI-800X600-85 */
{ 848,  480,  60, P_POS, P_POS, 1088,  517, 112,  8,  16,   6,  112,  23,  337500, 0x0008682D, VIC_unknown, GenByVESA, 0x48, Prog, VESA_SN(12)}, /* 513.DVI-848X480-60 */
{1024,  384,  87, P_POS, P_POS, 1264,  408, 176,  4,   8,   0,   56,  20,  449000, 0x0008683C, VIC_unknown, GenByVESA, 0x48, Interl, VESA_SN(13)}, /* 514.DVI-1024X768-43 */
{1024,  768,  60, P_NEG, P_NEG, 1344,  806, 136,  6,  24,   3,  160,  29,  650000, 0x00006C41, VIC_unknown, GenByVESA, 0x70, Prog, VESA_SN(14)}, /* 515.DVI-1024X768-60 */
{1024,  768,  70, P_NEG, P_NEG, 1328,  806, 136,  6,  24,   3,  144,  29,  750000, 0x00006832, VIC_unknown, GenByVESA, 0x60, Prog, VESA_SN(15)}, /* 516.DVI-1024X768-70 */
{1024,  768,  75, P_POS, P_POS, 1312,  800,  96,  3,  16,   1,  176,  28,  787500, 0x0000693B, VIC_unknown, GenByVESA, 0x60, Prog, VESA_SN(16)}, /* 517.DVI-1024X768-75 */
{1024,  768,  85, P_POS, P_POS, 1376,  808,  96,  3,  48,   1,  208,  36,  945000, 0x0000683F, VIC_unknown, GenByVESA, 0xA0, Prog, VESA_SN(17)}, /* 518.DVI-1024X768-85 */
{1152,  864,  75, P_POS, P_POS, 1600,  900, 128,  3,  64,   1,  256,  32, 1080000, 0x00004824, VIC_unknown, GenByVESA, 0x48, Prog, VESA_SN(18)}, /* 519.DVI-1152X864-75 */
{1280,  768,  60, P_POS, P_NEG, 1440,  790,  32,  7,  48,   3,   80,  12,  682500, 0x00006A39, VIC_unknown, GenByVESA, 0x48, Prog, VESA_SN(19)}, /* 520.DVI-1280X768-60 */
{1280,  768,  60, P_NEG, P_POS, 1664,  798, 128,  7,  64,   3,  192,  20,  795000, 0x00006835, VIC_unknown, GenByVESA, 0x48, Prog, VESA_SN(20)}, /* 521.DVI-1280X768-60 */
{1280,  768,  75, P_NEG, P_POS, 1696,  805, 128,  7,  80,   3,  208,  27, 1022500, 0x00004822, VIC_unknown, GenByVESA, 0x48, Prog, VESA_SN(21)}, /* 522.DVI-1280X768-75 */
{1280,  768,  85, P_NEG, P_POS, 1712,  809, 136,  7,  80,   3,  216,  31, 1175000, 0x00004A31, VIC_unknown, GenByVESA, 0x48, Prog, VESA_SN(22)}, /* 523.DVI-1280X768-85 */
{1280,  960,  60, P_POS, P_POS, 1800, 1000, 112,  3,  96,   1,  312,  36, 1080000, 0x00004824, VIC_unknown, GenByVESA, 0x48, Prog, VESA_SN(23)}, /* 524.DVI-1280X960-60 */
{1280,  960,  85, P_POS, P_POS, 1728, 1011, 160,  3,  64,   1,  224,  47, 1485000, 0x00004B44, VIC_unknown, GenByVESA, 0x48, Prog, VESA_SN(24)}, /* 525.DVI-1280X960-85 */
{1280, 1024,  60, P_POS, P_POS, 1688, 1066, 112,  3,  48,   1,  248,  38, 1080000, 0x00004824, VIC_unknown, GenByVESA, 0xB0, Prog, VESA_SN(25)}, /* 526.DVI-1280X1024-60 */
{1280, 1024,  75, P_POS, P_POS, 1688, 1066, 144,  3,  16,   1,  248,  38, 1350000, 0x0000482D, VIC_unknown, GenByVESA, 0xE8, Prog, VESA_SN(26)}, /* 527.DVI-1280X1024-75 */
{1280, 1024,  85, P_POS, P_POS, 1728, 1072, 160,  3,  64,   1,  224,  44, 1575000, 0x0000493B, VIC_unknown, GenByVESA, 0x48, Prog, VESA_SN(27)}, /* 528.DVI-1280X1024-85 */
{1360,  768,  60, P_POS, P_POS, 1792,  795, 112,  6,  64,   3,  256,  18,  855000, 0x00006839, VIC_unknown, GenByVESA, 0x48, Prog, VESA_SN(28)}, /* 529.DVI-1360X768-60 */
{1400, 1050,  60, P_POS, P_NEG, 1560, 1080,  32,  4,  48,   3,   80,  23, 1010000, 0x00004A2A, VIC_unknown, GenByVESA, 0x48, Prog, VESA_SN(29)}, /* 530.DVI-1400X1050-60 */
{1400, 1050,  60, P_NEG, P_POS, 1864, 1089, 144,  4,  88,   3,  232,  32, 1217500, 0x00004C3D, VIC_unknown, GenByVESA, 0x48, Prog, VESA_SN(30)}, /* 531.DVI-1400X1050-60 */
{1400, 1050,  75, P_NEG, P_POS, 1896, 1099, 144,  4, 104,   3,  248,  42, 1560000, 0x00004834, VIC_unknown, GenByVESA, 0x48, Prog, VESA_SN(31)}, /* 532.DVI-1400X1050-75 */
{1400, 1050,  85, P_NEG, P_POS, 1912, 1105, 152,  4, 104,   3,  256,  48, 1795000, 0x0000483C, VIC_unknown, GenByVESA, 0x48, Prog, VESA_SN(32)}, /* 533.DVI-1400X1050-85 */
{1440,  900,  60, P_POS, P_NEG, 1600,  926,  32,  6,  48,   3,   80,  17,  887500, 0x00006A4A, VIC_unknown, GenByVESA, 0x48, Prog, VESA_SN(33)}, /* 534.DVI-1440X900-60 */
{1440,  900,  60, P_NEG, P_POS, 1904,  934, 152,  6,  80,   3,  232,  25, 1065000, 0x00004928, VIC_unknown, GenByVESA, 0x48, Prog, VESA_SN(34)}, /* 535.DVI-1440X900-60 */
{1440,  900,  75, P_NEG, P_POS, 1936,  942, 152,  6,  96,   3,  248,  33, 1367500, 0x00004A39, VIC_unknown, GenByVESA, 0x48, Prog, VESA_SN(35)}, /* 536.DVI-1440X900-75 */
{1440,  900,  85, P_NEG, P_POS, 1952,  948, 152,  6, 104,   3,  256,  39, 1570000, 0x00004B48, VIC_unknown, GenByVESA, 0x48, Prog, VESA_SN(36)}, /* 537.DVI-1440X900-85 */
{1600, 1200,  60, P_POS, P_POS, 2160, 1250, 192,  3,  64,   1,  304,  46, 1620000, 0x00004836, VIC_unknown, GenByVESA, 0x48, Prog, VESA_SN(37)}, /* 538.DVI-1600X1200-60 */
{1600, 1200,  65, P_POS, P_POS, 2160, 1250, 192,  3,  64,   1,  304,  46, 1755000, 0x00005075, VIC_unknown, GenByVESA, 0x48, Prog, VESA_SN(38)}, /* 539.DVI-1600X1200-65 */
{1600, 1200,  70, P_POS, P_POS, 2160, 1250, 192,  3,  64,   1,  304,  46, 1890000, 0x0000483F, VIC_unknown, GenByVESA, 0x48, Prog, VESA_SN(39)}, /* 540.DVI-1600X1200-70 */
{1600, 1200,  75, P_POS, P_POS, 2160, 1250, 192,  3,  64,   1,  304,  46, 2025000, 0x00002926, VIC_unknown, GenByVESA, 0x48, Prog, VESA_SN(40)}, /* 541.DVI-1600X1200-75 */
{1600, 1200,  85, P_POS, P_POS, 2160, 1250, 192,  3,  64,   1,  304,  46, 2295000, 0x0000292B, VIC_unknown, GenByVESA, 0x48, Prog, VESA_SN(41)}, /* 542.DVI-1600X1200-85 */
{1680, 1050,  60, P_POS, P_NEG, 1840, 1080,  32,  6,  48,   3,   80,  21, 1190000, 0x00004C3B, VIC_unknown, GenByVESA, 0x48, Prog, VESA_SN(42)}, /* 543.DVI-1680X1050-60 */
{1680, 1050,  60, P_NEG, P_POS, 2240, 1089, 176,  6, 104,   3,  280,  30, 1462500, 0x00004B43, VIC_unknown, GenByVESA, 0x48, Prog, VESA_SN(43)}, /* 544.DVI-1680X1050-60 */
{1680, 1050,  75, P_NEG, P_POS, 2272, 1099, 176,  6, 120,   3,  296,  40, 1870000, 0x00004A4E, VIC_unknown, GenByVESA, 0x48, Prog, VESA_SN(44)}, /* 545.DVI-1680X1050-75 */
{1680, 1050,  85, P_NEG, P_POS, 2288, 1105, 176,  6, 128,   3,  304,  46, 2147500, 0x00002B31, VIC_unknown, GenByVESA, 0x48, Prog, VESA_SN(45)}, /* 546.DVI-1680X1050-85 */
{1792, 1344,  60, P_NEG, P_POS, 2448, 1394, 200,  3, 128,   1,  328,  46, 2047500, 0x00002B2F, VIC_unknown, GenByVESA, 0x48, Prog, VESA_SN(46)}, /* 547.DVI-1792X1344-60 */
{1792, 1344,  75, P_NEG, P_POS, 2456, 1417, 216,  3,  96,   1,  352,  69, 2610000, 0x00003057, VIC_unknown, GenByVESA, 0x48, Prog, VESA_SN(47)}, /* 548.DVI-1792X1344-75 */
{1856, 1392,  60, P_NEG, P_POS, 2528, 1439, 224,  3,  96,   1,  352,  43, 2182500, 0x00002B32, VIC_unknown, GenByVESA, 0x48, Prog, VESA_SN(48)}, /* 549.DVI-1856X1392-60 */
{1856, 1392,  75, P_NEG, P_POS, 2560, 1500, 224,  3, 128,   1,  352, 104, 2880000, 0x0000272A, VIC_unknown, GenByVESA, 0x48, Prog, VESA_SN(49)}, /* 550.DVI-1856X1392-75 */
#if (CONFIG_AST1500_SOC_VER >= 2) /* This timing has very short HSync+BackPorch time. CRT need this patch to avoid line noises. */
{1920, 1200,  60, P_POS, P_NEG, 2080, 1235,  32,  6,  (48 - 40),  3, (80 + 40),  26, 1540000, 0x00004C4D, VIC_unknown, GenByVESA, 0x48, Prog, VESA_SN(50)}, /* 551.DVI-1920X1200-60 */
#else
{1920, 1200,  60, P_POS, P_NEG, 2080, 1235,  32,  6,  48,   3,   80,  26,  1540000, 0x00004C4D, VIC_unknown, GenByVESA, 0x48, Prog, VESA_SN(50)}, /* 551.DVI-1920X1200-60 */
#endif
{1920, 1200,  60, P_NEG, P_POS, 2592, 1245, 200,  6, 136,   3,  336,  36, 1932500, 0x00004C61, VIC_unknown, GenByVESA, 0x48, Prog, VESA_SN(51)}, /* 552.DVI-1920X1200-60 */
{1920, 1200,  75, P_NEG, P_POS, 2608, 1255, 208,  6, 136,   3,  344,  46, 2452500, 0x0000292E, VIC_unknown, GenByVESA, 0x48, Prog, VESA_SN(52)}, /* 553.DVI-1920X1200-75 */
{1920, 1200,  75, P_NEG, P_POS, 2642, 1262, 208,  6, 162,   3,  352,  53, 2812500, 0x00002729, VIC_unknown, GenByVESA, 0x48, Prog, VESA_SN(53)}, /* 554.DVI-1920X1200-85 */
#if 0 /* pixel clock 234MHz may hang AST1520 system for unknown reason. Use 235MHz instead. */
{1920, 1440,  60, P_NEG, P_POS, 2600, 1500, 208,  3, 128,   1,  344,  56, 2340000, 0x00002827, VIC_unknown, GenByVESA, 0x48, Prog, VESA_SN(54)}, /* 555.DVI-1920X1440-60 */
#else
{1920, 1440,  60, P_NEG, P_POS, 2600, 1500, 208,  3, 128,   1,  344,  56, 2350000, 0x00002827, VIC_unknown, GenByVESA, 0x48, Prog, VESA_SN(54)}, /* 555.DVI-1920X1440-60 */
#endif
{1920, 1440,  75, P_NEG, P_POS, 2640, 1500, 224,  3, 144,   1,  352,  56, 2970000, 0x00003063, VIC_unknown, GenByVESA, 0x48, Prog, VESA_SN(55)}, /* 556.DVI-1920X1440-75 */
{2560, 1600,  60, P_NEG, P_POS, 2720, 1646,  32,  6,  48,   3,   80,  37, 2685000, 0x00002A38, VIC_unknown, GenByVESA, 0x48, Prog, VESA_SN(56)}, /* 557.DVI-2560X1600-75 */

/* VESA New */
/* -HA  -VA -Rate  -Hor  -Ver    -HT    -VT  -Hs -Vs  -HFP -VFP  -HBP -VBP -DCLK10000  -PClk       -HDMIType    -GCLK  -ADC03 -ScanMode -SN */
{ 800,  600, 120, P_POS, P_NEG,  960,  636,  32,  4,  48,   3,   80,  29,  732500, 0x00006A3D, VIC_unknown, GenByCVT_RB, 0x70, Prog, VESA_SN(57)},
{1024,  384,  87, P_POS, P_POS, 1264,  408, 176,  4,   8,   0,   56,  20,  449000, 0x0008683C, VIC_unknown, GenByVESA,   0x48, Prog, VESA_SN(58)}, /* VESA-1024X768-43 */
{1024,  768, 120, P_POS, P_NEG, 1184,  813,  32,  4,  48,   3,   80,  38, 1155000, 0x00004B35, VIC_unknown, GenByCVT_RB, 0xF0, Prog, VESA_SN(59)},
{1152,  864,  85, P_NEG, P_POS, 1552,  907, 128,  3,  72,   1,  200,  39, 1196500, 0x00004723, VIC_unknown, GenByVESA,   0x48, Prog, VESA_SN(60)}, /* VESA?? 1152x864@85Hz */
{1152,  864, 100, P_NEG, P_POS, 1568,  915, 128,  3,  80,   1,  208,  47, 1434700, 0x0000472A, VIC_unknown, GenByVESA,   0x48, Prog, VESA_SN(61)}, /* VESA?? 1152x864@100Hz */
{1152,  864,  60, P_NEG, P_POS, 1520,  895, 120,  3,  64,   1,  184,  27,  816200, 0x00006A44, VIC_unknown, GenByVESA,   0x48, Prog, VESA_SN(62)}, /* VESA?? 1152x864@60Hz */
{1280,  768, 120, P_POS, P_NEG, 1440,  813,  32,  7,  48,   3,   80,  35, 1402500, 0x00004C46, VIC_unknown, GenByCVT_RB, 0xF0, Prog, VESA_SN(63)},
{1280,  800,  60, P_POS, P_NEG, 1440,  823,  32,  6,  48,   3,   80,  14,  710000, 0x00006C47, VIC_unknown, GenByCVT_RB, 0x70, Prog, VESA_SN(64)},
{1280,  800,  60, P_NEG, P_POS, 1680,  831, 128,  6,  72,   3,  200,  22,  835000, 0x00006731, VIC_unknown, GenByCVT,    0xB0, Prog, VESA_SN(65)},
{1280,  800,  75, P_NEG, P_POS, 1696,  838, 128,  6,  80,   3,  208,  29, 1065000, 0x00004928, VIC_unknown, GenByCVT,    0xB0, Prog, VESA_SN(66)},
{1280,  800,  85, P_NEG, P_POS, 1712,  843, 136,  6,  80,   3,  216,  34, 1225000, 0x00004A33, VIC_unknown, GenByCVT,    0xF0, Prog, VESA_SN(67)},
{1280,  800, 120, P_POS, P_NEG, 1440,  847,  32,  6,  48,   3,   80,  38, 1462500, 0x00004B43, VIC_unknown, GenByCVT_RB, 0xF0, Prog, VESA_SN(68)},
{1280,  960, 120, P_POS, P_NEG, 1440, 1017,  32,  4,  48,   3,   80,  50, 1755000, 0x00005075, VIC_unknown, GenByCVT_RB, 0xF0, Prog, VESA_SN(69)},
{1280, 1024, 120, P_POS, P_NEG, 1440, 1084,  32,  7,  48,   3,   80,  50, 1872500, 0x00004A4E, VIC_unknown, GenByCVT_RB, 0xF0, Prog, VESA_SN(70)},
{1360,  768, 120, P_POS, P_NEG, 1520,  813,  32,  5,  48,   3,   80,  37, 1482500, 0x00004B44, VIC_unknown, GenByCVT_RB, 0xF0, Prog, VESA_SN(71)},
{1366,  768,  60, P_POS, P_POS, 1500,  800,  56,  3,  14,   1,   64,  28,  720000, 0x0000672A, VIC_unknown, GenByCVT_RB, 0xF0, Prog, VESA_SN(72)},
{1366,  768,  60, P_POS, P_NEG, 1500,  800,  56,  3,  14,   1,   64,  28,  720000, 0x0000672A, VIC_unknown, GenByCVT_RB, 0xF0, Prog, VESA_SN(73)},
{1366,  768,  60, P_POS, P_NEG, 1792,  798, 143,  3,  70,   3,  213,  24,  855000, 0x00006839, VIC_unknown, GenByCVT,   0xF0,  Prog, VESA_SN(74)},
{1366,  768,  60, P_POS, P_POS, 1792,  798, 143,  3,  70,   3,  213,  24,  855000, 0x00006839, VIC_unknown, GenByCVT,   0xF0,  Prog, VESA_SN(75)},
{1400, 1050, 120, P_POS, P_NEG, 1560, 1112,  32,  4,  48,   3,   80,  55, 2080000, 0x00002927, VIC_unknown, GenByCVT_RB, 0xF0, Prog, VESA_SN(76)},
{1440,  900, 120, P_POS, P_NEG, 1600,  953,  32,  6,  48,   3,   80,  44, 1827500, 0x0000483D, VIC_unknown, GenByCVT_RB, 0xF0, Prog, VESA_SN(77)},
{1600,  900,  60, P_POS, P_NEG, 1800, 1000,  80,  3,  24,   1,   96,  96, 1080000, 0x00004824, VIC_unknown, GenByCVT_RB, 0xF0, Prog, VESA_SN(78)},
{1600, 1200, 120, P_POS, P_NEG, 1760, 1271,  32,  4,  48,   3,   80,  64, 2682500, 0x00002C43, VIC_unknown, GenByCVT_RB, 0xF0, Prog, VESA_SN(79)},
{1680, 1050, 120, P_POS, P_NEG, 1840, 1112,  32,  6,  48,   3,   80,  53, 2455000, 0x0000292E, VIC_unknown, GenByCVT_RB, 0xF0, Prog, VESA_SN(80)},
{1920, 1080,  60, P_POS, P_POS, 2200, 1125,  44,  5,  88,   4,  148,  36, 1485000, 0x00004B44, VIC_unknown, GenByVESA,   0xF0, Prog, VESA_SN(81)},
{2048, 1152,  60, P_POS, P_POS, 2250, 1200,  80,  3,  26,   1,   96,  44, 1620000, 0x00004836, VIC_unknown, GenByCVT_RB, 0xF0, Prog, VESA_SN(82)},
{2560, 1600,  60, P_POS, P_NEG, 2720, 1646,  32,  6,  48,   3,   80,  37, 2685000, 0x00002A38, VIC_unknown, GenByCVT_RB, 0xF0, Prog, VESA_SN(83)},

/*VESA CVT Timing*/
/* -HA  -VA  -Rate  -Hor   -Ver   -HT  -VT  -Hs -Vs  -HFP -VFP -HBP -VBP  -DCLK10000 -PClk       -HDMIType    -GCLK     -ADC03 -ScanMode    -SN */
{ 640,  480,  50, P_NEG, P_POS,  800,  497,  64,  4,  16,   3,   80,  10,  197500, 0x000C6C4F, VIC_unknown, GenByCVT,    0x48, Prog, VESA_SN(84)}, /* 301.CVT-640X 480-50 */
{ 640,  480,  60, P_NEG, P_POS,  800,  500,  64,  4,  16,   3,   80,  13,  237500, 0x000C6C5F, VIC_unknown, GenByCVT,    0x28, Prog, VESA_SN(85)}, /* 302.CVT-640X 480-60 */
{ 640,  480,  75, P_NEG, P_POS,  816,  504,  64,  4,  24,   3,   88,  17,  307500, 0x00086829, VIC_unknown, GenByCVT,    0x30, Prog, VESA_SN(86)}, /* 303.CVT-640X 480-75 */
{ 640,  480,  85, P_NEG, P_POS,  816,  507,  64,  4,  24,   3,   88,  20,  350000, 0x00086C46, VIC_unknown, GenByCVT,    0x30, Prog, VESA_SN(87)}, /* 304.CVT-640X 480-85 */
{ 640,  480,  60, P_POS, P_NEG,  800,  494,  32,  4,  48,   2,   80,   8,  235000, 0x000C6C5E, VIC_unknown, GenByCVT_RB, 0x28, Prog, VESA_SN(88)}, /* 305.CVT-640X480-60-RB */
{ 800,  600,  50, P_NEG, P_POS,  992,  621,  72,  4,  24,   3,   96,  14,  307500, 0x00086829, VIC_unknown, GenByCVT,    0x48, Prog, VESA_SN(89)}, /* 306.CVT-800X 600-50 */
{ 800,  600,  60, P_NEG, P_POS, 1024,  624,  80,  4,  32,   3,  112,  17,  382500, 0x00086833, VIC_unknown, GenByCVT,    0x60, Prog, VESA_SN(90)}, /* 307.CVT-800X 600-60 */
{ 800,  600,  75, P_NEG, P_POS, 1040,  629,  80,  4,  40,   3,  120,  22,  490000, 0x00086C62, VIC_unknown, GenByCVT,    0x60, Prog, VESA_SN(91)}, /* 308.CVT-800X 600-75 */
{ 800,  600,  85, P_NEG, P_POS, 1056,  633,  80,  4,  48,   3,  128,  26,  567500, 0x00006B34, VIC_unknown, GenByCVT,    0x68, Prog, VESA_SN(92)}, /* 309.CVT-800X 600-85 */
{ 800,  600,  60, P_POS, P_NEG,  960,  618,  32,  4,  48,   2,   80,  12,  355000, 0x00086C47, VIC_unknown, GenByCVT_RB, 0x60, Prog, VESA_SN(93)}, /* 310.CVT-800X600-60-RB */
{1024,  768,  50, P_NEG, P_POS, 1312,  793, 104,  4,  40,   3,  144,  18,  520000, 0x00006927, VIC_unknown, GenByCVT,    0x48, Prog, VESA_SN(94)}, /* 311.CVT-1024X768-50 */
{1024,  768,  60, P_NEG, P_POS, 1328,  798, 104,  4,  48,   3,  152,  23,  635000, 0x00006725, VIC_unknown, GenByCVT,    0x70, Prog, VESA_SN(95)}, /* 312.CVT-1024X768-60 */
{1024,  768,  75, P_NEG, P_POS, 1360,  805, 104,  4,  64,   3,  168,  30,  820000, 0x00006C52, VIC_unknown, GenByCVT,  0x60, Prog, VESA_SN(96)}, /* 313.CVT-1024X768-75 */
{1024,  768,  85, P_NEG, P_POS, 1376,  809, 104,  4,  72,   3,  176,  34,  945000, 0x0000683F, VIC_unknown, GenByCVT,  0xA0, Prog, VESA_SN(97)}, /* 314.CVT-1024X768-85 */
{1024,  768,  60, P_POS, P_NEG, 1184,  790,  32,  4,  48,   2,   80,  16,  560000, 0x0000692A, VIC_unknown, GenByCVT_RB, 0x70, Prog, VESA_SN(98)}, /* 315.CVT-1024X768-60-RB */
{1280,  960,  50, P_NEG, P_POS, 1680,  991, 128,  4,  72,   3,  200,  24,  830000, 0x00006C53, VIC_unknown, GenByCVT,  0x48, Prog, VESA_SN(99)}, /* 316.CVT-1280X960-50 */
{1280,  960,  60, P_NEG, P_POS, 1696,  996, 128,  4,  80,   3,  208,  29, 1012500, 0x00004926, VIC_unknown, GenByCVT,  0x48, Prog,  VESA_SN(100)}, /* 317.CVT-1280X960-60 */

{1280,  960,  75, P_NEG, P_POS, 1728, 1005, 136,  4,  88,   3,  224,  38, 1300000, 0x00004C41, VIC_unknown, GenByCVT,  0x48, Prog, VESA_SN(101)}, /* 318.CVT-1280X960-75 */
{1280,  960,  85, P_NEG, P_POS, 1728, 1011, 136,  4,  88,   3,  224,  44, 1482500, 0x00004B44, VIC_unknown, GenByCVT,  0x48, Prog,  VESA_SN(102)}, /* 319.CVT-1280X960-85 */
{1280,  960,  60, P_POS, P_NEG, 1440,  988,  32,  4,  48,   2,   80,  22,  852500, 0x00006A47, VIC_unknown, GenByCVT_RB, 0x48, Prog, VESA_SN(103)}, /* 320.CVT-1280X960-60-RB */
{1400, 1050,  50, P_NEG, P_POS, 1848, 1083, 144,  4,  80,   3,  224,  26, 1000000, 0x00004C32, VIC_unknown, GenByCVT,  0x48, Prog, VESA_SN(104)}, /* 321.CVT-1400X1050-50 */
{1400, 1050,  60, P_POS, P_NEG, 1560, 1080,  32,  4,  48,   2,   80,  24, 1010000, 0x00004A2A, VIC_unknown, GenByCVT_RB, 0x48, Prog,  VESA_SN(105)}, /* 325.CVT-1400X1050-60-RB */
{1600, 1200,  50, P_NEG, P_POS, 2128, 1238, 168,  4,  96,   3,  264,  31, 1315000, 0x0000482C, VIC_unknown, GenByCVT,  0x48, Prog, VESA_SN(106)}, /* 326.CVT-1600X1200-50 */
{1600, 1200,  60, P_NEG, P_POS, 2160, 1245, 168,  4, 112,   3,  280,  38, 1610000, 0x0000472F, VIC_unknown, GenByCVT,  0x48, Prog, VESA_SN(107)}, /* 327.CVT-1600X1200-60 */
{1600, 1200,  75, P_NEG, P_POS, 2176, 1255, 168,  4, 120,   3,  288,  48, 2047500, 0x00002B2F, VIC_unknown, GenByCVT,  0x48, Prog, VESA_SN(108)}, /* 328.CVT-1600X1200-75 */
{1600, 1200,  85, P_NEG, P_POS, 2192, 1262, 168,  4, 128,   3,  296,  55, 2350000, 0x00002A31, VIC_unknown, GenByCVT,  0x48, Prog, VESA_SN(109)}, /* 329.CVT-1600X1200-85 */
{1600, 1200,  60, P_POS, P_NEG, 1760, 1235,  32,  4,  48,   2,   80,  29, 1302500, 0x00004762, VIC_unknown, GenByCVT_RB, 0x48, Prog, VESA_SN(110)}, /* 330.CVT-1600X1200-60-RB */
{1920, 1440,  50, P_NEG, P_POS, 2592, 1484, 200,  4, 136,  13,  336,  27, 1922500, 0x00004738, VIC_unknown, GenByCVT,  0x48, Prog, VESA_SN(111)}, /* 331.CVT-1920X1440-50 */
{1920, 1440,  60, P_NEG, P_POS, 2608, 1493, 208,  4, 136,   3,  344,  46, 2335000, 0x00002722, VIC_unknown, GenByCVT,  0x48, Prog, VESA_SN(112)}, /* 332.CVT-1920X1440-60 */
{1920, 1440,  75, P_NEG, P_POS, 2640, 1506, 208,  4, 152,   3,  360,  59, 2980000, 0x00002A3E, VIC_unknown, GenByCVT,  0x48, Prog, VESA_SN(113)}, /* 333.CVT-1920X1440-75 */
{1920, 1440,  60, P_POS, P_NEG, 2080, 1481,  32,  4,  48,   2,   80,  35, 1847500, 0x00004A4D, VIC_unknown, GenByCVT_RB, 0x48, Prog, VESA_SN(114)}, /* 334.CVT-1920X1440-60-RB */
{2048, 1536,  50, P_NEG, P_POS, 2768, 1583, 216,  4, 144,   3,  360,  40, 2190000, 0x00003049, VIC_unknown, GenByCVT,  0x48, Prog, VESA_SN(115)}, /* 335.CVT-2048X1536-50 */
{2048, 1536,  60, P_NEG, P_POS, 2800, 1592, 224,  4, 152,   3,  376,  49, 2672000, 0x00003059, VIC_unknown, GenByCVT,  0x48, Prog, VESA_SN(116)}, /* 336.CVT-2048X1536-60 */
{2048, 1536,  60, P_POS, P_NEG, 2208, 1580,  32,  4,  48,   2,   80,  38, 2092500, 0x00002B30, VIC_unknown, GenByCVT_RB, 0x48, Prog, VESA_SN(117)}, /* 337.CVT-2048X1536-60-RB */
{1280, 1024,  50, P_NEG, P_POS, 1680, 1057, 128,  7,  72,  3, 200,  23,  885000, 0x0000683B, VIC_unknown, GenByCVT,  0x48, Prog, VESA_SN(118)}, /* 339.CVT-1280X1024-50 */
{1280, 1024,  60, P_NEG, P_POS, 1712, 1063, 136,  7,  80,  3, 216,  29, 1090000, 0x00004B32, VIC_unknown, GenByCVT,  0xB0, Prog, VESA_SN(119)}, /* 340.CVT-1280X1024-60 */
{1280, 1024,  75, P_NEG, P_POS, 1728, 1072, 136,  7,  88,  3, 224,  38, 1387500, 0x00004934, VIC_unknown, GenByCVT,  0xE8, Prog, VESA_SN(120)}, /* 341.CVT-1280X1024-75 */
{1280, 1024,  85, P_NEG, P_POS, 1744, 1078, 136,  7,  96,  3, 232,  44, 1595000, 0x00004B49, VIC_unknown, GenByCVT,  0x48, Prog, VESA_SN(121)}, /* 342.CVT-1280X1024-85 */
{1280, 1024,  60, P_POS, P_NEG, 1440, 1054,  32,  7,  48,  2,  80,  21,  910000, 0x00006C5B, VIC_unknown, GenByCVT_RB, 0x48, Prog, VESA_SN(122)}, /* 343.CVT-1280X1024-60-RB */
{1280,  768,  50, P_NEG, P_POS, 1648,  793, 128,  7,  56,  3, 184,  15,  652500, 0x00006931, VIC_unknown, GenByCVT,  0x48, Prog, VESA_SN(123)}, /* 344.CVT-1280X768-50 */
{1280,  768,  60, P_POS, P_NEG, 1440,  790,  32,  7,  48,  2,  80,  13,  682500, 0x00006A39, VIC_unknown, GenByCVT_RB, 0x48, Prog, VESA_SN(124)}, /* 348.CVT-1280X768-60-RB */
{ 848,  480,  50, P_NEG, P_POS, 1056,  497,  80,  5,  24,  3, 104,  9,   260000, 0x00086927, VIC_unknown, GenByCVT,  0x48, Prog, VESA_SN(125)}, /* 349.CVT-848X 480-50 */
{ 848,  480,  60, P_NEG, P_POS, 1056,  500,  80,  5,  24,  3, 104,  12,  315000, 0x0008682A, VIC_unknown, GenByCVT,  0x48, Prog, VESA_SN(126)}, /* 350.CVT-848X 480-60 */
{ 848,  480,  75, P_NEG, P_POS, 1088,  504,  80,  5,  40,  3, 120,  16,  410000, 0x00086C52, VIC_unknown, GenByCVT,  0x48, Prog, VESA_SN(127)}, /* 351.CVT-848X 480-75 */
{ 848,  480,  85, P_NEG, P_POS, 1088,  507,  80,  5,  40,  3, 120,  19,  467500, 0x00086A4E, VIC_unknown, GenByCVT,  0x48, Prog, VESA_SN(128)}, /* 352.CVT-848X 480-85 */
{ 848,  480,  60, P_POS, P_NEG, 1008,  494,  32,  5,  48,  2,  80,   7,  297500, 0x00086C3B, VIC_unknown, GenByCVT_RB, 0x48, Prog, VESA_SN(129)}, /* 353.CVT-848X480-60-RB */
{1064,  600,  50, P_NEG, P_POS, 1320,  621, 104,  5,  24,  3, 128,  13,  407500, 0x00086A44, VIC_unknown, GenByCVT,  0x48, Prog, VESA_SN(130)}, /* 354.CVT-1064X600-50 */
{1064,  600,  60, P_NEG, P_POS, 1352,  624, 104,  5,  40,  3, 144,  16,  505000, 0x00006A2A, VIC_unknown, GenByCVT,  0x48, Prog, VESA_SN(131)}, /* 355.CVT-1064X600-60 */
{1064,  600,  75, P_NEG, P_POS, 1384,  629, 104,  5,  56,  3, 160,  21,  652500, 0x00006931, VIC_unknown, GenByCVT,  0x48, Prog, VESA_SN(132)}, /* 356.CVT-1064X600-75 */
{1064,  600,  85, P_NEG, P_POS, 1400,  633, 112,  5,  56,  3, 168,  25,  752500, 0x00006B45, VIC_unknown, GenByCVT,  0x48, Prog, VESA_SN(133)}, /* 357.CVT-1064X600-85 */
{1064,  600,  60, P_POS, P_NEG, 1224,  618,  32,  5,  48,  2,  80,  11,  452500, 0x00086B53, VIC_unknown, GenByCVT_RB, 0x48, Prog, VESA_SN(134)}, /* 358.CVT-1064X600-60-RB */
{1280,  720,  50, P_NEG, P_POS, 1632,  744, 128,  5,  48,  3, 176,  16,  605000, 0x00006723, VIC_unknown, GenByCVT,  0x48, Prog, VESA_SN(135)}, /* 359.CVT-1280X720-50 */
{1280,  720,  60, P_NEG, P_POS, 1664,  748, 128,  5,  64,  3, 192,  20,  745000, 0x00006A3E, VIC_unknown, GenByCVT,  0x48, Prog, VESA_SN(136)}, /* 360.CVT-1280X720-60 */
{1280,  720,  75, P_NEG, P_POS, 1696,  755, 128,  5,  80,  3, 208,  27,  957500, 0x00006738, VIC_unknown, GenByCVT,  0x48, Prog, VESA_SN(137)}, /* 361.CVT-1280X720-75 */
{1280,  720,  85, P_NEG, P_POS, 1712,  759, 136,  5,  80,  3, 216,  31, 1102500, 0x00004A2E, VIC_unknown, GenByCVT,  0x48, Prog, VESA_SN(138)}, /* 362.CVT-1280X720-85 */
{1280,  720,  60, P_POS, P_NEG, 1440,  741,  32,  5,  48,  2,  80,  14,  640000, 0x00006930, VIC_unknown, GenByCVT_RB, 0x48, Prog, VESA_SN(139)}, /* 363.CVT-1280X720-60-RB */
{1360,  768,  50, P_NEG, P_POS, 1744,  793, 136,  5,  56,  3, 192,  17,  690000, 0x0000682E, VIC_unknown, GenByCVT,  0x48, Prog, VESA_SN(140)}, /* 364.CVT-1360X768-50 */
{1360,  768,  60, P_NEG, P_POS, 1776,  798, 136,  5,  72,  3, 208,  22,  847500, 0x00006C55, VIC_unknown, GenByCVT,  0x48, Prog, VESA_SN(141)}, /* 365.CVT-1360X768-60 */
{1360,  768,  75, P_NEG, P_POS, 1808,  805, 144,  5,  80,  3, 224,  29, 1090000, 0x00004B32, VIC_unknown, GenByCVT,  0x48, Prog, VESA_SN(142)}, /* 366.CVT-1360X768-75 */
{1360,  768,  85, P_NEG, P_POS, 1824,  809, 144,  5,  88,  3, 232,  33, 1252500, 0x0000492F, VIC_unknown, GenByCVT,  0x48, Prog, VESA_SN(143)}, /* 367.CVT-1360X768-85 */
{1360,  768,  60, P_POS, P_NEG, 1520,  790,  32,  5,  48,  2,  80,  15,  720000, 0x0000672A, VIC_unknown, GenByCVT_RB, 0x48, Prog, VESA_SN(144)}, /* 368.CVT-1360X768-60-RB */
{1704,  960,  50, P_NEG, P_POS, 2232,  991, 176,  5,  88,  3, 264,  23, 1105000, 0x00004A2E, VIC_unknown, GenByCVT,  0x48, Prog, VESA_SN(145)}, /* 369.CVT-1704X960-50 */
{1704,  960,  60, P_NEG, P_POS, 2264,  996, 176,  5, 104,  3, 280,  28, 1352500, 0x00004B3E, VIC_unknown, GenByCVT,  0x48, Prog, VESA_SN(146)}, /* 370.CVT-1704X960-60 */
{1704,  960,  75, P_NEG, P_POS, 2296, 1005, 176,  5, 120,  3, 296,  37, 1727500, 0x00004A48, VIC_unknown, GenByCVT,  0x48, Prog, VESA_SN(147)}, /* 371.CVT-1704X960-75 */
{1704,  960,  85, P_NEG, P_POS, 2312, 1011, 184,  5, 120,  3, 304,  43, 1985000, 0x00004B5B, VIC_unknown, GenByCVT,  0x48, Prog, VESA_SN(148)}, /* 372.CVT-1704X960-85 */
{1704,  960,  60, P_POS, P_NEG, 1864,  988,  32,  5,  48,  2,  80,  21, 1102500, 0x00004A2E, VIC_unknown, GenByCVT_RB, 0x48, Prog, VESA_SN(149)}, /* 373.CVT-1704X960-60-RB */
{1864, 1050,  50, P_NEG, P_POS, 2456, 1083, 192,  5, 104,  3, 296,  25, 1327500, 0x00004B3D, VIC_unknown, GenByCVT,  0x48, Prog, VESA_SN(150)}, /* 374.CVT-1864X1050-50 */
{1864, 1050,  60, P_NEG, P_POS, 2488, 1089, 192,  5, 120,  3, 312,  31, 1625000, 0x0000493D, VIC_unknown, GenByCVT,  0x48, Prog, VESA_SN(151)}, /* 375.CVT-1864X1050-60 */
{1864, 1050,  75, P_NEG, P_POS, 2520, 1099, 200,  5, 128,  3, 328,  41, 2075000, 0x00002927, VIC_unknown, GenByCVT,  0x48, Prog, VESA_SN(152)}, /* 376.CVT-1864X1050-75 */
{1864, 1050,  85, P_NEG, P_POS, 2536, 1105, 200,  5, 136,  3, 336,  47, 2380000, 0x00002C3B, VIC_unknown, GenByCVT,  0x48, Prog, VESA_SN(153)}, /* 377.CVT-1864X1050-85 */
{1864, 1050,  60, P_POS, P_NEG, 2024, 1080,  32,  5,  48,  2,  80,  23, 1310000, 0x00004B3C, VIC_unknown, GenByCVT_RB, 0x48, Prog, VESA_SN(154)}, /* 378.CVT-1864X1050-60-RB */
{1920, 1080,  50, P_NEG, P_POS, 2544, 1114, 200,  5, 112,  3, 312,  26, 1415000, 0x00004A3B, VIC_unknown, GenByCVT,  0x48, Prog, VESA_SN(155)}, /* 379.CVT-1920X1080-50 */
{1920, 1080,  60, P_NEG, P_POS, 2576, 1120, 200,  5, 128,  3, 328,  32, 1730000, 0x00004A48, VIC_unknown, GenByCVT,  0x48, Prog, VESA_SN(156)}, /* 380.CVT-1920X1080-60 */
{1920, 1080,  75, P_NEG, P_POS, 2608, 1130, 208,  5, 136,  3, 344,  42, 2207500, 0x00002A2E, VIC_unknown, GenByCVT,  0x48, Prog, VESA_SN(157)}, /* 381.CVT-1920X1080-75 */
{1920, 1080,  85, P_NEG, P_POS, 2624, 1137, 208,  5, 144,  3, 352,  49, 2532000, 0x00002B3A, VIC_unknown, GenByCVT,  0x48, Prog, VESA_SN(158)}, /* 382.CVT-1920X1080-85 */
{1920, 1080,  60, P_POS, P_NEG, 2080, 1111,  32,  5,  48,  2,  80,  24, 1385000, 0x00004934, VIC_unknown, GenByCVT_RB, 0x48, Prog, VESA_SN(159)}, /* 383.CVT-1920X1080-60-RB */
{2128, 1200,  50, P_NEG, P_POS, 2832, 1238, 224,  5, 128,  3, 352,  30, 1750000, 0x00004733, VIC_unknown, GenByCVT,  0x48, Prog, VESA_SN(160)}, /* 384.CVT-2128X1200-50 */
{2128, 1200,  60, P_NEG, P_POS, 2864, 1245, 224,  5, 144,  3, 368,  37, 2137500, 0x00002B31, VIC_unknown, GenByCVT,  0x48, Prog, VESA_SN(161)}, /* 385.CVT-2128X1200-60 */
{2128, 1200,  75, P_NEG, P_POS, 2896, 1255, 224,  5, 160,  3, 384,  47, 2725000, 0x00002933, VIC_unknown, GenByCVT,  0x48, Prog, VESA_SN(162)}, /* 386.CVT-2128X1200-75 */
{2128, 1200,  60, P_POS, P_NEG, 2288, 1235,  32,  5,  48,  2,  80,  28, 1695000, 0x00005071, VIC_unknown, GenByCVT_RB, 0x48, Prog, VESA_SN(163)}, /* 388.CVT-2128X1200-60-RB */
{2560, 1440,  50, P_NEG, P_POS, 3456, 1484, 272,  5, 176,  3, 448,  36, 2562000, 0x00002930, VIC_unknown, GenByCVT,  0x48, Prog, VESA_SN(164)}, /* 389.CVT-2560X1440-50 */
{2560, 1440,  60, P_POS, P_NEG, 2720, 1481,  32,  5,  48,  2,  80,  34, 2415000, 0x00002723, VIC_unknown, GenByCVT_RB, 0x48, Prog, VESA_SN(165)}, /* 391.CVT-2560X1440-60-RB */
{2728, 1536,  50, P_NEG, P_POS, 3688, 1583, 288,  5, 192,  3, 480,  39, 2917000, 0x00002C49, VIC_unknown, GenByCVT,  0x48, Prog, VESA_SN(166)}, /* 392.CVT-2728X1536-50 */
{2728, 1536,  60, P_POS, P_NEG, 2888, 1580,  32,  5,  48,  2,  80,  37, 2737000, 0x00002A39, VIC_unknown, GenByCVT_RB, 0x48, Prog, VESA_SN(167)}, /* 393.CVT-2728X1536-60-RB */
{ 768,  480,  50, P_NEG, P_POS,  960,  497,  72,  6,  24,  3,  96,   8,  237500, 0x000C6C5F, VIC_unknown, GenByCVT,  0x48, Prog, VESA_SN(168)}, /* 394.CVT-768X 480-50 */
{ 768,  480,  60, P_NEG, P_POS,  960,  500,  72,  6,  24,  3,  96,  11,  287500, 0x00086A30, VIC_unknown, GenByCVT,  0x48, Prog, VESA_SN(169)}, /* 395.CVT-768X 480-60 */
{ 768,  480,  75, P_NEG, P_POS,  976,  504,  72,  6,  32,  3, 104,  15,  367500, 0x00086831, VIC_unknown, GenByCVT,  0x48, Prog, VESA_SN(170)}, /* 396.CVT-768X 480-75 */
{ 768,  480,  85, P_NEG, P_POS,  992,  507,  72,  6,  40,  3, 112,  18,  425000, 0x00086C55, VIC_unknown, GenByCVT,  0x48, Prog, VESA_SN(171)}, /* 397.CVT-768X 480-85 */
{ 768,  480,  60, P_POS, P_NEG,  928,  494,  32,  6,  48,  2,  80,   6,  275000, 0x00086C37, VIC_unknown, GenByCVT_RB, 0x48, Prog, VESA_SN(172)}, /* 398.CVT-768X480-60-RB */
{ 960,  600,  50, P_NEG, P_POS, 1200,  621,  96,  6,  24,  3, 120,  12,  370000, 0x00086C4A, VIC_unknown, GenByCVT,  0x48, Prog, VESA_SN(173)}, /* 399.CVT-960X 600-50 */
{ 960,  600,  60, P_NEG, P_POS, 1216,  624,  96,  6,  32,  3, 128,  15,  452500, 0x00086B53, VIC_unknown, GenByCVT,  0x48, Prog, VESA_SN(174)}, /* 400.CVT-960X 600-60 */
{ 960,  600,  75, P_NEG, P_POS, 1248,  629,  96,  6,  48,  3, 144,  20,  587500, 0x00006A31, VIC_unknown, GenByCVT,  0x48, Prog, VESA_SN(175)}, /* 401.CVT-960X 600-75 */
{ 960,  600,  85, P_NEG, P_POS, 1264,  633,  96,  6,  56,  3, 152,  24,  677500, 0x00006B3E, VIC_unknown, GenByCVT,  0x48, Prog, VESA_SN(176)}, /* 402.CVT-960X 600-85 */
{ 960,  600,  60, P_POS, P_NEG, 1120,  618,  32,  6,  48,  2,  80,  10,  415000, 0x00086C53, VIC_unknown, GenByCVT_RB, 0x48, Prog, VESA_SN(177)}, /* 403.CVT-960X600-60-RB */
{1152,  720,  50, P_NEG, P_POS, 1472,  744, 112,  6,  48,  3, 160,  15,  545000, 0x00006B32, VIC_unknown, GenByCVT,  0x48, Prog, VESA_SN(178)}, /* 404.CVT-1152X720-50 */
{1152,  720,  60, P_NEG, P_POS, 1488,  748, 112,  6,  56,  3, 168,  19,  667500, 0x00006932, VIC_unknown, GenByCVT,  0x48, Prog, VESA_SN(179)}, /* 405.CVT-1152X720-60 */
{1152,  720,  75, P_NEG, P_POS, 1520,  755, 120,  6,  64,  3, 184,  26,  857500, 0x00006732, VIC_unknown, GenByCVT,  0x48, Prog, VESA_SN(180)}, /* 406.CVT-1152X720-75 */
{1152,  720,  85, P_NEG, P_POS, 1536,  759, 120,  6,  72,  3, 192,  30,  990000, 0x00006842, VIC_unknown, GenByCVT,  0x48, Prog, VESA_SN(181)}, /* 407.CVT-1152X720-85 */
{1152,  720,  60, P_POS, P_NEG, 1312,  741,  32,  6,  48,  2,  80,  13,  582500, 0x00006722, VIC_unknown, GenByCVT_RB, 0x48, Prog, VESA_SN(182)}, /* 408.CVT-1152X720-60-RB */
{1224,  768,  50, P_NEG, P_POS, 1576,  793, 120,  6,  56,  3, 176,  16,  622500, 0x00006B39, VIC_unknown, GenByCVT,  0x48, Prog, VESA_SN(183)}, /* 409.CVT-1224x768-50 */
{1224,  768,  60, P_NEG, P_POS, 1592,  798, 120,  6,  64,  3, 184,  21,  760000, 0x00006939, VIC_unknown, GenByCVT,  0x48, Prog, VESA_SN(184)}, /* 410.CVT-1224x768-60 */
{1224,  768,  75, P_NEG, P_POS, 1624,  805, 128,  6,  72,  3, 200,  28,  977500, 0x00006739, VIC_unknown, GenByCVT,  0x48, Prog, VESA_SN(185)}, /* 411.CVT-1224x768-75 */
{1224,  768,  85, P_NEG, P_POS, 1640,  809, 128,  6,  80,  3, 208,  32, 1125000, 0x00004A2F, VIC_unknown, GenByCVT,  0x48, Prog, VESA_SN(186)}, /* 412.CVT-1224x768-85 */
{1224,  768,  60, P_POS, P_NEG, 1384,  790,  32,  6,  48,  2,  80,  14,  655000, 0x00006B3C, VIC_unknown, GenByCVT_RB, 0x48, Prog, VESA_SN(187)}, /* 413.CVT-1224X768-60-RB */
{1536,  960,  50, P_NEG, P_POS, 2016,  991, 160,  6,  80,  3, 240,  22,  997500, 0x00006A53, VIC_unknown, GenByCVT,  0x48, Prog, VESA_SN(188)}, /* 414.CVT-1536x960-50 */
{1536,  960,  60, P_NEG, P_POS, 2032,  996, 160,  6,  88,  3, 248,  27, 1212500, 0x00004C3D, VIC_unknown, GenByCVT,  0x48, Prog, VESA_SN(189)}, /* 415.CVT-1536x960-60 */
{1536,  960,  75, P_NEG, P_POS, 2064, 1005, 160,  6, 104,  3, 264,  36, 1552500, 0x00004B47, VIC_unknown, GenByCVT,  0x48, Prog, VESA_SN(190)}, /* 416.CVT-1536x960-75 */
{1536,  960,  85, P_NEG, P_POS, 2080, 1011, 160,  6, 112,  3, 272,  42, 1785000, 0x00005077, VIC_unknown, GenByCVT,  0x48, Prog, VESA_SN(191)}, /* 417.CVT-1536x960-85 */
{1536,  960,  60, P_POS, P_NEG, 1696,  988,  32,  6,  48,  2,  80,  20, 1005000, 0x00004B2E, VIC_unknown, GenByCVT_RB, 0x48, Prog, VESA_SN(192)}, /* 418.CVT-1536X960-60-RB */
{1680, 1050,  50, P_NEG, P_POS, 2208, 1083, 176,  6,  88,  3, 264,  24, 1195000, 0x00004723, VIC_unknown, GenByCVT,  0x48, Prog, VESA_SN(193)}, /* 419.CVT-1680x1050-50 */
{1680, 1050,  60, P_POS, P_NEG, 1840, 1080,  32,  6,  48,  2,  80,  22, 1190000, 0x00004C3B, VIC_unknown, GenByCVT_RB, 0x48, Prog, VESA_SN(194)}, /* 423.CVT-1680X1050-60-RB */
{1728, 1080,  50, P_NEG, P_POS, 2288, 1114, 176,  6, 104,  3, 280,  25, 1272500, 0x00004A35, VIC_unknown, GenByCVT,  0x48, Prog, VESA_SN(195)}, /* 424.CVT-1728x1080-50 */
{1728, 1080,  60, P_NEG, P_POS, 2320, 1120, 184,  6, 112,  3, 296,  31, 1557500, 0x00004834, VIC_unknown, GenByCVT,  0x48, Prog, VESA_SN(196)}, /* 425.CVT-1728x1080-60 */
{1728, 1080,  75, P_NEG, P_POS, 2336, 1130, 184,  6, 120,  3, 304,  41, 1977500, 0x00004842, VIC_unknown, GenByCVT,  0x48, Prog, VESA_SN(197)}, /* 426.CVT-1728x1080-75 */
{1728, 1080,  85, P_NEG, P_POS, 2352, 1137, 184,  6, 128,  3, 312,  48, 2270000, 0x00002B34, VIC_unknown, GenByCVT,  0x48, Prog, VESA_SN(198)}, /* 427.CVT-1728x1080-85 */
{1728, 1080,  60, P_POS, P_NEG, 1888, 1111,  32,  6,  48,  2,  80,  23, 1257500, 0x0000482A, VIC_unknown, GenByCVT_RB, 0x48, Prog, VESA_SN(199)}, /* 428.CVT-1728X1080-60-RB */
{1920, 1200,  50, P_NEG, P_POS, 2560, 1238, 200,  6, 120,  3, 320,  29, 1582500, 0x00004A42, VIC_unknown, GenByCVT,  0x48, Prog, VESA_SN(200)}, /* 429.CVT-1920x1200-50 */
{1920, 1200,  85, P_NEG, P_POS, 2624, 1262, 208,  6,  144, 3, 352, 53, 2812000, 0x00002729, VIC_unknown, GenByCVT,  0x48, Prog, VESA_SN(201)}, /* 432.CVT-1920x1200-85 */
#if (CONFIG_AST1500_SOC_VER >= 2)
{1920, 1200,  60, P_POS, P_NEG, 2080, 1235,  32,  6,  (48 - 40),  2, (80 + 40),  27, 1540000, 0x00004C4D, VIC_unknown, GenByCVT_RB, 0x48, Prog, VESA_SN(202)},/* 433.CVT-1920X1200-60-RB */
#else
{1920, 1200,  60, P_POS, P_NEG, 2080, 1235,  32,  6,  48,  2,  80,  27, 1540000, 0x00004C4D, VIC_unknown, GenByCVT_RB, 0x48, Prog, VESA_SN(202)}, /* 433.CVT-1920X1200-60-RB */
#endif
{2304, 1440,  50, P_NEG, P_POS, 3104, 1484, 248, 6,  152,  3, 400, 35, 2302500, 0x00002A30, VIC_unknown, GenByCVT,  0x48, Prog, VESA_SN(203)}, /* 434.CVT-2304x1440-50 */
{2304, 1440,  60, P_NEG, P_POS, 3136, 1493, 248, 6,  168,  3, 416, 44, 2807000, 0x00002729, VIC_unknown, GenByCVT,  0x48, Prog, VESA_SN(204)}, /* 435.CVT-2304x1440-60 */
{2304, 1440,  60, P_POS, P_NEG, 2464, 1481,  32, 6,   48,  2,  80, 33, 2187500, 0x00002929, VIC_unknown, GenByCVT_RB, 0x48, Prog, VESA_SN(205)}, /* 436.CVT-2304X1440-60-RB */
{2456, 1536,  50, P_NEG, P_POS, 3320, 1583, 264, 6,  168, 3, 432, 38, 2625000, 0x00002B3C, VIC_unknown, GenByCVT,  0x48, Prog, VESA_SN(206)}, /* 437.CVT-2456x1536-50 */
{2456, 1536, 60, P_POS, P_NEG, 2616, 1580, 32, 6,  48,  2, 80,  36, 2477500, 0x00002C3E, VIC_unknown, GenByCVT_RB, 0x48, Prog, VESA_SN(207)}, /* 439.CVT-2456X1536-60-RB */

/* PC Timing */
/* -HA  -VA   -Rate  -Hor   -Ver    -HT    -VT  -Hs -Vs  -HFP -VFP -HBP -VBP  -DCLK10000 -PClk       -HDMIType    -GCLK     -ADC03 -ScanMode    -SN */
{ 640,  350,  70, P_POS, P_NEG,  800,  449,  96,  2, 16,  35,  48,  62,  251750, 0x00086A2A, VIC_unknown, GenByOther, 0x48, Prog, VESA_SN(208)}, /* 801.IBM-640X350-70 */
{ 720,  400,  70, P_NEG, P_POS,  900,  449, 108,  2, 16,  13,  56,  34,  283200, 0x00086721, VIC_unknown, GenByOther, 0x48, Prog, VESA_SN(209)}, /* 803.IBM-720X400-70 */
{1024,  384,  87, P_POS, P_POS, 1264,  409, 176,  4, 16,   1,  48,  20,  449000, 0x0008683C, VIC_unknown, GenByOther, 0x48, Prog, VESA_SN(210)}, /* 804.IBM-1024X768-87 */
{ 640,  480,  67, P_NEG, P_NEG,  864,  525,  64,  3, 64,   3,  96,  39,  302400, 0x00086723, VIC_unknown, GenByOther, 0x48, Prog, VESA_SN(211)}, /* 806.MAC-640X480-66 */
{ 834,  624,  75, P_NEG, P_NEG, 1152,  667,  64,  3, 30,   1, 224,  39,  572800, 0x0000692B, VIC_unknown, GenByOther, 0x48, Prog, VESA_SN(212)}, /* 807.MAC-832X624-75 */
{1024,  768,  60, P_NEG, P_NEG, 1312,  813,  96,  6, 64,   6, 128,  33,  639900, 0x00006930, VIC_unknown, GenByOther, 0x48, Prog, VESA_SN(213)}, /* 808.MAC-1024X768-60 */
{1024,  768,  75, P_NEG, P_NEG, 1328,  804,  96,  2, 32,   4, 176,  30,  800000, 0x0000693C, VIC_unknown, GenByOther, 0x48, Prog, VESA_SN(214)}, /* 809.MAC-1024X768-75 */
{1152,  870,  75, P_NEG, P_NEG, 1456,  915, 128,  3, 32,   3, 144,  39, 1000000, 0x00004C32, VIC_unknown, GenByOther, 0x48, Prog, VESA_SN(215)}, /* 810.MAC-1152X870-75 */

/*SPWG Panel Timing*/
/* -HA  -VA -Rate  -Hor   -Ver  -HT   -VT   -Hs -Vs -HFP -VFP -HBP -VBP -DCLK10000 -PClk    -HDMIType    -GCLK     -ADC03 -ScanMode    -SN */
{1024,  768,  60, P_NEG, P_NEG, 1184,  790,  32,  4, 48,  3,  80,  15,  560000, 0x0000692A, VIC_unknown, GenByOther, 0x48, Prog, VESA_SN(216)}, /* 851.XGA-1024X768 */
{1400, 1050,  60, P_NEG, P_NEG, 1560, 1080,  32,  4, 48,  3,  80,  23, 1010000, 0x00004A2A, VIC_unknown, GenByOther, 0x48, Prog, VESA_SN(217)}, /* 852.SXGA+1400X1050 */
{1600, 1200,  60, P_NEG, P_NEG, 1760, 1235,  32,  4, 48,  3,  80,  28, 1301990, 0x00004726, VIC_unknown, GenByOther, 0x48, Prog, VESA_SN(218)}, /* 853.UXGA-1600X1200 */
{1280,  800,  60, P_NEG, P_NEG, 1440,  823,  32,  6, 48,  3,  80,  14,  710000, 0x00006C47, VIC_unknown, GenByOther, 0x48, Prog, VESA_SN(219)}, /* 854.WXGA 1280X800 */
{1440,  900,  60, P_NEG, P_NEG, 1600,  926,  32,  6, 48,  3,  80,  17,  887500, 0x00006A4A, VIC_unknown, GenByOther, 0x48, Prog, VESA_SN(220)}, /* 855.WXGA 1440X900 */
{1680, 1050,  60, P_NEG, P_NEG, 1840, 1080,  32,  6, 48,  3,  80,  21, 1190000, 0x00004C3B, VIC_unknown, GenByOther, 0x48, Prog, VESA_SN(221)}, /* 856.WSXGA+1680X1050 */
#if (CONFIG_AST1500_SOC_VER >= 2)
{1920, 1200,  60, P_NEG, P_NEG, 2080, 1235,  32,  6,  (48 - 40),  3, (80 + 40),  26, 1540000, 0x00004C4D, VIC_unknown, GenByOther, 0x48, Prog, VESA_SN(222)}, /* 857.WUXGA-1920X1200 */
#else
{1920, 1200,  60, P_NEG, P_NEG, 2080, 1235,  32,  6, 48,  3,  80,  26, 1540000, 0x00004C4D, VIC_unknown, GenByOther, 0x48, Prog, VESA_SN(222)}, /* 857.WUXGA-1920X1200 */
#endif
{2048, 1536,  60, P_NEG, P_NEG, 2208, 1580,  32,  4, 48,  3,  80,  37, 2092000, 0x00002B30, VIC_unknown, GenByOther, 0x48, Prog, VESA_SN(223)}, /* 858.QXGA-2048X1536 */

/* GTF */
{1024,  768,  75, P_POS, P_NEG, 1360,  802,  112, 3, 56,  1, 168,  30,  810079, 0x00006836, VIC_unknown, GenByVESA,  0x60, Prog, VESA_SN(224)}, /* This is GTF timing from nVidia */
{1280,  800,  75, P_POS, P_NEG, 1712,  835,  136, 3, 80,  1, 216,  31, 1070020, 0x00004B31, VIC_unknown, GenByCVT,   0xB0, Prog, VESA_SN(225)}, /* This is GTF timing from nVidia */

/* Additional modes (from quantumdata 780A) */
/* -HA -VA  -Rate  -Hor   -Ver  -HT    -VT  -Hs -Vs  -HFP -VFP -HBP -VBP  -DCLK10000 -PClk       -HDMIType    -GCLK     -ADC03 -ScanMode    -SN */
{ 852,  480,  60, P_NEG, P_NEG, 1072,  529,  96,  2,  16,  10, 108,  37,   340252,  0x00086933, VIC_unknown,  GenByOther,  0x48,  Prog, VESA_SN(226)},
{1400,  788,  60, P_POS, P_NEG, 1560,  811,  32,  5,  48,   3,  80,  15,   757500,  0x00007065, VIC_unknown,  GenByOther,  0x48,  Prog, VESA_SN(227)},
{1280, 1024,  96, P_POS, P_POS, 1760, 1082,  18,  3,  12,   1, 450,  54,  1258291,  0x0000482A, VIC_unknown,  GenByOther,  0x48,  Prog, VESA_SN(228)},
{1365, 1024,  60, P_POS, P_POS, 1840, 1087, 120,  3,  55,  12, 300,  48,  1200000,  0x00004723, VIC_unknown,  GenByOther,  0x48,  Prog, VESA_SN(229)},
{1365, 1024,  75, P_POS, P_POS, 1800, 1065, 136,  3,  32,   3, 267,  35,  1440000,  0x0000472A, VIC_unknown,  GenByOther,  0x48,  Prog, VESA_SN(230)},
{1440,  960,  72, P_NEG, P_NEG, 1792, 1000, 152,  3,  40,   1, 160,  36,  1290240,  0x0000482B, VIC_unknown,  GenByOther,  0x48,  Prog, VESA_SN(231)},
#if 0 /* pixel clock 234MHz may hang AST1520 system for unknown reason. Use 235MHz instead. */
{1920, 1440,  60, P_NEG, P_POS, 2600, 1500, 208,  3, 128,   1, 344,  56,  2340000,  0x00002827, VIC_unknown,  GenByOther,  0x48,  Prog, VESA_SN(232)}, /* duplicate of VESA_SN(54) */
#else
{1920, 1440,  60, P_NEG, P_POS, 2600, 1500, 208,  3, 128,   1, 344,  56,  2350000,  0x00002827, VIC_unknown,  GenByOther,  0x48,  Prog, VESA_SN(232)}, /* duplicate of VESA_SN(54) */
#endif
{1920, 1440,  75, P_NEG, P_POS, 2640, 1500, 224,  3, 144,   1,  44,  56,  2970000,  0x00003063, VIC_unknown,  GenByOther,  0x48,  Prog, VESA_SN(233)},
{2048, 1280,  60, P_NEG, P_NEG, 2784, 1325,  28,  3,  18,   1, 690,  41,  2213280,  0x00002A2E, VIC_unknown,  GenByOther,  0x48,  Prog, VESA_SN(234)},
{ 832,  624,  75, P_NEG, P_NEG, 1120,  654,  96,  3,  32,   3, 160,  24,   549360,  0x00006C37, VIC_unknown,  GenByOther,  0x48,  Prog, VESA_SN(235)},
{1024,  576,  60, P_POS, P_POS, 1320,  596, 144,  4,  16,   2, 136,  14,   472032,  0x0008683F, VIC_unknown,  GenByOther,  0x48,  Prog, VESA_SN(236)},
{1365, 1024,  60, P_NEG, P_NEG, 1832, 1060,  18,  3,  12,   1, 437,  32,  1165152,  0x00004722, VIC_unknown,  GenByOther,  0x48,  Prog, VESA_SN(237)},
{1400,  788,  60, P_NEG, P_NEG, 1800,  850, 112,  3, 248,  58,  40,   1,   918000,  0x00006B54, VIC_unknown,  GenByOther,  0x48,  Prog, VESA_SN(238)},
{1600, 1024,  60, P_NEG, P_NEG, 2144, 1060, 168,  3, 110,   1, 266,  32,  1363580,  0x00004933, VIC_unknown,  GenByOther,  0x48,  Prog, VESA_SN(239)},
/*
 * 2560x1200
 * following 2560x1200 timings are createed by VESA CVT generator
 */
/* -HA -VA -Rate  -Hor   -Ver    -HT   -VT  -Hs -Vs -HFP -VFP -HBP -VBP -DCLK10000 -PClk       -HDMIType      -GCLK     -ADC03 -ScanMode  -SN */
{2560, 1200,  30, P_NEG, P_POS, 3624, 1224, 256, 10,  96,   3, 352,  11,  1197500,  0x00004723, VIC_unknown,  GenByCVT,    0x48,  Prog, VESA_SN(240)}, /* 2560x1200@30Hz */
{2560, 1200,  60, P_NEG, P_POS, 3456, 1245, 272, 10, 176,   3, 448,  32,  2577500,  0x0000282B, VIC_unknown,  GenByCVT,    0x48,  Prog, VESA_SN(241)}, /* 2560x1200@60Hz */
{2560, 1200,  60, P_POS, P_NEG, 2720, 1235,  32, 10,  48,   3,  80,  22,  2015000,  0x00002A2A, VIC_unknown,  GenByCVT_RB, 0x48,  Prog, VESA_SN(242)}, /* 2560x1200@60Hz,RB */

/* 1400x1050 */
/* -HA -VA -Rate  -Hor   -Ver    -HT   -VT  -Hs -Vs -HFP -VFP -HBP -VBP -DCLK10000 -PClk       -HDMIType      -GCLK     -ADC03 -ScanMode  -SN */
{1400, 1050,  60, P_NEG, P_POS, 1880, 1087, 152,  3,  88,   1, 240,  33,  1126100,  0x0000504B, VIC_unknown,  GenByCVT,    0x48,  Prog, VESA_SN(243)}, /* 1400x1050@60Hz */
{1400, 1050,  72, P_NEG, P_POS, 1896, 1094, 152,  3,  96,   1, 248,  40,  1493400,  0x00004938, VIC_unknown,  GenByCVT,    0x48,  Prog, VESA_SN(244)}, /* 1400x1050@72Hz */
{1400, 1050,  75, P_NEG, P_POS, 1896, 1096, 152,  3,  96,   1, 248,  42,  1558500,  0x00004834, VIC_unknown,  GenByCVT,    0x48,  Prog, VESA_SN(245)}, /* 1400x1050@75Hz */
{1400, 1050,  85, P_NEG, P_POS, 1912, 1103, 152,  3, 104,   1, 256,  49,  1792600,  0x00004B52, VIC_unknown,  GenByCVT,    0x48,  Prog, VESA_SN(246)}, /* 1400x1050@85Hz */
{1400, 1050, 100, P_NEG, P_POS, 1928, 1112, 152,  3, 112,   1, 264,  58,  2143900,  0x00002B31, VIC_unknown,  GenByCVT,    0x48,  Prog, VESA_SN(247)}, /* 1400x1050@100Hz */

/* 1440x1050 */
/* -HA -VA -Rate  -Hor   -Ver    -HT   -VT  -Hs -Vs -HFP -VFP -HBP -VBP -DCLK10000 -PClk       -HDMIType      -GCLK     -ADC03 -ScanMode  -SN */
{1440, 1050,  60, P_NEG, P_POS, 1864, 1089, 144,  4,  88,   3, 232,  32,  1217500,  0x00004C3D, VIC_unknown,  GenByCVT,    0x48,  Prog, VESA_SN(248)}, /* 1440x1050@60Hz */
{1440, 1050,  75, P_NEG, P_POS, 1896, 1099, 144,  4, 104,   3, 248,  42,  1560000,  0x00004834, VIC_unknown,  GenByCVT,    0x48,  Prog, VESA_SN(249)}, /* 1440x1050@75Hz */
{1440, 1050,  85, P_NEG, P_POS, 1912, 1105, 152,  4, 104,   3, 256,  48,  1795000,  0x0000483C, VIC_unknown,  GenByCVT,    0x48,  Prog, VESA_SN(250)}, /* 1440x1050@85Hz */

/* 3840x1080 from LG monitor EDID */
/* -HA -VA -Rate  -Hor   -Ver    -HT   -VT  -Hs -Vs -HFP -VFP -HBP -VBP -DCLK10000 -PClk       -HDMIType      -GCLK     -ADC03 -ScanMode  -SN */
{3840, 1080,  25, P_POS, P_POS, 4800, 1099, 384, 10,  96,   3, 480,   6,  1317500,  0x0000482C, VIC_unknown, GenByOther,   0x48,  Prog, VESA_SN(251)}, /* 38400x1080@25Hz */
{3840, 1080,  30, P_POS, P_POS, 4848, 1102, 384, 10, 120,   3, 504,   9,  1600000,  0x0000493C, VIC_unknown, GenByOther,   0x48,  Prog, VESA_SN(252)}, /* 38400x1080@30Hz */
{3840, 1080,  60, P_POS, P_NEG, 4000, 1111,  32, 10,  48,   3,  80,  18,  2665000,  0x00002932, VIC_unknown,  GenByCVT,    0x48,  Prog, VESA_SN(253)}, /* 3840x1080@60Hz, RB */

/* special timing of requiremnet */
/* -HA -VA -Rate  -Hor   -Ver    -HT   -VT  -Hs -Vs -HFP -VFP -HBP -VBP -DCLK10000 -PClk       -HDMIType      -GCLK      -ADC03 -ScanMode  -SN */
{2560, 1440,  30, P_NEG, P_POS, 2720, 1401,  32,  5,  48,   3,  80,  13,  1190000,  0x00004C3B, VIC_unknown,  GenByCVT_RB, 0x48,  Prog, VESA_SN(254)}, /* 2560x1440@30Hz,RB, low rate */
{2560, 1600,  30, P_NEG, P_POS, 2720, 1623,  32,  6,  48,   3,  80,  14,  1322500,  0x0000482C, VIC_unknown,  GenByCVT_RB, 0x48,  Prog, VESA_SN(255)}, /* 2560x1600@30Hz,RB, low rate */

/* pixel clock out of spec. */
#if 0
{1920, 1200, 120, P_NEG, P_POS, 2080, 1271,  32,  6,  48,   3,   80,  62, 3170000, 0x00002A42, VIC_unknown, GenByVESA, 0x48, Prog, 0}, /* 558.DVI-1920X1200-75 */
{2048, 1536,  60, P_NEG, P_POS, 2708, 2048, 192,  3, 164, 463,  304,  46, 3300000, 0x00002837, VIC_unknown, GenByVESA, 0x48, Prog, 0}, /* 559.DVI-2048X1536-75 */
{1792, 1344, 120, P_POS, P_NEG, 1952, 1423,  32,  4,  48,   3,   80,  72, 3332500, 0x00002C53, VIC_unknown, GenByCVT_RB, 0xF0, Prog, 0},
{1856, 1392, 120, P_POS, P_NEG, 2016, 1474,  32,  4,  48,   3,   80,  75, 3565000, 0x00002734, VIC_unknown, GenByCVT_RB, 0xF0, Prog, 0},
{1920, 1200, 120, P_POS, P_NEG, 2080, 1271,  32,  6,  48,   3,   80,  62, 3170000, 0x00002A42, VIC_unknown, GenByCVT_RB, 0xF0, Prog}, /* HW not support */
{1920, 1440, 120, P_POS, P_NEG, 2080, 1525,  32,  4,  48,   3,   80,  78, 3805000, 0x00002C5F, VIC_unknown, GenByCVT_RB, 0xF0, Prog}, /* HW not support */
{2560, 1600,  60, P_NEG, P_POS, 3504, 1658, 280   6, 192,   3,  472,  49, 3485000, 0x0000283A, VIC_unknown, GenByCVT,    0xF0,  Prog}, /* Code not support */
{2560, 1600,  85, P_NEG, P_POS, 3536, 1682, 280,  6, 208,   3,  488,  73, 5052500, 0x0000294B, VIC_unknown, GenByCVT,    0xF0,  Prog}, /* HW not support */
{2560, 1600, 120, P_POS, P_NEG, 2720, 1694,  32,  6,  48,   3,   80,  85, 5527500, 0x0000294B, VIC_unknown, GenByCVT_RB, 0xF0,  Prog}, /* HW not support */
{2560, 1920,  60, P_POS, P_NEG, 2720, 1975,  32,  4,  48,   2,   80,  49, 3222000, 0x0000272F, VIC_unknown, GenByCVT_RB, 0x48, Prog, 0}, /* 338.CVT-2560X1920-60-RB */
{2128, 1200,  85, P_NEG, P_POS, 2912, 1262, 232,  5, 160,   3,  392,  54, 3122000, 0x00002834, VIC_unknown, GenByCVT,  0x48, Prog, 0}, /* 387.CVT-2128X1200-85 */
{2560, 1440,  60, P_NEG, P_POS, 3488, 1493, 272,  5, 192,   3,  464,  45, 3122000, 0x00002834, VIC_unknown, GenByCVT,  0x48, Prog, 0}, /* 390.CVT-2560X1440-60 */
{2456, 1536,  60, P_NEG, P_POS, 3352, 1592, 264,  6, 184,   3,  448,  47, 3200000, 0x0000293C, VIC_unknown, GenByCVT,  0x48, Prog, 0 }, /* 438.CVT-2456x1536-60 */
#endif
/******************************************************************************************************************************************************************/

/* User-defined Timing */
/*
 * template
 * -HA  -VA   -Rate  -Hor   -Ver    -HT    -VT  -Hs -Vs  -HFP -VFP -HBP -VBP  -DCLK10000 -PClk       -HDMIType    -GCLK     -ADC03 -ScanMode    -SN
 * {640,   480,    60, P_NEG, P_NEG,  800,  525,   96,  2,   16,  10,  48,  33,   252000, 0x00086A2A, VIC_DMT0659,  GenByHDTV,   0x28, Prog,  USER_SN(0)},
 *
 * Please add your own timing here.
 */
};

unsigned int AstModeCount = sizeof (AstModeTable) / sizeof (MODE_ITEM);

struct hdmi_vic_to_sn_convert
{
	e_hdmi_vic vic;
	unsigned short sn;
};

struct timing_convert {
	unsigned short sn;
	unsigned short new_sn;
};


struct hdmi_vic_to_sn_convert vic_to_sn_tb[] = {
	{VIC_unknown      ,TIMING_SN_INVALID      }, // =   0     ,
	{VIC_DMT0659      ,HDMI_SN(0)       }, // =   1     ,//4:3                  640x480p @ 59.94/60 Hz
	{VIC_480p         ,HDMI_SN(1)       }, // =   2     ,//4:3     8:9          720x480p @ 59.94/60 Hz
	{VIC_480pH        ,HDMI_SN(2)       }, // =   3     ,//16:9    32:37         720x480p @ 59.94/60 Hz
	{VIC_720p         ,HDMI_SN(3)       }, // =   4     ,//16:9     1:1         1280x720p @ 59.94/60 Hz
	{VIC_1080i        ,HDMI_SN(4)       }, // =   5     ,//16:9     1:1        1920x1080i @ 59.94/60 Hz
	{VIC_480i         ,HDMI_SN(5)       }, // =   6     ,// 4:3     8:9    720(1440)x480i @ 59.94/60 Hz
	{VIC_480iH        ,HDMI_SN(6)       }, // =   7     ,//16:9    32:37   720(1440)x480i @ 59.94/60 Hz
	{VIC_240p         ,HDMI_SN(7)       }, // =   8     ,// 4:3     8:9    720(1440)x240p @ 59.94/60 Hz
	{VIC_240pH        ,HDMI_SN(8)       }, // =   9     ,//16:9    32:37   720(1440)x240p @ 59.94/60 Hz
	{VIC_480i4x       ,HDMI_SN(32)      }, // =  10     ,// 4:3     8:9       (2880)x480i @ 59.94/60 Hz
	{VIC_480i4xH      ,HDMI_SN(32)      }, // =  11     ,//16:9    32:37      (2880)x480i @ 59.94/60 Hz
	{VIC_240p4x       ,HDMI_SN(7)       }, // =  12     ,// 4:3     8:9       (2880)x240p @ 59.94/60 Hz
	{VIC_240p4xH      ,HDMI_SN(8)       }, // =  13     ,//16:9    32:37      (2880)x240p @ 59.94/60 Hz
	{VIC_480p2x       ,HDMI_SN(1)       }, // =  14     ,// 4:3     8:9         1440x480p @ 59.94/60 Hz
	{VIC_480p2xH      ,HDMI_SN(2)       }, // =  15     ,//16:9    32:37        1440x480p @ 59.94/60 Hz
	{VIC_1080p        ,HDMI_SN(13)      }, // =  16     ,//16:9     1:1        1920x1080p @ 59.94/60 Hz
	{VIC_576p         ,HDMI_SN(14)      }, // =  17     ,// 4:3    16:15         720x576p @ 50 Hz
	{VIC_576pH        ,HDMI_SN(15)      }, // =  18     ,//16:9    64:45         720x576p @ 50 Hz
	{VIC_720p50       ,HDMI_SN(16)      }, // =  19     ,//16:9     1:1        1280x720p @ 50 Hz
	{VIC_1080i25      ,HDMI_SN(17)      }, // =  20     ,//16:9     1:1        1920x1080i @ 50 Hz*
	{VIC_576i         ,HDMI_SN(18)      }, // =  21     ,// 4:3    16:15   720(1440)x576i @ 50 Hz
	{VIC_576iH        ,HDMI_SN(19)      }, // =  22     ,//16:9    64:45   720(1440)x576i @ 50 Hz
	{VIC_288p         ,HDMI_SN(20)      }, // =  23     ,// 4:3    16:15   720(1440)x288p @ 50 Hz
	{VIC_288pH        ,HDMI_SN(21)      }, // =  24     ,//16:9    64:45   720(1440)x288p @ 50 Hz
	{VIC_576i4x       ,HDMI_SN(18)      }, // =  25     ,// 4:3    16:15      (2880)x576i @ 50 Hz
	{VIC_576i4xH      ,HDMI_SN(19)      }, // =  26     ,//16:9    64:45      (2880)x576i @ 50 Hz
	{VIC_288p4x       ,HDMI_SN(20)      }, // =  27     ,// 4:3    16:15      (2880)x288p @ 50 Hz
	{VIC_288p4xH      ,HDMI_SN(21)      }, // =  28     ,//16:9    64:45      (2880)x288p @ 50 Hz
	{VIC_576p2x       ,HDMI_SN(14)      }, // =  29     ,// 4:3    16:15        1440x576p @ 50 Hz
	{VIC_576p2xH      ,HDMI_SN(15)      }, // =  30     ,//16:9    64:45        1440x576p @ 50 Hz
	{VIC_1080p50      ,HDMI_SN(28)      }, // =  31     ,//16:9     1:1        1920x1080p @ 50 Hz
	{VIC_1080p24      ,HDMI_SN(29)      }, // =  32     ,//16:9     1:1        1920x1080p @ 23.98/24 Hz
	{VIC_1080p25      ,HDMI_SN(30)      }, // =  33     ,//16:9     1:1        1920x1080p @ 25 Hz
	{VIC_1080p30      ,HDMI_SN(45)      }, // =  34     ,//16:9     1:1        1920x1080p @ 29.97/30 Hz
	{VIC_480p4x       ,HDMI_SN(1)       }, // =  35     ,// 4:3     8:9       (2880)x480p @ 59.94/60 Hz
	{VIC_480p4xH      ,HDMI_SN(2)       }, // =  36     ,//16:9    32:37      (2880)x480p @ 59.94/60 Hz
	{VIC_576p4x       ,HDMI_SN(14)      }, // =  37     ,//4:3     16:15      (2880)x576p @ 50 Hz
	{VIC_576p4xH      ,HDMI_SN(15)      }, // =  38     ,//16:9    64:45      (2880)x576p @ 50 Hz
	{VIC_1080i25A     ,HDMI_SN(74)      }, // =  39     ,//16:9     1:1        1920x1080i @ 50 Hz* (1250 Total)
	{VIC_1080i50      ,HDMI_SN(75)      }, // =  40     ,//16:9     1:1        1920x1080i @ 100 Hz
	{VIC_720p100      ,HDMI_SN(76)      }, // =  41     ,//16:9     1:1         1280x720p @ 100 Hz
	{VIC_576p100      ,HDMI_SN(77)      }, // =  42     ,// 4:3     8:9          720x576p @ 100 Hz
	{VIC_576p100H     ,HDMI_SN(78)      }, // =  43     ,//16:9    32:37         720x576p @ 100 Hz
	{VIC_576i50       ,HDMI_SN(79)      }, // =  44     ,// 4:3    16:15   720(1440)x576i @ 100 Hz
	{VIC_576i50H      ,HDMI_SN(80)      }, // =  45     ,//16:9    64:45   720(1440)x576i @ 100 Hz
	{VIC_1080i60      ,HDMI_SN(81)      }, // =  46     ,//16:9     1:1        1920x1080i @ 119.88/120 Hz
	{VIC_720p120      ,HDMI_SN(82)      }, // =  47     ,//16:9     1:1         1280x720p @ 119.88/120 Hz
	{VIC_480p119      ,HDMI_SN(83)      }, // =  48     ,// 4:3    16:15         720x480p @ 119.88/120 Hz
	{VIC_480p119H     ,HDMI_SN(84)      }, // =  49     ,//16:9    64:45         720x480p @ 119.88/120 Hz
	{VIC_480i59       ,HDMI_SN(85)      }, // =  50     ,// 4:3     8:9    720(1440)x480i @ 119.88/120 Hz
	{VIC_480i59H      ,HDMI_SN(86)      }, // =  51     ,//16:9    32:37   720(1440)x480i @ 119.88/120 Hz
	{VIC_576p200      ,HDMI_SN(87)      }, // =  52     ,// 4:3    16:15         720x576p @ 200 Hz
	{VIC_576p200H     ,HDMI_SN(88)      }, // =  53     ,//16:9    64:45         720x576p @ 200 Hz
	{VIC_576i100      ,HDMI_SN(89)      }, // =  54     ,// 4:3    16:15   720(1440)x576i @ 200 Hz
	{VIC_576i100H     ,HDMI_SN(90)      }, // =  55     ,//16:9    64:45   720(1440)x576i @ 200 Hz
	{VIC_480p239      ,HDMI_SN(91)      }, // =  56     ,// 4:3     8:9          720x480p @ 239.76/240 Hz
	{VIC_480p239H     ,HDMI_SN(92)      }, // =  57     ,//16:9    32:37         720x480p @ 239.76/240 Hz
	{VIC_480i119      ,HDMI_SN(93)      }, // =  58     ,// 4:3     8:9    720(1440)x480i @ 239.76/240 Hz
	{VIC_480i119H     ,HDMI_SN(94)      }, // =  59     ,//16:9    32:37   720(1440)x480i @ 239.76/240 Hz
	{VIC_720p24       ,HDMI_SN(48)      }, // =  60     ,//16:9     1:1         1280x720p @ 23.98/24 Hz
	{VIC_720p25       ,HDMI_SN(49)      }, // =  61     ,//16:9     1:1         1280x720p @ 25Hz
	{VIC_720p30       ,HDMI_SN(50)      }, // =  62     ,//16:9     1:1         1280x720p @ 29.97/30 Hz
	{VIC_1080p120     ,HDMI_SN(13)      }, // TBD =  63     ,//16:9     1:1        1920x1080p @ 119.88/120 Hz
	{VIC_1080p100     ,HDMI_SN(28)      }, // TBD =  64     ,//16:9     1:1        1920x1080p @ 100 Hz
	{VIC_720p24W1     ,HDMI_SN(48)      }, // =  65     ,//64:27    4:3         1280x720p @ 23.98/24 Hz
	{VIC_720p25W1     ,HDMI_SN(25)      }, // =  66     ,//64:27    4:3         1280x720p @ 25Hz
	{VIC_720p30W1     ,HDMI_SN(50)      }, // =  67     ,//64:27    4:3         1280x720p @ 29.97/30 Hz
	{VIC_720p50W1     ,HDMI_SN(16)      }, // =  68     ,//64:27    4:3         1280x720p @ 50 Hz
	{VIC_720pW1       ,HDMI_SN(3)       }, // =  69     ,//64:27    4:3         1280x720p @ 59.94/60 Hz
	{VIC_720p100W1    ,HDMI_SN(76)      }, // =  70     ,//64:27    4:3         1280x720p @ 100 Hz
	{VIC_720p120W1    ,HDMI_SN(82)      }, // =  71     ,//64:27    4:3         1280x720p @ 119.88/120 Hz
	{VIC_1080p24W1    ,HDMI_SN(29)      }, // =  72     ,//64:27    4:3        1920x1080p @ 23.98/24 Hz
	{VIC_1080p25W1    ,HDMI_SN(30)      }, // =  73     ,//64:27    4:3        1920x1080p @ 25Hz
	{VIC_1080p30W1    ,HDMI_SN(45)      }, // =  74     ,//64:27    4:3        1920x1080p @ 29.97/30 Hz
	{VIC_1080p50W1    ,HDMI_SN(28)      }, // =  75     ,//64:27    4:3        1920x1080p @ 50 Hz
	{VIC_1080pW1      ,HDMI_SN(13)      }, // =  76     ,//64:27    4:3        1920x1080p @ 59.94/60 Hz
	{VIC_1080p100W1   ,HDMI_SN(28)      }, // =  77     ,//64:27    4:3        1920x1080p @ 100 Hz
	{VIC_1080p120W1   ,HDMI_SN(13)      }, // =  78     ,//64:27    4:3        1920x1080p @ 119.88/120 Hz
	{VIC_720p24W2     ,HDMI_SN(124)     }, // =  79     ,//64:27   64:63        1680x720p @ 23.98/24 Hz
	{VIC_720p25W2     ,HDMI_SN(125)     }, // =  80     ,//64:27   64:63        1680x720p @ 25Hz
	{VIC_720p30W2     ,HDMI_SN(126)     }, // =  81     ,//64:27   64:63        1680x720p @ 29.97/30 Hz
	{VIC_720p50W2     ,HDMI_SN(127)     }, // =  82     ,//64:27   64:63        1680x720p @ 50 Hz
	{VIC_720pW2       ,HDMI_SN(128)     }, // =  83     ,//64:27   64:63        1680x720p @ 59.94/60 Hz
	{VIC_720p100W2    ,HDMI_SN(129)     }, // =  84     ,//64:27   64:63        1680x720p @ 100 Hz
	{VIC_720p120W2    ,HDMI_SN(130)     }, // =  85     ,//64:27   64:63        1680x720p @ 119.88/120 Hz
	{VIC_1080p24W3    ,HDMI_SN(113)     }, // =  86     ,//64:27    1:1        2560x1080p @ 23.98/24 Hz
	{VIC_1080p25W3    ,HDMI_SN(114)     }, // =  87     ,//64:27    1:1        2560x1080p @ 25Hz
	{VIC_1080p30W3    ,HDMI_SN(115)     }, // =  88     ,//64:27    1:1        2560x1080p @ 29.97/30 Hz
	{VIC_1080p50W3    ,HDMI_SN(116)     }, // =  89     ,//64:27    1:1        2560x1080p @ 50 Hz
	{VIC_1080pW3      ,HDMI_SN(117)     }, // =  90     ,//64:27    1:1        2560x1080p @ 59.94/60 Hz
	{VIC_1080p100W3   ,HDMI_SN(118)     }, // Out of Spec =  91     ,//64:27    1:1        2560x1080p @ 100 Hz
	{VIC_1080p120W3   ,HDMI_SN(119)     }, // Out of Spec =  92     ,//64:27    1:1        2560x1080p @ 119.88/120 Hz
	{VIC_2160p24      ,HDMI_SN(103)     }, // =  93     ,//16:9     1:1        3840x2160p @ 23.98/24 Hz
	{VIC_2160p25      ,HDMI_SN(105)     }, // =  94     ,//16:9     1:1        3840x2160p @ 25Hz
	{VIC_2160p30      ,HDMI_SN(107)     }, // =  95     ,//16:9     1:1        3840x2160p @ 29.97/30 Hz
	{VIC_2160p50      ,HDMI_SN(104)     }, // =  96     ,//16:9     1:1        3840x2160p @ 50 Hz
	{VIC_2160p        ,HDMI_SN(106)     }, // =  97     ,//16:9     1:1        3840x2160p @ 59.94/60 Hz
	{VIC_2160p24E     ,HDMI_SN(108)     }, // =  98     ,//256:135   1:1        4096x2160p @ 23.98/24 Hz
	{VIC_2160p25E     ,HDMI_SN(110)     }, // =  99     ,//256:135   1:1        4096x2160p @ 25Hz
	{VIC_2160p30E     ,HDMI_SN(112)     }, // = 100     ,//256:135   1:1        4096x2160p @ 29.97/30 Hz
	{VIC_2160p50E     ,HDMI_SN(109)     }, // = 101     ,//256:135   1:1        4096x2160p @ 50 Hz
	{VIC_2160pE       ,HDMI_SN(111)     }, // = 102     ,//256:135   1:1        4096x2160p @ 59.94/60 Hz
	{VIC_2160p24W1    ,HDMI_SN(103)     }, // = 103     ,// 64:27    4:3        3840x2160p @ 23.98/24 Hz
	{VIC_2160p25W1    ,HDMI_SN(105)     }, // = 104     ,// 64:27    4:3        3840x2160p @ 25Hz
	{VIC_2160p30W1    ,HDMI_SN(107)     }, // = 105     ,// 64:27    4:3        3840x2160p @ 29.97/30 Hz
	{VIC_2160p50W1    ,HDMI_SN(104)     }, // = 106     ,// 64:27    4:3        3840x2160p @ 50 Hz
	{VIC_2160pW1      ,HDMI_SN(106)     }, // = 107     ,// 64:27    4:3        3840x2160p @ 59.94/60 Hz
	{VIC_MAX          ,TIMING_SN_INVALID      },
};

/*
** This table covert non-CRT-friendly timings. Ex: out of spec, Special HDMI VICs.
** FIXME: This is for SoC op mode >= 3 Only.
** FIXME: This table only takes 'force timing convert' into consideration.
*/
struct timing_convert crt_preferred_timing_sn_tb[] = {
	/* VIC_480p2x 4x, to VIC_480p */
	{HDMI_SN(11)    ,HDMI_SN(1)    },
	{HDMI_SN(12)    ,HDMI_SN(1)    },
	{HDMI_SN(39)    ,HDMI_SN(1)    },
	{HDMI_SN(70)    ,HDMI_SN(1)    },
	{HDMI_SN(71)    ,HDMI_SN(1)    },
	/* VIC_576p2x 4x, to VIC_576p */
	{HDMI_SN(26)    ,HDMI_SN(14)   },
	{HDMI_SN(27)    ,HDMI_SN(14)   },
	{HDMI_SN(40)    ,HDMI_SN(14)   },
	{HDMI_SN(67)    ,HDMI_SN(14)   },
	{HDMI_SN(68)    ,HDMI_SN(14)   },
	{HDMI_SN(72)    ,HDMI_SN(14)   },
	{HDMI_SN(73)    ,HDMI_SN(14)   },
	/* VIC_240p2x 4x, to VIC_240p */
	{HDMI_SN(53)    ,HDMI_SN(7)    },
	{HDMI_SN(54)    ,HDMI_SN(7)    },
	{HDMI_SN(57)    ,HDMI_SN(7)    },
	{HDMI_SN(58)    ,HDMI_SN(7)    },
	/* VIC_288p2x 4x, to VIC_288p */
	{HDMI_SN(61)    ,HDMI_SN(20)   },
	{HDMI_SN(62)    ,HDMI_SN(20)   },
	{HDMI_SN(65)    ,HDMI_SN(20)   },
	{HDMI_SN(66)    ,HDMI_SN(20)   },
	/* High refresh rate to low */
	{HDMI_SN(76)    ,HDMI_SN(16)   },
	{HDMI_SN(77)    ,HDMI_SN(14)   },
	{HDMI_SN(78)    ,HDMI_SN(14)   },
	{HDMI_SN(82)    ,HDMI_SN(16)   },
	{HDMI_SN(83)    ,HDMI_SN(1)    },
	{HDMI_SN(84)    ,HDMI_SN(1)    },
	{HDMI_SN(87)    ,HDMI_SN(14)   },
	{HDMI_SN(88)    ,HDMI_SN(14)   },
	{HDMI_SN(91)    ,HDMI_SN(1)    },
	{HDMI_SN(92)    ,HDMI_SN(1)    },
	/* 3D Special Timing */
	{HDMI_SN(95)    ,HDMI_SN(3)    },
	{HDMI_SN(96)    ,HDMI_SN(16)   },
	{HDMI_SN(97)    ,HDMI_SN(48)   },
	{HDMI_SN(98)    ,HDMI_SN(50)   },
	{HDMI_SN(99)    ,HDMI_SN(29)   },
	{HDMI_SN(100)   ,HDMI_SN(69)   },
	{HDMI_SN(101)   ,HDMI_SN(28)   },
	{HDMI_SN(102)   ,HDMI_SN(13)   },
	/* 2160p over spec. SoC V3 specific. */
	{HDMI_SN(104)   ,HDMI_SN(105)  },
	{HDMI_SN(106)   ,HDMI_SN(107)  },
	{HDMI_SN(109)   ,HDMI_SN(105)  }, /* SoC V3 can't convert 4096 to 1920. So, use 3840 timing instead. */
	{HDMI_SN(111)   ,HDMI_SN(107)  }, /* SoC V3 can't convert 4096 to 1920. So, use 3840 timing instead. */
	/* VIC_1080p W3 to 1080p */
	{HDMI_SN(113)   ,HDMI_SN(29)   },
	{HDMI_SN(114)   ,HDMI_SN(30)   },
	{HDMI_SN(115)   ,HDMI_SN(69)   },
	{HDMI_SN(116)   ,HDMI_SN(28)   },
	{HDMI_SN(117)   ,HDMI_SN(13)   },
	{HDMI_SN(118)   ,HDMI_SN(28)   },
	{HDMI_SN(119)   ,HDMI_SN(13)   },
	/* VE Only Timing. */
	{HDMI_SN(120)   ,HDMI_SN(105)  },
	{HDMI_SN(121)   ,HDMI_SN(107)  },
	{HDMI_SN(122)   ,HDMI_SN(105)  }, /* SoC V3 can't convert 4096 to 1920. So, use 3840 timing instead. */
	{HDMI_SN(123)   ,HDMI_SN(107)  }, /* SoC V3 can't convert 4096 to 1920. So, use 3840 timing instead. */
	{TIMING_SN_INVALID  ,TIMING_SN_INVALID      },
};

/* For 3D to 2D */
#define AST_TIMING_SN_1080P24_3D	(HDMI_SN(99))
#define AST_TIMING_SN_1080P30_3D	(HDMI_SN(100))
#define AST_TIMING_SN_1080P50_3D	(HDMI_SN(101))
#define AST_TIMING_SN_1080P60_3D	(HDMI_SN(102))
#define AST_TIMING_SN_1080P24	(HDMI_SN(29))
#define AST_TIMING_SN_1080P25	(HDMI_SN(30))
#define AST_TIMING_SN_1080P30	(HDMI_SN(45))
#define AST_TIMING_SN_1080P50	(HDMI_SN(28))
#define AST_TIMING_SN_1080P60	(HDMI_SN(13))

struct timing_convert hdmi_3d_to_2d[] = {
	{AST_TIMING_SN_1080P24_3D, AST_TIMING_SN_1080P24},
	{AST_TIMING_SN_1080P30_3D, AST_TIMING_SN_1080P30},
	{AST_TIMING_SN_1080P50_3D, AST_TIMING_SN_1080P50},
	{AST_TIMING_SN_1080P60_3D, AST_TIMING_SN_1080P60},
	{TIMING_SN_INVALID, TIMING_SN_INVALID}, /* TIMING_SN_INVALID means end of table */
};

/* For 2160p convert */
#define AST_TIMING_SN_4K24		(HDMI_SN(103))
#define AST_TIMING_SN_4K25		(HDMI_SN(105))
#define AST_TIMING_SN_4K30		(HDMI_SN(107))
#define AST_TIMING_SN_4K50		(HDMI_SN(104)) /* SoC V3 should never happen or used. */
#define AST_TIMING_SN_4K60		(HDMI_SN(106)) /* SoC V3 should never happen or used. */
#define AST_TIMING_SN_MD_4K24_420	(HDMI_SN(131))
#define AST_TIMING_SN_MD_4K25_420	(HDMI_SN(132))
#define AST_TIMING_SN_MD_4K30_420	(HDMI_SN(133))
#define AST_TIMING_SN_MD_4K24_420_SMPTE	(HDMI_SN(134))
#define AST_TIMING_SN_MD_4K25_420_SMPTE	(HDMI_SN(135))
#define AST_TIMING_SN_MD_4K30_420_SMPTE	(HDMI_SN(136))
#define AST_TIMING_SN_MD_4K50		(HDMI_SN(120))
#define AST_TIMING_SN_MD_4K60		(HDMI_SN(121))
#define AST_TIMING_SN_4K24_SMPTE	(HDMI_SN(108))
#define AST_TIMING_SN_4K25_SMPTE	(HDMI_SN(110))
#define AST_TIMING_SN_4K30_SMPTE	(HDMI_SN(112))
#define AST_TIMING_SN_4K50_SMPTE	(HDMI_SN(109)) /* SoC V3 should never happen or used. */
#define AST_TIMING_SN_4K60_SMPTE	(HDMI_SN(111)) /* SoC V3 should never happen or used. */
#define AST_TIMING_SN_MD_4K50_SMPTE	(HDMI_SN(122))
#define AST_TIMING_SN_MD_4K60_SMPTE	(HDMI_SN(123))

struct timing_convert hdmi_4k_to_1080p[] = {
	{AST_TIMING_SN_4K24, AST_TIMING_SN_1080P24},
	{AST_TIMING_SN_4K25, AST_TIMING_SN_1080P25},
	{AST_TIMING_SN_4K30, AST_TIMING_SN_1080P30},
	{AST_TIMING_SN_MD_4K50, AST_TIMING_SN_1080P50},
	{AST_TIMING_SN_MD_4K60, AST_TIMING_SN_1080P60},
	{AST_TIMING_SN_4K50, AST_TIMING_SN_1080P50}, /* SoC V3 should never happen or used. */
	{AST_TIMING_SN_4K60, AST_TIMING_SN_1080P60}, /* SoC V3 should never happen or used. */
	{AST_TIMING_SN_4K24_SMPTE, AST_TIMING_SN_1080P24},
	{AST_TIMING_SN_4K25_SMPTE, AST_TIMING_SN_1080P25},
	{AST_TIMING_SN_4K30_SMPTE, AST_TIMING_SN_1080P30},
	{AST_TIMING_SN_MD_4K50_SMPTE, AST_TIMING_SN_1080P50},
	{AST_TIMING_SN_MD_4K60_SMPTE, AST_TIMING_SN_1080P60},
	{AST_TIMING_SN_4K50_SMPTE, AST_TIMING_SN_1080P50}, /* SoC V3 should never happen or used. */
	{AST_TIMING_SN_4K60_SMPTE, AST_TIMING_SN_1080P60}, /* SoC V3 should never happen or used. */
	{TIMING_SN_INVALID, TIMING_SN_INVALID}, /* TIMING_SN_INVALID means end of table */
};

struct timing_convert hdmi_4k_to_4k_low[] = {
	{AST_TIMING_SN_MD_4K60, AST_TIMING_SN_4K30},
	{AST_TIMING_SN_MD_4K50, AST_TIMING_SN_4K25},
	{AST_TIMING_SN_MD_4K60_SMPTE, AST_TIMING_SN_4K30_SMPTE},
	{AST_TIMING_SN_MD_4K50_SMPTE, AST_TIMING_SN_4K25_SMPTE},
	{AST_TIMING_SN_4K60, AST_TIMING_SN_4K30}, /* SoC V3 should never happen or used. */
	{AST_TIMING_SN_4K50, AST_TIMING_SN_4K25}, /* SoC V3 should never happen or used. */
	{AST_TIMING_SN_4K60_SMPTE, AST_TIMING_SN_4K30_SMPTE}, /* SoC V3 should never happen or used. */
	{AST_TIMING_SN_4K50_SMPTE, AST_TIMING_SN_4K25_SMPTE}, /* SoC V3 should never happen or used. */
	{TIMING_SN_INVALID, TIMING_SN_INVALID}, /* TIMING_SN_INVALID means end of table */
};

struct timing_convert hdmi_4k_low_420_to_4k_low_444[] = {
	{AST_TIMING_SN_MD_4K24_420, AST_TIMING_SN_4K24},
	{AST_TIMING_SN_MD_4K25_420, AST_TIMING_SN_4K25},
	{AST_TIMING_SN_MD_4K30_420, AST_TIMING_SN_4K30},
	{AST_TIMING_SN_MD_4K24_420_SMPTE, AST_TIMING_SN_4K24_SMPTE},
	{AST_TIMING_SN_MD_4K25_420_SMPTE, AST_TIMING_SN_4K25_SMPTE},
	{AST_TIMING_SN_MD_4K30_420_SMPTE, AST_TIMING_SN_4K30_SMPTE},
	{TIMING_SN_INVALID, TIMING_SN_INVALID}, /* TIMING_SN_INVALID means end of table */
};

/* For interlace to progressive. */
#define AST_TIMING_SN_1920x1080i60     (HDMI_SN(4))
#define AST_TIMING_SN_1920x1080i60d1   (HDMI_SN(42))
#define AST_TIMING_SN_1920x1080i50     (HDMI_SN(17))
#define AST_TIMING_SN_1920x1080i50d1   (HDMI_SN(41))
#define AST_TIMING_SN_1920x1080i50_25A (HDMI_SN(74))
#define AST_TIMING_SN_1920x1080i100    (HDMI_SN(75))
#define AST_TIMING_SN_1920x1080i120    (HDMI_SN(81))
#define AST_TIMING_SN_720x480i60       (HDMI_SN(5))
#define AST_TIMING_SN_720x480i60d1     (HDMI_SN(32))
#define AST_TIMING_SN_720x480i60d2     (HDMI_SN(51))
#define AST_TIMING_SN_720x480i60H      (HDMI_SN(6))
#define AST_TIMING_SN_720x480i60Hd1    (HDMI_SN(52))
#define AST_TIMING_SN_720x480i60x4     (HDMI_SN(55))
#define AST_TIMING_SN_720x480i60x4H    (HDMI_SN(56))
#define AST_TIMING_SN_720x480i120      (HDMI_SN(85))
#define AST_TIMING_SN_720x480i120H     (HDMI_SN(86))
#define AST_TIMING_SN_720x480i240      (HDMI_SN(93))
#define AST_TIMING_SN_720x480i240H     (HDMI_SN(94))
#define AST_TIMING_SN_720x576i50       (HDMI_SN(18))
#define AST_TIMING_SN_720x576i50d1     (HDMI_SN(33))
#define AST_TIMING_SN_720x576i50d2     (HDMI_SN(59))
#define AST_TIMING_SN_720x576i50H      (HDMI_SN(60))
#define AST_TIMING_SN_720x576i50Hd1    (HDMI_SN(19))
#define AST_TIMING_SN_720x576i50x4     (HDMI_SN(63))
#define AST_TIMING_SN_720x576i50x4H    (HDMI_SN(64))
#define AST_TIMING_SN_720x576i100      (HDMI_SN(79))
#define AST_TIMING_SN_720x576i100H     (HDMI_SN(80))
#define AST_TIMING_SN_720x576i200      (HDMI_SN(89))
#define AST_TIMING_SN_720x576i200H     (HDMI_SN(90))
#define AST_TIMING_SN_1024x768i87      (VESA_SN(13))

#define AST_TIMING_SN_1920x1080p60     (AST_TIMING_SN_1080P60)
#define AST_TIMING_SN_1920x1080p50     (AST_TIMING_SN_1080P50)
#define AST_TIMING_SN_720x480p60       (HDMI_SN(1))
#define AST_TIMING_SN_720x576p50       (HDMI_SN(14))
#define AST_TIMING_SN_1024x768p50      (VESA_SN(94))
#define AST_TIMING_SN_1280x70p60       (HDMI_SN(3))
#define AST_TIMING_SN_1280x70p50       (HDMI_SN(16))


struct timing_convert table_1080i_to_720p[] = {
	{AST_TIMING_SN_1920x1080i60, AST_TIMING_SN_1280x70p60},
	{AST_TIMING_SN_1920x1080i60d1, AST_TIMING_SN_1280x70p60},
	{AST_TIMING_SN_1920x1080i50, AST_TIMING_SN_1280x70p50},
	{AST_TIMING_SN_1920x1080i50d1, AST_TIMING_SN_1280x70p50},
	{AST_TIMING_SN_1920x1080i50_25A, AST_TIMING_SN_1280x70p50},
	{AST_TIMING_SN_1920x1080i100, AST_TIMING_SN_1280x70p50},
	{AST_TIMING_SN_1920x1080i120, AST_TIMING_SN_1280x70p60},
	{TIMING_SN_INVALID, TIMING_SN_INVALID}, /* TIMING_SN_INVALID means end of table */
};

struct timing_convert table_interlace_to_progressive[] = {
	{AST_TIMING_SN_1920x1080i60     ,AST_TIMING_SN_1920x1080p60},
	{AST_TIMING_SN_1920x1080i60d1   ,AST_TIMING_SN_1920x1080p60},
	{AST_TIMING_SN_1920x1080i50     ,AST_TIMING_SN_1920x1080p50},
	{AST_TIMING_SN_1920x1080i50d1   ,AST_TIMING_SN_1920x1080p50},
	{AST_TIMING_SN_1920x1080i50_25A ,AST_TIMING_SN_1920x1080p50},
	{AST_TIMING_SN_1920x1080i100    ,AST_TIMING_SN_1920x1080p50},
	{AST_TIMING_SN_1920x1080i120    ,AST_TIMING_SN_1920x1080p60},
	{AST_TIMING_SN_720x480i60       ,AST_TIMING_SN_720x480p60},
	{AST_TIMING_SN_720x480i60d1     ,AST_TIMING_SN_720x480p60},
	{AST_TIMING_SN_720x480i60d2     ,AST_TIMING_SN_720x480p60},
	{AST_TIMING_SN_720x480i60H      ,AST_TIMING_SN_720x480p60},
	{AST_TIMING_SN_720x480i60Hd1    ,AST_TIMING_SN_720x480p60},
	{AST_TIMING_SN_720x480i60x4     ,AST_TIMING_SN_720x480p60},
	{AST_TIMING_SN_720x480i60x4H    ,AST_TIMING_SN_720x480p60},
	{AST_TIMING_SN_720x480i120      ,AST_TIMING_SN_720x480p60},
	{AST_TIMING_SN_720x480i120H     ,AST_TIMING_SN_720x480p60},
	{AST_TIMING_SN_720x480i240      ,AST_TIMING_SN_720x480p60},
	{AST_TIMING_SN_720x480i240H     ,AST_TIMING_SN_720x480p60},
	{AST_TIMING_SN_720x576i50       ,AST_TIMING_SN_720x576p50},
	{AST_TIMING_SN_720x576i50d1     ,AST_TIMING_SN_720x576p50},
	{AST_TIMING_SN_720x576i50d2     ,AST_TIMING_SN_720x576p50},
	{AST_TIMING_SN_720x576i50H      ,AST_TIMING_SN_720x576p50},
	{AST_TIMING_SN_720x576i50Hd1    ,AST_TIMING_SN_720x576p50},
	{AST_TIMING_SN_720x576i50x4     ,AST_TIMING_SN_720x576p50},
	{AST_TIMING_SN_720x576i50x4H    ,AST_TIMING_SN_720x576p50},
	{AST_TIMING_SN_720x576i100      ,AST_TIMING_SN_720x576p50},
	{AST_TIMING_SN_720x576i100H     ,AST_TIMING_SN_720x576p50},
	{AST_TIMING_SN_720x576i200      ,AST_TIMING_SN_720x576p50},
	{AST_TIMING_SN_720x576i200H     ,AST_TIMING_SN_720x576p50},
	{AST_TIMING_SN_1024x768i87      ,AST_TIMING_SN_1024x768p50}, /* There is no 43Hz progressive mode?! */
	{TIMING_SN_INVALID, TIMING_SN_INVALID}, /* TIMING_SN_INVALID means end of table */
};

#endif

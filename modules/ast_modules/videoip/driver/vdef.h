/*
 * Copyright (c) 2004-2012
 * ASPEED Technology Inc. All Rights Reserved
 * Proprietary and Confidential
 *
 * By using this code you acknowledge that you have signed and accepted
 * the terms of the ASPEED SDK license agreement.
 */

#ifndef _VDEF_H_
#define _VDEF_H_

#include <asm/arch/drivers/edid.h>
#include <aspeed/features_video.h>
#include <asm/arch/drivers/vbuf.h>
#include <asm/arch/regs-scu.h>

#ifndef API_VER
#define API_VER 2
#endif

#define TIMESTAMP_FROM_SCU 0

/*
 * according to H.J.
 * the configuration of user defined header parameter (VR0x80 (VR0x180) bit[8:0])
 * should got reference only when profile is enable
 *
 * But there is a improper behavior at AST1520:
 *	the control of bit 6 always takes effect whether profile is enabled
 *
 * It will cause AST1520 abnormal if we defined a special value which bit 6 is set
 * (and nobody cares about V1_USE_HEADER or V2_USE_HEADER
 * So just change V1_USE_HEADER and V2_USE_HEADER to 0x0
 */
#define V1_USE_HEADER               0x0000
#define V2_USE_HEADER               0x0000

#define     VIDEO1                              0
#define     VIDEO1_ON                           0x01
#if (CONFIG_AST1500_SOC_VER == 1)
#define     VIDEO2                              1
#define     VIDEO2_ON                           0x02

#define     VIDEOM_ON                          0x04
#define     VIDEOM                              2
#endif

#define     SINGLE_CODEC_SINGLE_CAPTURE			0
#define     AUTO_CODEC_SINGLE_CAPTURE				2
#define     AUTO_CODEC_AUTO_CAPTURE				3

#if (CONFIG_AST1500_SOC_VER >= 2)
/* ADC Engine */
#define		ADC_OFFSET							(0x9000) //offset from SCU_BASE
#define     ADC_CTRL                            (0+ADC_OFFSET)
	#define	ADC_ENGINE_ON_BIT   0
	#define	ADC_ENGINE_ON_MASK (1<<ADC_ENGINE_ON_BIT)
		#define	ADC_ENGINE_ON 1
		#define	ADC_ENGINE_OFF 0
	#define	ADC_OP_MODE_BIT   1
	#define	ADC_OP_MODE_MASK (0xF<<ADC_OP_MODE_BIT)
	#define	ADC_CH_EN_BIT   16
	#define	ADC_CH_EN_MASK (0x3FFF<<ADC_CH_EN_BIT)
#define     ADC_CLK_CTRL                        (0x0C+ADC_OFFSET)
	#define	ADC_CLK_DIV_BIT 0
	#define	ADC_CLK_DIV_MASK (0x3FF<<ADC_CLK_DIV_BIT)
#define     ADC_DATA_01                         (0x10+ADC_OFFSET)
	#define	ADC_D0_BIT 0
	#define	ADC_D0_MASK (0x3FF<<ADC_D0_BIT)
#endif /* #if (CONFIG_AST1500_SOC_VER >= 2) */

#define     INPUT_BITCOUNT_YUV444				4
#define     INPUT_BITCOUNT_YUV420				2

/* HW comment value */
//PASSWORD
#define     VIDEO_UNLOCK_KEY                    0x1A038AA8


//#define     SAMPLE_RATE                                 12000000.0
#ifdef OSC_NEW
    #define     SAMPLE_RATE                                 ((unsigned long)24576000)
#else
    #define     SAMPLE_RATE                                 ((unsigned long)24000000)
#endif

#define     MODEDETECTION_VERTICAL_STABLE_MAXIMUM       0x3 /* Bruce160530. Change from 0xa to 0x3. 3 frames should be enough. */
#define     MODEDETECTION_HORIZONTAL_STABLE_MAXIMUM     0xa
#define     MODEDETECTION_VERTICAL_STABLE_THRESHOLD     0x4
#define     MODEDETECTION_HORIZONTAL_STABLE_THRESHOLD   0x8

#define     MODEDETECTION_EDGE_PIXEL_THRES_DIGITAL      2
#define     MODEDETECTION_EDGE_PIXEL_THRES_ANALOGE      0x25 //0x20 Bruce160302. Increase from 0x20 to 0x25 for CPHD-3 1024x768p60Hz+One special board. Which need 0x21 to correctly detect boundary.

#define     MODEDETECTION_OK							0
#define     MODEDETECTION_ERROR							1
#define     JUDGE_MODE_ERROR							2
#define     MODEDETECTION_INTERRUPTED					3
#define     MODEDETECTION_TIMEOUT						4

#define	DUAL_EDGE_CLK10000 1625000 /* 160MHz => 162.5MHz to include 1. 1600x1200, 60Hz (162), 2. 1864x1050 60hz (162.5) */

extern u32 QValue[32];

typedef struct _CTL_REG_G {
	u32   CompressMode:1;
	u32   SkipEmptyFrame:1;
	u32   MemBurstLen:2;
	u32   LineBufEn:2;
	u32   Unused:26;
} CTL_REG_G;


typedef union _U_CTL_G {
	u32       Value;
	CTL_REG_G   CtlReg;
} U_CTL_G;

typedef struct _MODE_DETECTION_PARAM_REG {
	u32   Unused1:8;
	u32   EdgePixelThres:8;
	u32   VerStableMax:4;
	u32   HorStableMax:4;
	u32   VerDiffMax:4;
	u32   HorDiffMax:4;
} MODE_DETECTION_PARAM_REG;

typedef struct _CRC_PRI_PARAM_REG {
	u32   Enable:1;
	u32   HighBitOnly:1;
	u32   SkipCountMax:6;
	u32   PolyLow:8;
	u32   PolyHigh:16;
} CRC_PRI_PARAM_REG;

typedef union _U_CRC_PRI_PARAM {
	u32               Value;
	CRC_PRI_PARAM_REG   CRCPriParam;
} U_CRC_PRI_PARAM;

typedef struct _CRC_SEC_PARAM_REG {
	u32   Unused1:8;
	u32   PolyLow:8;
	u32   PolyHigh:16;
} CRC_SEC_PARAM_REG;

typedef union _U_CRC_SEC_PARAM {
	u32               Value;
	CRC_SEC_PARAM_REG   CRCSecParam;
} U_CRC_SEC_PARAM;

typedef struct _GENERAL_INFO {
	u8                EnableVideoM;
	u8                CenterMode;
	u8				RC4NoResetFrame;
	u8				RC4TestMode;
	U_CTL_G             uCtlReg;
	U_CRC_PRI_PARAM     uCRCPriParam;
	U_CRC_SEC_PARAM     uCRCSecParam;
} GENERAL_INFO, *PGENERAL_INFO;

typedef struct _SEQ_CTL_REG {
	u32   Unused1:1;
	u32   Unused2:1;
	u32   Unused3:1;
	u32   CaptureAutoMode:1;
	u32   Unused4:1;
	u32   CodecAutoMode:1;
	u32   Unused5:1;
	u32   WatchDog:1;
	u32   CRTSel:1;
	u32   AntiTearing:1;
	u32   DataType:2;
	u32   Unused6:20;
} SEQ_CTL_REG;

typedef union _U_SEQ_CTL {
	u32               Value;
	SEQ_CTL_REG   SeqCtlReg;
} U_SEQ_CTL;

typedef struct _CTL_REG {
	u32   SrcHsync:1;
	u32   SrcVsync:1;
	u32   ExtSrc:1;
	u32   AnalongExtSrc:1;
	u32   IntTimingGen:1;
	u32   IntDataFrom:1;
	u32   WriteFmt:2;
	u32   VGACursor:1;
#if (CONFIG_AST1500_SOC_VER == 1)
	u32   LinearMode:1;
	u32   ClockDelay:2;
#else
	u32	ClockDelay:3;
#endif
	u32   CCIR656Src:1;
	u32   PortClock:1;
#if (CONFIG_AST1500_SOC_VER == 1)
	u32   ExtPort:1;
#else
	u32   InterlaceMode:1;
#endif
#if (CONFIG_AST1500_SOC_VER == 1)
	u32   Unused1:1;
#else
	u32   HSyncInv:1;
#endif
	u32   FrameRate:8;
	u32   Unused2:8;
} CTL_REG;

typedef union _U_CTL {
	u32       Value;
	CTL_REG     CtlReg;
} U_CTL_REG;

typedef struct _TIMING_GEN_SETTING_H {
	u32   HDEEnd:13;
	u32   Unused1:3;
	u32   HDEStart:13;
	u32   Unused2:3;
} TIMING_GEN_SETTING_H;

typedef struct _TIMING_GEN_SETTING_V {
	u32   VDEEnd:13;
	u32   Unused1:3;
	u32   VDEStart:13;
	u32   Unused2:3;
} TIMING_GEN_SETTING_V;

typedef struct _BCD_CTL_REG {
	u32   Enable:1;
#if (CONFIG_AST1500_SOC_VER == 1)
	u32   Unused1:15;
#else
	u32   Enable_ABCD:1;
#if (CONFIG_AST1500_SOC_VER >= 3)
	u32   Double_Buf_BCD:1;
#else
	u32   Y_Only:1;
#endif
	u32   Pass2_Delay:2;
	u32   Pass3_Delay:3;
	u32   Unused1:8;
#endif
	u32   Tolerance:8;
#if (CONFIG_AST1500_SOC_VER == 1)
	u32   Unused2:8;
#else
	u32   Tolerance_ABCD:8;
#endif
} BCD_CTL_REG;

typedef union _U_BCD_CTL {
	u32           Value;
	BCD_CTL_REG     BCDCtlReg;
} U_BCD_CTL;

typedef struct _COMPRESS_WINDOW_REG {
	u32   VerLine:13;
	u32   Unused1:3;
	u32   HorPixel:13;
	u32   Unused2:3;
} COMPRESS_WINDOW_REG;

typedef struct _STREAM_BUF_SIZE {
	u32   PacketSize:3;
	u32   RingBufNum:2;
	u32   Unused1:11;
	u32   SkipHighMBThres:7;
	u32   SkipTestMode:2;
	u32   Unused2:7;
} STREAM_BUF_SIZE;

typedef union _U_STREAM_BUF {
	u32               Value;
	STREAM_BUF_SIZE     StreamBufSize;
} U_STREAM_BUF;


typedef struct _COMPRESS_CTL_REG {
	u32   JPEGOnly:1; /* True: Jpeg Only mode(Disable VQ), False:Jpeg and VQ mix mode */
	u32   En4VQ:1; /* True: 1, 2, 4 color mode, False: 1,2 color mode */
	u32   CodecMode:1; /* High and best Quantization encoding/decoding setting*/
	u32   DualQuality:1;
	u32   EnBest:1;
	u32   EnRC4:1;
	u32   NorChromaDCTTable:5;
	u32   NorLumaDCTTable:5;
	u32   EnHigh:1;
	u32   TestCtl:2;
	u32   UVFmt:1;
	u32   HufTable:2;
	u32   AlterValue1:5;
	u32   AlterValue2:5;
} COMPRESS_CTL_REG;

typedef union _U_COMPRESS_CTL {
	u32               Value;
	COMPRESS_CTL_REG    CompressCtlReg;
} U_COMPRESS_CTL;

typedef struct _QUANTI_TABLE_LOW_REG {
	u32   ChromaTable:5;
	u32   LumaTable:5;
	u32   Unused1:22;
} QUANTI_TABLE_LOW_REG;

typedef union _U_CQUANTI_TABLE_LOW {
	u32                   Value;
	QUANTI_TABLE_LOW_REG    QTableLowReg;
} U_QUANTI_TABLE_LOW;

typedef struct _QUANTI_VALUE_REG {
	u32   High:15;
	u32   Unused1:1;
	u32   Best:15;
	u32   Unused2:1;
} QUANTI_VALUE_REG;

typedef union _U_QUANTI_VALUE {
	u32               Value;
	QUANTI_VALUE_REG    QValueReg;
} U_QUANTI_VALUE;

typedef struct _BSD_PARAM_REG {
	u32   HighThres:8;
	u32   LowThres:8;
	u32   HighCount:6;
	u32   Unused1:2;
	u32   LowCount:6;
	u32   Unused2:2;
} BSD_PARAM_REG;

typedef union _U_BSD_PARAM {
	u32           Value;
	BSD_PARAM_REG   BSDParamReg;
} U_BSD_PARAM;

#if (API_VER < 2)
typedef struct _VIDEO_INFO {
	u8	  EnableRC4;
	u8      DownScalingMethod;
	u16    AnalogDifferentialThreshold; /* BCD tolerance */
	u16    DigitalDifferentialThreshold; /* BCD tolerance */
	u8      HighLumaTable; /* if High and best Jpeg codec enable, use HighLumaTable and HighChromaTable, otherwise HighDeQuantiValue and BestDequantiValue*/
	u8      HighChromaTable;
	u8      HighDeQuantiValue;
	u8      BestDequantiValue;
	U_SEQ_CTL               uSeqCtlReg;
	U_CTL_REG               uCtlReg;
	U_BCD_CTL               uBCDCtlReg;
	U_STREAM_BUF            uStreamBufSize;
	U_COMPRESS_CTL          uCompressCtlReg;
	U_QUANTI_TABLE_LOW      uQTableLowReg;
	U_QUANTI_VALUE          uQValueReg;
	U_BSD_PARAM             uBSDParamReg;
} VIDEO_INFO, *PVIDEO_INFO ;
#else //#if (API_VER < 2)
typedef struct _VIDEO_INFO {
	u8	  CryptoMode;//0:none;1:RC4;2:AES
	u8	  FastCrypto;
	u8      DownScalingMethod;
	u16    AnalogDifferentialThreshold; /* BCD tolerance */
	u16    DigitalDifferentialThreshold; /* BCD tolerance */
	u8      HighLumaTable; /* if High and best Jpeg codec enable, use HighLumaTable and HighChromaTable, otherwise HighDeQuantiValue and BestDequantiValue*/
	u8      HighChromaTable;
	u8      HighDeQuantiValue;
	u8      BestDequantiValue;
	U_SEQ_CTL               uSeqCtlReg;
	U_CTL_REG               uCtlReg;
	U_BCD_CTL               uBCDCtlReg;
	U_STREAM_BUF            uStreamBufSize;
	U_COMPRESS_CTL          uCompressCtlReg;
	U_QUANTI_TABLE_LOW      uQTableLowReg;
	U_QUANTI_VALUE          uQValueReg;
	U_BSD_PARAM             uBSDParamReg;
} VIDEO_INFO, *PVIDEO_INFO ;
#endif //#if (API_VER < 2)

typedef struct _VIDEOM_SEQ_CTL_REG {
	u32   Unused1:1;  //Bit 0
	u32   Unused2:1;	//Bit 1
	u32   Unused3:1;		//Bit 2
	u32   StreamMode:1;	//Bit 3
	u32   Unused4:1;	//Bit 4
	u32   CodecAutoMode:1;	//Bit 5
	u32	Unused6:1;	//Bit 6
	u32   Unused7:1;	//Bit 7
	u32   SrcSel:1;	//Bit 8
	u32   Unused9:1;	//Bit 9
	u32   DataType:2;  //Bit[11:10]
	u32   Unused12:20;
} VIDEOM_SEQ_CTL_REG;

typedef union _U_VIDEOM_SEQ_CTL {
	u32					Value;
	VIDEOM_SEQ_CTL_REG		SeqCtlReg;
} U_VIDEOM_SEQ_CTL;

typedef struct _VIDEOM_INFO {
	u8      DownScalingMethod;
	u16    AnalogDifferentialThreshold; /* BCD tolerance */
	u16    DigitalDifferentialThreshold; /* BCD tolerance */
	u8      HighLumaTable; /* if High and best Jpeg codec enable, use HighLumaTable and HighChromaTable, otherwise HighDeQuantiValue and BestDequantiValue*/
	u8      HighChromaTable;
	u8      HighDeQuantiValue;
	u8      BestDequantiValue;
	u8      PacketSize;   //the same as video1 & video2
	u8      RingBufNum;
	u8	  EnableRC4;
	U_VIDEOM_SEQ_CTL        uSeqCtlReg;
	U_BCD_CTL               uBCDCtlReg;
	U_COMPRESS_CTL          uCompressCtlReg;
	U_QUANTI_TABLE_LOW      uQTableLowReg;
	U_QUANTI_VALUE          uQValueReg;
	U_BSD_PARAM             uBSDParamReg;
} VIDEOM_INFO, *PVIDEOM_INFO ;

typedef struct _ENGINE_CONFIG
{
	VIDEO_INFO 		V1Info;
#if (API_VER < 2)
	VIDEO_INFO		V2Info;
	VIDEOM_INFO		VMInfo;
#endif
	GENERAL_INFO 	GenInfo;
} ENGINE_CONFIG, *PENGINE_CONFIG;


#endif



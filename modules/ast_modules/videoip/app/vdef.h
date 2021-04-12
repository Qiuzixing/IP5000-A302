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

#include <linux/types.h>

typedef struct _CTL_REG_G {
	__u32   CompressMode:1;
	__u32   SkipEmptyFrame:1;
	__u32   MemBurstLen:2;
	__u32   LineBufEn:2;
	__u32   Unused:26;
} CTL_REG_G;


typedef union _U_CTL_G {
	__u32       Value;
	CTL_REG_G   CtlReg;
} U_CTL_G;

typedef struct _MODE_DETECTION_PARAM_REG {
	__u32   Unused1:8;
	__u32   EdgePixelThres:8;
	__u32   VerStableMax:4;
	__u32   HorStableMax:4;
	__u32   VerDiffMax:4;
	__u32   HorDiffMax:4;
} MODE_DETECTION_PARAM_REG;

typedef struct _CRC_PRI_PARAM_REG {
	__u32   Enable:1;
	__u32   HighBitOnly:1;
	__u32   SkipCountMax:6;
	__u32   PolyLow:8;
	__u32   PolyHigh:16;
} CRC_PRI_PARAM_REG;

typedef union _U_CRC_PRI_PARAM {
	__u32               Value;
	CRC_PRI_PARAM_REG   CRCPriParam;
} U_CRC_PRI_PARAM;

typedef struct _CRC_SEC_PARAM_REG {
	__u32   Unused1:8;
	__u32   PolyLow:8;
	__u32   PolyHigh:16;
} CRC_SEC_PARAM_REG;

typedef union _U_CRC_SEC_PARAM {
	__u32               Value;
	CRC_SEC_PARAM_REG   CRCSecParam;
} U_CRC_SEC_PARAM;

typedef struct _GENERAL_INFO {
	__u8                EnableVideoM;
	__u8                CenterMode;
	__u8				RC4NoResetFrame;
	__u8				RC4TestMode;
	U_CTL_G             uCtlReg;
	U_CRC_PRI_PARAM     uCRCPriParam;
	U_CRC_SEC_PARAM     uCRCSecParam;
} GENERAL_INFO, *PGENERAL_INFO;

typedef struct _SEQ_CTL_REG {
	__u32   Unused1:1;
	__u32   Unused2:1;
	__u32   Unused3:1;
	__u32   CaptureAutoMode:1;
	__u32   Unused4:1;
	__u32   CodecAutoMode:1;
	__u32   Unused5:1;
	__u32   WatchDog:1;
	__u32   CRTSel:1;
	__u32   AntiTearing:1;
	__u32   DataType:2;
	__u32   Unused6:20;
} SEQ_CTL_REG;

typedef union _U_SEQ_CTL {
	__u32               Value;
	SEQ_CTL_REG   SeqCtlReg;
} U_SEQ_CTL;

typedef struct _CTL_REG {
	__u32   SrcHsync:1;
	__u32   SrcVsync:1;
	__u32   ExtSrc:1;
	__u32   AnalongExtSrc:1;
	__u32   IntTimingGen:1;
	__u32   IntDataFrom:1;
	__u32   WriteFmt:2;
	__u32   VGACursor:1;
	__u32   LinearMode:1;
	__u32   ClockDelay:2;
	__u32   CCIR656Src:1;
	__u32   PortClock:1;
	__u32   ExtPort:1;        
	__u32   Unused1:1;
	__u32   FrameRate:8;
	__u32   Unused2:8;
} CTL_REG;

typedef union _U_CTL {
	__u32       Value;
	CTL_REG     CtlReg;
} U_CTL_REG;

typedef struct _TIMING_GEN_SETTING_H {
	__u32   HDEEnd:13;
	__u32   Unused1:3;
	__u32   HDEStart:13;
	__u32   Unused2:3;
} TIMING_GEN_SETTING_H;

typedef struct _TIMING_GEN_SETTING_V {
	__u32   VDEEnd:13;
	__u32   Unused1:3;
	__u32   VDEStart:13;
	__u32   Unused2:3;
} TIMING_GEN_SETTING_V;

typedef struct _BCD_CTL_REG {
	__u32   Enable:1;
	__u32   Unused1:15;
	__u32   Tolerance:8;
	__u32   Unused2:8;
} BCD_CTL_REG;

typedef union _U_BCD_CTL {
	__u32           Value;
	BCD_CTL_REG     BCDCtlReg;
} U_BCD_CTL;

typedef struct _COMPRESS_WINDOW_REG {
	__u32   VerLine:13;
	__u32   Unused1:3;
	__u32   HorPixel:13;
	__u32   Unused2:3;
} COMPRESS_WINDOW_REG;

typedef struct _STREAM_BUF_SIZE {
	__u32   PacketSize:3;
	__u32   RingBufNum:2;
	__u32   Unused1:11;
	__u32   SkipHighMBThres:7;
	__u32   SkipTestMode:2;
	__u32   Unused2:7;
} STREAM_BUF_SIZE;

typedef union _U_STREAM_BUF {
	__u32               Value;
	STREAM_BUF_SIZE     StreamBufSize;
} U_STREAM_BUF;


typedef struct _COMPRESS_CTL_REG {
	__u32   JPEGOnly:1; /* True: Jpeg Only mode(Disable VQ), False:Jpeg and VQ mix mode */
	__u32   En4VQ:1; /* True: 1, 2, 4 color mode, False: 1,2 color mode */
	__u32   CodecMode:1; /* High and best Quantization encoding/decoding setting*/
	__u32   DualQuality:1;
	__u32   EnBest:1;
	__u32   EnRC4:1;
	__u32   NorChromaDCTTable:5;
	__u32   NorLumaDCTTable:5;
	__u32   EnHigh:1;
	__u32   TestCtl:2;
	__u32   UVFmt:1;
	__u32   HufTable:2;
	__u32   AlterValue1:5;
	__u32   AlterValue2:5;
} COMPRESS_CTL_REG;

typedef union _U_COMPRESS_CTL {
	__u32               Value;
	COMPRESS_CTL_REG    CompressCtlReg;
} U_COMPRESS_CTL;

typedef struct _QUANTI_TABLE_LOW_REG {
	__u32   ChromaTable:5;
	__u32   LumaTable:5;
	__u32   Unused1:22;
} QUANTI_TABLE_LOW_REG;

typedef union _U_CQUANTI_TABLE_LOW {
	__u32                   Value;
	QUANTI_TABLE_LOW_REG    QTableLowReg;
} U_QUANTI_TABLE_LOW;

typedef struct _QUANTI_VALUE_REG {
	__u32   High:15;
	__u32	Unused1:1;
	__u32   Best:15;
	__u32   Unused2:1;
} QUANTI_VALUE_REG;

typedef union _U_QUANTI_VALUE {
	__u32               Value;
	QUANTI_VALUE_REG    QValueReg;
} U_QUANTI_VALUE;

typedef struct _BSD_PARAM_REG {
	__u32   HighThres:8;
	__u32   LowThres:8;
	__u32   HighCount:6;
	__u32   Unused1:2;
	__u32   LowCount:6;
	__u32   Unused2:2;
} BSD_PARAM_REG;

typedef union _U_BSD_PARAM {
	__u32           Value;
	BSD_PARAM_REG   BSDParamReg;
} U_BSD_PARAM;

typedef struct _VIDEO_INFO {
	__u8	  EnableRC4;
	__u8      DownScalingMethod;
	__u16    AnalogDifferentialThreshold; /* BCD tolerance */
	__u16    DigitalDifferentialThreshold; /* BCD tolerance */
	__u8      HighLumaTable; /* if High and best Jpeg codec enable, use HighLumaTable and HighChromaTable, otherwise HighDeQuantiValue and BestDequantiValue*/
	__u8      HighChromaTable;
	__u8      HighDeQuantiValue;
	__u8      BestDequantiValue;
	U_SEQ_CTL               uSeqCtlReg;
	U_CTL_REG               uCtlReg;
	U_BCD_CTL               uBCDCtlReg;
	U_STREAM_BUF            uStreamBufSize;
	U_COMPRESS_CTL          uCompressCtlReg;
	U_QUANTI_TABLE_LOW      uQTableLowReg;
	U_QUANTI_VALUE          uQValueReg;
	U_BSD_PARAM             uBSDParamReg;
} VIDEO_INFO, *PVIDEO_INFO ;

typedef struct _VIDEOM_SEQ_CTL_REG {
	__u32   Unused1:1;  //Bit 0
	__u32   Unused2:1;	//Bit 1
	__u32   Unused3:1;		//Bit 2
	__u32   StreamMode:1;	//Bit 3
	__u32   Unused4:1;	//Bit 4
	__u32   CodecAutoMode:1;	//Bit 5
	__u32	Unused6:1;	//Bit 6
	__u32   Unused7:1;	//Bit 7
	__u32   SrcSel:1;	//Bit 8
	__u32   Unused9:1;	//Bit 9
	__u32   DataType:2;  //Bit[11:10]
	__u32   Unused12:20;
} VIDEOM_SEQ_CTL_REG;

typedef union _U_VIDEOM_SEQ_CTL {
	__u32					Value;
	VIDEOM_SEQ_CTL_REG		SeqCtlReg;
} U_VIDEOM_SEQ_CTL;

typedef struct _VIDEOM_INFO {
	__u8      DownScalingMethod;
	__u16    AnalogDifferentialThreshold; /* BCD tolerance */
	__u16    DigitalDifferentialThreshold; /* BCD tolerance */
	__u8      HighLumaTable; /* if High and best Jpeg codec enable, use HighLumaTable and HighChromaTable, otherwise HighDeQuantiValue and BestDequantiValue*/
	__u8      HighChromaTable;
	__u8      HighDeQuantiValue;
	__u8      BestDequantiValue;
	__u8      PacketSize;   //the same as video1 & video2
	__u8      RingBufNum;
	__u8	  EnableRC4;
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
	VIDEO_INFO		V2Info;
	VIDEOM_INFO		VMInfo;
	GENERAL_INFO 	GenInfo;
} ENGINE_CONFIG, *PENGINE_CONFIG;

typedef struct _VIDEO_INFO_V2 {
	__u8	  CryptoMode;//0:none;1:RC4;2:AES
	__u8	  FastCrypto;
	__u8      DownScalingMethod;
	__u16    AnalogDifferentialThreshold; /* BCD tolerance */
	__u16    DigitalDifferentialThreshold; /* BCD tolerance */
	__u8      HighLumaTable; /* if High and best Jpeg codec enable, use HighLumaTable and HighChromaTable, otherwise HighDeQuantiValue and BestDequantiValue*/
	__u8      HighChromaTable;
	__u8      HighDeQuantiValue;
	__u8      BestDequantiValue;
	U_SEQ_CTL               uSeqCtlReg;
	U_CTL_REG               uCtlReg;
	U_BCD_CTL               uBCDCtlReg;
	U_STREAM_BUF            uStreamBufSize;
	U_COMPRESS_CTL          uCompressCtlReg;
	U_QUANTI_TABLE_LOW      uQTableLowReg;
	U_QUANTI_VALUE          uQValueReg;
	U_BSD_PARAM             uBSDParamReg;
} VIDEO_INFO_V2, *PVIDEO_INFO_V2 ;

typedef struct _ENGINE_CONFIG_V2
{
	VIDEO_INFO_V2 		V1Info;
	GENERAL_INFO 	GenInfo;
} ENGINE_CONFIG_V2, *PENGINE_CONFIG_V2;

#endif /* _VDEF_H_ */


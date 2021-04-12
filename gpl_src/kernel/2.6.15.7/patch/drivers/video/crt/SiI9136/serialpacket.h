/*
 *****************************************************************************
 *
 * Copyright 2010, Silicon Image, Inc.  All rights reserved.
 * No part of this work may be reproduced, modified, distributed, transmitted,
 * transcribed, or translated into any language or computer format, in any form
 * or by any means without written permission of: Silicon Image, Inc., 1060
 * East Arques Avenue, Sunnyvale, California 94085
 *****************************************************************************
 */
/*
 *****************************************************************************
 * @file  serialpacket.h
 *
 * @brief Serial packet definitions.
 *
 *****************************************************************************
*/


/*
Any changes made to this file MUST be accompanied by
	corresponding changes to PacketDefGen.c
	so that the C# namespace will be kept in sync with this file.

Note:
	The Keil compiler packs structures on byte boundaries by default.

	Therefore, any usage of this file by an application compiled by a Microsoft compiler
	must bracket the inclusion of this file as follows:

#pragma pack(push,1)
#include "serialpacket.h"
#pragma pack(pop)

	Do NOT use Microsoft specific syntax within this file!!!!
*/
#ifndef _SERIALPACKET_H_
#define _SERIALPACKET_H_
//#include "si_basetypes.h"
//#include "videomodedesc.h"
#define STRINGIZE(x) #x
#define OFFSETOF(type, member) (unsigned int)(&((type *)0)->member)
#define PRINTF_ARGS_NAME(nType,nMemberLevel0,nMemberLevel1) "\t\t\tpublic const int %s=",STRINGIZE(nMemberLevel1)
#define PRINTF_ARGS_VALUE(nType,nMemberLevel0,nMemberLevel1) "0x%02X;\n",OFFSETOF(nType,nMemberLevel0.nMemberLevel1)




typedef struct _TpiFwVersion_t
{
	char major;
	char decimalPoint;
	char buildNumTensPlace;
	char buildNumOnesPlace;
    char buildNumAlphaPlace;
}TpiFwVersion_t,*PTpiFwVersion_t;


typedef struct _FirmwarePacketHeader_t
{
//	uint8_t StartOfPacket;
	uint8_t OpCode;
	uint8_t	PayloadSizeInBytes;  // Note this is the amount of data, NOT including this header
}FirmwarePacketHeader_t,*PFirmwarePacketHeader_t;

typedef struct _VideoModeChange_t
{
	uint8_t	VIC;									//Arg( 0)
	uint8_t	PRD;									//Arg( 1)
	uint8_t InputColorSpaceDepthRangeExpansion;		//Arg( 2)
	uint8_t OutputColorSpaceDepthRangeCompression;	//Arg( 3)
	uint8_t SYNC_GEN;								//Arg( 4)
	uint8_t SyncPolarityDetection;					//Arg( 5)
	uint8_t YC_Input_Mode;							//Arg( 6)
	uint8_t Colorimetry;							//Arg( 7)
	uint8_t ThreeDStructure_ExtData;				//Arg( 8)
	uint8_t AspectRatio;							//Arg( 9)
	uint8_t HDCPAuthenticated;						//Arg(10)
	uint8_t HDMIVideoFormat;						//Arg(11)
	uint8_t	ColorSpaceConversionMethod;				//Arg(12)
}VideoModeChange_t,*PVideoModeChange_t;


typedef struct _MapI2S_t
{
	uint8_t TPI_I2S_EN_0;
	uint8_t TPI_I2S_EN_1;
	uint8_t TPI_I2S_EN_2;
	uint8_t TPI_I2S_EN_3;
}MapI2S_t,*PMapI2S_t;

typedef struct _ConfigI2SInput_t
{
	uint8_t I2S_InputConfig;						//Arg(0)
}ConfigI2SInput_t,*PConfigI2SInput_t;

typedef struct _I2SStreamHeader_t
{
	uint8_t	channelStatusByte0;						//Arg( 0)
	uint8_t	channelStatusByte1;						//Arg( 1)
	uint8_t	channelStatusByte2;						//Arg( 2)
	uint8_t	clockkAccuracySamplingFreq;				//Arg( 3)
	uint8_t	sampleWordLength;						//Arg( 4)
}I2SStreamHeader_t,*PI2SStreamHeader_t;

typedef struct _HDMI_VSIF_t   // type:0x81 Version:0x01 Length:0x06 or 0x07
{
	// checksum only in HDMI,not in CEA spec.
	uint8_t	DataByte1_IEEE_Reg_Id_7_0;
	uint8_t	DataByte2_IEEE_Reg_Id_15_8;
	uint8_t	DataByte3_IEEE_Reg_Id_23_16;
	uint8_t	DataByte4_HDMI_Video_Format;
	uint8_t	DataByte5_HDMI_VIC;
	uint8_t	DataByte6_HDMI_3D_Structure; //optional
}HDMI_VSIF_t,*PHDMI_VSIF_t;

typedef struct _AVIInfoFrameData_t //type: 0x82 version: 0x02 Length: 0x0D
{
	// checksum only in HDMI,not in CEA spec.
	uint8_t	DataByte1_RV_Y1_Y0_A0_B1_B0_S1_S0;			//Arg(1)
	uint8_t	DataByte2_C1_C0_M1_M0_R3_R3_R1_R0;			//Arg(2)
	uint8_t	DataByte3_ITC_EC2_EC1_EC0_Q1_Q0_SC1_SC0;	//Arg(3)
	uint8_t	DataByte4_VIC;								//Arg(4)
	uint8_t	DataByte5_YQ1_YQ0_CN1_CN0_PR3_PR2_PR1_PR0;	//Arg(5)
	uint8_t	DataByte6_EndTopBarLSB;						//Arg(6)
	uint8_t	DataByte7_EndTopBarMSB;						//Arg(7)
	uint8_t	DataByte8_StartBottomBarLSB;
	uint8_t	DataByte9_StartBottomBarMSB;
	uint8_t	DataByte10_EndLeftBarLSB;
	uint8_t	DataByte11_EndLeftBarMSB;
	uint8_t	DataByte12_StartRightBarLSB;
	uint8_t	DataByte13_StartRightBarMSB;
}AVIInfoFrameData_t,*PAVIInfoFrameData_t;

typedef struct _SPDOrACP_t
{
	uint8_t dummy;
}SPDOrACP_t,*PSPDOrACP_t;

typedef struct _AudioInfoFrame_t	//type: 0x84 version: 0x01 Length: 0x0A
{
	// checksum only in HDMI,not in CEA spec.
	uint8_t	DataByte1_CT3_CT2_CT1_CT0_RSV_CC2_CC1_CC0;
	uint8_t	DataByte2_RSV_RSV_RSV_SF2_SF1_SF0_SS1_SS0;
	uint8_t	DataByte3_CTdependent;
	uint8_t	DataByte4_CA7_CA6_CA5_CA4_CA3_CA2_CA1_CA0;
	uint8_t	DataByte5_DmInh_LSV3_LSV2_LSV1_LSV0_RSV_LFEPBL1_LFEPBL0;
	uint8_t	DataByte6_Reserved;
	uint8_t	DataByte7_Reserved;
	uint8_t	DataByte8_Reserved;
	uint8_t	DataByte9_Reserved;
	uint8_t	DataByte10_Reserved;
}AudioInfoFrame_t,*PAudioInfoFrame_t;

typedef struct _MPEG_HDMI_VSIF_t
{
	uint8_t dummy;
}MPEG_HDMI_VSIF_t,*PMPEG_HDMI_VSIF_t;

typedef struct _Generic1_ISrc1_t
{
	uint8_t dummy;
}Generic1_ISrc1_t,*PGeneric1_ISrc1_t;

typedef struct _Generic2_Isrc2_t
{
	uint8_t dummy;
}Generic2_Isrc2_t,*PGeneric2_Isrc2_t;

typedef struct _MPEG_IF_t
{
	uint8_t dummy;
}MPEG_IF_t,*PMPEG_IF_t;

typedef struct _DedicatedGbd_t
{
	uint8_t dummy;
}DedicatedGbd_t,*PDedicatedGbd_t;


typedef union
{

	AVIInfoFrameData_t	aviInfoFrame;
	SPDOrACP_t			spdOrAcp;
	AudioInfoFrame_t	audio;
	MPEG_HDMI_VSIF_t	mpegHdmiVsif;
	Generic1_ISrc1_t	generic1Isrc1;
	Generic2_Isrc2_t	generic2Isrc2;
	MPEG_IF_t			mpeg;
	HDMI_VSIF_t			hdmiVsif;
	DedicatedGbd_t		dedicatedGbd;
	uint8_t				bytewise[1]; //'would like this to be zero, but keil complains...
}InfoFrameData_u;

typedef struct _SetInfoFramesData_t
{
	uint8_t	typeIF; // arg(0)
	InfoFrameData_u infoFrameData;

}SetInfoFramesData_t,*PSetInfoFramesData_t;
typedef struct _HwAviInfoFramesData_t
{
	uint8_t	typeIF;
	uint8_t CRC;
	InfoFrameData_u infoFrameData;
}HwAviInfoFramesData_t,*PHwAviInfoFramesData_t;

typedef struct _CommModeSwitch_t
{
	uint8_t state;
}CommModeSwitch_t,*PCommModeSwitch_t;

typedef struct _EHDMIMode_t
{
	uint8_t	mode;
}EHDMIMode_t,*PEHDMIMode_t;

typedef struct _HDCPOverrideMode_t
{
	uint8_t	mode;
}HDCPOverrideMode_t,*PHDCPOverrideMode_t;

typedef struct _CECAction_t
{
	uint8_t	action;
}CECAction_t,*PCECAction_t;

typedef struct SimulationControl_t
{
	uint8_t	flags;
}SimulationControl_t,*PSimulationControl_t;

typedef struct HDMItoDVISwitch_t
{
	uint8_t	mode;
}HDMItoDVISwitch_t,*PHDMItoDVISwitch_t;

typedef struct ReadBlockReq_t
{
	uint8_t	deviceId;
	uint8_t	offset;
	uint8_t	numBytes;
}ReadBlockReq_t,*PReadBlockReq_t;

typedef struct ReadBlockResp_t
{
	uint8_t readData[16];
}ReadBlockResp_t,*PReadBlockResp_t;

typedef struct WriteBlock_t
{
	uint8_t	deviceId;
	uint8_t	offset;
	uint8_t	numBytes;
	uint8_t writeData[16];
}WriteBlock_t,*PWriteBlock_t;

typedef struct ReadIndirectBlock_t
{
	uint8_t	page;
	uint8_t	offset;
	uint8_t	numBytes;
	uint8_t readData[16];
}ReadIndirectBlock_t,*PReadIndirectBlock_t;

typedef struct WriteIndirectBlock_t
{
	uint8_t	page;
	uint8_t	offset;
	uint8_t	numBytes;
	uint8_t writeData[16];
}WriteIndirectBlock_t,*PWriteIndirectBlock_t;

typedef struct _AudioModeSetup_t
{
	uint8_t	intfSelLayoutMuteEncoding;	//Arg(0)
	uint8_t	sampleSizeFreqHBR;			//Arg(1)
	uint8_t	speakerConfiguration;		//Arg(2)
	uint8_t	audioHandling;				//Arg(3)
	uint8_t	audioInputWordLength;		//Arg(4)
	uint8_t	channelCount;				//Arg(5)
	uint8_t	audioCheckSum;				//Arg(6)
}AudioModeSetup_t,*PAudioModeSetup_t;

typedef struct _SimulationConfigResponse_t
{
	uint8_t	flags;
}SimulationConfigResponse_t,*PSimulationConfigResponse_t;

typedef struct _EDIDBlockRequest_t
{
	uint8_t	BlockId;
}EDIDBlockRequest_t,*PEDIDBlockRequest_t;


typedef struct _EDIDBlockResponse_t
{
	uint8_t	edidData[128]; // variable length
}EDIDBlockResponse_t,*PEDIDBlockResponse_t;

typedef struct _VideoModeResponse_t
{
	uint8_t status;
	VideoModeDescription_t	vidModeDesc;
}VideoModeResponse_t, *PVideoModeResponse_t;

typedef struct _SyncData_t
{
	uint8_t	deDlyLow;					//Arg(0)
	uint8_t	deGenSyncPolDeDlyHigh;		//Arg(1)
	uint8_t	deTop;						//Arg(2)
	uint8_t	Arg3Reserved;				//Arg(3)
	uint8_t	deCnt7_0;					//Arg(4)
	uint8_t	deCnt11_8;					//Arg(5)
	uint8_t	deLin7_0;					//Arg(6)
	uint8_t	deLin10_8;					//Arg(7)
	uint8_t	hRes7_0;					//Arg(8)
	uint8_t	hRes13_8;					//Arg(9)
	uint8_t	vRes7_0;					//Arg(10)
	uint8_t	vRes11_8;					//Arg(11)
}SyncData_t,*PSyncData_t;
typedef struct _EmbeddedSyncData_t
{
	uint8_t	hBitToHSync7_0;				//Arg(12)
	uint8_t	embSyncExtr_hBitToHSync9_8;	//Arg(13)
	uint8_t	field2Offset7_0;			//Arg(14)
	uint8_t	field2Offset11_8;			//Arg(15)
	uint8_t	hWidth7_0;					//Arg(16)
	uint8_t hWidth9_8;					//Arg(17)
	uint8_t vBitToVSync;				//Arg(18)
	uint8_t	vWidth;						//Arg(19)

}EmbeddedSyncData_t,*PEmbeddedSyncData_t;

typedef struct _VideoSyncSetup_t
{
	SyncData_t			deSyncData;
	EmbeddedSyncData_t embSyncData;
}VideoSyncSetup_t,*PVideoSyncSetup_t;
typedef struct _ReadProductCode_t
{
	uint8_t	productCode;
}ReadProductCode_t,*PReadProductCode_t;

typedef struct _ReadFirmwareVersion_t
{
	TpiFwVersion_t	sVersion;
}ReadFirmwareVersion_t,*PReadFirmwareVersion_t;

typedef struct _ReadControlCommandVersion_t
{
	uint8_t	version;
}ReadControlCommandVersion_t,*PReadControlCommandVersion_t;

typedef struct _RequestEDID_t
{
	uint8_t blockId;
}RequestEDID_t,*pRequestEDID_t;

typedef struct _FirmwareCommandPacket_t
{
	FirmwarePacketHeader_t	header;
	union
	{
		uint8_t						bytes[128];
		VideoModeChange_t			vidModeChange;
		MapI2S_t					mapI2S;
		ConfigI2SInput_t			configI2SInput;
		I2SStreamHeader_t			i2sStreamHeader;
		SetInfoFramesData_t			setInfoFrames;
		HwAviInfoFramesData_t		hwAviInfoFramesData;
		CommModeSwitch_t			commModeSwitch;
		EHDMIMode_t					ehdmiMode;
		HDCPOverrideMode_t			hdcpOverrideMode;
		CECAction_t					cecAction;
		SimulationControl_t			simulationControl;
		HDMItoDVISwitch_t			hdmiToDviSwitch;
		ReadBlockReq_t				readBlockReq;
		ReadBlockResp_t				readBlockResp;
		WriteBlock_t				writeBlock;
		ReadIndirectBlock_t			readIndirectBlock;
		WriteIndirectBlock_t		writeIndirectBlock;
		AudioModeSetup_t			audioModeSetup;
		SimulationConfigResponse_t	simConfigResponse;
		EDIDBlockRequest_t			edidBlockRequest;
		EDIDBlockResponse_t			edidBlockResponse;
		VideoModeResponse_t			evtVidModeResponse;
		VideoSyncSetup_t			videoSyncSetup;
		ReadProductCode_t			readProductCode;
		ReadFirmwareVersion_t		readFirmwareVersion;
		ReadControlCommandVersion_t	readControlCommandVersion;
		RequestEDID_t				requestEdid;

	}payload;
}FirmwareCommandPacket_t,*PFirmwareCommandPacket_t;

typedef union _FirmwareScratchPad_u
{
	uint8_t		EDID_Data[128];
    uint8_t     printfData[128];
	FirmwareCommandPacket_t CommData;
}FirmwareScratchPad_u,*PFirmwareScratchPad_u;

typedef enum
{
	  VideoModeSetupWrite				= 0x01
	, VideoModeSetupRead				= 0x81


	, VideoSyncSetupRead				= 0x82

	, XVYColorControlDataWrite			= 0x03


	, AudioModeSetupWrite				= 0x08
	, AudioModeSetupRead				= 0x88

	, I2SMappingWrite					= 0x09


	, I2SInputConfigurationWrite		= 0x0A
	, I2SInputConfigurationRead			= 0x8A

	, I2SStreamHeaderSettingsWrite		= 0x0B
	, I2SStreamHeaderSettingsRead		= 0x8B

	, InfoFrameDataWrite 				= 0x10
	, InfoFrameDataRead					= 0x90

	, CommModeWrite						= 0x11
	, CommModeRead						= 0x91

	, EHDMImodeEnableDisableWrite		= 0x20
	, EHDMImodeEnableDisableRead		= 0xA0

	, HDCPOverrideModeWrite				= 0x30
	, HDCPOverrideModeRead				= 0xB0

	, CECActionWrite					= 0x40
	, CECActionRead						= 0xC0

	, SimulationControlWrite			= 0x48
	, SimulationControlRead				= 0xC8

	, HDMItoDVISwitchWrite				= 0x50
	, HDMItoDVISwitchRead				= 0xD0

	, BlockWrite						= 0x60
	, BlockRead							= 0xE0


	, ProductCodeRead					= 0xE1


	, FirmwareVersionRead				= 0xE2


	, FirmwareControlPacketVersionRead	= 0xE3
}SerialPacketOpCode_e;

typedef enum
{
      FwEventPing								= 0x01
    , FwEventPingResponse						= 0x81

	, RequestSimulationConfigurationEvent		= 0x02
	, RequestSimulationConfigurationResponse	= 0x82

	, RequestSimulatedEDIDPAgeEvent				= 0x03
	, RequestSimulatedEDIDPAgeResponse			= 0x83

	, RequestVideoModeEvent						= 0x04
	, RequestVideoModeResponse					= 0x84

	, RequestPowerUpInitEvent					= 0x05
	, RequestPowerUpInitResponse				= 0x85
}FirmwareEventOpCode_e;

// API Command Lengths
//=======================

// These are here because they are referenced in av_config.c
// That needs to change.

#define VIDEO_SETUP_CMD_LEN     	sizeof(VideoModeChange_t)

#define VIDEO_SYNC_SETUP_CMD_LEN	sizeof(VideoSyncSetup_t)

#define AUDIO_SETUP_CMD_LEN 		sizeof(AudioModeSetup_t)
#define I2S_MAPPING_CMD_LEN     	sizeof(MapI2S_t)
#define I2S_INPUT_CFG_CMD_LEN   	sizeof(ConfigI2SInput_t)
#define I2S_STREAM_HDR_CMD_LEN  	sizeof(I2SStreamHeader_t)

#define INFOFRAME_SIZE(x)           (sizeof(x) + sizeof(SetInfoFramesData_t) - sizeof(InfoFrameData_u))
#define AVI_INFOFRAME_CMD_LEN		INFOFRAME_SIZE(AVIInfoFrameData_t)
#define AUDIO_INFOFRAME_CMD_LEN		INFOFRAME_SIZE(AudioInfoFrame_t)

#define COMM_MODE_CMD_LEN			sizeof(CommModeSwitch_t)

#define EHDMI_MODE_CMD_LEN			sizeof(EHDMIMode_t)
#define HDCP_OVERRIDE_MODE_CMD_LEN	sizeof(HDCPOverrideMode_t)

#define PRODUCT_CODE_CMD_LEN		sizeof(ReadProductCode_t)
#define TPI_FW_VERSION_CMD_LEN		((uint8_t)sizeof(TPI_FW_VERSION))
#define CTRL_CMD_VERSION_CMD_LEN	sizeof(ReadControlCommandVersion_t)

#endif

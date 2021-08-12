#ifndef __FUNCEXCUTE_H__
#define __FUNCEXCUTE_H__

#ifdef __cplusplus
extern "C"{
#endif

#define KDS_BOARD_INFO_FILE "/etc/board_info.json"
#define KDS_VSRSION_FILE	"/etc/version"
#define KDS_HOSTNAME_FILE	"/etc/hostname"

#define MAX_CHAN_NAME_LEN 32
#define MAX_CHAN_IP_LEN 64
#define CEC_MAX_CMD_NAME_LEN 32
#define CEC_MAX_CMD_COMENT_LEN 32
#define IR_MAX_CMD_NAME_LEN 15
#define IR_MAX_CMD_COMENT_LEN 32
#define MAX_IP_ADDR_LEN 64
#define MAX_DEV_MOD_NAME_LEN 64
#define MAX_DEV_NAME_LEN 64
#define MAC_ADDR_LEN 32
#define SERIAL_NUMBER_LEN 16
#define MAX_SIGNALE_LEN  64
#define MAX_PORT_LEN 32
#define MAX_EDID_LEN 32
#define MAX_ONELOG_LEN 256
#define MAX_PARAM_LEN 256
#define BUFSIZE 128
#define PORT 5588
typedef enum _AudioInputMode_E
{
	AUDIO_IN_HDMI = 0,   
	AUDIO_IN_ANALOG = 1,
	AUDIO_IN_NONE = 2,
	ADUIO_IN_DANTE = 4
}AudioInputMode_E;
typedef enum _EDIDMode_E
{
	EDID_MODE_INPUT = 0,
	EDID_MODE_OUTPUT,
	EDID_MODE_DEFAULT,
	EDID_MODE_CUSTOM,

}EDIDMode_E;


typedef enum _PortDirectionType_E
{
	DIRECTION_IN,
	DIRECTION_OUT,
	DIRECTION_BOTH,
}PortDirectionType_E;
typedef enum _PortSignalType_E
{
	PORT_HDMI,
	PORT_ANALOG_AUDIO,
	PORT_RS232,
	PORT_IR,
	PORT_USB_A,
	PORT_USB_B,
	PORT_USB_C,
	PORT_DANTE,
	PORT_HDBT,
	PORT_AMPLIFIED_AUDIO,
	PORT_TOS,
	PORT_SPDIF,
	PORT_MIC,
	PORT_STREAM,
}PortSignalType_E;
typedef enum _SignalType_E
{
	SIGNAL_VIDEO,
	SIGNAL_AUDIO,
	SIGNAL_RS232,
	SIGNAL_IR,
	SIGNAL_USB,
	SIGNAL_ARC,
}SignalType_E;
typedef enum _HDCPMode_E
{
	HDCP_OFF = 0,
	HDCP_ON = 1,
	HDCP_MIRROR = 3,
}HDCPMode_E;

typedef enum _ViewMode_E
{
	VIEW_MODE_PIP_OFF = 0,
	VIEW_MODE_PIP_ON = 1,
	VIEW_MODE_PREVIEW = 2,
	VIEW_MODE_QUAD =3,
	VIEW_MODE_POP = 5,
	VIEW_MODE_VIDEOWALL = 15,

}ViewMode_E;
typedef enum _VideoRotation_E
{
	ROTATION_0,
	ROTATION_90,
	ROTATION_180,
	ROTATION_270,
}VideoRotation_E;

typedef enum _AudioSampleRate_E
{
	SAMPLE_RATE_8000 = 8000,
	SAMPLE_RATE_16000 = 16000,
	SAMPLE_RATE_22500= 22500,
	SAMPLE_RATE_32000 = 32000,
	SAMPLE_RATE_44100 = 44100,
	SAMPLE_RATE_48000 = 48000,
}AudioSampleRate_E;
typedef enum _AVConnectMode_E
{
	CONNECT_MANUAL,
	CONNECT_PRIROITY,
	CONNECT_LAST,
}AVConnectMode_E;
typedef enum _AudioFormat_E
{
	AUDIO_FORMAT_PCM,
}AudioFormat_E;
typedef enum _EDIDSourceTyep_E
{
	EDID_TYPE_INPUT,
	EDID_TYPE_OUTPUT,
	EDID_TYPE_DEFAULT,
	EDID_TYPE_CUSTOM,
}EDIDSourceTyep_E;

typedef enum _ColorSpaceMode_E
{
	COLOR_AUTO = 0,
	COLOR_RGB,
	COLOR_RGB_YUV444,
	COLOR_RGB_YUV422,
	COLOR_RGB_YUV444_YUV422,
}ColorSpaceMode_E;

typedef enum _VideoStatusType_E
{
	VIDEO_ENABLED,
	VIDEO_DISABLED,
	VIDEO_BY_BLANK_PICTURE,
	
}VideoStatusType_E;

typedef enum _CodecActionType_E
{
	CODEC_ACTION_STOP,
	CODEC_ACTION_PLAY,
	CODEC_ACTION_SAVECONFIG,
}CodecActionType_E;

typedef enum _EdidModeType_E
{
	PASSTHRU,
	CUSTOM,
	DEFAULT,
}EdidModeType_E;

typedef enum _State_E
{
	OFF,
	ON,
}State_E;

typedef struct   _MuteInfo_S
{
       PortDirectionType_E direction;
	PortSignalType_E portFormat;
	int portIndex ;
	SignalType_E signal;
	int index;
	State_E state;
}MuteInfo_S;

typedef struct   _EdidInfo_S
{
	int input_id;
        EdidModeType_E mode;
	int index;
}EdidInfo_S;

typedef struct   _EdidName_S
{
	int index;
	char name[32];
}EdidName_S;

typedef struct   _ChSelect_S
{
	SignalType_E  signal;
	int ch_id;
}ChSelect_S;

typedef struct   _AudioInfo_S
{
       PortDirectionType_E direction;
	PortSignalType_E portFormat;
	SignalType_E signal;
	int portIndex ;
}AudioInfo_S;


typedef struct   _PortInfo_S
{
       PortDirectionType_E direction;
	PortSignalType_E portFormat;
	SignalType_E signal;
	int portIndex ;
	int index;
}PortInfo_S;
typedef struct _AudioSignalInfo_S
{
	int chn ;
	AudioSampleRate_E sampleRate;
	AudioFormat_E format;
}AudioSignalInfo_S;

typedef struct _ViewModeInfo_S
{

	int hStyle;
	int vStyle;
}ViewModeInfo_S;
typedef struct  _WndBezelinfo_S
{
	int hValue;
	int vValue;
	int hOffset;
	int vOffset;
}WndBezelinfo_S;


typedef struct _VideoWallSetupInfo_S
{
	VideoRotation_E rotation;
}VideoWallSetupInfo_S;

typedef struct _ChannelInfo_S
{
	int id;
	char name[MAX_CHAN_NAME_LEN+1];
	char ip[MAX_CHAN_IP_LEN+1];
}ChannelInfo_S;
typedef struct _VideoReslotion_S
{
	int width;
	int height;
}VideoReslotion_S;


typedef struct _CECMessageInfo_S
{
	int portId;
	int serialNumb;
	int hexByte;
	char cmdName[CEC_MAX_CMD_NAME_LEN+1];
	char cmdComent[CEC_MAX_CMD_COMENT_LEN+1];
}CECMessageInfo_S;

typedef struct _IRMessageInfo_S
{
	int irId;
	int serialNumb;
	int repeat;
	int totalPacket;
	int packId;
	char cmdName[IR_MAX_CMD_NAME_LEN+1];
	char cmdComent[IR_MAX_CMD_COMENT_LEN+1];

}IRMessageInfo_S;

typedef enum _UartParityType_E
{
	PARITY_NONE,
	PARITY_ODD,
	PARITY_EVEN,
	PARITY_MARK,
	PARITY_SPACE,

}UartParityType_E;
typedef struct _UartMessageInfo_S
{
	int comNumber;
	int rate;
	int bitWidth;
	UartParityType_E parity;
	int serialType;
	int term_485;
	float stopBitsMode;
}UartMessageInfo_S;
typedef struct _ComRouteInfo_S
{
	int portType;
	int portNumber;
	int rePlay;
	int HeartTimeout; // 1-3600S
}ComRouteInfo_S;
typedef struct _TunnelParam_S
{
	int portType;
	int portNumber;
	int reMoteNumber;
	int rePlay;
	int connectType;
	char comName[IR_MAX_CMD_NAME_LEN+1];
	char ipAddr[MAX_IP_ADDR_LEN+1];
}TunnelParam_S;
typedef struct _NetWorkInfo_S
{
	char ipAddr[MAX_IP_ADDR_LEN+1];
	char mask[MAX_IP_ADDR_LEN+1];
	char gateway[MAX_IP_ADDR_LEN+1];
	char dns1[MAX_IP_ADDR_LEN+1];
	char dns2[MAX_IP_ADDR_LEN+1];
}NetWorkInfo_S;
typedef struct _BeaconInfo_S
{
	int portNumber;
	int udpPort;
	int tcpPort;
	char deviceMod[MAX_DEV_MOD_NAME_LEN+1];
	char deviceName[MAX_DEV_NAME_LEN+1];
	char macAddr[MAC_ADDR_LEN];
	char ipAddr[IR_MAX_CMD_NAME_LEN+1];
}BeaconInfo_S;
typedef struct _TimeSyncConf_S
{
	int enable; //使能
	int  syncInerval; //同步时间间隔
	char serverIp[MAX_IP_ADDR_LEN]; //服务器IP
}TimeSyncConf_S;

typedef struct _EDIDPortInfo_S
{
	int id;
	int type;
}EDIDPortInfo_S;

typedef enum _BoardInfoType_E
{
	BOARD_HOSTNAME,
	BOARD_MODEL,
	BOARD_SN,
	BOARD_MAC,
	BOARD_HW_VERSION,
	BOARD_FW_VERSION,
	BOARD_BUILD_DATE
}BoardInfoType_E;

int GetBoardInfo(BoardInfoType_E type, char* info, int size);

int  EX_SetAudSrcMode(int mode);
int  EX_GetAudSrcMode(int *mode);
int EX_SetAudGainLevel(PortInfo_S*info,int gain);
int EX_GetAudGainLevel(PortInfo_S*info,int *gain);
int EX_GetAudParam(PortInfo_S*info,AudioSignalInfo_S*param);
int EX_SetAutoSwitchMode(PortInfo_S*info,AVConnectMode_E mode);
int EX_GetAutoSwitchMode(PortInfo_S*info,AVConnectMode_E *mode);
int EX_GetEDIDSupport(int index ,int mode ,int *size);

int EX_CopyEDID(EDIDPortInfo_S*src,EDIDPortInfo_S*dest,int bitMap,int safemode);

int EX_SetEDIDColorSpaceMode(PortInfo_S *info,ColorSpaceMode_E mode);
int EX_GetEDIDColorSpaceMode(PortInfo_S *info,ColorSpaceMode_E *mode);

int EX_SetEDIDLockStatus(int index,int lock);
int EX_GetEDIDLockStatus(int index,int *lock);

int EX_SetHDCPMode(int index,HDCPMode_E mode);
int EX_GetHDCPMode(int index,HDCPMode_E *mode);

int EX_GetHDCPStatus(int io,int index);	

int EX_SetViewMode(ViewMode_E mode,ViewModeInfo_S*info);
int EX_GetViewMode(ViewMode_E *mode,ViewModeInfo_S*info);

int EX_SetWndBezelInfo( int mode ,int index, WndBezelinfo_S*info);
int EX_GetWndBezelInfo( int *mode ,int *index, WndBezelinfo_S*info);

int EX_SetVideoWallSetupInfo(int id,VideoWallSetupInfo_S *info);
int EX_GetVideoWallSetupInfo(int* id,VideoWallSetupInfo_S *info);

int EX_StartOverlay(char*confFile,int outtime);
int EX_StopOverlay(void);

int EX_SetEncoderAVChannelId(int id);
int EX_GetEncoderAVChannelId(int *id);

int EX_SetDecoderAVChannelId(ChSelect_S * id);
int EX_GetDecoderAVChannelId(ChSelect_S * id);

int EX_SetVideoImageStatus(int scalerId,VideoStatusType_E status);
int EX_GetVideoImageStatus(int scalerId,VideoStatusType_E *status);

int EX_SetVideoCodecAction(CodecActionType_E type);
int EX_GetVideoCodecAction(CodecActionType_E *type);

int EX_SetColorSpaceConvertMode(int index,int convertMode);
int EX_GetColorSpaceConvertMode(int index,int *convertMode);

int EX_SetVideoImageScaleMode(int mode,int res);
int EX_GetVideoImageScaleMode(int *mode,char*res);
int EX_GetVideoViewReslotion(int mode, int index, int nativeFlag,int * res);
int EX_GetVideoFrameRate(int *fps);
int EX_GetVideoBitRate(int *kbps);

int EX_SendCECMsg(CECMessageInfo_S*info);

int EX_RecvCECNtfy(int *port,int*hexByte,char*cmdComment);


int EX_SetCECGateWayMode(int mode);

int EX_SendIRmessage(IRMessageInfo_S*info);
int EX_SendIRStop(int irId,int serialNumb,char*command);
int EX_SetRouteMatch(PortInfo_S*inPortInfo,PortInfo_S*matchPortInfo,int num);
int EX_GetRouteMatch(PortInfo_S*inPortInfo,PortInfo_S*matchPortInfo);
int EX_SetUartConf(UartMessageInfo_S*conf);
int EX_GetUartConf(int comId,UartMessageInfo_S*conf);
int EX_AddComRoute(ComRouteInfo_S*info,int comId);
int EX_RemoveComRoute(int comId);
int EX_GetComRoute(int comId,ComRouteInfo_S*info);

int EX_GetOpenTunnelParam(int tunnelId,TunnelParam_S*param);

int EX_SetUSBCtrl(int type);
int EX_GetMulticastInfo(char*ip,int *ttl);
int EX_SetMacAddr(int netid,char*macAddr);
int EX_GetMacAddr(int netid,char*macAddr);
int EX_SetDNSName(int id,char*name);
int EX_GetDNSName(int id,char*name);
int EX_ResetDNSName(char *name);
int EX_SetDHCPMode(int netid,int mode);
int EX_GetDHCPMode(int netid,int* mode);
int EX_SetNetWorkConf(int netId,NetWorkInfo_S*netInfo);
int EX_GetNetWorkConf(int netId,NetWorkInfo_S*netInfo);
int EX_SetNetPort(char* portType,int portNumber);
int EX_GetNetPort(char*portType,int *portNumber);
int EX_SetSecurityStatus(int status);
int EX_Login(char*name,char*password);
int EX_GetLoginInfo(char*name,char*password);
int EX_Logout(void);
int EX_GetDevVersion(char*version);
int EX_Upgrade(void);
int EX_SetDeviceNameModel(char*mod);
int EX_GetDeviceNameModel(char*mod);
int EX_SetSerialNumber(char*data);
int EX_GetSerialNumber(char*data);

int EX_SetLockFP(int lockFlag);
int EX_GetLockFP(int *lockFlag);
int EX_SetIDV(void);
int EX_SetStandbyMode(int Mode);
int EX_DeviceReset(void);
int EX_FactoryRecovery(void);
int EX_SetBeacon(int portNumber,int status,int rateSecond);
int EX_GetBeaconInfo(int portNumber,BeaconInfo_S*info);

int EX_GetDevBuildDate(char *date,char*hms);
int EX_SetTimeAndDate(char*weekDay,char*date,char*hms);
int EX_GetTimeAndDate(char*weekDay,char*date,char*hms);
int EX_SetTimeZero(int tz,int timingMethod);
int EX_GetTimeZero(int *tz,int *timingMethod);
int EX_SetTimeSyncInfo(TimeSyncConf_S*syncInfo);
int EX_GetTimeSyncInfo(TimeSyncConf_S*syncInfo);

int EX_GetSignalList(char info[][MAX_SIGNALE_LEN],int num);
int EX_GetPortList(char info[][MAX_PORT_LEN],int num);
int EX_GetActiveCliNUm(void);
int EX_SetLogEvent(int action,int period);
int EX_GetLogTail(int lineNumber,char log[][MAX_ONELOG_LEN]);

int EX_GetAudAnalogGainDir(AudioInfo_S*info,char * gain);
int EX_SetAudAnalogGainDir(AudioInfo_S*info,char * gain);
int EX_SetAutoSwitchPriority(AudioInfo_S * info,AudioInfo_S * gain,int count);
int EX_SetEDIDMode(EdidInfo_S * info);
int EX_GetEDIDMode(int cmdID,EdidInfo_S * info);
int EX_GetEdidList(char info[][MAX_EDID_LEN],int num);
int EX_AddEDID(EdidName_S * info);
int EX_RemoveEDID(int comID);
int EX_SetActiveEDID(int input_ID,int index_ID );
int EX_GetActiveEDID(int input_ID );
int EX_SetEDIDNetSrc(int input_ID,char*macAddr );
int EX_GetEDIDNetSrc(int input_ID,char*macAddr );
int EX_GetEDIDNetSrc(int msg,char*date );
int EX_GetSendMsgNum(int msg,char*date );
int EX_GetRecvMsgNum(int msg,char*date );
int EX_GetVidOutRatio(char*date );
int EX_SetChannelName(char date[32] );
int EX_GetChannelName(char * date);
int EX_SetVidMute(MuteInfo_S * mute );
int EX_GetVidMute(MuteInfo_S * mute);
int EX_GetConnectionList(char info[][MAX_SIGNALE_LEN],int num);
int EX_GetHWVersion(char * date);
int EX_GetDevStatus(void);
int EX_GetHWTemp(int  id,int iMode);
int EX_GetAutoSwitchPriority(AudioInfo_S * gain,int count);
int EX_SetDanteName(char date[32]);
int EX_GetDanteName(char * date);
int EX_GetStandbyVersion(char * date);
int EX_GetUPGTime(char * day,char * time);
int EX_SetVidOutput(char info[][MAX_PARAM_LEN],int count );
int EX_GetVidOutput(char * date);
int EX_SetOsdDisplay(int  mode );
int EX_GetOsdDisplay(void);
int EX_SetDaisyChain(int  mode );
int EX_GetDaisyChain(void);
int EX_SetMethod(int  mode );
int EX_GetMethod(void);
int EX_SetTimeOut(int  iTime );
int EX_GetTimeOut(void);
int EX_SetMulticastStatus(char * ip,int ttl );
int EX_SetGatewayPort(int iGw_Type,int iNetw_Id);
int EX_GetGatewayPort(int iGw_Type);
int EX_SetVlanTag(int iGw_Type,int iTag);
int EX_GetVlanTag(int iGw_Type);
int EX_SetPassword(char * login_level,char * iNew_Pass);
int EX_GetPassword(char * login_level);
int EX_SetRollback(char * type);
int EX_GetLogEvent(int * action,int * period);
int EX_GetLogResetEvent(int * iLog,char*date,char*hms);
int EX_SetIRGateway(int  iIr_mode);
int EX_GetIRGateway(void);
int EX_GetStandbyMode(int * value);
int EX_SetStandbyTimeOut(int  iTime );
int EX_GetStandbyTimeOut(void);
int EX_RmEDID(int iEDID);
int EX_SetVideoWallStretch(int  index,int mode );
int EX_GetVideoWallStretch(int  index);
int EX_AutomaticReporting(char * info);
int EX_GetCECGateWayMode(void);
int Clear_Re(void);

int classTest(int a,int b);
#ifdef __cplusplus
}
#endif



#endif

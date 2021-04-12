/*
 * Copyright (c) 2004-2012
 * ASPEED Technology Inc. All Rights Reserved
 * Proprietary and Confidential
 *
 * By using this code you acknowledge that you have signed and accepted
 * the terms of the ASPEED SDK license agreement.
 */

#ifndef _VIDEOIP_IOCTL_H_
#define _VIDEOIP_IOCTL_H_

struct socket_info_t {
	/* Both host/client */
	unsigned int mtu; //1500 or 8000. little endian.
	unsigned int ip_daddr; //dest IP address. little endian.
	unsigned short udp_dest;  //destination port. little endian.

	/* host only */
	unsigned char mac_dest[6];
	unsigned char mac_source[6];
	unsigned int ip_saddr; //source IP address. little endian.
	unsigned short udp_source; //source port (optional 0). little endian.
};

//means client how to operate host stream.
enum {
	OP_USE_ONLY_V1 = 1,
	OP_USE_ONLY_V2,
	OP_TEST_CRT1,
	OP_TEST_CRT2
};

typedef  struct _IOCTL_CLIENT_INFO
{
	ENGINE_CONFIG 	e_cfg;

	int				V1TxSockFd;
#if 0//useless
	int				V2TxSockFd;
	int				VMTxSockFd;
#endif
	int				CmdSockFd;
	int				CurOp;
	/* For Multicast Support */
	int				V1McSockFd;
} IOCTL_CLIENT_INFO, *PIOCTL_CLIENT_INFO; 

typedef  struct _IOCTL_CLIENT_INFO_V2
{
	int CastingMode;//0:TCP-only unicasting; 1:unicasting; 2:multicasting; 3: UDP-only multicasting
	int 			CtrlSockFd;
	int 			DataSockFd;
	struct socket_info_t skinfo;
} IOCTL_CLIENT_INFO_V2, *PIOCTL_CLIENT_INFO_V2; 

#define	QUALITY_LEVEL_NUM		4
#define	QUALITY_TEXT		0
#define	QUALITY_HIGH		1
#define	QUALITY_MED			2
#define	QUALITY_LOW			3

#define	MAX_SECTION_NAME_LEN		16

typedef struct  _FILE_SECTION
{
	char	SectionName[MAX_SECTION_NAME_LEN];
	unsigned int		QualityLevel;
} FILE_SECTION, *PFILE_SECTION;

typedef struct  _QUALITY_PARAM
{
	unsigned int	NormalLumaTableSelection;
	unsigned int	NormalChromaTableSelection;
	unsigned int	HighQualityEnable;
	unsigned int	BestQualityEnable;
	unsigned int	HighDeValueOrHighLumaJpeg;
	unsigned int	BestDeValueOrHighChromaJpeg;
	unsigned int	CodecMode;
	unsigned int	FrameRate;
	unsigned int	HorScale;
	unsigned int	VerScale;
} QUALITY_PARAM, *PQUALITY_PARAM;

typedef struct  _CTRL_VIDEO
{
	QUALITY_PARAM		QualityParam[QUALITY_LEVEL_NUM];
	unsigned int		DynamicQualityEnable;
	unsigned int		DefaultQualityMode;
	unsigned int		StreamBufPktSize;
	unsigned int		StreamBufPktNum;
	unsigned int            VideoBitRateLimit;
} CTRL_VIDEO, *PCTRL_VIDEO; 

typedef  struct _IOCTL_CONFIG_INFO
{
	ENGINE_CONFIG	e_cfg;

	CTRL_VIDEO		Ctrl1920;
	CTRL_VIDEO		Ctrl1280;
	CTRL_VIDEO		Ctrl800;
} IOCTL_CONFIG_INFO, *PIOCTL_CONFIG_INFO; 

typedef  struct _IOCTL_REMOTE_INFO
{
	int CastingMode;//0:TCP-only unicasting; 1:unicasting; 2:multicasting; 3: UDP-only multicasting
	int 			CtrlSockFd;
	int 			DataSockFd;
	struct socket_info_t skinfo;
} IOCTL_REMOTE_INFO, *PIOCTL_REMOTE_INFO; 

#define	MAX_QUALITY_NUM		6

typedef struct  _QUALITY_PARAM_V2
{
	unsigned int	NormalLumaTableSelection;
	unsigned int	NormalChromaTableSelection;
	unsigned int	HighQualityEnable;
	unsigned int	BestQualityEnable;
	unsigned int	HighDeValueOrHighLumaJpeg;
	unsigned int	BestDeValueOrHighChromaJpeg;
	unsigned int	CodecMode;
	unsigned int	FrameRate;
	unsigned int	HorScale;
	unsigned int	VerScale;
	unsigned int	YUV420;
	unsigned int	JpegOnlyMode;
	unsigned int	VQ4ColorMode;
	unsigned int	EnableBCD;
	unsigned int	BCDThreshold;
	unsigned int	EnableABCD;
	unsigned int	ABCDThreshold;
	unsigned int	Pass2BCDDelay;
	unsigned int	Pass3BCDDelay;
	unsigned int	TruncatedBits;
	unsigned int	EnableDithering;
} QUALITY_PARAM_V2, *PQUALITY_PARAM_V2;

typedef struct  _CTRL_VIDEO_V2
{
	unsigned int		QualityNum;//the number of qualities in this profile
	QUALITY_PARAM_V2		QualityParam[MAX_QUALITY_NUM];
	unsigned int		DynamicQualityEnable;
	unsigned int		DefaultQualityMode;
	unsigned int		StreamBufPktSize;
	unsigned int		StreamBufPktNum;
	unsigned int            VideoBitRateLimit;
} CTRL_VIDEO_V2, *PCTRL_VIDEO_V2; 

#define MISC_CFG_MAGIC_NUM (0x15200101)
struct misc_cfg {
	unsigned int magic_num; /* 0x15200101 1520 1.1 */
	unsigned int port_select; /* 0: digital, 1: analog */
};

typedef union {
	struct misc_cfg misc_cfg;
	ENGINE_CONFIG_V2 e_cfg;
} MISC_CFG;

typedef  struct _IOCTL_CONFIG_INFO_V2
{
	MISC_CFG			cfg;
	CTRL_VIDEO_V2		Ctrl4K;
	CTRL_VIDEO_V2		Ctrl4K_low_frame_rate;
	CTRL_VIDEO_V2		Ctrl1920;
	CTRL_VIDEO_V2		Ctrl1920_low_frame_rate;
	CTRL_VIDEO_V2		Ctrl1280;
	CTRL_VIDEO_V2		Ctrl1280_low_frame_rate;
	CTRL_VIDEO_V2		Ctrl800;
} IOCTL_CONFIG_INFO_V2, *PIOCTL_CONFIG_INFO_V2; 

#define  IOCTL_HOST_ENABLE_REMOTE    	0x1101 //used to pass V1McSockFd in multicast mode
#define  IOCTL_HOST_ONLY_V1    	0x1103 //used to pass e_cfg, Ctrl1920, Ctrl1280, & Ctrl800 for AST1500
#define  IOCTL_CLIENT_ONLY_V1   0x1104
#define  IOCTL_HOST_ONLY_V2    	0x1105 //used to pass e_cfg, Ctrl1920, Ctrl1280, & Ctrl800 for AST1510 & later SOC versions (hope so)
#define  IOCTL_CLIENT_ONLY_V2   0x1106
#define  IOCTL_HOST_ADD_CLIENT    	0x1107
#if 0//useless
#define  IOCTL_V1HOST_TEST_CRT		0x1121
#define  IOCTL_V1CLIENT_TEST_CRT	0x1122
#define  IOCTL_V2HOST_TEST_CRT		0x1123
#define  IOCTL_V2CLIENT_TEST_CRT	0x1124
#endif

#endif /* _VIDEOIP_IOCTL_H_ */


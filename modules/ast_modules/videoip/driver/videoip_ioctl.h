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

#include "vdef.h"

struct socket_info_t {
	/* Both host/client */
	unsigned int mtu; //1500 or 8000
	unsigned int ip_daddr; //dest IP address. in little endian.
	unsigned short udp_dest;  //destination port. in little endian.

	/* host only */
	unsigned char mac_dest[6];
	unsigned char mac_source[6];
	unsigned int ip_saddr; //source IP address. in little endian.
	unsigned short udp_source; //source port (optional 0). in little endian.
};

typedef struct _IO_ACCESS_DATA {
    unsigned long Address;
    unsigned long Data;
    unsigned long Value;
} IO_ACCESS_DATA, *PIO_ACCESS_DATA;

typedef  struct _IOCTL_CLIENT_INFO
{
#if (API_VER < 2)
	ENGINE_CONFIG 	e_cfg;
#endif

	int CastingMode;//0:TCP-only unicasting; 1:unicasting; 2:multicasting; 3: UDP-only multicasting
	int CtrlSockFd;
	int DataSockFd;
	struct socket_info_t skinfo;
#if (API_VER >= 2)
	int session_id;
#endif
} IOCTL_CLIENT_INFO, *PIOCTL_CLIENT_INFO;

#if (API_VER < 2)
#define	QUALITY_LEVEL_NUM		4

typedef struct  _QUALITY_PARAM
{
	unsigned int	NormalLumaTableSelection;
	unsigned int	NormalChromaTableSelection;
	unsigned int	HighQualityEnable;
	unsigned int	BestQualityEnable;
	unsigned int	HighDeValueOrHighLumaJpeg;
	unsigned int	BestDeValueOrHighChromaJpeg;
	unsigned int	CodecMode;
	unsigned int	FrameRateControl;
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
	unsigned int		VideoBitRateLimit;
} CTRL_VIDEO, *PCTRL_VIDEO;
#else //#if (API_VER < 2)

#define	MAX_QUALITY_NUM		6
typedef struct  _QUALITY_PARAM
{
	unsigned int	NormalLumaTableSelection;
	unsigned int	NormalChromaTableSelection;
	unsigned int	HighQualityEnable;
	unsigned int	BestQualityEnable;
	unsigned int	HighDeValueOrHighLumaJpeg;
	unsigned int	BestDeValueOrHighChromaJpeg;
	unsigned int	CodecMode;
	unsigned int	FrameRateControl;
	unsigned int	HorScale;
	unsigned int	VerScale;
	unsigned int	YUV420;
	unsigned int	JpegOnlyMode;
	unsigned int	VQ4ColorMode;
	unsigned int	EnableBCD; // 0:disable, 1:enable, 3:enable with double BCD
	unsigned int	BCDThreshold;
	unsigned int	EnableABCD;
	unsigned int	ABCDThreshold;
	unsigned int	Pass2BCDDelay;
	unsigned int	Pass3BCDDelay;
	unsigned int	TruncatedBits;
	unsigned int	EnableDithering;
} QUALITY_PARAM, *PQUALITY_PARAM;

typedef struct  _CTRL_VIDEO
{
	unsigned int		QualityNum;//the number of qualities in this profile
	QUALITY_PARAM		QualityParam[MAX_QUALITY_NUM];
	unsigned int		DynamicQualityEnable;
	unsigned int		DefaultQualityMode;
	unsigned int		StreamBufPktSize;
	unsigned int		StreamBufPktNum;
	unsigned int		VideoBitRateLimit;
} CTRL_VIDEO, *PCTRL_VIDEO;
#endif //#if (API_VER < 2)

struct misc_cfg {
	unsigned int magic_num; /* MISC_CFG_MAGIC_NUM 0x15200101 1520 1.1 */
	unsigned int port_select; /* 0: digital, 1: analog */
};

typedef union {
	struct misc_cfg misc_cfg;
	ENGINE_CONFIG e_cfg;
} MISC_CFG;

typedef  struct _IOCTL_CONFIG_INFO
{
	MISC_CFG			cfg;

	CTRL_VIDEO	Ctrl4K;
	CTRL_VIDEO	Ctrl4K_low_frame_rate;
	CTRL_VIDEO      Ctrl1920;
#if (API_VER >= 2)
	CTRL_VIDEO      Ctrl1920_low_frame_rate;
#endif
	CTRL_VIDEO      Ctrl1280;
#if (API_VER >= 2)
	CTRL_VIDEO      Ctrl1280_low_frame_rate;
#endif
	CTRL_VIDEO      Ctrl800;
} IOCTL_CONFIG_INFO, *PIOCTL_CONFIG_INFO;

typedef  struct _IOCTL_REMOTE_INFO
{
	int CastingMode;//0:TCP-only unicasting; 1:unicasting; 2:multicasting; 3: UDP-only multicasting
	int 			CtrlSockFd;
	int 			DataSockFd;
	struct socket_info_t skinfo;
	int session_id;
} IOCTL_REMOTE_INFO, *PIOCTL_REMOTE_INFO;

#define  MISC_CFG_MAGIC_NUM (0x15200101)
#define  IOCTL_HOST_ENABLE_REMOTE    	0x1101
#define  IOCTL_HOST_ONLY_V1    	0x1103
#define  IOCTL_CLIENT_ONLY_V1   0x1104
#define  IOCTL_HOST_ONLY_V2    	0x1105
#define  IOCTL_CLIENT_ONLY_V2   0x1106
#define  IOCTL_HOST_ADD_CLIENT    	0x1107
#define  IOCTL_V2_DECODE_STREAM_READ   0x1131

#endif /* _VIDEOIP_IOCTL_H_ */


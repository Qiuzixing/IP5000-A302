/*
** Copyright (c) 2004-2014
** ASPEED Technology Inc. All Rights Reserved
** Proprietary and Confidential
**
** By using this code you acknowledge that you have signed and accepted
** the terms of the ASPEED SDK license agreement.
**
*/
#ifndef _VIDEO_HAL_H_
#define _VIDEO_HAL_H_

#ifndef API_VER
#define API_VER 2
#endif

#include <aspeed/hdcp1.h>
#include "hdmi_def.h"
#include "audio_def.h"

/* Copied from crt.h */
typedef struct _Audio_Info {
	unsigned char Audio_On;//useless to audio Tx side
	/*
	unsigned char SampleFrequency;
	#define B_FS_44100		  (0)  44.1kHz
	#define B_FS_NOTID		  (1)  non indicated
	#define B_FS_48000		  (2)  48kHz
	#define B_FS_32000		  (3)  32kHz
	#define B_FS_22050		  (4)  22.05kHz
	#define B_FS_24000		  (6)  24kHz
	#define B_FS_88200		  (8)  88.2kHz
	#define B_FS_768000 	  (9)  768kHz, HBR
	#define B_FS_96000		  (10) 96kHz
	#define B_FS_176400 	  (12) 176.4kHz
	#define B_FS_192000 	  (14) 192kHz
	*/
	unsigned char SampleFrequency;
	unsigned char ValidCh; //valid value are 2,4,6,8. non-HD NLPCM's value will be 2. Used to map to the number of I2S channel used.
	unsigned char SampleSize;
#if (API_VER >= 2)
	unsigned char io_in_use;
	unsigned char enable_upstream;
	unsigned char DownSampling;
	unsigned char Mode16bits;
	/*
	//BruceToDo. The define is different from ITE6604 driver.
	unsigned char Audio_Type
	#define B_CAP_AUDIO_ON  (1<<7)
	#define B_CAP_HBR_AUDIO (1<<6)
	#define B_CAP_DSD_AUDIO (1<<5)
	#define B_LAYOUT        (1<<4)
	#define B_MULTICH       (1<<4)
	#define B_HBR_BY_SPDIF  (1<<3)
	#define B_SPDIF         (1<<2)
	#define B_CAP_LPCM      (1<<0)
	*/
	unsigned char Audio_Type; //0x80 On, 0x00 and LPCM, 0x40 HBR, 0x10 NLPCM, 0x20 DSD
	unsigned char bSPDIF;
	/* caner add for fixing AudioInfoFrame issue */
	unsigned char AUD_DB[5]; //Audio infoframe PB1~PB5. (PB0 checksum not included)
	unsigned char ucIEC60958ChStat[5]; //channel status bytes
#endif
} Audio_Info;

typedef struct _INFO_FRAME_
{
	unsigned char number;//0 means data are invalid. Will be reset to 0 when mode changes.
	unsigned char data[31];
}
InfoFrame, *LpInfoFrame;

/* Copied from vesa.h */
typedef enum {
	PosPolarity =0,
	NegPolarity
}eVPolarity;

typedef enum {
	Interl =0, //interlace
	Prog =1, //progressive
	Unknown = 2 //Unknown. For analog M.D. only.
}eVScanMode;

typedef enum {
	s_RGB =0,
	s_DVI =1,
	s_HDMI_4x3 =2,
	s_HDMI_16x9 =3,
}eVSignalType;

#define COLOR_PIXEL_FORMAT_RGB        0
#define COLOR_PIXEL_FORMAT_YUV444     1
#define COLOR_PIXEL_FORMAT_YUV422     2
#define COLOR_PIXEL_FORMAT_YUV420     3
#define COLOR_COLORIMETRY_NA          0 /* N/A for RGB */
#define COLOR_COLORIMETRY_BT_601      0
#define COLOR_COLORIMETRY_BT_709      1
#define COLOR_QUANTIZATION_FULL       0
#define COLOR_QUANTIZATION_LIMITED    1
/*
** This color_mode_s structure will be used by VIDEO_MODE_INFO and CAPTURE_INFO.
** CAPTURE_INFO requires it to be exactly "unsigned char size".
*/
typedef struct {
	unsigned char pixel_format:3,
	              colorimetry:3,
	              quantization:1,
	              is_dual_edge:1;
} __attribute__ ((packed)) COLOR_MODE;

#define QUANTIZE_STR(s) \
({ \
	char *str; \
	switch(s) \
	{ \
	case COLOR_QUANTIZATION_FULL: \
		str = "Full"; \
		break; \
	case COLOR_QUANTIZATION_LIMITED: \
		str = "Limited"; \
		break; \
	default: \
		str = "Unknown?!"; \
		break; \
	} \
	str; \
})

#define COLOR_SPCE_STR(s) \
({ \
	char *str; \
	switch(s) \
	{ \
	case COLOR_COLORIMETRY_BT_601: \
		str = "BT601"; \
		break; \
	case COLOR_COLORIMETRY_BT_709: \
		str = "BT709"; \
		break; \
	default: \
		str = "Unknown?!"; \
		break; \
	} \
	str; \
})

#define PIXEL_FRMT_STR(s) \
({ \
	char *str; \
	switch(s) \
	{ \
	case COLOR_PIXEL_FORMAT_RGB: \
		str = "RGB"; \
		break; \
	case COLOR_PIXEL_FORMAT_YUV444: \
		str = "YUV444"; \
		break; \
	case COLOR_PIXEL_FORMAT_YUV422: \
		str = "YUV422"; \
		break; \
	case COLOR_PIXEL_FORMAT_YUV420: \
		str = "YUV420"; \
		break; \
	default: \
		str = "Unknown?!"; \
		break; \
	} \
	str; \
})

#define SIGNAL_TYPE_STR(s) \
({ \
	char *str; \
	switch(s) \
	{ \
		case s_RGB: \
			str = "RGB"; \
			break; \
		case s_DVI: \
			str = "DVI"; \
			break; \
		case s_HDMI_4x3: \
			str = "HDMI 4:3"; \
			break; \
		case s_HDMI_16x9: \
			str = "HDMI 16:9"; \
			break; \
		default: \
			str = "?"; \
			break; \
	} \
	str; \
})

#define IS_HDMI_SIGNAL(s) (((s) == s_HDMI_4x3) ||((s) == s_HDMI_16x9))

typedef struct _VIDEO_MODE_INFO
{
	//USHORT    X;
	//USHORT    Y;
	//USHORT    ColorDepth;
	unsigned int RefreshRate;
	unsigned int ModeIndex; //TimingTableIdx
	int PresetMatch;
	//int       isInterlace;

	//Added.
	unsigned int HActive;
	unsigned int VActive;
	unsigned int HTotal;
	unsigned int VTotal;
	unsigned int PCLK; //KHz
	unsigned int HFrontPorch;
	unsigned int HSyncWidth;
	unsigned int HBackPorch;
	unsigned int VFrontPorch;
	unsigned int VSyncWidth;
	unsigned int VBackPorch;
	eVScanMode ScanMode; //interlace or progressive
	eVPolarity VPolarity;
	eVPolarity HPolarity;
	/* SignalType decides whether to send AVInfoFrame.
	** The source can use TV timing without AVInfoFrame.
	** Client should use the same timing (polarity) to avoid "overscan" issue.
	*/
	eVSignalType SignalType;
	unsigned char ColorDepth;
#define HDCP_DISABLE	0
#define HDCP_V_1X	1
#define HDCP_V_20	2
#define HDCP_V_21	3
#define HDCP_V_22	4
#define HDCP_V_11_AND_22	5
	/* NOTE: EnHDCP is limited to 3bits size due to CAPTURE_INFO structure define. */
	unsigned char EnHDCP; /* False: Disable HDCP, True: See above HDCP version */
	unsigned int HStart;
	unsigned int VStart;
	COLOR_MODE src_color_mode; /* color mode from video source. */
} VIDEO_MODE_INFO, *PVIDEO_MODE_INFO;


#define HDCP_VER_STR(s) \
({ \
	char *str; \
	switch(s) \
	{ \
		case HDCP_DISABLE: \
			str = "Disable"; \
			break; \
		case HDCP_V_1X: \
			str = "1.x"; \
			break; \
		case HDCP_V_20: \
			str = "2.0"; \
			break; \
		case HDCP_V_21: \
			str = "2.1"; \
			break; \
		case HDCP_V_22: \
			str = "2.2"; \
			break; \
		default: \
			str = "?"; \
			break; \
	} \
	str; \
})


/*
** Start of declare for Host
*/
#ifdef CONFIG_ARCH_AST1500_HOST

typedef enum {
	VRXHAL_VENDORSPEC_INFOFRAME_TYPE = 0x01,
	VRXHAL_AVI_INFOFRAME_TYPE = 0x02,
	VRXHAL_SPD_INFOFRAME_TYPE = 0x03,
	VRXHAL_AUDIO_INFOFRAME_TYPE = 0x04,
	VRXHAL_MPEG_INFOFRAME_TYPE = 0x05,
	VRXHAL_HDR_INFOFRAME_TYPE = 0x07,
} e_vrxhal_infoframe_type;
#if 0
#define VENDORSPEC_INFOFRAME_TYPE 0x01
#define AVI_INFOFRAME_TYPE  0x02
#define SPD_INFOFRAME_TYPE 0x03
#define AUDIO_INFOFRAME_TYPE 0x04
#define MPEG_INFOFRAME_TYPE 0x05
#endif

#define KSV_FIFO_DEVICE_COUNT 8//equal to HDMIRX_MAX_KSV
#define KSV_FIFO_SIZE 40//KSV_LENGTH * KSV_FIFO_DEVICE_COUNT

typedef void (*PFN_UPDATE_AUDIO_INFO)(Audio_Info);
typedef void (*PFN_UPDATE_VIDEO_INFO)(unsigned int);
typedef void (*PFN_UPDATE_INFOFRAME)(e_vrxhal_infoframe_type);
typedef int (*PFN_VIDEO_PIXEL_CLK)(void);

void vrxhal_reg_audio_event_callback(PFN_UPDATE_AUDIO_INFO);
void vrxhal_dereg_audio_event_callback(void);
void vrxhal_reg_video_event_callback(PFN_UPDATE_VIDEO_INFO);
void vrxhal_dereg_video_event_callback(void);
void vrxhal_reg_infoframe_callback(PFN_UPDATE_INFOFRAME);
void vrxhal_dereg_infoframe_callback(void);
void vrxhal_reg_pixel_clk_callback(PFN_VIDEO_PIXEL_CLK pf);
void vrxhal_dereg_pixel_clk_callback(void);

int vrxhal_pixel_clk(void);
void vrxhal_sync_audio(int force_update);
int vrxhal_get_video_timing_info(void *pIn);
int vrxhal_get_video_avi_info(unsigned char *pData);
int vrxhal_get_video_vsd_info(unsigned char *pData);
int vrxhal_get_video_hdr_info(unsigned char *pData);
int vrxhal_is_dev_exist(void);
int vrxhal_get_video_hdmi_n_cts(unsigned int *pn, unsigned int *pcts);
int vrxhal_hdmi_hpd_ctrl(unsigned int level);

//extern void ResetAudio(void);
//int GetAudioInfoFrame(unsigned char *pData);

typedef enum {
	recver_chip__none = 0,
	recver_chip_cat6023,
	recver_chip_it6802,
} recver_chip;

typedef enum {
	recver_cap_none = 0,
	recver_cap_cec = 1,
} recver_cap;

typedef void (*pfn_sync_audio)(int force_update);
typedef int (*pfn_video_timing_info)(void *pIn);
typedef int (*pfn_video_avi_info)(unsigned char *pData);
typedef int (*pfn_video_vsd_info)(unsigned char *pData);
typedef int (*pfn_video_hdr_info)(unsigned char *pData);
typedef int (*pfn_dev_exist)(void);
typedef void (*pfn_video_update)(void);
typedef void (*pfn_audio_update)(void);
typedef int (*pfn_cec_send)(unsigned char *, unsigned int);
typedef int (*pfn_cec_topology)(unsigned char *);
typedef void (*pfn_HDCP1_set_upstream_port_mode)(unsigned char repeater);
typedef void (*pfn_HDCP1_set_upstream_port_auth_status)(unsigned int state, unsigned short Bstatus, void *KSV_list);
typedef int (*pfn_hdmi_n_cts)(unsigned int *pn, unsigned int *pcts);
typedef int (*pfn_hdmi_hpd_ctrl)(unsigned int level);

struct recver_info {
	recver_chip chip;
	recver_cap cap;

	pfn_dev_exist dev_exist;
	pfn_sync_audio sync_audio;
	pfn_video_timing_info video_timing_info;
	pfn_video_avi_info video_avi_info;
	pfn_video_vsd_info video_vsd_info;
	pfn_video_hdr_info video_hdr_info;
	pfn_video_update video_update;
	pfn_audio_update audio_update;
	pfn_cec_send cec_send;
	pfn_cec_topology cec_topology;
	pfn_HDCP1_set_upstream_port_mode hdcp_mode;
	pfn_HDCP1_set_upstream_port_auth_status hdcp_status;
	pfn_hdmi_n_cts hdmi_n_cts;
	pfn_hdmi_hpd_ctrl hdmi_hpd_ctrl;
};
#endif //#ifdef CONFIG_ARCH_AST1500_HOST
/*
** End of declare for Host
*/


/*
** Start of declare for Client
*/
#ifdef CONFIG_ARCH_AST1500_CLIENT
#endif //#ifdef CONFIG_ARCH_AST1500_CLIENT
/*
** End of declare for Client
*/

typedef void (*PFN_CECIF_RX)(unsigned char *, unsigned int);
void reg_cecif_rx_callback(PFN_CECIF_RX);
void dereg_cecif_rx_callback(void);
void cecif_rx(unsigned char *, unsigned int);
#endif //#ifndef _VIDEO_HAL_H_


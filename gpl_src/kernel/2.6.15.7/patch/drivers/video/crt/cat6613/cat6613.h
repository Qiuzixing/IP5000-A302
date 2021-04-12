#ifndef _CAT6613_HAL_H_
#define _CAT6613_HAL_H_

#include <aspeed/features.h>

#ifndef BOOL
#define BOOL	int
#endif

#ifndef BYTE
#define BYTE	unsigned char
#endif

#ifndef ULONG
#define ULONG	unsigned long
#endif

/*
 * HDMITX_SetOutput Capability
 */
#define CAP_HDMI	0x01
#define CAP_HDCP	0x02
#define CAP_AUDIO	0x04

// for sample clock
#define AUDFS_22p05KHz  4
#define AUDFS_44p1KHz 	0
#define AUDFS_88p2KHz 	8
#define AUDFS_176p4KHz  12
#define AUDFS_24KHz  	6
#define AUDFS_48KHz  	2
#define AUDFS_96KHz  	10
#define AUDFS_192KHz 	14
#define AUDFS_768KHz 	9
#define AUDFS_32KHz  	3
#define AUDFS_OTHER    	1
 
#ifndef _VESA_H_
typedef enum tagHDMI_Video_Type {
    HDMI_Unkown = 0 ,
    HDMI_640x480p60 = 1 ,
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
} HDMI_Video_Type ;
#endif

typedef enum tagHDMI_Aspec {
    HDMI_4x3 ,
    HDMI_16x9
} HDMI_Aspec;

typedef enum tagHDMI_OutputColorMode {
    HDMI_RGB444,
    HDMI_YUV444,
    HDMI_YUV422
} HDMI_OutputColorMode ;

typedef enum tagHDMI_Colorimetry {
    HDMI_ITU601,
    HDMI_ITU709
} HDMI_Colorimetry ;

typedef enum _TXVideo_State_Type {
    TXVSTATE_Unplug = 0,
    TXVSTATE_HPD,
    TXVSTATE_WaitForMode,
    TXVSTATE_WaitForVStable,
    TXVSTATE_VideoInit,
    TXVSTATE_VideoSetup,
    TXVSTATE_VideoOn,
    TXVSTATE_Reserved
} TXVideo_State_Type ;


typedef enum _TXAudio_State_Type {
    TXASTATE_AudioOff = 0,
    TXASTATE_AudioPrepare,
    TXASTATE_AudioOn,
    TXASTATE_AudioFIFOFail,
    TXASTATE_Reserved
} TXAudio_State_Type ;

/*
 * export functions
 */
unsigned int cat6613_exist(struct s_crt_drv *crt);
unsigned int cat6613_rd_edid(struct s_crt_drv *crt, unsigned int blk_num, unsigned char *pEdid);
unsigned int cat6613_rd_edid_chksum(struct s_crt_drv *crt, unsigned char *pChksum);
#if 0
unsigned int xmiter_init_cat6613(struct s_crt_drv *crt);
#endif
unsigned int cat6613_disable(struct s_crt_drv *crt);
unsigned int cat6613_setup_video(struct s_crt_drv *crt, struct s_crt_info *info);
unsigned int cat6613_setup_audio(Audio_Info *audio_info);
void cat6613_disable_audio(void);
unsigned int cat6613_check_hotplug(struct s_crt_drv *crt);
#ifdef CONFIG_ARCH_AST1500_HOST
typedef enum {
    PCLK_LOW = 0 ,
    PCLK_MEDIUM,
    PCLK_HIGH
} VIDEOPCLKLEVEL ;
void cat6613_setup_loopback_video(VIDEOPCLKLEVEL level, unsigned char bHDMI, unsigned char *pAVI_InfoFrame, unsigned char *pHDMI_InfoFrame);
#endif

#if SUPPORT_HDCP_REPEATER
void CAT6613_HDCP1_Downstream_Port_Set_Mode(unsigned char repeater);
void CAT6613_HDCP1_Downstream_Port_Auth(int enable);
void CAT6613_HDCP1_Downstream_Port_Encrypt(unsigned char enable);
#endif

#endif // _CAT6613_HAL_H_


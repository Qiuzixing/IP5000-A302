#ifdef LinuxAP
/*
 * HDMITX_SetOutput Capability
 */
#define CAP_HDMI	0x01
#define CAP_HDCP	0x02
#define CAP_AUDIO	0x04

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
#endif	/* LinuxAP */

/* IOCTL */
#define  IOCTL_CAT6613_IO_READ    0x1103
#define  IOCTL_CAT6613_IO_WRITE   0x1104
#define  IOCTL_CAT6613_GETEDID	  0x1105
#define  IOCTL_CAT6613_SETOUTPUT  0x1106

typedef struct _IO_CAT6613_DATA {
    unsigned char Bank;
    unsigned char Index;
    unsigned char Data;
} IO_CAT6613_DATA, *PIO_CAT6613_DATA;

typedef struct _EDID_DATA {
    unsigned char Blk;
    unsigned char Data[128];
} EDID_DATA, *PEDID_DATA;

typedef struct _OUTPUT_OPTION {
    unsigned char bInputColorMode;
    unsigned char bOutputColorMode;                      
    unsigned char OutputVideoTiming;
    unsigned long VCLK;
    unsigned char bAudioSampleFreq;
    unsigned char ChannelNumber;
    unsigned char bAudSWL;
    unsigned char bSPDIF;
    unsigned char  bCapibility;	
    unsigned char  ColorDepth;	
} OUTPUT_OPTION, *POUTPUT_OPTION;

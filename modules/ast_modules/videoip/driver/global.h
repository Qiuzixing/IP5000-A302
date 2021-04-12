/*
 * Copyright (c) 2004-2012
 * ASPEED Technology Inc. All Rights Reserved
 * Proprietary and Confidential
 *
 * By using this code you acknowledge that you have signed and accepted
 * the terms of the ASPEED SDK license agreement.
 */


#ifndef _GLOBAL_H_
#define _GLOBAL_H_

#include <aspeed/features.h>

#include <linux/types.h>
#include <linux/list.h>
#include <linux/timer.h>
#include <linux/wait.h>
#include <linux/spinlock.h>
#include <linux/completion.h>
#include <aspeed/hdcp1.h>
#include <asm/arch/drivers/crt.h>
#include <asm/arch/drivers/video_hal.h>

#include "videoip_ioctl.h"
#include "vesa.h"
#include "hdcp2_sm.h"

//#define     DyNAMIC_QUALITY_EN
//#define     DETECT_PCLK
//#define ADJUST_DISP
//#define ENABLE_LOG
//#define ENCODER_COUNT   //calculate encoder performance
//#define BITRATE_COUNT   //calculate transmit performance
//#define LOCAL_MONITOR_DISABLE
#define GW_SECRET 1
#define IOV_THREE_ENABLE
#define ENCODE_PERFORMACE 1
#define PROFILE_ENABLE 1
#define PROFILE_DUMP_FRAME  (PROFILE_ENABLE && 0)
#define PROFILE_INTERVAL  2000 // in msec
#define PROFILE_FRAMERATE (PROFILE_ENABLE && 1)
#define PROFILE_BITRATE (PROFILE_ENABLE && 1)
//#define DISABLE_CEA_BLOCK   //disable EDID for 128 to 255
//ASUS EAH4850: Disable Monitor hot-plug, the vga will read i2c and cause writing the eeprom fail.
//solution: moving hot-plug after writing eeprom
#define AMD_WORK_AROUND1
 //get the most resolution of monitor.
 //If mode that get from mode detection is large than this, downscale it.
#define MONITOR_MAX_RES
//#define DEL_INTERLACE_MODE  //1920X1080i change to 720P
//#define TEST_EDID_CLIENT //assign one of EDID to test
#define INTERLACE_SUPPORT
//#define HDMI_1920_SHRINK
#define VIDEO_POLLING 0
#define VE_ENCODE_SW_WATCHDOG 1
#define VE_DECODE_SW_WATCHDOG 1
#define SYNC_WITH_VSYNC 1
#if (CONFIG_AST1500_SOC_VER == 2)
	#define DELAY_DEQUEUE 0
	#define VIDEO_WALL 0
	#define	STREAM_ERROR_DBG	0
#else
	#define DELAY_DEQUEUE 1
	#define VIDEO_WALL 1
	#define	STREAM_ERROR_DBG	0
#endif
#define DBG_GPIO 0

#define DBG_STREAM_ERR 0

#define HOST_VE_START_NO_REAL_CLIENT	1
#define VIDEO_CM_ONLY_ONE_REAL_ENTITY
#define DBG_VE_THREAD	1

#if DBG_GPIO
#include <asm/arch/gpio.h>
extern unsigned int gFE; //Frame End
extern unsigned int gPS; //Pkt start
extern unsigned int gPE; //Pkt end
extern unsigned int gP4;
#define TOGGLE_GPIO1() \
    do { \
        gFE ^= 1; \
        gpio_direction_output(AST1500_GPH0, gFE); \
    } while (0)

#define TOGGLE_GPIO2() \
    do { \
        gPS ^= 1; \
        gpio_direction_output(AST1500_GPC6, gPS); \
    } while (0)

#define TOGGLE_GPIO3() \
    do { \
        gPE ^= 1; \
        gpio_direction_output(AST1500_GPC7, gPE); \
    } while (0)

#define TOGGLE_GPIO4() \
    do { \
        gP4 ^= 1; \
        gpio_direction_output(AST1500_GPH1, gP4); \
    } while (0)

#define TOGGLE_GPIO1_ON() \
    do { \
        gFE = 1; \
        gpio_direction_output(AST1500_GPH0, gFE); \
    } while (0)

#define TOGGLE_GPIO1_OFF() \
    do { \
        gFE = 0; \
        gpio_direction_output(AST1500_GPH0, gFE); \
    } while (0)

#define TOGGLE_GPIO2_ON() \
    do { \
        gPS = 1; \
        gpio_direction_output(AST1500_GPC6, gPS); \
    } while (0)

#define TOGGLE_GPIO2_OFF() \
    do { \
        gPS = 0; \
        gpio_direction_output(AST1500_GPC6, gPS); \
    } while (0)

#define TOGGLE_GPIO3_ON() \
    do { \
        gPE = 1; \
        gpio_direction_output(AST1500_GPC7, gPE); \
    } while (0)

#define TOGGLE_GPIO3_OFF() \
    do { \
        gPE = 0; \
        gpio_direction_output(AST1500_GPC7, gPE); \
    } while (0)

#define TOGGLE_GPIO4_ON() \
    do { \
        gP4 = 1; \
        gpio_direction_output(AST1500_GPH1, gP4); \
    } while (0)

#define TOGGLE_GPIO4_OFF() \
    do { \
        gP4 = 0; \
        gpio_direction_output(AST1500_GPH1, gP4); \
    } while (0)

#else
#if 0//client dequeue debug
#include <asm/arch/gpio.h>
extern unsigned int gVI;
extern unsigned int gKF;
#define TOGGLE_GPIO_VI() \
    do { \
        gVI ^= 1; \
        gpio_direction_output(AST1500_GPH0, gVI); \
    } while (0)

#define TOGGLE_GPIO_KF() \
    do { \
        gKF ^= 1; \
        gpio_direction_output(AST1500_GPH1, gKF); \
    } while (0)
#endif
#define TOGGLE_GPIO1() do {} while (0)
#define TOGGLE_GPIO2() do {} while (0)
#define TOGGLE_GPIO3() do {} while (0)
#define TOGGLE_GPIO4() do {} while (0)
#define TOGGLE_GPIO1_ON()  do {} while (0)
#define TOGGLE_GPIO1_OFF()  do {} while (0)
#define TOGGLE_GPIO2_ON()  do {} while (0)
#define TOGGLE_GPIO2_OFF()  do {} while (0)
#define TOGGLE_GPIO3_ON()  do {} while (0)
#define TOGGLE_GPIO3_OFF()  do {} while (0)
#define TOGGLE_GPIO4_ON()  do {} while (0)
#define TOGGLE_GPIO4_OFF()  do {} while (0)
#endif

/* Bruce160408. When calculate ring buf, To == From should treat as size 0. */
#define ABS_RING_BUF(ToValue, FromValue, TotalSize)	\
		((ToValue >= FromValue)?(ToValue - FromValue):(TotalSize - FromValue + ToValue))
#define ADD_RING_PT(CurPt, Delta, RingSize) \
        (((CurPt) + (Delta)) & ((RingSize) - 1))


#ifdef MAIN_GLOBALS
#define MAIN_EXT
#else
#define MAIN_EXT extern
#endif

#if (3 <= CONFIG_AST1500_SOC_VER)
#define QUALITY_AUTO_YUV420_CHECK
#endif

/* First definition in videoip_main.c */
MAIN_EXT unsigned char *g_pVLinearAddr;
MAIN_EXT unsigned char *g_pVLinearAddr_c; /* cacheable */
MAIN_EXT struct s_crt_drv *crt;

typedef struct _PT_NODE {
	union {
		u32	PtCmd;
		u32	timeStamp;
	};
	u32 ulPt;
	u32 size;
	u32 quality_mode;
} PT_NODE;

#define	MAX_PT_BUF	(1<<10)

MAIN_EXT PT_NODE			V1PtBuf[MAX_PT_BUF];
MAIN_EXT u32 			V1PtHead,   V1PtTail;
MAIN_EXT wait_queue_head_t  V1PtWq;
MAIN_EXT spinlock_t   		V1PtLock;

#define   	V1BUF_HEAD 		(V1PtBuf[V1PtHead])
#define   	V1BUF_TAIL 		(V1PtBuf[V1PtTail])

#define		INCBUF(x,mod) 	(((x) + 1) & ((mod) - 1))

#define is_PtBuf_empty() (V1PtHead == V1PtTail)

static inline int is_PtBuf_full(void)
{
	return (INCBUF(V1PtHead, MAX_PT_BUF) == V1PtTail);
}


#ifdef MAIN_GLOBALS
	#ifdef CONFIG_VIDEO_DEBUG
		//unsigned long videoip_debug_flag = 0xffffffff;
		unsigned long videoip_debug_flag = 0xfffffffd;
	#else
		unsigned long videoip_debug_flag = 0;
	#endif

#else
	extern unsigned long videoip_debug_flag;
#endif

enum {
	Pt_V1PacketReady	= 0,
	Pt_V1FrameComplete,
	HOST_SET_INFO,	//use data socket to inform host info
	CLIENT_SET_INFO,	//use cmd socket to inform client info
	CLIENT_READY,
	CLIENT_DUMP_STREAM_BUFFER,
	CLIENT_REQUEST_FULL_FRAME,
	CLIENT_FIX_QUALITY_MODE,
	CLIENT_ANTI_DITHER,
	CLIENT_REQUEST_FULL_FRAME_ACK,
#if SUPPORT_HDCP_REPEATER
	CLIENT_HDCP_MESSAGE,
#endif
	CLIENT_REQUEST_FULL_JPEG,
};

struct videoip_task {
	struct task_struct *thread;
	struct completion thread_done;
	char *name;
	void (*loop_ops)(struct videoip_task *);
#if (DBG_VE_THREAD == 1)
	atomic_t ref_count;
#endif
};

typedef enum
{
    BUF_EMPTY = 0,
    BUF_SEND,
} eBUF_TYPE;

typedef struct sRECT
{
    u16    Width;
    u16    Height;
} RECT;

//host mode detection events
#define MODE_DETECT_START	1
#define MODE_DETECT_DONE	2
#define MODE_DETECT_STOP	3

#define PWR_SAVE_COUNTDOWN    15000 //in msec

typedef struct _DISPLAY_INFO
{
	int valid;//used by host in multicast mode
	int primary_EDID;//whether the following EDID is the primary EDID or not
	u8 EDID[EDID_ROM_SIZE];//this EDID may not be the same as the EDID in EEPROM
} DISPLAY_INFO, *PDISPLAY_INFO;

typedef struct _CAPTURE_INFO
{
	RECT        CapRect; //capture window. The native video source
	RECT        ComRect; //default compress window. The compress window size which could be down-scaled from video source.
	RECT        CurRect; // current compress window. May be different from "Compress Window" when dynmamic scaling.
	RECT        CRTRect; // resolution to be used on display CRT.

	unsigned char RefreshRate; //video source refresh rate.
	unsigned char ColorDepth;
	unsigned char inputSelect:4, //For host only?
	              HDMI_Mode:4;
	COLOR_MODE src_color_mode; //Mapping to video_hal.h::COLOR_MODE

	union {
		unsigned short idx;
		unsigned short sn;
	} __attribute__ ((packed)) timing;

	// unsigned short
	unsigned short isInterlace:1, // 1: interlace 0:progressive
	               PresetMatch:1,
	               EnHDCP:3, /* False: Disable HDCP, True: See video_hal.h HDCP_DISABLE defines */
	               reserved2:11;
}  __attribute__ ((packed)) CAPTURE_INFO, *PCAPTURE_INFO;

typedef struct _QUALITY_CONFIG
{
	CTRL_VIDEO    CrtVideo;
} QUALITY_CONFIG, *PQUALITY_CONFIG;

typedef struct _GLOBAL_CONFIG
{
	QUALITY_CONFIG q_cfg;
	CAPTURE_INFO cap_info;
} GLOBAL_CONFIG, *PGLOBAL_CONFIG;


typedef struct _CLIENT_CONTEXT
{
	unsigned int	active;
	struct list_head	list;
	struct socket *CtrlSocket; //Also used as an id of this client context.
	//struct socket *tcp_V1Socket;
	//struct socket *tcp_V2Socket;
	//struct socket *tcp_VMSocket;

	struct videoip_task client_cmd_task;
	DISPLAY_INFO disp_info;
	unsigned long ip; //IpV4 4 bytes ip format
#if SUPPORT_HDCP_REPEATER
	//HDCP task relevant
	struct videoip_task hdcp2_task;
	wait_queue_head_t	hdcp2_task_waitQ;
	HDCP2_Repeater_Downstream_Port_Context	Hdcp2_context;
#endif
#if defined(VIDEO_CM_ONLY_ONE_REAL_ENTITY)
	atomic_t users;
#endif
} CLIENT_CONTEXT, *PCLIENT_CONTEXT;

typedef void (*ve_worker_func)(void *pdata);

typedef struct _event_struct
{
	struct list_head	list;
	ve_worker_func worker;
	void *pdata;
	unsigned int free_pdata;
	struct work_struct work;
	u32 session_id;
}event_struct, *pevent_struct;

/* Quality Factors. Used to configure different quality mode */
typedef struct _q_factor
{
#if (CONFIG_AST1500_SOC_VER >= 2)
	u32 bcd_ctrl_reg;
	u32 bcd_ctrl_reg_mask;
#endif
	u32 compress_ctrl_reg;
	u32 compress_ctrl_reg_mask;
	u32 quanti_value_reg;
	u32 compress_win_reg;
	u32 CRT_HorScale;
	u32 CRT_VerScale;
	unsigned int HorScale; //Just copied from q_cfg
	unsigned int VerScale;//Just copied from q_cfg
	unsigned int maxFrameRate;
	unsigned int upQualityScoreThreshold;
	unsigned int VE_FrameInterval;
	unsigned int earlyThreshold;
	unsigned int delayThreshold;
	unsigned int abnormalThreshold;
	unsigned int textFrameLimit;
} Q_FACTOR, *PQ_FACTOR;

#define QCHG_IDLE 0
#define QCHG_START 1
#define QCHG_STOPPING_VE 2
#define QCHG_CAP_IDLE 3 //Capture engine is idle
#define QCHG_COM_IDLE 4 //Compress engine is idle


typedef struct  _Q_PARAM
{
	unsigned char	NormalLumaTableSelection;
	unsigned char	NormalChromaTableSelection;
	unsigned char	HighDeValueOrHighLumaJpeg;
	unsigned char	BestDeValueOrHighChromaJpeg;

	unsigned char	FrameRateControl:4; // 4-bits
	unsigned char	TruncatedBits:4; // 4-bits
	unsigned char	HorScale:4; // 4-bits
	unsigned char	VerScale:4; // 4-bits
	unsigned char	BCDThreshold:4; // 4-bits
	unsigned char	ABCDThreshold:4; //4-bits
	unsigned char Reserved1;

	unsigned int	HighQualityEnable:1; //bool
	unsigned int	BestQualityEnable:1; //bool
	unsigned int	CodecMode:1; //bool
	unsigned int	YUV420:1; //bool
	unsigned int	JpegOnlyMode:1; //bool
	unsigned int	VQ4ColorMode:1; //bool
	unsigned int	EnableBCD:1; //bool
	unsigned int	EnableABCD:1; //bool
	unsigned int	Pass2BCDDelay:1; //bool
	unsigned int	Pass3BCDDelay:1; //bool
	unsigned int	EnableDithering:1; //bool
	unsigned int	Reserved2:21;
} __attribute__ ((packed)) Q_PARAM, *PQ_PARAM;


typedef struct _PACKED_Q_CFG
{
	unsigned char		QualityNum;//the number of qualities in this profile
	unsigned char		DefaultQualityMode;
	unsigned short	DynamicQualityEnable:1;
	unsigned short       Reserved1:15;

	unsigned int		StreamBufPktSize;
	unsigned int		StreamBufPktNum;
	unsigned int	VideoBitRateLimit;
	Q_PARAM		QualityParam[MAX_QUALITY_NUM];
} __attribute__ ((packed)) QCFG, *PQCFG;

typedef struct PACKET_INFO
{
	/* Start of info header */
	unsigned int magic; //VIDEO_MAGIC Use this value to differenciate differenct header versions.
	unsigned int from; //from which host
	unsigned int framSeq;
	/* Start of packet info */
	unsigned int ulData; //data size
	unsigned int ulCmd; //data type: Pt_V1FrameComplete, ....
	unsigned int time_stamp;

	/* Start of config info */
	unsigned int cfg_sn; //config serial number
	unsigned char quality_mode;
	unsigned char Reserved1;
	unsigned short frame_rate;
	CAPTURE_INFO cap_info;
	QCFG s_q_cfg;
	InfoFrame	AVI_Info;
	InfoFrame	HDMI_Info;
	InfoFrame	HDR_Info; /* HDMI Type 0x07 Dynamic Range and Mastering Info Frame. */
	/* It is 256 - 16 bytes in total. Can't add anymore data unless increase HW UDP's hear size. */
}  __attribute__ ((packed)) PacketInfo, *LpPacketInfo;

#define PKT_INFO_SIZE (sizeof(PacketInfo))

typedef struct CTRL_PACKET_INFO
{
	/* Start of info header */
	unsigned int magic; //0x564F4950 VOIP
	unsigned int from; //from which host
	unsigned int framSeq;
	/* Start of packet info */
	unsigned int ulData; //data size
	unsigned int ulCmd; //data type: Pt_V1FrameComplete, ....
} CtrlPacketInfo, *LpCtrlPacketInfo;

#define CTRL_PKT_INFO_SIZE (sizeof(CtrlPacketInfo))

#if (CONFIG_AST1500_SOC_VER >= 3)
#define NO_FUNC_DECLAR
#include "desc_mode.h"
#undef NO_FUNC_DECLAR
#endif

#if (CONFIG_AST1500_SOC_VER >= 3)
#define SW_FLIP_WHEN_REFRESH_RATE_UNMATCH_CASE_2 0
#define DOUBLE_BUF_WHEN_REFRESH_RATE_UNMATCH_CASE_1 1
#endif

#define AST_VIDEO_KEY_LEN_DW_MAX 64 /* 64 doulbe words, 256 bytes */
#define AST_VIDEO_KEY_LEN_DW_AES 44 /* 44 doulbe words, AES-128bit only need 176 bytes */

#define AST_VIDEO_MODE_HOST	0
#define AST_VIDEO_MODE_CLIENT	1

#define AST_VIDEO_ENCRYPT_NONE	0
#define AST_VIDEO_ENCRYPT_RC4	1
#define AST_VIDEO_ENCRYPT_AES	2

typedef struct _VIDEOIP_DEVICE
{
	int mode;//0:host 1:client
	int encrypt;
	int magic;
#define INPUT_SELECT_DIGITAL	0
#define INPUT_SELECT_ANALOG     1
//use the same field to store client output select
//moved to cap_info	int inputSelect;
	//BruceToDo. 101126. Define CastingMode
	int CastingMode;//0:TCP-only unicasting; 1:unicasting; 2:multicasting; 3: UDP-only multicasting
	int isLoopbackEdidPrimary;
	int SourceModeStable;
//	int loopbackInitialized;
	int disableLoopback;
#define REMOTE_STATE_OFF 0
#define REMOTE_STATE_ON  1
#define REMOTE_STATE_DISABLING 2
	int RemoteState;
	int loopbackEnabled;
	//misc. flags
	int drvExit;
	int		dataAborted;
#if (CONFIG_AST1500_SOC_VER >= 3)
	#define VE_IS_DEAD_RESET_ONLY 2
#endif
	int ve_is_dead;
	int is_data_thread_busy;
	u32 skip_frame; // 1: skip current frame for UDP
#if (3 <= CONFIG_AST1500_SOC_VER)
	#define FULL_FRAME_NO_NEED 0
	#define FULL_FRAME_NEED_NORMAL 1
	#define FULL_FRAME_NEED_HIGH  2
	unsigned int full_frame_need;
#endif
#define ERR_MISMATCH_SEQ (1 << 0)
#define ERR_SOCK_SEND (1 << 1)
#define ERR_SOCK_RECV (1 << 2)
#define ERR_UNKNOWN_CMD (1 << 3)
#define ERR_UNKNOWN_EVENT (1 << 4)
#define ERR_KMALLOC (1 << 5)
	unsigned int errReported;
	//int		dataBusy;
#if DBG_STREAM_ERR
	unsigned int StreamErrorTest;
#endif
	unsigned int CRT_HorScale;
	unsigned int CRT_VerScale;
	int	horScale;
	int	verScale;
	//int wait_frame;
	int dest_buffer_to_use;
	u32 dest_buffer_offset;
	u32 intEnable;
	//southstar: the most supported res of the monitor from edid
	u16          usMaxWidth;
	u16          usMaxHeigth;

	unsigned int data_socket_busy;
	struct socket *CtrlSocket;
	struct socket *DataSocket;
#define MAX_MC_BUF_SIZE (64*1024)
#define PKT_CHUNK_SIZE (63*1024) //bytes. Sending over 64K bytes will fail the UDP stack.

	u32 lastPt; //Used to split big packet into small packets

	struct list_head client_list;

	struct videoip_task tcp_V1;
	//struct videoip_task tcp_Cmd;
	struct workqueue_struct *event_work_queue;
	struct list_head	event_list;
	spinlock_t	event_list_lock;
	struct semaphore event_lock;

#if SUPPORT_HDCP_REPEATER
#define HDCP_MODE_LEGACY	0//HDCP repeaters and HDCP2 are disabled.
#define HDCP_MODE_STANDARD	1//HDCP repeaters and HDCP2 are enabled in accordance with HDCP specifications.
#define HDCP_MODE_FAST		2//HDCP repeaters and HDCP2 are enabled with some simplifications.

	unsigned int	Hdcp_mode;
#define HDCP_MODE_EX_MASK 0xFFFF0000
#define HDCP_MODE_EX_RSA_KEY_PAIR_UNAVAILABLE (1 << 16) //disable when an RSA key pair is available.
#define HDCP_MODE_EX_HDCP_NO_RETRY            (1 << 17) //don't retry on HDCP fail. For legacy mode only.
#define HDCP_MODE_EX_HDCP_ALWAYS_ON           (1 << 18) //HDCP 1.x always ON. For legacy mode only.
#define HDCP_MODE_EX_HDCP_ALWAYS_OFF          (1 << 19) //HDCP always OFF. For legacy mode only.
#define HDCP_MODE_EX_HDCP_ALWAYS_ON_22        (1 << 20) //HDCP 2.2 always ON. For legacy mode only.
#define HDCP_MODE_EX_DOWN_CNVRT_IS_ON         (1 << 21) //HDCP down convert mode STATUS is ON. For legacy mode only.
	unsigned int	Hdcp_mode_ex;
	unsigned int	Hdcp2_version;

#define DRVO_H_2160P_TO_1080P                 (1 << 0)  //Host: Convert 4K to 1080p
#define DRVO_H_2160P_YUV420_TO_1080P          (1 << 1)  //Host: Convert 4K 60Hz YUV420 to 1080p. (default convert to 4K 30Hz)
#define DRVO_H_MULTICAST_STOP_TX_NO_CLIENT    (1 << 2)  /* Host: Stop sending multicast packet out when no client attached */
#define DRVO_C_FAST_SWITCH_NO_DISABLE_DESKTOP (1 << 16) /* Client: Do not disable desktop while fast-switch */
#define DRVO_C_TIMING_CHECK_STRICT            (1 << 17) /* Client: Do strict check for video timing */
	unsigned int drv_option;

#define SP_H_2160P_TO_1080P                   (1 << 0)  //Host: Do host convert 4K to 1080p
#define SP_H_HALF_FRAME_RATE                  (1 << 1)  //Host: Force half frame rate. For 4K60Hz to 4K30Hz.
	unsigned int patch; /* special code patch. */

#if defined(CONFIG_ARCH_AST1500_HOST)
	//upstream HDCP 1.x
	HDCP1_Repeater_Upstream_Port_Context	Hdcp1_repeater_upstream_port_context;
	//downstream HDCP 1.x
	HDCP1_Repeater_Downstream_Port_Context	Hdcp1_repeater_downstream_port_context;
	//downstream HDCP 2.x
	unsigned char	lc_128[HDCP2_GLOBAL_CONSTANT_LENGTH];
	DCP_RSA_Public_Key	kpub_dcp;//3072-bit RSA public key of DCP LLC
	unsigned char enable_HDCP2_encryption;
	//k_s and r_iv are shared among clients.
	unsigned char	k_s[HDCP2_SESSION_KEY_LENGTH];
	unsigned char	r_iv[HDCP2_IV_LENGTH];
	unsigned char inputCtr[8];
#endif
#if defined(CONFIG_ARCH_AST1500_CLIENT)
	//HDCP task relevant
	struct videoip_task	Cmd_network_task;
	struct videoip_task	hdcp2_task;
	wait_queue_head_t	hdcp2_task_waitQ;
	HDCP2_Repeater_Upstream_Port_Context	Hdcp2_repeater_upstream_port_context;
	//downstream HDCP 1.x
	HDCP1_Repeater_Downstream_Port_Context	Hdcp1_repeater_downstream_port_context;
/* output format for single edge source output type. 1080p */
#define OF_SE_YUV          (0)
#define OF_SE_RGB          (1)
#define OF_SE_PASSTHROUGH  (2)
/* output format for dual edge source output type. 2160p, 2K. */
#define OF_DUAL_EDGE_SHIFT (2)
#define OF_DE_YUV          (0UL << OF_DUAL_EDGE_SHIFT)
#define OF_DE_RGB          (1UL << OF_DUAL_EDGE_SHIFT)
#define OF_DE_PASSTHROUGH  (2UL << OF_DUAL_EDGE_SHIFT)
/* Macros for OF_XXX */
#define OF_YUV             (0)
#define OF_RGB             (1)
#define OF_PASSTHROUGH     (2)
#define OF_MASK            (0x3)
#define OF_GET(is_de, cfg) ((is_de)?(((cfg) >> OF_DUAL_EDGE_SHIFT) & OF_MASK):((cfg) & OF_MASK))
	u32 hdmi_force_rgb_output;
	u32 hdmi_hdr_mode_off;
#endif
#endif
#if defined(CONFIG_ARCH_AST1500_HOST)
	spinlock_t	modeDetect_event_list_lock;
	u32 analog_edge_detect_threshold; //MODEDETECTION_EDGE_PIXEL_THRES_ANALOGE
	unsigned int input_to_select;
	unsigned int has_dual_input;
	unsigned int life_cycle;
#endif
	u32				V1LastFrameEndPt;
	u32				V1FrameSize;
#define IFF_IDLE                        (0)
#define IFF_NORMAL_PRIORITY (1)
#define IFF_HIGH_PRIORITY      (2)
	unsigned int insertFullFrame; //0:stop/idle, 1:do for normal case, 2:do it right away.
#define	ANALOG_QUALITY_MODE_LIMIT	1
#define	QUALITY_MODE_AUTO	-1
#define	QUALITY_MODE_TEXT	0
#define	QUALITY_MODE_HIGH	1
#define	QUALITY_MODE_MED	2
#define	QUALITY_MODE_LOW	3
	int		QualityModeSet; // QUALITY_MODE_AUTO means use the setting from .inf.
	unsigned int		QualityModetoUse;
	unsigned int		QualityModeinUse;
	unsigned int	maxFrameRate;
	unsigned int	FrameInterval; /* in ms */
	unsigned int	maxFrameInterval; /* in ms */
	unsigned int	kick_wd_threshold;
	unsigned int	kick_wd_counter;
	unsigned int	VE_FrameInterval;//in timestamp uint
	unsigned int	CRT_FrameInterval;//in timestamp uint
	unsigned int	earlyThreshold;//in timestamp uint
	unsigned int	delayThreshold;//in timestamp uint
	unsigned int	abnormalThreshold;//in timestamp uint
	unsigned int	upQualityScoreThreshold;
	unsigned int	lastTimeStamp;
	unsigned int	currentTimeStamp;
#if (CONFIG_AST1500_SOC_VER >= 3)
#define	NULL_FRAME_SIZE_DESC_MODE 448
#define VE_LIMIT_FRAME_SIZE_DESC_MODE 3000000 /* Max frame size VE can handle. */
#endif
#define	NULL_FRAME_SIZE	128
	unsigned int	contNullFrames;
	unsigned int	textFrameLimit;
	unsigned int	contTextFrames;
//	unsigned int	contFrames;
//	unsigned int	delayedFrames;
	unsigned int	upQualityScore;
	unsigned int	downQualityScore;
	unsigned int	peakFrameSize;
	unsigned int	boundaryFrameSize;
	unsigned int	boundaryFactor;
	unsigned int	FrameCnt;
#if (CONFIG_AST1500_SOC_VER >= 2)
	unsigned int	HwFrameCnt;
#endif
	unsigned int	downFrameSize;
	unsigned int	upFrameSize;
	int				falseP;
	unsigned int    FakePScore;
#if (API_VER < 2)
	Q_FACTOR q_factors[QUALITY_LEVEL_NUM];
#else
	Q_FACTOR q_factors[MAX_QUALITY_NUM];
#endif
	unsigned int QChgState; //Used for host only.

	unsigned int	lastDequeueTimeStamp;

	u32               ulLastTimeStamp;
	unsigned int kick_field;//0:odd;1:even

	CTRL_VIDEO	Ctrl4K;
	CTRL_VIDEO	Ctrl4K_low_frame_rate;
	CTRL_VIDEO	Ctrl1920;
#if (API_VER >= 2)
	CTRL_VIDEO	Ctrl1920_low_frame_rate;
#endif
	CTRL_VIDEO	Ctrl1280;
#if (API_VER >= 2)
	CTRL_VIDEO	Ctrl1280_low_frame_rate;
#endif
	CTRL_VIDEO	Ctrl800;

	u32               ulLastNetTime;
	unsigned int               ulTotalRecvData;
	u32 	ulV1LastHWWritePt; 	/* HandleV1TxHost */
	u32	ulV1OldSWWritePt;		/* HandleV1RxClient */

#if STREAM_ERROR_DBG
#define FE_SLOT_SIZE 16 //Must be 2^n
	u32 	frameEnds[FE_SLOT_SIZE];//used to record 16 frame ends if useful
#endif

	u32	nSeqNum;
#define HOST_PACKET_SIZE 128
#define HOST_PACKET_NUM 8
#define CLIENT_PACKET_SIZE 128
#if (CONFIG_AST1500_SOC_VER == 1)
#define CLIENT_PACKET_NUM 32
#else
//Bruce140318. Reduce client vbuf size from 64 to 32. Reserved for snapshot.
#define CLIENT_PACKET_NUM 32 //64
#endif
	unsigned long		pktSize;
	unsigned long		pktNum;
	unsigned long		buffSize;
//common states
#define STATE_INITIALIZING 0x0//start state
#define STATE_OPERATING 0x1
//#define STATE_RESETING 0x2
//host-specific states
//#define HOST_STATE_READING_EDID 0x11
//#define HOST_STATE_WAITING_CLIENT_INFO 0x12
#define HOST_STATE_DETECTING_MODE 0x13
//#define HOST_STATE_WAITING_CLIENT_READY 0x15
#define HOST_STATE_SUSPENDING 0x16
//client-specific states
//#define CLIENT_STATE_READING_EDID 0x22
#define CLIENT_STATE_WAITING_HOST_MODE 0x23
//#define CLIENT_STATE_WAITING_HOST_RESCALE_READY 0x26
//#define CLIENT_STATE_WAITING_HOST_QUALITY_MODE_READY 0x27
#define CLIENT_STATE_SUSPENDING 0x28 //This is a state data stream is stopped. VE doesn't own the crt at this state. VE could be either ready or not.
	unsigned int		opState;
	unsigned int		led_state; /* 1: led on */

	struct timer_list	watchdog_timer;
	struct timer_list	pwr_save_timer;
#if (CONFIG_AST1500_SOC_VER >= 3)
	struct timer_list	full_jpeg_mode_timer;
#endif
	unsigned int user_frame_rate_control;
	unsigned int current_bit_rate_control; //in Mbps
#define MODE_DETECT_STATE__INIT 0
#define MODE_DETECT_STATE__PHASE1 1
#define MODE_DETECT_STATE__PHASE2 2
#define MODE_DETECT_STATE__PHASE3 3
	unsigned int		modeDetect_State;

	unsigned int RequestFullFrameRetry;
	//CLIENT_INFO clientInfo;
	unsigned int anti_dither_thres;
	unsigned int force_1080i_to_720p;
	int	worstQualityMode; //Bruce110131. Used in host. Replace the one in disp_info.

	unsigned int video_rx_status;
	unsigned int video_rx_life_cycle;

	/*
	** Bruce161124. AST152X -> AST1500 interlace mode.
	**
	** VideoDev.progressively_capture_interlace == 0:
	** Host keep interlace HW capability and encode interlace mode as 2 fields in one frame.
	** Client decode interlace frame, but display only upper part of frame.
	** ==> User will see only first field information.
	**
	** VideoDev.progressively_capture_interlace == 1:
	** Host capture interlace timing using progressive way.
	** ==> each field is captured as separated frame.
	** Client decode each frame and display odd/even frame one by one.
	** ==> User will see picture flashing, but get complete odd/even information.
	*/
	unsigned int progressively_capture_interlace; //SOC_OP_MODE == 1 only.

	/* Global Config */
	ENGINE_CONFIG e_cfg;
	QUALITY_CONFIG q_cfg;
	CAPTURE_INFO cap_info;
	DISPLAY_INFO disp_info;
	VIDEO_MODE_INFO 	Video1ModeInfo;
	unsigned int force_timing_cfg;
#if (CONFIG_AST1500_SOC_VER >= 3)
	u32 mode_reg_shadow;
#endif
	unsigned int pkt_chunk_size;

	unsigned int current_sender;
	unsigned int current_cfg_sn;
	unsigned long src_stable_timeout; // in msecs
	unsigned long src_unavailable_timeout; //in msecs
	unsigned long pwr_save_timeout; //in msecs
	QCFG current_s_q_cfg;
	unsigned int pause_video_stream;
#ifdef CONFIG_ARCH_AST1500_CLIENT
	atomic_t sw_frame_queue_len;
	atomic_t hw_frame_queue_len; //for descriptor mode only
	atomic_t sw_frame_queue_len_limit;
#if (CONFIG_AST1500_SOC_VER == 2)
	u32 terminal_cnt_420;
#endif
#endif
	InfoFrame	AVI_Info;
	InfoFrame	HDMI_Info;
	InfoFrame	HDR_Info;

#if DELAY_DEQUEUE
	unsigned int hw_timer_value;
#endif

#if VIDEO_WALL
	unsigned int vw_max_row; // 0:disable, 1:/2, 2:/3, 3:/4,  //2, 3, 4
	unsigned int vw_row_num; // 0, 1, 2, 3
	unsigned int vw_max_col; // 0:disable, 1:/2, 2:/3, 3:/4,
	unsigned int vw_col_num; // 0, 1, 2, 3
	unsigned int vw_fc_h_total; // frame comp. in 0.1mm. include left and right border
	unsigned int vw_fc_h_active; // frame comp. in 0.1mm
	unsigned int vw_fc_v_total; // frame comp. in 0.1mm. include top and bottom border
	unsigned int vw_fc_v_active; //  frame comp.in 0.1mm
	unsigned int vw_cap_height;
	int vw_v_shift; // vertical shift
#endif

#if (CONFIG_AST1500_SOC_VER >= 3)
	struct ve_desc_context_t desc_context;
	struct work_struct gen_lock_work;
#define DOUBLE_BUFFER_DISABLE		0
#define DOUBLE_BUFFER_ENABLE		1
#define DOUBLE_BUFFER_ENABLE_NO_VE_FLIP 2
	u32 double_buffer;
	u32 last_vsync;
	u32 vsync_diff;
	u32 full_jpeg_mode;
#if defined(CONFIG_ARCH_AST1500_HOST)
	u32 frame_rate[MAX_QUALITY_NUM];
#else
	u32 frame_rate;
#endif
#if defined(CONFIG_ARCH_AST1500_CLIENT)
	u32 ve_flip_full_range_yuv;
#endif
#endif
	u32 snapshot_busy;
	u32 crt_interlace;

	struct socket_info_t sk_info;
#if (CONFIG_AST1500_SOC_VER >= 3) && defined(CONFIG_ARCH_AST1500_HOST)
	u8 ip4_tos; /* IPv4 TOS / DSCP + ECN */
#endif
#if (CONFIG_AST1500_SOC_VER >= 3) && defined(CONFIG_ARCH_AST1500_HOST)
	u32 pixel_clk; /* from Htotal x Vtotal x refresh rate */
#endif

	unsigned int video_type;
	unsigned int loopback_edid_patch;
	unsigned int remote_edid_patch;
	//unsigned int	EDID_Changed;//a tempary flag telling that EDID has been changed during mode detetcion
	u8 loopbackEDID[256];
	unsigned int	EEPROM_EDID_Valid;
	u8 eeprom1[512];
	u32 key[AST_VIDEO_KEY_LEN_DW_MAX];

	PacketInfo pkt_info; //Host: Used as send hdr. Client: Used as hdr receive buffer
	u32 session_id;
}VIDEOIP_DEVICE, *PVIDEOIP_DEVICE;

MAIN_EXT VIDEOIP_DEVICE 	VideoDev;

#define VIDEO_MAGIC_SEED (0x564F4950 + 1) /* VOIP + "Header Format Version" */
#define VIDEO_MAGIC (VideoDev.magic)

#if (VE_ENCODE_SW_WATCHDOG || VE_DECODE_SW_WATCHDOG)
#define restart_sw_watchdog(msecs) do { mod_timer(&VideoDev.watchdog_timer, jiffies + msecs_to_jiffies(msecs)); } while(0)
#define cancel_sw_watchdog() do { del_timer_sync(&VideoDev.watchdog_timer); } while(0)
#define is_sw_watchdog_on() timer_pending(&VideoDev.watchdog_timer)
#else
#define restart_sw_watchdog(msecs) do {} while (0)
#define cancel_sw_watchdog() do {} while (0)
#define is_sw_watchdog_on() (0)
#endif

#define pwr_save_after(msecs) \
do { \
	mod_timer(&VideoDev.pwr_save_timer, jiffies + msecs_to_jiffies(msecs)); \
} while(0)

#define cancel_pwr_save_timer()\
do { \
	del_timer_sync(&VideoDev.pwr_save_timer); \
} while(0)

#endif /* _GLOBAL_H_ */

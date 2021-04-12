/*
** Copyright (c) 2004-2014
** ASPEED Technology Inc. All Rights Reserved
** Proprietary and Confidential
**
** By using this code you acknowledge that you have signed and accepted
** the terms of the ASPEED SDK license agreement.
**
*/
#ifndef _SII9136_MAIN_H_
#define _SII9136_MAIN_H_

#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/workqueue.h>

#include <asm/arch/drivers/board_def.h>
#include <asm/arch/ast-scu.h>
#include <asm/arch/drivers/I2C.h>
#include <asm/arch/drivers/crt.h>
#include <asm/arch/drivers/video_hal.h>
#include "si_apiConfigure.h"

#define NO_INT_MODE 0
#define SUPPORT_HDMI_INFOFRAME_ONLY 1
#define PIXEL_REPETITION_BY_AVIINFO 1
#include <asm/arch/gpio.h>

#define ENABLE_INT() do {gpio_cfg(GPIO_CAT6613_INT, AST1500_GPIO_INT_LEVEL_LOW);} while (0)
#define DISABLE_INT() \
do { \
        gpio_cfg(GPIO_CAT6613_INT, AST1500_GPIO_INT_DISABLE); \
        /* ack interrupt. Disabling GPIO int won't clear its status, \
        ** so we need to do it by ourself. */ \
        gpio_ack_int_stat(GPIO_CAT6613_INT); \
} while (0)

/* for v_tx_drv_option */
/*
 * [1:0]: clock edge select: 1: rising, 0: falling
 * [1]: 4K clock edge select
 * [0]: 1080 clock edge select
 */
#define DRVOP_EDGE_SELECT_MASK (0x3 << 0) /* 1: use rising, 0: use falling */

#ifndef _VESA_H_
typedef enum tagHDMI_Video_Type {
	HDMI_Unknown = 0,
	HDMI_640x480p60 = 1,
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

struct sii9136_video_data {
	u8 hdmi:1;
	u8 hdcp:1;
	u8 audio:1;
#if (SUPPORT_HDMI_INFOFRAME_ONLY == 1)
	u8 infoframe_only:1;
	u8:4;
#else
	u8:5;
#endif

	u8 color_depth;
	u32 vclk;
	MODE_ITEM *display_mode;
	u8 input_color_mode;
	u8 color_mode;
	u16 vic;
	u8 pixelrep;
	u8 colorimetry;

	/* keep infoframe data together and stay in the end of data structure, DO NOT change this */
	InfoFrame avi_info;
	InfoFrame vs_info;
	InfoFrame hdr_info;
};

#define AUDIO_INFOFRAME_LEN 10

typedef union _Audio_InfoFrame {
        u8 AUD_HB[3] ;
        u8 AUD_DB[AUDIO_INFOFRAME_LEN] ;
} Audio_InfoFrame ;

struct sii9136_audio_data {
	u8 sample_frequency;
	u8 valid_ch;
	u8 sample_size;
	u8 audio_type;
	u8 is_spdif;

	Audio_InfoFrame infoframe;
	u8 iec60958_channel_status[5];
};

struct sii9136_drv_data {
	struct workqueue_struct *wq;
	struct work_struct timerwork;
	struct work_struct setup_video_task;
	struct work_struct setup_audio_task;
	struct work_struct disable_task;
#if SUPPORT_HDCP_REPEATER
	struct work_struct hdcp_auth_start_task;
	struct work_struct hdcp_auth_stop_task;
#endif
#if (IS_CEC == 1)
	struct work_struct work_cec_tx;
	struct work_struct work_cec_topology; /* query topolog */
	struct work_struct work_cec_physical_address; /* request pa for active device */

	u16 cec_topology;
#endif

	struct sii9136_video_data video;
	struct sii9136_video_data current_video;
	struct sii9136_audio_data audio;

	struct semaphore reg_lock;
#if (IS_CEC == 1)
	struct semaphore cec_lock;
#endif
	u32 pixel_repetition_by_avi;
};

#if defined(CONFIG_ARCH_AST1500_CLIENT) && (IS_CEC == 1)
unsigned int sii9136_cec_send(unsigned char *buf, unsigned int size);
#endif
#endif /* #ifndef _SII9136_MAIN_H_ */

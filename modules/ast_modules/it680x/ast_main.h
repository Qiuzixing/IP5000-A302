/*
** Copyright (c) 2004-2014
** ASPEED Technology Inc. All Rights Reserved
** Proprietary and Confidential
**
** By using this code you acknowledge that you have signed and accepted
** the terms of the ASPEED SDK license agreement.
**
*/
#ifndef _AST_MAIN_H_
#define _AST_MAIN_H_

#include <linux/workqueue.h>
#include <asm/semaphore.h>
#include <asm/arch/drivers/video_hal.h>
#include <asm/arch/drivers/crt.h>
#include "config.h"

#define EXTERNAL_HDCP_CHIP

#if (INTR_MODE == 1)
#define GPIO_IT6802_INT GPIO_CAT6023_INT
#endif

struct it680x_drv_data {
	void *it6802data;
	struct workqueue_struct *wq;
	struct work_struct timerwork;
	struct work_struct cec_pa_work;
	struct work_struct audio_update_work;
	struct work_struct cec_work;
	struct semaphore reg_lock;

	int force_update_audio_info;
	Audio_Info audio_info;
#if 0
	PFN_UPDATE_VIDEO_INFO video_event_callback;
	PFN_UPDATE_INFOFRAME infoframe_callback;
	PFN_UPDATE_AUDIO_INFO audio_event_callback;
#endif
	unsigned int dev_exist;
};

int vrx_register_info(struct recver_info *info);
void vrx_video_event(unsigned int status);
void vrx_infoframe(e_vrxhal_infoframe_type type);
void vrx_audio_event(Audio_Info info);

#if defined(EXTERNAL_HDCP_CHIP)
extern int (*ast_v_hdcp_status)(void);
extern int (*ast_v_hdcp_version)(void);
#endif
#endif //#ifndef _AST_MAIN_H_

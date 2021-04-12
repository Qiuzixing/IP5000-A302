/*
 * Copyright (c) 2019 ASPEED Technology Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */
#ifndef _IT6805_H_
#define _IT6805_H_

#include <linux/workqueue.h>
#include <asm/semaphore.h>
#include <asm/arch/drivers/video_hal.h>
#include <asm/arch/drivers/crt.h>
#include "config.h"
#include "iTE6805_Global.h"

#define MODULE_NAME "it6805"

#if (AST_IT6805_INTR == 1)
#define GPIO_IT6802_INT GPIO_CAT6023_INT
#endif

struct it680x_drv_data {
	void *dev_data;
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

#endif /* #ifndef _IT6805_H_ */

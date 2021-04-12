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



#ifndef _AST_WRAP_H_
#define _AST_WRAP_H_

#include "config.h"
#include "iTE6805_SYS.h"
#include "ast_it6805.h"

#define IT6805_DEV_DATA		_iTE6805_DATA
#define IT6805_VTIMING		_iTE6805_VTiming
#define IT6805_IDENTIFY_CHIP	iTE6805_Identify_Chip
#define IT6805_HPD_CTRL		iTE6805_Set_HPD_Ctrl
#define IT6805_FSM_INIT		iTE6805_Init_fsm
#define IT6805_FSM		iTE6805_MainBody /*iTE6805_FSM*/

//for module_param(drv_param, int, S_IRUGO);
extern int drv_param;
#define PARAM_DISABLE_HDCP			(0x1UL << 0)
#define PARAM_AUDIO_FORCE_LPCM2CH48KHZ		(0x1UL << 1)
#define PARAM_AUDIO_ALWAYS_ON			(0x1UL << 2) /* Used in conjunction with PARAM_AUDIO_FORCE_LPCM2CH48KHZ */


struct it680x_drv_data;

int is_dev_exists(void);
int ast_dev_init(struct it680x_drv_data *d);
void ast_fsm_timer_handler(struct it680x_drv_data *d);
#if (AST_IT6805_INTR == 1)
void ast_intr_handler(struct it680x_drv_data *d);
#endif
void ast_notify_video_state_change(void *drv_context);
int ast_get_video_timing_info(struct it680x_drv_data *d, void *pIn);
int ast_get_avi_info(struct it680x_drv_data *d, unsigned char *pData);
int ast_get_vsd_info(struct it680x_drv_data *d, unsigned char *pData);
int ast_get_hdr_info(struct it680x_drv_data *d, unsigned char *pData);
int ast_get_hdmi_n_cts(struct it680x_drv_data *d, unsigned int *pn, unsigned int *pcts);
void ast_schedule_audio_info_update(struct it680x_drv_data *d, int force_update);
void ast_hpd_ctrl(struct it680x_drv_data *d, unsigned int level);

#endif //#ifndef _AST_WRAP_H_

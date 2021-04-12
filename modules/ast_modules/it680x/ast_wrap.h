/*
** Copyright (c) 2004-2014
** ASPEED Technology Inc. All Rights Reserved
** Proprietary and Confidential
**
** By using this code you acknowledge that you have signed and accepted
** the terms of the ASPEED SDK license agreement.
**
*/

#ifndef _AST_WRAP_H_
#define _AST_WRAP_H_

#include "Mhlrx.h"

//for module_param(drv_param, int, S_IRUGO);
extern int drv_param;
#define PARAM_DISABLE_HDCP			(0x1UL << 0)
#define PARAM_AUDIO_FORCE_LPCM2CH48KHZ		(0x1UL << 1)
#define PARAM_AUDIO_ALWAYS_ON			(0x1UL << 2) /* Used in conjunction with PARAM_AUDIO_FORCE_LPCM2CH48KHZ */


struct it680x_drv_data;

int is_dev_exists(void);
int ast_dev_init(struct it680x_drv_data *d);
void ast_fsm_timer_handler(struct it680x_drv_data *d);
#if (INTR_MODE == 1)
void ast_intr_handler(struct it680x_drv_data *d);
#endif
void ast_notify_video_state_change(void *drv_context);
int ast_get_video_timing_info(struct it680x_drv_data *d, void *pIn);
int _get_avi_info(struct it680x_drv_data *d, unsigned char *pData);
int _get_hdr_info(struct it680x_drv_data *d, unsigned char *pData);
int ast_get_avi_info(struct it680x_drv_data *d, unsigned char *pData);
int ast_get_vsd_info(struct it680x_drv_data *d, unsigned char *pData);
int ast_get_hdr_info(struct it680x_drv_data *d, unsigned char *pData);
int ast_get_hdmi_n_cts(struct it680x_drv_data *d, unsigned int *pn, unsigned int *pcts);
void ast_schedule_audio_info_update(struct it680x_drv_data *d, int force_update);


#endif //#ifndef _AST_WRAP_H_

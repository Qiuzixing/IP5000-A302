/*
 * Copyright (c) 2004-2012
 * ASPEED Technology Inc. All Rights Reserved
 * Proprietary and Confidential
 *
 * By using this code you acknowledge that you have signed and accepted
 * the terms of the ASPEED SDK license agreement.
 */

#ifndef _VGA_RX_DRV_H_
#define _VGA_RX_DRV_H_

void vgarx_drv_ini(void);
void vgarx_init(void);
int vgarx_get_video_timing_info(void *pIn);
u32 vgarx_hsync_adjust_size(int mode_index, unsigned int is_ypbpr);
void vgarx_config_mode(int inMode);
void vgarx_set_phase(u32 value);
u32 vgarx_reg_get(u32 index);
void vgarx_reg_set(u32 index, u32 val);
#endif /* #ifndef _VGA_RX_DRV_H_ */

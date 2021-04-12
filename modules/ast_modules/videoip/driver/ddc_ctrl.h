/*
 * Copyright (c) 2004-2012
 * ASPEED Technology Inc. All Rights Reserved
 * Proprietary and Confidential
 *
 * By using this code you acknowledge that you have signed and accepted
 * the terms of the ASPEED SDK license agreement.
 */
#ifndef _DDC_CTRL_H_
#define _DDC_CTRL_H_

#define EDID_LOOPBACK 1
#define EDID_REMOTE   2
int EDID_interested_in(void);

void ddc_drv_init(void);
void detach_video_in(void);
void attach_video_in(int do_hotplug);

unsigned int validate_n_recover_edid_eeprom(unsigned char type);
void update_edid_eeprom(unsigned char *pEdid);
void do_hotplug(void);
unsigned int auto_detect_dual_input(void);

#endif /* #ifndef _DDC_CTRL_H_ */


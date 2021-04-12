/*
 * Copyright (c) 2004-2012
 * ASPEED Technology Inc. All Rights Reserved
 * Proprietary and Confidential
 *
 * By using this code you acknowledge that you have signed and accepted
 * the terms of the ASPEED SDK license agreement.
 */

#ifndef _DECODE_WORKER_H_
#define _DECODE_WORKER_H_

void do_CLIENT_EVENT_STOP_STREAM(unsigned int unlink);
void do_CLIENT_EVENT_START_STREAM(IOCTL_CLIENT_INFO *ioctl_info);
void do_CLIENT_EVENT_TX_HOTPLUG(void *pdata);
void do_CLIENT_EVENT_STOP_VE(void *pdata);
void do_CLIENT_EVENT_GOT_HOST_CHANGE_MODE(void *pdata);
void do_CLIENT_EVENT_GOT_HOST_MODE(PGLOBAL_CONFIG pFromHost);
void do_CLIENT_EVENT_DUMP_STREAM_ERROR(void *pdata);
void do_CLIENT_EVENT_ANTI_DITHER(void *pdata);
void do_CLIENT_EVENT_FIX_QUALITY_MODE(void *pdata);
void do_CLIENT_EVENT_GOT_STREAM_ERROR(void *pdata);
void do_CLIENT_EVENT_REQUEST_FULL_JPEG(int type);
void do_CLIENT_EVENT_TRIGGER_MODE_CHG(void *pdata);
void do_CLIENT_EVENT_REQUEST_FULL_FRAME(unsigned int why);
void do_CLIENT_EVENT_PWR_SAVE(void *pdata);
void do_CLIENT_EVENT_ENCRYPT_RESET(int off);
void do_CLIENT_EVENT_ENCRYPT_KEY_SETUP(void *pdata);
#if (CONFIG_AST1500_SOC_VER >= 3)
void do_CLIENT_EVENT_ON_CRT_SCALING_CHANGE(void *crt_chg);
void do_CLIENT_EVENT_HANDLE_VE_HANG_DESC(VIDEOIP_DEVICE * v);
void do_CLIENT_EVENT_HANDLE_VE_HANG_STREAM(VIDEOIP_DEVICE * v);
#endif /* #if (CONFIG_AST1500_SOC_VER >= 3) */
#endif /* #ifndef _DECODE_WORKER_H_ */

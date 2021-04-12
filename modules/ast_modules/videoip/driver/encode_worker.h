/*
 * Copyright (c) 2004-2012
 * ASPEED Technology Inc. All Rights Reserved
 * Proprietary and Confidential
 *
 * By using this code you acknowledge that you have signed and accepted
 * the terms of the ASPEED SDK license agreement.
 */

#ifndef _ENCODE_WORKER_H_
#define _ENCODE_WORKER_H_

#include "global.h" /* DISPLAY_INFO */

struct client_info_s {
	/*
	** display_info MUST placed at the first of struct client_info_s.
	** We played pointer trick in some socket receving func.
	*/
	DISPLAY_INFO display_info;
	struct socket *ctrl_socket;
};

#ifdef CONFIG_ARCH_AST1500_HOST
void do_HOST_EVENT_TX_HOTPLUG(void *pdata);
void do_HOST_EVENT_GOT_CLIENT_INFO(struct client_info_s *client);
void do_HOST_EVENT_MODE_DETECTION_DONE(unsigned int life_cycle);
void do_HOST_EVENT_GOT_CLIENT_READY(struct socket *sk);
void do_HOST_EVENT_GOT_MODE_CHG(void *pdata);
void do_HOST_EVENT_GOT_CLIENT_REQUEST_FULL_FRAME(int why);
void do_HOST_EVENT_GOT_CLIENT_REQUEST_FULL_JPEG(int type);
void do_HOST_EVENT_ADD_CLIENT(struct socket *sk);
void do_HOST_EVENT_DEL_CLIENT(struct socket *sk);
void do_HOST_EVENT_DEL_CLIENT_BY_IP(unsigned long ip);
void do_HOST_EVENT_TO_SUSPEND(void *pdata);
void do_HOST_EVENT_DISABLE_REMOTE_DONE(void *pdata);
void do_HOST_EVENT_DISABLE_REMOTE(void *pdata);
void do_HOST_EVENT_DETECT_MODE(unsigned int life_cycle);
void do_HOST_EVENT_ENCRYPT_RESET(int off);
void do_HOST_EVENT_ENCRYPT_KEY_SETUP(void *pdata);
void do_HOST_EVENT_HANDLE_VE_HANG(VIDEOIP_DEVICE *v);
void do_HOST_EVENT_UPDATE_INFOFRAME(e_vrxhal_infoframe_type type);
#endif /* #ifdef CONFIG_ARCH_AST1500_HOST */

#endif /* #ifndef _ENCODE_WORKER_H_ */

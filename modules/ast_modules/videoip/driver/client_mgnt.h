/*
 * Copyright (c) 2004-2012
 * ASPEED Technology Inc. All Rights Reserved
 * Proprietary and Confidential
 *
 * By using this code you acknowledge that you have signed and accepted
 * the terms of the ASPEED SDK license agreement.
 */

#ifndef __CLIENT_MGNT_H__
#define __CLIENT_MGNT_H__

#include "global.h"

#define VIRTUAL_CLIENT
#ifdef VIRTUAL_CLIENT
#define IS_VIRTUAL(pClient) ((pClient)->ip == -1)
#define IS_INTERNAL_VIRTUAL(pClient) (IS_VIRTUAL(pClient) && ((pClient)->CtrlSocket == NULL))
#endif

int cm_no_client(void);

#if defined(VIDEO_CM_ONLY_ONE_REAL_ENTITY)
#define cm_add_client(pclient) list_add(&pclient->list, &VideoDev.client_list)
#else
#define cm_add_client(pclient) \
	do { \
		list_add_tail(&pclient->list, &VideoDev.client_list); \
	} while (0)
#endif

PCLIENT_CONTEXT cm_get_context(struct socket *id);
void cm_notify_mode_detect_done(PCLIENT_CONTEXT pClient);
void cm_kill_client(PCLIENT_CONTEXT pClient, u32 force);
PCLIENT_CONTEXT cm_new_client(struct socket *sk);
void cm_kill_client_by_ip(unsigned long ip);
void cm_print_all_clients(unsigned char *buf);
unsigned int cm_hdcp2_add_event(PCLIENT_CONTEXT pClient, unsigned int event, void *pdata, unsigned int len, unsigned int free_data);
#if (HOST_VE_START_NO_REAL_CLIENT == 1)
int default_virtual_client_init(void);
#endif

#endif //#ifndef __CLIENT_MGNT_H__


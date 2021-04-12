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

#include <linux/list.h>
#include <linux/socket.h>
#include <linux/tcp.h>
#include "i2s_task.h"
#include <asm/arch/drivers/crt.h>

typedef struct _CLIENT_CONTEXT
{
	struct list_head	list;
	struct socket *tcp_CmdSocket; //Also used as an id of this client context.
	//struct socket *tcp_V1Socket;
	//struct socket *tcp_V2Socket;
	//struct socket *tcp_VMSocket;
#if 0//useless
	struct i2s_task client_cmd_task;
#endif
	struct i2s_task tx_data_t;
//	ULONG opState;
//	DISPLAY_INFO disp_info;
	int initialized;
	int killed;
	unsigned long ip; //IpV4 4 bytes ip format
} CLIENT_CONTEXT, *PCLIENT_CONTEXT;

#define cm_add_client(pclient) \
	do { \
		list_add_tail(&pclient->list, &client_list); \
	} while (0) \

PCLIENT_CONTEXT cm_get_context(struct socket *id);
void cm_send_cmd(CLIENT_CONTEXT *pClient, ULONG cmd, PUCHAR data, ULONG data_len);
void cm_audio_init(CLIENT_CONTEXT *pClient, Audio_Info *audio_info);
void cm_audio_change(Audio_Info *audio_info);
void cm_kill_client(PCLIENT_CONTEXT pClient);
PCLIENT_CONTEXT cm_new_client(struct socket *sk);
void cm_kill_client_by_ip(unsigned long ip);
void cm_print_all_clients(void);

#endif //#ifndef __CLIENT_MGNT_H__


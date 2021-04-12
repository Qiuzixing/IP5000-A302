/*
 * Copyright (c) 2004-2012
 * ASPEED Technology Inc. All Rights Reserved
 * Proprietary and Confidential
 *
 * By using this code you acknowledge that you have signed and accepted
 * the terms of the ASPEED SDK license agreement.
 */

/*
 * For client management
 */
#include "i2s_types.h"
//#include "i2s.h"
#include "client_mgnt.h"
#include "i2s_cmd.h"
#include "i2s_network.h"
#include "i2s_event.h"

struct list_head client_list;

PCLIENT_CONTEXT cm_get_context(struct socket *id)
{
	PCLIENT_CONTEXT pClient = NULL;

	if (list_empty(&client_list))
		return NULL;

	list_for_each_entry(pClient, &client_list, list) {
		if (pClient->tcp_CmdSocket == id)
			return pClient;
	}

	return NULL;
}

void cm_print_all_clients(void)
{
	PCLIENT_CONTEXT pClient = NULL;

	if (list_empty(&client_list))
		return;;

	printk("-------------------------\n");
	list_for_each_entry(pClient, &client_list, list) {
		unsigned long ip;

		ip = pClient->ip;

		printk("%ld.%ld.%ld.%ld (0x%08lX)\n",
		    (ip & 0x000000FF),
		    ((ip>>8) & 0x000000FF),
		    ((ip>>16) & 0x000000FF),
		    ((ip>>24) & 0x000000FF),
		    ip);
	}
	printk("-------------------------\n");

	return;

}


static PCLIENT_CONTEXT _get_context_by_ip(unsigned long ip)
{
	PCLIENT_CONTEXT pClient = NULL;

	if (list_empty(&client_list))
		return NULL;

	list_for_each_entry(pClient, &client_list, list) {
		if (pClient->ip == ip)
			return pClient;
	}

	return NULL;
}


static void _kill_client(PCLIENT_CONTEXT pClient)
{
	struct socket *sock = pClient->tcp_CmdSocket;

	I2S_info("--->_kill_client(%p)\n", pClient);
	list_del(&pClient->list);
	pClient->killed = 1;

	if (sock != NULL) {
//#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,24)
		sock->ops->shutdown(sock, 2);
//#else
//		kernel_sock_shutdown(sock, SHUT_RDWR);
//#endif
		I2S_info("CTRL socket shut down(%p)\n", pClient);
	}

	if (pClient->tx_data_t.thread != NULL) {
		stop_i2s_thread(&pClient->tx_data_t);
		I2S_info("i2s tx data thread stopped(%p)\n", pClient);
	}
	if (sock != NULL) {
		//sock_release(sock);
		sockfd_put(sock);
		I2S_info("CTRL socket released(%p)\n", pClient);
		pClient->tcp_CmdSocket = NULL;
	}

	kfree(pClient);
	I2S_info("_kill_client(%p)<---\n", pClient);
}

void cm_kill_client(PCLIENT_CONTEXT pClient)
{
	PCLIENT_CONTEXT t;

	if (pClient) {
		_kill_client(pClient);
		return;
	}

	/* If pClient == NULL */
	if (list_empty(&client_list))
		return;

	list_for_each_entry_safe(pClient, t, &client_list, list) {
		_kill_client(pClient);
	}

}

void cm_kill_client_by_ip(unsigned long ip)
{
	PCLIENT_CONTEXT pClient;

	I2S_info("kill IP %ld.%ld.%ld.%ld\n",
	    (ip & 0x000000FF),
	    ((ip>>8) & 0x000000FF),
	    ((ip>>16) & 0x000000FF),
	    ((ip>>24) & 0x000000FF)
	    );

	pClient = _get_context_by_ip(ip);

	if (pClient) {
		_kill_client(pClient);
	}
}

#if 0//useless
static void check_link(struct i2s_task *ut)
{
	int dummy;
	PCLIENT_CONTEXT pClient = container_of(ut, CLIENT_CONTEXT, client_cmd_task);
	struct socket *sock = pClient->tcp_CmdSocket;

	I2S_info("--->check_link(%p)\n", pClient);
	while (!pClient->killed) {
		if (signal_pending(current)) {
			I2S_info("check_link signal catched(%p)\n", pClient);
			break;
		}

		if (socket_xmit(0, sock, (char *)&dummy, sizeof(int), 0) != sizeof(int)) {
			I2S_info("client(%p) disconnected!\n", pClient);
#if 0//we cannot kill a client until its thread exits.
			pClient->client_cmd_task.thread = NULL;
			if (!i2sExit)
				cm_kill_client(pClient);
#else
			add_event(EVENT_CLIENT_DISCONNECTED, NULL, sock);
			schedule();
#endif
//rely on _kill_client to send	the kill signal to this thread and this thread will exit the loop when this kill signal is catched		break;
		}
	}
	sock->ops->shutdown(sock, 2);
	I2S_info("CTRL socket shut down(%p)\n", pClient);
	//sock_release(sock);
	sockfd_put(sock);
	I2S_info("CTRL socket released(%p)\n", pClient);
	I2S_info("check_link(%p)<---\n", pClient);
}
#endif

static unsigned long _get_socket_ip(struct socket *sk)
{
	struct sockaddr addr;
	int addrlen;

	if (sk->ops->getname(sk, &addr, &addrlen, 1)) {
		I2S_err("Can't get socket(%p)'s ip addr\n", sk);
		//ToDo.
	}
	/* The sa_data starts from 2 bytes of port num and 4 bytes of IP in big endian. */
	I2S_info("sk(%p) IP:%d.%d.%d.%d (0x%08lx)\n",
	        sk,
	        addr.sa_data[2],
	        addr.sa_data[3],
	        addr.sa_data[4],
	        addr.sa_data[5],
	        (*(long*)(addr.sa_data+2)));
	return (*(unsigned long*)(addr.sa_data+2));
}

PCLIENT_CONTEXT cm_new_client(struct socket *sk)
{
	PCLIENT_CONTEXT pClient;

	I2S_info("cm_new_client(%p)\n", sk);
	pClient = kmalloc(sizeof(CLIENT_CONTEXT), GFP_KERNEL);
	if (pClient == NULL) {
		I2S_err("Failed to allocate client context\n");
		return NULL;
	}
	memset(pClient, 0, sizeof(CLIENT_CONTEXT));

	//init client context
	INIT_LIST_HEAD(&pClient->list);
	pClient->tcp_CmdSocket = sk;
	pClient->ip = _get_socket_ip(sk);
#if 0//useless
	i2s_task_init(&pClient->client_cmd_task, "check_link", check_link);
	start_i2s_thread(&pClient->client_cmd_task);
#endif

	//add client context to client list
	cm_add_client(pClient);

	return pClient;
}

void cm_send_cmd(PCLIENT_CONTEXT pClient, ULONG cmd, PUCHAR data, ULONG data_len)
{
	Ctrl_Header	ctrl_header;
	I2S_info("cm_send_cmd (%08lX)\n", cmd);
	ctrl_header.Cmd = cmd;
	ctrl_header.DataLength = data_len;
	if (pClient){
		socket_xmit(1, pClient->tcp_CmdSocket, (PUCHAR)&ctrl_header, sizeof(Ctrl_Header), 0);
		if (data_len)
			socket_xmit(1, pClient->tcp_CmdSocket, data, data_len, 0);
		return;
	}

	/* If pClient == NULL */
	if (list_empty(&client_list))
		return;

	list_for_each_entry(pClient, &client_list, list) {
		if (pClient->initialized)
		{
			socket_xmit(1, pClient->tcp_CmdSocket, (PUCHAR)&ctrl_header, sizeof(Ctrl_Header), 0);
			if (data_len)
				socket_xmit(1, pClient->tcp_CmdSocket, data, data_len, 0);
		}
	}
}

void cm_audio_init(CLIENT_CONTEXT *pClient, Audio_Info *audio_info)
{
	if (pClient){
		if (pClient->initialized)
		{
			I2S_info("Audio should be initialized just once.\n");
			BUG();
		}
		else
		{
			pClient->initialized = 1;
			cm_send_cmd(pClient, CMD_CHANGE_AUDIO_SETTINGS, (PUCHAR)audio_info, sizeof(Audio_Info));
		}
		return;
	}

	/* If pClient == NULL */
	if (list_empty(&client_list))
		return;

	list_for_each_entry(pClient, &client_list, list) {
		if (!pClient->initialized)
		{
			pClient->initialized = 1;
			cm_send_cmd(pClient, CMD_CHANGE_AUDIO_SETTINGS, (PUCHAR)audio_info, sizeof(Audio_Info));
		}
	}
}

void cm_audio_change(Audio_Info *audio_info)
{
	CLIENT_CONTEXT *pClient;

	/* If pClient == NULL */
	if (list_empty(&client_list))
		return;

	list_for_each_entry(pClient, &client_list, list) {
#if 0
		if (!pClient->initialized)
		{
			I2S_info("Audio should have been initialized\n");
			BUG();
		}
		else
#endif
		{
			cm_send_cmd(pClient, CMD_CHANGE_AUDIO_SETTINGS, (PUCHAR)audio_info, sizeof(Audio_Info));
		}
	}
}


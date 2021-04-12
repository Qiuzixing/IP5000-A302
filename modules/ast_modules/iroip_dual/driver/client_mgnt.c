/*
 * Copyright (c) 2004-2012
 * ASPEED Technology Inc. All Rights Reserved
 * Proprietary and Confidential
 *
 * By using this code you acknowledge that you have signed and accepted
 * the terms of the ASPEED SDK license agreement.
 */

#ifdef CONFIG_ARCH_AST1500_HOST

/*
 * For client management
 */
#include "ir_types.h"
#include "ir_network.h"
#include "client_mgnt.h"

struct list_head client_list_head;
spinlock_t	client_list_lock;
//extern Ctrl_Header	ctrl_header;
extern void Net_Rx_Thread(struct ir_task *irt);

PCLIENT_CONTEXT cm_get_context(struct socket *id)
{
	PCLIENT_CONTEXT pClient = NULL;

	if (list_empty(&client_list_head))
		return NULL;

	list_for_each_entry(pClient, &client_list_head, list) {
		if (pClient->data_socket == id)
			return pClient;
	}

	return NULL;
}

void cm_print_all_clients(void)
{
	PCLIENT_CONTEXT pClient = NULL;

	if (list_empty(&client_list_head))
		return;;

	printk("-------------------------\n");
	list_for_each_entry(pClient, &client_list_head, list) {
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

	if (list_empty(&client_list_head))
		return NULL;

	list_for_each_entry(pClient, &client_list_head, list) {
		if (pClient->ip == ip)
			return pClient;
	}

	return NULL;
}



static void _kill_client(PCLIENT_CONTEXT pClient)
{
	IR_info("--->_kill_client(%p)\n", pClient);

	if (pClient->data_socket) {
		pClient->data_socket->ops->shutdown(pClient->data_socket, 2);
	}

	if (pClient->net_t.thread != NULL)
		stop_ir_thread(&pClient->net_t);

	/* Bruce100810. sock_release() is needed to release socket's memory.
	** I call sock_release() after videoip_thread_stop() so that
	** kernel won't free the socket when the socket is still been used.
	*/
	if (pClient->data_socket) {
		//sock_release(pClient->data_socket);
		sockfd_put(pClient->data_socket);
	}

	kfree(pClient);
	IR_info("_kill_client(%p)<---\n", pClient);
}

void cm_kill_client(PCLIENT_CONTEXT pClient)
{
	PCLIENT_CONTEXT t;

	if (pClient) {
		PCLIENT_CONTEXT pMatch;
		spin_lock(&client_list_lock);
		list_for_each_entry_safe(pMatch, t, &client_list_head, list) {
			if (pMatch == pClient) {
				list_del(&pClient->list);
				spin_unlock(&client_list_lock);
				_kill_client(pClient);
				return;
			}
		}
		spin_unlock(&client_list_lock);
		return;
	}

	spin_lock(&client_list_lock);
	/* If pClient == NULL */
	if (list_empty(&client_list_head)) {
		spin_unlock(&client_list_lock);
		return;
	}

	list_for_each_entry_safe(pClient, t, &client_list_head, list) {
		list_del(&pClient->list);
		spin_unlock(&client_list_lock);
		_kill_client(pClient);
		spin_lock(&client_list_lock);
	}
	spin_unlock(&client_list_lock);	
}

void cm_kill_client_by_ip(unsigned long ip)
{
	PCLIENT_CONTEXT pClient;

	IR_info("kill IP %ld.%ld.%ld.%ld\n",
	    (ip & 0x000000FF),
	    ((ip>>8) & 0x000000FF),
	    ((ip>>16) & 0x000000FF),
	    ((ip>>24) & 0x000000FF)
	    );

	pClient = _get_context_by_ip(ip);
	
	if (pClient) {
		cm_kill_client(pClient);
	}
}


static unsigned long _get_socket_ip(struct socket *sk)
{
	struct sockaddr addr;
	int addrlen;
	
	if (sk->ops->getname(sk, &addr, &addrlen, 1)) {
		IR_err("Can't get socket(%p)'s ip addr\n", sk);
		//ToDo.
	}
	/* The sa_data starts from 2 bytes of port num and 4 bytes of IP in big endian. */
	IR_info("sk(%p) IP:%d.%d.%d.%d (0x%08lx)\n", 
	        sk,
	        addr.sa_data[2],
	        addr.sa_data[3],
	        addr.sa_data[4],
	        addr.sa_data[5],
	        (*(long*)(addr.sa_data+2)));
	return (*(unsigned long*)(addr.sa_data+2));
}


PCLIENT_CONTEXT cm_new_client(struct socket *sk, struct s_ir_dev *ir)
{
	PCLIENT_CONTEXT pClient;

	IR_info("cm_new_client(%p)\n", sk);	
	pClient = kmalloc(sizeof(CLIENT_CONTEXT), GFP_KERNEL);
	if (pClient == NULL) {
		IR_err("Failed to allocate client context\n");
		return NULL;
	}
	memset(pClient, 0, sizeof(CLIENT_CONTEXT));

	//init client context
//	INIT_LIST_HEAD(&pClient->list);
	pClient->data_socket = sk;
	pClient->ip = _get_socket_ip(sk);
//	pClient->opState = STATE_INITIALIZING;
	pClient->ir = ir;
	ir_task_init(&pClient->net_t, "NET", Net_Rx_Thread);
	pClient->net_t.pContext = pClient;

	//add client context to client list
	cm_add_client(pClient);

	start_ir_thread(&pClient->net_t);
	//start cmd_task thread
//	pClient->opState = HOST_STATE_WAITING_CLIENT_INFO;
//	printk("Client(%p) enter HOST_STATE_WAITING_CLIENT_INFO\n", pClient);

	return pClient;
}

void cm_init(void)
{
	INIT_LIST_HEAD(&client_list_head);
	spin_lock_init(&client_list_lock);
}

#if 0
void cm_send_cmd(PCLIENT_CONTEXT pClient, ULONG cmd, PUCHAR data, ULONG data_len)
{
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
		socket_xmit(1, pClient->tcp_CmdSocket, (PUCHAR)&ctrl_header, sizeof(Ctrl_Header), 0);
		if (data_len)
			socket_xmit(1, pClient->tcp_CmdSocket, data, data_len, 0);
	}
}
#endif

#endif

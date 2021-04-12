/*
 * Copyright (c) 2004-2012
 * ASPEED Technology Inc. All Rights Reserved
 * Proprietary and Confidential
 *
 * By using this code you acknowledge that you have signed and accepted
 * the terms of the ASPEED SDK license agreement.
 */

#include <net/sock.h> 
#include "i2s_network.h"

extern int i2sInit;

int socket_xmit(int send, struct socket *sock, char *buf, int size, int msg_flags)
{
	int result;
	struct msghdr msg;
	struct kvec iov;
	int total = 0;

	if (!sock || !buf || !size) {
		printk("socket_xmit: invalid arg, sock %p buff %p size %d\n",
				sock, buf, size);
		return -1;
	}

	do {
		sock->sk->sk_allocation = GFP_NOIO;
		iov.iov_base    = buf;
		iov.iov_len     = size;
		msg.msg_name    = NULL;
		msg.msg_namelen = 0;
		msg.msg_control = NULL;
		msg.msg_controllen = 0;
		msg.msg_namelen    = 0;
		msg.msg_flags      = msg_flags | MSG_NOSIGNAL;

#if 0//steven
		if (send)
			result = kernel_sendmsg(sock, &msg, &iov, 1, size);
		else
			result = kernel_recvmsg(sock, &msg, &iov, 1, size, MSG_WAITALL);
			//result = kernel_recvmsg(sock, &msg, &iov, 1, size, MSG_DONTWAIT);

		if (result <= 0) {
			udbg("videoip_xmit: %s sock %p buf %p size %u ret %d total %d\n",
					send ? "send" : "receive", sock, buf, size, result, total);
			goto err;
		}
#else
		if (send){
			result = kernel_sendmsg(sock, &msg, &iov, 1, size);
			if (result < 0){
				printk("!!!kernel_sendmsg return %d, size(%d)!!!\n", result, size);
#if 0
				if ((!VideoDev.drvExit) && (!(VideoDev.errReported & ERR_SOCK_SEND)))
				{
					VideoDev.errReported |= ERR_SOCK_SEND;
					//It is the caller's decision
					//ast_notify_user("e_videoip_error");
				}
#endif
				goto err;
			}
		}
		else{
			result = kernel_recvmsg(sock, &msg, &iov, 1, size, MSG_WAITALL);
			if (result <= 0){
				printk("!!!kernel_recvmsg return %d!!!\n", result);
				if (result == 0){
					break;
				}
#if 0
				if ((!VideoDev.drvExit) && (!(VideoDev.errReported & ERR_SOCK_RECV)))
				{
					VideoDev.errReported |= ERR_SOCK_RECV;
					//It is the caller's decision
					//ast_notify_user("e_videoip_error");
				}
#endif
				goto err;
			}
		}
#endif

#if 1//steven
		if (!i2sInit)
			break;
#endif
		size -= result;
#if 0
            //ss add: for debug
            if (size > 0)
			udbg("videoip_xmit: resend %s sock %p buf %p size %u ret %d total %d\n",
					send ? "send" : "receive", sock, buf, size, result, total);
#endif
		buf += result;
		total += result;

#if 0
		if (VideoDev.mc_V1Socket) {
			if (send && (size > 0))
				printk("Can't finish send?! (%d)\n", size);

			break;
		}
#endif
	} while (size > 0);

	return total;

err:
	return result;
}

static int
_move_iov(
struct kvec **data,
size_t *num,
struct kvec *vec,
unsigned amount
)
{
	struct kvec *iv = *data;
	int i;
	int len;

	/*
	 *	Eat any sent kvecs
	 */
	while (iv->iov_len <= amount) {
		amount -= iv->iov_len;
		iv++;
		(*num)--;
	}

	/*
	 *	And chew down the partial one
	 */
	vec[0].iov_len = iv->iov_len-amount;
	vec[0].iov_base =((unsigned char *)iv->iov_base)+amount;
	iv++;

	len = vec[0].iov_len;

	/*
	 *	And copy any others
	 */
	for (i = 1; i < *num; i++) {
		vec[i] = *iv++;
		len += vec[i].iov_len;
	}

	//*data = vec;
	return len;
}

#if 0
int
socket_xmit_iov(
int send,
int transport,
struct socket *sock,
struct kvec *rq_iov,
size_t rq_iovlen,
int size,
int msg_flags)
{
	int result;
	struct msghdr msg;
	struct kvec iov[4];
	unsigned int total = 0;

	//dbg_xmit("enter\n");

	if (!sock || !size) {
		printk("videoip_xmit: invalid arg, sock %p size %d\n",
				sock, size);
		return -1;
	}

#if 0
	if (dbg_flag_xmit) {
		if (send) {
			if (!in_interrupt())
				printk(KERN_DEBUG "%-10s:", current->comm);
			else
				printk(KERN_DEBUG "interupt  :");

			printk("videoip_xmit: sending... , sock %p, size %d, msg_flags %d\n",
					sock, size, msg_flags);
		}
	}
#endif

	do {

		sock->sk->sk_allocation = GFP_NOIO;
		msg.msg_name    = NULL;
		msg.msg_namelen = 0;
		msg.msg_control = NULL;
		msg.msg_controllen = 0;
		msg.msg_namelen    = 0;
		msg.msg_flags      = msg_flags | MSG_NOSIGNAL;

		_move_iov(&rq_iov, &rq_iovlen, iov, total);

#if 0//steven
		if (send)
			result = kernel_sendmsg(sock, &msg, iov, rq_iovlen, size);
		else
			result = kernel_recvmsg(sock, &msg, iov, rq_iovlen, size, MSG_WAITALL);

		if (result <= 0) {
			/* Bruce. It is noisy.
			printk("usbip_xmit: %s sock %p buf %p size %u ret %d total %d\n",
					send ? "send" : "receive", sock, buf, size, result, total); */
			goto err;
		}
#else
	if (send){
		result = kernel_sendmsg(sock, &msg, iov, rq_iovlen, size);
		if (result < 0){
			printk("!!!kernel_sendmsg return %d, size(%d)!!!\n", result, size);
#if 0
			if ((!VideoDev.drvExit) && (!(VideoDev.errReported & ERR_SOCK_SEND)))
			{
				VideoDev.errReported |= ERR_SOCK_SEND;
				//It is the caller's decision
				//ast_notify_user("e_videoip_error");
			}
#endif
			goto err;
		}
	}
	else{
		result = kernel_recvmsg(sock, &msg, iov, rq_iovlen, size, MSG_WAITALL);
		if (result <= 0){
			printk("!!!kernel_recvmsg return %d!!!\n", result);
			if (result == 0){
				break;
			}
#if 0
			if ((!VideoDev.drvExit) && (!(VideoDev.errReported & ERR_SOCK_RECV)))
			{
				VideoDev.errReported |= ERR_SOCK_RECV;
				//It is the caller's decision
				//ast_notify_user("e_videoip_error");
			}
#endif
			goto err;
		}
	}
#endif

		if (!i2sInit)
			break;

		size -= result;
		total += result;

		if (!transport) {
			if (send && (size > 0))
				printk("Can't finish send?! (%d)\n", size);

			break;
		}
	} while (size > 0);

	return total;

err:
	printk("S{%d},T{%d}\n", result, total);
	return result;
}

#else
int
socket_xmit_iov_tcp(
int send,
struct socket *sock,
struct kvec *rq_iov,
size_t rq_iovlen,
int size,
int msg_flags)
{
	int result;
	struct msghdr msg;
	struct kvec iov[MAX_TCP_IOV_LENGTH];
	unsigned int total = 0;

	//dbg_xmit("enter\n");

	if (!sock || !size) {
		printk("socket_xmit_iov_tcp: invalid arg, sock %p size %d\n",
				sock, size);
		return -1;
	}

#if 0
	if (dbg_flag_xmit) {
		if (send) {
			if (!in_interrupt())
				printk(KERN_DEBUG "%-10s:", current->comm);
			else
				printk(KERN_DEBUG "interupt  :");

			printk("videoip_xmit: sending... , sock %p, size %d, msg_flags %d\n",
					sock, size, msg_flags);
		}
	}
#endif

	do {

		sock->sk->sk_allocation = GFP_NOIO;
		msg.msg_name    = NULL;
		msg.msg_namelen = 0;
		msg.msg_control = NULL;
		msg.msg_controllen = 0;
		msg.msg_namelen    = 0;
		msg.msg_flags      = msg_flags | MSG_NOSIGNAL;

		_move_iov(&rq_iov, &rq_iovlen, iov, total);

#if 0//steven
		if (send)
			result = kernel_sendmsg(sock, &msg, iov, rq_iovlen, size);
		else
			result = kernel_recvmsg(sock, &msg, iov, rq_iovlen, size, MSG_WAITALL);

		if (result <= 0) {
			/* Bruce. It is noisy.
			printk("usbip_xmit: %s sock %p buf %p size %u ret %d total %d\n",
					send ? "send" : "receive", sock, buf, size, result, total); */
			goto err;
		}
#else
	if (send){
		result = kernel_sendmsg(sock, &msg, iov, rq_iovlen, size);
		if (result < 0){
			printk("!!!kernel_sendmsg return %d, size(%d)!!!\n", result, size);
#if 0
			if ((!VideoDev.drvExit) && (!(VideoDev.errReported & ERR_SOCK_SEND)))
			{
				VideoDev.errReported |= ERR_SOCK_SEND;
				//It is the caller's decision
				//ast_notify_user("e_videoip_error");
			}
#endif
			goto err;
		}
	}
	else{
		result = kernel_recvmsg(sock, &msg, iov, rq_iovlen, size, MSG_WAITALL);
		if (result <= 0){
			printk("!!!kernel_recvmsg return %d!!!\n", result);
			if (result == 0){
				break;
			}
#if 0
			if ((!VideoDev.drvExit) && (!(VideoDev.errReported & ERR_SOCK_RECV)))
			{
				VideoDev.errReported |= ERR_SOCK_RECV;
				//It is the caller's decision
				//ast_notify_user("e_videoip_error");
			}
#endif
			goto err;
		}
	}
#endif

		if (!i2sInit)
			break;

		size -= result;
		total += result;
	} while (size > 0);

	return total;

err:
	printk("S{%d},T{%d}\n", result, total);
	return result;
}

int
socket_xmit_iov_udp(
int send,
struct socket *sock,
struct kvec *rq_iov,
size_t rq_iovlen,
int size,
int msg_flags)
{
	int result;
	struct msghdr msg;
	unsigned int total = 0;

	//dbg_xmit("enter\n");

	if (!sock || !size) {
		printk("socket_xmit_iov_udp: invalid arg, sock %p size %d\n",
				sock, size);
		return -1;
	}

	sock->sk->sk_allocation = GFP_NOIO;
	msg.msg_name	= NULL;
	msg.msg_namelen = 0;
	msg.msg_control = NULL;
	msg.msg_controllen = 0;
	msg.msg_namelen    = 0;
	msg.msg_flags	   = msg_flags | MSG_NOSIGNAL;
	if (send){
		result = kernel_sendmsg(sock, &msg, rq_iov, rq_iovlen, size);
		if (result < 0){
			printk("!!!kernel_sendmsg return %d, size(%d)!!!\n", result, size);
			goto err;
		}
	}
	else{
		result = kernel_recvmsg(sock, &msg, rq_iov, rq_iovlen, size, MSG_WAITALL);
		if (result <= 0){
			printk("!!!kernel_recvmsg return %d!!!\n", result);
			goto err;
		}
	}
	
	size -= result;
	total += result;
	
	if (send && (size > 0))
		printk("Can't finish send?! (%d)\n", size);

	return total;

err:
	printk("S{%d},T{%d}\n", result, total);
	return result;
}
#endif

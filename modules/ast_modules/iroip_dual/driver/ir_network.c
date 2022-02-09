/*
 * Copyright (c) 2004-2012
 * ASPEED Technology Inc. All Rights Reserved
 * Proprietary and Confidential
 *
 * By using this code you acknowledge that you have signed and accepted
 * the terms of the ASPEED SDK license agreement.
 */

#include <net/sock.h>

int socket_xmit(int send, struct socket *sock, char *buf, int size, int msg_flags)
{
	int result;
	struct msghdr msg;
	struct kvec iov;
	int total = 0;

	if (!sock || !buf || !size) {
		printk("videoip_xmit: invalid arg, sock %p buff %p size %d\n",
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

		if (send) {
			result = kernel_sendmsg(sock, &msg, &iov, 1, size);
			if (result < 0){
				printk("!!!kernel_sendmsg return %d, size(%d)!!!\n", result, size);
				goto err;
			}
		} else {
			result = kernel_recvmsg(sock, &msg, &iov, 1, size, MSG_WAITALL);
			if (result <= 0){
				printk("!!!kernel_recvmsg return %d!!!\n", result);
				if (result == 0){
					break;
				}
				goto err;
			}
		}

		size -= result;
		buf += result;
		total += result;

	} while (size > 0);

	return total;

err:
	return result;
}

static int _move_iov(struct kvec **data, size_t *num, struct kvec *vec, unsigned int amount)
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

int socket_xmit_iov(int send, int is_tcp, struct socket *sock, struct kvec *rq_iov, size_t rq_iovlen, int size, int msg_flags)
{
	int result;
	struct msghdr msg;
	struct kvec iov[4];
	unsigned int total = 0;

	if (!sock || !size) {
		printk("videoip_xmit: invalid arg, sock %p size %d\n",
				sock, size);
		return -1;
	}

	do {

		sock->sk->sk_allocation = GFP_NOIO;
		msg.msg_name    = NULL;
		msg.msg_namelen = 0;
		msg.msg_control = NULL;
		msg.msg_controllen = 0;
		msg.msg_namelen    = 0;
		msg.msg_flags      = msg_flags | MSG_NOSIGNAL;

		_move_iov(&rq_iov, &rq_iovlen, iov, total);

		if (send) {
			result = kernel_sendmsg(sock, &msg, iov, rq_iovlen, size);
			if (result < 0) {
				printk("!!!kernel_sendmsg return %d, size(%d)!!!\n", result, size);
				goto err;
			}
		} else {
			result = kernel_recvmsg(sock, &msg, iov, rq_iovlen, size, MSG_WAITALL);
			if (result <= 0) {
				if (result == 0)
					break;

				printk("!!!kernel_recvmsg return %d!!!\n", result);
				goto err;
			}
		}

		size -= result;
		total += result;

		if (!is_tcp) {
			if (send && (size > 0))
				printk("ERROR!! Can't finish send?! (%d)\n", size);

			break;
		}
	} while (size > 0);

	return total;

err:
	printk("S{%d},T{%d}\n", result, total);
	return result;
}


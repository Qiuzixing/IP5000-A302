#include <linux/version.h>
#include "astdebug.h"

#include "network.h"


 /*  Send/receive messages over TCP/IP. I refer drivers/block/nbd.c */
int skt_xmit(int send, struct socket *sock, char *buf, int size, int msg_flags)
{
	int result;
	struct msghdr msg;
	struct kvec iov;
	int total = 0;

	if (!sock || !buf || !size) {
		uerr("usbip_xmit: invalid arg, sock %p buff %p size %d\n",
				sock, buf, size);
		return -EINVAL;
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

		if (send)
			result = kernel_sendmsg(sock, &msg, &iov, 1, size);
		else
			result = kernel_recvmsg(sock, &msg, &iov, 1, size, MSG_WAITALL);

		if (result <= 0) {
			goto err;
		}

		size -= result;
		buf += result;
		total += result;

	} while (size > 0);

	return total;

err:
	return result;
}


void skt_shutdown(struct socket *tcp_socket)
{
	if (tcp_socket) {
		udbg("shutdown tcp_socket %p\n", tcp_socket);
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,24)
		/* in /usr/include/sys/socket.c, SHUT_RDWR is defined as 2. */
		tcp_socket->ops->shutdown(tcp_socket, 2);
#else
		kernel_sock_shutdown(tcp_socket, SHUT_RDWR);
#endif
	}
}


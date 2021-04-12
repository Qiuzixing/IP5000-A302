/*
 * Copyright (c) 2004-2012
 * ASPEED Technology Inc. All Rights Reserved
 * Proprietary and Confidential
 *
 * By using this code you acknowledge that you have signed and accepted
 * the terms of the ASPEED SDK license agreement.
 */

#ifndef _I2S_NETWORK_
#define _I2S_NETWORK_

#include <linux/socket.h>
#include <linux/tcp.h>
#include <linux/file.h> //For sockfd_put()

#define MAX_TCP_IOV_LENGTH	4
int socket_xmit(int send, struct socket *sock, char *buf, int size, int msg_flags);
#if 0
int socket_xmit_iov(int send, int transport, struct socket *sock, struct kvec *rq_iov, size_t rq_iovlen, int size, int msg_flags);
#else
int socket_xmit_iov_tcp(int send, struct socket *sock, struct kvec *rq_iov, size_t rq_iovlen, int size, int msg_flags);
int socket_xmit_iov_udp(int send, struct socket *sock, struct kvec *rq_iov, size_t rq_iovlen, int size, int msg_flags);
#endif

#endif

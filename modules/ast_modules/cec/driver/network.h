/*
 * Copyright (c) 2017
 * ASPEED Technology Inc. All Rights Reserved
 * Proprietary and Confidential
 *
 * By using this code you acknowledge that you have signed and accepted
 * the terms of the ASPEED SDK license agreement.
 */

#ifndef _NETWORK_H_
#define _NETWORK_H_

#include <net/sock.h>
#include <linux/file.h> /* For sockfd_put() */

#define IR_NET_PROT_UDP 0
#define IR_NET_PROT_TCP 1

struct cec_net_header {
	unsigned int magic;
	unsigned int length;
	unsigned int reserved1; /* TODO => Src, client should check this filed */
	unsigned int reserved2; /* TODO => Dst, client should chech this field */
};

#define CEC_MAGIC_NUM 0xB0CE1BAD
#define CEC_MAGIC_NUM_FRAME  (CEC_MAGIC_NUM + 0)
#define CEC_MAGIC_NUM_TOPOLOGY (CEC_MAGIC_NUM + 1)

int socket_xmit(int send, struct socket *sock, char *buf, int size, int msg_flags);
int socket_xmit_iov(int send, int is_tcp, struct socket *sock, struct kvec *rq_iov, size_t rq_iovlen, int size, int msg_flags);

#endif /* #ifndef _NETWORK_H_ */

/*
 * Copyright (c) 2004-2012
 * ASPEED Technology Inc. All Rights Reserved
 * Proprietary and Confidential
 *
 * By using this code you acknowledge that you have signed and accepted
 * the terms of the ASPEED SDK license agreement.
 */

#ifndef _IR_NETWORK_H_
#define _IR_NETWORK_H_

#include <net/sock.h> 
#include <linux/file.h> /* For sockfd_put() */
#define IR_NET_PROT_UDP	0
#define IR_NET_PROT_TCP	1
typedef struct _NetHeader_Struct
{
	unsigned int magic;
	unsigned int length;
	unsigned int reserved1;
	unsigned int reserved2;
} NetHeader_Struct, *PNetHeader_Struct;

#define IR_MAGIC_NUM 0xB0CE1BAD

int socket_xmit(int send, struct socket *sock, char *buf, int size, int msg_flags);
int socket_xmit_iov(int send, int is_tcp, struct socket *sock, struct kvec *rq_iov, size_t rq_iovlen, int size, int msg_flags);

#endif /* #ifndef _IR_NETWORK_H_ */

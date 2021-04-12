/*
 * Copyright (c) 2017
 * ASPEED Technology Inc. All Rights Reserved
 * Proprietary and Confidential
 *
 * By using this code you acknowledge that you have signed and accepted
 * the terms of the ASPEED SDK license agreement.
 */
#ifndef _IR_RS_H_
#define _IR_RS_H_

#include <errno.h>

#define u8	unsigned char
#define u16	unsigned short
#define u32	unsigned int

#undef IR_APP_DEBUG

#if defined(IR_APP_DEBUG)
#define msg(fmt, args...) printf(fmt, ##args)
#else
#define msg(fmt, args...)
#endif

#define IR_PORT		59400 /* IR net port */
#define IR_S_D_PORT	59401 /* irsd port */
#define IR_R_D_PORT	59402 /* irrd port */
#define IR_R_PORT	59403 /* irr port */


#define IR_SIZE_TEXT_MAX 4096
#define IR_SIZE_PAIR_MAX 256

/* from iroip_dual/driver/ir_network.h */
#define IR_MAGIC_NUM 0xB0CE1BAD

struct _ir_net_header {
	u32 magic;
	u32 length;
	u32 reserved[2];
};

static int create_socket_to_local(u16 port)
{
	int sockfd;
	struct sockaddr_in dest;

	/* create socket */
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);

	/* initialize value in dest */
	bzero(&dest, sizeof(dest));
	dest.sin_family = AF_INET;
	dest.sin_port = htons(port);
	dest.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

	/* Connecting to server */
	while (connect(sockfd, (struct sockaddr *)&dest, sizeof(dest)) != 0) {
		/* sleep for 10ms. */
		usleep(10000);
	}

	return sockfd;
}

static int create_socket_to_local_tcp(u16 port)
{
	int fd;
	struct sockaddr_in dest;
	struct sockaddr_in myaddr;   /* address that client uses */

	/* create socket */
	fd = socket(AF_INET, SOCK_STREAM, 0);

	/* Bind to an arbitrary return address. */
	bzero((char *)&myaddr, sizeof(myaddr));
	myaddr.sin_family = AF_INET;
	myaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	myaddr.sin_port = htons(0);


	if (bind(fd, (struct sockaddr *)&myaddr, sizeof(myaddr)) < 0) {
		perror("bind failed!");
		exit(1);
	}

	/* initialize value in dest */
	bzero(&dest, sizeof(dest));
	dest.sin_family = AF_INET;
	dest.sin_port = htons(port);
	dest.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

	/* Connecting to server */
	while (1) {
		if (connect(fd, (struct sockaddr *)&dest, sizeof(dest)) < 0) {
			if (errno == ECONNREFUSED)
				break;
		} else {
			break;
		}

		usleep(10000);
	}

	return fd;
}
#endif /* #ifndef _IR_RS_H_ */

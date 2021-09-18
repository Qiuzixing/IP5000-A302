/*
 * Copyright (c) 2004-2012
 * ASPEED Technology Inc. All Rights Reserved
 * Proprietary and Confidential
 *
 * By using this code you acknowledge that you have signed and accepted
 * the terms of the ASPEED SDK license agreement.
 */

#include <stdlib.h>
#include <syslog.h>
#include <stdio.h>
#include <string.h>
#include "type.h"
#include "astnetwork.h"
#include "debug.h"

int SetReuseaddr(int sockfd)
{
	const int val = 1;
	int ret;

	ret = setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));
	if (ret < 0)
		err("setsockopt SO_REUSEADDR\n");

	return ret;
}

int SetNodelay(int sockfd)
{
	const int val = 1;
	int ret;

	//ret = setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, &val, sizeof(val));
	ret = setsockopt(sockfd, SOL_SOCKET, TCP_NODELAY, &val, sizeof(val));
	if (ret < 0)
		err("setsockopt TCP_NODELAY\n");

	return ret;
}

int SetKeepalive(int sockfd)
{
	const int val = 1;
	int ret;

	ret = setsockopt(sockfd, SOL_SOCKET, SO_KEEPALIVE, &val, sizeof(val));
	if (ret < 0)
		err("setsockopt SO_KEEPALIVE\n");

	return ret;
}

int tcp_connect(char *hostname, char *service)
{
	struct addrinfo hints, *res, *res0;
	int sockfd;
	int err;


	memset(&hints, 0, sizeof(hints));
	hints.ai_socktype = SOCK_STREAM;

	/* get all possible addresses */
	err = getaddrinfo(hostname, service, &hints, &res0);
	if (err) {
		err("%s %s: %s\n", hostname, service, gai_strerror(err));
		return -1;
	}

	/* try all the addresses */
	for (res = res0; res; res = res->ai_next) {
		char hbuf[NI_MAXHOST], sbuf[NI_MAXSERV];

		err = getnameinfo(res->ai_addr, res->ai_addrlen,
				hbuf, sizeof(hbuf), sbuf, sizeof(sbuf), NI_NUMERICHOST | NI_NUMERICSERV);
		if (err) {
			err("%s %s: %s\n", hostname, service, gai_strerror(err));
			continue;
		}

		dbg("trying %s port %s\n", hbuf, sbuf);

		sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
		if (sockfd < 0) {
			err("socket\n");
			continue;
		}

		/* should set TCP_NODELAY for usbip */
		SetNodelay(sockfd);
		/* TODO: write code for heatbeat */
		SetKeepalive(sockfd);

		err = connect(sockfd, res->ai_addr, res->ai_addrlen);
		if (err < 0) {
			close(sockfd);
			continue;
		}

		/* connected */
		dbg("connected to %s:%s\n", hbuf, sbuf);
		freeaddrinfo(res0);
		return sockfd;
	}


	dbg("%s:%s, %s", hostname, service, "no destination to connect to\n");
	freeaddrinfo(res0);

	return -1;
}


int mc_create_sender(char *mgroup, int port)
{
	struct sockaddr_in addr;
	int fd;
	struct ip_mreq mreq;
	int yes = 1;
	
	/* create what looks like an ordinary UDP socket */
	if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		perror("socket");
		return -1;
	}

	/* set up bind address */
	memset(&addr, 0, sizeof(struct sockaddr_in));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(0);
     
	/* bind to receive address */
	if (bind(fd, (struct sockaddr *)&addr, sizeof(struct sockaddr_in)) < 0) {
		perror("bind");
		close(fd);
		return -1;
	}

	/* set up destination address */
	memset(&addr, 0, sizeof(struct sockaddr_in));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(mgroup);
	addr.sin_port = htons(port);

	if (connect(fd, (struct sockaddr *)&addr, sizeof(struct sockaddr)) < 0) {
		perror("connect");
		close(fd);
		return -1;
	}
#if 0	
	char *message = "Hello Start Kernel World!\n";
	if (sendto(fd, message, strlen(message) + 1, 0, &addr, sizeof(addr)) < 0) {
		perror("sendto");
		close(fd);
		return -1;
	}
#endif
	return fd;
}

int mc_connect(char *mgroup, int port)
{
	struct sockaddr_in addr;
	struct ip_mreq mreq;
	int fd;
	int yes = 1;
	
	/* create what looks like an ordinary UDP socket */
	if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		perror("socket");
		return -1;
	}

	/* allow multiple sockets to use the same PORT number */
	if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) < 0) {
		perror("Reusing ADDR failed");
		close(fd);
		return -1;
	}

	/* set up destination address */
	memset(&addr,0,sizeof(addr));
	addr.sin_family=AF_INET;
	addr.sin_addr.s_addr=htonl(INADDR_ANY);
	addr.sin_port=htons(port);
     
	/* bind to receive address */
	if (bind(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
		perror("bind");
		close(fd);
		return -1;
	}
	
	/* use setsockopt() to request that the kernel join a multicast group */
	mreq.imr_multiaddr.s_addr=inet_addr(mgroup);
	mreq.imr_interface.s_addr=htonl(INADDR_ANY);
	if (setsockopt(fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) < 0) {
		perror("setsockopt");
		close(fd);
		return -1;
	}

	return fd;
}





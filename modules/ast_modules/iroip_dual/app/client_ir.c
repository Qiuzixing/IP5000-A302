/*
 * Copyright (c) 2004-2012
 * ASPEED Technology Inc. All Rights Reserved
 * Proprietary and Confidential
 *
 * By using this code you acknowledge that you have signed and accepted
 * the terms of the ASPEED SDK license agreement.
 */

#include <unistd.h>
#include <stdlib.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <string.h>
#include <getopt.h>
#include <linux/socket.h>
#include <asm/delay.h>
#include "ir_ioctl.h"

#define IP_TTL_DEFAULT 64
#define IP4_MULTICAST(x)    (((x) & htonl(0xf0000000)) == htonl(0xe0000000))

static void Start_Ir(int transport, int ctrlconn, int dataconn)
{
	IO_ACCESS_DATA IO_Data;
	int fd = open("/dev/iroip", O_RDONLY);

	IO_Data.transport = transport;
	IO_Data.UpstreamSocket = ctrlconn;
	IO_Data.DownstreamSocket = dataconn;
	ioctl(fd, IOCTL_START_IR, &IO_Data);
	close(fd);
}

static int udp_create_sender(char *dst_addr, int port)
{
	struct sockaddr_in addr;
	int fd;
	struct ip_mreq mreq;
	int yes = 1;

	printf("udp_create_sender (%s, %d)\n", dst_addr, port);

	/* create what looks like an ordinary UDP socket */
	fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (fd < 0) {
		perror("socket");
		return -1;
	}

	/* set up bind address */
	memset(&addr, 0, sizeof(struct sockaddr_in));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(0);

	/* bind to send address */
	if (bind(fd, (struct sockaddr *)&addr, sizeof(struct sockaddr_in)) < 0) {
		perror("bind");
		return -1;
	}

	/* set up destination address */
	memset(&addr, 0, sizeof(struct sockaddr_in));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(dst_addr);
	addr.sin_port = htons(port);

	if (IP4_MULTICAST(addr.sin_addr.s_addr)) {
		unsigned char loop = 0;
		unsigned char ttl = IP_TTL_DEFAULT;

		if (setsockopt(fd, IPPROTO_IP, IP_MULTICAST_LOOP, &loop, sizeof(loop)) < 0)
			perror("sockopt: mulitcast loopback");
		if (setsockopt(fd, IPPROTO_IP, IP_MULTICAST_TTL, &ttl, sizeof(ttl)) < 0)
			perror("sockopt: ttl");
		}


	if (connect(fd, (struct sockaddr *)&addr, sizeof(struct sockaddr)) < 0) {
		perror("connect");
		return -1;
	}

	return fd;
}

static int udp_create_receiver(char *mgroup, int port)
{
	struct sockaddr_in addr;
	struct ip_mreq mreq;
	int fd;
	int yes = 1;

	printf("udp_create_receiver (%s, %d)\n", mgroup, port);	
	/* create what looks like an ordinary UDP socket */
	if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		perror("socket");
		return -1;
	}

	/* allow multiple sockets to use the same PORT number */
	if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) < 0) {
		perror("setsockopt (SO_REUSEADDR)");
		return -1;
	}

	/* set up destination address */
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(port);

	/* bind to receive address */
	if (bind(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
		perror("bind");
		return -1;
	}

	if (mgroup != NULL) {
		/* use setsockopt() to request that the kernel join a multicast group */
		mreq.imr_multiaddr.s_addr = inet_addr(mgroup);
		mreq.imr_interface.s_addr = htonl(INADDR_ANY);
		if (setsockopt(fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) < 0) {
			perror("setsockopt (IP_ADD_MEMBERSHIP)");
			return -1;
		}
	}

	return fd;
}

static int tcp_create_client(char *host_addr, int port)
{
	int sockfd;
	int sndbuf = 0x100000;
	struct sockaddr_in addr_svr;
	int ret = 0;

	printf("tcp_create_client (%s,%d)\n", host_addr, port);	
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd == -1) {
		perror("socket\n");
		ret = -1;
		goto done;
	}

	ret = setsockopt(sockfd, SOL_SOCKET, SO_SNDBUF, &sndbuf, 0x100000);
	if (ret != 0) {
		perror("setsockopt (SO_SNDBUF)\n");
		goto done;
	}

	memset(&addr_svr, 0, sizeof(addr_svr));
	addr_svr.sin_family = AF_INET;
	addr_svr.sin_port = htons(port);
	addr_svr.sin_addr.s_addr = inet_addr(host_addr);
	ret = connect(sockfd, (struct sockaddr *)&addr_svr, sizeof(addr_svr));
	if (ret == -1) {
		perror("connect\n");
		goto done;
	}

	printf("Connected\n");

done:
	if (ret == 0)
		return sockfd;
	else {
		if (sockfd >= 0)
			close(sockfd);
		return ret;
	}
}

static const char* short_opts = "ht:m:a:";
static const struct option long_opts[] = {
	{"help",	0,	NULL, 'h'},
	{"transport",	1,	NULL, 't'},
	{"multicast",	1,	NULL, 'm'},
	{"address",	1,	NULL, 'a'},
	{NULL,		0,		NULL,  0}
};

int main(int argc, char *argv[])
{
	int ctrlconnfd = -1, dataconnfd = -1;
	int transport = 0; /* 0:UDP; 1:TCP */
	char *mc_addr = NULL;
	char *host_addr = NULL;
	int ret = 0;

	for (;;) {
		int c;

		c = getopt_long(argc, argv, short_opts, long_opts, NULL);

		if (c == -1)
			break;

		switch (c) {
		case 't':
			if (strncmp(optarg, "tcp", 3) == 0) {
				printf("TCP transport\n");
				transport = 1;
			}
			break;
		case 'm':
			mc_addr = optarg;
			printf("multicast address: %s\n", mc_addr);
			break;
		case 'a':
			host_addr = optarg;
			printf("host address: %s\n", host_addr);
			break;
		default:
			/* to do:print help manual */
			goto done;
		}
	}

	if ((mc_addr) && (transport)) {
		printf("multicast only in UDP\n");
		transport = 0;
	}

	ctrlconnfd = tcp_create_client(host_addr, IR_UPSTREAM_PORT);
	if (ctrlconnfd < 0) {
		printf("failed to create CTRL client\n");
		ret = -1;
		goto done;
	}

	if (mc_addr) { /* multicast */
		dataconnfd = udp_create_receiver(mc_addr, IR_DOWNSTREAM_PORT);
		if (dataconnfd < 0) {
			printf("failed to create multicast data receiver\n");
			ret = -2;
			goto done;
		}
	} else { /* unicast */
		if (transport) { /* TCP */
			dataconnfd = tcp_create_client(host_addr, IR_DOWNSTREAM_PORT);
			if (dataconnfd < 0) {
				printf("failed to create TCP data client\n");
				ret = -2;
				goto done;
			}
		} else { /* UDP */
			dataconnfd = udp_create_receiver(NULL, IR_DOWNSTREAM_PORT);
			if (dataconnfd < 0) {
				printf("failed to create UDP data receiver\n");
				ret = -2;
				goto done;
			}
		}
	}

	if (ctrlconnfd != -1)
		close(ctrlconnfd);

	ctrlconnfd = udp_create_sender(host_addr, IR_DOWNSTREAM_PORT);

	Start_Ir(transport, ctrlconnfd, dataconnfd);

done:
	return ret;
}

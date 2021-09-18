/*
 * Copyright (c) 2004-2012
 * ASPEED Technology Inc. All Rights Reserved
 * Proprietary and Confidential
 *
 * By using this code you acknowledge that you have signed and accepted
 * the terms of the ASPEED SDK license agreement.
 */

#include <netdb.h>
#include <fcntl.h>
#include <getopt.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <unistd.h>
#include <errno.h>
#include "ir_ioctl.h"

#define IP_TTL_DEFAULT 64
#define IP4_MULTICAST(x)    (((x) & htonl(0xf0000000)) == htonl(0xe0000000))

static void Start_Ir(int transport, int connfd)
{
	IO_ACCESS_DATA IO_Data;
	int fd = open("/dev/iroip", O_RDONLY);

	IO_Data.transport = transport;
	IO_Data.DownstreamSocket = connfd;
	ioctl(fd, IOCTL_START_IR, &IO_Data);
	close(fd);
}

static void Add_Client(int connfd)
{
	IO_ACCESS_DATA IO_Data;
	int fd = open("/dev/iroip", O_RDONLY);

	IO_Data.UpstreamSocket = connfd;
	ioctl(fd, IOCTL_ADD_CLIENT, &IO_Data);
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
		close(fd);
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
		close(fd);
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
	fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (fd < 0) {
		perror("socket");
		return -1;
	}

	/* allow multiple sockets to use the same PORT number */
	if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) < 0) {
		perror("setsockopt (SO_REUSEADDR)");
		close(fd);
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
		close(fd);
		return -1;
	}

	if (mgroup != NULL) {
		/* use setsockopt() to request that the kernel join a multicast group */
		mreq.imr_multiaddr.s_addr = inet_addr(mgroup);
		mreq.imr_interface.s_addr = htonl(INADDR_ANY);
		if (setsockopt(fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) < 0) {
			perror("setsockopt (IP_ADD_MEMBERSHIP)");
			close(fd);
			return -1;
		}
	}

	return fd;
}

static int tcp_create_server(int port)
{
	int sockfd = -1;
	int reuseaddr = 1, tcpnodelay = 1;
	int ret = 0;
	struct sockaddr_in addr_svr;

	printf("tcp_create_server (%d)\n", port);	
	memset(&addr_svr, 0, sizeof(addr_svr));
	addr_svr.sin_family= AF_INET;
	addr_svr.sin_port= htons(port);
	addr_svr.sin_addr.s_addr = INADDR_ANY;

	sockfd = socket(AF_INET, SOCK_STREAM, 0);

	if (sockfd == -1) {
		perror("socket\n");
		ret = -1;
		goto done;
	}

	ret = setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &reuseaddr, sizeof(reuseaddr));
	if (ret < 0) {
		perror("setsockopt (SO_REUSEADDR)\n");
		goto done;
	}
	ret = setsockopt(sockfd, SOL_SOCKET, TCP_NODELAY, &tcpnodelay, sizeof(tcpnodelay));
	if (ret < 0) {
		perror("setsockopt (TCP_NODELAY)\n");
		goto done;
	}

	ret = bind(sockfd, (struct sockaddr *)&addr_svr, sizeof(addr_svr));
	if (ret == -1) {
		perror("bind\n");
		goto done;
	}

	ret = listen(sockfd, SOMAXCONN);
	if (ret == -1) {
		perror("listen\n");
	}

done:
	if (ret == 0)
		return sockfd;
	else {
		if (sockfd >= 0)
			close(sockfd);
		return ret; 
	}
}

static const char* short_opts = "ht:m:";
static const struct option long_opts[] = {
	{"help",	0,	NULL, 'h'},
	{"transport",	1,	NULL, 't'},
	{"multicast",	1,	NULL, 'm'},
	{NULL,		0,		NULL,  0}
};

int main(int argc, char *argv[])
{
	int reuseaddr = 1, tcpnodelay = 1, err;
	struct sockaddr_in addr_svr, addr_cln;
	socklen_t sLen = sizeof(addr_cln);
	int ctrlsvrfd = -1, ctrlconnfd = -1, datasvrfd = -1, dataconnfd = -1;
	int transport = 0; /* 0:UDP; 1:TCP */
	char *mc_addr = NULL;
	int ret = -1;

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
		case 'h':
			ret = 0;
		default:
			/* to do:print help manual */
			goto done;
		}
	}

	if ((mc_addr) && (transport)) {
		printf("multicast only in UDP\n");
		transport = 0;
	}

	/*
	**1. If multicasting is in use, create the UDP data sender using the multicast group address as the destination address.
	**2. Create the TCP CTRL server.
	**3. Accept a new TCP CTRL connection.
	**4. If multicasting is in use, go back to step 3.
	**5. If transport in use is TCP, create the TCP data server and accept the TCP data connection.
	**    Otherwise create the UDP data sender using the peer address learned from step 3 as the destination address.
	*/

	/* create the multicast UDP data sender */
	if (mc_addr) {
		dataconnfd = udp_create_sender(mc_addr, IR_DOWNSTREAM_PORT);
		if (dataconnfd < 0) {
			printf("failed to create the IR multicast sender\n");
			goto done;
		}
		Start_Ir(transport, dataconnfd);
		if (close(dataconnfd)) {
			printf("Failed to close IR multicast data fd (%d)\n", errno);
		}
	}

	if (transport) {/* TCP */
		/* create the TCP data server */
		datasvrfd = tcp_create_server(IR_DOWNSTREAM_PORT);
		if (datasvrfd < 0) {
			printf("failed to create the IR data server\n");
			goto done;
		}
	}
	/* create the TCP CTRL server */
	ctrlsvrfd = tcp_create_server(IR_UPSTREAM_PORT);
	if (ctrlsvrfd < 0) {
		printf("failed to create the IR CTRL server\n");
		goto done;
	}

	/* accept a TCP CTRL connection */
	ctrlconnfd = accept(ctrlsvrfd, (struct sockaddr *)&addr_cln, &sLen);
	if (ctrlconnfd < 0) {
		printf ("failed to accept a IR CTRL connection\n");
		goto done;
	}
	printf("IR CTRL connection accepted (%s)\n", inet_ntoa(addr_cln.sin_addr));

	if (!mc_addr) { /* unicast */
		if (transport) { /* TCP */
			/* accept the TCP data connection */
			dataconnfd = accept(datasvrfd, NULL, &sLen);
			if (dataconnfd < 0) {
				printf ("failed to accept the IR data connection\n");
				goto done;
			}
			printf("IR data connection accepted\n");
		} else { /* UDP */
			/* create the unicast UDP data sender */
			dataconnfd = udp_create_sender(inet_ntoa(addr_cln.sin_addr), IR_DOWNSTREAM_PORT);
			if (dataconnfd < 0) {
				printf("failed to create the IR unicast sender\n");
				goto done;
			}
		}

		Start_Ir(transport, dataconnfd);
	}

	if (close(ctrlconnfd))
		printf("Failed to close IR multicast control fd (%d)\n", errno);

	ctrlconnfd = udp_create_receiver(NULL, IR_DOWNSTREAM_PORT);
	Add_Client(ctrlconnfd);

	ret = 0;

done:
	return ret;
}

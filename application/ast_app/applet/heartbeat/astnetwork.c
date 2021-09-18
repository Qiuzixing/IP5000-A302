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
#include "astnetwork.h"
#include "debug.h"
#include <fcntl.h>
#include <errno.h>
#include <sys/ioctl.h> /* SIOCGIFHWADDR ioctl */
#include <linux/if.h> /* struct ifreq s; */

#define IP4_MULTICAST(x)    (((x) & htonl(0xf0000000)) == htonl(0xe0000000))
#define IP_TTL_DEFAULT 64

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

int tcp_connect_timeout(char *hostname, char *service, int secs)
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
		fcntl(sockfd, F_SETFL, O_NONBLOCK);

		err = connect(sockfd, res->ai_addr, res->ai_addrlen);
		if (err < 0 && errno == EINPROGRESS) {
			fd_set fds;
			struct timeval timeout;

			FD_ZERO(&fds);
			FD_SET(sockfd, &fds);
			timeout.tv_usec = 0;
			timeout.tv_sec = secs;
			err = select(sockfd+1, NULL, &fds, NULL, &timeout);
			if (err > 0) {
				int optval;
				socklen_t optlen = sizeof(optval);

				getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &optval, &optlen);
				if (optval == 0) {
					//connect successfully
					goto connected;
				}
			}
			//timeout or error
			close(sockfd);
			continue;

		} else if (err < 0) {
			close(sockfd);
			continue;
		}
connected:
		/* connected */
		dbg("connected to %s:%s\n", hbuf, sbuf);
		freeaddrinfo(res0);
		return sockfd;
	}


	dbg("%s:%s, %s", hostname, service, "no destination to connect to\n");
	freeaddrinfo(res0);

	return -1;
}

/*
** Create multicast send socket. Send to destination mgroup IP and port.
*/
int udp_create_sender(unsigned int ip_to_send_to, int port, unsigned int *binded_ip)
{
	struct sockaddr_in addr;
	int fd;
	struct ip_mreq mreq;

	/* create what looks like an ordinary UDP socket */
	if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		perror("udp_create_sender socket");
		return -1;
	}

	/* set up bind address */
	memset(&addr, 0, sizeof(struct sockaddr_in));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(0);

	/* bind to receive address */
	if (bind(fd, (struct sockaddr *)&addr, sizeof(struct sockaddr_in)) < 0) {
		perror("udp_create_sender bind");
		close(fd);
		return -1;
	}

	/* set up destination address */
	memset(&addr, 0, sizeof(struct sockaddr_in));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = ip_to_send_to;
	addr.sin_port = htons(port);

	if (IP4_MULTICAST(addr.sin_addr.s_addr)) {
		unsigned char loop = 0;
		unsigned char ttl = IP_TTL_DEFAULT;
		struct ip_mreq mreq;
		/* DO NOT loopback */
		if (0 > setsockopt(fd, IPPROTO_IP, IP_MULTICAST_LOOP, &loop, sizeof(loop)))
			perror("sockopt: mulitcast loopback");
		/* Set TTL to a higher value. */
		if (0 > setsockopt(fd, IPPROTO_IP, IP_MULTICAST_TTL, &ttl, sizeof(ttl)))
			perror("sockopt: ttl");

		/* Bruce161202. Add join mc group and hope mc network gets more robust. */
		/* use setsockopt() to request that the kernel join a multicast group */
		mreq.imr_multiaddr.s_addr = ip_to_send_to;
		mreq.imr_interface.s_addr = htonl(INADDR_ANY);
		if (setsockopt(fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) < 0)
			perror("setsockopt join multicast group fail");
	}

	if (connect(fd, (struct sockaddr *)&addr, sizeof(struct sockaddr)) < 0) {
		perror("udp_create_sender connect");
		close(fd);
		return -1;
	}

	if (binded_ip) {
		socklen_t len = sizeof(addr);
		if (getsockname(fd, (struct sockaddr *)&addr, &len) < 0) {
			perror("getsockname failed");
		} else {
			dbg("IP address is:%s (%08X)\n", inet_ntoa(addr.sin_addr), addr.sin_addr.s_addr);
			dbg("Port is:%d\n", (int)ntohs(addr.sin_port));
		}
		*binded_ip = inet_addr(inet_ntoa(addr.sin_addr));
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

/*
** Create multicast listener. Listen to mgroup IP and port.
*/
static int _udp_connect(unsigned int ip_to_listen, int port, unsigned int specify_peer)
{
	struct sockaddr_in addr;
	struct ip_mreq mreq;
	int fd;
	int yes = 1;

	/* create what looks like an ordinary UDP socket */
	if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		perror("udp_connect socket");
		return -1;
	}

	/* allow multiple sockets to use the same PORT number */
	if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) < 0) {
		perror("Reusing ADDR failed");
		close(fd);
		return -1;
	}

	/* set up destination address */
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	/*
	** Bruce161102. By specifying addr.sin_addr.s_addr=ip_to_listen
	** only IP:Port matched will be forwarded from IP layer to app.
	** If specified with 'htonl(INADDR_ANY)', then only 'port' will be matched.
	** Then different multicast IP will be forwarded to this socket when
	** port numbers are the same.
	*/
	if (IP4_MULTICAST(ip_to_listen)) {
		addr.sin_addr.s_addr = ip_to_listen;
	} else {
		/* for unicast, bind to any local address interface will be fine. */
		addr.sin_addr.s_addr = htonl(INADDR_ANY);
	}
	addr.sin_port = htons(port);

	/* bind to receive address */
	if (bind(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
		perror("udp_connect bind");
		close(fd);
		return -1;
	}

	if (IP4_MULTICAST(ip_to_listen)) {
		/* use setsockopt() to request that the kernel join a multicast group */
		mreq.imr_multiaddr.s_addr = ip_to_listen;
		mreq.imr_interface.s_addr = htonl(INADDR_ANY);
		if (setsockopt(fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) < 0) {
			perror("udp_connect setsockopt");
			close(fd);
			return -1;
		}
	} else {
		if (specify_peer) {
			/* For unicast case, use connect() to sepcify whom to receive. */
			/* Without specifying peer address, any UDP source send to listen_port will be passed to this socket. */
			memset(&addr, 0, sizeof(struct sockaddr_in));
			addr.sin_family = AF_INET;
			addr.sin_addr.s_addr = ip_to_listen;
			addr.sin_port = htons(0); /* Receive from any sender port number. */
			if (connect(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
				perror("udp_connect connect");
				close(fd);
				return -1;
			}
		}
	}

	return fd;
}

int udp_connect(unsigned int ip_to_listen, int port)
{
	return _udp_connect(ip_to_listen, port, 1);
}

int udp_connect_as_server(unsigned int ip_to_listen, int port)
{
	return _udp_connect(ip_to_listen, port, 0);
}

unsigned int get_mac_addr(void)
{
	struct ifreq s;
	int fd = socket(PF_INET, SOCK_DGRAM, IPPROTO_IP);

	strcpy(s.ifr_name, "eth0");
	if (0 == ioctl(fd, SIOCGIFHWADDR, &s)) {
#if 0
		int i;
		for (i = 0; i < 6; ++i) {
			printf(" %02x", (unsigned char) s.ifr_addr.sa_data[i]);
		}
#endif
		close(fd);
		/* return lower 4 bytes of MAC address. */
		return *((unsigned int*)(((unsigned char*)s.ifr_addr.sa_data) + 2));
	} else {
		close(fd);
		return 0;
	}
}

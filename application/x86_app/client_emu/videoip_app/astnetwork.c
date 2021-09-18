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
#include <sys/ioctl.h>
#include <errno.h>
#include <arpa/inet.h>
#include <net/if_arp.h>
#include <net/if.h>
#include "global.h"
#include "astdebug.h"
#include "astnetwork.h"

int SetReuseaddr(int sockfd)
{
	const int val = 1;
	int ret;

	ret = setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));
	if (ret < 0)
		err("setsockopt SO_REUSEADDR");

	return ret;
}

int SetNodelay(int sockfd)
{
	const int val = 1;
	int ret;

	//ret = setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, &val, sizeof(val));
	ret = setsockopt(sockfd, SOL_SOCKET, TCP_NODELAY, &val, sizeof(val));
	if (ret < 0)
		err("setsockopt TCP_NODELAY");

	return ret;
}

int SetKeepalive(int sockfd)
{
	const int val = 1;
	int ret;

	ret = setsockopt(sockfd, SOL_SOCKET, SO_KEEPALIVE, &val, sizeof(val));
	if (ret < 0)
		err("setsockopt SO_KEEPALIVE");

	return ret;
}

int set_sockopt_multicast_loop(int sockfd, unsigned char cfg)
{
	unsigned char loop = cfg;

	return setsockopt(sockfd, IPPROTO_IP, IP_MULTICAST_LOOP, &loop, sizeof(loop));
}

int set_sockopt_ttl(int sockfd, unsigned char cfg)
{
	unsigned char ttl = cfg;

	return setsockopt(sockfd, IPPROTO_IP, IP_MULTICAST_TTL, &ttl, sizeof(ttl));
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
		err("%s %s: %s", hostname, service, gai_strerror(err));
		return -1;
	}

	/* try all the addresses */
	for (res = res0; res; res = res->ai_next) {
		char hbuf[NI_MAXHOST], sbuf[NI_MAXSERV];

		err = getnameinfo(res->ai_addr, res->ai_addrlen,
				hbuf, sizeof(hbuf), sbuf, sizeof(sbuf), NI_NUMERICHOST | NI_NUMERICSERV);
		if (err) {
			err("%s %s: %s", hostname, service, gai_strerror(err));
			continue;
		}

		dbg("trying %s port %s\n", hbuf, sbuf);

		sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
		if (sockfd < 0) {
			err("socket");
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
		dbg("connected to %s:%s", hbuf, sbuf);
		freeaddrinfo(res0);
		return sockfd;
	}


	dbg("%s:%s, %s", hostname, service, "no destination to connect to");
	freeaddrinfo(res0);

	return -1;
}

int udp_create_sender(char *dst_addr, int port)
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
	addr.sin_addr.s_addr = inet_addr(dst_addr);
	addr.sin_port = htons(port);

	if (IP4_MULTICAST(addr.sin_addr.s_addr)) {
		if (0 > set_sockopt_multicast_loop(fd, 0))
			perror("sockopt: mulitcast loopback");

		if (0 > set_sockopt_ttl(fd, IP_TTL_DEFAULT))
			perror("sockopt: ttl");
	}

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

int udp_connect(char *mgroup, int port)
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

	if (mgroup != NULL) {
		/* use setsockopt() to request that the kernel join a multicast group */
		mreq.imr_multiaddr.s_addr=inet_addr(mgroup);
		mreq.imr_interface.s_addr=htonl(INADDR_ANY);
		if (setsockopt(fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) < 0) {
			perror("setsockopt");
			close(fd);
			return -1;
		}
	}

	return fd;
}

int get_dst_ip(int skfd, unsigned int *ip_little_endian, unsigned short *port_little_endian)
{
	struct sockaddr_in ip;
	socklen_t ip_len = sizeof(struct sockaddr_in);

	if (getpeername(skfd, (struct sockaddr *)&ip, &ip_len)) {
		printf("Failed to get remote IP?! (%d)\n", errno);
		return -1;
	}

	//printf("Remote IP info: %s:%d\n", inet_ntoa(ip.sin_addr), ntohs(ip.sin_port));
	*ip_little_endian = (unsigned int)ntohl(ip.sin_addr.s_addr);
	*port_little_endian = (unsigned short)ntohs(ip.sin_port);

	return 0;
}

int get_my_ip(int skfd, unsigned int *ip_little_endian, unsigned short *port_little_endian)
{
	struct sockaddr_in ip;
	socklen_t ip_len = sizeof(struct sockaddr_in);

	if (getsockname(skfd, (struct sockaddr *)&ip, &ip_len)) {
		printf("Failed to get local IP?! (%d)\n", errno);
		return -1;
	}

	*ip_little_endian = (unsigned int)ntohl(ip.sin_addr.s_addr); //assume we are always running on little endian CPU.
	*port_little_endian = (unsigned short)ntohs(ip.sin_port);

	//printf("Local IP info: %s:%d, 0x%08X,0x%08X\n", inet_ntoa(ip.sin_addr), ntohs(ip.sin_port), *ip_little_endian, ip.sin_addr.s_addr);

	return 0;
}

int unicast_dst_ip_to_mac(const unsigned int ip, unsigned char *mac)
{
/*  arp_flags and at_flags field values */
#define	ATF_INUSE	0x01	/* entry in use */
#define ATF_COM		0x02	/* completed entry (enaddr valid) */
#define	ATF_PERM	0x04	/* permanent entry */
#define	ATF_PUBL	0x08	/* publish entry (respond for other host) */
#define	ATF_USETRAILERS	0x10	/* has requested trailers */
#define	ATF_PROXY	0x20	/* Do PROXY arp */

	int s;
	struct arpreq arpreq;
	struct sockaddr_in *sin;

	memset(&arpreq, 0, sizeof(arpreq));

	sin = (struct sockaddr_in *) &arpreq.arp_pa;
	sin->sin_family = AF_INET;
	sin->sin_addr.s_addr = ip;
	//inet_aton(ip, &sin->sin_addr);
	strncpy(arpreq.arp_dev, "eth0", 15);

	s = socket(AF_INET, SOCK_DGRAM, 0);
	if (s < 0) {
		perror("socket");
		return -1;
	}
	if (ioctl(s, SIOCGARP, (caddr_t)&arpreq) < 0) {
		if (errno == ENXIO) {
			printf("arp (%s) -- no entry.\n", inet_ntoa(sin->sin_addr));
		}
		perror("ioctl");
		close(s);
		return -1;
	}
	//printf("IP address:       %s\n", inet_ntoa(sin->sin_addr));

	if (arpreq.arp_flags & ATF_COM) {
		memcpy(mac, (unsigned char *) &arpreq.arp_ha.sa_data[0], 6);
#if 0
		printf("Ethernet address: %02X:%02X:%02X:%02X:%02X:%02X",
				mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
		if (arpreq.arp_flags & ATF_PERM) printf(" PERM");
		if (arpreq.arp_flags & ATF_PUBL) printf(" PUBLISHED");
		if (arpreq.arp_flags & ATF_USETRAILERS) printf(" TRAILERS");
		if (arpreq.arp_flags & ATF_PROXY) printf(" PROXY");
		printf("\n");
#endif
	} else {
		printf("*** INCOMPLETE ***\n");
	}
	close(s);
	return 0;
}

int multicast_ip_to_mac(const unsigned int ip, unsigned char *mac)
{
	unsigned char *ip_digit;
	
	ip_digit = (unsigned char *)&ip;
	
	if ((ip_digit[0] & 0xF0) != 0xE0) {
		printf("Invalid multicast IP address!!\n");
		return -1;
	}
	/*
	** Multicast MAC is constructed by fixed higher order 25bits 01:00:5E:0
	** and lower 23bits copied from IP's lower 23bits.
	*/
	mac[0] = 0x01;
	mac[1] = 0x00;
	mac[2] = 0x5E;
	mac[3] = ip_digit[1] & 0x7F;
	mac[4] = ip_digit[2];
	mac[5] = ip_digit[3];
#if 0
	printf("Multicast Ethernet address: %02X:%02X:%02X:%02X:%02X:%02X\n",
			mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
#endif
	return 0;
}

int get_my_mac(int skfd, unsigned char *mac)
{
	struct ifreq ifr;
	char *iface = "eth0";

	ifr.ifr_addr.sa_family = AF_INET;
	strncpy(ifr.ifr_name , iface , IFNAMSIZ-1);

	if (ioctl(skfd, SIOCGIFHWADDR, &ifr)) {
		printf("Failed to get local MAC addr?! (%d)\n", errno);
		return -1;
	}

	memcpy(mac, (unsigned char *)ifr.ifr_hwaddr.sa_data, 6);
#if 0
	//display mac address
	printf("Local Ethernet address: %02X:%02X:%02X:%02X:%02X:%02X\n",
			mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
#endif
	return 0;
}

int get_mtu(int skfd, unsigned int *mtu)
{
	struct ifreq ifr;
	char *iface = "eth0";

	ifr.ifr_addr.sa_family = AF_INET;
	strncpy(ifr.ifr_name , iface , IFNAMSIZ-1);

	if (ioctl(skfd, SIOCGIFMTU, &ifr)) {
		printf("Failed to get local MAC addr?! (%d)\n", errno);
		return -1;
	}

	*mtu = (unsigned int)ifr.ifr_mtu;
	return 0;
}


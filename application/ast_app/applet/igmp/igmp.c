/*
 * Copyright (c) 2004-2012
 * ASPEED Technology Inc. All Rights Reserved
 * Proprietary and Confidential
 *
 * By using this code you acknowledge that you have signed and accepted
 * the terms of the ASPEED SDK license agreement.
 */

/*
 * Most of the code is reference from:
 * http://cboard.cprogramming.com/networking-device-communication/107801-linux-raw-socket-programming.html
 */

#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <getopt.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/igmp.h>
#include <netdb.h> //for gethostbyname()
#include <linux/ip.h>
#include <linux/igmp.h>
#include "debug.h"

/*
 * RFC2236,
 * 2. Introduction
 * All IGMP messages described in this document are sent with IP TTL 1,
 * and contain the IP Router Alert option [RFC 2113] in their IP header.
 */
#define IGMPV2_IPOPT_RA /* IP option - router alert */

#define USE_MALLOC 0

#if defined(IGMPV2_IPOPT_RA)
#define IPOPT_SIZE (4)
#else
#define IPOPT_SIZE (0)
#endif

#define PACKET_SIZE (64 + IPOPT_SIZE)

/*
 * in_cksum --
 * Checksum routine for Internet Protocol
 * family headers (C Version)
 */
static unsigned short in_cksum(unsigned short *addr, int len)
{
	register int sum = 0;
	u_short answer = 0;
	register u_short *w = addr;
	register int nleft = len;
	/*
	* Our algorithm is simple, using a 32 bit accumulator (sum), we add
	* sequential 16 bit words to it, and at the end, fold back all the
	* carry bits from the top 16 bits into the lower 16 bits.
	*/
	while (nleft > 1) {
		sum += *w++;
		nleft -= 2;
	}

	/* mop up an odd byte, if necessary */
	if (nleft == 1) {
		*(u_char *) (&answer) = *(u_char *) w;
		sum += answer;
	}

	/* add back carry outs from top 16 bits to low 16 bits */
	sum = (sum >> 16) + (sum & 0xffff);		/* add hi 16 to low 16 */
	sum += (sum >> 16);				/* add carry */
	answer = ~sum;				/* truncate to 16 bits */
	return (answer);
}

static void getip(char *addr)
{
	char buffer[256];
	struct hostent* h;
	
	gethostname(buffer, 256);
	strcat(buffer, ".local");
	h = (struct hostent*)gethostbyname(buffer);

	if (h) {
		strncpy(addr, (char*)inet_ntoa(*(struct in_addr *)h->h_addr), 15);	
	} else {
		strncpy(addr, "0.0.0.0", 15);
	}	
}

/*
** Specific query:
**    - Use multicast group ip for both IP.dst_addr and IGMP.group
** General query:
**    - Use 224.0.0.1 as IP.dst_addr
**    - Use 0.0.0.0 as IGMP.group
*/
static int igmpV2_send(
	char *dst_addr, 
	char *src_addr, 
	unsigned char igmp_type, 
	unsigned char igmp_code, 
	char *igmp_group)
{
	int optval;
	struct sockaddr_in addr;
	int fd = 0;
	struct iphdr *ip = NULL;
	struct igmphdr *igmp = NULL;
#if USE_MALLOC
	char *packet = NULL;
#else
	char packet[PACKET_SIZE];
#endif
	int ipopt_size = IPOPT_SIZE;

	info("igmpV2_send (%s %s 0x%02X %d %s)\n", 
		dst_addr, src_addr, igmp_type, igmp_code, igmp_group);

	if (getuid() != 0) {
		err("root privelidges needed\n");
		return -1;
	}

	/* create what looks like an ordinary UDP socket */
	/* Use IPPROTO_RAW, otherwise IGMP field will be overwritten by under layer. */
	if ((fd = socket(AF_INET, SOCK_RAW, IPPROTO_RAW)) < 0) {
		perror("socket");
		return -1;
	}

	/* 
	*	IP_HDRINCL must be set on the socket so that
	*	the kernel does not attempt to automatically add
	*	a default ip header to the packet
	*/
	//setsockopt(fd, IPPROTO_IP, IP_HDRINCL, &optval, sizeof(int));
	/* Set to IPPROTO_IGMP, otherwise packet won't be sent. ?? */
	setsockopt(fd, IPPROTO_IGMP, IP_HDRINCL, &optval, sizeof(int));

	/* Use static memory so that no memory leaking caused by "kill -9". */
#if USE_MALLOC
	/* allocate all necessary memory */
	packet = malloc(sizeof(struct iphdr) + ipopt_size + sizeof(struct igmphdr));
	if (packet == NULL) {
		perror("malloc");
		goto OUT;
	}
#else
	memset(packet, 0, PACKET_SIZE);
#endif
	ip = (struct iphdr *)packet;
	igmp = (struct igmphdr *)(packet + ipopt_size + sizeof(struct iphdr));

	/* here the ip packet is set up except checksum */
	ip->ihl = (sizeof(struct iphdr) + ipopt_size) >> 2;
	ip->version = 4;
	ip->tos = 0;
	ip->tot_len = sizeof(struct iphdr) + ipopt_size + sizeof(struct igmphdr);
	ip->id = htons((uint16_t)random()); //From sniffer, it is a sequence number
	ip->frag_off = 0;
	ip->ttl = 1;
	ip->protocol = IPPROTO_IGMP;
	ip->check = 0;
	ip->saddr = inet_addr(src_addr); // Use 0.0.0.0?
	ip->daddr = inet_addr(dst_addr);
#if defined(IGMPV2_IPOPT_RA)
	/* Copy flag, 1b'1  + Class, 2b'00 + Option 5b'10100 (20) */
	((unsigned char *)&ip[1])[0] = ((0x1 << 7) | (0 << 5) | 20);
	/* length: 4 */
	((unsigned char *)&ip[1])[1] = 4;
	/* value: 0 - Router shall examine packet*/
	((unsigned char *)&ip[1])[2] = 0; ((unsigned char *)&ip[1])[3] = 0;
#endif
	/*
	 * here the igmp packet is created
	 * also the ip checksum is generated
	 */
	igmp->type = igmp_type;
	igmp->code = igmp_code; // the value depends on igmp_type
	igmp->csum = 0;
	igmp->group = inet_addr(igmp_group); //specified multicast group addr

	igmp->csum = in_cksum((unsigned short *)igmp, sizeof(struct igmphdr));
	ip->check = in_cksum((unsigned short *)ip, sizeof(struct iphdr) + ipopt_size);

	/* set up destination address */
	memset(&addr, 0, sizeof(struct sockaddr_in));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = ip->daddr;

	/* now the packet is sent */
	sendto(fd, packet, ip->tot_len, 0, (struct sockaddr *)&addr, sizeof(struct sockaddr));

OUT:
	if (fd >= 0)
		close(fd);

#if USE_MALLOC
	if (packet != NULL)
		free(packet);
#endif

	return 0;
}

struct option longopts[] = {
	{"m_ip",	required_argument,	NULL, 'm'}, //required
	{"s_ip",	required_argument,	NULL, 's'}, //raw mode
	{"d_ip",	required_argument,	NULL, 'd'}, //raw mode
	{"igmp_type",	required_argument,	NULL, 't'}, //required
	{"igmp_code",	required_argument,	NULL, 'c'}, //raw mode
	{"raw",	no_argument,	NULL, 'r'}, //raw mode
	{NULL,		0,		NULL,  0}
};

int main(int argc, char *argv[])
{
	char *destination_ip = NULL;
	char *source_ip = NULL;
	char *multicast_ip = NULL;
	char dst_addr[15]; //ip.dst_addr
	char src_addr[15]; //ip.src_addr
	char igmp_group[15]; //multicast group address
	unsigned char igmp_type = IGMP_HOST_LEAVE_MESSAGE; //default is leave
	unsigned char igmp_code = 0;
	int raw_mode = 0;
	

	for (;;) {
		int c;
		int index = 0;

		c = getopt_long(argc, argv, "am:s:t:d:c:", longopts, &index);

		if (c == -1)
			break;

		switch (c) {
			case 'm':
				dbg("-m %s\n", optarg);
				multicast_ip = optarg;
				break;
			case 's':
				dbg("-s %s\n", optarg);
				source_ip = optarg;
				break;
			case 'd':
				dbg("-d %s\n", optarg);
				destination_ip = optarg;
				break;
			case 't':
				dbg("-t %s\n", optarg);
				igmp_type = (unsigned char)strtoul(optarg, NULL, 0);
				break;
			case 'c':
				dbg("-c %s\n", optarg);
				igmp_code = (unsigned char)strtoul(optarg, NULL, 0);
				break;
			case 'r':
				dbg("-a\n");
				raw_mode = 1;
				break;
			default:
				err("getopt error (%d)\n", c);
		}
	}
	if (multicast_ip == NULL) {
		strncpy(igmp_group, "0.0.0.0", 15);
	} else {
		strncpy(igmp_group, multicast_ip, 15);
	}
	if (destination_ip == NULL) {
		strncpy(dst_addr, igmp_group, 15);
	} else {
		strncpy(dst_addr, destination_ip, 15);
	}
	if (source_ip == NULL) {
		getip(src_addr);
	} else {
		strncpy(src_addr, source_ip, 15);
	}
	{
		dbg("multicast_ip = %s\n", dst_addr);
		dbg("source_ip = %s\n", src_addr);
	}

	if (!raw_mode) {
		switch(igmp_type) {
			case IGMP_HOST_MEMBERSHIP_QUERY: //0x11
				if (strncmp(igmp_group, "0.0.0.0", 7)) {
					//specific query
					igmp_code = 10; // 1 sec for specific query.
					strncpy(dst_addr, igmp_group, 15);
				} else {
					//general query
					igmp_code = 100; // 10 secs
					strncpy(dst_addr, "224.0.0.1", 15); //to all-hosts group
				}
				break;
			case IGMPV2_HOST_MEMBERSHIP_REPORT: //0x16
				igmp_code = 0;
				strncpy(dst_addr, igmp_group, 15);
				break;
			case IGMP_HOST_LEAVE_MESSAGE: //0x17
				igmp_code = 0;
				strncpy(dst_addr, "224.0.0.2", 15); //to all-routers multicast group
				break;
			default:
				err("un-supported IGMP packet type %d\n", igmp_type);
				break;
		}
	}

	igmpV2_send(dst_addr, src_addr, igmp_type, igmp_code, igmp_group);

	return 0;
}

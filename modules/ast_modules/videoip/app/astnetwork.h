/*
 * Copyright (c) 2004-2012
 * ASPEED Technology Inc. All Rights Reserved
 * Proprietary and Confidential
 *
 * By using this code you acknowledge that you have signed and accepted
 * the terms of the ASPEED SDK license agreement.
 */


#ifndef _ASTNETWORK_H_
#define _ASTNETWORK_H_

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#define VIDEOIP_V1TX_PORT 	3240
#define VIDEOIP_V2TX_PORT 	3241
#define VIDEOIP_VMTX_PORT 	3242
#define VIDEOIP_TX_PORT 	3243
#define VIDEOIP_RX_PORT 	3244
#define VIDEOIP_V1UDP_PORT 	3245

#define VIDEOIP_V1TX_PORT_STRING 	"3240"
#define VIDEOIP_V2TX_PORT_STRING 	"3241"
#define VIDEOIP_VMTX_PORT_STRING 	"3242"
#define VIDEOIP_TX_PORT_STRING 		"3243"
#define VIDEOIP_RX_PORT_STRING 		"3244"
#define VIDEOIP_V1UDP_PORT_STRING	"3245"

#define IP_TTL_DEFAULT 64
#define IP4_MULTICAST(x)    (((x) & htonl(0xf0000000)) == htonl(0xe0000000))

int SetReuseaddr(int sockfd);
int SetNodelay(int sockfd);
int SetKeepalive(int sockfd);
int set_sockopt_multicast_loop(int sockfd, unsigned char cfg);
int set_sockopt_ttl(int sockfd, unsigned char cfg);

int tcp_connect(char *hostname, char *service);
int udp_create_sender(char *dst_addr, int port);
int udp_connect(char *mgroup, int port);

int get_dst_ip(int skfd, unsigned int *ip_little_endian, unsigned short *port_little_endian);
int get_my_ip(int skfd, unsigned int *ip_little_endian, unsigned short *port_little_endian);
int unicast_dst_ip_to_mac(const unsigned int ip, unsigned char *mac);
int multicast_ip_to_mac(const unsigned int ip, unsigned char *mac);
int get_my_mac(int skfd, unsigned char *mac);
int get_mtu(int skfd, unsigned int *mtu);
int get_gateway(unsigned int *ip_little_endian);
#endif /* _ASTNETWORK_H_ */

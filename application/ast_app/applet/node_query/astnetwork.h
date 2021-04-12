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
#include <netdb.h>

int SetReuseaddr(int sockfd);
int SetNodelay(int sockfd);
int SetKeepalive(int sockfd);
int tcp_connect(char *hostname, char *service);
int tcp_connect_timeout(char *hostname, char *service, int secs);
int udp_create_sender(unsigned int ip_to_send_to, int port, unsigned int *binded_ip, unsigned int local_intf);
int udp_connect(unsigned int ip_to_listen, int port);
int udp_connect_as_server(unsigned int ip_to_listen, int port);

#endif /* _ASTNETWORK_H_ */

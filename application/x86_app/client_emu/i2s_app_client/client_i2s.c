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
//#include <asm/delay.h>
#include "ioaccess.h"
#include "2100def.h"

#define SIM 1

void TX_I2S (int transport, int ctrlconn, int dataconn)
{
    IO_ACCESS_DATA      IO_Data;
    int    fd = open ("/dev/i2s", O_RDONLY);

//    printf ("transport = %d\n", transport);
//    printf ("data socket = %d\n", dataconn);
//    printf ("client CTRL socket = %d\n", ctrlconn);
//	memset(&IO_Data, 0, sizeof(IO_ACCESS_DATA));
    IO_Data.transport = transport;
    IO_Data.CtrlSocket = ctrlconn;
    IO_Data.I2sSocket = dataconn;
    ioctl(fd, IOCTL_I2S_TX, &IO_Data);
    close(fd);    
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
			perror("setsockopt (IP_ADD_MEMBERSHIP)");
			close(fd);
			return -1;
		}
	}

	return fd;
}

static int tcp_create_client(char *host_addr, int port)
{
	int sockfd;
    int    sndbuf = 0x100000;
	struct sockaddr_in addr_svr;
	int ret = 0;

	printf("tcp_create_client (%s,%d)\n", host_addr, port);	
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd == -1){
		perror("socket\n");
		ret = -1;
		goto done;
    }
	
    ret = setsockopt(sockfd, SOL_SOCKET, SO_SNDBUF, &sndbuf, 0x100000);
	if (ret != 0)
	{
		perror("setsockopt (SO_SNDBUF)\n");
		goto done;
	}
	
    memset(&addr_svr, 0, sizeof(addr_svr));
    addr_svr.sin_family = AF_INET;
    addr_svr.sin_port= htons(port);
    addr_svr.sin_addr.s_addr = inet_addr(host_addr);
    ret = connect(sockfd, (struct sockaddr *)&addr_svr, sizeof(addr_svr));
    if (ret == -1) {
		perror("connect\n");
		goto done;
    }
    printf ("Connected\n");
	
done:
	if (ret == 0)
		return sockfd;
	else
	{
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

int  main (int argc, char *argv[])
{
//	struct sockaddr_in addr_svr;
//    int    temp = 0, test;
//    IO_ACCESS_DATA      IO_Data;
	int ctrlconnfd = -1, dataconnfd = -1;
	int transport = 0;//0:UDP; 1:TCP
	char *mc_addr = NULL;
	char *host_addr = NULL;
	int ret = 0;
	int no_tcp = 0;

	for (;;) {
		int c;

		c = getopt_long(argc, argv, short_opts, long_opts, NULL);

		if (c == -1)
			break;

		switch (c)
		{
			case 't':
				if (strncmp(optarg, "tcp", 3) == 0)//TCP only
				{
					printf("TCP transport\n");
					transport = 1;
				}
				else if (strncmp(optarg, "no_tcp", 6) == 0)//UDP only
				{
					printf("No TCP transport\n");
					no_tcp = 1;
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
				//to do:print help manual
				goto done;
		}
	}
	if ((mc_addr) && (transport))
	{
		printf("multicast only in UDP\n");
		transport = 0;
	}

	if (!mc_addr || !no_tcp) {
		ctrlconnfd = tcp_create_client(host_addr, CTRL_PORT);
		if (ctrlconnfd < 0)
		{
			printf("failed to create CTRL client\n");
			ret = -1;
			goto done;
		}
	}
	if (mc_addr)
	{//multicast
		dataconnfd = udp_create_receiver(mc_addr, I2S_PORT);
		if (dataconnfd < 0)
		{
			printf("failed to create multicast data receiver\n");
			ret = -2;
			goto done;
		}
	}
	else
	{//unicast
		if (transport)
		{//TCP
			dataconnfd = tcp_create_client(host_addr, I2S_PORT);
			if (dataconnfd < 0)
			{
				printf("failed to create TCP data client\n");
				ret = -2;
				goto done;
			}
		}
		else
		{//UDP
			dataconnfd = udp_create_receiver(NULL, I2S_PORT);
			if (dataconnfd < 0)
			{
				printf("failed to create unicast data receiver\n");
				ret = -2;
				goto done;
			}
		}
	}
#ifndef SIM
	TX_I2S(transport, ctrlconnfd, dataconnfd);
#else
	for (;;) {
		sleep(60);
	}
#endif

done:
//	if (ret != 0)
//	{
//		if (dataconnfd >= 0)
//			close(dataconnfd);
//		if (ctrlconnfd >= 0)
//			close(ctrlconnfd);
//	}
    return ret;
}

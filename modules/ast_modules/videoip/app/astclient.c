/*
 * Copyright (c) 2004-2012
 * ASPEED Technology Inc. All Rights Reserved
 * Proprietary and Confidential
 *
 * By using this code you acknowledge that you have signed and accepted
 * the terms of the ASPEED SDK license agreement.
 */

#include <unistd.h>
#include <netdb.h>
#include <strings.h>
#include <stdlib.h>
#include <syslog.h>
#include <stdio.h>
#include <string.h> /* string.h */
#include <sys/types.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#include <getopt.h>
#include <signal.h>

#include "vdef.h"
#include "getinf.h"
#include "videoip_ioctl.h"
#include "astcommon.h"

#define ASTHOST_GLOBALS
#include "global.h"
#include "astdebug.h"
#include "astnetwork.h"

static const char version[] = 
	" ($Id: astclient.c 13 2007-05-27 16:17:59Z southstar $)";

static char *soc_version = NULL;

char *mc_addr = NULL;
int no_tcp = 0;

static void show_help(void)
{

}

static void signal_handler(int i)
{
	dbg("signal catched, code %d", i);
}

static void set_signal(void)
{
	struct sigaction act;

	bzero(&act, sizeof(act));
	act.sa_handler = signal_handler;
	sigemptyset(&act.sa_mask);
	sigaction(SIGTERM, &act, NULL);
	sigaction(SIGINT, &act, NULL);
}

static void UseVideo1(char *host)
{
	int sockfd;
	int	Sendbytes;
	int VideoOP;
	int	devfd;

	IOCTL_CLIENT_INFO ClientInfo;	

	info("Enter Use Video1");

	devfd = open ("/dev/videoip", O_RDONLY);
	if (devfd < 0)
	{
		err("open device");
		return;
	}

	set_signal();

	sockfd = tcp_connect(host, VIDEOIP_RX_PORT_STRING);
	if (sockfd < 0) {
		err("tcp1 connect");
		return;
	}
	ClientInfo.CmdSockFd = sockfd;

	VideoOP = OP_USE_ONLY_V1;
	if ((Sendbytes = send(sockfd, &VideoOP, sizeof(int), 0)) == -1)
	{
		err("Send Video OP");
		exit(1);
	}

	ClientInfo.V1TxSockFd = -1;
	ClientInfo.V1McSockFd = -1;

	if (mc_addr) {
		if (!strncmp(mc_addr, "udp", 3)) {
			/*
			** Create udp socket and assigned the sockfd to ClientInfo.V1McSockFd.
			*/
			sockfd = udp_connect(NULL, VIDEOIP_V1UDP_PORT);
			if (sockfd < 0) {
				err("udp socket connect fail\n");
				return;
			}
			ClientInfo.V1McSockFd = sockfd;
		} else {
			sockfd = udp_connect(mc_addr, VIDEOIP_V1UDP_PORT);
			if (sockfd < 0) {
				err("multicast socket connect fail\n");
				return;
			}
			ClientInfo.V1McSockFd = sockfd;
		}
	} else {
		/* This is the code path of TCP. */
		sockfd = tcp_connect(host, VIDEOIP_V1TX_PORT_STRING);
		if (sockfd < 0) {
			err("tcp1 connect");
			return;
		}
		ClientInfo.V1TxSockFd = sockfd;
	}

	LoadInf(&ClientInfo.e_cfg);
	ioctl (devfd, IOCTL_CLIENT_ONLY_V1, &ClientInfo);

	close (devfd);
	pause(); //suspend process until signal
	info("astclient shutdown");

	if (ClientInfo.V1TxSockFd != -1)
		close(ClientInfo.V1TxSockFd);
	if (ClientInfo.V1McSockFd != -1)
		close(ClientInfo.V1McSockFd);
	close(ClientInfo.CmdSockFd);

	return;
}

static void fill_hw_udp_header(PIOCTL_CLIENT_INFO_V2 pClientInfo)
{
	struct socket_info_t *skinfo = &pClientInfo->skinfo;
	unsigned int d;

	memset(skinfo, 0, sizeof(struct socket_info_t));

	switch (pClientInfo->CastingMode) {
	case 0: // unicast. TCP ctrl + TCP data
		// HW UDP can't support this mode. Do nothing
		break;
	case 1: // unicast. TCP ctrl + UDP data
		/*
		** Local IP address only valid when a socket is explicitly
		** 'connect()' to a host. Binding to 'any local address'
		** result in 0.0.0.0 local IP get from getsockname().
		** As a result, we call get_my_ip() twice with different 
		** socket fd. One to get IP, the other to get port number.
		*/
		get_my_ip(pClientInfo->CtrlSockFd, &skinfo->ip_daddr, (unsigned short *)&d);
		get_my_ip(pClientInfo->DataSockFd, &d, &skinfo->udp_dest);
		get_mtu(pClientInfo->DataSockFd, &skinfo->mtu);
		break;
	case 2: // multicast. TCP ctrl + UDP (MC) data
		skinfo->ip_daddr = (unsigned int)ntohl(inet_addr(mc_addr));
		get_my_ip(pClientInfo->DataSockFd, &d, &skinfo->udp_dest);
		get_mtu(pClientInfo->DataSockFd, &skinfo->mtu);
		break;
	case 3: // multicast. UDP (MC) ctrl + UDP (MC) data
		skinfo->ip_daddr = (unsigned int)ntohl(inet_addr(mc_addr));
		get_my_ip(pClientInfo->DataSockFd, &d, &skinfo->udp_dest);
		get_mtu(pClientInfo->DataSockFd, &skinfo->mtu);
		break;
	default:
		break;
	};

	printf("\n\n\n\n================\n");
	printf("HW UDP Socket Info:\n");
	printf("MTU: %d\n", skinfo->mtu);
	printf("Dst IP addr: 0x%08X\n", skinfo->ip_daddr);
	printf("Dst UDP Port: 0x%04X\n", skinfo->udp_dest);
	printf("Dst MAC: %02X:%02X:%02X:%02X:%02X:%02X\n",
		skinfo->mac_dest[0], skinfo->mac_dest[1], skinfo->mac_dest[2], skinfo->mac_dest[3], skinfo->mac_dest[4], skinfo->mac_dest[5]);
	printf("Src MAC: %02X:%02X:%02X:%02X:%02X:%02X\n",
		skinfo->mac_source[0], skinfo->mac_source[1], skinfo->mac_source[2], skinfo->mac_source[3], skinfo->mac_source[4], skinfo->mac_source[5]);
	printf("Src IP addr: 0x%08X\n", skinfo->ip_saddr);
	printf("Src UDP Port: 0x%04X\n", skinfo->udp_source);
	printf("================\n");
}

static void UseVideo1_V2(char *host)
{
	int devfd, sockfd;
	int is_multicast = 0;

	IOCTL_CLIENT_INFO_V2 ClientInfo;	

	info("Enter Use Video1");

	devfd = open("/dev/videoip", O_RDONLY);

	if (devfd < 0) {
		err("open device");
		return;
	}

	set_signal();

	/* If multicast address is asigned and is not 'udp' */
	if ((mc_addr) && (strncmp(mc_addr, "udp", 3)))
		is_multicast = 1;


	if (is_multicast) {
		sockfd = udp_create_sender(mc_addr, VIDEOIP_RX_PORT);
		if (sockfd == -1) {
			err("multicast socket for ctrl connect fail\n");
			return;
		}
	} else {
		sockfd = tcp_connect(host, VIDEOIP_RX_PORT_STRING);
		if (sockfd < 0) {
			err("tcp1 connect");
			return;
		}
		close(sockfd);
		sockfd = udp_create_sender(host, VIDEOIP_RX_PORT);
		if (sockfd == -1) {
			err("udp socket for ctrl connect fail\n");
			return;
		}
	}

	ClientInfo.CtrlSockFd = sockfd;

	if (is_multicast) {
		sockfd = udp_connect(mc_addr, VIDEOIP_V1UDP_PORT);
		if (sockfd < 0) {
			err("multicast socket connect fail\n");
			return;
		}

		if (no_tcp)//UDP-only multicast
			ClientInfo.CastingMode = 3;
		else
			ClientInfo.CastingMode = 2;
	} else {
		if (!strncmp(mc_addr, "udp", 3)) { /* UDP */
			/*
			** Create udp socket and assigned the sockfd to ClientInfo.V1McSockFd.
			*/
			sockfd = udp_connect(NULL, VIDEOIP_V1UDP_PORT);
			if (sockfd < 0) {
				err("udp socket connect fail\n");
				return;
			}
			ClientInfo.CastingMode = 1;
		} else {
			sockfd = tcp_connect(host, VIDEOIP_V1TX_PORT_STRING);
			if (sockfd < 0) {
				err("tcp1 connect");
				return;
			}
			ClientInfo.CastingMode = 0;
		}
	}
	ClientInfo.DataSockFd = sockfd;

	fill_hw_udp_header(&ClientInfo);
	ioctl(devfd, IOCTL_CLIENT_ONLY_V2, &ClientInfo);

	close(devfd);
	//Bruce120816. Sockets are owned by kernel at this point. They will still
	//be available even though we close them here. So, I see no need to pause
	//astclient here. We terminate this process normally. The socket will still
	//be vaild and will be closed by kernel driver eventually.
	//pause(); //suspend process until signal
	//info("astclient shutdown");

//	if (ClientInfo.V1TxSockFd != -1 && ClientInfo.V1TxSockFd != 0)
	close(ClientInfo.DataSockFd);
	close(ClientInfo.CtrlSockFd);

	return;
}


static const struct option longopts[] = {
	{"soc_ver",	required_argument,	NULL, 's'},
	{"UseVideo1",	no_argument,	NULL, '0'},
	{"debug",	no_argument,	NULL, 'd'},
	{"version",	no_argument,	NULL, 'v'},
	{"help",	no_argument,	NULL, 'h'},
	{"multicast",	required_argument,	NULL, 'm'},
	{"transport",	required_argument,	NULL, 't'},
	{NULL,		0,		NULL,  0}
};

int main(int argc, char *argv[])
{
	int daemonize = 0;
	char *host_ip = NULL;

	enum {
		cmd_help = 1,
		cmd_version,
		cmd_OnlyV1,
		cmd_OnlyV2,
		cmd_CRT1,
		cmd_CRT2		
	} cmd = cmd_help;

	videoip_use_stderr = 1;
	videoip_use_syslog = 0;

	for (;;) {
		int c;
		int index = 0;

		c = getopt_long(argc, argv, "s:vhd0:m:t:", longopts, &index);

		if (c == -1)
			break;

		switch (c) {
		case 's':
			soc_version = optarg;
			break;
		case 'd':
			videoip_use_debug = 1;
			continue;
		case 'v':
			cmd = cmd_version;
			break;
		case 'h':
			cmd = cmd_help;
			break;
		case '0':
			cmd = cmd_OnlyV1;
			host_ip = optarg;
			break;
		case 'm':
			mc_addr = optarg;
			break;
		case 't':
			if (strncmp(optarg, "no_tcp", 6) == 0) {
				printf("No TCP transport\n");
				no_tcp = 1;
			}
			break;
		case '?':
			break;
		default:
			err("getopt");
		}
	}

	switch (cmd) {
	case cmd_version:
		printf("%s\n", version);
		break;
	case cmd_help:
		show_help();
		break;
	case cmd_OnlyV1:
		if (!strncmp(soc_version, "V1", 2)) {
			info("SOC V1");
			UseVideo1(host_ip);
		} else if (!strncmp(soc_version, "V2", 2)) {
			info("SOC V2");
			UseVideo1_V2(host_ip);
		} else
			err("unsupported SOC version: %s\n", soc_version);
		break;
	default:
		info("unknown cmd");
		show_help();
	}

	return 0;
}

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
#include <string.h>
#include <stdlib.h>
#include <syslog.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <errno.h>

#include <getopt.h>
#include <signal.h>

#include "vdef.h"
#include "getinf.h"
#include "videoip_ioctl.h"

#define ASTHOST_GLOBALS
#include "global.h"
#include "astdebug.h"
#include "astnetwork.h"

void ast_send_event(const char *event);

static const char version[] = 
	" ($Id: asthost.c 13 2007-05-27 16:17:59Z southstar $)";

char *mc_addr = NULL;
struct sockaddr_storage ss;
int no_tcp = 0;

static void show_help(void)
{

}

static struct addrinfo *my_getaddrinfo(char *host, char *port)
{
	int ret;
	struct addrinfo hints, *ai_head;

	bzero(&hints, sizeof(hints));

	hints.ai_family   = PF_UNSPEC; /* PF_UNSPEC stands for using IPV6 or IPV4 by system */
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags    = AI_PASSIVE; //for use in binding a socket for accepting incoming connections

	ret = getaddrinfo(host, port, &hints, &ai_head);
	if (ret) {
		err("%s: getaddrinfo is error", port);
		return NULL;
	}

	return ai_head;
}

static void log_addrinfo(struct addrinfo *ai)
{
	int ret;
	char hbuf[NI_MAXHOST];
	char sbuf[NI_MAXSERV];

	ret = getnameinfo(ai->ai_addr, ai->ai_addrlen, hbuf, sizeof(hbuf),
			sbuf, sizeof(sbuf), NI_NUMERICHOST | NI_NUMERICSERV);
	if (ret)
		err("getnameinfo is error");

	info("listen at [%s]:%s", hbuf, sbuf);
}

#define MAXSOCK 20
int max_fd = 20; /* the highest file descriptor (for select()) */

static int listen_all_addrinfo(struct addrinfo *ai_head, int lsock[])
{
	struct addrinfo *ai;
	int n = 0;		/* number of sockets */

	for (ai = ai_head; ai && n < MAXSOCK; ai = ai->ai_next) {
		int ret;

		lsock[n] = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);
		if (lsock[n] < 0)
			continue;

		SetReuseaddr(lsock[n]);
		SetNodelay(lsock[n]);

		if (lsock[n] >= FD_SETSIZE) {
			close(lsock[n]);
			lsock[n] = -1;
			continue;
		}

		ret = bind(lsock[n], ai->ai_addr, ai->ai_addrlen);
		if (ret < 0) {
			close(lsock[n]);
			lsock[n] = -1;
			continue;
		}

		ret = listen(lsock[n], SOMAXCONN);
		if (ret < 0) {
			close(lsock[n]);
			lsock[n] = -1;
			continue;
		}

		log_addrinfo(ai);

		/* next if succeed */
		n++;
	}

	if (n == 0) {
		err("no socket to listen to");
		return -1;
	}

	dbg("listen %d address%s", n, (n==1)?"":"es");

	return n;
}
	
static const struct option longopts[] = {
	{"daemon",	no_argument,	NULL, 'D'},
	{"debug",	no_argument,	NULL, 'd'},
	{"version",	no_argument,	NULL, 'v'},
	{"help",	no_argument,	NULL, 'h'},
	{"multicast",	required_argument,	NULL, 'm'},
	{"transport",	required_argument,	NULL, 't'},
	{NULL,		0,		NULL,  0}
};

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

static int my_accept_sa(int lsock)
{
	int csock;
	//struct sockaddr_storage ss;
	socklen_t len = sizeof(ss);
	char host[NI_MAXHOST], port[NI_MAXSERV];
	int ret;

	bzero(&ss, sizeof(ss));

	csock = accept(lsock, (struct sockaddr *) &ss, &len);
	if (csock < 0) {
		err("accept");
		return -1;
	}

	ret = getnameinfo((struct sockaddr *) &ss, len,
			host, sizeof(host), port, sizeof(port),
			(NI_NUMERICHOST | NI_NUMERICSERV));
	if (ret)
		err("getnameinfo, %s", gai_strerror(ret));

	info("connected from %s:%s", host, port);

	return csock;
}

static int my_accept(int lsock)
{
	int csock;
	struct sockaddr_storage ss;
	socklen_t len = sizeof(ss);
	char host[NI_MAXHOST], port[NI_MAXSERV];
	int ret;

	bzero(&ss, sizeof(ss));

	csock = accept(lsock, (struct sockaddr *) &ss, &len);
	if (csock < 0) {
		err("accept");
		return -1;
	}

	ret = getnameinfo((struct sockaddr *) &ss, len,
			host, sizeof(host), port, sizeof(port),
			(NI_NUMERICHOST | NI_NUMERICSERV));
	if (ret)
		err("getnameinfo, %s", gai_strerror(ret));

	info("connected from %s:%s", host, port);

	return csock;
}

static void fill_hw_udp_header(PIOCTL_REMOTE_INFO pHostInfo)
{
	struct socket_info_t *skinfo = &pHostInfo->skinfo;

	memset(skinfo, 0, sizeof(struct socket_info_t));

	switch (pHostInfo->CastingMode) {
	case 0: // unicast. TCP ctrl + TCP data
		// HW UDP can't support this mode. Do nothing
		break;
	case 1: // unicast. TCP ctrl + UDP data
		get_dst_ip(pHostInfo->DataSockFd, &skinfo->ip_daddr, &skinfo->udp_dest);
		get_my_ip(pHostInfo->DataSockFd, &skinfo->ip_saddr, &skinfo->udp_source);
		if (unicast_dst_ip_to_mac(htonl(skinfo->ip_daddr), skinfo->mac_dest) == -ENXIO) {
			/*
			 * ENXIO - No such device or address
			 * not the same subnet, use gatway's MAC instead
			 */
			unsigned int ip = 0;

			get_gateway(&ip);
			unicast_dst_ip_to_mac(htonl(ip), skinfo->mac_dest);
		}
		get_my_mac(pHostInfo->DataSockFd, skinfo->mac_source);
		get_mtu(pHostInfo->DataSockFd, &skinfo->mtu);
		break;
	case 2: // multicast. TCP ctrl + UDP (MC) data
		get_dst_ip(pHostInfo->DataSockFd, &skinfo->ip_daddr, &skinfo->udp_dest);
		get_my_ip(pHostInfo->DataSockFd, &skinfo->ip_saddr, &skinfo->udp_source);
		multicast_ip_to_mac(htonl(skinfo->ip_daddr), skinfo->mac_dest);
		get_my_mac(pHostInfo->DataSockFd, skinfo->mac_source);
		get_mtu(pHostInfo->DataSockFd, &skinfo->mtu);
		break;
	case 3: // multicast. UDP (MC) ctrl + UDP (MC) data
		get_dst_ip(pHostInfo->DataSockFd, &skinfo->ip_daddr, &skinfo->udp_dest);
		get_my_ip(pHostInfo->DataSockFd, &skinfo->ip_saddr, &skinfo->udp_source);
		multicast_ip_to_mac(htonl(skinfo->ip_daddr), skinfo->mac_dest);
		get_my_mac(pHostInfo->DataSockFd, skinfo->mac_source);
		get_mtu(pHostInfo->DataSockFd, &skinfo->mtu);
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

static int video_socket_listen(char *port, int lsock[])
{
	struct addrinfo *ai_head;
	int ret = 0;

	ai_head = my_getaddrinfo(NULL, port);

	if (ai_head == NULL)
		goto exit;

	ret = listen_all_addrinfo(ai_head, lsock);
	freeaddrinfo(ai_head);

	if (ret != 1) {
		if (ret <= 0)
			err("port %s no socket to listen to", port);
		else
			err("port %s listion sock > 1", port);
	}

exit:
	return ret;
}

static void video_host(char *mac_addr)
{
	int is_multicast = 0;
	IOCTL_REMOTE_INFO HostInfo;
	int devfd;
	int DataSockFd = -1, CtrlSockFd;
	fd_set fds;
	int ctrl_n;
	int ctrl_lsock[MAXSOCK], data_socket_tcp;
	int i, ret;

	memset(&HostInfo, 0, sizeof(IOCTL_REMOTE_INFO));

	/* If multicast address is asigned and is not 'udp' */
	if ((mc_addr) && (strncmp(mc_addr, "udp", 3)))
		is_multicast = 1;

	/* data socket */
	if (is_multicast) {
		DataSockFd = udp_create_sender(mc_addr, VIDEOIP_V1UDP_PORT);

		if (DataSockFd == -1)
			exit(1);

		HostInfo.DataSockFd = DataSockFd;
		HostInfo.CastingMode = 2; /* multicast */

		if (no_tcp) {
			/* UDP-only multicast */
			HostInfo.CastingMode = 3;
		}

		devfd = open("/dev/videoip", O_RDONLY);
		fill_hw_udp_header(&HostInfo);
		ioctl(devfd, IOCTL_HOST_ENABLE_REMOTE, &HostInfo);
		close(devfd);

		if (close(DataSockFd))
			err("Failed to close data socket (%d)\n", errno);
	} else {
		/* -m udp, UDP unicast */
		HostInfo.CastingMode = 1;

		if (!mc_addr) {
			/* FIXME obsolete */
			int data_n;
			int data_lsock[MAXSOCK];

			data_n = video_socket_listen(VIDEOIP_V1TX_PORT_STRING, data_lsock);

			if (data_n != 1)
				exit(1);

			data_socket_tcp = data_lsock[0];
			/* TCP-only unicast */
			HostInfo.CastingMode = 0;
		}
	}

	/* control socket */
	if (is_multicast) {
		/* add the virtual client, from which host receives CMDs in UDP-only mode */
		CtrlSockFd = udp_connect(mc_addr, VIDEOIP_RX_PORT);
		if (CtrlSockFd < 0) {
			err("multicast socket for ctrl connect fail\n");
			exit(1);
		}
		HostInfo.CtrlSockFd = CtrlSockFd;
		devfd = open("/dev/videoip", O_RDONLY);
		ioctl(devfd, IOCTL_HOST_ADD_CLIENT, &HostInfo);
		close(devfd);
		//A7 removed
		//ast_send_event("e_videoip_init_ok");
		return;
	}

	FD_ZERO(&fds);

	/* Rx socket */
	ctrl_n = video_socket_listen(VIDEOIP_RX_PORT_STRING, ctrl_lsock);

	if (ctrl_n <= 0) {
		err("no socket to listen to");
		exit(1);
	}

	info("Rx sock num [%d]", ctrl_n);

	for (i = 0; i < ctrl_n; i++)
		FD_SET(ctrl_lsock[i], &fds);

	//A7 removed
	//ast_send_event("e_videoip_init_ok");

	while (1) {
		info("\n\nwaiting for connections");
		info("-----------------------------------------------------------");
		ret = select(max_fd + 1, &fds, NULL, NULL, NULL);
		if (ret < 0) {
			if (errno != EINTR)
				err("select returned funny value");

			info("bye...");
			exit(1);
		}

		for (i = 0; i < ctrl_n; i++) {
			if (FD_ISSET(ctrl_lsock[i], &fds)) {
				CtrlSockFd = my_accept_sa(ctrl_lsock[i]);
				if (CtrlSockFd < 0) {
					err("accept()-ing failed");
					continue;
				}
				close(CtrlSockFd);

				CtrlSockFd = udp_connect(NULL, VIDEOIP_RX_PORT);
				if (CtrlSockFd < 0) {
					err("multicast socket for ctrl connect fail\n");
					continue;
				}
				HostInfo.CtrlSockFd = CtrlSockFd;

				devfd = open("/dev/videoip", O_RDONLY);

				if (!mc_addr) {
					/* TCP-only unicast */
					DataSockFd = my_accept(data_socket_tcp);
					if (DataSockFd < 0)
						err("V1Tx accept()-ing failed");
				} else if (!strncmp(mc_addr, "udp", 3)) {
					/* unicast */
					DataSockFd = udp_create_sender(inet_ntoa(((struct sockaddr_in *)(&ss))->sin_addr), VIDEOIP_V1UDP_PORT);
				}
				HostInfo.DataSockFd = DataSockFd;

				if (DataSockFd != -1) {
					fill_hw_udp_header(&HostInfo);
					ioctl(devfd, IOCTL_HOST_ENABLE_REMOTE, &HostInfo);

					if (close(DataSockFd))
						err("Failed to close data socket (%d)\n", errno);
				}

				ioctl(devfd, IOCTL_HOST_ADD_CLIENT, &HostInfo);
				close(devfd);

				if (close(CtrlSockFd))
					err("Failed to close control socket fd (%d)\n", errno);
			}
		}
	}
}

static void do_standalone_mode(int daemonize)
{
	if (daemonize) {
		if (daemon(0,0) < 0)
			err("daemonizing failed");

		videoip_use_syslog = 1;
	}

	set_signal();

	video_host(mc_addr);

	return;
}

int main(int argc, char *argv[])
{
	int daemonize = 0;

	enum {
		cmd_standalone_mode = 1,
		cmd_help,
		cmd_version
	} cmd = cmd_standalone_mode;

	videoip_use_stderr = 1;
	videoip_use_syslog = 0;

	for (;;) {
		int c;
		int index = 0;

		c = getopt_long(argc, argv, "vhdDm:t:", longopts, &index);

		if (c == -1)
			break;

		switch (c) {
		case 'd':
			videoip_use_debug = 1;
			continue;
		case 'v':
			cmd = cmd_version;
			break;
		case 'h':
			cmd = cmd_help;
			break;
		case 'D':
			daemonize = 1;
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
	case cmd_standalone_mode:
		do_standalone_mode(daemonize);
		break;
	case cmd_version:
		printf("%s\n", version);
		break;
	case cmd_help:
		show_help();
		break;
	default:
		info("unknown cmd");
		show_help();
	}

	return 0;
}

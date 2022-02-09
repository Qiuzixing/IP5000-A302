/*
 * Copyright (c) 2004-2012
 * ASPEED Technology Inc. All Rights Reserved
 * Proprietary and Confidential
 *
 * By using this code you acknowledge that you have signed and accepted
 * the terms of the ASPEED SDK license agreement.
 */


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include <getopt.h>
#include <signal.h>
#include <errno.h>

// For send_event
#include <sys/poll.h>
#include <sys/types.h>
#include <unistd.h>
#include <linux/types.h>
#include <linux/netlink.h>
#define MAX_PAYLOAD 1024  /* maximum payload size*/


#include "astnetwork.h"
#include "debug.h"


#define HEARTBEAT_PORT 6755
#define HEARTBEAT_PORT_STR "6755"
static int heartbeatInt = 500;//msec
static int heartbeatTimeout = 8000;



static const struct option longopts[] = {
	{"host",	no_argument,	NULL, 'h'},
	{"client",	no_argument,	NULL, 'c'},
	{"gw_ip", required_argument,	NULL, 'd'},
	{"multicast", no_argument,	NULL, 'm'},
	{NULL,		0,		NULL,  0}
};

void send_event(unsigned int dest_pid, char *event_msg)
{
	struct msghdr msg_hdr;

	struct sockaddr_nl src_addr, dest_addr;
	struct nlmsghdr *nlh = NULL;
	struct iovec iov;
	int sock_fd;

	sock_fd = socket(PF_NETLINK, SOCK_RAW, NETLINK_USERSOCK);
	if (sock_fd == -1) {
		err("socket()) failed\n");
		goto err_out;
	}

	memset(&src_addr, 0, sizeof(src_addr));
	src_addr.nl_family = AF_NETLINK;
	src_addr.nl_pid = getpid();  /* self pid */
	/* interested in group 1<<0 */
	src_addr.nl_groups = -1;
	if (bind(sock_fd, (struct sockaddr*)&src_addr, sizeof(src_addr))) {
		err("bind failed\n");
		goto err_out;
	}
	

	memset(&dest_addr, 0, sizeof(dest_addr));
	dest_addr.nl_family = AF_NETLINK;
	dest_addr.nl_pid = dest_pid;   /* 0 for Linux Kernel */
	dest_addr.nl_groups = 1; /* multicast */

	nlh=(struct nlmsghdr *)malloc(NLMSG_SPACE(MAX_PAYLOAD));
	if (!nlh) {
		err("failed to allocate nlh\n");
		goto err_out;
	}

	/* Fill the netlink message header */
	memset(nlh, 0, sizeof(struct nlmsghdr));
	nlh->nlmsg_len = NLMSG_SPACE(MAX_PAYLOAD);
	nlh->nlmsg_pid = src_addr.nl_pid;  /* self pid */
	nlh->nlmsg_flags = 0;
	/* Fill in the netlink message payload */
	//!! REMEMBER to ADD ONE SPACE after the event_msg
	sprintf(NLMSG_DATA(nlh), "%s", event_msg);
	//strcpy(NLMSG_DATA(nlh), event_msg);

	memset(&iov, 0, sizeof(struct iovec));
	memset(&msg_hdr, 0, sizeof(struct msghdr));
	iov.iov_base = (void *)nlh;
	iov.iov_len = nlh->nlmsg_len;
	msg_hdr.msg_name = (void *)&dest_addr;
	msg_hdr.msg_namelen = sizeof(dest_addr);
	msg_hdr.msg_iov = &iov;
	msg_hdr.msg_iovlen = 1;

	sendmsg(sock_fd, &msg_hdr, 0);
	
err_out:
	if (sock_fd >= 0)
		close(sock_fd);

	if (nlh)
		free(nlh);
}


static int my_accept(int lsock, char *host)
{
	int csock;
	struct sockaddr_storage ss;
	socklen_t len = sizeof(ss);
	char port[NI_MAXSERV];
	int ret;

	bzero(&ss, sizeof(ss));

	csock = accept(lsock, (struct sockaddr *) &ss, &len);
	if (csock < 0) {
		err("accept\n");
		return -1;
	}

	ret = getnameinfo((struct sockaddr *) &ss, len,
			host, NI_MAXHOST, port, sizeof(port),
			(NI_NUMERICHOST | NI_NUMERICSERV));
	if (ret)
		err("getnameinfo, %s\n", gai_strerror(ret));

	info("connected from %s:%s\n", host, port);

	return csock;
}



static void do_heartbeat_host(void)
{
	char buf[1];
	int csock = -1, lsock = -1;
	struct timeval timeout;
	char host_n[NI_MAXHOST];
	fd_set fds;
	struct sockaddr_in gw_addr;
	int ret = -1;

	lsock = socket(AF_INET, SOCK_STREAM, 0);
	if (lsock == -1) {
		err("[vhub] error socket\n");
		goto done;
	}
	SetReuseaddr(lsock);
	SetNodelay(lsock);

	gw_addr.sin_family = AF_INET;
	gw_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	gw_addr.sin_port = htons(HEARTBEAT_PORT);
	ret = bind(lsock, (struct sockaddr *)&gw_addr, sizeof(gw_addr));
	if (ret != 0) {
		err("[vhub]error bind\n");
		goto done;
	}
	ret = listen(lsock, 1);
	if (ret != 0) {
		err("[vhub]error listen\n");
		goto done;
	}
	//send_event(-1, "e_heartbeat_init_ok");
	while (1) {
		csock = my_accept(lsock, host_n);
		if(csock < 0) {
			err("[vhub]error accept\n");
			continue;
		}
		break;
	}
	/* lsock MUST be close, otherwise the other client can successfully conntect to it! */
	close(lsock);
	lsock = -1;
	info("vhub heartbeat socket connected\n");
	//send_event(-1, "e_attaching");
	timeout.tv_usec = 0;
	while (1) {
		FD_ZERO(&fds);
		FD_SET(csock, &fds);
		timeout.tv_sec = heartbeatTimeout / 1000;
		ret = select(FD_SETSIZE + 1, &fds, NULL, NULL, &timeout);
		if (ret < 0) {
			err("[vhub]error select\n");
			//to do
		} else if (ret == 0) {
			info("vhub heartbeat timeouted\n");
			break;
		} else {
			ret = recv(csock, buf, 1, 0);
			if (ret < 0) {
				err("[vhub]error recv\n");
				//to do
			} else if (ret == 0) {
				info("[vhub]client disconnected\n");
				break;
			} else {
//					info("recv heartbeat from client\n");
				send(csock, buf, 1, 0);
			}
		}
	}

done:
	if (csock >= 0)
		close(csock);
	if (lsock >= 0)
		close(lsock);

	send_event(-1, "e_no_vhub_heartbeat");

	exit (ret);
}

static int send_hb(int sockfd, unsigned char *buf, int buf_size)
{
	int retry_cnt = 3;

	while (retry_cnt--) {
		if (send(sockfd, buf, buf_size, MSG_DONTWAIT|MSG_NOSIGNAL) < 0) {
			if (errno == EAGAIN) {
				info("Send heartbeat failed! Socket buf full?!\n");
				usleep(1000*500);
				continue;
			}
			info("Send heartbeat failed! disconnected?!\n");
			return -1;
		}
	}
	if (retry_cnt)
		return 0;
	// Send heartbeat failed
	return -1;
}

static void do_heartbeat_client(char *host)
{
	int sockfd = -1;
	char buf[1];
	struct timeval timeout;
	fd_set	fds;
	int ret = -1;
	int is_connected = 0;
	
	sockfd = tcp_connect(host, HEARTBEAT_PORT_STR);
	if (sockfd == -1) {
		err("error connect vhub heartbeat socket\n");
		goto done;
	}
	info("vhub heartbeat socket connected\n");
	//send_event(-1, "e_vhub_heartbeat_init_ok");
	
	timeout.tv_usec = 0;
	timeout.tv_sec = heartbeatTimeout / 1000;
	while (1) {
		if (send_hb(sockfd, buf, 1)) {
			info("Send heartbeat failed!!\n");
			break;
		}
		FD_ZERO(&fds);
		FD_SET(sockfd, &fds);
		timeout.tv_sec = heartbeatTimeout / 1000;
		ret = select(FD_SETSIZE + 1, &fds, NULL, NULL, &timeout);
		if (ret < 0) {
			err("!!!!![vhub]error select\n");
			//to do
		} else if (ret == 0) {
			info("!!!!!receive vhub heartbeat timeouted\n");
			break;
		} else {
			ret = recv(sockfd, buf, 1, 0);
			if (ret < 0) {
				err("[vhub]error recv\n");
				//to do
			} else if (ret == 0) {
				info("[vhub]gateway disconnected\n");
				break;
			} else {
//					info("recv heartbeat from gateway\n");
				if (!is_connected) {
					send_event(-1, "e_vhub_heartbeat_init_ok");
					is_connected = 1;
				}
				usleep(heartbeatInt * 1000);
			}
		}
	}

done:
	if (sockfd >= 0)
		close(sockfd);

	send_event(-1, "e_no_vhub_heartbeat");

	exit(ret);
}

#define HEARTBEAT_MC_PORT 3246

static void do_heartbeat_host_multicast(char *mc_addr)
{
	int mc_fd;
	int heartbeat = 0;
	//char mc_addr[128] = "225.0.0.37";
	
	mc_fd = mc_create_sender(mc_addr, HEARTBEAT_MC_PORT);
	if (mc_fd == -1) {
		exit(1);
	}

	send_event(-1, "e_heartbeat_init_ok");
	info("heartbeat multicast mode. Attaching...\n");
	send_event(-1, "e_attaching");

	while (1) {
		usleep(heartbeatInt * 1000);
		if (send(mc_fd, &heartbeat, sizeof(heartbeat), 0) <= 0)
			break;
		
		heartbeat++;
		//info(".");
	}

	//ToDo. Multicast sender would never return fail even link disconnected.
	send_event(-1, "e_no_heartbeat");
	
}


static void do_heartbeat_client_multicast(char *mc_addr)
{
	//char mc_addr[128] = "225.0.0.37";
	int mc_fd = -1;
	int is_connected = 0;
	struct timeval timeout;
	fd_set	fds;
	int ret;
	int heartbeat = 0;

	mc_fd = ret = mc_connect(mc_addr, HEARTBEAT_MC_PORT);
	if (ret < 0) {
		err("Failed to create heartbeat multicast socket!!(%d)\n", ret);
		goto done;
	}

	while (1) {
		/* fds and timeout becomes undefined after select. */
		FD_ZERO(&fds);
		FD_SET(mc_fd, &fds);
		timeout.tv_usec = 0;
		timeout.tv_sec = heartbeatTimeout / 1000;
		ret = select(mc_fd + 1, &fds, NULL, NULL, &timeout);
		//info("s");
		if (ret < 0) {
			err("!!!!!error select\n");
			break;
		} else if (ret == 0 && is_connected) {
			if (is_connected)
				info("!!!!!receive heartbeat timeouted\n");
			else
				info("No heartbeat received.\n");
			break;
		} else {
			if (!FD_ISSET(mc_fd, &fds)) {
				if (!is_connected) {
					/* Its kind of exception? This should be the case when host is not ready yet. */
					info("I guess No heartbeat received.\n");
					break;
				}
				continue;
			}
			
			if (!is_connected) {
				info("heartbeat connected\n");
				is_connected = 1;
				send_event(-1, "e_heartbeat_init_ok");
			}
			ret = recv(mc_fd, &heartbeat, sizeof(heartbeat), 0);
			//info("r");
			if (ret < 0) {
				err("error recv\n");
				goto done;
			} else if (ret == 0) {
				info("gateway disconnected\n");
				goto done;
			}
		}
	}

done:
	if (mc_fd >= 0)
		close(mc_fd);

	send_event(-1, "e_no_heartbeat");

	exit(ret);
}


int main(int argc, char *argv[])
{
	char *gw_ip;
	unsigned int is_multicast = 0;
	
	enum {
		cmd_host,
		cmd_client,
		cmd_help
	} cmd = cmd_host;


	for (;;) {
		int c;
		int index = 0;

		c = getopt_long(argc, argv, "hcd:m", longopts, &index);

		if (c == -1)
			break;

		switch (c) {
			case 'h':
				cmd = cmd_host;
				break;
			case 'c':
				cmd = cmd_client;
				break;
			case 'd':
				gw_ip = optarg;
				break;
			case 'm':
				is_multicast = 1;
				break;
			case '?':
				cmd = cmd_help;
				break;
			default:
				err("getopt\n");
		}
	}

	switch (cmd) {
		case cmd_host:
			if (is_multicast)
				do_heartbeat_host_multicast(gw_ip);
			else
				do_heartbeat_host();
			break;
		case cmd_client:
			//argv[2] should be the host's ip address
			if (is_multicast)
				do_heartbeat_client_multicast(gw_ip);
			else
				do_heartbeat_client(gw_ip);			
			break;
		case cmd_help:
			//show_help();
			//ast_heartbeat -h 
			//ast_heartbeat -c  -d gw_ip
			break;
		default:
			info("unknown cmd\n");
			//show_help();
	}

	return 0;
}

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
#include <getopt.h>
#include <strings.h>
#include <string.h>

#include "debug.h"
#include "astnetwork.h"
#include "remote.h"


static const struct option longopts[] = {
	{"host",	no_argument,	NULL, 'h'},
	{"client",	no_argument,	NULL, 'c'},
	{"target_ip", required_argument,	NULL, 'd'},
	{"reg", required_argument,	NULL, 'r'},
	{"event", required_argument,	NULL, 'e'},
	{NULL,		0,		NULL,  0}
};


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

	info("connected from %s:%s\n",host, port);

	return csock;
}

static inline int tcp_send(int sockfd, const void *buf, size_t len, int flags)
{
	int ret;
	
	ret = send(sockfd, buf, len, flags);
	if (ret == -1) {
		err("send FAILED (%d))\n", ret);
	}
	
	return ret;
}

static inline int tcp_recv(int sockfd, void *buf, size_t len, int flags)
{
	int ret;
	
	ret = recv(sockfd, buf, len, flags);
	if (ret == -1) {
		err("recv FAILED(%d)\n", ret);
	}
	
	return ret;
}


static int send_event(int sockfd, char *event)
{
	int ret;
	struct s_rc_header hdr;
	
	hdr.cmd_type = RC_CMD_EVENT;
	hdr.data_size = strlen(event) + 1;
	
	ret = tcp_send(sockfd, &hdr, sizeof(hdr), 0);
	if (ret <= 0)
		goto out;
		
	ret = tcp_send(sockfd, event, hdr.data_size, 0);
	if (ret <= 0)
		goto out;

	ret = tcp_recv(sockfd, &hdr, sizeof(hdr), 0);
	if (hdr.cmd_type != RC_CMD_ACCEPTED) {
		err("Handshake error!!\n");
	}

out:
	return ret;
}

static int send_reg_ip(int sockfd, char *ip)
{
	int ret;
	struct s_rc_header hdr;
	
	hdr.cmd_type = RC_CMD_REG;
	hdr.data_size = strlen(ip) + 1;
	
	ret = tcp_send(sockfd, &hdr, sizeof(hdr), 0);
	if (ret <= 0)
		goto out;
		
	ret = tcp_send(sockfd, ip, hdr.data_size, 0);
	if (ret <= 0)
		goto out;

	ret = tcp_recv(sockfd, &hdr, sizeof(hdr), 0);
	if (hdr.cmd_type != RC_CMD_ACCEPTED) {
		err("Handshake error!!\n");
	}

out:
	return ret;
}

static int send_cmd(int sockfd, unsigned int cmd, char *data)
{
	int ret = -1;
	
	switch (cmd) {
	case RC_CMD_REG:
		ret = send_reg_ip(sockfd, data);
		break;
	case RC_CMD_EVENT:
		ret = send_event(sockfd, data);
		break;
	default:
		err("Unknown remote command?!(%d)\n", cmd);
		break;
	}
	
	return ret;
}


static void do_rc_host(char *host, unsigned int cmd, char *data)
{
	int sockfd = -1;
	char buf[1];
	struct timeval timeout;
	fd_set	fds;
	int ret = -1;
	
	sockfd = tcp_connect(host, RC_TCP_PORT_STR);
	if (sockfd == -1) {
		err("error connect coip socket\n");
		goto done;
	}
	SetNodelay(sockfd);
	info("rc socket connected\n");
	//send_event(-1, "e_coip_init_ok");
	
	ret = send_cmd(sockfd, cmd, data);

#if NEVER_STOP
#include <unistd.h>
	while (1) { sleep(1); }; //TestOnly
#endif

done:
	if (sockfd >= 0)
		close(sockfd);

	exit(ret);
}

int handle_cmd_event(char *data)
{
	int ret;
	char cmd[RC_MAX_SIZE];
	
	sprintf(cmd, "/usr/local/bin/ast_send_event -1 %s", data);
	
	ret = system(cmd);
	if (ret < 0) {
		err("rc event failed(%d). event(%s)\n", ret, cmd);
	}
	return ret;
}

int handle_cmd_reg(char *data)
{
	FILE *fp;
	/* 
	** write the host's ip address to /etc/gw_ip. 
	** The link manager will update /etc/hosts correspondingly.
	*/
	fp = fopen("/etc/gw_ip", "w");
	if (fp == NULL) {
		err("Failed to open /etc/gw_ip!?\n");
		return -1;
	}
	//ToDo. validate the "data"
	info("gw's ip:%s\n", data);
	fprintf(fp, "%s", data);
	
	fflush(fp);
	fsync(fileno(fp));
	return fclose(fp);
}

static int recv_cmd(int sockfd)
{
	int ret;
	struct s_rc_header hdr;
	char data[RC_MAX_SIZE];
	
	while (1) {
		ret = tcp_recv(sockfd, &hdr, sizeof(hdr), 0);
		if (ret <= 0) {
			break;
		}
		
		if ((hdr.cmd_type & REMOTE_VER_MASK) != REMOTE_VER) {
			err("Invalid command (0x%08x)\n", hdr.cmd_type);
			break;
		}
		if (hdr.data_size) {
			ret = tcp_recv(sockfd, data, hdr.data_size, 0);
			if (ret <= 0) {
				err("Failed to receive cmd data (%d). Size(%d). cmd(0x%08x)\n", ret, hdr.data_size, hdr.cmd_type);
				break;
			}
		}
		
		switch (hdr.cmd_type) {
		case RC_CMD_REG:
			ret = handle_cmd_reg(data);
			break;
		case RC_CMD_EVENT:
			ret = handle_cmd_event(data);
			break;
		}
		
		if (ret < 0)
			break;
		
		hdr.cmd_type = RC_CMD_ACCEPTED;
		hdr.data_size = 0;
		tcp_send(sockfd, &hdr, sizeof(hdr), 0);
	}
}

static int do_rc_client(void)
{
	int csock = -1, lsock = -1;
	struct timeval timeout;
	char host_n[NI_MAXHOST];
	fd_set fds;
	struct sockaddr_in my_addr;
	int ret = -1;

#if NEVER_STOP
	while (1) {
#endif
		lsock = socket(AF_INET, SOCK_STREAM, 0);
		if (lsock == -1) {
			err("error socket\n");
			goto done;
		}
		SetReuseaddr(lsock);
		SetNodelay(lsock);

		my_addr.sin_family = AF_INET;
		my_addr.sin_addr.s_addr = htonl(INADDR_ANY);
		my_addr.sin_port = htons(RC_TCP_PORT);
		ret = bind(lsock, (struct sockaddr *)&my_addr, sizeof(my_addr));
		if (ret != 0) {
			err("error bind\n");
			goto done;
		}

		ret = listen(lsock, 1);
		if (ret != 0) {
			err("error listen\n");
			goto done;
		}

		//send_event(-1, "e_coip_init_ok");
		while (1) {
			csock = my_accept(lsock, host_n);
			if(csock < 0) {
				err("error accept\n");
				continue;
			}
			break;
		}
		//Accept only one connection.
		close(lsock);
		SetNodelay(csock);
		info("rc sender connected\n");

		info("rc csock=%d\n", csock);
		ret = recv_cmd(csock);
#if NEVER_STOP
		if (csock >= 0)
			close(csock);
		
		csock = -1;
	}
#endif

done:
	if (csock >= 0)
		close(csock);
	if (lsock >= 0)
		close(lsock);

	exit (ret);

}

int main(int argc, char *argv[])
{
	char *target_ip;
	unsigned int type;
	char *data;
	
	enum {
		cmd_host,
		cmd_client,
		cmd_help
	} cmd = cmd_host;


	for (;;) {
		int c;
		int index = 0;

		c = getopt_long(argc, argv, "hcd:r:e:", longopts, &index);

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
				target_ip = optarg;
				break;
			case 'r':
				type = RC_CMD_REG;
				data = optarg;
				break;
			case 'e':
				type = RC_CMD_EVENT;
				data = optarg;
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
			//Host is the program to control the client board over ip.
			do_rc_host(target_ip, type, data);
			break;
		case cmd_client:
			//Client is the board to be controlled over ip.
			do_rc_client();
			break;
		case cmd_help:
			//show_help();
			//coip -h 
			//coip -c  -d target_ip
			break;
		default:
			err("unknown cmd\n");
			//show_help();
	}

	return 0;
}


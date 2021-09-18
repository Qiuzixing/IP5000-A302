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
#include <errno.h>
#include <sys/epoll.h>

#include "uds_ipc.h"
#include "ioaccess.h"

#define	u8	unsigned char
#define	u16	unsigned short
#define	u32	unsigned int

#define msg(fmt, args...) fprintf(stdout, "ACTL: "fmt, ##args)
#define ERR(fmt, args...) fprintf(stderr, "ACTL: "fmt, ##args)

int kill_me = 0;
const char *cmd_str[] = {
	"unknown",
	"start",
	"stop",
	"exit",
	"invalid"
};

enum ctrl_cmd_e {
	CMD_UNKNOWN = 0,
	START,
	STOP,
	EXIT,
	CMD_MAX
};

enum ctrl_casting_e {
	CASTING_UNKNOWN = 0,
	UNICAST,
	MULTICAST,
	CASTING_MAX
};

const char *casting_str[] = {
	"unknown",
	"uc",
	"mc",
	"invalid"
};

#define MAX_IP_STR_LEN            15

struct video_ctrl {
	int cmd;
	u32 session_id;
	enum ctrl_casting_e casting;
	unsigned char ip[MAX_IP_STR_LEN + 1]; //xxx.xxx.xxx.xxx
	unsigned int port;
	unsigned char ip_r[MAX_IP_STR_LEN + 1]; /* receive IP */
	unsigned int port_r; /* receive port */
};

void dump_ctrl(struct video_ctrl *ctrl)
{
#if 1
	msg("cmd = %d\n", ctrl->cmd);
	msg("session id = %d\n", ctrl->session_id);
	msg("casting = %d\n", ctrl->casting);
	msg("ip = %s\n", ctrl->ip);
	msg("port = %d\n", ctrl->port);
	msg("ip_r = %s\n", ctrl->ip_r);
	msg("port_r = %d\n", ctrl->port_r);
#endif
}

int parse_cmd_start(unsigned char *cmd_buffer, struct video_ctrl *ctrl)
{
	char *save_ptr;
	char *token;
	int i;
	/*
	 * cmd format:
	 * cmd:session_id:casting_mode:ip:port:ip_r:port_r
	 *	ip: peer IP address
	 *		unicast:
	 *			Host: client's IP
	 *			Client: host's IP
	 *		multicast:
	 *			multicast address
	 *	port: port number which peer used to receive data
	 *	ip_r: IP address for local device to receive data
	 *		unicast:
	 *			Host: host's IP
	 *			Client: client's IP
	 *		multicast:
	 *			multicast address
	 *	port_r: port number which local used to receive data
	 * Ex: host
	 *	start:1001:uc:169.254.6.94:3245:169.254.10.133:3244
	 *	start:1001:mc:225.10.1.100:3245:225.10.1.100:3244
	 * Ex: client
	 *	start:2001:uc:169.254.10.133:3244:169.254.6.94:3245
	 *	start:2001:mc:225.10.1.100:3244:225.10.1.100:3245
	 */

	if (!cmd_buffer || !strlen(cmd_buffer))
		goto err;

	/* session id */
	token = strtok_r(cmd_buffer, ":", &save_ptr);
	if (!token)
		goto err;

	ctrl->session_id = strtoul(token, NULL, 0);

	/* casting mode */
	token = strtok_r(NULL, ":", &save_ptr);
	if (!token)
		goto err;

	for (i = (CASTING_UNKNOWN + 1); i < CASTING_MAX; i++) {
		if (!strncmp(token, casting_str[i], strlen(casting_str[i]))) {
			/* Found! */
			ctrl->casting = i;
			break;
		}
	}

	if (i == CASTING_MAX)
		goto err;

	/* ip */
	token = strtok_r(NULL, ":", &save_ptr);
	if (!token)
		goto err;

	strncpy(ctrl->ip, token, MAX_IP_STR_LEN);

	/* parse port */
	token = strtok_r(NULL, ":", &save_ptr);
	if (!token)
		goto err;

	ctrl->port = strtoul(token, NULL, 0);

	/* ip_r */
	token = strtok_r(NULL, ":", &save_ptr);
	if (!token)
		goto err;

	strncpy(ctrl->ip_r, token, MAX_IP_STR_LEN);

	/* port_r */
	token = strtok_r(NULL, ":", &save_ptr);
	if (!token)
		goto err;

	ctrl->port_r = strtoul(token, NULL, 0);
done:
	dump_ctrl(ctrl);
	return 0;
err:
	return -1;
}

int parse_cmd_generic(unsigned char *cmd_buffer, struct video_ctrl *ctrl)
{
	return 0;

}

int parse_cmd_unknown(unsigned char *cmd_buffer, struct video_ctrl *ctrl)
{
	return -1;
}

#ifdef AST_VIDEO_APP_CLIENT
void TX_I2S(int transport, int ctrlconn, int dataconn)
{
	IO_ACCESS_DATA IO_Data;
	int fd = open("/dev/i2s", O_RDONLY);

	IO_Data.transport = transport;
	IO_Data.CtrlSocket = ctrlconn;
	IO_Data.I2sSocket = dataconn;
	ioctl(fd, IOCTL_I2S_TX, &IO_Data);
	close(fd);
}
#else
static void RX_I2S(int transport, int connfd, int multicast)
{
	IO_ACCESS_DATA IO_Data;
	int fd = open("/dev/i2s", O_RDONLY);

	IO_Data.transport = transport;
	IO_Data.I2sSocket = connfd;
	IO_Data.Data = multicast;
	ioctl(fd, IOCTL_I2S_RX, &IO_Data);
	close(fd);
}

static void Add_Client(int connfd)
{
	IO_ACCESS_DATA IO_Data;
	int fd = open("/dev/i2s", O_RDONLY);

	IO_Data.CtrlSocket = connfd;
	ioctl(fd, IOCTL_ADD_CLIENT, &IO_Data);
	close(fd);
}
#endif

static int udp_create_receiver(char *mgroup, int port)
{
	struct sockaddr_in addr;
	struct ip_mreq mreq;
	int fd;
	int yes = 1;

	printf("udp_create_receiver (%s, %d)\n", mgroup, port);
	/* create what looks like an ordinary UDP socket */
	fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (fd < 0) {
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
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(port);

	/* bind to receive address */
	if (bind(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
		perror("bind");
		close(fd);
		return -1;
	}

	if (mgroup != NULL) {
		/* use setsockopt() to request that the kernel join a multicast group */
		mreq.imr_multiaddr.s_addr = inet_addr(mgroup);
		mreq.imr_interface.s_addr = htonl(INADDR_ANY);
		if (setsockopt(fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) < 0) {
			perror("setsockopt (IP_ADD_MEMBERSHIP)");
			close(fd);
			return -1;
		}
	}

	return fd;
}

#define IP_TTL_DEFAULT 64
#define IP4_MULTICAST(x)    (((x) & htonl(0xf0000000)) == htonl(0xe0000000))

static int udp_create_sender(char *dst_addr, int port)
{
	struct sockaddr_in addr;
	int fd;
	struct ip_mreq mreq;
	int yes = 1;

	printf("udp_create_sender (%s, %d)\n", dst_addr, port);

	/* create what looks like an ordinary UDP socket */
	fd = socket(AF_INET, SOCK_DGRAM, 0);

	if (fd < 0) {
		perror("socket");
		return -1;
	}

	/* set up bind address */
	memset(&addr, 0, sizeof(struct sockaddr_in));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(0);

	/* bind to send address */
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
		unsigned char loop = 0;
		unsigned char ttl = IP_TTL_DEFAULT;

		if (setsockopt(fd, IPPROTO_IP, IP_MULTICAST_LOOP, &loop, sizeof(loop)) < 0)
			perror("sockopt: mulitcast loopback");

		if (setsockopt(fd, IPPROTO_IP, IP_MULTICAST_TTL, &ttl, sizeof(ttl)) < 0)
			perror("sockopt: ttl");
	}

	if (connect(fd, (struct sockaddr *)&addr, sizeof(struct sockaddr)) < 0) {
		perror("connect");
		close(fd);
		return -1;
	}

	return fd;
}

#ifdef AST_VIDEO_APP_CLIENT
void do_cmd_start(struct video_ctrl *ctrl)
{
	int is_multicast = 0;
	int devfd;
	int DataSockFd = -1, CtrlSockFd;
	int net_tx_fd, net_rx_fd;

	if (ctrl->casting == MULTICAST)
		is_multicast = 1;

	if (is_multicast)
		net_rx_fd = udp_create_receiver(ctrl->ip_r, ctrl->port_r);
	else
		net_rx_fd = udp_create_receiver(NULL, ctrl->port_r);

	if (net_rx_fd < 0) {
		printf("failed to create the I2S receiver\n");
		goto fail_rx_fd;
	}

	net_tx_fd = udp_create_sender(ctrl->ip, ctrl->port);

	if (net_tx_fd < 0) {
		printf("failed to create the I2S sender\n");
		goto fail_tx_fd;
	}

	TX_I2S(0, net_tx_fd, net_rx_fd);

	close(net_tx_fd);
fail_tx_fd:
	close(net_rx_fd);
fail_rx_fd:
	return;
}
#else
void do_cmd_start(struct video_ctrl *ctrl)
{
	int is_multicast = 0;
	int devfd;
	int DataSockFd = -1, CtrlSockFd;
	int net_tx_fd, net_rx_fd;

	if (ctrl->casting == MULTICAST)
		is_multicast = 1;

	net_tx_fd = udp_create_sender(ctrl->ip, ctrl->port);

	if (net_tx_fd < 0) {
		printf("failed to create the I2S sender\n");
		goto fail_tx_fd;
	}

	RX_I2S(0, net_tx_fd, is_multicast);


	if (is_multicast)
		net_rx_fd = udp_create_receiver(ctrl->ip_r, ctrl->port_r);
	else
		net_rx_fd = udp_create_receiver(NULL, ctrl->port_r);

	if (net_rx_fd < 0) {
		printf("failed to create the I2S receiver\n");
		goto fail_rx_fd;
	}

	Add_Client(net_rx_fd);

	close(net_rx_fd);
fail_rx_fd:
	close(net_tx_fd);
fail_tx_fd:
	return;
}
#endif

void do_cmd_stop(struct video_ctrl *ctrl)
{

}

void do_cmd_exit(struct video_ctrl *ctrl)
{
	kill_me = 1;
}

struct v_cmd {
	u8 *string;
	u32 cmd;
	int (*parse)(u8 *cmd_buf, struct video_ctrl *ctrl);
	void (*handle)(struct video_ctrl *trl);
};

struct v_cmd v_cmd_table[] = {
	{"start", START, parse_cmd_start, do_cmd_start},
	{"stop", STOP, parse_cmd_generic, do_cmd_stop},
	{"exit", EXIT, parse_cmd_generic, do_cmd_exit},
	{NULL, CMD_UNKNOWN, parse_cmd_unknown, NULL},
};

static void handle_ctrl_cmd(char *cmd_buffer)
{
	struct video_ctrl ctrl, *pctrl = &ctrl;
	struct v_cmd *pvcmd;
	char *token;
	int i;

	if (!strlen(cmd_buffer))
		return;

	memset(pctrl, 0, sizeof(struct video_ctrl));

	token = strsep(&cmd_buffer, ":");
	for (i = 0; ; i++) {
		pvcmd = v_cmd_table + i;

		if (pvcmd->string == NULL)
			return;

		if (!strncmp(token, pvcmd->string, strlen(pvcmd->string))) {
			/* Found! */
			break;
		}
	}

	if (pvcmd->parse(cmd_buffer, pctrl)) {
		ERR("parse_cmd() error! %s\n", cmd_buffer);
		return;
	}

	pctrl->cmd = pvcmd->cmd;

	if (pvcmd->handle)
		pvcmd->handle(pctrl);
}

#define MAX_PAYLOAD               4096

unsigned char cmd_buffer[MAX_PAYLOAD];

int on_ctrl_recv(void *p1, void *p2)
{
	int listen_fd = (int)p1;
	int cmd_fd = -1;
	int nbytes;

	cmd_fd = accept(listen_fd, NULL, NULL);
	if (cmd_fd < 0) {
		ERR("accept() failed [%d:%s]\n", errno, strerror(errno));
		goto err;
	}
	/* receive cmd */
	nbytes = read(cmd_fd, cmd_buffer, MAX_PAYLOAD);
	if (nbytes < 0) {
		ERR("read() failed [%d:%s]\n", errno, strerror(errno));
		goto err;
	}
	cmd_buffer[nbytes] = 0;

	/* handle cmd */
	handle_ctrl_cmd(cmd_buffer);

	/*
	 * whether ack or not,
	 * once we close fd, ipc query will not get stuck in ipc_get
	 */
#if 0
	if (0) {
		char *ret = "ok";

		nbytes = write(cmd_fd, ret, strlen(ret));
		if (nbytes < 0) {
			err("write() failed [%d:%s]\n", errno, strerror(errno));
			return;
		}
	}
#endif

	/* close socket and out. */
	close(cmd_fd);

	return 0;
err:
	if (!(cmd_fd < 0))
		close(cmd_fd);

	return -1;
}

typedef int (*event_handler_func)(void *, void *);

struct do_event_s {
	event_handler_func handler;
	void *p1;
	void *p2;
};

void *reg_do_event_handler(struct do_event_s *e, event_handler_func func, void *p1, void *p2)
{
	e->handler = func;
	e->p1 = p1;
	e->p2 = p2;

	return (void *)e;
}

int main(void)
{
	int ctrl_fd = -1, epfd = -1;
	int ne;
	struct do_event_s *e;
	struct epoll_event ep_event;
	struct do_event_s ctrl_event_handle;

	ctrl_fd = create_uds_server("@a_ctrl");

	if (ctrl_fd < 0)
		goto done;

	/* About size '5', Since Linux 2.6.8, the size argument is ignored, but must be greater than zero */
	epfd = epoll_create(5);
	if (epfd < 0) {
		perror("epoll_create1 failed");
		goto done;
	}

	ep_event.events = EPOLLIN;
	ep_event.data.ptr = reg_do_event_handler(&ctrl_event_handle, on_ctrl_recv, (void *)ctrl_fd, NULL);

	if (epoll_ctl(epfd, EPOLL_CTL_ADD, ctrl_fd, &ep_event) < 0) {
		perror("epoll_ctl failed");
		goto done;
	}

	for (; !kill_me;) {
		ne = epoll_wait(epfd, &ep_event, 1, -1); /* wait indefinitely */

		if (ne < 0) /* something wrong. */
			continue;

		if (ne == 0) /* case for epoll timeout. */
			continue;

		/* Handle fd events. */
		e = (struct do_event_s *)ep_event.data.ptr;
		(e->handler)(e->p1, e->p2);
	}

done:
	printf("done");
	if (ctrl_fd >= 0)
		close(ctrl_fd);
	if (epfd >= 0)
		close(epfd);

	return 0;
}


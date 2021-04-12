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

#include "vdef.h"
#include "getinf.h"
#include "videoip_ioctl.h"
#include "astcommon.h"

#define ASTHOST_GLOBALS
#include "global.h"
#include "astdebug.h"
#include "astnetwork.h"

#define    u8                unsigned char
#define    u16               unsigned short
#define    u32               unsigned int


#define msg(fmt, args...) fprintf(stdout, "VCTRL:"fmt, ##args)

int kill_me = 0;
const char *cmd_str[] = {
	"unknown",
	"start",
	"stop",
	"attach",
	"exit",
	"invalid"
};

enum ctrl_cmd_e {
	CMD_UNKNOWN = 0,
	START,
	STOP,
	ATTACH,
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

#ifdef AST_VIDEO_APP_CLIENT
static void fill_hw_udp_header(PIOCTL_CLIENT_INFO_V2 pClientInfo, char *mc_addr)
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
#else
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
#endif
#define MAX_IP_STR_LEN            15

struct video_ctrl {
	int cmd;
	enum ctrl_casting_e casting;
	unsigned char ip[MAX_IP_STR_LEN + 1]; //xxx.xxx.xxx.xxx
	unsigned int port;
	unsigned char ip_r[MAX_IP_STR_LEN + 1]; /* receive IP */
	unsigned int port_r; /* receive port */
	unsigned int session_id;
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

	if (!cmd_buffer || !strlen(cmd_buffer)) {
		goto err;
	}

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

#ifndef AST_VIDEO_APP_CLIENT
int parse_cmd_attach(unsigned char *cmd_buffer, struct video_ctrl *ctrl)
{
	char *save_ptr;
	char *token;
	int i;
	/*
	 * cmd format:
	 * cmd:session_id:casting_mode:ip:port
	 *	ip: IP address for local device to receive data
	 *		unicast:
	 *			Host: host's IP
	 *		multicast:
	 *			multicast address
	 *	port: port number which local used to receive data
	 * Ex: host
	 *	attaching:1001:uc:169.254.10.133:3244
	 *	attaching:1001:mc:225.10.1.100:3244
	 */

	if (!cmd_buffer || !strlen(cmd_buffer)) {
		goto err;
	}

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

done:
	dump_ctrl(ctrl);
	return 0;
err:
	return -1;
}
#endif /* #ifndef AST_VIDEO_APP_CLIENT */

int parse_cmd_unknown(unsigned char *cmd_buffer, struct video_ctrl *ctrl)
{
	return -1;
}

#ifdef AST_VIDEO_APP_CLIENT
void do_cmd_start(struct video_ctrl *ctrl)
{
	int devfd, data_sock_fd, ctrl_sock_fd;
	IOCTL_CLIENT_INFO_V2 ClientInfo;

	devfd = open("/dev/videoip", O_RDONLY);

	if (devfd < 0) {
		err("open device");
		return;
	}

	ctrl_sock_fd = udp_create_sender(ctrl->ip, ctrl->port);

	if (ctrl_sock_fd == -1) {
		err("multicast socket for ctrl connect fail\n");
		goto fail_tx_fd;
	}

	ClientInfo.CtrlSockFd = ctrl_sock_fd;

	if (ctrl->casting == MULTICAST) {
		ClientInfo.CastingMode = 2;
		data_sock_fd = udp_connect(ctrl->ip_r, ctrl->port_r);
	} else {
		ClientInfo.CastingMode = 1;
		data_sock_fd = udp_connect(NULL, ctrl->port_r);
	}

	/*
	 * all ports used in video driver do not overlap with ephemeral port (60000~65535)
	 * so never get "data_sock_fd, < 0"
	 */
	if (data_sock_fd < 0) {
		err("multicast socket connect fail\n");
		goto fail_rx_fd;
	}

	ClientInfo.DataSockFd = data_sock_fd;
	ClientInfo.session_id = ctrl->session_id;

	fill_hw_udp_header(&ClientInfo, ctrl->ip);
	ioctl(devfd, IOCTL_CLIENT_ONLY_V2, &ClientInfo);

	//Bruce120816. Sockets are owned by kernel at this point. They will still
	//be available even though we close them here. So, I see no need to pause
	//astclient here. We terminate this process normally. The socket will still
	//be vaild and will be closed by kernel driver eventually.
	//pause(); //suspend process until signal
	//info("astclient shutdown");

	close(data_sock_fd);
fail_rx_fd:
	close(ctrl_sock_fd);
fail_tx_fd:
	close(devfd);
	return;
}
#else
void do_cmd_start(struct video_ctrl *ctrl)
{
	int devfd, DataSockFd;
	IOCTL_REMOTE_INFO HostInfo;

	devfd = open("/dev/videoip", O_RDONLY);

	if (devfd < 0) {
		err("open device");
		return;
	}

	DataSockFd = udp_create_sender(ctrl->ip, ctrl->port);

	if (DataSockFd == -1)
		goto fail_sock_fd;

	memset(&HostInfo, 0, sizeof(IOCTL_REMOTE_INFO));

	if (ctrl->casting == MULTICAST)
		HostInfo.CastingMode = 2; /* multicast */
	else
		HostInfo.CastingMode = 1;

	HostInfo.DataSockFd = DataSockFd;
	HostInfo.session_id = ctrl->session_id;
	fill_hw_udp_header(&HostInfo);
	ioctl(devfd, IOCTL_HOST_ENABLE_REMOTE, &HostInfo);

	if (close(DataSockFd))
		err("Failed to close data socket (%d)\n", errno);

fail_sock_fd:
	close(devfd);
}
#endif

void do_cmd_stop(struct video_ctrl *ctrl)
{

}

#ifndef AST_VIDEO_APP_CLIENT
void do_cmd_attach(struct video_ctrl *ctrl)
{
	int devfd;
	int CtrlSockFd;
	int is_multicast = 0;
	IOCTL_REMOTE_INFO HostInfo;

	memset(&HostInfo, 0, sizeof(IOCTL_REMOTE_INFO));

	if (ctrl->casting == MULTICAST)
		is_multicast = 1;

	devfd = open("/dev/videoip", O_RDONLY);

	if (devfd < 0) {
		err("open device");
		return;
	}

	if (is_multicast)
		CtrlSockFd = udp_connect(ctrl->ip, ctrl->port);
	else
		CtrlSockFd = udp_connect(NULL, ctrl->port);

	/*
	 * all ports used in video driver do not overlap with ephemeral port (60000~65535)
	 * so never get "sockfd < 0"
	 */
	if (CtrlSockFd < 0) {
		err("socket for ctrl connect fail\n");
		goto fail_sock_fd;
	}

	HostInfo.CtrlSockFd = CtrlSockFd;
	HostInfo.session_id = ctrl->session_id;
	ioctl(devfd, IOCTL_HOST_ADD_CLIENT, &HostInfo);

	close(CtrlSockFd);
fail_sock_fd:
	close(devfd);
}
#endif /* #ifndef AST_VIDEO_APP_CLIENT */

void do_cmd_exit(struct video_ctrl *ctrl)
{
	kill_me = 1;

	return;
}

struct v_cmd {
	u8 *string;
	u32 cmd;
	int (*parse) (u8 *cmd_buf, struct video_ctrl *ctrl);
	void (*handle) (struct video_ctrl *trl);

};

struct v_cmd v_cmd_table[] = {
	{"start", START, parse_cmd_start, do_cmd_start},
	{"stop", STOP, parse_cmd_generic, do_cmd_stop},
#ifndef AST_VIDEO_APP_CLIENT
	{"attach", ATTACH, parse_cmd_attach, do_cmd_attach},
#endif
	{"exit", EXIT, parse_cmd_generic, do_cmd_exit},
	{NULL, CMD_UNKNOWN, parse_cmd_unknown, NULL},
};

void handle_ctrl_cmd(unsigned char *cmd_buffer)
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
		err("parse_cmd() error! %s\n", cmd_buffer);
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
		err("accept() failed [%d:%s]\n", errno, strerror(errno));
		goto err;
	}
	/* receive cmd */
	nbytes = read(cmd_fd, cmd_buffer, MAX_PAYLOAD);
	if (nbytes < 0) {
		err("read() failed [%d:%s]\n", errno, strerror(errno));
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

	ctrl_fd = create_uds_server("@v_ctrl");

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
	if (ctrl_fd >= 0)
		close(ctrl_fd);
	if (epfd >= 0)
		close(epfd);

	return 0;
}

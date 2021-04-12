/*
 * Copyright (c) 2004-2012
 * ASPEED Technology Inc. All Rights Reserved
 * Proprietary and Confidential
 *
 * By using this code you acknowledge that you have signed and accepted
 * the terms of the ASPEED SDK license agreement.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <termios.h>
#include <signal.h>
#include <strings.h>
#include <time.h>

#include <getopt.h>
#include <netdb.h> /* resolve hostname */
#include <string.h> /* memset */
#include <netinet/tcp.h> //For TCP
#include <linux/netlink.h>
#include <asm/types.h>

#include "astdebug.h"

#define MAXMESG 2048
#define SOIP_PORT 59500 /* 6752 */
#define SOIP_PORT_MULITCAST 59500 /* 6753 */
#define TIMEOUT 2 /* 2 seconds */
#define MAX_STR_LEN   64

#define TOKEN_CHECK_INTERVAL 1 /* second */
#define TOKEN_TIME 3 /* second, The default value of token timeout. */

struct s_req_pkt {
	char target_name[MAX_STR_LEN]; /* ast-client1111.local */
	char target_ip[MAX_STR_LEN]; /* 255.255.255.255 */
	char uart_config[MAX_STR_LEN]; /* 115200-8n1 */
};

/* Globals */
int uart_fd = -1;

fd_set master_fd;
/* maximum file descriptor number */
int fdmax = 0;
unsigned int client_ip = 0;

static const struct option longopts[] = {
	{"host",	no_argument,	NULL, 'h'},
	{"client",	no_argument,	NULL, 'c'},
	{"UART_port", required_argument,	NULL, 'f'},
	{"host_ip", required_argument,	NULL, 'd'},
	{"UART_config", required_argument,	NULL, 'b'},
	{"test", no_argument,	NULL, 't'},
	{"token_time", required_argument,	NULL, 'o'},
	{"multicast",   required_argument,      NULL, 'm'},
	{NULL,		0,		NULL,  0}
};

#if defined(SOIP_UART_DEBUG)
#define UART_PRINTF(fd, fmt, args...) \
	do { \
		char s[MAX_STR_LEN]; \
		sprintf(s, fmt, ##args); \
		_block_fd(fd); \
		write(fd, s, strlen(s)); \
		_nonblock_fd(fd); \
	} while (0)
#else
#define UART_PRINTF(fd, fmt, args...)  do {} while (0)
#endif


void help(void)
{
	fprintf(stderr, "\
 Usage: \n\
  Host:    soip -h -d <device> -b sss-dps \n\
  Client:  soip -c -d <device> \n\
  Echo Test: soip -t -d <device> -b sss-dps \n\
\n\
    -d <device>       local serial device. (ex:/dev/ttyS0) \n\
    -b sss-dps        baud rate setting. (ex: 115200-8n1) \n\
       sss is speed (50,..,230400)\n\
       d is data bits (5,6,7,8)\n\
       p is parity type (N,E,O)\n\
       s is stop bits (1,2)\n\
\n\
 Host Control Command: \n\
    Connect: ^Nast_c <client_id> sss-dps   \n\
    Disconnect: ^N    \n\
");
}

static int _nonblock_fd(int fd)
{
	long arg;

	arg = fcntl(fd, F_GETFL, NULL);

	if (arg < 0) {
		err("fcntl GET failed (%s)", strerror(errno));
		return -1;
	}

	arg |= O_NONBLOCK;

	if (fcntl(fd, F_SETFL, arg) < 0) {
		err("fcntl SET failed (%s)", strerror(errno));
		return -1;
	}
	return 0;
}

static int _block_fd(int fd)
{
	long arg;

	arg = fcntl(fd, F_GETFL, NULL);

	if (arg < 0) {
		err("fcntl GET failed (%s)", strerror(errno));
		return -1;
	}

	arg &= (~O_NONBLOCK);

	if (fcntl(fd, F_SETFL, arg) < 0) {
		err("fcntl SET failed (%s)", strerror(errno));
		return -1;
	}
	return 0;
}

static int block_write(int fd, char *buf, int cnt)
{
	int size = cnt;

	if (!cnt) {
		dbg("block_write no cnt?! (%d)", strerror(errno));
		return 0;
	}

	_block_fd(fd);
	cnt = write(fd, buf, cnt);
	if (cnt < 0) {
		err("write failed?! (%d)", strerror(errno));
		return cnt;
	}
	if (cnt < size) {
		err("Can't finish write?! %d written for %d", cnt, size);
		return -1;
	}
	_nonblock_fd(fd);
	return 0;
}

static int block_send(int fd, char *buf, int cnt)
{
	int size = cnt;

	if (!cnt) {
		dbg("block_send not cnt?! (%d)", strerror(errno));
		return 0;
	}

	_block_fd(fd);
	cnt = send(fd, buf, cnt, MSG_NOSIGNAL);
	if (cnt < 0) {
		err("send failed?! (%d)", strerror(errno));
		return cnt;
	}
	if (cnt < size) {
		err("Can't finish send?! %d written for %d", cnt, size);
		return -1;
	}
	_nonblock_fd(fd);
	return 0;
}

static int setserial(int s, struct termios *cfg, int speed, int data, unsigned char parity, int stopb)
{
	speed_t speed_cfg;

	cfmakeraw(cfg);

	switch (speed) {
	case 50:
		speed_cfg = B50; break;
	case 75:
		speed_cfg = B75; break;
	case 110:
		speed_cfg = B110; break;
	case 134:
		speed_cfg = B134; break;
	case 150:
		speed_cfg = B150; break;
	case 200:
		speed_cfg = B200; break;
	case 300:
		speed_cfg = B300; break;
	case 600:
		speed_cfg = B600; break;
	case 1200:
		speed_cfg = B1200; break;
	case 1800:
		speed_cfg = B1800; break;
	case 2400:
		speed_cfg = B2400; break;
	case 4800:
		speed_cfg = B4800; break;
	case 9600:
		speed_cfg = B9600; break;
	case 19200:
		speed_cfg = B19200; break;
	case 38400:
		speed_cfg = B38400; break;
	case 57600:
		speed_cfg = B57600; break;
	case 115200:
		speed_cfg = B115200; break;
	case 230400:
		speed_cfg = B230400; break;
	default:
		speed_cfg = B115200; break;
	}

	cfsetispeed(cfg, speed_cfg);
	cfsetospeed(cfg, speed_cfg);

	switch (parity|32) {
	case 'e':
		cfg->c_cflag |= PARENB; cfg->c_cflag &= ~PARODD; break;
	case 'o':
		cfg->c_cflag |= PARENB; cfg->c_cflag |= PARODD ; break;
	case 'n':
	default:
		cfg->c_cflag &= ~PARENB; break;
	}

	cfg->c_cflag &= ~CSIZE;

	switch (data) {
	case 5:
		cfg->c_cflag |= CS5; break;
	case 6:
		cfg->c_cflag |= CS6; break;
	case 7:
		cfg->c_cflag |= CS7; break;
	case 8:
	default:
		cfg->c_cflag |= CS8; break;
	}

	if (stopb == 1)
		cfg->c_cflag &= ~CSTOPB;
	else
		cfg->c_cflag |= CSTOPB;

	return tcsetattr(s, TCSANOW, cfg);
}

static int open_uart(char *uart_port, char *uart_config)
{
	int fd;
	int speed, data, stopb;
	unsigned char parity;
	unsigned int n;
	struct termios cfg;

	fd = open(uart_port, O_RDWR|O_NDELAY);
	if (fd < 0) {
		err("could not open device %s", uart_port);
		return -1;
	}

	n = sscanf(uart_config, "%d-%d%c%d", &speed, &data, &parity, &stopb);
	if (n < 4) {
		err("invalid argument %1d from %s", read+1, uart_config);
		return -2;
	}

	if (setserial(fd, &cfg, speed, data, parity, stopb) < 0) {
		err("could not initialize device %s", uart_port);
		return -3;
	}

	return fd;
}

static int udp_create_receiver(char *mgroup, int port)
{
	struct sockaddr_in addr;
	struct ip_mreq mreq;
	int fd;
	int yes = 1;

	/* create what looks like an ordinary UDP socket */
	fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (fd < 0) {
		perror("socket");
		return -1;
	}

	/* allow multiple sockets to use the same PORT number */
	if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) < 0) {
		perror("setsockopt (SO_REUSEADDR)");
		return -1;
	}

	/* set up destination address */
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	/*
	 * the reason for INADDR_ANY:
	 * two scenario with multicast:
	 * (1) host holds token: multicast (host => clients)
	 * (2) client holds token: unicast (host <=> client)
	 *
	 * no need for concern with multicast filter,
	 * it will get done after IP_ADD_MEMBERSHIP option
	 */
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(port);

	/* bind to receive address */
	if (bind(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
		perror("bind");
		return -1;
	}

	if (mgroup != NULL) {
		/* use setsockopt() to request that the kernel join a multicast group */
		mreq.imr_multiaddr.s_addr = inet_addr(mgroup);
		mreq.imr_interface.s_addr = htonl(INADDR_ANY);
		if (setsockopt(fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) < 0) {
			perror("setsockopt (IP_ADD_MEMBERSHIP)");
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
		return -1;
	}

	return fd;
}

/*
** Loop of Serial over IP
*/
int soip_client_loop(int uart_fd, int net_rx_fd, int net_tx_fd)
{
	int ret;
	fd_set fds;
	int uc, sc;
	char ub[MAXMESG], sb[MAXMESG];

	signal(SIGPIPE, SIG_IGN);

	while (1) {
		sc = 0;
		uc = 0;
		FD_ZERO(&fds);
		FD_SET(uart_fd, &fds);
		FD_SET(net_rx_fd, &fds);

		ret = select(net_rx_fd+1, &fds, NULL, NULL, NULL);
		if (ret < 0) {
			err("Error connecting %s", strerror(errno));
			goto out;
		}

		if (ret == 0) {
			err("Timeout in client_loop(). Should never happen");
			ret = -2;
			goto out;
		}

		if (FD_ISSET(uart_fd, &fds)) { /* Read client UART */
			memset(ub, 0, MAXMESG);
			uc = read(uart_fd, ub, MAXMESG);
			if ((uc < 0) && (errno != EAGAIN)) {
				ret = errno;
				err("uart read failed! (%s)", strerror(errno));
				goto out;
			}
		}

		if (FD_ISSET(net_rx_fd, &fds)) { /* Read socket */
			memset(sb, 0, MAXMESG);
			sc = read(net_rx_fd, sb, MAXMESG);
			if ((sc <= 0) && (errno != EAGAIN)) {
				ret = errno;
				err("socket read failed! (%s)", strerror(errno));
				goto out;
			}
		}

		if (sc) { /* Send to client UART */
			ret = block_write(uart_fd, sb, sc);
			if (ret < 0)
				goto out;
		}

		if (uc) { /* Send to socket. Includes the CTRL_CODE. */
			ret = block_write(net_tx_fd, ub, uc);
			if (ret < 0)
				goto out;
		}
	}

out:
	signal(SIGPIPE, SIG_DFL);
	return ret;
}

/*
** This test function simply echo what is read from the partner.
*/
unsigned int do_soip3_test(int uart_fd)
{
	int ret = 0;
	static char buf[MAXMESG];
	int cnt;
	fd_set fds;

	while (1) {
		FD_ZERO(&fds);
		FD_SET(uart_fd, &fds);

		ret = select(uart_fd+1, &fds, NULL, NULL, NULL);
		if (ret < 0) {
			err("Error connecting %s", strerror(errno));
			goto out;
		}
		if (ret == 0) {
			err("Timeout in host_loop(). Should never happen");
			ret = -2;
			goto out;
		}

		if (FD_ISSET(uart_fd, &fds)) {
			cnt = read(uart_fd, buf, MAXMESG);
			if (cnt <= 0) {
				ret = errno;
				err("uart read failed! (%s)", strerror(errno));
				goto out;
			}

			if (cnt) {
				_block_fd(uart_fd);
				cnt = write(uart_fd, buf, cnt);
				_nonblock_fd(uart_fd);

				if (cnt < 0) {
					err("uart write failed?! (%d)", strerror(errno));
					return cnt;
				}
			}
		}

	}

out:
	return ret;
}

unsigned int do_soip_client(int uart_fd, char *host_ip, char *mc_addr)
{
	int ret = 0;
	struct s_req_pkt req;
	int net_rx_fd, net_tx_fd;

	memset(&req, 0, sizeof(struct s_req_pkt));
	strcat(req.target_ip, host_ip);
	dbg("Host ip is %s\n", req.target_ip);

	net_rx_fd = udp_create_receiver(mc_addr, SOIP_PORT);

	if (net_rx_fd < 0)
		goto out;

	net_tx_fd = udp_create_sender(host_ip, SOIP_PORT);

	if (net_tx_fd < 0) {
		close(net_rx_fd);
		goto out;
	}

	while (1)
		soip_client_loop(uart_fd, net_rx_fd, net_tx_fd);


	info("Ending SoIP...\n");

	close(net_rx_fd);
	close(net_tx_fd);

out:
	return ret;
}

static int do_send_out(int uc, char *ubuff, int client_token)
{
	int ret;
	ret = block_send(client_token, ubuff, uc);

	if (ret < 0) {
		err("socket write failed! (%s), remove connection..", strerror(errno));
		return 0;
	} else
		return 1;
}

#define SOIP_FD_SET(fd) \
	do { \
		FD_SET((fd), &master_fd); \
		if ((fd) > fdmax) \
			fdmax = (fd); \
	} while (0)

int token_host_own = 1;

#define HOST_OWN_TOKEN() { client_ip = 0; }
#define CLIENT_OWN_TOKEN(ip) { client_ip = (ip); }
#define TOKEN_OWN_BY_HOST()  (client_ip == 0)
#define IS_TOKEN_OWNER(ip) (client_ip == (ip))

int do_soip_host(int uart_fd, const int token_time, char *mc_addr)
{
	fd_set read_fds;
	int i;
	int ret = -1;
	int uc, sc;
	char ub[MAXMESG], sb[MAXMESG];
	time_t last_check_time = time(NULL);
	int time_remain = 0;
	int socket_fd;
	int num_bytes;
	int is_multicast = 0;
	int net_tx_fd = -1, net_tx_fd_multicast = -1, net_rx_fd;

	if (mc_addr)
		is_multicast = 1;

	if (is_multicast)
		net_tx_fd_multicast = udp_create_sender(mc_addr, SOIP_PORT);

	FD_ZERO(&master_fd);
	FD_ZERO(&read_fds);

	SOIP_FD_SET(uart_fd);

	net_rx_fd = udp_create_receiver(NULL, SOIP_PORT);

	if (net_rx_fd < 0)
		goto done;

	SOIP_FD_SET(net_rx_fd);


	if (is_multicast) {
		int net_rx_fd_multicast;

		/*
		 * control multiple devices in guest mode
		 * create socket for host to receive multicast data
		 */
		net_rx_fd_multicast = udp_create_receiver(mc_addr, SOIP_PORT_MULITCAST);

		if (net_rx_fd_multicast < 0)
			goto done;

		SOIP_FD_SET(net_rx_fd_multicast);
	}

	/* At start, no client should have token, host holds token */
	HOST_OWN_TOKEN();

	for (;;) {
		struct timeval timeout;

		for (;;) {
			/* copy it */
			read_fds = master_fd;

			timeout.tv_usec = 0;
			timeout.tv_sec = TOKEN_CHECK_INTERVAL;
			ret = select(fdmax + 1, &read_fds, NULL, NULL, &timeout);
			if (ret < 0) {
				err("Client-select() error lol!");
				goto done;
			}

			if (abs(time(NULL) - last_check_time) >= TOKEN_CHECK_INTERVAL) {
				if (time_remain <= 0) {
					HOST_OWN_TOKEN();
					time_remain = 0;
				} else
					time_remain--;
				last_check_time = time(NULL);
			}

			if (ret > 0) {
				dbg("soip3 select() OK...\n");
				/*
				** Bruce141204. A hack to allow immediately release token when token_time is set to 0
				*/
				if (token_time == 0) {
					HOST_OWN_TOKEN();
				}
				break;
			}
		}

		/* run through the existing connections looking for data to be read */
		for (i = 0; i <= fdmax; i++) {
			if (FD_ISSET(i, &read_fds)) { /* we got one... */
				if (i == uart_fd) {
					memset(ub, 0, MAXMESG);
					uc = read(i, ub, MAXMESG);
					if ((uc < 0) && (errno != EAGAIN)) {
						ret = errno;
						err("host uart read failed! (%s)", strerror(errno));
						goto done;
					}

					if (uc) {
						int tx_fd = net_tx_fd; /* net_tx_fd is unicast */

						if (is_multicast && TOKEN_OWN_BY_HOST())
							tx_fd = net_tx_fd_multicast;

						if (tx_fd != -1)
							do_send_out(uc, ub, tx_fd);
					}
				} else { //the fds that we are interested, client has transmission coming
					dbg("socket(%d) event, time_remain=%d", i, time_remain);
					memset(sb, 0, MAXMESG);

					struct sockaddr_in src_addr;
					socklen_t addrlen;

					addrlen = sizeof(struct sockaddr);

					sc = recvfrom(i, sb, MAXMESG, 0, &src_addr, &addrlen);

					if (TOKEN_OWN_BY_HOST()) {
						CLIENT_OWN_TOKEN(src_addr.sin_addr.s_addr);
						time_remain = token_time;

						if (net_tx_fd >= 0)
							close(net_tx_fd);
						net_tx_fd = udp_create_sender(inet_ntoa(src_addr.sin_addr), SOIP_PORT);
					}

					if (IS_TOKEN_OWNER(src_addr.sin_addr.s_addr)) {
						time_remain = token_time;
						ret = block_write(uart_fd, sb, sc);
						if (ret < 0) {
							err("host UART write failed! (%s)", strerror(errno));
							//goto done;
						}
					}
				}
			}
		}
	}

done:
	dbg("done called! (%s)", strerror(errno));

	for (i = 0; i <= fdmax; i++) {
		if (FD_ISSET(i, &master_fd))
			close(i);
	}

	return ret;
}

int main(int argc, char **argv)
{
	char *uart_port;
	char *uart_config;
	char *host_ip;
	char *mc_addr = NULL;
	int token_time = TOKEN_TIME;

	enum {
		cmd_host,
		cmd_client,
		cmd_test,
		cmd_help
	} cmd = cmd_help;

	for (;;) {
		int c;
		int index = 0;

		c = getopt_long(argc, argv, "hcf:d:b:to:m:", longopts, &index);

		if (c == -1)
			break;

		switch (c) {
		case 'h':
			cmd = cmd_host;
			break;
		case 'c':
			cmd = cmd_client;
			break;
		case 't':
			cmd = cmd_test;
			break;
		case 'f':
			uart_port = optarg;
			break;
		case 'd':
			host_ip = optarg;
			break;
		case 'b':
			uart_config = optarg;
			break;
		case 'o':
			token_time = atoi(optarg);
			break;
		case 'm':
			mc_addr = optarg;
			break;
		case '?':
			cmd = cmd_help;
			break;
		default:
			err("getopt\n");
		}
	}

	uart_fd = open_uart(uart_port, uart_config);

	if (uart_fd < 0)
		goto exit;

	switch (cmd) {
	case cmd_host:
		do_soip_host(uart_fd, token_time, mc_addr);
		break;
	case cmd_client:
		do_soip_client(uart_fd, host_ip, mc_addr);
		break;
	case cmd_test:
		do_soip3_test(uart_fd);
		break;
	case cmd_help:
		help();
		break;
	default:
		err("unknown cmd\n");
		help();
	}

	if (uart_fd > 0)
		close(uart_fd);

exit:
	return 0;
}


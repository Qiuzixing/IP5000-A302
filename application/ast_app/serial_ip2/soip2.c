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
#include <netdb.h> //resolve hostname
#include <string.h> //memset
#include <netinet/tcp.h> //For TCP
#include <linux/netlink.h>
#include <asm/types.h>

#include "astdebug.h"

#define MAXMESG 2048
//#define SOIP2_PORT 6752
#define SOIP2_PORT 50001

//#define CTRL_CODE 0x0E /* Ctrl+N */
#define TIMEOUT 2 // 2 seconds

#define MAX_STR_LEN   64

#define TOKEN_CHECK_INTERVAL 1 //second
#define TOKEN_TIME 3 //second, The default value of token timeout.
#define MAX_PAYLOAD 1024  /* maximum payload size*/
#define SOIP2_MSG_TAG "e_no_heartbeat_"
#define MSG_MAGIC_NUM 0x19760614
#define MSG_CMDTYPE_ACTION 2 //action
#define MAX_MSG_PKT_SIZE (sizeof(t_msg_hdr))
#define MSG_PKT_HDR_SIZE (sizeof(t_msg_hdr) - MAX_PAYLOAD)

typedef struct
{
	unsigned long ip; //For ipV4. 4 bytes long.
} t_c_info;

struct s_req_pkt
{
	//unsigned int magic_num;
	//char req[MAX_STR_LEN]; //CMD_CONNECT, etc...
	char target_name[MAX_STR_LEN]; //ast-client1111.local
	char target_ip[MAX_STR_LEN]; //255.255.255.255
	char uart_config[MAX_STR_LEN]; //115200-8n1	
};

typedef struct
{
	//int MgcNum;
	//int CmdType;
	//int DataSize;
	//int reserved;
	//char data[MAX_PAYLOAD];
	char ip[MAX_PAYLOAD];
} t_msg_hdr;

/* Globals */
int uart_fd = -1;

fd_set master_fd;
/* maximum file descriptor number */
int fdmax = 0;
/* listening socket descriptor */
int listener_fd = -1;
t_c_info client_list[FD_SETSIZE];
int token_store;


static const struct option longopts[] = {
	{"host",	no_argument,	NULL, 'h'},
	{"client",	no_argument,	NULL, 'c'},
	{"UART_port", required_argument,	NULL, 'f'},
	{"host_ip", required_argument,	NULL, 'd'},
	{"UART_config", required_argument,	NULL, 'b'},
	{"test", no_argument,	NULL, 't'},
	{"token_time", required_argument,	NULL, 'o'},
	{NULL,		0,		NULL,  0}
};


#define UART_PRINTF(fd, fmt, args...) \
	do { \
		char s[MAX_STR_LEN]; \
		sprintf(s, fmt, ##args); \
		_block_fd(fd); \
		write(fd, s, strlen(s)); \
		_nonblock_fd(fd); \
	} while (0)


void help(void)
{
	fprintf(stderr,"\
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
	return;
}

static int _nonblock_fd(int fd)
{
	long arg;

	if ((arg = fcntl(fd, F_GETFL, NULL)) < 0) {
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

	if ((arg = fcntl(fd, F_GETFL, NULL)) < 0) {
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
	
	if (!cnt)
	{
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
	
	if (!cnt)
	{
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


int setserial(int s,struct termios*cfg,int speed,int data,unsigned char parity,int stopb){
	cfmakeraw(cfg);
	switch(speed){
		case 50     : { cfsetispeed(cfg,B50)    ; cfsetospeed(cfg,B50)    ; break; }
		case 75     : { cfsetispeed(cfg,B75)    ; cfsetospeed(cfg,B75)    ; break; }
		case 110    : { cfsetispeed(cfg,B110)   ; cfsetospeed(cfg,B110)   ; break; }
		case 134    : { cfsetispeed(cfg,B134)   ; cfsetospeed(cfg,B134)   ; break; }
		case 150    : { cfsetispeed(cfg,B150)   ; cfsetospeed(cfg,B150)   ; break; }
		case 200    : { cfsetispeed(cfg,B200)   ; cfsetospeed(cfg,B200)   ; break; }
		case 300    : { cfsetispeed(cfg,B300)   ; cfsetospeed(cfg,B300)   ; break; }
		case 600    : { cfsetispeed(cfg,B600)   ; cfsetospeed(cfg,B600)   ; break; }
		case 1200   : { cfsetispeed(cfg,B1200)  ; cfsetospeed(cfg,B1200)  ; break; }
		case 1800   : { cfsetispeed(cfg,B1800)  ; cfsetospeed(cfg,B1800)  ; break; }
		case 2400   : { cfsetispeed(cfg,B2400)  ; cfsetospeed(cfg,B2400)  ; break; }
		case 4800   : { cfsetispeed(cfg,B4800)  ; cfsetospeed(cfg,B4800)  ; break; }
		case 9600   : { cfsetispeed(cfg,B9600)  ; cfsetospeed(cfg,B9600)  ; break; }
		case 19200  : { cfsetispeed(cfg,B19200) ; cfsetospeed(cfg,B19200) ; break; }
		case 38400  : { cfsetispeed(cfg,B38400) ; cfsetospeed(cfg,B38400) ; break; }
		case 57600  : { cfsetispeed(cfg,B57600) ; cfsetospeed(cfg,B57600) ; break; }
		case 115200 : { cfsetispeed(cfg,B115200); cfsetospeed(cfg,B115200); break; }
		case 230400 : { cfsetispeed(cfg,B230400); cfsetospeed(cfg,B230400); break; }
	}
	switch(parity|32){
		case 'n' : { cfg->c_cflag &= ~PARENB; break; }
		case 'e' : { cfg->c_cflag |= PARENB; cfg->c_cflag &= ~PARODD; break; }
		case 'o' : { cfg->c_cflag |= PARENB; cfg->c_cflag |= PARODD ; break; }
	}
	cfg->c_cflag &= ~CSIZE;
	switch(data){
		case 5 : { cfg->c_cflag |= CS5; break; }
		case 6 : { cfg->c_cflag |= CS6; break; }
		case 7 : { cfg->c_cflag |= CS7; break; }
		case 8 : { cfg->c_cflag |= CS8; break; }
	}
	if(stopb==1)cfg->c_cflag&=~CSTOPB;
	else cfg->c_cflag|=CSTOPB;
	return tcsetattr(s,TCSANOW,cfg);
}

int SetNodelay(int sockfd)
{
	const int val = 1;
	int ret;

	ret = setsockopt(sockfd, SOL_SOCKET, TCP_NODELAY, &val, sizeof(val));
	if (ret < 0)
		err("setsockopt TCP_NODELAY\n");

	return ret;
}

int SetReuseaddr(int sockfd)
{
	const int val = 1;
	int ret;

	ret = setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));
	if (ret < 0)
		err("setsockopt SO_REUSEADDR\n");

	return ret;
}

int SetKeepalive(int sockfd)
{
	const int val = 1;
	int ret;

	ret = setsockopt(sockfd, SOL_SOCKET, SO_KEEPALIVE, &val, sizeof(val));
	if (ret < 0)
		err("setsockopt SO_KEEPALIVE\n");

	return ret;
}


unsigned int open_uart(char *uart_port, char *uart_config)
{
	int uart_fd;
	int speed, data, stopb;
	unsigned char parity;
	unsigned int n;
	struct termios cfg;

	if ((uart_fd = open(uart_port, O_RDWR|O_NDELAY)) < 0) {
		err("could not open device %s", uart_port);
		return -1;
	}

	n = sscanf(uart_config, "%d-%d%c%d", &speed, &data, &parity, &stopb);
	if (n < 4) {
		err("invalid argument %1d from %s", read+1, uart_config);
		return -2;
	}
	if (setserial(uart_fd, &cfg, speed, data, parity, stopb) < 0) {
		err("could not initialize device %s", uart_port);
		return -3;
	}

	return uart_fd;
}


int soip2_connect_to_host(int fd, struct s_req_pkt *req, int soip_port)
{
	int ret = 0;
	struct sockaddr_in addr;
	int socket_fd = -1;
	//char ip[MAX_STR_LEN];

	//UART_PRINTF(fd, "Finding host (%s)...", req->target_ip);

	//create TCP socket
	socket_fd = ret = socket(PF_INET, SOCK_STREAM, 0);
	if (ret < 0) {
		err("Error on create TCP socket %d", ret);
		return ret;
	}

	SetNodelay(socket_fd);
	SetKeepalive(socket_fd);

	//fill out addr info
	addr.sin_family = AF_INET;
	//addr.sin_port = htons(SOIP2_PORT);
	addr.sin_port = htons(soip_port);
	addr.sin_addr.s_addr = inet_addr(req->target_ip);

	// Set non-blocking
	if ((ret = _nonblock_fd(socket_fd)))
		goto out;

	//UART_PRINTF(fd, "Connecting...\n\r");
	//connect() with timeout
	ret = connect (socket_fd, (struct sockaddr*)&addr, sizeof(addr));
	if (ret < 0) {
		socklen_t lon;
		int valopt;
		struct timeval tv;
		fd_set fds;

		if (errno != EINPROGRESS) {
			err("error connecting", strerror(errno));
			goto out;
		}
		info("EINPROGRESS in connect() - selecting");
		do {
			tv.tv_sec = TIMEOUT;
			tv.tv_usec = 0;
			FD_ZERO(&fds);
			FD_SET(socket_fd, &fds);
			ret = select(socket_fd+1, NULL, &fds, NULL, &tv);
			if (ret < 0 && errno != EINTR) {
				err("Error connecting %s", strerror(errno));
				goto out;
			}
			if (ret == 0) {
				err("Timeout in select()");
				ret = -2;
				goto out;
			}
			if (ret > 0) {
				// Socket selected for write
				lon = sizeof(int);
				if (getsockopt(socket_fd, SOL_SOCKET, SO_ERROR, (void*)(&valopt), &lon) < 0)
				{
					err("Error in getsockopt() %s", strerror(errno));
					ret = -1;
					goto out;
				}
				// Check the value returned....
				if (valopt) {
					err("Error in delayed connection() %s", strerror(valopt));
					/* Bruce150925. valopt is > 0. Should return < 0 for errors */
					ret = -3;
					goto out;
				}
				// Conntect() success!
				break;
			}
			err("EINTR?");
		} while (1);
	}

#if 0
	// Set blocking
	if ((ret = _block_fd(socket_fd)))
		goto out;
#endif

	return socket_fd;

out: //Failed
	if (socket_fd >= 0)
		close(socket_fd);

	return ret;
}

unsigned int handshake_host(int socket_fd, struct s_req_pkt *req)
{
	int ret;
	char ack[MAX_STR_LEN];

	_block_fd(socket_fd);

	ret = send(socket_fd, req, sizeof(struct s_req_pkt), 0);
	if (ret != sizeof(struct s_req_pkt)) {
		err("Error send req pkt! (%d)", ret);
		goto out;
	}

	//ToDo. Should have timeout check here
	ret = recv(socket_fd, ack, MAX_STR_LEN, 0);
	if (ret < 0) {
		err("Error recv ack pkt! (%d)", ret);
		goto out;
	}
	if (ret == 0) {
		err("Client disconnected?!");
		ret = -1;
		goto out;
	}

	//ToDo. Check ack!?
	ret = 0;
out:
	_nonblock_fd(socket_fd);
	return ret;
}

int handshake_client(int socket_fd, char *uart_port)
{
	int ret = 0;
	struct s_req_pkt req;
	char ack[MAX_STR_LEN];
	int uart_fd = -1;

	_block_fd(socket_fd);

	ret = recv(socket_fd, &req, sizeof(struct s_req_pkt), 0);
	if (ret != sizeof(struct s_req_pkt)) {
		err("Error recv req pkt! (%d)", ret);
		ret = -1;
		goto out;
	}
	info("Got request!uart_cfg(%s)", req.uart_config);
	//Create uart
	uart_fd = ret = open_uart(uart_port, req.uart_config);
	if (ret < 0) {
		strncpy(ack, "FAIL: to open UART (%d)", MAX_STR_LEN);
	} else {
		strncpy(ack, "OK:", MAX_STR_LEN);
	}
	info("Send ack");
	ret = send(socket_fd, ack, MAX_STR_LEN, 0);
	if (ret < 0) {
		err("Error send ack pkt! (%d)", ret);
		if (uart_fd > 0)
			close(uart_fd);
		
		ret = -1;
		goto out;
	}

	if (uart_fd > 0)
		ret = uart_fd;
out:
	_nonblock_fd(socket_fd);
	return ret;
}

/*
** Loop of Serial over IP
*/
int soip2_client_loop(int uart_fd, int socket_fd)
{
	int ret;
	fd_set fds;
	int uc, sc;
	char ub[MAXMESG], sb[MAXMESG];

	signal(SIGPIPE, SIG_IGN);
	while (1)
	{
		//char *end;

		sc = 0;
		uc = 0;
		FD_ZERO(&fds);
		FD_SET(uart_fd, &fds);
		FD_SET(socket_fd, &fds);

		ret = select(socket_fd+1, &fds, NULL, NULL, NULL);
		if (ret < 0) {
			err("Error connecting %s", strerror(errno));
			goto out;
		}
		if (ret == 0) {
			err("Timeout in client_loop(). Should never happen");
			ret = -2;
			goto out;
		}

		if (FD_ISSET(uart_fd, &fds))
		{
			/* Read client UART */
			memset(ub, 0, MAXMESG);
			uc = read(uart_fd, ub, MAXMESG);
			if ((uc < 0) && (errno != EAGAIN)) {
				ret = errno;
				err("uart read failed! (%s)", strerror(errno));
				goto out;
			}
		}
		if (FD_ISSET(socket_fd, &fds))
		{
			/* Read socket */
			memset(sb, 0, MAXMESG);
			sc = read(socket_fd, sb, MAXMESG);
			if ((sc <= 0) && (errno != EAGAIN)) {
				ret = errno;
				err("socket read failed! (%s)", strerror(errno));
				goto out;
			}
		}

		if (sc) {
			/* Parse "^N" */
			/*
			end = strchr(sb, CTRL_CODE);
			if (end)
				sc = end - sb;
			*/

			/* Send to client UART */
			ret = block_write(uart_fd, sb, sc);
			if (ret < 0)
				goto out;
			/*
			if (end) //go out if ctrl code found.
				goto out;
			*/
		}

		if (uc) {
			/* Parse "^N" */
			/*
			end = strchr(ub, CTRL_CODE);
			if (end)
				uc = end - ub + 1; //Pass ctrl code to client.
			*/
			/* Send to socket. Includes the CTRL_CODE. */
			ret = block_write(socket_fd, ub, uc);
			if (ret < 0)
				goto out;
			/*
			if (end) //go out if ctrl code found.
				goto out;
			*/
		}

	}


out:
	signal(SIGPIPE, SIG_DFL);
	return ret;
}


int soip2_host_loop(int uart_fd, int socket_fd)
{
	int ret;
	fd_set fds;
	int uc, sc;
	char ub[MAXMESG], sb[MAXMESG];

	signal(SIGPIPE, SIG_IGN);
	while (1)
	{
		sc = 0;
		uc = 0;
		FD_ZERO(&fds);
		FD_SET(uart_fd, &fds);
		FD_SET(socket_fd, &fds);

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

		if (FD_ISSET(uart_fd, &fds))
		{
			/* Read host UART */
			memset(ub, 0, MAXMESG);
			uc = read(uart_fd, ub, MAXMESG);
			if ((uc < 0) && (errno != EAGAIN)) {
				ret = errno;
				err("uart read failed! (%s)", strerror(errno));
				goto out;
			}
		}
		if (FD_ISSET(socket_fd, &fds))
		{
			/* Read socket */
			memset(sb, 0, MAXMESG);
			sc = read(socket_fd, sb, MAXMESG);
			if ((sc <= 0) && (errno != EAGAIN)) {
				ret = errno;
				err("socket read failed! (%s)", strerror(errno));
				goto out;
			}
		}

		if (sc) {
			//char *end;
			/* Parse "^N" */
			/*
			end = strchr(sb, CTRL_CODE);
			if (end) {
				info("Host disconnect!");
				sc = end - sb;
			}
			*/

			/* Send to host UART */
			ret = block_write(uart_fd, sb, sc);
			if (ret < 0)
				goto out;

			/*
			if (end) //go out if ctrl code found.
				goto out;
			*/
		}

		if (uc) {
			/* Send to socket */
			ret = block_write(socket_fd, ub, uc);
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
unsigned int do_soip2_test(char *uart_port, char *uart_config)
{
	int ret = 0;
	static char buf[MAXMESG];
	int cnt;
	fd_set fds;

	/*
	** Open UART first and Wait for Commands
	*/
	info("Open %s using %s\n", uart_port, uart_config);
	uart_fd = ret = open_uart(uart_port, uart_config);
	if (ret < 0)
		goto out;

	while (1)
	{
		FD_ZERO(&fds);
		FD_SET(uart_fd, &fds);

		info(".");
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

		if (FD_ISSET(uart_fd, &fds))
		{
			cnt = read(uart_fd, buf, MAXMESG);
			if (cnt <= 0) {
				ret = errno;
				err("uart read failed! (%s)", strerror(errno));
				goto out;
			}
			if (cnt)
			{
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

unsigned int do_soip2_client(char *uart_port, char *uart_config, char *host_ip, const int soip_port)
{
	int ret = 0;
	struct s_req_pkt req;
	int socket_fd = -1;
	//signal(SIGINT, gotint);
	//signal(SIGTERM, gotint);
	
	/*
	** Open UART first and Wait for Commands
	*/
	uart_fd = ret = open_uart(uart_port, uart_config);
	if (ret < 0)
		goto out;

	while (1)
	{
		memset(&req, 0, sizeof(struct s_req_pkt));
		strcat(req.target_ip, host_ip);
		dbg("Host ip is %s\n", req.target_ip);
		/*
		** Try to connect to client
		** Create socket. Set to non-blocking.
		** Connect() and select() with timeout
		*/
		socket_fd = ret = soip2_connect_to_host(uart_fd, &req, soip_port);
		if (ret < 0) {
			/*
			** retry after 100ms so that CPU won't be overloaded.
			*/
			usleep(100000);
			continue;
		}
		/*
		** Simple handshake.
		** Send UART configure to client.
		** Wait for client ACK.
		** ACK: Echo ====<1111>====
		*/
		/*ret = handshake_host(socket_fd, &req);
		if (ret) {
			close(socket_fd);
			socket_fd = -1;
			continue;
		}*/

		//UART_PRINTF(uart_fd, "\n\r====<Start of %s>====\n\r", req.target_name);

		soip2_client_loop(uart_fd, socket_fd);

		close(socket_fd);
		socket_fd = -1;

		//UART_PRINTF(uart_fd, "\n\r====<End of %s>====\n\r", req.target_name);
		
	}

out:

	info("Ending SoIP...\n");
	if (socket_fd > 0)
		close(socket_fd);

	if (uart_fd > 0)
		close(uart_fd);
	
	return ret;
}

static int soip2_create_event_listener(void)
{
	struct sockaddr_nl src_addr;
	int event_listener;
	
	event_listener = socket(PF_NETLINK, SOCK_RAW, NETLINK_USERSOCK);
	if (event_listener == -1) {
		err("Not root\n");
		goto done;
	}

	memset(&src_addr, 0, sizeof(src_addr));
	src_addr.nl_family = AF_NETLINK;
	src_addr.nl_pid = getpid();  /* self pid */
	/* interested in group 1<<0 */
	src_addr.nl_groups = -1;
	if (bind(event_listener, (struct sockaddr*)&src_addr, sizeof(src_addr))) {
		err("event listener bind failed\n");
		goto done;
	}

	return event_listener;
done:
	if (event_listener >= 0)
		close(event_listener);
	return -1;
}

int soip2_host_start_listen(int soip_port)
{
	int ret = -1;
	struct sockaddr_in addr;
	int sock= -1;

	/*
	** Create network socket
	*/
	//create TCP socket
	sock = ret = socket(AF_INET, SOCK_STREAM, 0);
	if (ret < 0) {
		err("Error on create TCP socket %d", ret);
		return ret;
	}

	SetReuseaddr(sock);
	SetNodelay(sock);
	SetKeepalive(sock);

	//fill out addr info
	addr.sin_family = AF_INET;
	//addr.sin_port = htons(SOIP2_PORT);
	addr.sin_port = htons(soip_port);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);

	ret = bind(sock, (struct sockaddr *)&addr, sizeof(struct sockaddr_in));
	if (ret < 0) {
		err("Error on bind %s", strerror(errno));
		goto out;
	}

	ret = listen(sock, 10);
	if (ret != 0) {
		err("error listen %s", strerror(errno));
		goto out;
	}

	return sock;

out:
	if (sock >= 0)
		close(sock);
	
	return ret;
}

static int soip2_recv_event(int listener, t_msg_hdr *msg)
{
	struct sockaddr_nl dest_addr;
	struct nlmsghdr *nlh = NULL;
	struct iovec iov;
	struct msghdr msghdr;
	int len = 0;
	unsigned char tmp[NLMSG_SPACE(MAX_PAYLOAD)];
	//char tempstr[MAX_PAYLOAD];
	
	memset(&dest_addr, 0, sizeof(dest_addr));
	nlh = (struct nlmsghdr *)tmp;
#if 0
	nlh = (struct nlmsghdr *)malloc(NLMSG_SPACE(MAX_PAYLOAD));
	if (!nlh)
		err("failed to allocate nlh\n");
#endif

	memset(nlh, 0, NLMSG_SPACE(MAX_PAYLOAD));
	memset(&iov, 0, sizeof(iov));
	memset(&msghdr, 0, sizeof(msghdr));
	iov.iov_base = (void *)nlh;
	iov.iov_len = NLMSG_SPACE(MAX_PAYLOAD);
	msghdr.msg_name = (void *)&dest_addr;
	msghdr.msg_namelen = sizeof(dest_addr);
	msghdr.msg_iov = &iov;
	msghdr.msg_iovlen = 1;

	len = recvmsg(listener, &msghdr, 0);
	if (len <= 0) {
		err("recvmsg failed?! (%d)\n", len);
	}
	if (!NLMSG_OK(nlh, len))
		err("invalid nlh?!\n");

	/* we got some data from event socket. Send it out. */
	if (strncmp(SOIP2_MSG_TAG, NLMSG_DATA(nlh), strlen(SOIP2_MSG_TAG)) != 0) {
		//This is not what we interested. ignore it.
		dbg("ignore event:%s\n", NLMSG_DATA(nlh));
		len = 0;
		goto done;
	}

	//Found!
	//strncpy(tempstr, NLMSG_DATA(nlh), MAX_PAYLOAD);
	/*
	len = strlen(msg->data);
	if (len > MAX_PAYLOAD) {
		len = MAX_PAYLOAD;
		msg->data[len] = 0; //null-end in case of....
	}
	*/
	info("SOIP2 got event:%s\n", NLMSG_DATA(nlh));
	//copy only the ip part, no need to have SOIP2_MSG_TAG
	strncpy(msg->ip, 
	       ((unsigned char*)NLMSG_DATA(nlh)) + strlen(SOIP2_MSG_TAG), 
	       8 );
	dbg("SOIP2 got disconnect event with ip:%s\n", msg->ip);

	//msg->MgcNum = MSG_MAGIC_NUM;
	//msg->CmdType = MSG_CMDTYPE_ACTION;
	//msg->DataSize = len + 1; //plus null end
	//msg->reserved = 0;
	
	len = strlen(msg->ip);
	
done:
#if 0
	if (nlh)
		free(nlh);
#endif
	return len;
}

static void soip2_remove_connection(int fd)
{
	/* remove from master set */
	if (fd == fdmax)
		fdmax--;
	
	dbg("soip2 Client on socket %d removed..\n", fd);

	//if the removed client holds the token, give it back to host
	if (token_store == fd)
		token_store = listener_fd;

	FD_CLR(fd, &master_fd);
	/* remove from client info list */
	client_list[fd].ip = 0;
	/* close it... */
	close(fd);
}

static void _soip2_remove_connection_by_ip(unsigned long nIp)
{
	int i;
	for (i = 0; i <= fdmax; i++)
	{
		if (client_list[i].ip == nIp)
		{
			//dbg("soip2 client_list[i].ip (%X) nIp (%X)\n", client_list[i].ip, nIp);
			dbg("soip2 found disconnected client on socket %d removed it..\n", i);
			soip2_remove_connection(i);
		}
	}	
}

static void soip2_remove_connection_by_ip(char *ip)
{
	//int ret;
	unsigned long nIp;

	nIp = strtoul(ip, NULL, 16);
	_soip2_remove_connection_by_ip(nIp);
}

static int soip2_new_connection(int lfd)
{
	int newfd;
	int addrlen;
	int i;
	/* client address */
	struct sockaddr_in clientaddr;
	unsigned long nIp;

	addrlen = sizeof(clientaddr);
	if((newfd = accept(lfd, (struct sockaddr *)&clientaddr, &addrlen)) == -1)
	{
		err("client-accept() error!");
		return -1;
	}
	else
	{
		/*
		SetReuseaddr(newfd);
		SetNodelay(newfd);
		SetKeepalive(newfd);

		for (i=0; i<=fdmax; i++)
		{
			if (client_list[i].ip == (unsigned long)clientaddr.sin_addr.s_addr)
			{
				dbg("soip2 removing client on socket %d \n", i);
				soip2_remove_connection(i);
			}
		}
		*/
		nIp = (unsigned long)clientaddr.sin_addr.s_addr;
		info("SOIP2 connection from %s on socket %d\n", inet_ntoa(clientaddr.sin_addr), newfd);
		_soip2_remove_connection_by_ip(nIp);
		client_list[newfd].ip = nIp;
		return newfd;
	}
}

static int do_send_out(int uc, char *ubuff, int client_token)
{
	int ret = -1;
	ret = block_send(client_token, ubuff, uc);
	if (ret < 0)
	{
		//write failed, assume client is down, remove its connection
		err("socket write failed! (%s), remove connection..", strerror(errno));
		soip2_remove_connection(client_token);
		return 0;
	}
	else
		return 1;
}

static int do_broadcast(int uc, char *ubuff)
{
	//this function sends all host uart data to all clients with valid connection (broadcast)
	//if a client has a valid connection, the ip mustn't be 0
	int i;
	int ret = -1;
	int client_count = 0;
	
	/* Send to every client. */
	for (i=0; i<=fdmax; i++)
	{
		if(!FD_ISSET(i, &master_fd))
			continue;
		if (i == listener_fd)
			continue;
		if (i == uart_fd)
			continue;
		if (client_list[i].ip == 0)
			continue;
		ret = block_send(i, ubuff, uc);
		if (ret < 0)
		{
			//write failed, assume client is down, remove its connection
			err("socket write failed! (%s), remove connection..", strerror(errno));
			soip2_remove_connection(i);
			//client_count--;
		}
		else
			client_count++;
	}

	return client_count;
}


int do_soip2_host(char *uart_port, char *uart_config, const int token_time, const int soip_port)
{
	fd_set read_fds;
	int i;
	int ret = -1;
	int uc, sc;
	char ub[MAXMESG], sb[MAXMESG];
	time_t last_check_time = time(NULL);
	int time_remain = 0;
	int socket_fd;
	int event_fd = -1;
	int num_bytes;
	t_msg_hdr msg;

	FD_ZERO(&master_fd);
	FD_ZERO(&read_fds);
	memset(client_list, 0, sizeof(t_c_info)*FD_SETSIZE);

	#if 0
	uart_fd = open_uart(uart_port, uart_config);
	if (uart_fd < 0)
	{
		goto done;
	}

	if ((listener_fd = soip2_host_start_listen(soip_port)) == -1)
	{
		goto done;
	}

	if ((event_fd = soip2_create_event_listener()) == -1)
	{
		goto done;
	}
	#else
	uart_fd = -1;
	listener_fd = -1;
	event_fd = -1;
	while(1)
	{
		if(uart_fd < 0)
			uart_fd = open_uart(uart_port, uart_config);

		if(listener_fd < 0)
			listener_fd = soip2_host_start_listen(soip_port);

		if(event_fd < 0)
			event_fd = soip2_create_event_listener();

		if((uart_fd >= 0)&&(listener_fd >= 0)&&(event_fd >= 0))
		{
			break;
		}
		else
		{
			printf("uart_fd=%d listener_fd=%d event_fd=%d\n",uart_fd,listener_fd,event_fd);
			sleep(5);
		}
	}

	#endif

	FD_SET(uart_fd, &master_fd);
	if(uart_fd > fdmax)
	{ /* keep track of the maximum */
		fdmax = uart_fd;
	}
	FD_SET(listener_fd, &master_fd);
	/* keep track of the biggest file descriptor */
	if(listener_fd > fdmax)
	{ /* keep track of the maximum */
		fdmax = listener_fd;
	}
	FD_SET(event_fd, &master_fd);
	if(event_fd > fdmax)
	{ /* keep track of the maximum */
		fdmax = event_fd;
	}

	//At start, no client should have token, host holds token
	token_store = listener_fd;
	//signal(SIGINT, gotint);
	//signal(SIGTERM, gotint);
	for(;;)
	{
			struct timeval timeout;
	
			for(;;) {
			
				/* copy it */
				read_fds = master_fd;
	
				timeout.tv_usec = 0;
				timeout.tv_sec = TOKEN_CHECK_INTERVAL;
				ret = select(fdmax+1, &read_fds, NULL, NULL, &timeout);
				if(ret < 0) {
					err("Client-select() error lol!");
					goto done;
				}
				//dbg("T(%d)(%d)\n", time(NULL), last_check_time);
				if (abs(time(NULL) - last_check_time) >= TOKEN_CHECK_INTERVAL ) {
					//Do the periodic check.
					if (time_remain <=0)
					{
						token_store = listener_fd;
						time_remain = 0;
					}
					else
						time_remain--;
					//update check_time
					last_check_time = time(NULL);
				}
				if (ret > 0) {
					dbg("soip2 select() OK...\n");
					/*
					** Bruce141204. A hack to allow immediately release token when token_time is set to 0
					*/
					if (token_time == 0) {
						token_store = listener_fd;
					}
					break;
				}
			}
	 
			/*run through the existing connections looking for data to be read*/
			for(i = 0; i <= fdmax; i++)
			{
				if(FD_ISSET(i, &read_fds))
				{ /* we got one... */
					if(i == event_fd)
					{
						//we get event from msg_channel for client drop out notification
						num_bytes = soip2_recv_event(event_fd, &msg);
						//client disconnect event arrived
						if (num_bytes > 0) 
						{
							soip2_remove_connection_by_ip(msg.ip);
						}
					}
					else if(i == listener_fd)
					{
						/* handle client connections */
						socket_fd = soip2_new_connection(listener_fd);
						if (socket_fd >= 0)
						{
							FD_SET(socket_fd, &master_fd); /* add to master set */
							if(socket_fd > fdmax)
							{ /* keep track of the maximum */
								fdmax = socket_fd;
							}
							dbg("soip2 new client connection on socket %d! (%s)", socket_fd, strerror(errno));
						}
					}
					else if (i == uart_fd)
					{
						memset(ub, 0, MAXMESG);
						uc = read(i, ub, MAXMESG);
						if ((uc < 0) && (errno != EAGAIN))
						{
							ret = errno;
							err("host uart read failed! (%s)", strerror(errno));
							goto done;
						}
						
						if (uc)
						{
							int client_count = 0;
							if(token_store != listener_fd) //if token is issued to client
							{
								dbg("called by do_send_out! (%s)", strerror(errno));
								client_count = do_send_out(uc, ub, token_store);
							}
							else //if no client holds token, write to everyone that has connection (with ip present)
							{
								dbg("called by do_broadcast! (%s)", strerror(errno));
								client_count = do_broadcast(uc, ub);
							}
							dbg("soip2 client count! (%d)", client_count);
							if(client_count <= 0)//no client connected, token give back to host
								token_store = listener_fd;
						}
					}
					else
					{ //the fds that we are interested, client has transmission coming
						dbg("socket(%d) event, token_store=%d, time_remain=%d, listener_fd=%d", i, token_store, time_remain, listener_fd);
						memset(sb, 0, MAXMESG);
						sc = read(i, sb, MAXMESG);
						if ((sc <= 0) && (errno != EAGAIN))
						{
							ret = errno;
							err("socket read failed! (%d)(%s)", ret, strerror(errno));
							//goto done;
							soip2_remove_connection(i);
							continue;
						}
						
						if (token_store == listener_fd)
						{
							token_store = i;
							time_remain = token_time;
						}
						else
						{
							if(token_store == i)
								time_remain = token_time;
						}
						
						if(token_store == i) //client holds the token, if not discard its transmission
						{
							ret = block_write(uart_fd, sb, sc);
							if (ret < 0)
							{
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

	for(i = 0; i <= fdmax; i++)
	{
		if(FD_ISSET(i, &master_fd))
			close(i);
	}
	if (uart_fd > 0)
		close(uart_fd);

	exit (ret);

}


int main(int argc, char **argv)
{
	char *uart_port;
	char *uart_config;
	char *host_ip;
	int soip_port = SOIP2_PORT;
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

		c = getopt_long(argc, argv, "hcf:d:b:to:p:", longopts, &index);

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
			case '?':
				cmd = cmd_help;
				break;
			case 'p':
				soip_port = atoi(optarg);
				break;
			default:
				err("getopt\n");
		}
	}

	switch (cmd) {
		case cmd_host:
			do_soip2_host(uart_port, uart_config, token_time, soip_port);
			break;
		case cmd_client:
			do_soip2_client(uart_port, uart_config, host_ip, soip_port);
			break;
		case cmd_test:
			do_soip2_test(uart_port, uart_config);
			break;
		case cmd_help:
			help();
			break;
		default:
			err("unknown cmd\n");
			help();
	}

	return 0;

}



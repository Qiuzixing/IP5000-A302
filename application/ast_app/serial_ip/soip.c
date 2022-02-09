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

#include <getopt.h>
#include <netdb.h> //resolve hostname
#include <string.h> //memset
#include <netinet/tcp.h> //For TCP

#include "astdebug.h"

#define MAXMESG 2048
#define SOIP_PORT 6752
#define CTRL_CODE 0x0E /* Ctrl+N */
#define TIMEOUT 2 // 2 seconds

#define MAX_STR_LEN   64
#define CMD_MAGIC     0x6752
#define CMD_CONNECT   "ast_c"

/* Used to create ast-Rx-1111.local string. */
#define H_PREFIX   "-Rx-"
#define H_TAIL     ".local"


struct s_req_pkt
{
	unsigned int magic_num;
	char req[MAX_STR_LEN]; //CMD_CONNECT, etc...
	char client_name[MAX_STR_LEN]; //ast-Rx-1111.local
	char uart_config[MAX_STR_LEN/2]; //115200-8n1	
	char ctrl_code[MAX_STR_LEN/2]; //default is ^N (0x0E)
};

/* Globals */
int uart_fd = -1;
int socket_fd = -1;
int listen_fd = -1; //For client use only.
char hostname_prefix[MAX_STR_LEN];
char ctrl_code = CTRL_CODE;

static const struct option longopts[] = {
	{"host",	no_argument,	NULL, 'h'},
	{"client",	no_argument,	NULL, 'c'},
	{"UART_port", required_argument,	NULL, 'd'},
	{"UART_config", required_argument,	NULL, 'b'},
	{"hostname_prefix", required_argument,	NULL, 'p'},
	{"test", no_argument,	NULL, 't'},
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
		return 0;
	
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

void gotint(int x){
	if (uart_fd > 0) {
		tcflush(uart_fd, TCIOFLUSH);
		close(uart_fd);
	}

	if (socket_fd > 0) {
		//Send a CTRL_CODE to the other end to indicate disconnect.
		char c = ctrl_code;
		block_write(socket_fd, &c, 1);
		close(socket_fd);
	}

	if (listen_fd > 0) {
		close(listen_fd);
	}
	
	printf("SoIP exiting.\n");
	exit(1);
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


int resolve_hostname(char *hostname, char *ipv4)
{
	struct hostent *host = NULL;

	/*ToDo. validate input*/
	/*ToDo. Add timeout?*/

	host = gethostbyname(hostname);
	if (host == NULL){ //Unknown host 
		err("Unknown host?!:%s", hostname);
		return -1;
	}

	sprintf(ipv4, "%d.%d.%d.%d\0", host->h_addr[0], host->h_addr[1], host->h_addr[2], host->h_addr[3]);

	info("The IP address of %s is:%s", hostname, ipv4);
	
	return 0;
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

char * wait_for_cmd(int fd)
{
	int i, n;
	fd_set rdy;
	static char buf[MAXMESG];
	int start_ptr = -1;
	int wrt_ptr = 0;

	memset(buf, 0, MAXMESG);

	FD_ZERO(&rdy);
	FD_SET((unsigned int)fd, &rdy);

	do {
		i = select(fd+1, &rdy, NULL, NULL, NULL);
		if (i < 0) {
			err("wait for cmd failed?!(%d)", i);
			return NULL;
		}
		if (i == 0) {
			err("should never happen");
			return NULL;
		}

		if (FD_ISSET(fd, &rdy)) {
			/* Read all available data once and parse it latter. */
			n = read(fd, buf + wrt_ptr, (MAXMESG - wrt_ptr));
		} else {
			err("unknown case?!select return with fd not ready?");
			return NULL;
		}

		for (i = wrt_ptr; i < (wrt_ptr + n); i++) {
			if (buf[i] == CTRL_CODE) {
				UART_PRINTF(fd, "\n\r>");
				start_ptr = i + 1;
				continue;
			}

			if (start_ptr != -1) {
				block_write(fd, &buf[i], 1);
				if (buf[i] == '\n' || buf[i] == '\r') {
					UART_PRINTF(fd, "\n\r");
					/* Handle the "^N\n" case. */
					if (start_ptr == i) {
						start_ptr = -1;
						continue;
					}
					/* OK. It is a valid cmd format. Return. */
					buf[i] = '\0';
					return (buf + start_ptr);
				}
			}
		}

		if (start_ptr == -1) {
			memset(buf, 0, n);
			wrt_ptr = 0;
		} else {
			wrt_ptr += n;
			/* Handle the wrap case. */
			if ((wrt_ptr + 1) == MAXMESG) {
				err("cmd buffer overflow is unlikely to happen?!");
				/* Copy the valid data (from start_ptr) back to the buffer head. */
				memcpy(buf, buf + start_ptr, (MAXMESG - start_ptr));
				wrt_ptr = (MAXMESG - start_ptr);
				start_ptr = 0;
			}
		}

	} while (1);
}

unsigned int wait_for_connect_req(int uart_fd, struct s_req_pkt *req)
{
	unsigned int ret = 0, n;
	char *cmd;
	int t;

	do 
	{
		cmd = wait_for_cmd(uart_fd);
		if (cmd == NULL) {
			ret = -4;
			goto out;
		}

		if (strncmp(cmd, CMD_CONNECT, strlen(CMD_CONNECT))) {
			UART_PRINTF(uart_fd, "Unknown Command:(%s)\n\r", cmd);
			continue;
		}

		/*
		**Parse "connect" command and arguments
		**Format: ast_c <client_name> <baud_rate>
		**cmd: ast_c 
		**to: 1111 => ast-client1111.local
		**baud rate: 115200-8n1
		*/
		memset(req, 0, sizeof(struct s_req_pkt));
		
		strcpy(req->client_name, hostname_prefix);
		/* Bruce170509. Changed Format. Now hostname_prefix contains both astparam's hostname_prefix+hostname_middle */
		/* strcat(req->client_name, H_PREFIX); */
		n = sscanf(cmd,"%s %s %s %x", req->req, (req->client_name + strlen(req->client_name)), req->uart_config, req->ctrl_code);
		if(n < 3){
			UART_PRINTF(uart_fd, "Invalid argument (%s)\n\r", cmd);
			err("invalid argument (%s)", cmd);
			continue;
		}
		/* New supported control code setting. It is optional. */
		if (req->ctrl_code[0] != 0) {
			ctrl_code = req->ctrl_code[0];
		} else {
			ctrl_code = CTRL_CODE;
		}
		n = sscanf(req->uart_config, "%d-%d%c%d", &t, &t, &t, &t);
		if (n < 4) {
			UART_PRINTF(uart_fd, "Invalid UART argument (%s)\n\r", req->uart_config);
			continue;
		}

		strcat(req->client_name, H_TAIL);
		info("client_name:%s", req->client_name);
		req->magic_num = CMD_MAGIC;
		break;
	} while (1);

out:
	return ret;
}

int connect_to_client(int fd, struct s_req_pkt *req)
{
	int ret = 0;
	struct sockaddr_in addr;
	int socket_fd = -1;
	char ip[MAX_STR_LEN];

	UART_PRINTF(fd, "Finding client (%s)...", req->client_name);
	//resolve host name
	ret = resolve_hostname(req->client_name, ip);
	if (ret) {
		UART_PRINTF(fd, "Unknown client\n\r");
		return ret;
	}
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
	addr.sin_port = htons(SOIP_PORT);
	addr.sin_addr.s_addr = inet_addr(ip);

	// Set non-blocking
	if ((ret = _nonblock_fd(socket_fd)))
		goto out;

	UART_PRINTF(fd, "Connecting...\n\r");
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
					ret = valopt;
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
	info("Got request!uart_cfg(%s)\n", req.uart_config);
	info("ctrl_code=0x%02X\n", req.ctrl_code[0]);
	if (req.ctrl_code[0] != 0) {
		ctrl_code = req.ctrl_code[0];
	} else {
		ctrl_code = CTRL_CODE;
	}
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
int host_loop(int uart_fd, int socket_fd)
{
	int ret;
	fd_set fds;
	int uc, sc;
	char ub[MAXMESG], sb[MAXMESG];

	signal(SIGPIPE, SIG_IGN);
	while (1)
	{
		char *end;

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
			/* Parse "^N" */
			end = strchr(sb, ctrl_code);
			if (end)
				sc = end - sb;

			/* Send to host UART */
			//info("From Client:\n%s", sb);
			if ((ret = block_write(uart_fd, sb, sc)))
				goto out;

			if (end) //go out if ctrl code found.
				goto out;
		}

		if (uc) {
			/* Parse "^N" */
			end = strchr(ub, ctrl_code);
			if (end)
				uc = end - ub + 1; //Pass ctrl code to client.

			/* Send to socket. Includes the CTRL_CODE. */
			//info("To Client(%d):\n%s", uc, ub);
			if ((ret = block_write(socket_fd, ub, uc)))
				goto out;

			if (end) //go out if ctrl code found.
				goto out;
		}

	}


out:
	signal(SIGPIPE, SIG_DFL);
	return ret;
}


int client_loop(int uart_fd, int socket_fd)
{
	int ret;
	fd_set fds;
	int uc, sc;
	char ub[MAXMESG], sb[MAXMESG];
	char *end;

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
			err("Timeout in client_loop(). Should never happen");
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
			/* Parse "^N" */
			end = strchr(sb, ctrl_code);
			if (end) {
				info("Host disconnect!");
				sc = end - sb;
			}

			/* Send to client UART */
			//info("From Host(%d):%s", sc, sb);
			if ((ret = block_write(uart_fd, sb, sc)))
				goto out;

			if (end) //go out if ctrl code found.
				goto out;
		}

		if (uc) {
			/* Parse "^N" */
			end = strchr(ub, ctrl_code);
			if (end)
				uc = end - ub + 1; //Pass ctrl code to host.

			/* Send to socket */
			//info("To Host:%s", ub);
			if ((ret = block_write(socket_fd, ub, uc)))
				goto out;

			if (end) //go out if ctrl code found.
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
unsigned int do_soip_test(char *uart_port, char *uart_config)
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
			if (cnt < 0) {
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

unsigned int do_soip_host(char *uart_port, char *uart_config)
{
	int ret = 0;
	struct s_req_pkt req;

	signal(SIGINT, gotint);
	signal(SIGTERM, gotint);
	
	/*
	** Open UART first and Wait for Commands
	*/
	uart_fd = ret = open_uart(uart_port, uart_config);
	if (ret < 0)
		goto out;

	while (1)
	{
		ret = wait_for_connect_req(uart_fd, &req);
		if (ret)
			goto out;

		/*
		** Try to connect to client
		** Create socket. Set to non-blocking.
		** Connect() and select() with timeout
		*/
		socket_fd = ret = connect_to_client(uart_fd, &req);
		if (ret < 0)
			continue;

		/*
		** Simple handshake.
		** Send UART configure to client.
		** Wait for client ACK.
		** ACK: Echo ====<1111>====
		*/
		ret = handshake_host(socket_fd, &req);
		if (ret) {
			close(socket_fd);
			socket_fd = -1;
			continue;
		}

		UART_PRINTF(uart_fd, "\n\r====<Start of %s>====\n\r", req.client_name);

		host_loop(uart_fd, socket_fd);

		close(socket_fd);
		socket_fd = -1;

		UART_PRINTF(uart_fd, "\n\r====<End of %s>====\n\r", req.client_name);
		
	}

out:

	info("Ending SoIP...\n");
	if (socket_fd >= 0)
		close(socket_fd);

	if (uart_fd > 0)
		close(uart_fd);
	
	return ret;
}


int client_start_listen(void)
{
	int ret = -1;
	struct sockaddr_in addr;
	int sock= -1;

	/*
	** Create network socket
	*/
	//create TCP socket
	sock = ret = socket(PF_INET, SOCK_STREAM, 0);
	if (ret < 0) {
		err("Error on create TCP socket %d", ret);
		return ret;
	}

	SetReuseaddr(sock);
	SetNodelay(sock);
	SetKeepalive(sock);

	//fill out addr info
	addr.sin_family = AF_INET;
	addr.sin_port = htons(SOIP_PORT);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);

	ret = bind(sock, (struct sockaddr *)&addr, sizeof(struct sockaddr_in));
	if (ret < 0) {
		err("Error on bind %s", strerror(errno));
		goto out;
	}

	ret = listen(sock, 1);
	if (ret != 0) {
		err("error listen %s", strerror(errno));
		goto out;
	}

	return sock;

out:
	if (sock > 0)
		close(sock);
	
	return ret;
}


int do_soip_client(char *uart_port)
{
	int ret = 0;
	
	listen_fd = ret = client_start_listen();
	if (ret < 0)
		goto out;
	
	signal(SIGINT, gotint);
	signal(SIGTERM, gotint);
	while (1)
	{
		/*
		** accept connection request
		*/
		socket_fd = ret = accept(listen_fd, NULL, NULL);
		if(ret < 0) {
			err("error accept %s", strerror(errno));
			continue;
		}

		/*
		** Simple handshake.
		** Wait for UART configure
		** Open UART
		** Send host an ACK.
		*/
		uart_fd = ret = handshake_client(socket_fd, uart_port);
		if (ret < 0) {
			close(socket_fd);
			socket_fd = -1;
			continue;
		}

		client_loop(uart_fd, socket_fd);
		close(uart_fd);
		uart_fd = -1;
		close(socket_fd);
		socket_fd = -1;

	}

out:
	return ret;
}


int main(int argc, char **argv)
{
	char *uart_port;
	char *uart_config;
	
	enum {
		cmd_host,
		cmd_client,
		cmd_test,
		cmd_help
	} cmd = cmd_help;
	
	// Initial default hostname_prefix value
	strcpy(hostname_prefix, "ast");
	
	for (;;) {
		int c;
		int index = 0;

		c = getopt_long(argc, argv, "hcd:b:tp:", longopts, &index);

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
			case 'd':
				uart_port = optarg;
				break;
			case 'b':
				//Only required by host
				uart_config = optarg;
				break;
			case 'p':
				//Only required by host
				strncpy(hostname_prefix, optarg, MAX_STR_LEN);
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
			do_soip_host(uart_port, uart_config);
			break;
		case cmd_client:
			do_soip_client(uart_port);
			break;
		case cmd_test:
			do_soip_test(uart_port, uart_config);
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



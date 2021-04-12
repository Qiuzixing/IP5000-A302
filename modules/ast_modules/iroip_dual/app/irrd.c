/*
 * Copyright (c) 2017
 * ASPEED Technology Inc. All Rights Reserved
 * Proprietary and Confidential
 *
 * By using this code you acknowledge that you have signed and accepted
 * the terms of the ASPEED SDK license agreement.
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/errno.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include "ir_ioctl.h"
#include "ir_rs.h"
#include "ir_nec.h"

#define RECV_SIZE 32768
#define IR_BUF_SIZE (IR_SIZE_PAIR_MAX * 2)
#define IR_BUF_SIZE_TEXT ((IR_SIZE_PAIR_MAX * 2) * 5) /* 4 character + 1 space */

struct ast_ir {
	u32 state;
	u32 size[2];
	u32 pulse;
	u32 space;
	u32 pair;

	u32 index_t; /* for conut on T */

	u32 track_num;
	u32 track_start;
	u32 track_done;

	u32 freq;
	u32 freq_temp;

	u32 factor; /* factor compare to 1MHz; do 'sample count-to-time' transform if our IR clock is 2MHz, factor is 2 */

	u32 buf_index;
	u32 buf[IR_BUF_SIZE];
	u32 decode[IR_BUF_SIZE]; /* store pronto result */
	u8 decode_text[IR_BUF_SIZE_TEXT]; /* store pronto result */
};

void ir_pronto(struct ast_ir *ir)
{
	u32 i, divisor, fix;
	/*
	 * 1 - 0000 indicates raw IR data (you can ignore this value)
	 * 2 - P, frequency f = 1000000 / (P * 0.241246)
	 *	P * 0.241246 =  1000000 / f
	 *	P = (1000000 / f) / 0.241246 = (1000000 / f) * 1000000 / 241246
	 *		= (1000000 / f) * 500000 / 120623 = 4145146 / f
	 * 3 - length of the first burst pair sequence
	 * 4 - length of the second burst pair sequence
	 */
	ir->decode[0] = 0;
	ir->decode[1] = 4145146 / ir->freq;
	ir->decode[2] = ir->buf_index >> 1;
	ir->decode[3] = 0;

	divisor = 1000000 * ir->factor;
	fix = divisor >> 1; /* for rounding */

	for (i = 0; i < ir->buf_index; i++)
		ir->decode[i + 4] = (ir->buf[i] * ir->freq  + fix) / divisor;

#if defined(IR_APP_DEBUG)
	{
		u32 t;

		t = 4 + ir->buf_index;
		for (i = 0; i < t; i++) {
			if ((i % 0x10) == 0)
				msg("\n");
			msg("%.4x ", ir->decode[i]);
		}
		msg("\n");
	}
#endif
}


static void ir_init(struct ast_ir *ir)
{
	ir->state = 0;
	ir->size[0] = 0;
	ir->size[1] = 0;
	ir->pulse = 0;
	ir->space = 0;
	ir->pair = 0;

	ir->index_t = -1;
	ir->track_num = 16;
	ir->track_start = 2;
	ir->track_done = 0;
	ir->freq = 0;
	ir->freq_temp = 0;

	ir->factor = 2;
	ir->buf_index = 0;
	memset(ir->buf, 0, (IR_BUF_SIZE << 2));
}

int ir_space_pop(struct ast_ir *ir)
{
	int ret;

	if (ir->buf_index == 0) {
		ir->space = 0;
		return 0;
	}

	ret = ir->space;
	ir->space = 0;

	if ((ir->buf_index & 0x1) == 0) {
		ir->buf[ir->buf_index] = 0;
		ir->buf_index++;
	}

	ir->buf[ir->buf_index] = ret;
	ir->buf_index++;

	return ret;
}

int ir_pulse_pop(struct ast_ir *ir)
{
	int ret;

	ret = ir->pulse;
	ir->pulse = 0;

	if ((ir->buf_index & 0x1) == 1) {
		ir->buf[ir->buf_index] = 0;
		ir->buf_index++;
	}

	//ir->buf[ir->buf_index] = ret / ir->factor;
	ir->buf[ir->buf_index] = ret;
	ir->buf_index++;

	return ret;
}

void ir_space_add(struct ast_ir *ir, int size)
{
	ir->space += size;
	ir->size[0] = 0; ir->size[1] = 0;
}

void ir_pulse_add(struct ast_ir *ir, int size)
{
	ir->pulse += size;
	ir->size[0] = 0; ir->size[1] = 0;
}

#define IR_LOW_MAX (33)

static void ir_freq(struct ast_ir *ir, u32 v, u32 repeat, u32 end)
{
	u32 time;

	if (ir->freq)
		return;

	/* 0 => 1 */
	if ((ir->state == 0) && (v == 1)) {
		ir->index_t++;

		if (ir->index_t <= ir->track_start)
			return;

		time = ir->size[0] + ir->size[1];

		if (time <= 0)
			return;

		if (ir->track_done == 0) {
			/* 30Khz, T = 33.33 micro sec, filter if > (2 * 33.33) */
			if (time < (IR_LOW_MAX << 1)) {
				ir->freq_temp += time;
				ir->track_done++;
			}
		} else {
			u32 avg, max, min;

			avg = ir->freq_temp / ir->track_done;
			max = avg << 1; min = avg >> 1;

			if ((time > min) && (time < max)) {
				ir->freq_temp += time;
				ir->track_done++;
			}
		}

		if ((ir->track_done >= ir->track_num) || (end == 1)) {
			if (ir->freq_temp > 0) {
				u32 period, freq;

				period = (ir->freq_temp / ir->track_done) / ir->factor; /* in micro second */
				freq =  1000000 * ir->factor * ir->track_done / ir->freq_temp; /* in Hz */

				ir->freq = freq;
			}
		}
	}
}

static void ir_update(struct ast_ir *ir, u32 v, u32 repeat, u32 end)
{
	if (repeat == 0) {
		if (end == 1)
			goto end_handle;
		return;
	}

	ir_freq(ir, v, repeat, end);

	if ((ir->state == 0) && (v == 1)) { /* 0 => 1 */
		u32 size;

		size = ir->size[0] + ir->size[1];
		if (size > 0) {
			if (ir->size[1] == 0) {
				/* never get high */
				ir_space_add(ir, ir->size[0]);
				ir_space_pop(ir);
			} else {
				ir_pulse_add(ir, size);
			}
		}
	}

	ir->state = v;

	if ((v == 0) && (repeat > (IR_LOW_MAX << 1))) {
		/* long silence */
		if (ir->size[1] > 0) {
			/* pop pulse, pulse += IR_LOW_MAX  */
			ir_pulse_add(ir, IR_LOW_MAX);
			ir_pulse_pop(ir);
			repeat -= IR_LOW_MAX;
		}
	}

	ir->size[v] += repeat;

	if (end == 1) {
end_handle:
		if (v == 0) {
			if (ir->size[1] > 0) {
				ir_pulse_add(ir, ir->size[0] + ir->size[1]);
				ir_pulse_pop(ir);
			} else {
				ir_space_add(ir, ir->size[0]);
				ir_space_pop(ir);
			}
		} else {
			ir_pulse_add(ir, ir->size[0] + ir->size[1]);
			ir_pulse_pop(ir);
			/* space padding, set size to ir->factor let ir_space_pop() get a non-zero value */
			ir_space_add(ir, ir->factor);
			ir_space_pop(ir);
		}
	}
}

static int ir_parse(struct ast_ir *ir, char *buf, u32 length)
{
	u32 index, index_fix, v, repeat, end;
#if (SRC_ENCODE_MODE == 0) /* Complex IR run-length code */
	short code;
#else
	char code;
#endif

	index = 0; end = 0;

	ir_init(ir);

	while (index < length) {
#if (SRC_ENCODE_MODE == 0) /* Complex IR run-length code */
		/* Complex IR run-length code */
		code = *(short *) (buf + index);
		if (code & 0x1) {
			index_fix = 1;
		} else {
			code = code & 0xff;
			index_fix = 0;
		}

		v = (code >> 1) & 0x1;
		repeat = (code >> 2) & 0x3FFF;
#else
		/* Simple IR run-length code */
		code = *(buf + index);
		v = code & 0x1;
		repeat = code >> 1;
#endif

		if ((index + 1 + index_fix) == length)
			end = 1;

		ir_update(ir, v, repeat, end);

		index += (1 + index_fix);
	}

	if ((ir->buf_index > 0) && (ir->freq)) {

		msg("\nfreq %d Hz\n", ir->freq);

		ir_pronto(ir);

		return 0;
	}

	return -1;
}

static int create_send_socket_for_driver(u16 port)
{
	int sockfd;
	IO_ACCESS_DATA IO_Data;
	int fd = open("/dev/iroip", O_RDONLY);

	sockfd = create_socket_to_local(port);

	memset(&IO_Data, 0, sizeof(IO_ACCESS_DATA));
	IO_Data.data = sockfd;
	ioctl(fd, IOCTL_IR_START_LOCAL, &IO_Data);
	close(fd);

	/* Close connection */
	close(sockfd);

	return 0;
}

char rec_buf[RECV_SIZE];

static int irrd_udp_rx_socket_fd(u16 port)
{
	int socket_fd; /* file description into transport */
	struct sockaddr_in myaddr; /* address of this service */

	/* Get a socket into UDP/IP */
	socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (socket_fd < 0) {
		perror("socket failed");
		goto exit;
	}

	/* Set up our address */
	bzero((char *)&myaddr, sizeof(myaddr));
	myaddr.sin_family = AF_INET;
	myaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	myaddr.sin_port = htons(port);

	/* Bind to the address to which the service will be offered */
	if (bind(socket_fd, (struct sockaddr *)&myaddr, sizeof(myaddr)) < 0) {
		perror("bind failed\n");
		close(socket_fd);
		socket_fd = -1;
	}

exit:
	return socket_fd;
}

void enable_keepalive(int sock)
{
	int yes, idle, interval, maxpkt;

	yes = 1;
	if (setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, &yes, sizeof(int))) {
		perror("SO_KEEPALIVE");
		return;
	}

	idle = 1;
	if (setsockopt(sock, IPPROTO_TCP, TCP_KEEPIDLE, &idle, sizeof(int))) {
		perror("TCP_KEEPIDLE");
		return;
	}

	interval = 1;
	if (setsockopt(sock, IPPROTO_TCP, TCP_KEEPINTVL, &interval, sizeof(int))) {
		perror("TCP_KEEPINTVL");
		return;
	}

	maxpkt = 8;
	if (setsockopt(sock, IPPROTO_TCP, TCP_KEEPCNT, &maxpkt, sizeof(int))) {
		perror("TCP_KEEPCNT");
		return;
	}
}

static ir_buffer_normalize(struct ast_ir *ir)
{
	int i;

	for (i = 0; i < ir->buf_index ; i++)
		ir->buf[i] = ir->buf[i] / ir->factor;
}

int main(int argc, char *argv[])
{
	int listen_fd, udp_rx_fd, tcp_fd = -1;
	int length; /* length of address structure */
	struct sockaddr_in myaddr; /* address of this service */
	struct sockaddr_in client_addr; /* address of client */

	fd_set master;    /* master file descriptor list */
	fd_set read_fds;  /* temp file descriptor list for select() */
	int fdmax;        /* maximum file descriptor number */
	int ret, reuseaddr = 1;
	int nbytes; /* the number of read */
	struct ast_ir ir;
	unsigned int nec_addr = 0;

	if (argc >= 2)
		nec_addr = (unsigned int)strtoul(argv[1], NULL, 0);

	udp_rx_fd = irrd_udp_rx_socket_fd(IR_R_D_PORT);

	if (udp_rx_fd < 0)
		exit(1);

	/* create a sock for driver to send IR data */
	create_send_socket_for_driver(IR_R_D_PORT);

	FD_ZERO(&master);
	FD_SET(udp_rx_fd, &master);
	fdmax = udp_rx_fd;

	/* Get a socket into TCP/IP */
	listen_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (listen_fd < 0) {
		perror("socket failed");
		exit(1);
	}

	/* Set up our address */
	bzero((char *)&myaddr, sizeof(myaddr));
	myaddr.sin_family = AF_INET;
	myaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	myaddr.sin_port = htons(IR_R_D_PORT);

	ret = setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &reuseaddr, sizeof(reuseaddr));

	if (ret < 0)
		perror("setsockopt (SO_REUSEADDR)\n");

	/* Bind to the address to which the service will be offered */
	if (bind(listen_fd, (struct sockaddr *)&myaddr, sizeof(myaddr)) < 0) {
		perror("bind failed");
		exit(1);
	}

	/* Set up the socket for listening, with a queue length of 1 */
	if (listen(listen_fd, 1) < 0) {
		perror("listen failed");
		exit(1);
	}

	FD_SET(listen_fd, &master);

	if (listen_fd > fdmax)
		fdmax = listen_fd;

	while (1) {
		read_fds = master; /* copy it, why? */
		ret = select(fdmax + 1, &read_fds, NULL, NULL, NULL);

		if (ret <= 0) {
			if (ret == 0)
				perror("select timeout");
			else
				perror("select error");

			exit(1);
		}

		if (tcp_fd != -1) {
			if (FD_ISSET(tcp_fd, &read_fds)) {
				nbytes = recv(tcp_fd, rec_buf, RECV_SIZE, 0);
				if (nbytes <= 0) { /* -1: error, 0: peer shutdown */
					FD_CLR(tcp_fd, &master);
					close(tcp_fd);
					tcp_fd = -1; /* set to -1 for new connection */
					/* connection closed,  ready to accept new one */
					FD_SET(listen_fd, &master);
				}
			}
		}

		if (FD_ISSET(udp_rx_fd, &read_fds)) {
			int tx_fd;
			struct _ir_net_header *phdr;

			nbytes = recvfrom(udp_rx_fd, rec_buf, RECV_SIZE, 0, (struct sockaddr *)&client_addr, (socklen_t *)&length);
			if (nbytes != -1) {
				phdr = (struct _ir_net_header *) rec_buf;
				if ((phdr->magic == IR_MAGIC_NUM) && (phdr->length == (u32)(nbytes - 16))) {
					char *dest = ir.decode_text;
					u32 num = 0, i;

					if (ir_parse(&ir, rec_buf + 16, nbytes - 16) == 0) {
						ir_buffer_normalize(&ir);
						ir_decode_nec(ir.freq, ir.buf, ir.buf_index, nec_addr);

						for (i = 0; i < ir.buf_index + 4; i++)  /* pronto header: 4 */
							num += snprintf(dest + num, IR_BUF_SIZE_TEXT - num, "%.4x ", ir.decode[i]);
						num += snprintf(dest + num, IR_BUF_SIZE_TEXT - num, "\n");

						tx_fd = create_socket_to_local(IR_R_PORT);
						sendto(tx_fd, ir.decode_text, num, 0, NULL, 0);
						close(tx_fd);

						if (tcp_fd >= 0) {
							int ret;

							ret = send(tcp_fd, ir.decode_text, num, MSG_NOSIGNAL); /* MSG_NOSIGNAL to avoid SIGPIPE if peer breaks connection */
							if (ret <= 0) {
								FD_CLR(tcp_fd, &master);
								close(tcp_fd);
								tcp_fd = -1; /* set to -1 for new connection */
								/* connection closed,  ready to accept new one */
								FD_SET(listen_fd, &master);
							}
						}
					}
				}
			}
		}

		if (FD_ISSET(listen_fd, &read_fds)) {
			if (tcp_fd == -1) {
				tcp_fd = accept(listen_fd, (struct sockaddr *)&client_addr, &length);

				if (tcp_fd >= 0) {
					enable_keepalive(tcp_fd);
					FD_SET(tcp_fd, &master);

					if (tcp_fd > fdmax)
						fdmax = tcp_fd;
					/* connection establishd, mask listen */
					FD_CLR(listen_fd, &master);
				} else {
					/* just print message, not exit */
					perror("could not accept call");
				}
			}
		}
	}

	close(udp_rx_fd);
}

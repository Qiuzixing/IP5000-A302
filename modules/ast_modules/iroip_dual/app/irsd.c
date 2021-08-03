/*
 * Copyright (c) 2017
 * ASPEED Technology Inc. All Rights Reserved
 * Proprietary and Confidential
 *
 * By using this code you acknowledge that you have signed and accepted
 * the terms of the ASPEED SDK license agreement.
 */
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <ctype.h>
#include "ir_rs.h"

#define IR_DMA_MAX_SIZE  0x3FFF
#define IR_DMA_MAX_SIZE_SHORT  0x3F
#define IR_FACTOR 2 /* XXX IR samplie clock is 2MHz*/

struct ir_dma {
	u8 type:1;
	u8 v:1;
	u8 repeat:6;
};

struct ir_dma_long {
	u16 type:1;
	u16 v:1;
	u16 repeat:14;
};

/*
 * IR_SIZE_TEXT_MAX is 4096 ~= 400 pluse-space pair
 * worst case: all long format (impossible) => 400 * 2 * 2 = 1600
 * extend IR_SIZE_DMA from 2048 to 4096: SONY format, pulse volume need more
 */
#define IR_SIZE_DMA 4096

struct ir_dma_buffer {
	unsigned int index;
	char buf[IR_SIZE_DMA];
};

struct ir_dma_buffer dma_buffer;

const char *sep = " , \n\r";
const char telnet_end_char = '\n'; /* is '\r''\n' => use '\n' */

int pronto_hex[IR_SIZE_TEXT_MAX];

struct gc_compress_info {
	u8 name;
	u32 pulse;
	u32 space;
};

#define GC_COMPRESS_PAIR_MAX 15 /* only support first 15 unique on-off pair */
static u32 gc_compress_info_valid = 0;
static struct gc_compress_info gc_c_info[GC_COMPRESS_PAIR_MAX];


static void gc_compress_info_init(void)
{
	gc_compress_info_valid = 0;
}

static int gc_compress_info_add(u32 pulse, u32 space)
{
	struct gc_compress_info *ptr;
	u32 i;

	ptr = gc_c_info;

	for (i = 0; i < gc_compress_info_valid; i++) {
		if ((ptr->pulse == pulse) && (ptr->space == space))
			break;
		ptr++;
	}

	if ((i == gc_compress_info_valid) && (gc_compress_info_valid < GC_COMPRESS_PAIR_MAX)) {
		ptr = gc_c_info + gc_compress_info_valid;
		ptr->name = 'A' + gc_compress_info_valid;
		ptr->pulse = pulse;
		ptr->space = space;
		gc_compress_info_valid++;
	}
}

static struct gc_compress_info *gc_c_info_by_name(u8 name)
{
	struct gc_compress_info *ptr;
	u32 i;

	ptr = gc_c_info;

	for (i = 0; i < gc_compress_info_valid; i++) {
		if (ptr->name == name)
			return ptr;
		ptr++;
	}

	return NULL;
}

static int gc_string_to_int(char *buf, unsigned int *dest)
{
	unsigned char *word, c;
	u32 done = 0, i, result, len;

	gc_compress_info_init();

	while (1) {

		word = strsep((char **)&buf, sep);

		if (!word)
			break;

		len = strlen(word);

		if (len == 0)
			continue;

		result = 0;

		for (i = 0; i < len; i++) {
			c = word[i];
			if (isdigit(c)) {
				result = result * 10 + c - '0';
			} else {
				struct gc_compress_info *ptr;

				if (result > 0) {
					dest[done] = result;
					done++;
					result = 0;

					/* on1:3, off1:4, on2:5, off2:6*/
					if ((done > 3) & (done & 0x1) == 1)
						gc_compress_info_add(dest[done - 2], dest[done - 1]);
				}

				/* search compressd pair */
				ptr = gc_c_info_by_name(c);

				if (ptr) {
					dest[done] = ptr->pulse;
					done++;
					dest[done] = ptr->space;
					done++;

					continue;
				}

				/* not found => ERROR, return 0 to skip global_cache_to_dma() */
				return 0;
			}
		}

		if (result > 0) {
			dest[done] = result;
			done++;
			/* on1:3, off1:4, on2:5, off2:6*/
			if ((done > 3) & (done & 0x1) == 1)
				gc_compress_info_add(dest[done - 2], dest[done - 1]);
		}
	}

	return done;
}

static int user_string_to_hex(char *buf, unsigned int *dest)
{
	unsigned char *word;
	int i = 0;

	while (1) {
		word = strsep((char **)&buf, sep);
		if (!word)
			break;

		if (strlen(word) == 0)
			continue;
		dest[i] = (unsigned int) strtoul(word, NULL, 16);
		i++;
	}

	return i;
}

void ir_pulse_from_pronto(unsigned int volume, unsigned int periodx1000000, struct ir_dma_buffer *p)
{
	u32 todo, work, half_t, i = 0;
	struct ir_dma *dma;

	half_t =  (periodx1000000 * IR_FACTOR / 2) / 1000000;
	todo = volume * (periodx1000000 / 100) * IR_FACTOR / 10000; /* volume * (periodx1000000 / 100) * IR_FACTOR * 100 / 1000000 */

	while (todo) {
		if (p->index >= IR_SIZE_DMA) /* too long to store in DMA */
			break;

		dma = (struct ir_dma *) (p->buf + p->index);

		/* todo is always N * half_t, never get else case */
		if (todo >= half_t)
			work = half_t;
		else
			work = todo;

		dma->type = 0;

		if (i & 0x1)
			dma->v = 0;
		else
			dma->v = 1;

		dma->repeat = work;
		p->index++;

		todo -= work;
		i++;
	}
}

void ir_space_from_pronto(unsigned int volume, unsigned int periodx1000000, struct ir_dma_buffer *p)
{
	unsigned int todo, work, size, half_t;

	half_t =  (periodx1000000 * IR_FACTOR / 2) / 1000000;
	todo = volume * (periodx1000000 / 100) * IR_FACTOR / 10000; /* volume * (periodx1000000 / 100) * IR_FACTOR * 100 / 1000000 */
	size = IR_DMA_MAX_SIZE;

	while (todo) {
		if (p->index >= IR_SIZE_DMA) /* too long to store in DMA */
			break;

		if (todo >= size)
			work = size;
		else
			work = todo;

		if (work > IR_DMA_MAX_SIZE_SHORT) {
			struct ir_dma_long *dma = (struct ir_dma_long *) (p->buf + p->index);

			dma->type = 1;
			dma->v = 0;
			dma->repeat = work;
			p->index += 2;
		} else {
			struct ir_dma *dma = (struct ir_dma *) (p->buf + p->index);

			dma->type = 0;
			dma->v = 0;
			dma->repeat = work;
			p->index++;
		}

		todo -= work;
	}
}

static void ir_dma_buffer_init(struct ir_dma_buffer *p)
{
	p->index = 16;
	memset(p->buf, 0, IR_SIZE_DMA);
}

static int global_cache_to_dma(int *buf, unsigned int num, struct ir_dma_buffer *pdb)
{
	/*
	 * sendir,<module:port>,<ID>,<freq>,<repeat>,<offset>,<on1>,<off1>,<on2>,<off2>,…,<onN>,<offN>
	 * where:
	 * <module:port> = address of the desired IR channel
	 * <ID> = 0|1|2|…|65535
	 * <freq> = 15000|…|500000 (in hertz)
	 * <repeat> = 1|2|…|20 (the IR command is sent <repeat> times)
	 * <offset> = 1|3|5|…|383 used if <repeat> is greater than 1
	 * <on1> = 4|5|…|50000  (number of pulses)
	 * <off1> = 4|5|…|50000 (absence of pulse periods of the carrier frequency)
	 * N = the count of <on>,<off> pairs, which must be less than 260 (total of 520 numbers)
	 *
	 * what we do is <freq>,<repeat>,<offset>,<on1>,<off1>,<on2>,<off2>,…,<onN>,<offN>
	 */

	struct _ir_net_header *header;
	u32 freq, tx1000000, repeat, offset, once_max, i;
	u32 data_offset_bias = 3;

	if (num == 0)
		return -1;

	freq = buf[0];
	repeat = buf[1];
	offset = buf[2];
	tx1000000 = (1000 * 1000000 / freq) * 1000; /* micro second, 1000000 * 1000000 / freq */

	ir_dma_buffer_init(pdb);

	if (offset > 1) {
		/* index in global cache: on1 is 1, off1 is 2, on2 is 3 ... */
		once_max = (offset - 1) + 3 - 1;
	} else {
		once_max = num - 1;
	}

	for (i = data_offset_bias; i <= once_max ; i++) {
		if ((i & 0x1) == 1)
			ir_pulse_from_pronto(buf[i], tx1000000, pdb);
		else
			ir_space_from_pronto(buf[i], tx1000000, pdb);
	}

	if (repeat > 1) {
		u32 start, end, todo;

		todo = repeat - 1;
		start = offset + data_offset_bias;
		end = num - 1;

		while (todo) {
			for (i = start; i <= end; i++) {
				if ((i & 0x1) == 1)
					ir_pulse_from_pronto(buf[i], tx1000000, pdb);
				else
					ir_space_from_pronto(buf[i], tx1000000, pdb);
			}
			todo--;
		}
	}

	header = (struct _ir_net_header *) pdb->buf;

	header->magic = IR_MAGIC_NUM;
	header->length = (pdb->index - 16);

	return 0;
}

static int pronto_to_dma(int *pronto, unsigned int num, struct ir_dma_buffer *pdb)
{
	struct _ir_net_header *header;
	u32 tx1000000, i, once_max;

	/*
	 * 1 - 0000 indicates raw IR data (you can ignore this value)
	 * 2 - P, frequency f = 1000000 / (P * 0.241246)
	 *	P * 0.241246 =  1000000 / f
	 *	P = (1000000 / f) / 0.241246 = (1000000 / f) * 1000000 / 241246
	 *		= (1000000 / f) * 500000 / 120623
	 *	we need T for pronto-to-DMA
	 *	T = P * 120623 / 500000 (in micro second)
	 * 3 - length of the first burst pair sequence
	 * 4 - length of the second burst pair sequence
	 */

	if (pronto[0] != 0x0000)
		return -1;

	if (num < (4 + 2)) /* header: 4, at least one pair: 2 */
		return -1;

	once_max = 4 + (pronto[2] * 2);

	if (num < once_max)
		return -1;

	tx1000000 = pronto[1] * 241246;

	ir_dma_buffer_init(pdb);

	/* pronto to DMA: number of period * T */
	for (i = 4; i < once_max; i++) {
		if ((i & 0x1) == 0)
			ir_pulse_from_pronto(pronto[i], tx1000000, pdb);
		else
			ir_space_from_pronto(pronto[i], tx1000000, pdb);
	}

	header = (struct _ir_net_header *) pdb->buf;

	header->magic = IR_MAGIC_NUM;
	header->length = (pdb->index - 16);

	return 0;
}

static void pronto_string_to_dma_and_net_out(u8 *data)
{
	int tx_fd, cnt;

	cnt = user_string_to_hex(data, pronto_hex);

	if (pronto_to_dma(pronto_hex, cnt, &dma_buffer) != 0)
		return;

	tx_fd = create_socket_to_local(IR_PORT);
	sendto(tx_fd, dma_buffer.buf, dma_buffer.index, 0, NULL, 0);
	close(tx_fd);
}

static void string_to_dma_and_net_out(u8 *data, u32 global_cache)
{
	int tx_fd, cnt;

	if (global_cache) {
		cnt = gc_string_to_int(data, pronto_hex);
		if (global_cache_to_dma(pronto_hex, cnt, &dma_buffer) != 0)
			return;

	} else {
		cnt = user_string_to_hex(data, pronto_hex);
		if (pronto_to_dma(pronto_hex, cnt, &dma_buffer) != 0)
			return;
	}

	tx_fd = create_socket_to_local(IR_PORT);
	sendto(tx_fd, dma_buffer.buf, dma_buffer.index, 0, NULL, 0);
	close(tx_fd);
}

static int is_global_cache_format(u8 *buf)
{
	int ret = 0;
	/*
	 * global cache example: sendir,10:3,3456,23400,1,1,24,48,24,960
	 * each field got separate by ','
	 */
	if (strchr(buf, ',') != NULL)
		ret = 1;

	return ret;
}

int main(void)
{
	int socket_fd;      /* file description into transport */
	int recfd;     /* file descriptor to accept        */
	int length;     /* length of address structure      */
	int nbytes;     /* the number of read **/
	char buf[BUFSIZ];
	char data[IR_SIZE_TEXT_MAX];
	int data_index = 0;
	struct sockaddr_in myaddr; /* address of this service */
	struct sockaddr_in client_addr; /* address of client    */
	int reuseaddr = 1, ret = 0, i;
	int global_cache = -1;

	/* Get a socket into TCP/IP */
	socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (socket_fd < 0) {
		perror("socket failed");
		exit(1);
	}

	/* Set up our address */
	bzero((char *)&myaddr, sizeof(myaddr));
	myaddr.sin_family = AF_INET;
	myaddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	myaddr.sin_port = htons(IR_S_D_PORT);

	ret = setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &reuseaddr, sizeof(reuseaddr));

	if (ret < 0)
		perror("setsockopt (SO_REUSEADDR)\n");

	/* Bind to the address to which the service will be offered */
	if (bind(socket_fd, (struct sockaddr *)&myaddr, sizeof(myaddr)) < 0) {
		perror("bind failed");
		exit(1);
	}

	/* Set up the socket for listening, with a queue length of 5 */
	if (listen(socket_fd, 20) < 0) {
		perror("listen failed");
		exit(1);
	}

	/* Loop continuously, waiting for connection requests and performing the service */
	length = sizeof(client_addr);

	while (1) {
		recfd = accept(socket_fd, (struct sockaddr *)&client_addr, &length);
		if (recfd < 0) {
			perror("could not accept call");
			exit(1);
		}

		memset(&data, 0, IR_SIZE_TEXT_MAX);
		data_index = 0;

		while (1) {
			nbytes = read(recfd, &buf, BUFSIZ);

			if  (nbytes <= 0) {
				if (data_index > 0) {
					data[data_index] = '\0';
					string_to_dma_and_net_out(data, global_cache);
					data_index = 0; /* already flush */
					global_cache = -1; /* reset type flag */
				}
				break;
			}

			if (global_cache != is_global_cache_format(buf)) {
				if (global_cache != -1) {
					data[data_index] = '\0';
					string_to_dma_and_net_out(data, global_cache);
					data_index = 0; /* already flush */
				}

				global_cache = is_global_cache_format(buf); /* update type flag */
			}


			for (i = 0; i < nbytes; i++) {
				if ((buf[i] == '\0') || (buf[i] == telnet_end_char)) {
					data[data_index] = '\0';
					string_to_dma_and_net_out(data, global_cache);
					data_index = 0; /* already flush */
					global_cache = -1; /* reset type flag */
				} else {
					data[data_index] = buf[i];
					data_index++;
				}
			}
		}

		close(recfd);
	}
}

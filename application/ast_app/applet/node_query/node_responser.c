/* Copyright (c) 2017
 * ASPEED Technology Inc. All Rights Reserved
 * Proprietary and Confidential
 *
 * By using this code you acknowledge that you have signed and accepted
 * the terms of the ASPEED SDK license agreement.
 */
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <sys/socket.h>
#include <string.h>
#include <sys/file.h> /* for flock() */
#include <sys/epoll.h>
#include <errno.h>
#include <unistd.h> /* for usleep() */
#include "node.h"
#include "debug.h"
#include "astparam.h"
#include "astnetwork.h"
#include "ast_send_event.h"

#define LISTEN_NODES_MAX  16  /* support up to 16 mac */
#define MAX_CMD_PAYLOAD 128

typedef struct {
	int session; /* from notifyer. Used to avoid duplication. */
	char mac[13]; /* in string format */
} node_info_s;

struct nr_cfg_s {
	FILE *lock_file;
	uint16_t period_ms;
	char my_mac[6]; /* Saved in big endian order */
	char my_mac_str[13]; /* MAC string 12+1 bytes */
	node_info_s listen_nodes[LISTEN_NODES_MAX]; /* Used to be matched in on_notify_chg_received. */
	int ctrl_fd;
	int listen_fd;
	int reply_fd; /* only valid for a while. */
	char param_buf[ASTPARAM_BLOCK_SIZE]; /* node query rx buffer */
	/* node response packet buffers */
	struct nqpkt_hdr_s nq_pkt_hdr;
	char response_param[ASTPARAM_BLOCK_SIZE];
};

#include "node_responser_ctrl.c"

typedef int (*event_handler_func)(struct nr_cfg_s *);

/* Global variable */
struct nr_cfg_s cfg;

#if 0
void dump_buf(void *buf_ptr, unsigned int len)
{
	unsigned int i;
	char *buf = (char *)buf_ptr;

	for (i = 0; i < len; i++) {
		if (!(i & 0xF))
			1("\n");
		info("%02X ", buf[i]);
	}
	info("\n");
}
#else
inline void dump_buf(void *buf_ptr, unsigned int len) {}
#endif

inline int is_in_mac_list(
		struct nr_cfg_s *c,
		char *mac_list,
		unsigned int list_size)
{
	char *mac_list_max = mac_list + list_size;
	char *imac;
	char *my_mac = c->my_mac;

	for (imac = mac_list; imac < mac_list_max; imac += 6) {
		dbg("mac: %02X:%02X:%02X:%02X:%02X:%02X vs %02X:%02X:%02X:%02X:%02X:%02X\n"
			     , imac[0]
			     , imac[1]
			     , imac[2]
			     , imac[3]
			     , imac[4]
			     , imac[5]
			     , my_mac[0]
			     , my_mac[1]
			     , my_mac[2]
			     , my_mac[3]
			     , my_mac[4]
			     , my_mac[5]);

		if (!memcmp(my_mac, imac, 6)) {
			/* matched */
			dbg("in ACK list. Ignore.\n");
			return 1;
		}
	}
	return 0;
}

int lock_node_query_file(struct nr_cfg_s *c)
{
	int ret = 0;

	c->lock_file = fopen("/etc/node_query_file.lck", "r");
	if (c->lock_file == NULL) {
		err("failed to open node_query_file.lck\n");
		ret = -1;
		goto done;
	}
	if (flock(fileno(c->lock_file), LOCK_EX)) {
		err("node_query_file.lck lock failed?!\n");
		ret = -1;
		goto done;
	}
done:
	return ret;
}

int unlock_node_query_file(struct nr_cfg_s *c)
{
	if (c->lock_file) {
		flock(fileno(c->lock_file), LOCK_UN);
		fclose(c->lock_file);
		c->lock_file = NULL;
	}
	return 0;
}

void read_param_file(struct nr_cfg_s *c, struct nqpkt_hdr_s *nq_pkt_hdr, char *param_buf, char *file_name)
{
	FILE *rf = NULL;
	unsigned int nbytes;

	if (lock_node_query_file(c) < 0)
		goto clean;

	rf = fopen(file_name, "r");
	if (!rf) {
		err("Failed to open reply param file %s (%d:%s)\n",
		                                     file_name,
		                                     errno,
		                                     strerror(errno));
		goto clean;
	}

	nbytes = fread(param_buf, 1, ASTPARAM_BLOCK_SIZE, rf);
	if (nbytes < 0) {
		err("Read reply param file fail! %s (%d:%s)\n", file_name, errno, strerror(errno));
		goto clean;
	}
	if (nbytes == ASTPARAM_BLOCK_SIZE) {
		err("reply param file too big! %s\n", file_name);
		goto clean;
	}

	nq_pkt_hdr->block1_size = (uint16_t)nbytes;
	goto done;

clean:
	nq_pkt_hdr->block1_size = MIN_ASTPARAM_DATA_SIZE;
	memset(param_buf, 0, MIN_ASTPARAM_DATA_SIZE);
done:
	if (rf)
		fclose(rf);

	unlock_node_query_file(c);

	return;
}

void read_reply_file(struct nr_cfg_s *c, char *file_name)
{
	int size;
	struct nqpkt_hdr_s *nq_pkt = &c->nq_pkt_hdr;

	memset(nq_pkt, 0, sizeof(NQPKT_HDR_SIZE));
	nq_pkt->magic = NQ_PKT_MAGIC;

	/* TODO. If we can append reply file after essential parts, we can improve match time. */
	read_param_file(c, nq_pkt, c->response_param, file_name);

	/* reply_type is essential type in query packet. */
	size = setParam(c->response_param, ASTPARAM_BLOCK_SIZE, "REPLY_TYPE", file_name);
	if (size > 0)
		nq_pkt->block1_size = size;
	/* Always add/overwrite MY_MAC as "must-have" param */
	/* NOTE: This means MY_MAC CAN'T be runtime overwritten by reply_file. */
	size = setParam(c->response_param, ASTPARAM_BLOCK_SIZE, "MY_MAC", c->my_mac_str);
	if (size > 0)
		nq_pkt->block1_size = size;
}

int do_match(struct nr_cfg_s *c, char *match_buf, char *resp_buf)
{
	char *key_value_pair;

	match_buf = dataFieldParam(match_buf);

	while (match_buf) {
		match_buf = popParamKeyValue(match_buf, &key_value_pair);
		dbg("MATCH:%s\n", key_value_pair);
		/* no more keys to match */
		if (!key_value_pair)
			break;

		if (!hasParamKeyValue(resp_buf, key_value_pair))
			goto not_match;
	}
	/* all matched. */
	dbg("all matched\n");
	return 0;
not_match:
	dbg("Not match\n");
	return 1;
}

int send_node_response(
		struct nr_cfg_s *c,
		struct nqpkt_hdr_s *nq_pkt,
		char *response_param)
{
#define IOV_NUM 2
	int nbytes;
	struct msghdr msg_hdr;
	struct iovec iov[IOV_NUM];

	//TODO. How to recover over UDP limit (64KB) case?
	/* Send query response packet. */
	memset(&iov, 0, sizeof(struct iovec) * IOV_NUM);
	memset(&msg_hdr, 0, sizeof(struct msghdr));
	iov[0].iov_base = (void *)nq_pkt;
	iov[0].iov_len = NQPKT_HDR_SIZE;
	iov[1].iov_base = (void *)response_param;
	iov[1].iov_len = nq_pkt->block1_size;
	msg_hdr.msg_iov = iov;
	msg_hdr.msg_iovlen = IOV_NUM;

	nbytes = sendmsg(c->reply_fd, &msg_hdr, 0);
	if (nbytes < 0) {
		err("node response send failed?! (%d:%s)", errno, strerror(errno));
		if (errno == EMSGSIZE) {
			err("size over 64KB! (%d)\n", NQPKT_TOTAL_SIZE(nq_pkt));
		}
	}
	return nbytes;
#undef IOV_NUM
}

void back_off(unsigned int period_ms)
{
	unsigned int t;

	/* return immediately if back off is not needed. */
	if (!period_ms)
		return;

	t = random() % period_ms;
	dbg("backoff for %d\n", t);
	usleep(t * 1000);
}

void on_notify_chg_received(struct nr_cfg_s *c, char *param_buf, unsigned int data_len)
{
	char *mac, *session, *chg_info;
	node_info_s *node;
	int i, s;
	char event_str[MAX_CMD_PAYLOAD]; /* e_nq_notify_chg::MAC::essential */

	/* node_query will use --match_key to put FROM_MAC in param_buf. */
	mac = getParam(param_buf, "FROM_MAC");
	if (!mac) {
		err("NQ_NOTIFY_CHG: no FROM_MAC?!\n");
		return;
	}

	node = c->listen_nodes;
	for (i = 0; i < LISTEN_NODES_MAX; i++, node++) {
		if (strncmp(mac, node->mac, 12))
			continue;

		/* Matched. I'm interested in this node. */
		session = getParam(param_buf, "SESSION");
		if (!session)
			s = 0;
		else
			s = atoi(session);

		/* Check session. Ignore if duplicate. */
		if (s == node->session)
			return;

		/* Now, we matched without duplication. */
		chg_info = getParam(param_buf, "CHG_INFO");
		node->session = s;
		sprintf(event_str, "e_nq_notify_chg::%s::%s",
		        mac,
		        chg_info ? chg_info : NQPKT_DEFAULT_REPLY_TYPE);
		ast_send_event(-1, event_str);
		return;
	}
	/* If not in listen list. Simply ignore it. */
}

void on_node_query_received(struct nr_cfg_s *c, char *param_buf, unsigned int data_len)
{
	char *reply_type;

	/* open and read response param. */
	reply_type = getParam(param_buf, "REPLY_TYPE");
	if (!reply_type) {
		err("query without reply_type?!\n");
		read_reply_file(c, NQPKT_DEFAULT_REPLY_TYPE);
	} else if (!strncmp(reply_type, "NQ_NOTIFY_CHG", strlen("NQ_NOTIFY_CHG"))) {
		/* NQ_NOTIFY_CHG */
		/* This is special node_query command. handle it and return without reply. */
		on_notify_chg_received(c, param_buf, data_len);
		return;
	} else if (!strncmp(reply_type, "emu", strlen("emu"))) {
		/* emulator. for testing */
		read_reply_file(c, c->my_mac_str);
	} else {
		/* regular reply_type. Use reply_type as reply file name. */
		read_reply_file(c, reply_type);
	}

	/* Remove no interests key/values, like REPLY_TYPE */
	setParam(param_buf, data_len, "REPLY_TYPE", NULL);

	/* match keys */
	if (do_match(c, param_buf, c->response_param)) {
		/* doesn't match */
		goto done;
	}
	/* blocking call. back off for a period of time before reply. */
	back_off(c->period_ms);

	send_node_response(c, &c->nq_pkt_hdr, c->response_param);
done:
	return;
}

int on_node_query_coming(struct nr_cfg_s *c)
{
	struct nqpkt_hdr_s *nq_pkt;
	int ret = -1;
	int nbytes;
	char *param_buf, *mac_list;
	struct sockaddr_in src_addr;
	socklen_t src_addr_len = sizeof(struct sockaddr_in);

	/* receive node */
	nbytes = recvfrom(c->listen_fd, c->param_buf, ASTPARAM_BLOCK_SIZE, 0, (struct sockaddr *)&src_addr, &src_addr_len);
	if (nbytes < 0) {
		err("node query receiving failed?! (%d:%s)\n", errno, strerror(errno));
		goto done; //ignore?
	}
	dump_buf(c->param_buf, nbytes);
	nq_pkt = (struct nqpkt_hdr_s *)c->param_buf;
	if (nq_pkt->magic != NQ_PKT_MAGIC) {
		goto done;
	}
	if (nbytes != NQPKT_TOTAL_SIZE(nq_pkt)) {
		err("wrong node response packet size?! (%d vs %d)\n", nbytes, NQPKT_TOTAL_SIZE(nq_pkt));
		/* ignore */
		goto done;
	}

	/* create an unicast UDP socket for sending back response. */
	c->reply_fd = udp_create_sender(src_addr.sin_addr.s_addr, NQ_PORT, NULL, INADDR_ANY);
	if (c->reply_fd < 0) //Ignore socket create error?
		goto done;

	/* We accept period_ms as 0 ms. It is for node_list reply immediately case. */
	c->period_ms = nq_pkt->query_period;

	param_buf = nq_pkt->payload;
	mac_list = nq_pkt->payload + nq_pkt->block1_size;

	if (is_in_mac_list(c, mac_list, nq_pkt->block2_size)) {
		/* means queryer is not interests in my response. I'm done. */
		goto done;
	}
	/* handle received node info. */
	on_node_query_received(c,
	                       param_buf,
	                       nq_pkt->block1_size);
done:
	ret = 0;
err:
	if (c->reply_fd) {
		close(c->reply_fd);
		c->reply_fd = 0;
	}
	return ret;
}

int on_ctrl_recv(struct nr_cfg_s *c)
{
	int listen_fd = (int)c->ctrl_fd;
	int cmd_fd = -1;
	int nbytes;
	char cmd_buffer[MAX_CMD_PAYLOAD];

	cmd_fd = accept(listen_fd, NULL, NULL);
	if (cmd_fd < 0) {
		err("accept() failed [%d:%s]\n", errno, strerror(errno));
		goto err;
	}
	/* receive cmd */
	nbytes = read(cmd_fd, cmd_buffer, MAX_CMD_PAYLOAD);
	if (nbytes < 0) {
		err("read() failed [%d:%s]\n", errno, strerror(errno));
		goto err;
	}
	cmd_buffer[nbytes] = 0;

	/* handle cmd */
	handle_ctrl_cmd(cmd_buffer, c);

	/* close socket and out. */
	close(cmd_fd);

	return 0;
err:
	if (!(cmd_fd < 0))
		close(cmd_fd);

	return -1;
}

int start_node_responser(struct nr_cfg_s *c)
{
	struct epoll_event ep_event;
	int epfd, ne;
	int ret = -1;
	event_handler_func handler;

	c->ctrl_fd = create_uds_server("@nq_ctrl");
	if (c->ctrl_fd < 0)
		goto err;

	/* create rx socket */
	c->listen_fd = udp_connect(inet_addr(NQ_MC_IP), NR_PORT);
	if (c->listen_fd < 0)
		goto err;

	/* About size '5', Since Linux 2.6.8, the size argument is ignored, but must be greater than zero */
	epfd = epoll_create(5);
	if (epfd < 0) {
		perror("epoll_create failed");
		goto err;
	}

	/* Add ctrl_fd to epoll to wait for Rx event. */
	ep_event.events = EPOLLIN;
	ep_event.data.ptr = (void *)on_ctrl_recv;
	if (epoll_ctl(epfd, EPOLL_CTL_ADD, c->ctrl_fd, &ep_event) < 0) {
		perror("epoll_ctl failed");
		goto err;
	}

	/* Add node_query to epoll to wait for Rx event. */
	ep_event.events = EPOLLIN;
	ep_event.data.ptr = (void *)on_node_query_coming;
	if (epoll_ctl(epfd, EPOLL_CTL_ADD, c->listen_fd, &ep_event) < 0) {
		perror("epoll_ctl failed");
		goto err;
	}

	memset(&ep_event, 0, sizeof(ep_event));
	for (;;) {
		ne = epoll_wait(epfd, &ep_event, 1, -1);
		if (ne < 0) {
			/* something wrong. */
			perror("epoll_wait interrupted?!");
			break;
		}
		/* case for epoll/node_query timeout. */
		if (ne == 0) {
			/* Should not happen!? */
			continue;
		}
		/* Handle fd events. */
		handler = (event_handler_func)ep_event.data.ptr;
		if (handler(c))
			break;
	}
	ret = 0;

	epoll_ctl(epfd, EPOLL_CTL_DEL, c->ctrl_fd, NULL);
	epoll_ctl(epfd, EPOLL_CTL_DEL, c->listen_fd, NULL);
err:
	if (c->ctrl_fd > 0)
		close(c->ctrl_fd);
	if (c->listen_fd > 0)
		close(c->listen_fd);
	if (epfd > 0)
		close(epfd);

	return ret;
}

int main(int argc, char *argv[])
{
	int opt = 0;
	int long_index =0;
	struct nr_cfg_s *c = &cfg;
	unsigned long long my_mac;

	/* Specifying the expected options */
	static struct option long_options[] = {
		{"mac",              required_argument,       0,  'a' },
		{0,                  0,                       0,  0   }
	};

	if (argc < 2)
		goto err;

	/* init */
	memset(c, 0, sizeof(struct nr_cfg_s));

	while ((opt = getopt_long_only(argc, argv, "", long_options, &long_index )) != -1) {
		switch (opt) {
		case 'a': /* --mac */
			strncpy(c->my_mac_str, optarg, 12);
			/* NOTE: MUST inlucde #include <stdlib.h> for strtoull() to work as expected. */
			my_mac = strtoull(optarg, NULL, 16);
			c->my_mac[5] = (char)(my_mac >> 0);
			c->my_mac[4] = (char)(my_mac >> 8);
			c->my_mac[3] = (char)(my_mac >> 16);
			c->my_mac[2] = (char)(my_mac >> 24);
			c->my_mac[1] = (char)(my_mac >> 32);
			c->my_mac[0] = (char)(my_mac >> 40);
			info("MAC: %02X:%02X:%02X:%02X:%02X:%02X\n"
			     , c->my_mac[0]
			     , c->my_mac[1]
			     , c->my_mac[2]
			     , c->my_mac[3]
			     , c->my_mac[4]
			     , c->my_mac[5]);
			break;
		default:
			goto err;
		}
	}

	chdir(NQ_DEFAULT_PATH);

	srandom((unsigned int)my_mac);

	return start_node_responser(c);
err:
	//print_usage();
	err("I need --mac\n");
	return -1;
}

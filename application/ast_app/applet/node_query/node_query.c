/* Copyright (c) 2017
 * ASPEED Technology Inc. All Rights Reserved
 * Proprietary and Confidential
 *
 * By using this code you acknowledge that you have signed and accepted
 * the terms of the ASPEED SDK license agreement.
 */
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h> /* strtok_r() */
#include <sys/file.h> /* for flock() */
#include <sys/epoll.h>
#include <signal.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "node.h"
#include "astnetwork.h"
#include "astparam.h"
#include "debug.h"

#define MAX_FILE_PATH 256
#define KEY_POOL_SIZE 8
#define MAX_NODE_SIZE 4000

/* Action types bitmap. Can OR. */
#define	ACTION_MATCH_KEY         (0x1UL << 0)
#define	ACTION_SET_KEY           (0x1UL << 1)
#define	ACTION_GET_KEY           (0x1UL << 2)
#define	ACTION_DUMP              (0x1UL << 3)
#define	ACTION_WRITE_FILE        (0x1UL << 4)
#define	ACTION_NODE_FROM_FILE    (0x1UL << 5)
#define	ACTION_JSON_FORMAT       (0x1UL << 6) /* print in json format */

struct key_pair_s {
	char *key;
	char *value;
};

struct cfg_s {
	int query_rx_fd;
	int query_tx_fd;
	FILE *input_file;
	FILE *lock_file;      /* Used to protect node_query re-entry */
	FILE *file_lock_file; /* Used to protect node_info files. */
	char *input_file_name;
	char *output_file_name;
	char *reply_type; /* Maps to reply astparam file name. */
	struct key_pair_s match_key_pool[KEY_POOL_SIZE];
	struct key_pair_s set_key_pool[KEY_POOL_SIZE];
	struct key_pair_s get_key_pool[KEY_POOL_SIZE];
	unsigned int action;
	unsigned int max_prints; /* limit the number of node printed. 0 means no limitation. */
	unsigned int period_ms; /* query period in ms */
	unsigned int period_ms_overwrite; /* query period in ms */
	unsigned int received_node_cnt;
	unsigned int matched_node_cnt;
	unsigned int param_buf_data_len;
	unsigned int local_intf; /* inet_addr(ip) */
	char param_buf[ASTPARAM_BLOCK_SIZE];
	/* node query packet buffers. */
	struct nqpkt_hdr_s nq_pkt_hdr;
	char nq_param_buf[ASTPARAM_BLOCK_SIZE];
	char mac_list[MAX_NODE_SIZE * 6];
};

typedef int (*event_handler_func)(struct cfg_s *);

struct cfg_s cfg;

void print_usage() {
	/* TODO */
	printf("Usage: node_query\n");
}

#if 0
void dump_buf(void *buf_ptr, unsigned int len)
{
	unsigned int i;
	char *buf = (char *)buf_ptr;

	for (i = 0; i < len; i++) {
		if (!(i & 0xF))
			info("\n");
		info("%02X ", buf[i]);
	}
	info("\n");
}
#else
inline void dump_buf(void *buf_ptr, unsigned int len) {}
#endif
void parse_key_value(struct key_pair_s *key_pair, char *optarg)
{
	char *save_ptr;
	/*
	** expect optarg in following string format:
	** - key=value
	** - key            <= clean key
	** - key=           <= clear key
	*/
	key_pair->key = strtok_r(optarg, "=", &save_ptr);
	if (!key_pair->key) {
		err("$d\n", __LINE__);
		goto err;
	}
	/* value can be NULL. */
	key_pair->value = strtok_r(NULL, "=", &save_ptr);

	dbg("key=%s, value=%s\n", key_pair->key, key_pair->value);
err:
	return;
}

int add_key_pair_to_pool(struct key_pair_s *key_pair_pool, unsigned int pool_size, char *optarg)
{
	int i;
	struct key_pair_s *key_pair = key_pair_pool;
	/* Find a empty key_pair slot. */
	for (i = 0; i < pool_size; i++, key_pair++) {
		if (!key_pair->key)
			break;
	}
	/* return -1 if pool is full */
	if (i == pool_size) {
		err("key_pool is full!! (%d)\n", pool_size);
		return -1;
	}
	/* add optarg into this empty key_pair */
	parse_key_value(key_pair, optarg);
	return 0;
}

int open_input_file(struct cfg_s *c)
{
	if (!(c->action & ACTION_NODE_FROM_FILE))
		goto done;

	if (!c->input_file_name)
		goto done;

	c->input_file = fopen(c->input_file_name, "r");

	if (!c->input_file) {
		dbg("Ignore non-existing input file.");
		/* Ignore empty file. query_node_from_file() will take care of it. */
		//return -1;
	}
done:
	return 0;
}

int open_sockets(struct cfg_s *c)
{
	int ret = -1;

	if (c->action & ACTION_NODE_FROM_FILE) {
		ret = 0;
		goto done;
	}

	/* create queryer listen/receive UDP socket. */
	c->query_rx_fd = udp_connect_as_server(htonl(INADDR_ANY), NQ_PORT);
	if (c->query_rx_fd < 0) {
		err("Open query_rx_fd failed\n");
		goto err;
	}

	/* create queryer send multicast socket. */
	c->query_tx_fd = udp_create_sender(inet_addr(NQ_MC_IP), NR_PORT, NULL, c->local_intf);
	if (c->query_tx_fd , 0) {
		err("Open query_tx_fd failed\n");
		goto err;
	}

	ret = 0;
	goto done;

err:
	if (c->query_rx_fd > 0)
		close(c->query_rx_fd);

	if (c->query_tx_fd > 0)
		close(c->query_tx_fd);
done:
	return ret;
}

int lock_node_query(struct cfg_s *c)
{
	int ret = 0;

	c->lock_file = fopen("/etc/node_query.lck", "r");
	if (c->lock_file == NULL) {
		err("failed to open node_query.lck\n");
		ret = -1;
		goto done;
	}
	if (flock(fileno(c->lock_file), LOCK_EX)) {
		err("node_query.lck lock failed?!\n");
		ret = -1;
		goto done;
	}
done:
	return ret;
}

int unlock_node_query(struct cfg_s *c)
{
	if (c->lock_file) {
		flock(fileno(c->lock_file), LOCK_UN);
		fclose(c->lock_file);
		c->lock_file = NULL;
	}
	return 0;
}

int lock_node_query_file(struct cfg_s *c)
{
	int ret = 0;

	c->file_lock_file = fopen("/etc/node_query_file.lck", "r");
	if (c->file_lock_file == NULL) {
		err("failed to open node_query_file.lck\n");
		ret = -1;
		goto done;
	}
	if (flock(fileno(c->file_lock_file), LOCK_EX)) {
		err("node_query_file.lck lock failed?!\n");
		ret = -1;
		goto done;
	}
done:
	return ret;
}

int unlock_node_query_file(struct cfg_s *c)
{
	if (c->file_lock_file) {
		flock(fileno(c->file_lock_file), LOCK_UN);
		fclose(c->file_lock_file);
		c->file_lock_file = NULL;
	}
	return 0;
}

int set_keys(
		struct cfg_s *c,
		struct key_pair_s *set_key_pool,
		const unsigned int key_pool_size,
		char *buf,
		unsigned int data_len,
		unsigned int max_buf_size)
{
	int i;
	struct key_pair_s *key_pair = set_key_pool;
	unsigned int buf_size;
	unsigned int len_to_write = data_len;

	/* buf_size used to tell setParam() the max buf size it can use. */
	buf_size = max_buf_size;

	for (i = 0; i < key_pool_size; i++, key_pair++) {
		if (!key_pair->key)
			continue;
		len_to_write = setParam(buf, buf_size, key_pair->key, key_pair->value);
		if (len_to_write < 0)
			break;
	}
	/* if < 0, means something wrong. Otherwise return the new len of buf. */
	return len_to_write;
}

int match_keys(struct cfg_s *c, struct key_pair_s *match_key_pool, const unsigned int key_pool_size, char *buf)
{
	struct key_pair_s *key_pair = match_key_pool;
	char *matched_value;
	int i, ret = 0;

	for (i = 0; i < key_pool_size; i++, key_pair++) {
		if (!key_pair->key)
			continue;
		/* User typo may causes matching a key with NULL value. We ignore this key pair match. */
		if (!key_pair->value)
			continue;

		matched_value = getParam(buf, key_pair->key);
		/* return, if not exists. */
		if (!matched_value) {
			ret = -1;
			break;
		}
		/* check if matched. */
		if (strcmp(key_pair->value, matched_value)) {
			ret = -1;
			break;
		}
	}
done:
	return ret;
}

#define jsprint(fmt, args...) \
	do { \
		fprintf(stdout, fmt, ##args); \
	} while(0)

/*
	The following characters are reserved in JSON and must be properly
	escaped to be used in strings:

	Backspace is replaced with \b
	Form feed is replaced with \f
	Newline is replaced with \n
	Carriage return is replaced with \r
	Tab is replaced with \t
	Double quote is replaced with \"
	Backslash is replaced with \\
	Slash is replaced with \/
*/
static inline void js_escape_and_print(char c)
{
	switch (c) {
		case '\b':
			jsprint("\\b");
			break;
		case '\f':
			jsprint("\\f");
			break;
		case '\n':
			jsprint("\\n");
			break;
		case '\r':
			jsprint("\\r");
			break;
		case '\t':
			jsprint("\\t");
			break;
		case '"':
			jsprint("\\\"");
			break;
		case '\\':
			jsprint("\\\\");
			break;
		case '/':
			jsprint("\\/");
			break;
		default:
			jsprint("%c", c);
			break;
	}
}

static inline void js_escape_and_print_str(char *str)
{
	unsigned int i = 0;

	if (!str)
		return;

	while(str[i]) {
		js_escape_and_print(str[i]);
		i++;
	}
}

static inline void js_print_obj_header(void)
{
		/*
		**{
		**"nodes":[
		*/
#if 0
		jsprint("{");
		jsprint("\n");
		jsprint("\"nodes\":[");
#else
		jsprint("{\n\"nodes\":[");
#endif
}

static inline void js_print_obj_footer(unsigned int count)
{
		/*
		**\n
		**],\n
		**"count":3\n
		**}\n
		*/
#if 0
		jsprint("\n");
		jsprint("],");
		jsprint("\n");
		jsprint("\"count\":%d", count);
		jsprint("\n");
		jsprint("}");
		jsprint("\n");
#else
		if (count)
			jsprint("\n");
		jsprint("],\n\"count\":%d\n}\n", count);
#endif
}

static inline void js_print_delimiter(void)
{
	jsprint(",");
}

static inline void js_print_obj_start(void)
{
	jsprint("\n{");
}

static inline void js_print_obj_end(void)
{
	jsprint("\n}");
}

static inline void js_print_attr_key_start_quote(void)
{
	jsprint("\n\t\"");
}

static inline void js_print_attr_delimiter(void)
{
	/* Print Ken End Quate, JSON Delimiter, Value Start Quate. */
	jsprint("\":\"");
}

static inline void js_print_attr_value_end_quote(void)
{
	/* Print Value End Quate. */
	jsprint("\"");
}

static inline void js_print_attr(char *key, char*value)
{
	if (!key)
		return;

	js_print_attr_key_start_quote();
	js_escape_and_print_str(key);
	js_print_attr_delimiter();
	js_escape_and_print_str(value);
	js_print_attr_value_end_quote();
}

static void print_header(unsigned int action)
{
	if (!(action & (ACTION_DUMP | ACTION_GET_KEY)))
		return;

	if (action & ACTION_JSON_FORMAT) {
		js_print_obj_header();
	} else if (action & ACTION_DUMP) {
		msg(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
	}
}

static void print_footer(unsigned int action, unsigned int count)
{
	if (!(action & (ACTION_DUMP | ACTION_GET_KEY)))
		return;

	if (action & ACTION_JSON_FORMAT) {
		js_print_obj_footer(count);
	} else if (action & ACTION_DUMP) {
		msg("<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\n");
	}
}

/* Sample JSON output format
{
"nodes":[
    {
        "MY_IP":"169.254.10.133",
        "IS_HOST":"y",
        "MULTICAST_ON":"n",
        "HOSTNAME":"ast3-Tx-9999",
        "CH_SELECT":"9999",
        "STATE":"s_srv_on",
        "REPLY_TYPE":"essential",
        "MY_MAC":"02C7C324D7E3"
    },
    {
        "MY_IP:169.254.10.167,
        "IS_HOST":"n",
        "HOSTNAME":"ast3-Rx-82EAA841BDF1",
        "REPLY_TYPE":"essential",
        "MY_MAC":"82EAA841BDF1"
    },
    {
        "MY_MAC":"82EAA841BDF2"
    }
],
"count":3
}
*/
#define JS_WAIT_KEY     0
#define JS_WAIT_DLMTR   1
#define JS_WAIT_VALUE   2
#define JS_WAIT_END     3

static void dump_param_json(struct cfg_s *c, char *buffer, unsigned int max_buf_size)
{
	int i;
	char *param = buffer + 4;
	unsigned int state = JS_WAIT_KEY;
	unsigned int entry_cnt = 0;

	/* print delimiter */
	if (c->matched_node_cnt)
		js_print_delimiter();

	js_print_obj_start();

	for (i = 0; (param[i] != 0 || param[ i+ 1]) != 0; i++) {
		switch(state) {
		case JS_WAIT_KEY:
			if ((param[i] == 0) || (param[i] == '=')) {
				/* WTF. Ignore the whole string. */
				for (; (param[i] != 0 || param[ i+ 1]) != 0; i++) {
					if (param[i] == 0)
						break;
					/* Check max buf. */
					if (i > (max_buf_size - 4))
						break;
				}
			} else {
				/* Entry start. */
				state = JS_WAIT_DLMTR;
				if (entry_cnt)
					js_print_delimiter();
				/* Print "Key Start Quote" */
				js_print_attr_key_start_quote();
				js_escape_and_print(param[i]);
			}
			break;
		case JS_WAIT_DLMTR:
			if (param[i] == 0) {
				/* WTF2. Print a dummp entry to close this un-normal case. */
				/* Print: Key End, Delimiter, Empty Value, */
				js_print_attr_delimiter();
				js_print_attr_value_end_quote();
				entry_cnt++;
				state = JS_WAIT_KEY;
			} else if (param[i] == '=') {
				/* Key ended and found delimiter. */
				entry_cnt++;
				/* Print Ken End Quate, JSON Delimiter, Value Start Quate. */
				js_print_attr_delimiter();
				state = JS_WAIT_VALUE;
			} else {
				/* Regular Key character. */
				js_escape_and_print(param[i]);
			}
			break;
		case JS_WAIT_VALUE:
			if (param[i] == 0) {
				/* Value ended. It is an empty value. */
				/* Print Value End Quate. */
				js_print_attr_value_end_quote();
				state = JS_WAIT_KEY;
			} else {
				/* Got first Value char, print, then to WAIT_END. */
				js_escape_and_print(param[i]);
				state = JS_WAIT_END;
			}
			break;
		case JS_WAIT_END:
			if (param[i] == 0) {
				/* Value ended. */
				/* Print Value End Quate. */
				js_print_attr_value_end_quote();
				state = JS_WAIT_KEY;
			} else {
				/* Regular Value character. */
				js_escape_and_print(param[i]);
			}
			break;
		}

		/* Check max buf. */
		if (i > (max_buf_size - 4))
			break;
	}
	/* Handle unexpected result. */
	switch(state) {
		case JS_WAIT_KEY:
			/* Normal case. */
			break;
		case JS_WAIT_DLMTR:
			/* WTF2. Print a dummp entry to close this un-normal case. */
			/* Print: Key End, Delimiter, Empty Value, */
			js_print_attr_delimiter();
			js_print_attr_value_end_quote();
			entry_cnt++;
			state = JS_WAIT_KEY;
			break;
		case JS_WAIT_VALUE:
			/* Value ended. It is an empty value. */
			/* Print Value End Quate. */
			js_print_attr_value_end_quote();
			state = JS_WAIT_KEY;
			break;
		case JS_WAIT_END:
			/* Value ended. */
			/* Print Value End Quate. */
			js_print_attr_value_end_quote();
			state = JS_WAIT_KEY;
			break;
	}
	js_print_obj_end();
}

static int get_keys_json(struct cfg_s *c, struct key_pair_s *get_key_pool, const unsigned int key_pool_size, char *buf)
{
	struct key_pair_s *key_pair = get_key_pool;
	char *matched_value;
	int i, ret = 0, cnt = 0;

	/* print delimiter */
	if (c->matched_node_cnt)
		js_print_delimiter();

	js_print_obj_start();

	for (i = 0; i < key_pool_size; i++, key_pair++) {
		if (!key_pair->key)
			continue;

		if (buf)
			matched_value = getParam(buf, key_pair->key);
		else
			matched_value = NULL;

		/* Save string pointer back to get_key_pool. */
		/* NOTE: The string points to param_buf, which will be overwrite on next receive. */
		key_pair->value = matched_value;

		/* print delimiter */
		if (cnt)
			js_print_delimiter();

		js_print_attr(key_pair->key, key_pair->value);
		cnt++;
	}

	js_print_obj_end();
	return ret;
}

static int get_keys(struct cfg_s *c, struct key_pair_s *get_key_pool, const unsigned int key_pool_size, char *buf)
{
	struct key_pair_s *key_pair = get_key_pool;
	char *matched_value;
	int i, ret = 0;

	for (i = 0; i < key_pool_size; i++, key_pair++) {
		if (!key_pair->key)
			continue;

		if (buf)
			matched_value = getParam(buf, key_pair->key);
		else
			matched_value = NULL;

		/* Save string pointer back to get_key_pool. */
		/* NOTE: The string points to param_buf, which will be overwrite on next receive. */
		key_pair->value = matched_value;

		if (!matched_value) {
			msg("%s=\"\"\n", key_pair->key);
			continue;
		}

		msg("%s=%s\n", key_pair->key, key_pair->value);
	}
done:
	return ret;
}

int write_to_file(struct cfg_s *c, char *buf, unsigned int buf_size)
{
	FILE *out = NULL;
	int ret = -1;

	if ((ret = lock_node_query_file(c)) < 0)
		goto done;

	out = fopen(c->output_file_name, "w+");
	if (!out)
		goto done;

	ret = fwrite(buf, buf_size, 1, out);
	if (!ret)
		goto done;

	fflush(out);

	ret = 0;

done:
	if (out)
		fclose(out);

	unlock_node_query_file(c);
	return ret;
}

int on_node_received(struct cfg_s *c, char *param_buf, unsigned int data_len, unsigned int max_buf_size)
{
	int ret = 0;
	unsigned int action = c->action;

	/*
	** Actions are designed running in following order:
	** - match keys first
	** - set keys
	** - get or dump keys: print out keys in interests
	*/
	c->received_node_cnt++;
	/* take action */
	if (action & ACTION_MATCH_KEY) {
		ret = match_keys(c, c->match_key_pool, KEY_POOL_SIZE, param_buf);
		if (ret < 0) /* means key doesn't full match */
			goto not_match;
	}
	/* Treat potentional errors as not matched and ignore. */
	if (action & ACTION_SET_KEY) {
		ret = set_keys(c, c->set_key_pool, KEY_POOL_SIZE, param_buf, data_len, max_buf_size);
		if (ret < 0)
			goto not_match;
		/* Update new param data len. */
		data_len = ret;
	}
	if (action & ACTION_WRITE_FILE) {
		ret = write_to_file(c, param_buf, data_len);
		if (ret < 0)
			goto not_match;
	}
	/* Start print out matched node. */
	if (action & ACTION_GET_KEY) {
		if (action & ACTION_JSON_FORMAT)
			get_keys_json(c, c->get_key_pool, KEY_POOL_SIZE, param_buf);
		else
			get_keys(c, c->get_key_pool, KEY_POOL_SIZE, param_buf);
	}
	else if (action & ACTION_DUMP) {
		if (action & ACTION_JSON_FORMAT) {
			dump_param_json(c, param_buf, data_len);
		} else {
			if (c->matched_node_cnt)
				msg("-------------------------------------\n");
			dumpParam(param_buf, data_len);
		}
	}
	/*
	** matched_node_cnt is needed in output print.
	** Must update AFTER output print.
	*/
	c->matched_node_cnt++;
not_match:
	return ret;
}

int query_node_from_file(struct cfg_s *c)
{
	c->param_buf_data_len = 0;
	if (c->input_file) {
		do {
			unsigned int s;

			if (lock_node_query_file(c) < 0) {
				err("lock_node_query_file failed?!\n");
				break;
			}

			if (fseek(c->input_file, 0L, SEEK_END) != 0) {
				err("read file no EOF?! (%d:%s)\n", errno, strerror(errno));
				break;
			}
			s = ftell(c->input_file);
			if (s == -1) {
				err("read file size error?! (%d:%s)\n", errno, strerror(errno));
				break;
			}
			dbg("Start reading (%d:%s) in %d bytes\n", fileno(c->input_file), c->input_file_name, s);
			if (s) {
				rewind(c->input_file);
				c->param_buf_data_len = fread(c->param_buf, s, 1, c->input_file);
				if ((c->param_buf_data_len < 1) && ferror(c->input_file)) {
					err("read file error?! (%d:%s)\n", errno, strerror(errno));
					break;
				}
				c->param_buf_data_len = s;
			}
		} while (0);
		fclose(c->input_file);
		c->input_file = NULL;
		unlock_node_query_file(c);

		/* Touch of. Clear output file in case node_query fail or timeout. */
		if (c->action & ACTION_WRITE_FILE) {
			if (!strcmp(c->input_file_name, c->output_file_name))
				clear_output_file(c);
		}
	}

	/* Patch for empty or invalid file */
	if (c->param_buf_data_len < MIN_ASTPARAM_DATA_SIZE) {
		dbg("read file error?! (%d)\n", c->param_buf_data_len);
		/* create an empty param data. */
		memset(c->param_buf, 0, MIN_ASTPARAM_DATA_SIZE);
		// FIXME. update crc?
		c->param_buf_data_len = MIN_ASTPARAM_DATA_SIZE;
	}

	return on_node_received(c, c->param_buf, c->param_buf_data_len, ASTPARAM_BLOCK_SIZE);
}

inline int mac_list_append(struct nqpkt_hdr_s *nq_pkt_hdr, char *mac_list, unsigned long long mac)
{
	char *mac_list_max = mac_list + nq_pkt_hdr->block2_size;
	char new_mac[6];
	char *imac;

	if (nq_pkt_hdr->block2_size >= MAX_NODE_SIZE * 6) {
		err("mac_list full!!\n");
		return -1;
	}
	/* in big endian */
	new_mac[5] = (char)(mac >> 0);
	new_mac[4] = (char)(mac >> 8);
	new_mac[3] = (char)(mac >> 16);
	new_mac[2] = (char)(mac >> 24);
	new_mac[1] = (char)(mac >> 32);
	new_mac[0] = (char)(mac >> 40);
	/* filter out duplicate */
	for (imac = mac_list; imac < mac_list_max; imac += 6) {
		if (!memcmp(new_mac, imac, 6)) {
			/* found!! match */
			info("Received duplicate node %02X:%02X:%02X:%02X:%02X:%02X skip\n"
			     , new_mac[0]
			     , new_mac[1]
			     , new_mac[2]
			     , new_mac[3]
			     , new_mac[4]
			     , new_mac[5]);
			return -2;
		}
	}
	/* append */
	memcpy(mac_list_max, new_mac, 6);
	nq_pkt_hdr->block2_size += 6;

	return 0;
}

int on_node_received_from_net(struct cfg_s *c, char *param_buf, unsigned int data_len, unsigned int max_buf_size)
{
	char *matched_value;
	unsigned long long mac;

	matched_value = getParam(param_buf, "MY_MAC");
	//dump_buf(&mac, 6);
	if (matched_value) {
		mac = strtoull(matched_value, NULL, 16);
		/* return if duplicated. */
		if (mac_list_append(&c->nq_pkt_hdr, c->mac_list, mac))
			return 0;
	} else {
		/* return, if not exists. */
		dbg("node info without MY_MAC?! Skip.\n");
		return 0;
	}
	return on_node_received(c, param_buf, data_len, max_buf_size);
}

int on_node_responding_from_net(struct cfg_s *c)
{
	struct nqpkt_hdr_s *nq_pkt;
	int ret = -1;
	int nbytes;
	char *pkt_param_buf;
	/* receive node */
	nbytes = read(c->query_rx_fd, c->param_buf, ASTPARAM_BLOCK_SIZE);
	if (nbytes < 0) {
		err("node receiving failed?! (%d:%s)\n", errno, strerror(errno));
		goto err;
	}
	dbg("node received %d bytes\n", nbytes);
	nq_pkt = (struct nqpkt_hdr_s *)c->param_buf;
	dump_buf(nq_pkt, nbytes);
	if (nq_pkt->magic != NQ_PKT_MAGIC) {
		err("wrong magic num (%08X)\n", nq_pkt->magic);
		goto done;
	}
	if (nbytes != NQPKT_TOTAL_SIZE(nq_pkt)) {
		err("wrong node response packet size?! (%d vs %d)\n", nbytes, NQPKT_TOTAL_SIZE(nq_pkt));
		/* ignore */
		goto done;
	}
	/* NOTE: response info doesn't contain block2 mac_list. */
	/* handle received node info. */
	nbytes = ASTPARAM_BLOCK_SIZE - NQPKT_HDR_SIZE;
	on_node_received_from_net(c,
	                          nq_pkt->payload,
	                          nq_pkt->block1_size,
	                          nbytes);
done:
	ret = 0;
err:
	return ret;
}

int fill_key_pairs(
	struct cfg_s *c,
	struct key_pair_s *key_pool,
	const unsigned int key_pool_size,
	char *buf,
	unsigned int max_buf_size)
{
	struct key_pair_s *key_pair = key_pool;
	int i;
	int len_to_write = -1;

	for (i = 0; i < key_pool_size; i++, key_pair++) {
		if (!key_pair->key)
			continue;

		len_to_write = setParam(buf, max_buf_size, key_pair->key, key_pair->value);
		/* return, if not exists. */
		if (len_to_write < 0)
			break;
	}
done:
	return len_to_write;
}

int init_node_query_pkt(struct cfg_s *c, struct nqpkt_hdr_s *nq_pkt, char *nq_param_buf, unsigned int max_buf_size)
{
	int len_to_write = -1;

	/* fill up query_buf. */
	memset(nq_pkt, 0, sizeof(struct nqpkt_hdr_s));
	memset(nq_param_buf, 0, max_buf_size);

	nq_pkt->magic = NQ_PKT_MAGIC;

	/* payload always has 'reply_type' key value. */
	if (c->reply_type)
		len_to_write = setParam(nq_param_buf, max_buf_size, "REPLY_TYPE", c->reply_type);
	else
		len_to_write = setParam(nq_param_buf, max_buf_size, "REPLY_TYPE", NQPKT_DEFAULT_REPLY_TYPE);

	if (len_to_write < 0) {
		dbg("Can't generate node query packet!?\n");
		goto err;
	}
	if (c->action & ACTION_MATCH_KEY) {
		len_to_write = fill_key_pairs(c,
									c->match_key_pool,
									KEY_POOL_SIZE,
									nq_param_buf,
									max_buf_size);
		if (len_to_write < 0) {
			dbg("Out of node query packet size limit?!\n");
			goto err;
		}
	}

	nq_pkt->block1_size = len_to_write;

	/* NOTE: init mac_list. Suppose already initialized to 0 when program start. */
err:
	return len_to_write;
}

int send_node_query(
		struct cfg_s *c,
		struct nqpkt_hdr_s *nq_pkt,
		char *param_buf,
		char *mac_list)
{
#define IOV_NUM 3
	int nbytes;
	struct msghdr msg_hdr;
	struct iovec iov[IOV_NUM];

	/* Update nq_pkt->query_period */
	nq_pkt->query_period = (uint16_t)c->period_ms;

	//TODO. How to recover over UDP limit (64KB) case?
	/* Send query packet. */
	memset(&iov, 0, sizeof(struct iovec) * IOV_NUM);
	memset(&msg_hdr, 0, sizeof(struct msghdr));
	iov[0].iov_base = (void *)nq_pkt;
	iov[0].iov_len = NQPKT_HDR_SIZE;
	iov[1].iov_base = (void *)param_buf;
	iov[1].iov_len = nq_pkt->block1_size;
	iov[2].iov_base = (void *)mac_list;
	iov[2].iov_len = nq_pkt->block2_size;
	msg_hdr.msg_iov = iov;
	msg_hdr.msg_iovlen = IOV_NUM;

	nbytes = sendmsg(c->query_tx_fd, &msg_hdr, 0);
	if (nbytes < 0) {
		err("node query send failed?! (%d:%s)", errno, strerror(errno));
		if (errno == EMSGSIZE) {
			err("size over 64KB! (%d)\n", NQPKT_TOTAL_SIZE(nq_pkt));
		}
	}
	return nbytes;
#undef IOV_NUM
}

unsigned int calc_timeout_period(struct cfg_s *c, unsigned int max_nodes, unsigned int received_nodes)
{
	unsigned int period;

	if (c->period_ms_overwrite) {
		period = c->period_ms_overwrite;
		goto done;
	}
	/*
	** We use max_prints as max nodes. We assume:
	** - if max_prints is 1 or small ==> there is strick match_key set and small
	**   reply expected.
	** - if max_prints is big ==> user expect node_list like behavior and should
	**   fill this value as expected network size.
	*/
	if (max_nodes > received_nodes)
		max_nodes -= received_nodes;
	/*
	** period_ms = ((S * max_node) / R) * 2
	** - 2 is a tolerance value
	** - R is desired data rate in KBytes/Sec
	** - S is response packet size in Bytes
	**
	** Assume:
	** - S = 200 bytes (estimate from essential reply_type)
	** - R = 10Mbps (1250KB/s)
	** If max_nodes is 500, period will be 160 ms.
	*/
	period = (NQ_ESSENTIAL_SIZE * max_nodes * 2) / NQ_NET_BANDWIDTH;

done:
	dbg("new period = %d\n", period);
	return period;
}

int query_node_from_net(struct cfg_s *c)
{
	int ret = -1, ne, retry = 0;
	struct epoll_event ep_event;
	int epfd;
	event_handler_func handler;
	int pre_recv_cnt;

	/* About size '5', Since Linux 2.6.8, the size argument is ignored, but must be greater than zero */
	epfd = epoll_create(5);
	if (epfd < 0) {
		perror("epoll_create failed");
		goto err;
	}

	/* Add node_query to epoll to wait for Rx event. */
	ep_event.events = EPOLLIN;
	ep_event.data.ptr = (void *)on_node_responding_from_net;
	if (epoll_ctl(epfd, EPOLL_CTL_ADD, c->query_rx_fd, &ep_event) < 0) {
		perror("epoll_ctl failed");
		goto err;
	}

	if (init_node_query_pkt(c, &c->nq_pkt_hdr, c->nq_param_buf, ASTPARAM_BLOCK_SIZE) < 0)
		goto err1;

	memset(&ep_event, 0, sizeof(ep_event));

	/* c->period_ms must be valid before calling send_node_query() */
	c->period_ms = calc_timeout_period(c, c->max_prints, c->received_node_cnt);
	/* Send node_query */
	send_node_query(c, &c->nq_pkt_hdr, c->nq_param_buf, c->mac_list);

	do {
		ne = epoll_wait(epfd, &ep_event, 1, c->period_ms + NQ_TIMEOUT);
		if (ne < 0) {
			/* something wrong. */
			perror("epoll_wait failed?!");
			break;
		}
		/* case for epoll/node_query timeout. */
		if (ne == 0) {
			if (++retry >= 3) {
				break;
			} else {
				dbg("node_query timeout (%d)\n", retry);
				/* Send node_query */
				send_node_query(c, &c->nq_pkt_hdr, c->nq_param_buf, c->mac_list);
				continue;
			}
		}
		/*
		** If mac_list is full, code will ignore new nodes and
		** c->received_node_cnt should not count up.
		** So, we are sure that query will eventually stop.
		**
		** Check matched_node_cnt may not be accurate comparing to received_node_cnt.
		*/
		pre_recv_cnt = c->received_node_cnt;

		/* Handle fd events. */
		handler = (event_handler_func)ep_event.data.ptr;
		if (handler(c))
			break;

		/* retry more if we got response. */
		if (retry && (c->received_node_cnt != pre_recv_cnt))
			retry--;

		/* max_prints == 0 means no limit. */
		if (c->max_prints == 0)
			continue;
		else if (c->matched_node_cnt >= c->max_prints)
			break;

		/* Update period_ms if received_node_cnt changes. */
		c->period_ms = calc_timeout_period(c, c->max_prints, c->received_node_cnt);
	} while(1);

	ret = 0;
err1:
	epoll_ctl(epfd, EPOLL_CTL_DEL, c->query_rx_fd, NULL);
err:
	if (epfd)
		close(epfd);

	dbg("Total %d nodes replied and %d matched\n", c->received_node_cnt, c->matched_node_cnt);

#if 0
	/*
	** Bruce170620. I think printing an empty GET_KEY value is confusing.
	** The caller should take care of no-result case by itself.
	*/
	/* Print empty key/value pairs if ACTION_GET_KEY specified. */
	if (c->matched_node_cnt == 0) {
		if (c->action & ACTION_GET_KEY) {
			get_keys(c, c->get_key_pool, KEY_POOL_SIZE, NULL);
		}
	}
#endif
	return ret;
}

int query_node(struct cfg_s *c)
{
	int ret = 0;
	unsigned int action = c->action;

	print_header(action);

	if (action & ACTION_NODE_FROM_FILE) {
		ret = query_node_from_file(c);
	} else {
		ret = query_node_from_net(c);
	}

	print_footer(action, c->matched_node_cnt);

	return ret;
}

int clear_output_file(struct cfg_s *c)
{
	FILE *out = NULL;
	int ret = -1;

	if (!(c->action & ACTION_WRITE_FILE))
		goto done;

	if (!c->output_file_name)
		goto done;

	if ((ret = lock_node_query_file(c)) < 0)
		goto fail;

	out = fopen(c->output_file_name, "w");
	if (!out) {
		err("open output file failed?! (%d:%s)\n", errno, strerror(errno));
		goto fail;
	}

	fflush(out);

done:
	ret = 0;
fail:
	if (out)
		fclose(out);

	unlock_node_query_file(c);

	return ret;
}

int start_node_query(struct cfg_s *c)
{
	int ret = 0;

	/* flock() */
	if ((ret = lock_node_query(c)) < 0)
		goto done;

	/* Touch of. Clear output file in case node_query fail or timeout. */
	if (c->action & ACTION_WRITE_FILE) {
		/* FIX: Output file may == input file, we should not clear output file before read it. */
		if (!((c->action & ACTION_NODE_FROM_FILE) && !strcmp(c->input_file_name, c->output_file_name))) {
			/* Let's clear output file.' */
			if ((ret = clear_output_file(c)) < 0)
				goto done;
		}
	}
	/* Open if */
	if ((ret = open_input_file(c)) < 0)
		goto done;
	/* Open sockets if necessary */
	if ((ret = open_sockets(c)) < 0)
		goto done;

	/* query node info */
	ret = query_node(c);

done:
	/* close sockets */
	if (c->query_rx_fd)
		close(c->query_rx_fd);

	if (c->query_tx_fd)
		close(c->query_tx_fd);

	/* close if/of files */
	if (c->input_file)
		fclose(c->input_file);

	unlock_node_query(c);
	return ret;
}

/*
** Parse commands:
** - cfg: period, reply_type, path, max, add_key_prefix, default_from
** - query: if
** - filter: match_key
** - action: set_key, dump, get_key
** - result: of
*/
int main(int argc, char *argv[])
{
	int opt = 0;
	int long_index =0;
	struct cfg_s *c = &cfg;

	/* Specifying the expected options */
	static struct option long_options[] = {
		{"if",               required_argument,       0,  'a' },
		{"of",               required_argument,       0,  'b' },
		{"match_key",        required_argument,       0,  'c' },
		{"set_key",          required_argument,       0,  'd' },
		{"get_key",          required_argument,       0,  'e' },
		{"dump",             no_argument,             0,  'f' },
		{"period",           required_argument,       0,  'g' },
		{"reply_type",       required_argument,       0,  'h' },
		{"path",             required_argument,       0,  'i' },
		{"max",              required_argument,       0,  'j' },
		{"add_key_prefix",   required_argument,       0,  'k' },
		{"default_from",     required_argument,       0,  'l' },
		{"json",             no_argument,             0,  'm' },
		{"intf",             required_argument,       0,  'n' },
		{0,                  0,                       0,  0   }
	};

	memset(c, 0, sizeof(struct cfg_s));
	/* limit max_prints to MAX_NODE_SIZE in case something wrong. */
	//c->max_prints = MAX_NODE_SIZE;
	/*
	** Set default max_prints to a smaller typical network size.
	** So that we can have a more optimized calc_timeout_period() value.
	*/
	c->max_prints = NQ_DEFAULT_MAX;
	c->local_intf = (unsigned int)INADDR_ANY;

	while ((opt = getopt_long_only(argc, argv, "", long_options, &long_index )) != -1) {
		switch (opt) {
		case 'a': /* --if */
			c->action |= ACTION_NODE_FROM_FILE;
			c->input_file_name = optarg;
			dbg("if=%s\n", c->input_file_name);
			break;
		case 'b': /* --of */
			c->action |= ACTION_WRITE_FILE;
			c->output_file_name = optarg;
			dbg("of=%s\n", c->output_file_name);
			break;
		case 'c': /* --match_key */
			c->action |= ACTION_MATCH_KEY;
			if (add_key_pair_to_pool(c->match_key_pool, KEY_POOL_SIZE, optarg))
				goto err;
			break;
		case 'd': /* --set_key */
			c->action |= ACTION_SET_KEY;
			if (add_key_pair_to_pool(c->set_key_pool, KEY_POOL_SIZE, optarg))
				goto err;
			break;
		case 'e': /* --get_key */
			c->action |= ACTION_GET_KEY;
			if (add_key_pair_to_pool(c->get_key_pool, KEY_POOL_SIZE, optarg))
				goto err;
			break;
		case 'f': /* --dump */
			c->action |= ACTION_DUMP;
			break;
		case 'g': /* --period */
			c->period_ms_overwrite = atoi(optarg);
			break;
		case 'h': /* --reply_type */
			c->reply_type = optarg;
			break;
		case 'i': /* --path */
			// node_query and node_responser must use the same path. So, don't open this option for now.
			dbg("%d\n", __LINE__);
			break;
		case 'j': /* --max */
			c->max_prints = atoi(optarg);
			break;
		case 'k': /* --add_key_prefix */
			dbg("%d\n", __LINE__);
			break;
		case 'l': /* --default_from */
			dbg("%d\n", __LINE__);
			break;
		case 'm': /* --json */
			c->action |= ACTION_JSON_FORMAT;
			break;
		case 'n': /* --intf local_ip_addr */
			c->local_intf = (unsigned int)inet_addr(optarg);
			break;
		default:
			print_usage();
			exit(EXIT_FAILURE);
		}
	}

	chdir(NQ_DEFAULT_PATH);

	return start_node_query(c);
err:
	return -1;
}

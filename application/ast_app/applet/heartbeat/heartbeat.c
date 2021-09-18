/*
 * Copyright (c) 2004-2016
 * ASPEED Technology Inc. All Rights Reserved
 * Proprietary and Confidential
 *
 * By using this code you acknowledge that you have signed and accepted
 * the terms of the ASPEED SDK license agreement.
 */

#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include <sched.h>
#include <sys/epoll.h>
#include <signal.h>
#include <errno.h>
#include <sys/time.h>
#include <stdlib.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netinet/igmp.h>
#include <netdb.h> //for gethostbyname()
#include <linux/ip.h>
#include <linux/igmp.h>
#include "uds_ipc.h"
#include "ipc.h"
#include "astnetwork.h"
#include "debug.h"
#include "multi_timer.h"

#define HB_MCAST_PINGER_IGMP_REPORT
#define HB_MCAST_ACKER_IGMP_REPORT
#undef HB_MCAST_ACKER_IGMP_QUERIER

#if defined(HB_MCAST_PINGER_IGMP_REPORT) || defined(HB_MCAST_ACKER_IGMP_REPORT)
/*
 * for multicast,
 * we expect IGMP report gets sent after socket option setup (IP_ADD_MEMBERSHIP) in udp_create_sender()
 *
 * But linux kernel only send 'IGMP report' when device joins into a new multicast group.
 * (please refer to ip_mc_inc_group() in net/ipv4/igmp.c for more details)
 *
 * If multiple users on single one device join the same one multicast group,
 * linux always increase user counter and send just one IGMP report out.
 * (and hand over to timer for consequent IGMP report if necessary)
 * The multicast entry with more than one user will get remove from system if all of sockets get closed.
 *
 * To define HB_MCAST_IGMP_REPORT_AT_HB_INIT is to force IGMP report sending at heartbeat initial stage.
 * We need this feature because not all sockets get closed before start of service (ex. USB & KMOIP case).
 */
#define HB_MCAST_IGMP_REPORT_AT_HB_INIT
#endif /* #if defined(HB_MCAST_PINGER_IGMP_REPORT) || defined(HB_MCAST_ACKER_IGMP_REPORT) */

/*
 * RFC2236,
 * 2. Introduction
 * All IGMP messages described in this document are sent with IP TTL 1,
 * and contain the IP Router Alert option [RFC 2113] in their IP header.
 */
#define IGMPV2_IPOPT_RA /* IP option - router alert */

#define MAX_IP_STR_LEN            15
#define MAX_PAYLOAD               4096
#define MAX_E_STR_BUF_SIZE        512 /* max 512 characters for event string */
#define HB_MAGIC_NUM              (0x19760614)
#define HB_TIMEOUT_MSEC           (5000) /* in msec. */
#define HB_BACKOFF_INIT_BASE      (1)    /* in msec. 0 is invalid for timer. */
#define HB_BACKOFF_INIT_INTERVAL  (32 - 1)  /* in msec. MUST be (2^x -1) */
#define HB_BACKOFF_BASE           (16)  /* in msec. */
#define HB_BACKOFF_INTERVAL       (256 - 1) /* in msec. MUST be (2^x -1) */
#define HB_MCAST_QUERIER_PERIOD   (65536 - 1) /* in msec  MUST be (2^x -1) */
#define HB_MCAST_REPORT_PERIOD    (65536 - 1) /* in msec  MUST be (2^x -1) */

enum hb_pkt_type_e {
	HB_PING = 0,
	HB_ACK,
	HB_CLOSE,
	HB_MSG
};

struct hb_pkt_s {
	unsigned int magic;
	unsigned int type;
	unsigned int session_id; /* A hb session id to distinguish different hb sessions from the same pinger. */
	unsigned int match_session_id; /* Only accept when seesion_id matched. */
	unsigned int payload_size; /* size of payload in bytes */
	unsigned char payload[];
};

enum ctrl_type_e {
	TYPE_UNKNOWN = 0,
	VIDEO,
	AUDIO,
	USB,
	IR,
	SERIAL,
	PUSHBUTTON,
	CEC,
	TYPE_MAX
};

const char *type_str[] = {
	"unknown",
	"v",
	"a",
	"u",
	"r",
	"s",
	"p",
	"c",
	"invalid"
};

enum ctrl_cmd_e {
	CMD_UNKNOWN = 0,
	START_PINGER,
	STOP_PINGER,
	START_ACKER,
	STOP_ACKER,
	MSG,
	EXIT,
	CMD_MAX
};

const char *cmd_str[] = {
	"unknown",
	"start_pinger",
	"stop_pinger",
	"start_acker",
	"stop_acker",
	"msg",
	"exit",
	"invalid"
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

struct ctrl_cmd {
	enum ctrl_type_e type;
	unsigned int session_id; /* TODO. limit to 32bits long. */
	enum ctrl_cmd_e cmd;
	enum ctrl_casting_e casting;
	unsigned char ping_ip[MAX_IP_STR_LEN + 1]; //xxx.xxx.xxx.xxx
	unsigned int ping_port;
	unsigned char msg_to_acker; //1: to acker without specific session_id, 2: to acker with specific session_id, 0: not to acker.
	unsigned char msg[MAX_PAYLOAD];
};

/*
** Total handlers includes
** - all hb channels: MAX_PINGER_HB_CH
** - ctrl handler: 1
** - acker: 1
*/
typedef int (*event_handler_func)(void *, void *);

struct do_event_s {
	event_handler_func handler;
	void *p1;
	void *p2;
};

struct hb_ch_s {
	unsigned int ref_cnt; /* usage tracking counter. */
	int hb_rx_fd;
	int hb_tx_fd;
	unsigned int ping_ip; /* in inet_addr() format. */
	unsigned int ping_port;
	enum ctrl_casting_e casting; /* casting mode. */
	unsigned int my_ip; /* in inet_addr() format. */
	unsigned int is_alive;
	unsigned int peer_session_id; /* A hb session id to distinguish different hb sessions from the same pinger. */
	unsigned int my_session_id;
	struct mtimer_s hb_timer; /* heartbeat timer. Timeout means no more heartbeats. */
	struct do_event_s event_handle;
	unsigned int is_pinger; /* is pinger or acker? */
	/* pinger only */
	unsigned int acked; /* whether this pinger is received by the acker or not. */
	unsigned int ack_cnt;
	struct mtimer_s backoff_timer;
#if defined(HB_MCAST_PINGER_IGMP_REPORT) || defined(HB_MCAST_ACKER_IGMP_REPORT)
	struct mtimer_s mcast_report_timer; /* multicast report timer */
#endif
	/* acker only */
	unsigned int last_pinger_ip; /* in inet_addr() format. */
	unsigned int first_pinger_ip; /* in inet_addr() format. */
#if defined(HB_MCAST_ACKER_IGMP_QUERIER)
	struct mtimer_s mcast_querier_timer; /* multicast querier timer */
#endif
};

struct tcmap_s {
	struct hb_ch_s *hb;
	unsigned int session_id;
};

void on_pinger_hb_timeout(struct hb_ch_s *hb);
void on_acker_hb_timeout(struct hb_ch_s *hb);
void handle_type_pinger_hb_timeout(struct tcmap_s *tcmap_item, enum ctrl_type_e type);
void announce_acker_close(struct hb_ch_s *hb);
void announce_pinger_close(struct hb_ch_s *hb);
void handle_msg(struct hb_ch_s *hb, char *msg, unsigned int msg_len);

/* Global variables. */
/* event poll fd */
int epfd = -1;
int kill_me = 0;
unsigned int hb_session_id;
unsigned char cmd_buffer[MAX_PAYLOAD];

#define MAX_PINGER_HB_CH TYPE_MAX
#define MAX_ACKER_HB_CH TYPE_MAX
struct hb_ch_s hb_pinger_list[MAX_PINGER_HB_CH];
struct hb_ch_s hb_acker_list[MAX_ACKER_HB_CH];
#define PINGER_HB2CH(hb) (((unsigned int)hb - (unsigned int)hb_pinger_list) / sizeof(struct hb_ch_s))
#define ACKER_HB2CH(hb) (((unsigned int)hb - (unsigned int)hb_acker_list) / sizeof(struct hb_ch_s))
/* NOTE. We assume both pinger and acker have the same MAX_TCMAP value. */
#define MAX_TCMAP TYPE_MAX
/*
** An array indexed by TYPE, save hb_pinger_list[] and hb_acker_list[] mapping pointer.
*/
struct tcmap_s type_pinger_hb_mapping[MAX_TCMAP];
struct tcmap_s type_acker_hb_mapping[MAX_TCMAP];

/* type to channel mapping (tcmap) init */
typedef int (*hb_construct_func)(struct hb_ch_s *hb, unsigned int ping_ip, unsigned int ping_port, enum ctrl_casting_e casting);
typedef void (*hb_destruct_func)(struct hb_ch_s *hb);
typedef void (*type_hb_match_func)(struct tcmap_s *tcmap_item, enum ctrl_type_e type);

static inline void tcmap_list_init(struct tcmap_s *tcmap, unsigned int max_tcmap_item)
{
	memset((void *)tcmap, 0, sizeof(struct tcmap_s) * max_tcmap_item);
}

static inline void tcmap_item_init(struct tcmap_s *tcmap_item)
{
	tcmap_item->hb = NULL;
	tcmap_item->session_id = 0;
}

static inline struct tcmap_s *tcmap_item_by_type(struct tcmap_s *tcmap, enum ctrl_type_e type)
{
	return &tcmap[type];
}

#if 0
static inline struct hb_ch_s *tcmap_type_to_hb(struct tcmap_s *tcmap, enum ctrl_type_e type)
{
	/* 0 means no mapping yet. */
	return tcmap[type].hb;
}

static inline void tcmap_map_hb_to_type(struct tcmap_s *tcmap, struct hb_ch_s *hb, enum ctrl_type_e type)
{
	tcmap[type].hb = hb;
}
#endif

static struct hb_ch_s *tcmap_alloc_hb(
	struct hb_ch_s *hb_list,
	const unsigned int hb_list_size,
	unsigned int ip,
	unsigned int port,
	enum ctrl_casting_e casting,
	hb_construct_func construct)
{
	struct hb_ch_s *hb = hb_list, *first_invalid = NULL;
	int i;
	/* Reuse hb when ping target is the same. */
	for (i = 0; i < hb_list_size; i++, hb++) {
		/* is this hb in use? */
		dbg("HBCH[%d][%p] cnt=%u\n", i, hb, hb->ref_cnt);
		if (!hb->ref_cnt) {
			if (!first_invalid) {
				first_invalid = hb;
				dbg("first_invalid[%d][%p]\n", i, first_invalid);
			}
			/* not in use, try next hb. */
			continue;
		}
		/* Is in use?, match ping_ip and ping_port */
		if ((hb->ping_ip == ip) && (hb->ping_port == port))
			break;
	}
	if (i == hb_list_size) {
		/* duplicate not found. return first invalid hb. */
		hb = first_invalid;
		if (!hb) {
			//BUG. Use the first ch to keep the app working.
			err("WTF %d\n", __LINE__);
			goto out;
		}
		/* do the hb socket alloc things. */
		if (construct(hb, ip, port, casting)) {
			err("WTF %d\n", __LINE__);
			hb = NULL;
			goto out;
		}
	}
out:
	if (hb)
		hb->ref_cnt++;
	return hb;
}

static inline struct hb_ch_s *tcmap_alloc_pinger_hb(unsigned int ip, unsigned int port, enum ctrl_casting_e casting, hb_construct_func construct)
{
	return tcmap_alloc_hb(hb_pinger_list, MAX_PINGER_HB_CH, ip, port, casting, construct);
}

static inline struct hb_ch_s *tcmap_alloc_acker_hb(unsigned int ip, unsigned int port, enum ctrl_casting_e casting, hb_construct_func construct)
{
	return tcmap_alloc_hb(hb_acker_list, MAX_ACKER_HB_CH, ip, port, casting, construct);
}

static void tcmap_free_hb(struct hb_ch_s *hb, hb_destruct_func destruct)
{
	if (!hb->ref_cnt) {
		//BUG!?
		dbg("WTF!? %d\n", __LINE__);
		return;
	}
	hb->ref_cnt--;
	/* free if no more used. */
	if (!hb->ref_cnt) {
		destruct(hb);
	}
}

static inline void tcmap_for_each_type(struct tcmap_s *tcmap, type_hb_match_func match_handler)
{
	unsigned int i;

	for (i = 0; i < MAX_TCMAP; i++, tcmap++) {
		match_handler(tcmap, i);
	}
}

static inline void tcmap_for_each_type_matches_hb(struct tcmap_s *tcmap, struct hb_ch_s *hb, type_hb_match_func match_handler)
{
	unsigned int i;

	for (i = 0; i < MAX_TCMAP; i++, tcmap++) {
		if (tcmap->hb != hb)
			continue;
		/* Matches. Handle it. */
		match_handler(tcmap, i);
	}
}

void *reg_do_event_handler(struct do_event_s *e, event_handler_func func, void *p1, void *p2)
{
	e->handler = func;
	e->p1 = p1;
	e->p2 = p2;

	return (void *)e;
}

void dump_cmd(struct ctrl_cmd *cmd)
{
#if 0
	printf("====================\n");
	printf("cmd=%s\n", cmd_str[cmd->cmd]);
	printf("type=%s\n", type_str[cmd->type]);
	printf("session_id=%d\n", cmd->session_id);
	printf("casting=%s\n", casting_str[cmd->casting]);
	printf("ping_ip=%s\n", cmd->ping_ip);
	printf("ping_port=%d\n", cmd->ping_port);
	printf("====================\n");
#endif
}

void dump_hb_pkt(struct hb_pkt_s *pkt)
{
#if 0
	printf("====================\n");
	printf("magic=0x%08X\n", pkt->magic);
	printf("type=%d\n", pkt->type);
	printf("payload_size=%d\n", pkt->payload_size);
	if (pkt->payload_size)
		printf("payload=%d\n", *((int *)pkt->payload));
	printf("====================\n");
#endif
}

/* This function MUST be called with valid hb->my_ip. */
static inline void update_hb_my_session_id(struct hb_ch_s *hb)
{
	/*
	** Use lower 3 bytes of IP address to avoid session_id collision.
	*/
	hb->my_session_id = (hb->my_ip & 0xFFFFFF00) | (++hb_session_id & 0xFFUL);
	//err("\n\n!!!!!!new_session_id:0x%08X\n\n", hb->my_session_id);
}

static inline unsigned int calc_hb_pkt_size(struct hb_pkt_s *pkt)
{
	/* A valid hb packet's size == header size + payload size. */
	return (sizeof(struct hb_pkt_s) + pkt->payload_size);
}

int parse_cmd_start_pinger(unsigned char *cmd_buffer, struct ctrl_cmd *cmd)
{
	char *save_ptr;
	char *token;
	int i;
	/*
	** cmd format:
	** cmd:type:session_id:casting_mode:ping_ip:ping_port
	** Ex:
	** start_pinger:u:session_id:mc:225.10.1.100:99
	*/
	dbg("parse_cmd_start_pinger:[%s]\n", cmd_buffer);
	if (!cmd_buffer || !strlen(cmd_buffer))
		goto err;

	/* parse type */
	token = strtok_r(cmd_buffer, ":", &save_ptr);
	if (!token) {
		/* something wrong */
		dbg("%d\n", __LINE__);
		goto err;
	}
	for (i = (TYPE_UNKNOWN + 1); i < TYPE_MAX; i++) {
		//dbg("%d:%s vs %s\n", __LINE__, token, type_str[i]);
		if (!strncmp(token, type_str[i], strlen(type_str[i]))) {
			/* Found! */
			cmd->type = i;
			break;
		}
	}
	if (i == TYPE_MAX) {
		dbg("%d\n", __LINE__);
		goto err;
	}

	/* parse session id */
	token = strtok_r(NULL, ":", &save_ptr);
	if (!token) {
		/* something wrong */
		dbg("%d\n", __LINE__);
		goto err;
	}
	//strncpy(cmd->session_id, token, strlen(token));
	cmd->session_id = strtoul(token, NULL, 10);

	/* parse casting mode */
	token = strtok_r(NULL, ":", &save_ptr);
	if (!token) {
		/* something wrong */
		dbg("%d\n", __LINE__);
		goto err;
	}
	for (i = (CASTING_UNKNOWN + 1); i < CASTING_MAX; i++) {
		if (!strncmp(token, casting_str[i], strlen(casting_str[i]))) {
			/* Found! */
			cmd->casting = i;
			break;
		}
	}
	if (i == CASTING_MAX) {
		dbg("%d\n", __LINE__);
		goto err;
	}

	/* parse ping_ip */
	token = strtok_r(NULL, ":", &save_ptr);
	if (!token) {
		/* something wrong */
		dbg("%d\n", __LINE__);
		goto err;
	}
	strncpy(cmd->ping_ip, token, MAX_IP_STR_LEN);

	cmd->ping_port = strtoul(save_ptr, NULL, 0);

done:
	dump_cmd(cmd);
	return 0;
err:
	return -1;
}

int parse_cmd_stop_pinger(unsigned char *cmd_buffer, struct ctrl_cmd *cmd)
{
	char *save_ptr;
	char *token;
	unsigned int i;
	/*
	** cmd format:
	** cmd:type:session_id
	** Ex:
	** stop_pinger:v:session_id
	*/
	dbg("parse_cmd_stop_pinger:[%s]\n", cmd_buffer);
	if (!cmd_buffer || !strlen(cmd_buffer))
		goto err;
	/* parse type */
	token = strtok_r(cmd_buffer, ":", &save_ptr);
	if (!token) {
		/* something wrong */
		dbg("%d\n", __LINE__);
		goto err;
	}
	for (i = (TYPE_UNKNOWN + 1); i < TYPE_MAX; i++) {
		//dbg("%d:%s vs %s\n", __LINE__, token, type_str[i]);
		if (!strncmp(token, type_str[i], strlen(type_str[i]))) {
			/* Found! */
			cmd->type = i;
			break;
		}
	}
	if (i == TYPE_MAX) {
		dbg("%d\n", __LINE__);
		goto err;
	}

	/* parse session id */
	//strncpy(cmd->session_id, save_ptr, strlen(save_ptr));
	cmd->session_id = strtoul(save_ptr, NULL, 10);

	dump_cmd(cmd);
	return 0;
err:
	return -1;
}

int parse_cmd_start_acker(unsigned char *cmd_buffer, struct ctrl_cmd *cmd)
{
	char *save_ptr;
	char *token;
	int i;
	/*
	** cmd format:
	** cmd:type:session_id:casting_mode:ping_ip:ping_port
	** Ex:
	** start_acker:v:session_id:mc:225.0.100.100:1234
	*/
	dbg("parse_cmd_start_acker:[%s]\n", cmd_buffer);
	if (!cmd_buffer || !strlen(cmd_buffer))
		goto err;

	/* parse type */
	token = strtok_r(cmd_buffer, ":", &save_ptr);
	if (!token) {
		/* something wrong */
		dbg("%d\n", __LINE__);
		goto err;
	}
	for (i = (TYPE_UNKNOWN + 1); i < TYPE_MAX; i++) {
		//dbg("%d:%s vs %s\n", __LINE__, token, type_str[i]);
		if (!strncmp(token, type_str[i], strlen(type_str[i]))) {
			/* Found! */
			cmd->type = i;
			break;
		}
	}
	if (i == TYPE_MAX) {
		dbg("%d\n", __LINE__);
		goto err;
	}

	/* parse session id */
	token = strtok_r(NULL, ":", &save_ptr);
	if (!token) {
		/* something wrong */
		dbg("%d\n", __LINE__);
		goto err;
	}
	//strncpy(cmd->session_id, token, strlen(token));
	cmd->session_id = strtoul(token, NULL, 10);

	/* parse casting mode */
	token = strtok_r(NULL, ":", &save_ptr);
	if (!token) {
		/* something wrong */
		dbg("%d\n", __LINE__);
		goto err;
	}
	for (i = (CASTING_UNKNOWN + 1); i < CASTING_MAX; i++) {
		if (!strncmp(token, casting_str[i], strlen(casting_str[i]))) {
			/* Found! */
			cmd->casting = i;
			break;
		}
	}
	if (i == CASTING_MAX) {
		dbg("%d\n", __LINE__);
		goto err;
	}

	/* parse ping_ip */
	token = strtok_r(NULL, ":", &save_ptr);
	if (!token) {
		/* something wrong */
		dbg("%d\n", __LINE__);
		goto err;
	}
	strncpy(cmd->ping_ip, token, MAX_IP_STR_LEN);

	cmd->ping_port = strtoul(save_ptr, NULL, 0);

done:
	dump_cmd(cmd);
	return 0;
err:
	return -1;
}

int parse_cmd_stop_acker(unsigned char *cmd_buffer, struct ctrl_cmd *cmd)
{
	char *save_ptr;
	char *token;
	unsigned int i;
	/*
	** cmd format:
	** cmd:type:session_id
	** Ex:
	** stop_acker:v:session_id
	*/
	dbg("parse_cmd_stop_acker:[%s]\n", cmd_buffer);
	if (!cmd_buffer || !strlen(cmd_buffer))
		goto err;
	/* parse type */
	token = strtok_r(cmd_buffer, ":", &save_ptr);
	if (!token) {
		/* something wrong */
		dbg("%d\n", __LINE__);
		goto err;
	}
	for (i = (TYPE_UNKNOWN + 1); i < TYPE_MAX; i++) {
		//dbg("%d:%s vs %s\n", __LINE__, token, type_str[i]);
		if (!strncmp(token, type_str[i], strlen(type_str[i]))) {
			/* Found! */
			cmd->type = i;
			break;
		}
	}
	if (i == TYPE_MAX) {
		dbg("%d\n", __LINE__);
		goto err;
	}

	/* parse session id */
	//strncpy(cmd->session_id, save_ptr, strlen(save_ptr));
	cmd->session_id = strtoul(save_ptr, NULL, 10);

	dump_cmd(cmd);
	return 0;
err:
	return -1;
}

int parse_cmd_msg(unsigned char *cmd_buffer, struct ctrl_cmd *cmd)
{
	char *save_ptr;
	char *token;
	int i;
	/*
	** cmd format:
	** msg:type:to_whom:specific_ip:msg_content
	** Ex:
	** ipc @hb_ctrl s msg:v:acker:255.255.255.255:ve_btn1_pressed
	** ipc @hb_ctrl s msg:u:acker:255.255.255.255:ue_request_usb:$MY_IP
	** ipc @hb_ctrl s msg:u:pinger:169.254.0.33:ue_stop_usb
	** ipc @hb_ctrl s msg:u:pinger:169.254.0.34:ue_start_usb
	**
	** type: to assign which channel to use. received message will be fired to this type. See "Type List" below.
	** to_whom: to assign whom to receive:
	** 	- acker:
	** 	- pinger:
	** specific_ip:
	** 	- xxx.xxx.xxx.xxx: specify which IP to receive.
	**    Use 255.255.255.255 means IP not specified.
	**    Use 0.0.0.0 means IP not specified but MUST match
	**    'match_session_id'. A sepecial feature to identify acker.
	*/
	dbg("parse_cmd_msg:[%s]\n", cmd_buffer);
	if (!cmd_buffer || !strlen(cmd_buffer))
		goto err;

	/* Save the msg content first. Format: type:to_whom:msg_content */
	strcpy(cmd->msg, cmd_buffer);

	/* parse type */
	token = strtok_r(cmd_buffer, ":", &save_ptr);
	if (!token) {
		/* something wrong */
		dbg("%d\n", __LINE__);
		goto err;
	}
	for (i = (TYPE_UNKNOWN + 1); i < TYPE_MAX; i++) {
		//dbg("%d:%s vs %s\n", __LINE__, token, type_str[i]);
		if (!strncmp(token, type_str[i], strlen(type_str[i]))) {
			/* Found! */
			cmd->type = i;
			break;
		}
	}
	if (i == TYPE_MAX) {
		dbg("%d\n", __LINE__);
		goto err;
	}

	/* parse to_whom */
	token = strtok_r(NULL, ":", &save_ptr);
	if (!token) {
		/* something wrong */
		dbg("%d\n", __LINE__);
		goto err;
	}
	if (!strncmp(token, "acker", strlen("acker")))
		cmd->msg_to_acker = 1;
	else
		cmd->msg_to_acker = 0;

	if (cmd->msg_to_acker) {
		/* parse specific_ip */
		token = strtok_r(NULL, ":", &save_ptr);
		if (!token) {
			/* something wrong */
			dbg("%d\n", __LINE__);
			goto err;
		}
		/*
		** inet_addr() returns -1 if error. Which is the same as 255.255.255.255.
		** Bruce think it is fine to treat error as broadcast. If someday it is
		** not true, use inet_aton() instead.
		*/
		/* 0.0.0.0 means specific session_id. */
		if (inet_addr(token) == 0) {
			cmd->msg_to_acker = 2;
		}
	}

done:
	dump_cmd(cmd);
	return 0;
err:
	return -1;
}

int parse_cmd(char *cmd_buffer, struct ctrl_cmd *cmd)
{
	char *token;
	int i;
	/*
	** cmd format:
	** cmd:xxxxxxxxxxxxxxxxxxxxxxx
	** Ex:
	** start_pinger:v:session_id:uc:225.0.100.1:99
	*/
	if (!strlen(cmd_buffer))
		goto err;

	memset(cmd, 0, sizeof(struct ctrl_cmd));
	token = strsep(&cmd_buffer, ":");
	for (i = (CMD_UNKNOWN + 1); i < CMD_MAX; i++) {
		if (!strncmp(token, cmd_str[i], strlen(cmd_str[i]))) {
			/* Found! */
			cmd->cmd = i;
			break;
		}
	}

	switch (cmd->cmd) {
		case START_PINGER:
			if (parse_cmd_start_pinger(cmd_buffer, cmd))
				goto err;
			break;
		case STOP_PINGER:
			if (parse_cmd_stop_pinger(cmd_buffer, cmd))
				goto err;
			break;
		case START_ACKER:
			if (parse_cmd_start_acker(cmd_buffer, cmd))
				goto err;
			break;
		case STOP_ACKER:
			if (parse_cmd_stop_acker(cmd_buffer, cmd))
				goto err;
			break;
		case MSG:
			if (parse_cmd_msg(cmd_buffer, cmd))
				goto err;
			break;
		case EXIT:
			/* do nothing */
			break;
		default:
			goto err;
	}

done:
	return 0;
err:
	err("parse_cmd() error! %s\n", cmd_buffer);
	memset(cmd, 0, sizeof(struct ctrl_cmd));
	return -1;
}

static unsigned int decide_backoff_time(unsigned int base, unsigned int interval)
{
	unsigned int backoff;
	backoff = base + (random() & interval);
	dbg("backoff=%u\n", backoff);
	return backoff;
}

void handle_type_pinger_hb_connected(struct tcmap_s *tcmap_item, enum ctrl_type_e type)
{
	struct hb_ch_s *hb = tcmap_item->hb;
	unsigned int session_id = tcmap_item->session_id;
	char event_str[MAX_E_STR_BUF_SIZE] = "echo TODO";

	dbg("Type:%s(%d) timeout\n", type_str[type], type);
	/* TODO. Notify LM? */
	switch (type) {
	case VIDEO:
	case AUDIO:
	case USB:
	case IR:
	case SERIAL:
	case PUSHBUTTON:
	case CEC:
		sprintf(event_str, "%se_heartbeat_init_ok:%u", type_str[type], session_id);
		hb_ipc_set(type_str[type], event_str);
		break;
	case TYPE_UNKNOWN: /* passthrough*/
	case TYPE_MAX: /* passthrough*/
	default:
		err("TBD %d\n", __LINE__); /* TODO */
		break;
	}
}

void on_pinger_hb_connected(struct hb_ch_s *hb, unsigned int peer_session_id)
{
	msg("Pinger HB connected!\n");
	hb->is_alive = 1;
	hb->peer_session_id = peer_session_id;
	tcmap_for_each_type_matches_hb(type_pinger_hb_mapping, hb, handle_type_pinger_hb_connected);
}

int on_hb_pinger_recv(void *p1, void *p2)
{
	int nbytes;
	struct hb_ch_s *hb = (struct hb_ch_s *)p1;
	int fd = hb->hb_rx_fd;
	struct hb_pkt_s *pkt;

	//dbg("on_hb_pinger_recv\n");
	nbytes = read(fd, cmd_buffer, MAX_PAYLOAD);
	if (nbytes < 0) {
		err("hb_rx failed?! (%d:%s)\n", errno, strerror(errno));
		goto err;
	}
	pkt = (struct hb_pkt_s *)cmd_buffer;
	if (pkt->magic != HB_MAGIC_NUM) {
		goto done;
	}
	if (nbytes != calc_hb_pkt_size(pkt)) {
		dbg("Wrong hb packet size? (%d)\n", nbytes);
		goto done;
	}
	switch (pkt->type) {
		case HB_ACK:
			/* break and do normal ping-ack procedure. */
			break;
		case HB_CLOSE:
			if (hb->peer_session_id == pkt->session_id) {
				msg("Pinger Received HB_CLOSE from %u!\n", hb->peer_session_id);
				on_pinger_hb_timeout(hb);
			}
			goto done; //ignore
		case HB_MSG:
			handle_msg(hb, pkt->payload, pkt->payload_size);
			goto done;
		default:
			goto done; //ignore
	}
	/* Yes, got an valid ACK. */
	dbg("[Ch%d] Got HB Ack\n", PINGER_HB2CH(hb));
	dump_hb_pkt(pkt);

	/* Restart hb timer. */
	start_timer(&hb->hb_timer, HB_TIMEOUT_MSEC, 0);

	if (pkt->payload_size) {
		unsigned int ack_ip = *((unsigned int*)pkt->payload);
		if (ack_ip == hb->my_ip) {
			dbg("ping acked!!\n");
			hb->acked = 1;
		} else if (ack_ip == 0) {
			/* means acker want to refresh client list. */
			hb->acked = 0;
		}
	}
	if (!hb->is_alive) {
		/* First connected. Notify LM. */
		on_pinger_hb_connected(hb, pkt->session_id);
	} else if (hb->peer_session_id != pkt->session_id) {
		if (hb->casting == UNICAST) {
			/* Only unicast more cares about acker session changes!? */
			msg("Pinger Received new Acker session %u from %u!\n", pkt->session_id, hb->peer_session_id);
			on_pinger_hb_timeout(hb);
			goto done;
		}
		/* Record new peer session id. This case should only happens under VLAN switching. */
		hb->peer_session_id = pkt->session_id;
	}
	/*
	** We send a ping when:
	** - I'm not 'acked' yet.
	** - there is not much ack received. ==> means there may be no pinger is pinging.
	*/
	/* book keeping. */
	if ((++hb->ack_cnt > 3) && (hb->acked)) {
		/* acker is alive. restart backoff_timer. */
		hb->ack_cnt = 0;
		/* Restart backoff_timer. */
		start_timer(&hb->backoff_timer, decide_backoff_time(HB_BACKOFF_BASE, HB_BACKOFF_INTERVAL), 0);
		dbg("restart backoff timer\n");
	}
done:
	return 0;
err:
	return -1;
}

#if 0
static void backoff_timer_handler(struct hb_ch_s *hb)
{
	//info("mtimer is working!!\n");
}
#endif


#if defined(HB_MCAST_ACKER_IGMP_QUERIER) || defined(HB_MCAST_PINGER_IGMP_REPORT) || defined(HB_MCAST_ACKER_IGMP_REPORT)
/*
 * following codes for IGMP is copy from application/ast_app/applet/igmp/igmp.c
 */
#if defined(IGMPV2_IPOPT_RA)
#define IPOPT_SIZE (4)
#else
#define IPOPT_SIZE (0)
#endif

#define PACKET_SIZE (64 + IPOPT_SIZE)

static unsigned short in_cksum(unsigned short *addr, int len)
{
	register int sum = 0;
	u_short answer = 0;
	register u_short *w = addr;
	register int nleft = len;

	/*
	 * Our algorithm is simple, using a 32 bit accumulator (sum), we add
	 * sequential 16 bit words to it, and at the end, fold back all the
	 * carry bits from the top 16 bits into the lower 16 bits.
	 */
	while (nleft > 1) {
		sum += *w++;
		nleft -= 2;
	}
	/* mop up an odd byte, if necessary */
	if (nleft == 1) {
		*(u_char *) (&answer) = *(u_char *) w;
		sum += answer;
	}
	/* add back carry outs from top 16 bits to low 16 bits */
	sum = (sum >> 16) + (sum & 0xffff);		/* add hi 16 to low 16 */
	sum += (sum >> 16);				/* add carry */
	answer = ~sum;				/* truncate to 16 bits */

	return answer;
}

/*
** Specific query:
**    - Use multicast group ip for both IP.dst_addr and IGMP.group
** General query:
**    - Use 224.0.0.1 as IP.dst_addr
**    - Use 0.0.0.0 as IGMP.group
*/
static int igmpV2_send(int dst_addr, int src_addr, unsigned char igmp_type, unsigned char igmp_code, int igmp_group)
{
	int optval;
	struct sockaddr_in addr;
	int fd = 0;
	struct iphdr *ip = NULL;
	struct igmphdr *igmp = NULL;
	char packet[PACKET_SIZE];
	int ipopt_size = IPOPT_SIZE;

	if (getuid() != 0) {
		err("root privelidges needed\n");
		return -1;
	}

	/* create what looks like an ordinary UDP socket */
	/* Use IPPROTO_RAW, otherwise IGMP field will be overwritten by under layer. */
	fd = socket(AF_INET, SOCK_RAW, IPPROTO_RAW);

	if (fd  < 0) {
		perror("socket");
		return -1;
	}

	/*
	 * IP_HDRINCL must be set on the socket so that
	 * the kernel does not attempt to automatically add
	 * a default ip header to the packet
	 */
	//setsockopt(fd, IPPROTO_IP, IP_HDRINCL, &optval, sizeof(int));
	/* Set to IPPROTO_IGMP, otherwise packet won't be sent. ?? */
	setsockopt(fd, IPPROTO_IGMP, IP_HDRINCL, &optval, sizeof(int));

	memset(packet, 0, PACKET_SIZE);
	ip = (struct iphdr *)packet;
	igmp = (struct igmphdr *)(packet + ipopt_size + sizeof(struct iphdr));

	/* here the ip packet is set up except checksum */
	ip->ihl = (sizeof(struct iphdr) + ipopt_size) >> 2;
	ip->version = 4;
	ip->tos = 0;
	ip->tot_len = sizeof(struct iphdr) + ipopt_size + sizeof(struct igmphdr);
	ip->id = htons((uint16_t)random()); //From sniffer, it is a sequence number
	ip->frag_off = 0;
	ip->ttl = 1;
	ip->protocol = IPPROTO_IGMP;
	ip->check = 0;
	ip->saddr = src_addr;
	ip->daddr = dst_addr;
#if defined(IGMPV2_IPOPT_RA)
	/* Copy flag, 1b'1  + Class, 2b'00 + Option 5b'10100 (20) */
	((unsigned char *)&ip[1])[0] = ((0x1 << 7) | (0 << 5) | 20);
	/* length: 4 */
	((unsigned char *)&ip[1])[1] = 4;
	/* value: 0 - Router shall examine packet*/
	((unsigned char *)&ip[1])[2] = 0; ((unsigned char *)&ip[1])[3] = 0;
#endif
	/*
	 * here the igmp packet is created
	 * also the ip checksum is generated
	 */
	igmp->type = igmp_type;
	igmp->code = igmp_code; // the value depends on igmp_type
	igmp->csum = 0;
	igmp->group = igmp_group; //specified multicast group addr

	igmp->csum = in_cksum((unsigned short *)igmp, sizeof(struct igmphdr));
	ip->check = in_cksum((unsigned short *)ip, sizeof(struct iphdr) + ipopt_size);

	/* set up destination address */
	memset(&addr, 0, sizeof(struct sockaddr_in));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = ip->daddr;

	/* now the packet is sent */
	sendto(fd, packet, ip->tot_len, 0, (struct sockaddr *)&addr, sizeof(struct sockaddr));
OUT:
	if (fd >= 0)
		close(fd);

	return 0;
}

#if defined(HB_MCAST_ACKER_IGMP_QUERIER)
static void mcast_querier_timer_handler(struct hb_ch_s *hb)
{
	igmpV2_send(hb->ping_ip, hb->my_ip, 0x11, 100, hb->ping_ip);
	start_timer(&hb->mcast_querier_timer, HB_MCAST_QUERIER_PERIOD, 0);
}
#endif

#if defined(HB_MCAST_PINGER_IGMP_REPORT) || defined(HB_MCAST_ACKER_IGMP_REPORT)
static void mcast_report_timer_handler(struct hb_ch_s *hb)
{
	igmpV2_send(hb->ping_ip, hb->my_ip, 0x16, 0, hb->ping_ip);
	start_timer(&hb->mcast_report_timer, HB_MCAST_REPORT_PERIOD, 0);
}
#endif
#endif /* #if defined(HB_MCAST_ACKER_IGMP_QUERIER) || defined(HB_MCAST_PINGER_IGMP_REPORT) || defined(HB_MCAST_ACKER_IGMP_REPORT) */

void destruct_hb_pinger(struct hb_ch_s *hb)
{
	msg("Destroy Pinger HB (%u)\n", hb->my_session_id);

	/* delete created timer. */
	delete_timer(&hb->hb_timer);
	delete_timer(&hb->backoff_timer);

#if defined(HB_MCAST_PINGER_IGMP_REPORT)
	if (hb->casting == MULTICAST)
		delete_timer(&hb->mcast_report_timer);
#endif

	/* close existing sockets. */
	if (hb->hb_rx_fd > 0) {
		if (epoll_ctl(epfd, EPOLL_CTL_DEL, hb->hb_rx_fd, NULL) < 0) {
			perror("epoll_ctl failed");
		}
		close(hb->hb_rx_fd);
	}

	if (hb->hb_tx_fd > 0) {
		announce_pinger_close(hb);
		close(hb->hb_tx_fd);
	}

	memset(hb, 0, sizeof(struct hb_ch_s));
	dbg("pinger channel %d destructed\n", PINGER_HB2CH(hb));
}

int construct_hb_pinger(
	struct hb_ch_s *hb,
	unsigned int ping_ip,
	unsigned int ping_port,
	enum ctrl_casting_e casting)
{
	struct epoll_event ep_event;
	int hb_rx_fd = 0, hb_tx_fd = 0; /* We assume fd must > 0 */
	unsigned int my_ip = 0;
	unsigned int msec;

	dbg("alloc ch %d [%p] for %s\n", PINGER_HB2CH(hb), hb, inet_ntoa(ping_ip));

	/* Create UDP receive socket. */
	hb_rx_fd = udp_connect(ping_ip, ping_port);
	if (hb_rx_fd < 0)
		goto err;

	/* Create UDP transmit socket. */
	hb_tx_fd = udp_create_sender(ping_ip, ping_port, &my_ip);
	if (hb_tx_fd < 0)
		goto err;

	/* Add MCRx to epoll to wait for MCRx event. */
	ep_event.events = EPOLLIN;
	ep_event.data.ptr = reg_do_event_handler(&hb->event_handle, on_hb_pinger_recv, (void *)hb, NULL);
	if (epoll_ctl(epfd, EPOLL_CTL_ADD, hb_rx_fd, &ep_event) < 0) {
		perror("epoll_ctl failed");
		goto err;
	}

	hb->hb_rx_fd = hb_rx_fd;
	hb->hb_tx_fd = hb_tx_fd;
	hb->ping_ip = ping_ip;
	hb->ping_port = ping_port;
	hb->casting = casting;
	hb->is_alive = 0;
	hb->peer_session_id = 0;
	hb->acked = 0;
	hb->ack_cnt = 0;
	hb->my_ip = my_ip;
	hb->is_pinger = 1; /* true */
	update_hb_my_session_id(hb);
	msg("Create Pinger HB (%u)\n", hb->my_session_id);

	/* create timer. */
	create_timer(&hb->hb_timer, NULL, NULL);
	//create_timer(&hb->backoff_timer, (mtimer_handler_func)backoff_timer_handler, hb);
	create_timer(&hb->backoff_timer, NULL, NULL);

	start_timer(&hb->hb_timer, HB_TIMEOUT_MSEC, 0);
	/* Start the first backoff timer. */
	msec = decide_backoff_time(HB_BACKOFF_INIT_BASE, HB_BACKOFF_INIT_INTERVAL);
	start_timer(&hb->backoff_timer, msec, 0);

#if defined(HB_MCAST_PINGER_IGMP_REPORT)
	if (hb->casting == MULTICAST) {
		create_timer(&hb->mcast_report_timer, (mtimer_handler_func)mcast_report_timer_handler, hb);
#if defined(HB_MCAST_IGMP_REPORT_AT_HB_INIT)
		start_timer(&hb->mcast_report_timer, 1, 0);
#else
		start_timer(&hb->mcast_report_timer, random() & HB_MCAST_REPORT_PERIOD, 0);

#endif
	}
#endif
	return 0;
err:
	dbg("FATAL: construct_hb_pinger failed!?\n");

	if (hb_rx_fd > 0) {
		if (epoll_ctl(epfd, EPOLL_CTL_DEL, hb_rx_fd, NULL) < 0) {
			perror("epoll_ctl failed");
		}
		close(hb_rx_fd);
	}

	if (hb_tx_fd > 0)
		close(hb_tx_fd);

	return -1;
}

void _stop_hb_pinger(struct tcmap_s *tcmap_item, enum ctrl_type_e type)
{
	struct hb_ch_s *hb;

	hb = tcmap_item->hb;
	/* deal with existing hb. */
	if (hb) {
		/* Bruce170825. I see no need to notify LM on 'stop pinger'. Just free the HB. */
		/* free hb */
		tcmap_free_hb(hb, destruct_hb_pinger);
		dbg("pinger [%s] stopped\n", type_str[type]);
	}
	/* un-associate hb to type */
	tcmap_item_init(tcmap_item);
}

void stop_hb_pinger(struct ctrl_cmd *cmd)
{
	struct tcmap_s *tcmap_item;

	tcmap_item = tcmap_item_by_type(type_pinger_hb_mapping, cmd->type);
	if (!tcmap_item->hb) {
		dbg("pinger not exists. Ignore\n");
		return;
	}
	/* Match cmd->session_id with tcmap_item->session_id */
	if (tcmap_item->session_id != cmd->session_id) {
		err("session id mismatch?! (%u vs %u)\n", tcmap_item->session_id, cmd->session_id);
		return;
	}

	_stop_hb_pinger(tcmap_item, cmd->type);
}

void start_hb_pinger(struct ctrl_cmd *cmd)
{
	struct hb_ch_s *hb;
	struct tcmap_s *tcmap_item = tcmap_item_by_type(type_pinger_hb_mapping, cmd->type);
	unsigned int ping_ip = inet_addr(cmd->ping_ip);

	_stop_hb_pinger(tcmap_item, cmd->type);

	/* create a new or use an existing hb. */
	hb = tcmap_alloc_pinger_hb(ping_ip, cmd->ping_port, cmd->casting, construct_hb_pinger);
	/* If tcmap_alloc_pinger_hb() somehow failed, hb will be NULL. This start procedure will be silently ignored. */
	tcmap_item->hb = hb;
	tcmap_item->session_id = cmd->session_id;
	/* Assume single process. If hb is reused one, we can check is_alive here. */
	if (hb && hb->is_alive) {
		handle_type_pinger_hb_connected(tcmap_item, cmd->type);
	}
}

void handle_type_first_pinger_attached(struct tcmap_s *tcmap_item, enum ctrl_type_e type)
{
	struct hb_ch_s *hb = tcmap_item->hb;
	unsigned int session_id = tcmap_item->session_id;
	char event_str[MAX_E_STR_BUF_SIZE] = "echo TODO";

	switch (type) {
	case VIDEO:
	case AUDIO:
	case USB:
	case IR:
	case SERIAL:
	case PUSHBUTTON:
	case CEC:
		sprintf(event_str, "%se_attaching:%u:%s",
			type_str[type], session_id, inet_ntoa(hb->first_pinger_ip));
		hb_ipc_query(type_str[type], event_str);
		break;
	case TYPE_UNKNOWN: /* passthrough*/
	case TYPE_MAX: /* passthrough*/
	default:
		err("TBD %d\n", __LINE__); /* TODO */
		break;
	}
}

void on_first_pinger_attached(struct hb_ch_s *hb)
{
	msg("First pinger attached!\n");

	tcmap_for_each_type_matches_hb(type_acker_hb_mapping, hb, handle_type_first_pinger_attached);
}

struct msg_info
{
	enum ctrl_type_e type;
	unsigned int is_to_acker; /* true or false */
	unsigned int specific_ip; /* in inet format */
	unsigned char msg[MAX_PAYLOAD];
};

int parse_msg_content(char *msg, struct msg_info *pmsg)
{
	char *save_ptr;
	char *token;
	int i;
	/*
	** msg format:
	** type:to_whom:specific_ip:msg_content
	** Ex:
	** v:acker:255.255.255.255:ve_btn1_pressed
	** u:acker:255.255.255.255:ue_request_usb:$MY_IP
	** u:pinger:169.254.0.33:ue_stop_usb
	** u:pinger:169.254.0.34:ue_start_usb
	*/
	dbg("parse_msg_content:[%s]\n", msg);
	if (!msg || !strlen(msg))
		goto err;

	/* parse type */
	token = strtok_r(msg, ":", &save_ptr);
	if (!token) {
		/* something wrong */
		dbg("%d\n", __LINE__);
		goto err;
	}
	for (i = (TYPE_UNKNOWN + 1); i < TYPE_MAX; i++) {
		//dbg("%d:%s vs %s\n", __LINE__, token, type_str[i]);
		if (!strncmp(token, type_str[i], strlen(type_str[i]))) {
			/* Found! */
			pmsg->type = i;
			break;
		}
	}
	if (i == TYPE_MAX) {
		dbg("%d\n", __LINE__);
		goto err;
	}

	/* parse to_whom */
	token = strtok_r(NULL, ":", &save_ptr);
	if (!token) {
		/* something wrong */
		dbg("%d\n", __LINE__);
		goto err;
	}
	if (!strncmp(token, "acker", strlen("acker"))) {
		pmsg->is_to_acker = 1;
	} else {
		pmsg->is_to_acker = 0;
	}
	/* parse specific_ip */
	token = strtok_r(NULL, ":", &save_ptr);
	if (!token) {
		/* something wrong */
		dbg("%d\n", __LINE__);
		goto err;
	}
	/*
	** inet_addr() returns -1 if error. Which is the same as 255.255.255.255.
	** Bruce think it is fine to treat error as broadcast. If someday it is
	** not true, use inet_aton() instead.
	 */
	pmsg->specific_ip = inet_addr(token);

	/* copy the last msg item */
	strncpy(pmsg->msg, save_ptr, MAX_PAYLOAD);

done:
	return 0;
err:
	return -1;

}

void handle_msg(struct hb_ch_s *hb, char *msg, unsigned int msg_len)
{
	struct msg_info the_msg;
	struct msg_info *pmsg = &the_msg;
	struct tcmap_s *tcmap_item;
	char event_str[MAX_E_STR_BUF_SIZE] = "echo TODO";

	/*
	** Command format:
	** msg:type:to_whom:specific_ip:msg_content
	** The real delivered message is:
	** type:to_whom:specific_ip:msg_content
	*/
	if (strlen(msg) + 1 != msg_len) {
		err("invalid msg length?! (%d vs %d)\n", strlen(msg), msg_len);
		goto out;
	}
	//memset(pmsg, 0, sizeof(struct msg_info)); //I think it is waste of time.
	pmsg->type = TYPE_UNKNOWN;
	pmsg->is_to_acker = 0; /* true or false */
	pmsg->specific_ip = 0; /* in inet format */
	pmsg->msg[0] = 0;

	if (parse_msg_content(msg, pmsg))
		goto out;
	/* pmsg is valid starting from here */

	/* Match roles first. Ignore if pinger/acker not match. */
	if ((pmsg->is_to_acker && hb->is_pinger) || (!pmsg->is_to_acker && !hb->is_pinger))
		goto out;

	if (pmsg->is_to_acker) {
		tcmap_item = tcmap_item_by_type(type_acker_hb_mapping, pmsg->type);
	} else {
		tcmap_item = tcmap_item_by_type(type_pinger_hb_mapping, pmsg->type);
	}
	if (hb != tcmap_item->hb) {
		err("hb is gone?!\n");
		goto out;
	}
	/* Match IP if needed. */
	if ((pmsg->specific_ip != 0xFFFFFFFF)
		&& (pmsg->specific_ip != 0) /* FIXME: Should only applies to Acker. */
		&& (pmsg->specific_ip != hb->my_ip)) {
		dbg("specific_ip not match (%08X vs %08X)\n", pmsg->specific_ip, hb->my_ip);
		goto out;
	}
	/* Notify xLM msg. */
	switch (pmsg->type) {
	case VIDEO:
	case AUDIO:
	case USB:
	case IR:
	case SERIAL:
	case PUSHBUTTON:
	case CEC:
		sprintf(event_str, "%s", pmsg->msg);
		hb_ipc_set(type_str[pmsg->type], event_str);
		break;
	case TYPE_UNKNOWN: /* passthrough*/
	case TYPE_MAX: /* passthrough*/
	default:
		err("TBD %d\n", __LINE__); /* TODO */
		break;
	}

out:
	return;
}

int on_hb_acker_recv(void *p1, void *p2)
{
	int nbytes;
	struct hb_ch_s *hb = (struct hb_ch_s *)p1;
	int hb_rx_fd = hb->hb_rx_fd;
	int hb_tx_fd = hb->hb_tx_fd;
	struct hb_pkt_s *pkt;
	struct sockaddr_in src_addr;
	socklen_t src_addr_len = sizeof(struct sockaddr_in);

	//dbg("on_hb_acker_recv\n");
	nbytes = recvfrom(hb_rx_fd, cmd_buffer, MAX_PAYLOAD, 0, (struct sockaddr *)&src_addr, &src_addr_len);
	if (nbytes < 0) {
		err("hb_rx failed?! (%d:%s)\n", errno, strerror(errno));
		goto err;
	}
	pkt = (struct hb_pkt_s *)cmd_buffer;
	if (pkt->magic != HB_MAGIC_NUM) {
		goto done; //ignore
	}
	if (nbytes != calc_hb_pkt_size(pkt)) {
		dbg("Wrong hb packet size? (%d)\n", nbytes);
		goto done; //ignore
	}
	switch (pkt->type) {
		case HB_PING:
			/* break and do normal ping-ack procedure. */
			break;
		case HB_CLOSE:
			if (hb->casting == UNICAST) {
				if (hb->peer_session_id == pkt->session_id) {
					msg("Acker Received HB_CLOSE from %u!\n", hb->peer_session_id);
					on_acker_hb_timeout(hb);
				}
			}
			goto done; //ignore
		case HB_MSG:
			/* If need match but not matched, skip. */
			if ((pkt->match_session_id) && (pkt->match_session_id != hb->my_session_id)) {
				err("!!Not Match:%u vs %u\n", pkt->match_session_id, hb->my_session_id);
				goto done;
			}
			/* Otherwise, handle this message */
			handle_msg(hb, pkt->payload, pkt->payload_size);
			goto done;
		default:
			goto done; //ignore
	}

	/* Yes, got an valid ping. */
	dbg("Got HB ping\n");
	dump_hb_pkt(pkt);

	if (hb->casting == UNICAST) {
		/* Create hb_tx_fd for unicast mode */
		if (hb_tx_fd <= 0) {
			unsigned int my_ip = 0;

			hb_tx_fd = udp_create_sender(src_addr.sin_addr.s_addr, hb->ping_port, &my_ip);
			if (hb_tx_fd < 0) //FIXME. Ignore socket create error?
				goto done;
			hb->hb_tx_fd = hb_tx_fd;
			hb->peer_session_id = pkt->session_id;
			hb->my_ip = my_ip;
			update_hb_my_session_id(hb);
			info("new UC hb ping from:%s, %u\n", inet_ntoa(src_addr.sin_addr.s_addr), hb->peer_session_id);
		} else if (hb->first_pinger_ip != src_addr.sin_addr.s_addr) {
			/* Ping from someone else. Drop it. Unicast ack to only one pinger. */
			goto done;
		} else if (hb->peer_session_id != pkt->session_id) {
			/* From the same pinger, but different session. */
			/* destruct previous session. */
			msg("Acker Received new pinger session %u from %u!\n", pkt->session_id, hb->peer_session_id);
			on_acker_hb_timeout(hb);
			goto done;
		}
	}

	if (!hb->is_alive) {
		/* TODO. Move into on_first_pinger_attached()?! */
		hb->is_alive = 1;
		hb->first_pinger_ip = src_addr.sin_addr.s_addr;
		on_first_pinger_attached(hb);
	}
	/* TODO. How to know it is the very FIRST HB init? What happen to acker when HB timeout? */
	/* Restart heartbeat timeout timer. */
	start_timer(&hb->hb_timer, HB_TIMEOUT_MSEC, 0);
	/* TODO. Notify LM. */
	if (pkt->payload_size) {
		/* It is a ping with valid client IP info. */
		hb->last_pinger_ip = *((unsigned int*)pkt->payload);
		dbg("hb ping from:%s\n", inet_ntoa(hb->last_pinger_ip));
	}

	/* Response ack. with the same ping payload. */
	pkt->type = HB_ACK;
	pkt->session_id = hb->my_session_id;
	/* ack back. */
	nbytes = write(hb_tx_fd, pkt, calc_hb_pkt_size(pkt));
	if (nbytes < 0) {
		dbg("hb_tx failed?! (%d:%s)\n", errno, strerror(errno));
		goto done; //ignore
	}
	dbg("Sent HB Ack\n");
	dump_hb_pkt(pkt);

done:
	return 0;
err:
	return -1;
}

void destruct_hb_acker(struct hb_ch_s *hb)
{
	msg("Destroy Acker HB (%u)\n", hb->my_session_id);

	/* delete created timer. */
	delete_timer(&hb->hb_timer);

#if defined(HB_MCAST_ACKER_IGMP_REPORT)
	if (hb->casting == MULTICAST)
		delete_timer(&hb->mcast_report_timer);
#endif

#if defined(HB_MCAST_ACKER_IGMP_QUERIER)
	if (hb->casting == MULTICAST)
		delete_timer(&hb->mcast_querier_timer);
#endif

	/* close existing sockets. */
	if (hb->hb_rx_fd > 0) {
		if (epoll_ctl(epfd, EPOLL_CTL_DEL, hb->hb_rx_fd, NULL) < 0) {
			perror("epoll_ctl failed");
		}
		close(hb->hb_rx_fd);
	}

	if (hb->hb_tx_fd > 0) {
		announce_acker_close(hb);
		close(hb->hb_tx_fd);
	}

	memset(hb, 0, sizeof(struct hb_ch_s));
	dbg("acker destructed\n");
}

void stop_hb_acker(struct ctrl_cmd *cmd)
{
	struct hb_ch_s *hb;
	struct tcmap_s *tcmap_item;

	tcmap_item = tcmap_item_by_type(type_acker_hb_mapping, cmd->type);
	hb = tcmap_item->hb;
	/* deal with existing hb. */
	if (hb) {
		/* TODO. match cmd->session_id with tcmap_item->session_id */
		/* free hb */
		tcmap_free_hb(hb, destruct_hb_acker);
		dbg("acker [%s] stopped\n", type_str[cmd->type]);
		/* TODO. notify LM? */
	}
	tcmap_item_init(tcmap_item);
}

int construct_hb_acker(
	struct hb_ch_s *hb,
	unsigned int ping_ip,
	unsigned int ping_port,
	enum ctrl_casting_e casting)
{
	struct epoll_event ep_event;
	int hb_rx_fd = 0, hb_tx_fd = 0; /* We assume fd must > 0 */
	unsigned int my_ip = 0;

	/* Create UDP receive socket. */
	hb_rx_fd = udp_connect_as_server(ping_ip, ping_port);
	if (hb_rx_fd < 0)
		goto err;

	/* hb_tx_fd and my_ip is created later on ping received. */
	if (casting == MULTICAST) {
		/* Create UDP transmit socket. */
		hb_tx_fd = udp_create_sender(ping_ip, ping_port, &my_ip);
		if (hb_tx_fd < 0)
			goto err;
	}

	/* Add MCRx to epoll to wait for MCRx event. */
	ep_event.events = EPOLLIN;
	ep_event.data.ptr = reg_do_event_handler(&hb->event_handle, on_hb_acker_recv, (void *)hb, NULL);
	if (epoll_ctl(epfd, EPOLL_CTL_ADD, hb_rx_fd, &ep_event) < 0) {
		perror("epoll_ctl failed");
		goto err;
	}

	hb->hb_rx_fd = hb_rx_fd;
	hb->hb_tx_fd = hb_tx_fd;
	hb->ping_ip = ping_ip;
	hb->ping_port = ping_port;
	hb->casting = casting;
	hb->is_alive = 0;
	hb->peer_session_id = 0;
	hb->first_pinger_ip = 0;
	hb->last_pinger_ip = 0;
	hb->my_ip = my_ip;
	hb->is_pinger = 0; /* false. */
	update_hb_my_session_id(hb);
	msg("Create Acker HB (%08X:%u)\n", hb, hb->my_session_id);

	/* create timer. */
	create_timer(&hb->hb_timer, NULL, NULL);
	//start_timer(&hb->hb_timer, HB_TIMEOUT_MSEC, 0);
#if defined(HB_MCAST_ACKER_IGMP_REPORT)
	if (hb->casting == MULTICAST) {
		create_timer(&hb->mcast_report_timer, (mtimer_handler_func)mcast_report_timer_handler, hb);
#if defined(HB_MCAST_IGMP_REPORT_AT_HB_INIT)
		start_timer(&hb->mcast_report_timer, 1, 0);
#else
		start_timer(&hb->mcast_report_timer, random() & HB_MCAST_REPORT_PERIOD, 0);
#endif
	}
#endif

#if defined(HB_MCAST_ACKER_IGMP_QUERIER)
	if (hb->casting == MULTICAST) {
		create_timer(&hb->mcast_querier_timer, (mtimer_handler_func)mcast_querier_timer_handler, hb);
		start_timer(&hb->mcast_querier_timer, random() & HB_MCAST_QUERIER_PERIOD, 0);
	}
#endif

	return 0;
err:
	if (hb_rx_fd > 0) {
		if (epoll_ctl(epfd, EPOLL_CTL_DEL, hb_rx_fd, NULL) < 0) {
			perror("epoll_ctl failed");
		}
		close(hb_rx_fd);
	}
	if (hb_tx_fd > 0)
		close(hb_tx_fd);

	return -1;
}

void start_hb_acker(struct ctrl_cmd *cmd)
{
	struct hb_ch_s *hb;
	struct tcmap_s *tcmap_item = tcmap_item_by_type(type_acker_hb_mapping, cmd->type);
	unsigned int ping_ip = inet_addr(cmd->ping_ip);

	stop_hb_acker(cmd);

	/* create a new or use an existing hb. */
	hb = tcmap_alloc_acker_hb(ping_ip, cmd->ping_port, cmd->casting, construct_hb_acker);
	/* If tcmap_alloc_acker_hb() somehow failed, hb will be NULL. This start procedure will be silently ignored. */
	tcmap_item->hb = hb;
	tcmap_item->session_id = cmd->session_id;
	/* Assume single process. If hb is reused one, we can check is_alive here. */
	if (hb && hb->is_alive) {
		handle_type_first_pinger_attached(tcmap_item, cmd->type);
	}
}

void send_msg_to(struct hb_ch_s *hb, char *msg, unsigned int match_session_id)
{
	int nbytes;
	int hb_tx_fd;
	struct hb_pkt_s *hb_pkt = (struct hb_pkt_s *)cmd_buffer;
	unsigned int msg_len;

	dbg("[Ch%d] send msg\n", ACKER_HB2CH(hb));

	hb_tx_fd = hb->hb_tx_fd;
	if (!hb_tx_fd)
		return;

	msg_len = strlen(msg) + 1;
	if (msg_len > (MAX_PAYLOAD - sizeof(struct hb_pkt_s))) {
		err("msg too long %d\n", msg_len);
		return;
	}

	/* Send MC ping */
	hb_pkt->magic = HB_MAGIC_NUM;
	hb_pkt->type = HB_MSG;
	hb_pkt->session_id = hb->my_session_id;
	hb_pkt->match_session_id = match_session_id;
	hb_pkt->payload_size = msg_len;
	strncpy(hb_pkt->payload, msg, msg_len);
	nbytes = write(hb_tx_fd, hb_pkt, calc_hb_pkt_size(hb_pkt));
	if (nbytes < 0) {
		if (errno == ECONNREFUSED) {
			/* announce_pinger_close() is called during destruct_hb_pinger(). DO NOT call on_pinger_hb_timeout() again. */
			/* "111::Connection refused" fail indicate link off right away. */
			//on_pinger_hb_timeout(hb);
			/* hb will be invalid after on_pinger_hb_timeout(). */
			return;
		}
		err("send_msg_to() write failed?! (%d:%s)", errno, strerror(errno));
	}
}

void send_msg(struct ctrl_cmd *cmd)
{
	if (cmd->msg_to_acker) {
		/* Send to acker */
		struct tcmap_s *tcmap_item = tcmap_item_by_type(type_pinger_hb_mapping, cmd->type);
		struct hb_ch_s *hb = tcmap_item->hb;

		if (!hb) {
			err("pinger for type (%s) is not available?!\n", type_str[cmd->type]);
			/* ignore */
			return;
		}
		if (!hb->is_alive) {
			err("pinger for type (%s) is not alive?!\n", type_str[cmd->type]);
			/* ignore? */
			return;
		}
		if (cmd->msg_to_acker == 2) {
			/* Want to match session_id. */
			send_msg_to(hb, cmd->msg, hb->peer_session_id);
		} else {
			send_msg_to(hb, cmd->msg, 0);
		}
	} else {
		/* Send to pinger */
		struct tcmap_s *tcmap_item = tcmap_item_by_type(type_acker_hb_mapping, cmd->type);
		struct hb_ch_s *hb = tcmap_item->hb;

		if (!hb) {
			err("acker for type (%s) is not available?!\n", type_str[cmd->type]);
			/* ignore */
			return;
		}
		if (!hb->is_alive) {
			err("acker for type (%s) is not alive?!\n", type_str[cmd->type]);
			/* ignore? */
			return;
		}
		send_msg_to(hb, cmd->msg, 0);
	}
}

void handle_ctrl_cmd(int cmd_fd, unsigned char *cmd_buffer)
{
	int nbytes;
	struct ctrl_cmd cmd;

	/* do something here. */
	msg("cmd:%s\n", cmd_buffer);
	if (parse_cmd(cmd_buffer, &cmd))
		return;

	switch (cmd.cmd) {
		case START_PINGER:
			start_hb_pinger(&cmd);
			break;
		case STOP_PINGER:
			stop_hb_pinger(&cmd);
			break;
		case START_ACKER:
			start_hb_acker(&cmd);
			break;
		case STOP_ACKER:
			stop_hb_acker(&cmd);
			break;
		case MSG:
			send_msg(&cmd);
			break;
		case EXIT:
			kill_me = 1;
			break;
		default:
			err("Unknown cmd?! [%s]\n", cmd_buffer);
			break;
	}

	return;
}

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
	handle_ctrl_cmd(cmd_fd, cmd_buffer);

#if 0 /* No need query here. Just use set. */
	/* TODO. ack what? */
	nbytes = write(cmd_fd, cmd_buffer, strlen(cmd_buffer));
	if (nbytes < 0) {
		err("write() failed [%d:%s]\n", errno, strerror(errno));
		return;
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

void announce_acker_close(struct hb_ch_s *hb)
{
	int nbytes;
	int hb_tx_fd;
	struct hb_pkt_s *hb_pkt = (struct hb_pkt_s *)cmd_buffer;

	dbg("[Ch%d] announce close\n", ACKER_HB2CH(hb));

	hb_tx_fd = hb->hb_tx_fd;
	if (!hb_tx_fd)
		return;

	/* Send MC ping */
	hb_pkt->magic = HB_MAGIC_NUM;
	hb_pkt->type = HB_CLOSE;
	hb_pkt->session_id = hb->my_session_id;
	hb_pkt->payload_size = 0;
	nbytes = write(hb_tx_fd, hb_pkt, calc_hb_pkt_size(hb_pkt));
	if (nbytes < 0) {
		if (errno == ECONNREFUSED) {
			/* announce_pinger_close() is called during destruct_hb_pinger(). DO NOT call on_pinger_hb_timeout() again. */
			/* "111::Connection refused" fail indicate link off right away. */
			//on_pinger_hb_timeout(hb);
			/* hb will be invalid after on_pinger_hb_timeout(). */
			return;
		}
		err("announce_acker_close write failed?! (%d:%s)", errno, strerror(errno));
	}
}

void announce_pinger_close(struct hb_ch_s *hb)
{
	int nbytes;
	int hb_tx_fd;
	struct hb_pkt_s *hb_pkt = (struct hb_pkt_s *)cmd_buffer;

	dbg("[Ch%d] announce close\n", PINGER_HB2CH(hb));

	hb_tx_fd = hb->hb_tx_fd;
	if (!hb_tx_fd)
		return;

	/* Send MC ping */
	hb_pkt->magic = HB_MAGIC_NUM;
	hb_pkt->type = HB_CLOSE;
	hb_pkt->session_id = hb->my_session_id;
	if (hb->acked) {
		hb_pkt->payload_size = 0;
	} else {
		hb_pkt->payload_size = sizeof(unsigned int);
		*((unsigned int *)hb_pkt->payload) = hb->my_ip;
	}
	nbytes = write(hb_tx_fd, hb_pkt, calc_hb_pkt_size(hb_pkt));
	if (nbytes < 0) {
		if (errno == ECONNREFUSED) {
			/* announce_pinger_close() is called during destruct_hb_pinger(). DO NOT call on_pinger_hb_timeout() again. */
			/* "111::Connection refused" fail indicate link off right away. */
			//on_pinger_hb_timeout(hb);
			/* hb will be invalid after on_pinger_hb_timeout(). */
			return;
		}
		err("announce_pinger_close write failed?! (%d:%s)", errno, strerror(errno));
	}
}

void on_backoff_timeout(struct hb_ch_s *hb)
{
	int msec, nbytes;
	int hb_tx_fd;
	struct hb_pkt_s *hb_pkt = (struct hb_pkt_s *)cmd_buffer;

	dbg("[Ch%d] on_backoff_timeout\n", PINGER_HB2CH(hb));

	hb_tx_fd = hb->hb_tx_fd;

	/* Send MC ping */
	hb_pkt->magic = HB_MAGIC_NUM;
	hb_pkt->type = HB_PING;
	hb_pkt->session_id = hb->my_session_id;
	if (hb->acked) {
		hb_pkt->payload_size = 0;
	} else {
		hb_pkt->payload_size = sizeof(unsigned int);
		*((unsigned int *)hb_pkt->payload) = hb->my_ip;
	}
	nbytes = write(hb_tx_fd, hb_pkt, calc_hb_pkt_size(hb_pkt));
	if (nbytes < 0) {
		if (errno == ECONNREFUSED) {
			/* "111::Connection refused" fail indicate link off right away. */
			msg("Acker doesn't exist. (%u)\n", hb->my_session_id);
			/*
			** Bruce170316. Ignore this error. HB timeout will handle timeout case.
			** Hope we can trigger less xLM events.
			*/
			goto ignore;

			on_pinger_hb_timeout(hb);
			/* hb will be invalid after on_pinger_hb_timeout(). */
			return;
		}
		err("on_backoff_timeout write failed?! (%d:%s)\n", errno, strerror(errno));
	}
ignore:
	/* reset ack_cnt between backoff interval. */
	hb->ack_cnt = 0;
	dbg("[Ch%d] Sent HB ping\n", PINGER_HB2CH(hb));
	dump_hb_pkt(hb_pkt);

	/* Fire next backoff timer. */
	msec = decide_backoff_time(HB_BACKOFF_BASE, HB_BACKOFF_INTERVAL);
	start_timer(&hb->backoff_timer, msec, 0);
}

void handle_type_pinger_hb_timeout(struct tcmap_s *tcmap_item, enum ctrl_type_e type)
{
	struct hb_ch_s *hb = tcmap_item->hb;
	unsigned int session_id = tcmap_item->session_id;
	char event_str[MAX_E_STR_BUF_SIZE] = "echo TODO";

	dbg("Type:%s(%d) timeout\n", type_str[type], type);
	/* un-associate hb to type */
	tcmap_item_init(tcmap_item);
	/* free hb */
	tcmap_free_hb(hb, destruct_hb_pinger);

	/* Notify LM */
	switch (type) {
	case VIDEO:
	case AUDIO:
	case USB:
	case IR:
	case SERIAL:
	case PUSHBUTTON:
	case CEC:
		sprintf(event_str, "%se_no_heartbeat:%u", type_str[type], session_id);
		hb_ipc_set(type_str[type], event_str);
		break;
	case TYPE_UNKNOWN: /* passthrough*/
	case TYPE_MAX: /* passthrough*/
	default:
		err("TBD %d\n", __LINE__); /* TODO */
		break;
	}
}

void on_pinger_hb_timeout(struct hb_ch_s *hb)
{
	//msg("Pinger HB timeout!\n");
	hb->hb_timer.fired = 0;
	hb->is_alive = 0;
	/* calling handle_type_hb_timeout() for each matched type. */
	tcmap_for_each_type_matches_hb(type_pinger_hb_mapping, hb, handle_type_pinger_hb_timeout);
}

void handle_type_acker_hb_timeout(struct tcmap_s *tcmap_item, enum ctrl_type_e type)
{
	struct hb_ch_s *hb = tcmap_item->hb;
	unsigned int session_id = tcmap_item->session_id;
	char event_str[MAX_E_STR_BUF_SIZE] = "echo TODO";

	dbg("Type:%s(%d) timeout\n", type_str[type], type);
	/* TODO. then what? stop backoff timer? stop pinger? */
	/* un-associate hb to type */
	tcmap_item_init(tcmap_item);
	/* free hb */
	tcmap_free_hb(hb, destruct_hb_acker);

	/* TODO. Notify LM? */
	switch (type) {
	case VIDEO:
	case AUDIO:
	case USB:
	case IR:
	case SERIAL:
	case PUSHBUTTON:
	case CEC:
		sprintf(event_str, "%se_no_heartbeat:%u", type_str[type], session_id);
		hb_ipc_set(type_str[type], event_str);
		break;
	case TYPE_UNKNOWN: /* passthrough*/
	case TYPE_MAX: /* passthrough*/
	default:
		err("TBD %d\n", __LINE__); /* TODO */
		break;
	}
}

void on_acker_hb_timeout(struct hb_ch_s *hb)
{
	//msg("Acker HB timeout!\n");
	hb->hb_timer.fired = 0;

#if 1
	/*
	** Bruce170405. Let LM decide how to handle no_heartbeat.
	** Take multicast USB and Video into consideration.
	** I can't find a better/simpler way to handle all xLM.
	** I'm going to notify no_heartbeat in both UC and MC case.
	** Maybe in the future, don't kill hb here, just notify and let
	** xLM decide how to handle this event.
	*/
	/* calling handle_type_hb_timeout() for each matched type. */
	tcmap_for_each_type_matches_hb(type_acker_hb_mapping, hb, handle_type_acker_hb_timeout);
#else
	/*
	** UC:
	** - close acker's hb_tx_fd.
	** - free everything.
	** - notify xLM no_heartbeat.
	** MC: Means all pingers leaves for a while.
	** - is_alive clear to 0. (wait for 'next' first pinger?)
	** - ??
	*/
	if (hb->casting == UNICAST) {
		/* calling handle_type_hb_timeout() for each matched type. */
		tcmap_for_each_type_matches_hb(type_acker_hb_mapping, hb, handle_type_acker_hb_timeout);
	} else {
		hb->is_alive = 0;
		hb->peer_session_id = 0;
		hb->first_pinger_ip = 0;
		hb->last_pinger_ip = 0;
	}
#endif
}

int on_timer_event(void)
{
	int i;
	struct hb_ch_s *hb;
	int fired_num = 0;

	//FIXME. Service timer in order. How?
	for (i = 0, hb = hb_pinger_list; i < MAX_PINGER_HB_CH; i++, hb++) {
		if (!hb->ref_cnt)
			continue;
		/* check heartbeat timeout. */
		if (hb->hb_timer.fired) {
			msg("Pinger HB timeout (%u)\n", hb->my_session_id);
			on_pinger_hb_timeout(hb);
			fired_num++;
		}
		/* Check backoff timer. */
		if (hb->backoff_timer.fired) {
			on_backoff_timeout(hb);
			fired_num++;
		}
	}
	/* check hb_acker. */
	for (i = 0, hb = hb_acker_list; i < MAX_ACKER_HB_CH; i++, hb++) {
		if (!hb->ref_cnt)
			continue;
		/* check heartbeat timeout. */
		if (hb->hb_timer.fired) {
			msg("Acker HB timeout (%u)\n", hb->my_session_id);
			on_acker_hb_timeout(hb);
			fired_num++;
		}
	}
	return fired_num;
}

void handle_pinger_destroy(struct tcmap_s *tcmap_item, enum ctrl_type_e type)
{
	struct hb_ch_s *hb = tcmap_item->hb;

	dbg("Destroy Type:%s(%d)\n", type_str[type], type);
	/* free hb */
	if (hb)
		tcmap_free_hb(hb, destruct_hb_pinger);
	/* un-associate hb to type */
	tcmap_item_init(tcmap_item);
}

void handle_acker_destroy(struct tcmap_s *tcmap_item, enum ctrl_type_e type)
{
	struct hb_ch_s *hb = tcmap_item->hb;

	dbg("Destroy Type:%s(%d)\n", type_str[type], type);
	/* free hb */
	if (hb)
		tcmap_free_hb(hb, destruct_hb_acker);
	/* un-associate hb to type */
	tcmap_item_init(tcmap_item);
}

void destroy_all(void)
{
	/* destroy pinger */
	tcmap_for_each_type(type_pinger_hb_mapping, handle_pinger_destroy);
	/* destroy acker. */
	tcmap_for_each_type(type_acker_hb_mapping, handle_acker_destroy);
}

int main(void)
{
	int ctrl_fd = -1;
	int ne;
	struct epoll_event ep_event;
	struct do_event_s *e;
	sigset_t sigmask, ori_sigmask;
	static struct do_event_s ctrl_event_handle;

	/* init. */
	srandom(get_mac_addr());
	memset(&hb_pinger_list, 0, sizeof(struct hb_ch_s) * MAX_PINGER_HB_CH);
	memset(&hb_acker_list, 0, sizeof(struct hb_ch_s) * MAX_ACKER_HB_CH);
	tcmap_list_init(type_pinger_hb_mapping, MAX_TCMAP);
	tcmap_list_init(type_acker_hb_mapping, MAX_TCMAP);
	hb_session_id = (unsigned int)random();

	ctrl_fd = create_uds_server("@hb_ctrl");
	if (ctrl_fd < 0) {
		goto done;
	}
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
	/* Get current sigmask by set NULL mask. */
	//sigprocmask(SIG_SETMASK, NULL, &ori_sigmask);
	sigfillset(&sigmask);
	/* block all signals. */
	sigprocmask(SIG_SETMASK, &sigmask, &ori_sigmask);
	sigemptyset(&sigmask);
	sigaddset(&sigmask, SIGRTMIN);
	for (; !kill_me;) {
		/* unblock timer signal. */
		sigprocmask(SIG_UNBLOCK, &sigmask, NULL);
		ne = epoll_wait(epfd, &ep_event, 1, HB_BACKOFF_INTERVAL);
		/* block timer signal. DO NOT accept interrupt in epoll handler. */
		sigprocmask(SIG_BLOCK, &sigmask, NULL);
		if (ne < 0) {
			/* something wrong. */
			if (errno == EINTR) {
				/* interrupted by signal */
				//dbg("time is up!!\n");
				on_timer_event();
			} else {
				perror("epoll_wait failed?!");
			}
			continue;
		}
		/* case for epoll timeout. */
		if (ne == 0) {
			/* in case timer event was missed. */
			if (on_timer_event()) {
				dbg("timer event missed?!\n");
			}
			continue;
		}
		/* Handle fd events. */
		e = (struct do_event_s *)ep_event.data.ptr;
		//dbg("e:%p\n", e);
		(e->handler)(e->p1, e->p2);
	}

done:
	/* Bruce see no need to restore sigmask. */
	//sigprocmask(SIG_SETMASK, &ori_sigmask, NULL);
	//sigprocmask(SIG_UNBLOCK, &sigmask, NULL);
	destroy_all();
	if (ctrl_fd > 0)
		close(ctrl_fd);
	if (epfd > 0)
		close(epfd);

	dbg("heartbeat exit\n");
	/*
	** Bruce. FIXME.
	** Don't know why the process can't stop when mtimer is created.
	** Send a SIGKILL signal to itself can resolve this problem.
	** But I don't know the side effect.
	*/
	raise(SIGKILL);
	return 0;
}


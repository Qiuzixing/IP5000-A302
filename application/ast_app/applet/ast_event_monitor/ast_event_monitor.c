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
#include <string.h>
#include <errno.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>

#include <linux/types.h>
#include <linux/netlink.h>

#include <sched.h>

#include "ast_event_monitor.h"

#define msg(fmt, args...) \
	do { \
		fprintf(stderr, fmt, ##args); \
	} while(0)

#define die(fmt, args...) \
	do { \
		msg(fmt, ##args); \
		goto err; \
	} while (0)

#if 0
void die(char *s)
{
	write(2,s,strlen(s));
	exit(1);
}
#endif

/* Bruce160226. Add ring buffer log to trace events. */
#define ELOG_ENTRY_SIZE 16 //0 ~ 0xF
#define ELOG_ITEM_STR_SIZE 128
char event_log[ELOG_ENTRY_SIZE][ELOG_ITEM_STR_SIZE];
unsigned int elog_idx = 0; //The empty entry index to be added

static inline void elog_init(void)
{
	memset(event_log, 0, sizeof(event_log));
}

static elog_add(char *str)
{
	strncpy(&event_log[elog_idx][0], str, ELOG_ITEM_STR_SIZE);
	elog_idx = (elog_idx + 1) & 0xF;
}

static elog_dump(void)
{
	unsigned int i;
	unsigned int idx = elog_idx;

	msg("Event trace dump:\n");
	for (i = 0; i < ELOG_ENTRY_SIZE; i++) {
		idx = (idx - 1) & 0xF;
		msg("EVENT[%d]:%s\n", idx, &event_log[idx][0]);
	}
}

#ifdef LEGACY_PIPE_METHOD
#include <sys/poll.h>

#define PIPE "/var/event_pipe"
#define PIPE_ACK "/var/event_pipe_ack"

void write_to_event_pipe(unsigned char *event_msg)
{
	FILE *fp = NULL, *fp_ack = NULL;
	int r;
	char ack[MAX_PAYLOAD];

	/*
	** About the named PIPE:
	** When opening a FIFO with O_RDONLY or O_WRONLY set: If O_NONBLOCK is
	** set:
	**
	**	An open() for reading only will return without delay. An open()
	**	for writing only will return an error if no process currently
	**	has the file open for reading.
	**
	**	If O_NONBLOCK is clear:
	**	An open() for reading only will block the calling thread until a
	**	thread opens the file for writing. An open() for writing only
	**	will block the calling thread until a thread opens the file for
	**	reading.
	*/
	fp = fopen(PIPE, "wb");
	if (fp == NULL) {
		msg("ERROR! can't open event_pipe?!\n");
		goto out;
	}
	sched_yield();
	elog_add(event_msg);
	r = fwrite(event_msg, strlen(event_msg), 1, fp);
	if (r < 1) {
		msg("Failed to write to pipe! %d\n", r);
	}
	//msg("E::(%s)(%d)\n", event_msg, strlen(event_msg));
	fflush(fp);
	sched_yield();
	fclose(fp);
	fp = NULL;

	/*
	** There is an issue where the event_msg written into pipe was disappear.
	** It is very difficult to reproduce and identify this issue.
	** I guess it is a bug of busybox (cat or shell). 
	** To resolve this issue, another "ack pipe" is added. 
	** This method forces the pipe to be written one at a time and it resolve the issue.
	*/
	fp_ack = fopen(PIPE_ACK, "rb");
	if (fp_ack == NULL) {
		msg("ERROR! can't open event_pipe?!\n");
		goto out;
	}
	sched_yield();
	r = fread(ack, MAX_PAYLOAD, 1, fp_ack);
	if (r < 1 && ferror(fp_ack)) {
		msg("Failed to read from pipe! %d\n", r);
		elog_dump();
	}
	sched_yield();
	if (strncmp(ack, event_msg, (strlen(event_msg)-1))) {
		msg("Compare error?! (%s):(%s)\n", event_msg, ack);
		elog_dump();
	}

out:
	if (fp)
		fclose(fp);
	if (fp_ack)
		fclose(fp_ack);
}

int main(int argc, char *argv[])
{
	struct sockaddr_nl src_addr, dest_addr;
	struct nlmsghdr *nlh = NULL;
	struct iovec iov;
	struct msghdr msg;
	int len;
	struct pollfd pfd;
	unsigned char *event_msg;
	unsigned int not_tick = 1; // true: is not watchdog tick event. false: else.

	/* Bruce160310.
	** Ignore PIPE write 'broken pipe' signal to avoid ast_send_event terminate unexpectly.
	*/
	signal(SIGPIPE, SIG_IGN);
	elog_init();
	memset(&pfd, 0, sizeof(pfd));
	pfd.events = POLLIN;
	pfd.fd = socket(PF_NETLINK, SOCK_RAW, NETLINK_USERSOCK);
	if (pfd.fd == -1)
		die("Not root\n");

	memset(&src_addr, 0, sizeof(src_addr));
	src_addr.nl_family = AF_NETLINK;
	src_addr.nl_pid = getpid();  /* self pid */
	/* interested in group 1<<0 */
	src_addr.nl_groups = -1;
	if (bind(pfd.fd, (struct sockaddr*)&src_addr, sizeof(src_addr)))
		die("bind failed\n");

	memset(&dest_addr, 0, sizeof(dest_addr));

	nlh = (struct nlmsghdr *)malloc(NLMSG_SPACE(MAX_PAYLOAD));
	if (!nlh)
		die("failed to allocate nlh\n");
	memset(nlh, 0, NLMSG_SPACE(MAX_PAYLOAD));

	memset(&iov, 0, sizeof(iov));
	memset(&msg, 0, sizeof(msg));
	iov.iov_base = (void *)nlh;
	iov.iov_len = NLMSG_SPACE(MAX_PAYLOAD);
	msg.msg_name = (void *)&dest_addr;
	msg.msg_namelen = sizeof(dest_addr);
	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;

	msg("Waiting for event message\n");
	while (-1 != poll(&pfd, 1, -1)) {
		int i, len;
		unsigned char *buf = (unsigned char *)nlh;

		if (not_tick)
			msg("---------------------------------------\n");

		sched_yield();
		/* Read message from kernel */
		len = recvmsg(pfd.fd, &msg, 0);
		sched_yield();
		if (len <= 0) {
			/*
			** Bruce160304.
			** Test: Fire 'e msg_toFFFFFFFF_e_pwr_status_chg_0' repeatly and very fast on host.
			** Problem: client's recvmsg() will return '-1' for unknown reason. And exit(). Which stopped the LM system.
			** Solution: Ignore this error seems fine. Not sure if there is any event lost?!
			*/
			msg("EVENT: recvmsg failed?! (%d)[%d:%s]\n", len, errno, strerror(errno));
			continue;
		}
#if 1
		if (!NLMSG_OK(nlh,len)) {
			msg("invalid nlh?!\n");
			continue;
		}
#endif
		// Print the data to stdout.
#if 0
		i = 0;
		while (i < len) {
			printf("%s\n", buf+i);
			i += strlen(buf+i) + 1;
		}
#endif
		not_tick = strncmp(NLMSG_DATA(nlh), "tick", 4);
		if (not_tick)
			msg("Got (%s) event\n", NLMSG_DATA(nlh));

		//printf("%s", NLMSG_DATA(nlh));
		//Add an extra space after the event_msg
		event_msg = NLMSG_DATA(nlh);
		strcat(event_msg, "\n");
		write_to_event_pipe(event_msg);
	}
	
err:
	if (pfd.fd != -1)
		close(pfd.fd);
	if (nlh)
		free(nlh);
	return 0;
}
#else /* #ifdef LEGACY_PIPE_METHOD */
#include <sys/un.h>

static int recv_event(int event_listener, struct nlmsghdr *nlh)
{
	struct iovec iov;
	struct msghdr msghdr;
	int len = 0;

	memset(&iov, 0, sizeof(iov));
	memset(&msghdr, 0, sizeof(msghdr));
	iov.iov_base = (void *)nlh;
	iov.iov_len = NLMSG_SPACE(MAX_PAYLOAD);

#if 0 /* From man page, msg_name is optional. */
	struct sockaddr_nl dest_addr;
	memset(&dest_addr, 0, sizeof(dest_addr));
	msghdr.msg_name = (void *)&dest_addr;
	msghdr.msg_namelen = sizeof(dest_addr);
#endif

	msghdr.msg_iov = &iov;
	msghdr.msg_iovlen = 1;

	len = recvmsg(event_listener, &msghdr, 0);
	if (len <= 0) {
		/*
		** Bruce160304.
		** Test: Fire 'e msg_toFFFFFFFF_e_pwr_status_chg_0' repeatly and very fast on host.
		** Problem: client's recvmsg() will return '-1' for unknown reason. And exit(). Which stopped the LM system.
		** Solution: Ignore this error seems fine. Not sure if there is any event lost?!
		*/
		msg("EVENT: recvmsg failed?! (%d)[%d:%s]\n", len, errno, strerror(errno));
		len = -1;
		goto done;
	}
	if (!NLMSG_OK(nlh, len)) {
		msg("invalid nlh?!\n");
		len = -1;
		goto done;
	}

done:
	return len;
}

static int create_event_listener(void)
{
	struct sockaddr_nl src_addr;
	int event_listener = 0;
	
	event_listener = socket(PF_NETLINK, SOCK_RAW, NETLINK_USERSOCK);
	if (event_listener == -1) {
		msg("Not root\n");
		goto done;
	}

	memset(&src_addr, 0, sizeof(src_addr));
	src_addr.nl_family = AF_NETLINK;
	src_addr.nl_pid = getpid();  /* self pid */
	/* interested in group 1<<0 */
	src_addr.nl_groups = -1;
	if (bind(event_listener, (struct sockaddr*)&src_addr, sizeof(src_addr))) {
		msg("event listener bind failed\n");
		goto done;
	}

	return event_listener;
done:
	if (event_listener && (event_listener != -1))
		close(event_listener);
	return -1;
}

int create_uds(void)
{
	int socket_fd, connection_fd;
	socklen_t address_length;
	struct sockaddr_un address;

	/* Use SOCK_SEQPACKET. Stream type socket with data boundary. */
	socket_fd = socket(PF_UNIX, SOCK_SEQPACKET, 0);
	if (socket_fd < 0)
		die("uds socket() failed\n");

	/* start with a clean address structure */
	memset(&address, 0, sizeof(struct sockaddr_un));

	address.sun_family = AF_UNIX;
	snprintf(address.sun_path, UNIX_PATH_MAX, UDS_PATH_NAME_LM_EVENT);
	address.sun_path[0] = 0;

	if (bind(socket_fd, (struct sockaddr *)&address, sizeof(struct sockaddr_un)) != 0) {
		die("uds bind() failed\n");
	}

	/* 5 means accept up to 5 connection request in queue. */
	if (listen(socket_fd, 5) != 0) {
		die("uds listen() failed\n");
	}

	return socket_fd;
err:
	return -1;
}

int main(int argc, char *argv[])
{
	struct sockaddr_nl src_addr, dest_addr;
	struct nlmsghdr *nlh = NULL;
	unsigned char nlbuf[NLMSG_SPACE(MAX_PAYLOAD)];
	struct iovec iov;
	struct msghdr msg;
	int len, netlink_fd = -1, uds_listen_fd = -1, lm_event_fd = -1;
	unsigned char *event_msg;
	unsigned int not_tick = 1; // true: is not watchdog tick event. false: else.

	/* Bruce160310.
	** Ignore PIPE write 'broken pipe' signal to avoid ast_send_event terminate unexpectly.
	*/
	signal(SIGPIPE, SIG_IGN);
	elog_init();
	nlh = (struct nlmsghdr *)nlbuf;
	memset(nlh, 0, NLMSG_SPACE(MAX_PAYLOAD));

	uds_listen_fd = create_uds();
	if (uds_listen_fd == -1)
		die("create_uds failed\n");

	netlink_fd = create_event_listener();
	if (netlink_fd == -1)
		die("prepare_netlink failed\n");

	msg("Waiting for event message\n");
	/* Start accepting LM ast_get_event UDS. */
	while ((lm_event_fd = accept(uds_listen_fd, NULL, NULL)) > -1) {
		if (not_tick)
			msg("---------------------------------------\n");

		/* Read message from kernel. Just retry if something goes wrong. */
		for (;;) {
			len = recv_event(netlink_fd, nlh);
			sched_yield();
			if (len < 0)
				continue;

			/* Got netlink message. */
			event_msg = NLMSG_DATA(nlh);
			/* Make sure event_msg is null ended. */
			event_msg[len] = 0;
			//printf("%s", NLMSG_DATA(nlh));
			//qzx20210728:ignore audio_detect msg,it will handle in audio_detect process
			if(0 == strncmp("e_audio_detect_pressed",event_msg,sizeof("e_audio_detect_pressed")) || 0 == strncmp("e_audio_detect_released",event_msg,sizeof("e_audio_detect_released")))
			{
				continue;
			}
			else
			{
				break;
			}
		}
		not_tick = strncmp(event_msg, "tick", 4);
		if (not_tick)
			msg("Got (%s) event\n", event_msg);

		elog_add(event_msg);
		/* Tell UDS, lm_event_fd. */
		len = write(lm_event_fd, event_msg, strlen(event_msg));
		if (len < 0) {
			msg("Write to lm_event_fd failed!? (%d)[%d:%s]\n", len, errno, strerror(errno));
			elog_dump();
		}
		close(lm_event_fd);
	}

err:
	if (uds_listen_fd != -1)
		close(uds_listen_fd);
	if (netlink_fd != -1)
		close(netlink_fd);
	if (nlh)
		free(nlh);
	return 0;
}
#endif /* #ifdef LEGACY_PIPE_METHOD */


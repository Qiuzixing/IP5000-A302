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

#include <sys/poll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <linux/types.h>
#include <linux/netlink.h>


#define MAX_PAYLOAD 1024  /* maximum payload size*/

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

void send_event(unsigned int dest_pid, char *event_msg)
{
	struct msghdr msg_hdr;

	struct sockaddr_nl src_addr, dest_addr;
	struct nlmsghdr *nlh = NULL;
	struct iovec iov;
	int sock_fd;

	sock_fd = socket(PF_NETLINK, SOCK_RAW, NETLINK_USERSOCK);
	if (sock_fd == -1)
		die("socket()) failed\n");

	memset(&src_addr, 0, sizeof(src_addr));
	sched_yield();
	src_addr.nl_family = AF_NETLINK;
	src_addr.nl_pid = getpid();  /* self pid */
	/* interested in group 1<<0 */
	src_addr.nl_groups = -1;
	if (bind(sock_fd, (struct sockaddr*)&src_addr, sizeof(src_addr)))
		die("bind failed\n");

	memset(&dest_addr, 0, sizeof(dest_addr));
	sched_yield();
	dest_addr.nl_family = AF_NETLINK;
	dest_addr.nl_pid = dest_pid;   /* 0 for Linux Kernel */
	dest_addr.nl_groups = 1; /* multicast */

	nlh=(struct nlmsghdr *)malloc(NLMSG_SPACE(MAX_PAYLOAD));
	if (!nlh)
		die("failed to allocate nlh\n");

	/* Fill the netlink message header */
	memset(nlh, 0, sizeof(struct nlmsghdr));
	sched_yield();
	nlh->nlmsg_len = NLMSG_SPACE(MAX_PAYLOAD);
	nlh->nlmsg_pid = src_addr.nl_pid;  /* self pid */
	nlh->nlmsg_flags = 0;
	/* Fill in the netlink message payload */
	//!! REMEMBER to ADD ONE SPACE after the event_msg
	sprintf(NLMSG_DATA(nlh), "%s", event_msg);
	//strcpy(NLMSG_DATA(nlh), event_msg);

	memset(&iov, 0, sizeof(struct iovec));
	memset(&msg_hdr, 0, sizeof(struct msghdr));
	iov.iov_base = (void *)nlh;
	iov.iov_len = nlh->nlmsg_len;
	msg_hdr.msg_name = (void *)&dest_addr;
	msg_hdr.msg_namelen = sizeof(dest_addr);
	msg_hdr.msg_iov = &iov;
	msg_hdr.msg_iovlen = 1;

	sched_yield();
	sendmsg(sock_fd, &msg_hdr, 0);
	sched_yield();

err:
	if (sock_fd != -1)
		close(sock_fd);

	if (nlh)
		free(nlh);
}

int main(int argc, char *argv[])
{
	if (argc < 3) {
		msg("Usage: ast_send_event [ast_event_monitor's PID] [event]\n");
		return(1);
	}
	
	send_event(atoi(argv[1]), argv[2]);
	return(0);
}


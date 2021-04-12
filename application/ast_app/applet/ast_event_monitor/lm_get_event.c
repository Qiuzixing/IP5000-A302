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
#include <sys/un.h>
#include <unistd.h>
#include <string.h>
#include <sched.h>
#include "ast_event_monitor.h"

#define msg(fmt, args...) \
	do { \
		fprintf(stderr, fmt, ##args); \
	} while(0)

int main(void)
{
	struct sockaddr_un address;
	int  socket_fd, nbytes;
	char buffer[MAX_PAYLOAD];

	socket_fd = socket(PF_UNIX, SOCK_SEQPACKET, 0);
	if (socket_fd < 0) {
		msg("socket() failed\n");
		return 1;
	}

	/* start with a clean address structure */
	memset(&address, 0, sizeof(struct sockaddr_un));

	address.sun_family = AF_UNIX;
	snprintf(address.sun_path, UNIX_PATH_MAX, UDS_PATH_NAME_LM_EVENT);
	/* Abstract names for Unix domain socket. Linux specific. Set first byte of address name null. */
	address.sun_path[0] = 0;

	while (connect(socket_fd, (struct sockaddr *) &address, sizeof(struct sockaddr_un)) != 0) {
		//msg("connect() failed\n");
		sched_yield();
	}

	nbytes = read(socket_fd, buffer, MAX_PAYLOAD);
	buffer[nbytes] = 0;
	printf("%s", buffer);

	close(socket_fd);

	return 0;
}

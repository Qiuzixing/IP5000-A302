/*
 * Copyright (c) 2004-2012
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
#include <errno.h>
#include <sched.h>

#define UNIX_PATH_MAX    108

#define info(fmt, args...) \
	do { \
		fprintf(stderr, fmt, ##args); \
	} while(0)

int create_uds_server(unsigned char *uds_name)
{
	int socket_fd, connection_fd;
	socklen_t address_length;
	struct sockaddr_un address;

	/* Use SOCK_SEQPACKET. Stream type socket with data boundary. */
	socket_fd = socket(PF_UNIX, SOCK_SEQPACKET, 0);
	if (socket_fd < 0) {
		info("uds socket() failed\n");
		goto err;
	}

	/* start with a clean address structure */
	memset(&address, 0, sizeof(struct sockaddr_un));

	address.sun_family = AF_UNIX;
	snprintf(address.sun_path, UNIX_PATH_MAX, uds_name);
	address.sun_path[0] = 0;

	if (bind(socket_fd, (struct sockaddr *)&address, sizeof(struct sockaddr_un)) != 0) {
		info("uds bind() failed (%d:%s)\n", errno, strerror(errno));
		goto err;
	}

	/* 16 means accept up to 16 connection request in queue. */
	if (listen(socket_fd, 16) != 0) {
		info("uds listen() failed\n");
		goto err;
	}

	return socket_fd;
err:
	if (socket_fd >= 0) {
		close(socket_fd);
	}
	return -1;
}

int create_uds_client(unsigned char *uds_name)
{
	struct sockaddr_un address;
	int  socket_fd, nbytes;

	socket_fd = socket(PF_UNIX, SOCK_SEQPACKET, 0);
	if (socket_fd < 0) {
		info("socket() failed\n");
		return errno;
	}

	/* start with a clean address structure */
	memset(&address, 0, sizeof(struct sockaddr_un));

	address.sun_family = AF_UNIX;
	snprintf(address.sun_path, UNIX_PATH_MAX, uds_name);
	/* Abstract names for Unix domain socket. Linux specific. Set first byte of address name null. */
	address.sun_path[0] = 0;

	while (connect(socket_fd, (struct sockaddr *) &address, sizeof(struct sockaddr_un)) != 0) {
		//msg("connect() failed\n");
		//sched_yield();
		/* sleep for 10ms. */
		usleep(10000);
	}

	return socket_fd;
}

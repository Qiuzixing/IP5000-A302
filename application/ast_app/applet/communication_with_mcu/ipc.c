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
#include <sys/file.h> /* for flock() */
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>

#include "uds_ipc.h"
#include "ipc.h"
#define info(fmt, args...) \
	do { \
		fprintf(stderr, fmt, ##args); \
	} while(0)

#define MAX_PAYLOAD 8192

int ipc_get(char *ipc_ch,char *buffer)
{
	int nbytes;
	int ipc_fd = create_uds_client(ipc_ch);
	if(ipc_fd < 0)
	{
		info("ipc_fd() failed [%d:%s]\n", errno, strerror(errno));
		return -1;
	}

	nbytes = read(ipc_fd, buffer, MAX_LEN);
	if (nbytes < 0) {
		info("ipc_get() failed [%d:%s]\n", errno, strerror(errno));
		close(ipc_fd);
		return -1;
	}
	close(ipc_fd);
	return 0;
}

int ipc_set(char *ipc_ch, ipc_relay_msg *value, unsigned int count)
{
	int nbytes;
	int ipc_fd = create_uds_client(ipc_ch);
	if(ipc_fd < 0)
	{
		info("ipc_fd() failed [%d:%s]\n", errno, strerror(errno));
		return -1;
	}
	nbytes = write(ipc_fd, value, count);
	if (nbytes < 0) {
		info("ipc_set() failed [%d:%s]\n", errno, strerror(errno));
		close(ipc_fd);
		return -1;
	}
	close(ipc_fd);
	return 0;

}

FILE *lock_ipc_query(char *file_name)
{
	FILE *lock_file = NULL;
	char f[1024];

	snprintf(f, 1024, "/var/lock/%s.lck", file_name);
	lock_file = fopen(f, "r");
	if (lock_file == NULL) {
		err("failed to open %s\n", f);
		goto done;
	}
	if (flock(fileno(lock_file), LOCK_EX)) {
		err("%s lock failed?!\n", f);
		fclose(lock_file);
		lock_file = NULL;
		goto done;
	}
done:
	return lock_file;
}

int unlock_ipc_query(FILE *lock_file)
{
	if (lock_file) {
		flock(fileno(lock_file), LOCK_UN);
		fclose(lock_file);
	}
	return 0;
}

static void signal_handler(int i)
{
	printf("ERROR: signal catched, code %d\n", i);
}

static void set_signal(void)
{
	struct sigaction act;
	int i;

	bzero(&act, sizeof(act));
	act.sa_handler = signal_handler;
	sigemptyset(&act.sa_mask);
	sigaction(SIGTERM, &act, NULL);
	sigaction(SIGINT, &act, NULL);
	sigaction(SIGPIPE, &act, NULL);

	for (i = 1; i <= 30; i++) {
		sigaction(i, &act, NULL);
	}
}


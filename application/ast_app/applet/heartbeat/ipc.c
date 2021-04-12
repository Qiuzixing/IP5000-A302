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

#define info(fmt, args...) fprintf(stderr, fmt, ##args)

#define MAX_PAYLOAD 4096

unsigned char buffer[MAX_PAYLOAD];

static void ipc_get(int ipc_fd)
{
	int nbytes;

	nbytes = read(ipc_fd, buffer, MAX_PAYLOAD);
	if (nbytes < 0) {
		info("ipc_get() failed [%d:%s]\n", errno, strerror(errno));
		return;
	}
	buffer[nbytes] = 0;
	/* Output result to stdout. */
	printf("%s", buffer);
}

static void ipc_set(int ipc_fd, unsigned char *value, unsigned int count)
{
	int nbytes;

	nbytes = write(ipc_fd, value, count);

	if (nbytes < 0)
		info("ipc_set() failed [%d:%s]\n", errno, strerror(errno));
}

static void ipc_query(int ipc_fd, unsigned char *value, unsigned int count)
{
	ipc_set(ipc_fd, value, count);
	/*
	 * from jerry's test
	 * read() in ipc_get() returns 0 if peer closed ipc_fd before ipc_get()
	 */
	ipc_get(ipc_fd);
}

static FILE *lock_ipc_query(char *file_name)
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

#define MAX_IPC_CH_STR_BUF_SIZE 128

int hb_ipc_set(const unsigned char *ipc_ch, unsigned char *ipc_value)
{
	FILE *lock_file = NULL;
	int ipc_fd = -1;
	char str[MAX_IPC_CH_STR_BUF_SIZE];

	sprintf(str, "@%s_lm_set", ipc_ch);

	/* open ipc channel */
	ipc_fd = create_uds_client(str);

	if (ipc_fd >= 0) {
		ipc_set(ipc_fd, ipc_value, strlen(ipc_value));
		close(ipc_fd);
	}

	return 0;
}

int hb_ipc_query(const unsigned char *ipc_ch, unsigned char *ipc_value)
{
	FILE *lock_file = NULL;
	int ipc_fd = -1;
	char str[MAX_IPC_CH_STR_BUF_SIZE];

	sprintf(str, "@%s_lm_query", ipc_ch);

	/* to avoid deadlock, acquire lock beofre connect (in create_uds_client) */
	lock_file = lock_ipc_query(str);

	if (!lock_file)
		return -1;

	/* open ipc channel */
	ipc_fd = create_uds_client(str);

	if (ipc_fd >= 0) {
		ipc_query(ipc_fd, ipc_value, strlen(ipc_value));
		close(ipc_fd);
	}

	unlock_ipc_query(lock_file);

	return 0;
}

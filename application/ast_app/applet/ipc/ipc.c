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

#define info(fmt, args...) \
	do { \
		fprintf(stderr, fmt, ##args); \
	} while(0)

#define MAX_PAYLOAD 8192
unsigned char buffer[MAX_PAYLOAD];

enum cmd_type {
	CMD_QUERY = 0,
	CMD_GET,
	CMD_SET
};

void ipc_get(int ipc_fd)
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

void ipc_set(int ipc_fd, unsigned char *value, unsigned int count)
{
	int nbytes;

	nbytes = write(ipc_fd, value, count);
	if (nbytes < 0) {
		info("ipc_set() failed [%d:%s]\n", errno, strerror(errno));
	}
}

void ipc_query(int ipc_fd, unsigned char *value, unsigned int count)
{
	ipc_set(ipc_fd, value, count);
	/*
	 * from jerry's test
	 * read() in ipc_get() returns 0 if peer closed ipc_fd before ipc_get()
	 */
	ipc_get(ipc_fd);
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

int main(int argc, char *argv[])
{
	enum cmd_type cmd_type;
	unsigned char *ipc_ch = NULL, *ipc_value = NULL;
	int ipc_fd = -1;
	FILE *lock_file = NULL;

	set_signal();
	/*
	** Usage example:
	** ipc #hb_ctrl q a:start:multicast:225.10.1.1:port
	** ipc #videoip_ctrl s value
	** ipc #ipc_channel g
	*/
	switch (argv[2][0]) {
		case 'q':
			cmd_type = CMD_QUERY;
			if (argc != 4) {
				info("Wrong parameters. %d\n", argc);
				goto err;
			}
			ipc_ch = argv[1];
			ipc_value = argv[3];
			break;
		case 'g':
			cmd_type = CMD_GET;
			if (argc != 3) {
				info("Wrong parameters. %d\n", argc);
				goto err;
			}
			ipc_ch = argv[1];
			break;
		case 's':
			cmd_type = CMD_SET;
			if (argc != 4) {
				info("Wrong parameters. %d\n", argc);
				goto err;
			}
			ipc_ch = argv[1];
			ipc_value = argv[3];
			break;
		default:
			info("unknown command type?! %s\n", argv[2]);
			goto err;
	}


	if (cmd_type == CMD_QUERY) {
		/*
		 * for query command, lock file before connect, create_uds_client()
		 *
		 * from jerry's test,
		 * epoll_wait on ipc_server will return once socket gets connected
		 *
		 * following procedures may cause ipc_server doesn't receive any data after epoll_wait
		 * (1)connect (2)try to acquire lock (3)query
		 * for example: 2 ipc execute query to the same socket
		 *
		 *      ipc#1: connect  =>  blocking
		 *      ipc#2:  connect =>  lock  => set => get
		 * ipc_server:    read
		 *--------------+--+---------+--------+------+------ t
		 *		t0 t1        t2      t3      t4
		 * t0: ipc#1 connect to X_lm_query
		 * t1: ipc#2 connect to X_lm_query
		 * t2: ipc#2 lock file
		 * t3: ipc#2 write something
		 * t4: ipc#2 read
		 *
		 * ipc_server start to receive data from connection by ipc#1 after t0 and gets blocked due to no data
		 * ipc#1 gets blocked before lock acquired (after t2)
		 * ipc#2 gets blocked after t4
		 *
		 * change to,
		 * (1)try to acquire lock (2)connect (3)query
		 */
		lock_file = lock_ipc_query(ipc_ch);

		if (lock_file == NULL)
			goto err;
	}

	/* open ipc channel */
	ipc_fd = create_uds_client(ipc_ch);
	if (ipc_fd < 0) {
		goto err;
	}
	/* do the command */
	switch (cmd_type) {
	case CMD_QUERY:
		ipc_query(ipc_fd, ipc_value, strlen(ipc_value));
		break;
	case CMD_GET:
		ipc_get(ipc_fd);
		break;
	case CMD_SET:
		ipc_set(ipc_fd, ipc_value, strlen(ipc_value));
		break;
	default:
		goto err;
	}

	close(ipc_fd);

	if (lock_file)
		unlock_ipc_query(lock_file);

	return 0;
err:
	if (ipc_fd >= 0)
		close(ipc_fd);

	if (lock_file)
		unlock_ipc_query(lock_file);

	return -1;
}

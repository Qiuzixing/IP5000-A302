/*
 * Copyright (c) 2017
 * ASPEED Technology Inc. All Rights Reserved
 * Proprietary and Confidential
 *
 * By using this code you acknowledge that you have signed and accepted
 * the terms of the ASPEED SDK license agreement.
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/errno.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include "ir_ioctl.h"
#include "ir_rs.h"


#define MAX_SIZE IR_SIZE_TEXT_MAX /* IR_SIZE_TEXT_MAX is 2048 */
char buf[MAX_SIZE];

int main(int argc, char *argv[])
{
	int tx_fd; /* file description into transport */
	int buf_index = 0, length;

	if (argc > 2) {
		int i, len;

		for (i = 1; i < argc; i++) {
			len = strlen(argv[i]);

			if ((buf_index + len) > MAX_SIZE)
				goto exit;

			memcpy(buf + buf_index, argv[i], len);
			buf_index += len;
			buf[buf_index] = ' ';
			buf_index++;
		}
		buf[buf_index - 1] = '\0';

		length = buf_index;
	} else if (argc == 2) {

		if (strlen(argv[1]) > MAX_SIZE)
			goto exit;

		memcpy(buf, argv[1], strlen(argv[1]));
		buf[strlen(argv[1])] = '\0';
		length = strlen(buf);
	} else {
		goto exit;
	}

	tx_fd = create_socket_to_local_tcp(IR_S_D_PORT);
	send(tx_fd, buf, length, 0);
	close(tx_fd);
exit:
	return 0;
}

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
#include "ir_ioctl.h"
#include "ir_rs.h"

#define MAX_SIZE BUFSIZ /* BUFSIZ is 8192 */

char rec_buf[MAX_SIZE];

int main(void)
{
	int socket_fd; /* file description into transport */
	int length; /* length of address structure */
	int nbytes; /* the number of read */
	struct sockaddr_in myaddr; /* address of this service */
	struct sockaddr_in client_addr; /* address of client */

	/* Get a socket into UDP/IP */
	socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (socket_fd < 0) {
		perror("socket failed");
		exit(EXIT_FAILURE);
	}

	/* Set up our address */
	bzero((char *)&myaddr, sizeof(myaddr));
	myaddr.sin_family = AF_INET;
	myaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	myaddr.sin_port = htons(IR_R_PORT);

	/* Bind to the address to which the service will be offered */
	if (bind(socket_fd, (struct sockaddr *)&myaddr, sizeof(myaddr)) < 0) {
		perror("bind failed\n");
		exit(1);
	}

	nbytes = recvfrom(socket_fd, rec_buf, MAX_SIZE, 0, (struct sockaddr *)&client_addr, (socklen_t *)&length);

	if (nbytes != -1) {
#if 0
		int i, *ptr;

		ptr = (int *) rec_buf;
		for (i = 0; i < (nbytes >> 2); i++) {
			if ((i % 0x10) == 0)
				printf("\n");
			printf("%.4x ", *ptr);
			ptr++;

		}
		printf("\n");
#else
		printf("%s", rec_buf);
#endif
	}
	close(socket_fd);
}

/*
 * Copyright (c) 2004-2012
 * ASPEED Technology Inc. All Rights Reserved
 * Proprietary and Confidential
 *
 * By using this code you acknowledge that you have signed and accepted
 * the terms of the ASPEED SDK license agreement.
 */

#include <stdio.h>
#include <getopt.h>

static const struct option longopts[] = {
	{"host",	no_argument,	NULL, 'h'},
	{"client",	no_argument,	NULL, 'c'},
	{NULL,		0,		NULL,  0}
};

#define err(fmt, args...) \
	do { \
		fprintf(stderr, fmt, ##args); \
	} while(0)

#define M_SIZE 512
static void print_default_mac(int is_host)
{
	if (is_host)
		fprintf(stdout, "00:01:02:03:04:05");
	else
		fprintf(stdout, "00:01:02:03:04:06");
}


static void gen_random_mac(int is_host)
{
	FILE *fd = NULL;
	size_t len;
	char mac[6];
	char t[M_SIZE];
	int i;

	fd = fopen("/dev/urandom", "w+");
	if (fd == NULL)
	{
		err("failed to open /dev/urandom\n");
		print_default_mac(is_host);
		goto done;
	}
	// Try to read some random numbers. Meanless?!
	len = fread(t, 1, 1, fd);
	if (len != 1)
	{
		err("failed to read 1bytes data from /dev/urandom (%d)\n", len);
		print_default_mac(is_host);
		goto done;
	}
	len = t[0];
	//err("%02X\n", t[0]);
	for (i=0; i < len; i++) 
	{
		int j;
		fread(t, 1, M_SIZE, fd);
#if 0
		for (j=0; j<512; j++) {
			err("%02X", t[j]);
		}
		err("\n");
#endif
	}
	// Ok. Let get a random MAC
	fread(mac, 1, 5, fd);
	if (is_host)
		printf("02");
	else
		printf("82");
	
	for (i=0; i<5; i++) {
		printf(":");
		printf("%02X", mac[i]);
	}
	printf("\n");
	//err("\n");
	
done:
	if (fd != NULL)
		fclose(fd);
}


int  main (int argc, char **argv)
{
	int is_host = 0;
	enum {
		cmd_host,
		cmd_client,
		cmd_help
	} cmd = cmd_host;

	for (;;) {
		int c;
		int index = 0;

		c = getopt_long(argc, argv, "hc", longopts, &index);

		if (c == -1)
			break;

		switch (c) {
			case 'h':
				cmd = cmd_host;
				is_host = 1;
				break;
			case 'c':
				cmd = cmd_client;
				is_host = 0;
				break;
			case '?':
				cmd = cmd_help;
				break;
			default:
				err("getopt\n");
		}
	}

	switch (cmd) {
		case cmd_host:
			gen_random_mac(is_host);
			break;
		case cmd_client:
			gen_random_mac(is_host);
			break;
		case cmd_help:
			break;
		default:
			err("unknown cmd\n");
	}

	return 0;
}


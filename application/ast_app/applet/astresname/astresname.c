/*
 * Copyright (c) 2004-2012
 * ASPEED Technology Inc. All Rights Reserved
 * Proprietary and Confidential
 *
 * By using this code you acknowledge that you have signed and accepted
 * the terms of the ASPEED SDK license agreement.
 */

#include <stdio.h>
//#include <sys/types.h>
//#include <sys/stat.h>
#include <netdb.h>

int main (int argc, char **argv)
{
	struct hostent *host = NULL;

	if (argc != 2)
	{
		printf("usages:\n");
		printf("astresname <host_name>\n");
		return 0;
	}
	host = gethostbyname(argv[1]);
	if (host != NULL)
	{
#if 0
		printf("The IP address of %s is:\n%d.%d.%d.%d", argv[1], host->h_addr[0], host->h_addr[1], host->h_addr[2], host->h_addr[3]);
#else
		printf("%d.%d.%d.%d", host->h_addr[0], host->h_addr[1], host->h_addr[2], host->h_addr[3]);
#endif
		return 0;
	}
	else
	{
		printf("Unknown host");
//		herror("");
		return -1;
	}
}


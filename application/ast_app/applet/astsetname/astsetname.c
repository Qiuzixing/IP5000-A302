/*
 * Copyright (c) 2004-2012
 * ASPEED Technology Inc. All Rights Reserved
 * Proprietary and Confidential
 *
 * By using this code you acknowledge that you have signed and accepted
 * the terms of the ASPEED SDK license agreement.
 */

#include <stdio.h>
#include <string.h>

int  main (int argc, char **argv)
{
	FILE *fd = NULL;
	size_t len;
	int ret = 0;
	char *buffer = NULL;
	int i, j;

	if (argc < 2)
	{
		printf("usages:\n");
		printf("astsetname <host_name>\n");
		ret = -1;
		goto done;
	}

	fd = fopen("/etc/avahi/avahi-daemon.conf", "r+");
	if (fd == NULL)
	{
		printf("failed to open /etc/avahi/avahi-daemon.conf\n");
		ret = -2;
		goto done;
	}
	
	buffer = malloc(0x800);
	if (buffer == NULL)
	{
		printf("failed to allocate 2K buffer\n");
		ret = -3;
		goto done;
	}
	
	len = fread(buffer, 1, 0x800, fd);
	for (i = 0; i < len; i++)
	{
		if (memcmp((buffer + i), "host-name=", 10) == 0)
		{
			for (j = (i + 1); j < len; j++)
			{
				if (buffer[j] == 10)
					break;
			}
			memmove(buffer + (i + 10) + strlen(argv[1]), buffer + j, len - (i + 10));
			memmove(buffer + (i + 10), argv[1], strlen(argv[1]));
		}
	}
	
	rewind(fd);
	len = fwrite(buffer, 1, 0x800, fd);

done:
	if (buffer != NULL)
	{
		free(buffer);
		buffer = NULL;
	}
	if (fd != NULL)
		fclose(fd);
	return ret;
}


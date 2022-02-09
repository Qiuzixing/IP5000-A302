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

int main(int argc, char *argv[])
{
	FILE *fp_s = NULL;
	FILE *fp_t = NULL;
	unsigned char *word;
	unsigned char *str;
	char *sep=" |,\n\r";
	unsigned char *src_buf;
	int src_size;

	fp_s = fopen(argv[1], "rb");
	if (fp_s == NULL) {
		printf("fopen source error\n");
		return -1;
	}

	fp_t = fopen(argv[2], "wb");
	if (fp_t == NULL) {
		printf("fopen dest error\n");
		return -1;
	}

	fseek(fp_s, 0, SEEK_END);
	src_size = ftell(fp_s) + 1;
	rewind(fp_s);
	rewind(fp_t);
	
	src_buf = malloc(src_size);
	if (src_buf == NULL) {
		printf("malloc fail!\n");
		return -1;
	}
	fread(src_buf, 1, src_size, fp_s);
	
	str = (unsigned char*)src_buf;
	while ((word = strsep((char**)&str, sep)) != NULL) {
		if (strlen(word) == 0)
			continue;

		fputc((int)strtoul(word, NULL, 16), fp_t);
	}

	fclose(fp_s);
	fclose(fp_t);

	free(src_buf);
	return 0;

}


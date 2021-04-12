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

int main(int argc, char *argv[])
{
	FILE *fp_s = NULL;
	FILE *fp_t = NULL;
	unsigned int i = 0;
	unsigned char byte;

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


	while (fread(&byte, 1, 1, fp_s)) {
		if (!(i %= 16))
			fprintf(fp_t, "\n");

		fprintf(fp_t, "0x%02x, ", byte);
		i++;
	}
	fprintf(fp_t, "\n");

	fclose(fp_s);
	fclose(fp_t);

	return 0;

}


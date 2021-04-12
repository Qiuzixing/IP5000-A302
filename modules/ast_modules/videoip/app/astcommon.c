/*
 * Copyright (c) 2004-2012
 * ASPEED Technology Inc. All Rights Reserved
 * Proprietary and Confidential
 *
 * By using this code you acknowledge that you have signed and accepted
 * the terms of the ASPEED SDK license agreement.
 */

#include <unistd.h>
#include <strings.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h> /* string.h */

void DumpMem(void * pAddr, int iSize)
{
	unsigned long ulStart, ulEnd;
	unsigned long i, j;
	unsigned long ulDataBuf;

	ulDataBuf = (unsigned long)pAddr;
    ulStart = ulDataBuf & (~0xF);
    ulEnd   = (ulDataBuf + iSize) & (~0xF);

	printf("Data address [%x] Size [%d]\n", ulDataBuf, iSize);
    for (i = ulStart; i < ulEnd; i=i+16)
    {
        printf("0x%.4x:", i);

            for (j = i; j<(i+16); j=j+4)
                printf("    %.8x", *((unsigned long *)j));
 
        printf("\n");
    }	
}


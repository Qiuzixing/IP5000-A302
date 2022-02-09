/*
 * Copyright (c) 2004-2012
 * ASPEED Technology Inc. All Rights Reserved
 * Proprietary and Confidential
 *
 * By using this code you acknowledge that you have signed and accepted
 * the terms of the ASPEED SDK license agreement.
 */

#ifndef _TYPE_H_
#define _TYPE_H_

	typedef unsigned char               BOOL;
	typedef unsigned char               UINT8;
	typedef unsigned short              UINT16;
	typedef unsigned int                UINT32;

	#define  FLONG                    unsigned long
	#define  BYTE                     unsigned char
	#define  INT                      int
	#define  VOID                     void
	#define  BOOLEAN                  unsigned short
	#define  ULONG                    unsigned long
	#define  USHORT                   unsigned short
	#define  UCHAR                    unsigned char
	#define  CHAR                     char
	#define  LONG                     long
	#define  PUCHAR                   UCHAR *
	#define  PULONG                   ULONG *

	#define  FAIL                     1

	#define  intfunc      int386

	#define  outdwport         outpd
	#define  indwport          inpd
	#define  outport           outp
	#define  inport            inp

	//#define     NULL    ((void *)0)
	#define     FALSE   0
	#define     TRUE    1

#endif

/*
 * Copyright (c) 2004-2012
 * ASPEED Technology Inc. All Rights Reserved
 * Proprietary and Confidential
 *
 * By using this code you acknowledge that you have signed and accepted
 * the terms of the ASPEED SDK license agreement.
 */

#ifndef _DEBUG_H_
#define _DEBUG_H_

#define err(fmt, args...) \
	do { \
		fprintf(stderr, fmt, ##args); \
	} while(0)

#define msg(fmt, args...) \
	do { \
		fprintf(stdout, fmt, ##args); \
	} while(0)

#define info err
#if 0
#define dbg err
#else
#define dbg(fmt, args...) do{}while(0)
#endif

#endif

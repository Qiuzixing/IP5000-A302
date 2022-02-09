/*
 * Copyright (c) 2004-2012
 * ASPEED Technology Inc. All Rights Reserved
 * Proprietary and Confidential
 *
 * By using this code you acknowledge that you have signed and accepted
 * the terms of the ASPEED SDK license agreement.
 */

#ifndef __DEBUG_H__
#define __DEBUG_H__

#include <stdio.h>

#define err(fmt, args...)	do { \
	fprintf(stderr, "err: %13s:%4d (%-12s) " fmt "\n", \
		__FILE__, __LINE__, __FUNCTION__,  ##args); \
} while (0)

#define info(fmt, args...)	do { \
	fprintf(stderr, fmt "\n",  ##args); \
} while (0)

#define dbg(fmt, args...)	do { \
	fprintf(stderr, "dbg: %13s:%4d (%-12s) " fmt "\n", \
		__FILE__, __LINE__, __FUNCTION__,  ##args); \
} while (0)

#endif /* #ifndef __DEBUG_H__ */


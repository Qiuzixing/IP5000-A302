/*
 * Copyright (c) 2004-2012
 * ASPEED Technology Inc. All Rights Reserved
 * Proprietary and Confidential
 *
 * By using this code you acknowledge that you have signed and accepted
 * the terms of the ASPEED SDK license agreement.
 */

/* must put after global.h */
#ifndef _AST_DEBUG_H_
#define _AST_DEBUG_H_

#define err(fmt, args...)	do { \
	if (videoip_use_syslog) { \
		syslog(LOG_ERR, "videoip err: %13s:%4d (%-12s) " fmt "\n", \
			__FILE__, __LINE__, __FUNCTION__,  ##args); \
	} \
	if (videoip_use_stderr) { \
		fprintf(stderr, "videoip err: %13s:%4d (%-12s) " fmt "\n", \
			__FILE__, __LINE__, __FUNCTION__,  ##args); \
	} \
} while (0)

#define info(fmt, args...)	do { \
	if (videoip_use_syslog) { \
		syslog(LOG_DEBUG, fmt, ##args); \
	} \
	if (videoip_use_stderr) { \
		fprintf(stderr, fmt "\n",  ##args); \
	} \
} while (0)

#define dbg(fmt, args...)	do { \
	if (videoip_use_debug) { \
		if (videoip_use_syslog) { \
			syslog(LOG_DEBUG, "videoip dbg: %13s:%4d (%-12s) " fmt, \
				__FILE__, __LINE__, __FUNCTION__,  ##args); \
		} \
		if (videoip_use_stderr) { \
			fprintf(stderr, "videoip dbg: %13s:%4d (%-12s) " fmt "\n", \
				__FILE__, __LINE__, __FUNCTION__,  ##args); \
		} \
	} \
} while (0)

#endif /* _AST_DEBUG_H_ */


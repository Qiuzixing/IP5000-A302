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

#if defined SOIP_DEBUG

#define err(fmt, args...) fprintf(stderr, "err: %13s:%4d (%-12s) " fmt "\n", __FILE__, __LINE__, __func__,  ##args)
#define info(fmt, args...) fprintf(stderr, fmt "\n",  ##args)
#define dbg(fmt, args...) fprintf(stderr, "dbg: %13s:%4d (%-12s) " fmt "\n", __FILE__, __LINE__, __func__,  ##args)


#else

#define dbg(fmt, args...) do {} while (0)
#define info(fmt, args...) do {} while (0)
#define err(fmt, args...) do {} while (0)

#endif

#endif /* #ifndef __DEBUG_H__ */


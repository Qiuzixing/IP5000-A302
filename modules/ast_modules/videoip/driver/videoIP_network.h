/*
 * Copyright (c) 2004-2012
 * ASPEED Technology Inc. All Rights Reserved
 * Proprietary and Confidential
 *
 * By using this code you acknowledge that you have signed and accepted
 * the terms of the ASPEED SDK license agreement.
 */

#ifndef _VIDEO_NETWORK_H_
#define _VIDEO_NETWORK_H_

#include <net/sock.h>

#define TRANS_TCP	0
#define TRANS_UDP	1
int videoip_xmit(int, int, struct socket *, char *, int, int);
int videoip_xmit_ex(int, int, struct socket *, struct kvec *, size_t, int, int, void *);
int video_control_send(u32 cmd, u32 data, u8 *buf, u32 buf_size);
#endif

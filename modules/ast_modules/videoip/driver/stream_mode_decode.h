/*
 * Copyright (c) 2004-2012
 * ASPEED Technology Inc. All Rights Reserved
 * Proprietary and Confidential
 *
 * By using this code you acknowledge that you have signed and accepted
 * the terms of the ASPEED SDK license agreement.
 */
#ifndef _STREAM_MODE_DECODE_H_
#define _STREAM_MODE_DECODE_H_

#include "global.h"

void V1RxClient_loop(struct videoip_task *ut);
void _handle_decode_ve_hang_stream(VIDEOIP_DEVICE *v);

#if SYNC_WITH_VSYNC
void stream_sw_flip_reset(void);
void stream_sw_flip(u32 profile);
void client_dequeue_frame(void);
#endif /* #if SYNC_WITH_VSYNC */

#endif /* #ifndef _STREAM_MODE_DECODE_H_ */


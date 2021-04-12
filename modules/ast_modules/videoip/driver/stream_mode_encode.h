/*
 * Copyright (c) 2004-2012
 * ASPEED Technology Inc. All Rights Reserved
 * Proprietary and Confidential
 *
 * By using this code you acknowledge that you have signed and accepted
 * the terms of the ASPEED SDK license agreement.
 */
#ifndef _STREAM_MODE_ENCODE_H_
#define _STREAM_MODE_ENCODE_H_

unsigned int split_video_packet(unsigned int  NewPt, unsigned int EndCmd);
void V1TxHost_loop(struct videoip_task *ut);

#endif /* #ifndef _STREAM_MODE_ENCODE_H_ */


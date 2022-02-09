/*
 * Copyright (c) 2004-2012
 * ASPEED Technology Inc. All Rights Reserved
 * Proprietary and Confidential
 *
 * By using this code you acknowledge that you have signed and accepted
 * the terms of the ASPEED SDK license agreement.
 */



#ifndef __VHUB_MFG__
#define __VHUB_MFG__

#define MFG_OFF                            0
#define MFG_ONE_WAY                  1
#define MFG_LOOP_BACK_SLOW    2
#define MFG_NEVER_COMPLETE     3
#define MFG_LOOP_BACK               4
#define MFG_ONE_WAY_SEQ          5

#define MFG_FLAG_SHORT_PKT_STOP  BIT0
#define MFG_FLAG_AGGREGATE_OUT   BIT1
#define MFG_FLAG_32_BUF                   BIT2

#define MFG_DEV_ID ((1/*busnum*/ << 16) | 1/*devnum*/)
extern u32 mfg_mode; //from 0~99
extern u32 mfg_flag; //Start from 100. Bitmap

void mfg_init(void);
void mfg_vhub_tx_urb(struct urb *urb);
void mfg_destroy(void);


#endif

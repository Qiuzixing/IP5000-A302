/*
 * Copyright (c) 2004-2012
 * ASPEED Technology Inc. All Rights Reserved
 * Proprietary and Confidential
 *
 * By using this code you acknowledge that you have signed and accepted
 * the terms of the ASPEED SDK license agreement.
 */

#ifndef _VE_HW_QUALITY_PROFILE_H_
#define _VE_HW_QUALITY_PROFILE_H_

#include "global.h"

#if defined(QUALITY_AUTO_YUV420_CHECK)
int is_the_same_yuv_420_status(int now, int next);
int profile_yuv420_status(u32 index);
#endif

void InitHwProfile(void);
void switchHWProfile(int quality_mode);


#endif /* #ifndef _VE_HW_QUALITY_PROFILE_H_ */

/*
 * Copyright (c) 2004-2012
 * ASPEED Technology Inc. All Rights Reserved
 * Proprietary and Confidential
 *
 * By using this code you acknowledge that you have signed and accepted
 * the terms of the ASPEED SDK license agreement.
 */

#ifndef _DELAY_DEQUEUE_H_
#define _DELAY_DEQUEUE_H_

#include "global.h"

#if DELAY_DEQUEUE
void fire_hw_timer(void);
void stop_hw_timer(void);

#endif /* #if DELAY_DEQUEUE */
#endif /* #ifndef _DELAY_DEQUEUE_H_ */

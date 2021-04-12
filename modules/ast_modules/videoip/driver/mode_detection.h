/*
 * Copyright (c) 2004-2012
 * ASPEED Technology Inc. All Rights Reserved
 * Proprietary and Confidential
 *
 * By using this code you acknowledge that you have signed and accepted
 * the terms of the ASPEED SDK license agreement.
 */
#ifndef _MODE_DETECTION_H_
#define _MODE_DETECTION_H_

void ve_start_timing_watchdog(void);
void ve_cfg_timing_watchdog(void);

void auto_phase_detection(void);

void mode_detect_work(unsigned int life_cycle);

#endif /* #ifndef _MODE_DETECTION_H_ */

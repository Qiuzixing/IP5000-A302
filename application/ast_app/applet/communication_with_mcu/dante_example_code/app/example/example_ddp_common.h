/*
* File     : example_ddp_common.h
* Created  : May 2017
* Updated  : 2017/05/15
* Author   : Jerry Kim
* Synopsis : Prototypes used across the Brooklyn-II/Broadway and Ultimo HostCPU API.
*
* Copyright 2005-2017, Audinate Pty Ltd and/or its licensors
* Confidential
* Audinate Copyright Header Version 1
*/

#ifndef EXAMPLE_DDP_CONMON_H_
#define EXAMPLE_DDP_CONMON_H_

#include "aud_platform.h"

/**
 * Dante route ready flag for user
 */
void hostcpu_ddp_route_ready(uint8_t route_ready_flag);

/**
 * Current sample rate update for user
 */
void hostcpu_ddp_srate_update(int new_srate);

#endif /* EXAMPLE_DDP_CONMON_H_ */


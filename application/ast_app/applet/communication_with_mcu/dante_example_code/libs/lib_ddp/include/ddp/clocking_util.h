/*
* File     : clock_util.h
* Created  : May 2016
* Updated  : Date: 2016/05/19
* Author   : Naeem Bacha <naeem.bacha@audinate.com>
* Synopsis : Utillity helper functions for clocking DDP.
*
* Copyright 2005-2017, Audinate Pty Ltd and/or its licensors
Confidential
* Audinate Copyright Header Version 1
*/

#ifndef CLOCKING_UTIL_H_
#define CLOCKING_UTIL_H_

#include "ddp.h"

#ifdef __cplusplus
extern "C" {
#endif

const char * ddp_port_state_to_string(uint8_t port_state);

const char * ddp_servo_state_to_string(uint8_t servo_state);

const char * ddp_pullup_to_string(uint32_t pullup);

const char * ddp_port_path_type_to_string(uint8_t castness);

#ifdef __cplusplus
}
#endif

#endif /* CLOCKING_UTIL_H_ */

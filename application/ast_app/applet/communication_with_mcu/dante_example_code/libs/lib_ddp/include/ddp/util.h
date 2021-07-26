/*
* File     : ddp_util.c
* Created  : August 2014
* Updated  : Date: 2014/08/22
* Author   : Michael Ung <michael.ung@audinate.com>
* Synopsis : Utillity helper functions for DDP.
*
* Copyright 2005-2017, Audinate Pty Ltd and/or its licensors
Confidential
* Audinate Copyright Header Version 1
*/

#ifndef _DDP_UTIL_H
#define _DDP_UTIL_H

#include "ddp.h"

#ifdef __cplusplus
extern "C" {
#endif

const char * ddp_opcode_to_string(ddp_opcode_t opcode);

const char * ddp_status_to_string(ddp_status_t status);

const char *
ddp_packet_header_flag_direction_to_string(uint8_t dir);

aud_bool_t
ddp_verify_spi_baud_rate(uint32_t baud);

aud_bool_t
ddp_verify_uart_baud_rate(uint32_t baud);

#ifdef __cplusplus
}
#endif

#endif // _DDP_UTIL_H

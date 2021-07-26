/*
 * File     : example_rx_uhip.h
 * Created  : October 2014
 * Updated  : 2014/10/02
 * Author   : Michael Ung
 * Synopsis : UHIP RX helper functions
 *
 * Copyright 2005-2017, Audinate Pty Ltd and/or its licensors
 * Confidential
 * Audinate Copyright Header Version 1
 */
#ifndef __EXAMPLE_RX_UHIP_H__
#define __EXAMPLE_RX_UHIP_H__

#include "aud_platform.h"

/**
 * Resets the RX buffers
 */
void reset_uhip_rx_buffers(void);

/**
 * Handles any pending RX messages
 */
void handle_uhip_rx(unsigned char *dante_date,unsigned short int data_len);

#endif // __EXAMPLE_RX_UHIP_H__

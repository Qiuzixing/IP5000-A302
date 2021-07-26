/*
 * File     : example_tx_uhip.h
 * Created  : October 2014
 * Updated  : 2014/10/02
 * Author   : Michael Ung
 * Synopsis : UHIP TX helper functions
 *
 * Copyright 2005-2017, Audinate Pty Ltd and/or its licensors
 * Confidential
 * Audinate Copyright Header Version 1
 */
#ifndef __EXAMPLE_TX_UHIP_H__
#define __EXAMPLE_TX_UHIP_H__

#include "aud_platform.h"

/**
 * Handles any pending TX messages
 */
void handle_uhip_tx(void);

/*
 * Wrapping DDP packet in uhip packet structure & send over physical layer
 * 1. Initialize tx buffer in UHIP structure
 * 2. Creating the UHIP structure with DDP packet
 * 3. Cobs encoding UHIP + DDP packet
 * 4. Add delimeter data and align packet size in 32bytes
 * 5. Send over SPI/UART
 */
aud_error_t prepare_uhip_packet_and_send(uint8_t *tx_buf, ddp_size_t ddp_packet_len);

#endif // __EXAMPLE_TX_UHIP_H__

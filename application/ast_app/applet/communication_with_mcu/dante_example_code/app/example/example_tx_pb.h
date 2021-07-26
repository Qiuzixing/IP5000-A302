/*
* File     : example_tx_pb.h
* Created  : October 2014
* Updated  : 2014/10/28
* Author   : Michael Ung
* Synopsis : HostCPU Implementation of sending example packet bridge requests
*
* Copyright 2005-2017, Audinate Pty Ltd and/or its licensors
* Confidential
* Audinate Copyright Header Version 1
*/

#ifndef __EXAMPLE_TX_PB_H__
#define __EXAMPLE_TX_PB_H__

#include "aud_platform.h"

/**
* Send a ConMon packet bridge packet
* @return AUD_SUCCESS if the packet was sucessfully created and transmitted, otherwise a value other than AUD_SUCCESS is returned
*/
aud_error_t send_conmon_packetbridge_packet(void);

/**
* Send a UDP packet bridge packet
* @return AUD_SUCCESS if the packet was sucessfully created and transmitted, otherwise a value other than AUD_SUCCESS is returned
*/
aud_error_t send_udp_packetbridge_packet(void);

#endif // __EXAMPLE_TX_PB_H__

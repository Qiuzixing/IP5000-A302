/*
 * File     : uhip_util.h
 * Created  : Oct 2014
 * Updated  : 2014/10/08
 * Author   : Michael Ung
 * Synopsis : Ultimo Host Interface Protocol (UHIP) Utility functions
 *
 * Copyright 2005-2017, Audinate Pty Ltd and/or its licensors
	Confidential
 * Audinate Copyright Header Version 1
 */

#ifndef __UHIP_UTIL_H__
#define __UHIP_UTIL_H__

#include "uhip_structures.h"


 /** \addtogroup UHIPFuncs
 *  @{
 */

/**
 * Converts a UHIP packet type to a string
 * @param type [in] the UHIP packet type
 * @return A string representation of this packet type
 */
const char * uhip_packet_type_to_string(UhipPacketType type);

/**
 * Converts a UHIP protocol control type to a string
 * @param type [in] the UHIP protocol control type
 * @return A string representation of this type
 */
const char * uhip_protocol_control_type_to_string(UhipProtocolControlType type);

/**@}*/


/** \addtogroup UHIPPBFuncs
*  @{
*/

/**
 * Converts a UHIP ConMon packet bridge channel to a string
 * @param channel [in] the UHIP ConMon packet bridge channel
 * @return A string representation of this channel
 */
const char * uhip_cmc_pb_chan_to_string(UhipConMonPacketBridgeChannel channel);

/**
 * Converts a UHIP UDP packet bridge channel to a string
 * @param channel [in] the UHIP UDP packet bridge channel
 * @return A string representation of this channel
 */
const char * uhip_udp_pb_chan_to_string(UhipUDPPacketBridgeChannel channel);

/**@}*/

#endif // __UHIP_UTIL_H__

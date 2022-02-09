/*
 * File     : uhip_packet_write.h
 * Created  : Sept 2014
 * Updated  : 2014/09/30
 * Author   : Michael Ung
 * Synopsis : Ultimo Host Interface Protocol (UHIP) packet write functions
 *
 * Copyright 2005-2017, Audinate Pty Ltd and/or its licensors
	Confidential
 * Audinate Copyright Header Version 1
 */

#ifndef __UHIP_PACKET_WRITE_H__
#define __UHIP_PACKET_WRITE_H__

#include "dante/dante_common.h"
#include "uhip_structures.h"


//! @cond Doxygen_Suppress
typedef struct uhip_packet_write_info
{
	union
	{
		uint8_t * buf8;
		uint16_t * buf16;
		uint32_t * buf32;
		uhip_header_t * header;
	} _;

	uint16_t max_length_bytes, curr_length_bytes;
} uhip_packet_write_info_t;
//! @endcond

/** \addtogroup UHIPFuncs
*  @{
*/

/**
* Initialises the UHIP packet write stucture
* @param packet_info [in] Pointer to a structure which will contain information about the UHIP packet buffer
* @param buf [in] Pointer to the the buffer to use
* @param buf_len [in] The length of the buffer
* @return AUD_SUCCESS if the UHIP message write structure is successfully initialised, else a non AUD_SUCCESS value is returned
*/
aud_error_t
uhip_packet_init_write
(
	uhip_packet_write_info_t * packet_info,
	uint32_t * buf,
	uint16_t buf_len
);

/**
* Builds a UHIP protocol control message
* @param packet_info [in] Pointer to a structure which has information about the UHIP packet buffer
* @param type [in] The protocol control type of this message
* @return AUD_SUCCESS if the UHIP protocol control message was successfully created, else a non AUD_SUCCESS value is returned
*/
aud_error_t
uhip_packet_write_protocol_control
(
	uhip_packet_write_info_t * packet_info,
	UhipProtocolControlType type
);

/**@}*/


/** \addtogroup UHIPPBFuncs
*  @{
*/

/**
* Builds a UHIP ConMon packet bridge message
* @param packet_info [in] Pointer to a structure which has information about the UHIP packet buffer
* @param channel [in] The ConMon channel
* @param payload [in] Pointer to the payload
* @param payload_length [in] Length of the payload
* @return AUD_SUCCESS if the UHIP ConMon message was successfully created, else a non AUD_SUCCESS value is returned
*/
aud_error_t
uhip_packet_write_cmc_packet_bridge
(
	uhip_packet_write_info_t * packet_info,
	UhipConMonPacketBridgeChannel channel,
	uint8_t * payload,
	uint16_t payload_length
);

/**
* Builds a UHIP UDP packet bridge message
* @param packet_info [in] Pointer to a structure which has information about the UHIP packet buffer
* @param channel [in] The UDP channel
* @param port [in] The destination UDP port to send this packet to
* @param ip [in] The destination IP address to send this packet to
* @param payload [in] Pointer to the payload
* @param payload_length [in] Length of the payload
* @return AUD_SUCCESS if the UHIP UDP message was successfully created, else a non AUD_SUCCESS value is returned
*/
aud_error_t
uhip_packet_write_udp_packet_bridge
(
	uhip_packet_write_info_t * packet_info,
	UhipUDPPacketBridgeChannel channel,
	uint16_t port,
	uint32_t ip,
	uint8_t * payload,
	uint16_t payload_length
);

/**@}*/

/** \addtogroup UHIPFuncs
*  @{
*/

/**
* Builds a dante event message
* @param packet_info [in] Pointer to a structure which has information about the UHIP packet buffer
* @param payload [in] Pointer to the payload
* @param payload_length [in] Length of the payload
* @return AUD_SUCCESS if the Dante Event message was successfully created, else a non AUD_SUCCESS value is returned
*/
aud_error_t
uhip_packet_write_dante_event
(
	uhip_packet_write_info_t * packet_info,
	uint32_t * payload,
	uint16_t payload_length
);

/**
* Builds a Dante Device Protocol (DDP) message
* @param packet_info [in] Pointer to a structure which has information about the UHIP packet buffer
* @param payload [in] Pointer to the payload
* @param payload_length [in] Length of the payload
* @return AUD_SUCCESS if the Dante Device Protocol message was successfully created, else a non AUD_SUCCESS value is returned
*/
aud_error_t
uhip_packet_write_dante_device_protcol
(
	uhip_packet_write_info_t * packet_info,
	uint32_t * payload,
	uint16_t payload_length
);

/**@}*/

#endif // __UHIP_PACKET_WRITE_H__

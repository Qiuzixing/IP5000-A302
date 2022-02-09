/*
 * File     : uhip_packet_read.h
 * Created  : Sept 2014
 * Updated  : 2014/09/30
 * Author   : Michael Ung
 * Synopsis : Ultimo Host Interface Protocol (UHIP) packet read functions
 *
 * Copyright 2005-2017, Audinate Pty Ltd and/or its licensors
	Confidential
 * Audinate Copyright Header Version 1
 */

#ifndef __UHIP_PACKET_READ_H__
#define __UHIP_PACKET_READ_H__

#include "dante/dante_common.h"
#include "uhip_structures.h"

/** \addtogroup UHIPPktHelpers
*  @{
*/

//! @cond Doxygen_Suppress
typedef struct uhip_version
{
	uint8_t major;
	uint8_t minor;
} uhip_version_t;
//! @endcond

//! @cond Doxygen_Suppress
typedef struct uhip_packet_read_info
{
	uint16_t packet_length_bytes;
	union
	{
		const uint8_t * buf8;
		const uint16_t * buf16;
		const uint32_t * buf32;
		const uhip_header_t * header;
	} _;
	uhip_version_t version;
} uhip_packet_read_info_t;
//! @endcond

/**
* \defgroup UHIPFuncs UHIP Functions
* @{
*/

/**
* Initialises the read of a UHIP packet (NOTE: this must be after COBS decoding)
* @param packet_info [in] Pointer to a structure which will contain information about the UHIP packet buffer
* @param buf [in] Pointer to the the received packet
* @param len [in] The length of the received packet
* @return AUD_SUCCESS if the UHIP message read structure is successfully initialised, else a non AUD_SUCCESS value is returned
*/
aud_error_t
uhip_packet_init_read
(
	uhip_packet_read_info_t * packet_info,
	const uint32_t * buf,
	uint16_t len
);

/**
* Reads the UHIP packet type of a received packet
* @param packet_info [in] Pointer to a structure which has information about the UHIP packet buffer
* @param out_type [out optional] The UHIP packet type of this message
* @return AUD_SUCCESS if the UHIP type successfully read, else a non AUD_SUCCESS value is returned
*/
aud_error_t
uhip_packet_read_get_type
(
	uhip_packet_read_info_t * packet_info,
	UhipPacketType * out_type
);

/**
* Reads data fields from a UHIP protocol control message
* @param packet_info [in] Pointer to a structure which has information about the UHIP packet buffer
* @param out_type [out optional] The protocol control type of this message
* @return AUD_SUCCESS if the UHIP protocol control message was successfully read, else a non AUD_SUCCESS value is returned
*/
aud_error_t
uhip_packet_read_protocol_control
(
	uhip_packet_read_info_t * packet_info,
	UhipProtocolControlType * out_type
);

/**@}*/
/**@}*/

/** \addtogroup UHIPPB
*  @{
*/

/**
* \defgroup UHIPPBFuncs Packet Bridge Functions
* @{
*/

/**
* Reads data fields from a UHIP ConMon packet bridge message
* @param packet_info [in] Pointer to a structure which has information about the UHIP packet buffer
* @param out_channel [out] The ConMon channel
* @param out_payload [out] Pointer to the payload
* @param out_payload_length [out] Length of the payload
* @return AUD_SUCCESS if the UHIP ConMon message was successfully read, else a non AUD_SUCCESS value is returned
*/
aud_error_t
uhip_packet_read_cmc_packet_bridge
(
	uhip_packet_read_info_t * packet_info,
	UhipConMonPacketBridgeChannel * out_channel,
	uint8_t ** out_payload,
	uint16_t * out_payload_length
);

/**
* Reads data fields from a UHIP UDP packet bridge message
* @param packet_info [in] Pointer to a structure which has information about the UHIP packet buffer
* @param out_channel [out] The UDP channel
* @param out_port [out] The source UDP port this was received from
* @param out_ip [out] The source IP address this was received from
* @param out_payload [out] Pointer to the payload
* @param out_payload_length [out] Length of the payload
* @return AUD_SUCCESS if the UHIP UDP message was successfully read, else a non AUD_SUCCESS value is returned
*/
aud_error_t
uhip_packet_read_udp_packet_bridge
(
	uhip_packet_read_info_t * packet_info,
	UhipUDPPacketBridgeChannel * out_channel,
	uint16_t * out_port,
	uint32_t * out_ip,
	uint8_t ** out_payload,
	uint16_t * out_payload_length
);

/**@}*/
/**@}*/

/** \addtogroup UHIPFuncs
*  @{
*/

/**
* Reads payload from a dante event message
* @param packet_info [in] Pointer to a structure which has information about the UHIP packet buffer
* @param out_payload [out] Pointer to the payload
* @param out_payload_length [out] Length of the payload
* @return AUD_SUCCESS if the Dante Event message was successfully read, else a non AUD_SUCCESS value is returned
*/
aud_error_t
uhip_packet_read_dante_event
(
	uhip_packet_read_info_t * packet_info,
	uint8_t ** out_payload,
	uint16_t * out_payload_length
);

/**
* Reads payload from a Dante Device Protocol (DDP) message
* @param packet_info [in] Pointer to a structure which has information about the UHIP packet buffer
* @param out_payload [out] Pointer to the payload
* @param out_payload_length [out] Length of the payload
* @return AUD_SUCCESS if the Dante Device Protocol message was successfully read, else a non AUD_SUCCESS value is returned
*/
aud_error_t
uhip_packet_read_dante_device_protcol
(
	uhip_packet_read_info_t * packet_info,
	uint8_t ** out_payload,
	uint16_t * out_payload_length
);

/**@}*/

#endif // __UHIP_PACKET_READ_H__

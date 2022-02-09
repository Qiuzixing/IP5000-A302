/*
 * File     : uhip_packet_write.c
 * Created  : Sept 2014
 * Updated  : 2014/09/30
 * Author   : Michael Ung
 * Synopsis : Ultimo Host Interface Protocol (UHIP) packet write functions
 *
 * Copyright 2005-2017, Audinate Pty Ltd and/or its licensors
	Confidential
 * Audinate Copyright Header Version 1
 */

#include <arpa/inet.h>
#include "uhip_packet_write.h"

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
)
{
	const uint16_t length_bytes = sizeof(uhip_header_t);

	if (!packet_info || !buf || !buf_len)
	{
		return AUD_ERR_INVALIDPARAMETER;
	}

	if (buf_len < length_bytes)
	{
		return AUD_ERR_NOMEMORY;
	}

	packet_info->_.buf32 = buf;
	packet_info->max_length_bytes = buf_len;
	packet_info->curr_length_bytes = 0;

	//clear the header
	memset(packet_info->_.header, 0x00, sizeof(uhip_header_t));

	//set the UHIP version
	packet_info->_.header->version_major = UHIP_MAJOR_VER;
	packet_info->_.header->version_minor = UHIP_MINOR_VER;

	return AUD_SUCCESS;
}

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
)
{
	if(!packet_info)
	{
		return AUD_ERR_INVALIDPARAMETER;
	}

	//check that this is an empty packet info
	if( (packet_info->curr_length_bytes != 0) || (packet_info->_.header->packet_type != 0) )
	{
		return AUD_ERR_INVALIDSTATE;
	}

	//check that there is enough room
	if(packet_info->max_length_bytes < sizeof(uhip_protocol_control_t))
	{
		return AUD_ERR_NOMEMORY;
	}

	//builds the packet
	uhip_protocol_control_t* protocol_control = (uhip_protocol_control_t*)packet_info->_.buf32;
	protocol_control->header.packet_type = (uint8_t)UhipPacketType_PROTOCOL_CONTROL;
	protocol_control->header.size_bytes = htons(sizeof(uhip_protocol_control_t));
	protocol_control->header.data_offset = htons(sizeof(uhip_header_t));
	protocol_control->type = type;
	protocol_control->pad = 0;
	protocol_control->pad1 = 0;

	//set the length
	packet_info->curr_length_bytes = sizeof(uhip_protocol_control_t);

	return AUD_SUCCESS;
}

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
)
{
	if(!packet_info)
	{
		return AUD_ERR_INVALIDPARAMETER;
	}

	//check that this is an empty packet info
	if( (packet_info->curr_length_bytes != 0) || (packet_info->_.header->packet_type != 0) )
	{
		return AUD_ERR_INVALIDSTATE;
	}

	//check that there is enough room
	if(packet_info->max_length_bytes < (sizeof(uhip_cmc_packet_bridge_t) + payload_length) )
	{
		return AUD_ERR_NOMEMORY;
	}

	//builds the packet
	uhip_cmc_packet_bridge_t* cmc_pb = (uhip_cmc_packet_bridge_t*)packet_info->_.buf32;
	cmc_pb->header.packet_type = (uint8_t)UhipPacketType_CONMON_PACKET_BRIDGE;
	cmc_pb->header.size_bytes = htons(sizeof(uhip_cmc_packet_bridge_t) + payload_length);
	cmc_pb->header.data_offset = htons(sizeof(uhip_cmc_packet_bridge_t));
	cmc_pb->ch_id = htons((uint16_t)channel);
	cmc_pb->pad = 0;
	memcpy(packet_info->_.buf8 + sizeof(uhip_cmc_packet_bridge_t), payload, payload_length);

	//set the length
	packet_info->curr_length_bytes = sizeof(uhip_cmc_packet_bridge_t) + payload_length;

	return AUD_SUCCESS;
}

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
)
{
	if(!packet_info)
	{
		return AUD_ERR_INVALIDPARAMETER;
	}

	//check that this is an empty packet info
	if( (packet_info->curr_length_bytes != 0) || (packet_info->_.header->packet_type != 0) )
	{
		return AUD_ERR_INVALIDSTATE;
	}

	//check that there is enough room
	if(packet_info->max_length_bytes < (sizeof(uhip_udp_packet_bridge_t) + payload_length) )
	{
		return AUD_ERR_NOMEMORY;
	}

	//builds the packet
	uhip_udp_packet_bridge_t* udp_pb = (uhip_udp_packet_bridge_t*)packet_info->_.buf32;
	udp_pb->header.packet_type = (uint8_t)UhipPacketType_UDP_PACKET_BRIDGE;
	udp_pb->header.size_bytes = htons(sizeof(uhip_udp_packet_bridge_t) + payload_length);
	udp_pb->header.data_offset = htons(sizeof(uhip_udp_packet_bridge_t));
	udp_pb->ch_id = htons((uint16_t)channel);
	udp_pb->port = htons(port);
	udp_pb->ip = htonl(ip);

	memcpy(packet_info->_.buf8 + sizeof(uhip_udp_packet_bridge_t), payload, payload_length);

	//set the length
	packet_info->curr_length_bytes = sizeof(uhip_udp_packet_bridge_t) + payload_length;

	return AUD_SUCCESS;
}

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
)
{
	if(!packet_info)
	{
		return AUD_ERR_INVALIDPARAMETER;
	}

	//check that this is an empty packet info
	if( (packet_info->curr_length_bytes != 0) || (packet_info->_.header->packet_type != 0) )
	{
		return AUD_ERR_INVALIDSTATE;
	}

	//check that there is enough room
	if(packet_info->max_length_bytes < (sizeof(uhip_header_t) + payload_length) )
	{
		return AUD_ERR_NOMEMORY;
	}

	//builds the packet
	packet_info->_.header->packet_type = (uint8_t)UhipPacketType_DANTE_EVENTS;
	packet_info->_.header->size_bytes = htons(sizeof(uhip_header_t) + payload_length);
	packet_info->_.header->data_offset = htons(sizeof(uhip_header_t));
	memcpy(packet_info->_.buf8 + sizeof(uhip_header_t), payload, payload_length);

	//set the length
	packet_info->curr_length_bytes = sizeof(uhip_header_t) + payload_length;

	return AUD_SUCCESS;
}

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
)
{
	if(!packet_info)
	{
		return AUD_ERR_INVALIDPARAMETER;
	}

	//check that this is an empty packet info
	if( (packet_info->curr_length_bytes != 0) || (packet_info->_.header->packet_type != 0) )
	{
		return AUD_ERR_INVALIDSTATE;
	}

	//check that there is enough room
	if(packet_info->max_length_bytes < (sizeof(uhip_header_t) + payload_length) )
	{
		return AUD_ERR_NOMEMORY;
	}

	//builds the packet
	packet_info->_.header->packet_type = (uint8_t)UhipPacketType_DDP;
	packet_info->_.header->size_bytes = htons(sizeof(uhip_header_t) + payload_length);
	packet_info->_.header->data_offset = htons(sizeof(uhip_header_t));
	memcpy(packet_info->_.buf8 + sizeof(uhip_header_t), payload, payload_length);

	//set the length
	packet_info->curr_length_bytes = sizeof(uhip_header_t) + payload_length;

	return AUD_SUCCESS;
}


/*
 * File     : uhip_packet_read.c
 * Created  : Sept 2014
 * Updated  : 2014/09/30
 * Author   : Michael Ung
 * Synopsis : Ultimo Host Interface Protocol (UHIP) packet read functions
 *
 * Copyright 2005-2017, Audinate Pty Ltd and/or its licensors
	Confidential
 * Audinate Copyright Header Version 1
 */

#include <arpa/inet.h>
#include "uhip_packet_read.h"

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
)
{
	if (!packet_info || !buf || !len)
	{
		return AUD_ERR_INVALIDPARAMETER;
	}
	packet_info->_.buf32 = buf;
	packet_info->packet_length_bytes = len;

	if (ntohs(packet_info->_.header->size_bytes) < sizeof(uhip_header_t))
	{
		return AUD_ERR_INVALIDDATA;
	}
	packet_info->version.major = packet_info->_.header->version_major;
	packet_info->version.minor = packet_info->_.header->version_minor;

	return AUD_SUCCESS;
}

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
)
{
	if (!packet_info)
	{
		return AUD_ERR_INVALIDPARAMETER;
	}

	if(out_type)
	{
		*out_type = (UhipPacketType)packet_info->_.header->packet_type;
	}

	return AUD_SUCCESS;
}

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
)
{
	if (!packet_info)
	{
		return AUD_ERR_INVALIDPARAMETER;
	}

	if( ((UhipPacketType)(packet_info->_.header->packet_type) != UhipPacketType_PROTOCOL_CONTROL) ||
		(packet_info->packet_length_bytes < sizeof(uhip_protocol_control_t)) )
	{
		return AUD_ERR_INVALIDDATA;
	}

	uhip_protocol_control_t* control_packet = (uhip_protocol_control_t*)packet_info->_.buf32;
	if(out_type)
	{
		*out_type = (UhipProtocolControlType)control_packet->type;
	}

	return AUD_SUCCESS;
}

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
)
{
	uint16_t payload_length = 0;

	if (!packet_info)
	{
		return AUD_ERR_INVALIDPARAMETER;
	}

	if( (packet_info->_.header->packet_type != UhipPacketType_CONMON_PACKET_BRIDGE) ||
		(packet_info->packet_length_bytes < sizeof(uhip_cmc_packet_bridge_t)) )
	{
		return AUD_ERR_INVALIDDATA;
	}

	uhip_cmc_packet_bridge_t* cmc_pb_packet = (uhip_cmc_packet_bridge_t*)packet_info->_.buf32;
	if(out_channel)
	{
		*out_channel = (UhipConMonPacketBridgeChannel)(ntohs(cmc_pb_packet->ch_id));
	}

	payload_length = packet_info->packet_length_bytes - sizeof(uhip_cmc_packet_bridge_t);
	if(out_payload)
	{
		if(payload_length)
		{
			*out_payload = (uint8_t *)packet_info->_.buf8 + sizeof(uhip_cmc_packet_bridge_t);
		}
		else
		{
			*out_payload = NULL;
		}
	}

	if(out_payload_length)
	{
		*out_payload_length = payload_length;
	}

	return AUD_SUCCESS;
}

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
)
{
	uint16_t payload_length = 0;

	if (!packet_info)
	{
		return AUD_ERR_INVALIDPARAMETER;
	}

	if( (packet_info->_.header->packet_type != UhipPacketType_UDP_PACKET_BRIDGE) ||
		(packet_info->packet_length_bytes < sizeof(uhip_udp_packet_bridge_t)) )
	{
		return AUD_ERR_INVALIDDATA;
	}

	uhip_udp_packet_bridge_t* udp_pb_packet = (uhip_udp_packet_bridge_t*)packet_info->_.buf32;
	if(out_channel)
	{
		*out_channel = (UhipUDPPacketBridgeChannel)(ntohs(udp_pb_packet->ch_id));
	}
	if(out_port)
	{
		*out_port = ntohs(udp_pb_packet->port);
	}
	if(out_ip)
	{
		*out_ip = ntohl(udp_pb_packet->ip);
	}

	payload_length = packet_info->packet_length_bytes - sizeof(uhip_udp_packet_bridge_t);
	if(out_payload)
	{
		if(payload_length)
		{
			*out_payload = (uint8_t *)packet_info->_.buf8 + sizeof(uhip_udp_packet_bridge_t);
		}
		else
		{
			*out_payload = NULL;
		}
	}

	if(out_payload_length)
	{
		*out_payload_length = payload_length;
	}

	return AUD_SUCCESS;
}

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
)
{
	uint16_t payload_length = 0;

	if (!packet_info)
	{
		return AUD_ERR_INVALIDPARAMETER;
	}

	if( (packet_info->_.header->packet_type != UhipPacketType_DANTE_EVENTS) ||
		(packet_info->packet_length_bytes <= sizeof(uhip_header_t)) )
	{
		return AUD_ERR_INVALIDDATA;
	}

	payload_length = packet_info->packet_length_bytes - sizeof(uhip_header_t);
	if(out_payload)
	{
		if(payload_length)
		{
			*out_payload = (uint8_t *)packet_info->_.buf8 + sizeof(uhip_header_t);
		}
		else
		{
			*out_payload = NULL;
		}
	}

	if(out_payload_length)
	{
		*out_payload_length = payload_length;
	}

	return AUD_SUCCESS;
}

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
)
{
	uint16_t payload_length = 0;

	if (!packet_info)
	{
		return AUD_ERR_INVALIDPARAMETER;
	}

	if( (packet_info->_.header->packet_type != UhipPacketType_DDP) ||
		(packet_info->packet_length_bytes <= sizeof(uhip_header_t)) )
	{
		return AUD_ERR_INVALIDDATA;
	}

	payload_length = packet_info->packet_length_bytes - sizeof(uhip_header_t);
	if(out_payload)
	{
		if(payload_length)
		{
			*out_payload = (uint8_t *)packet_info->_.buf8 + sizeof(uhip_header_t);
		}
		else
		{
			*out_payload = NULL;
		}
	}

	if(out_payload_length)
	{
		*out_payload_length = payload_length;
	}

	return AUD_SUCCESS;
}

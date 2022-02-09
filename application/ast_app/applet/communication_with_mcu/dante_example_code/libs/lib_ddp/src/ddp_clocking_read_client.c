/*
* File     : ddp_clocking_read_client.c
* Created  : August 2014
* Updated  : Date: 2014/08/22
* Author   : Michael Ung <michael.ung@audinate.com>
* Synopsis : All clocking DDP client read helper functions.
*
* Copyright 2005-2017, Audinate Pty Ltd and/or its licensors
Confidential
* Audinate Copyright Header Version 1
*/

#include "ddp/clocking_client.h"
#include "ddp/clocking_structures.h"

#if defined(AUD_PLATFORM_ULTIMO)

aud_error_t
ddp_read_clock_basic_legacy_response
(
	const ddp_packet_read_info_t * packet_info,
	uint16_t offset,
	ddp_request_id_t * out_request_id,
	ddp_status_t * out_status,
	uint32_t* out_capability_flags,
	uint8_t* out_clock_state,
	uint8_t* out_mute_state,
	uint8_t* out_ext_wc_state,
	uint8_t* out_preferred,
	uint32_t* out_drift
)
{
	aud_error_t result;
	ddp_message_read_info_t message_info;
	ddp_clock_basic_legacy_response_t * header;
	uint8_t dir = ddp_packet_read_get_direction(packet_info);
	ddp_opcode_t expected_opcode = DDP_OP_CLOCK_BASIC_LEGACY;

	if (!packet_info || offset >= packet_info->packet_length_bytes)
	{
		return AUD_ERR_INVALIDPARAMETER;
	}
	result = ddp_packet_read_message(packet_info, &message_info, offset);
	if (result != AUD_SUCCESS)
	{
		return result;
	}
	if(dir == DDP_PACKET_HEADER_FLAG_DIRECTION_REQUEST)
	{
		expected_opcode = DDP_OP_CLOCK_BASIC_LEGACY | DDP_OP_RESERVED_BIT_01;
	}
	result = ddp_packet_validate_message(&message_info, expected_opcode, sizeof(ddp_clock_basic_legacy_response_t));
	if (result != AUD_SUCCESS)
	{
		return result;
	}
	header = (ddp_clock_basic_legacy_response_t *)message_info.header.header;

	if (out_request_id)
	{
		*out_request_id = ntohs(header->subheader.request_id);
	}
	if (out_status)
	{
		*out_status = ntohs(header->subheader.status);
	}

	if (out_capability_flags)
	{
		*out_capability_flags = ntohl(header->payload.capability_flags);
	}
	if(out_clock_state)
	{
		*out_clock_state = header->payload.clock_state_legacy;
	}
	if(out_mute_state)
	{
		*out_mute_state = header->payload.mute_state;
	}
	if(out_ext_wc_state)
	{
		*out_ext_wc_state = header->payload.ext_wc_state;
	}
	if(out_preferred)
	{
		*out_preferred = header->payload.preferred;
	}
	if(out_drift)
	{
		*out_drift = ntohl(header->payload.drift);
	}

	return AUD_SUCCESS;
}

#endif // defined(AUD_PLATFORM_ULTIMO)

aud_error_t
ddp_read_clock_config_response
(
	const ddp_packet_read_info_t * packet_info,
	uint16_t offset,
	ddp_message_read_info_t * out_message_info,
	ddp_request_id_t * out_request_id,
	ddp_status_t * out_status,
	ddp_clock_config_control_params_t * out_config_params,
	uint16_t * out_num_clock_ports
)
{

	aud_error_t result;
	ddp_clock_config_response_t * header;
	uint16_t valid_flags;
	uint8_t dir = ddp_packet_read_get_direction(packet_info);
	ddp_opcode_t expected_opcode = DDP_OP_CLOCK_CONFIG;

	if (!packet_info || offset >= packet_info->packet_length_bytes)
	{
		return AUD_ERR_INVALIDPARAMETER;
	}
	result = ddp_packet_read_message(packet_info, out_message_info, offset);
	if (result != AUD_SUCCESS)
	{
		return result;
	}
	if(dir == DDP_PACKET_HEADER_FLAG_DIRECTION_REQUEST)
	{
		expected_opcode = DDP_OP_CLOCK_CONFIG | DDP_OP_RESERVED_BIT_01;
	}
	result = ddp_packet_validate_message(out_message_info, expected_opcode, sizeof(ddp_clock_config_response_t));
	if (result != AUD_SUCCESS)
	{
		return result;
	}
	header = (ddp_clock_config_response_t *)out_message_info->header.header;

	if (out_request_id)
	{
		*out_request_id = ntohs(header->subheader.request_id);
	}
	if (out_status)
	{
		*out_status = ntohs(header->subheader.status);
	}

	valid_flags = ntohs(header->payload.valid_flags);
	if (out_config_params)
	{
		if (valid_flags & CLOCK_CONFIG_RSP_PREFERRED_VALID)
		{
			out_config_params->preferred = header->payload.preferred;
		}
		if (valid_flags & CLOCK_CONFIG_RSP_WC_SYNC_VALID)
		{
			out_config_params->ext_word_clock_sync = header->payload.word_clock_sync;
		}
		if (valid_flags & CLOCK_CONFIG_RSP_LOGGING_VALID)
		{
			out_config_params->logging = header->payload.logging;
		}
		if (valid_flags & CLOCK_CONFIG_RSP_MULTICAST_VALID)
		{
			out_config_params->multicast = header->payload.multicast;
		}
		if (valid_flags & CLOCK_CONFIG_RSP_SLAVE_ONLY_VALID)
		{
			out_config_params->slave_only = header->payload.slave_only;
		}
		if (valid_flags & CLOCK_CONFIG_RSP_CLOCK_PROTOCOL_VALID)
		{
			out_config_params->clock_protocol = header->payload.clock_protocol;
			out_config_params->clock_protocol_mask = header->payload.clock_protocol_mask;
		}
		if (valid_flags & CLOCK_CONFIG_RSP_UNICAST_DELAY_CLOCK_PROTOCOL_VALID)
		{
			out_config_params->unicast_delay_clock_protocol = header->payload.unicast_delay_clock_protocol;
			out_config_params->unicast_delay_clock_protocol_mask = header->payload.unicast_delay_clock_protocol_mask;
		}
	}

	if (out_num_clock_ports)
	{
		*out_num_clock_ports = ntohs(header->payload.num_clock_port_structures);
	}

	return AUD_SUCCESS;
}

aud_error_t
ddp_read_clock_config_response_port
(
	const ddp_message_read_info_t * message_info,
	uint16_t port_idx,
	ddp_clock_config_port_params_t * out_port_params
)
{
	ddp_heap_read_info_t heap_info;
	aud_error_t result;
	ddp_clock_config_response_t * header;
	ddp_clock_config_port_t * clock_port;

	header = (ddp_clock_config_response_t *)message_info->header.header;

	if (port_idx >= ntohs(header->payload.num_clock_port_structures))
	{
		return AUD_ERR_INVALIDPARAMETER;
	}

	result = ddp_read_heap_arraystruct
				(
					message_info,
					&heap_info,
					ntohs(header->payload.clock_port_structs_offset),
					ntohs(header->payload.num_clock_port_structures),
					ntohs(header->payload.clock_port_structure_size)
				);

	if (result != AUD_SUCCESS)
	{
		return result;
	}

	clock_port = ((ddp_clock_config_port_t *)heap_info._.array32) + port_idx;

	out_port_params->port_id = ntohs(clock_port->port_id);
	out_port_params->enable_port = clock_port->enable_port;

	return AUD_SUCCESS;
}

aud_error_t
ddp_read_clock_pullup_response
(
	const ddp_packet_read_info_t * packet_info,
	uint16_t offset,
	ddp_request_id_t * out_request_id,
	ddp_status_t * out_status,
	uint8_t * out_current_pullup,
	uint8_t * out_reboot_pullup,
	const uint8_t ** out_current_subdomain,
	uint16_t * out_current_subdomain_length,
	const uint8_t ** out_reboot_subdomain,
	uint16_t* out_reboot_subdomain_length,
	const clock_supported_pullup_t ** out_supported_pullups,
	uint16_t * out_num_supported_pullups
)
{
	aud_error_t result;
	ddp_message_read_info_t message_info;
	ddp_clock_pullup_response_t * header;
	uint16_t valid_flags;
	ddp_heap_read_info_t heap_info;

	if (!packet_info || offset >= packet_info->packet_length_bytes)
	{
		return AUD_ERR_INVALIDPARAMETER;
	}
	result = ddp_packet_read_message(packet_info, &message_info, offset);
	if (result != AUD_SUCCESS)
	{
		return result;
	}
	result = ddp_packet_validate_message(&message_info, DDP_OP_CLOCK_PULLUP, sizeof(ddp_clock_pullup_response_t));
	if (result != AUD_SUCCESS)
	{
		return result;
	}
	header = (ddp_clock_pullup_response_t *)message_info.header.header;

	if (out_request_id)
	{
		*out_request_id = ntohs(header->subheader.request_id);
	}
	if (out_status)
	{
		*out_status = ntohs(header->subheader.status);
	}

	//get the valid flags
	valid_flags = ntohs(header->payload.valid_flags);

	if (out_current_pullup && (valid_flags & CLOCK_PULLUP_CURR_PULLUP_VALID) )
	{
		*out_current_pullup = header->payload.current_pullup;
	}

	if (out_reboot_pullup && (valid_flags & CLOCK_PULLUP_REBOOT_PULLUP_VALID) )
	{
		*out_reboot_pullup = header->payload.reboot_pullup;
	}
	if (out_current_subdomain && out_current_subdomain_length && (valid_flags & CLOCK_PULLUP_CURR_SUBDOMAIN_VALID))
	{
		if
		(
			ddp_read_heap_array8
			(
				&message_info,
				&heap_info,
				ntohs(header->payload.current_subdomain_offset),
				ntohs(header->payload.current_subdomain_size)
			) == AUD_SUCCESS
		)
		{
			*out_current_subdomain = heap_info._.array8;
			*out_current_subdomain_length = heap_info.length_bytes;
		}
		else
		{
			*out_current_subdomain = NULL;
			*out_current_subdomain_length = 0;
		}
	}

	if (out_reboot_subdomain && out_reboot_subdomain_length && (valid_flags & CLOCK_PULLUP_REBOOT_SUBDOMAIN_VALID))
	{
		if
		(
			ddp_read_heap_array8
			(
				&message_info,
				&heap_info,
				ntohs(header->payload.reboot_subdomain_offset),
				ntohs(header->payload.reboot_subdomain_size)
			) == AUD_SUCCESS
		)
		{
			*out_reboot_subdomain = heap_info._.array8;
			*out_reboot_subdomain_length = heap_info.length_bytes;
		}
		else
		{
			*out_reboot_subdomain = NULL;
			*out_reboot_subdomain_length = 0;
		}
	}

	if (out_supported_pullups && out_num_supported_pullups && (valid_flags & CLOCK_PULLUP_SUPPORTED_PULLUPS_VALID))
	{
		//check that the sizes match
		if(sizeof(clock_supported_pullup_t) != ntohs(header->payload.supported_pullups_size))
		{
			return AUD_ERR_INVALIDDATA;
		}

		if
		(
			ddp_read_heap_arraystruct
			(
				&message_info,
				&heap_info,
				ntohs(header->payload.supported_pullups_offset),
				ntohs(header->payload.num_supported_pullups),
				ntohs(header->payload.supported_pullups_size)
			) == AUD_SUCCESS
		)
		{
			*out_supported_pullups = (clock_supported_pullup_t*)heap_info._.array16;
			*out_num_supported_pullups = heap_info.num_elements;
		}
		else
		{
			*out_supported_pullups = NULL;
			*out_num_supported_pullups = 0;
		}
	}

	return AUD_SUCCESS;
}

aud_error_t
ddp_read_clock_basic2_response_header
(
	const ddp_packet_read_info_t * packet_info,
	uint16_t offset,
	ddp_message_read_info_t * out_message_info,
	ddp_request_id_t * out_request_id,
	ddp_status_t * out_status,
	uint32_t * out_capability_flags,
	uint8_t * out_clock_source,
	uint8_t * out_clock_state,
	uint8_t * out_servo_state,
	uint8_t * out_preferred,
	uint8_t * out_mute_state,
	uint8_t * out_ext_wc_state,
	uint8_t * out_clock_stratum,
	uint32_t * out_drift,
	uint32_t * out_max_drift,
	uint8_t const ** out_uuid,
	uint8_t const ** out_master_uuid,
	uint8_t const ** out_grandmaster_uuid,
	uint8_t * out_num_ports,
	uint8_t * out_port_size
)
{
	aud_error_t result;
	ddp_clock_basic2_response_t * header;
	ddp_heap_read_info_t heap_info;

	if (!packet_info || offset >= packet_info->packet_length_bytes)
	{
		return AUD_ERR_INVALIDPARAMETER;
	}
	result = ddp_packet_read_message(packet_info, out_message_info, offset);
	if (result != AUD_SUCCESS)
	{
		return result;
	}

	uint8_t dir = ddp_packet_read_get_direction(packet_info);
	ddp_opcode_t expected_opcode = DDP_OP_CLOCK_BASIC2;
	if(dir == DDP_PACKET_HEADER_FLAG_DIRECTION_REQUEST)
	{
		expected_opcode = DDP_OP_CLOCK_BASIC2 | DDP_OP_RESERVED_BIT_01;
	}
	result = ddp_packet_validate_message(out_message_info, expected_opcode, sizeof(ddp_clock_basic2_response_t));
	if (result != AUD_SUCCESS)
	{
		return result;
	}
	header = (ddp_clock_basic2_response_t *)out_message_info->header.header;

	if (out_request_id)
	{
		*out_request_id = ntohs(header->subheader.request_id);
	}
	if (out_status)
	{
		*out_status = ntohs(header->subheader.status);
	}
	if (out_capability_flags)
	{
		*out_capability_flags = ntohl(header->payload.capability_flags);
	}
	if (out_clock_source)
	{
		*out_clock_source = header->payload.clock_source;
	}
	if (out_clock_state)
	{
		*out_clock_state = header->payload.clock_state;
	}
	if (out_servo_state)
	{
		*out_servo_state = header->payload.servo_state;
	}
	if (out_preferred)
	{
		*out_preferred = header->payload.preferred;
	}
	if (out_mute_state)
	{
		*out_mute_state = header->payload.mute_state;
	}
	if (out_ext_wc_state)
	{
		*out_ext_wc_state = header->payload.ext_wc_state;
	}
	if (out_clock_stratum)
	{
		*out_clock_stratum = header->payload.clock_stratum;
	}
	if (out_drift)
	{
		*out_drift = ntohl(header->payload.drift);
	}
	if (out_max_drift)
	{
		*out_max_drift = ntohl(header->payload.max_drift);
	}
	if (out_uuid)
	{
		if (
			ddp_read_heap_array8(
				out_message_info,
				&heap_info,
				ntohs(header->payload.uuid_offset),
				header->payload.uuid_size)
			== AUD_SUCCESS
			)
		{
			*out_uuid = heap_info._.array8;
		}
		else
		{
			*out_uuid = NULL;
		}
	}
	if (out_master_uuid)
	{
		if (
			ddp_read_heap_array8(
				out_message_info,
				&heap_info,
				ntohs(header->payload.master_uuid_offset),
				header->payload.master_uuid_size)
			== AUD_SUCCESS
			)
		{
			*out_master_uuid = heap_info._.array8;
		}
		else
		{
			*out_master_uuid = NULL;
		}
	}
	if (out_grandmaster_uuid)
	{
		if (
			ddp_read_heap_array8(
				out_message_info,
				&heap_info,
				ntohs(header->payload.grandmaster_uuid_offset),
				header->payload.grandmaster_uuid_size)
			== AUD_SUCCESS
			)
		{
			*out_grandmaster_uuid = heap_info._.array8;
		}
		else
		{
			*out_grandmaster_uuid = NULL;
		}
	}
	if (out_num_ports)
	{
		*out_num_ports = header->payload.num_ports;
	}
	if (out_port_size)
	{
		*out_port_size = header->payload.port_size;
	}

	return AUD_SUCCESS;
}

aud_error_t
ddp_read_clock_basic2_response_port
(
	const ddp_message_read_info_t * message_info,
	uint8_t port_idx,
	ddp_clock_port_t ** out_port
)
{
	aud_error_t result;
	ddp_clock_basic2_response_t * header;
	ddp_heap_read_info_t heap_info;

	if (!message_info)
	{
		return AUD_ERR_INVALIDPARAMETER;
	}

	header = (ddp_clock_basic2_response_t *)message_info->header.header;

	if (port_idx >= header->payload.num_ports)
	{
		return AUD_ERR_INVALIDPARAMETER;
	}

	result = ddp_read_heap_arraystruct(message_info,
		&heap_info,
		ntohs(header->payload.port_array_offset),
		header->payload.num_ports,
		header->payload.port_size);

	if (result != AUD_SUCCESS)
	{
		return result;
	}

	*out_port = ((ddp_clock_port_t *)heap_info._.array32) + port_idx;

	(*out_port)->flags = ntohs((*out_port)->flags);
	(*out_port)->port_id = ntohs((*out_port)->port_id);

	return AUD_SUCCESS;
}

/*
* File     : ddp_clocking_write_client.c
* Created  : August 2014
* Updated  : Date: 2014/08/22
* Author   : Michael Ung <michael.ung@audinate.com>
* Synopsis : All clocking DDP client write helper functions.
*
* Copyright 2005-2017, Audinate Pty Ltd and/or its licensors
Confidential
* Audinate Copyright Header Version 1
*/

#include "ddp/clocking_client.h"
#include "ddp/clocking_structures.h"

/**
* @brief Clock pullup subdomain values
*/
static char const * clock_pullup_subdomains[] = { "_DFLT", "_ALT1", "_ALT2", "_ALT3", "_ALT4" };

#if defined(AUD_PLATFORM_ULTIMO)

aud_error_t
ddp_add_clock_basic_legacy_request
(
	ddp_packet_write_info_t * packet_info,
	ddp_request_id_t request_id
)
{
	aud_error_t result;
	ddp_message_write_info_t message_info;

	if (!packet_info)
	{
		return AUD_ERR_INVALIDPARAMETER;
	}

	result = ddp_packet_allocate_request
			(
				packet_info, &message_info,
				DDP_OP_CLOCK_BASIC_LEGACY,
				request_id,
				0
			);
	return result;
}

#endif // defined(AUD_PLATFORM_ULTIMO)

aud_error_t
ddp_add_clock_config_request
(
	ddp_packet_write_info_t * packet_info,
	ddp_request_id_t request_id,
	ddp_clock_config_control_params_t * config_params,
	uint16_t num_clock_ports,
	ddp_clock_config_port_params_t * port_params
)
{
	aud_error_t result;
	ddp_message_write_info_t message_info;
	ddp_clock_config_request_t * raw_header;
	ddp_heap_write_info_t heap_info;
	uint16_t control_flags = 0, i;
	ddp_clock_config_port_t * clock_port;

	if (!packet_info)
	{
		return AUD_ERR_INVALIDPARAMETER;
	}

	result = ddp_packet_allocate_request
			(
				packet_info,
				&message_info,
				DDP_OP_CLOCK_CONFIG,
				request_id,
				sizeof(raw_header->payload)
			);
	if (result != AUD_SUCCESS)
	{
		return result;
	}

	raw_header = (ddp_clock_config_request_t *)message_info._.buf32;

	if (config_params)
	{
		if (config_params->control_flags & CONTROL_FLAG_SET_PREFERRED)
		{
			raw_header->payload.preferred = config_params->preferred;
			control_flags |= CONTROL_FLAG_SET_PREFERRED;
		}

		if (config_params->control_flags & CONTROL_FLAG_SET_SLAVE_TO_WC)
		{
			raw_header->payload.ext_word_clock_sync = config_params->ext_word_clock_sync;
			control_flags |= CONTROL_FLAG_SET_SLAVE_TO_WC;
		}

		if (config_params->control_flags & CONTROL_FLAGS_SET_LOGGING)
		{
			raw_header->payload.logging = config_params->logging;
			control_flags |= CONTROL_FLAGS_SET_LOGGING;
		}

		if (config_params->control_flags & CONTROL_FLAGS_SET_MULTICAST)
		{
			raw_header->payload.multicast = config_params->multicast;
			control_flags |= CONTROL_FLAGS_SET_MULTICAST;
		}

		if (config_params->control_flags & CONTROL_FLAGS_SET_SLAVE_ONLY)
		{
			raw_header->payload.slave_only = config_params->slave_only;
			control_flags |= CONTROL_FLAGS_SET_SLAVE_ONLY;
		}

		if (config_params->control_flags & CONTROL_FLAGS_SET_CLOCK_PROTOCOL)
		{
			raw_header->payload.clock_protocol = config_params->clock_protocol;
			raw_header->payload.clock_protocol_mask = config_params->clock_protocol_mask;
			control_flags |= CONTROL_FLAGS_SET_CLOCK_PROTOCOL;
		}

		if (config_params->control_flags & CONTROL_FLAGS_SET_UNICAST_DELAY_CLOCK_PROTOCOL)
		{
			raw_header->payload.unicast_delay_clock_protocol = config_params->unicast_delay_clock_protocol;
			raw_header->payload.unicast_delay_clock_protocol_mask = config_params->unicast_delay_clock_protocol_mask;
			control_flags |= CONTROL_FLAGS_SET_UNICAST_DELAY_CLOCK_PROTOCOL;
		}
	}

	raw_header->payload.pad0 = 0;
	raw_header->payload.pad1 = 0;

	raw_header->payload.control_flags = htons(control_flags);

	if (num_clock_ports != 0 && port_params)
	{
		result = ddp_allocate_heap_arraystruct
					(
						&message_info,
						&heap_info,
						num_clock_ports,
						sizeof(ddp_clock_config_port_t)
					);
		if (result != AUD_SUCCESS)
		{
			return result;
		}
		raw_header->payload.clock_port_structs_offset = heap_info.msg_offset_bytes;
		raw_header->payload.num_clock_port_structures = htons(num_clock_ports);
		raw_header->payload.clock_port_structure_size = htons(sizeof(ddp_clock_config_port_t));

		clock_port = (ddp_clock_config_port_t *)heap_info._.array32;
		for (i = 0; i < num_clock_ports; ++i)
		{
			clock_port[i].port_id = htons(port_params[i].port_id);
			clock_port[i].enable_port = port_params[i].enable_port;
		}
	}
	else
	{
		raw_header->payload.num_clock_port_structures = 0;
	}

	return AUD_SUCCESS;
}

aud_error_t
ddp_add_clock_pullup_request
(
	ddp_packet_write_info_t * packet_info,
	ddp_request_id_t request_id,
	clock_pullup_control_fields_t* control_fields
)
{
	aud_error_t result;
	ddp_message_write_info_t message_info;
	ddp_clock_pullup_request_t * raw_header;
	uint16_t control_flags = 0;

	if (!packet_info)
	{
		return AUD_ERR_INVALIDPARAMETER;
	}

	result = ddp_packet_allocate_request
			(
				packet_info,
				&message_info,
				DDP_OP_CLOCK_PULLUP,
				request_id,
				sizeof(raw_header->payload)
			);
	if (result != AUD_SUCCESS)
	{
		return result;
	}

	raw_header = (ddp_clock_pullup_request_t *)message_info._.buf32;

	if(control_fields)
	{
		if (control_fields->valid_flags & CLOCK_PULLUP_SET_PULLUP)
		{
			if (control_fields->pullup > CLOCK_PULLUP_NEG_4_0_PCENT4) {
				return AUD_ERR_INVALIDPARAMETER;
			}

			raw_header->payload.pullup = control_fields->pullup;
			control_flags |= CLOCK_PULLUP_SET_PULLUP;

			if (!ddp_message_allocate_string(
				&message_info,
				&raw_header->payload.subdomain_offset,
				clock_pullup_subdomains[control_fields->pullup]
				))
			{
				return AUD_ERR_NOMEMORY;
			}

			raw_header->payload.subdomain_size = htons((uint16_t)strlen(clock_pullup_subdomains[control_fields->pullup]));
			control_flags |= CLOCK_PULLUP_SET_SUBDOMAIN;
		}
		else
		{
			raw_header->payload.subdomain_offset = 0;
			raw_header->payload.subdomain_size = 0;
		}
	}

	raw_header->payload.pad1 = 0;
	raw_header->payload.control_flags = htons(control_flags);

	return AUD_SUCCESS;
}

aud_error_t
ddp_add_clock_basic2_request
(
	ddp_packet_write_info_t * packet_info,
	ddp_request_id_t request_id
)
{
	aud_error_t result;
	ddp_message_write_info_t message_info;

	if (!packet_info)
	{
		return AUD_ERR_INVALIDPARAMETER;
	}

	result = ddp_packet_allocate_request
		(
			packet_info, &message_info,
			DDP_OP_CLOCK_BASIC2,
			request_id,
			0
		);

	return result;
}

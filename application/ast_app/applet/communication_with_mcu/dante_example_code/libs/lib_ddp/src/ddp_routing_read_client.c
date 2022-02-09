/*
* File     : ddp_routing_read_client.c
* Created  : September 2014
* Author   : Chamira Perera <cperera@audinate.com>
* Synopsis : All routing DDP client read helper functions.
*
* Copyright 2005-2017, Audinate Pty Ltd and/or its licensors
Confidential
* Audinate Copyright Header Version 1
*/

#include "ddp/routing_client.h"
#include "ddp/routing_structures.h"
#include "ddp/ddp_shared.h"
#include "ddp/routing_support.h"

aud_error_t
ddp_read_routing_basic_response
(
	const ddp_packet_read_info_t * packet_info,
	uint16_t offset,
	ddp_request_id_t * out_request_id,
	ddp_status_t * out_status,
	ddp_routing_basic_response_payload_t *out_rsp_payload
)
{
	aud_error_t result;
	ddp_message_read_info_t message_info;
	ddp_routing_basic_response_t * header;

	if (!packet_info || (offset >= packet_info->packet_length_bytes))
	{
		return AUD_ERR_INVALIDPARAMETER;
	}
	result = ddp_packet_read_message(packet_info, &message_info, offset);
	if (result != AUD_SUCCESS)
	{
		return result;
	}
	result = ddp_packet_validate_message(&message_info, DDP_OP_ROUTING_BASIC, sizeof(ddp_routing_basic_response_t));
	if (result != AUD_SUCCESS)
	{
		return result;
	}
	header = (ddp_routing_basic_response_t *)message_info.header.header;

	if (out_request_id)
	{
		*out_request_id = ntohs(header->subheader.request_id);
	}
	if (out_status)
	{
		*out_status = ntohs(header->subheader.status);
	}
	if (out_rsp_payload)
	{
		out_rsp_payload->cap_flags = 0;
		out_rsp_payload->num_intf = ntohs(header->payload.num_intf);

		out_rsp_payload->max_rx_flows = ntohs(header->payload.max_rx_flows);
		out_rsp_payload->max_tx_flows = ntohs(header->payload.max_tx_flows);
		out_rsp_payload->max_rx_slots = ntohs(header->payload.max_rx_slots);
		out_rsp_payload->max_tx_slots = ntohs(header->payload.max_tx_slots);

		out_rsp_payload->min_rx_latency = ntohl(header->payload.min_rx_latency);
		out_rsp_payload->max_rx_latency = ntohl(header->payload.max_rx_latency);

		out_rsp_payload->min_rx_fpp = ntohs(header->payload.min_rx_fpp);
		out_rsp_payload->max_rx_fpp = 0;
		out_rsp_payload->min_tx_fpp = ntohs(header->payload.min_tx_fpp);
		out_rsp_payload->max_tx_fpp = ntohs(header->payload.max_tx_fpp);

		out_rsp_payload->min_rx_dynamic_port = ntohs(header->payload.min_rx_dynamic_port);
		out_rsp_payload->max_rx_dynamic_port = ntohs(header->payload.max_rx_dynamic_port);

		out_rsp_payload->min_rx_static_port = ntohs(header->payload.min_rx_static_port);
		out_rsp_payload->max_rx_static_port = ntohs(header->payload.max_rx_static_port);

		out_rsp_payload->max_tx_label_per_channel = ntohs(header->payload.max_tx_label_per_channel);
		out_rsp_payload->max_tx_label_total = ntohs(header->payload.max_tx_label_total);

		out_rsp_payload->rx_chan_state_per_ddp_msg = header->payload.rx_chan_state_per_ddp_msg;
		out_rsp_payload->tx_chan_state_per_ddp_msg = header->payload.tx_chan_state_per_ddp_msg;
		out_rsp_payload->rx_flow_state_per_ddp_msg = header->payload.rx_flow_state_per_ddp_msg;
		out_rsp_payload->tx_flow_state_per_ddp_msg = header->payload.tx_flow_state_per_ddp_msg;
		out_rsp_payload->rx_chan_status_per_ddp_msg = header->payload.rx_chan_status_per_ddp_msg;
		out_rsp_payload->rx_flow_status_per_ddp_msg = header->payload.rx_flow_status_per_ddp_msg;
	}	

	return AUD_SUCCESS;
}

aud_error_t
ddp_read_routing_ready_state_response
(
	const ddp_packet_read_info_t * packet_info,
	uint16_t offset,
	ddp_request_id_t * out_request_id,
	ddp_status_t * out_status,
	uint8_t * out_network_ready_state,
	uint8_t * out_routing_ready_state
)
{
	aud_error_t result;
	ddp_message_read_info_t message_info;
	ddp_routing_ready_state_response_t * header;

	if (!packet_info || (offset >= packet_info->packet_length_bytes))
	{
		return AUD_ERR_INVALIDPARAMETER;
	}
	result = ddp_packet_read_message(packet_info, &message_info, offset);
	if (result != AUD_SUCCESS)
	{
		return result;
	}
	result = ddp_packet_validate_message(&message_info, DDP_OP_ROUTING_READY_STATE, sizeof(ddp_routing_ready_state_response_t));
	if (result != AUD_SUCCESS)
	{
		return result;
	}
	header = (ddp_routing_ready_state_response_t *)message_info.header.header;

	if (out_request_id)
	{
		*out_request_id = ntohs(header->subheader.request_id);
	}
	if (out_status)
	{
		*out_status = ntohs(header->subheader.status);
	}
	if (out_network_ready_state)
	{
		*out_network_ready_state = header->payload.network_ready_state;
	}
	if (out_routing_ready_state)
	{
		*out_routing_ready_state = header->payload.routing_ready_state;
	}

	return AUD_SUCCESS;
}

aud_error_t
ddp_read_routing_performance_config_response
(
	const ddp_packet_read_info_t * packet_info,
	uint16_t offset,
	ddp_request_id_t * out_request_id,
	ddp_status_t * out_status,
	uint16_t * out_unicast_fpp,
	uint32_t * out_unicast_latency,
	uint16_t * out_multicast_fpp,
	uint32_t * out_multicast_latency
)
{
	aud_error_t result;
	ddp_message_read_info_t message_info;
	ddp_routing_performance_config_response_t * header;
	uint16_t element_flags, unicast_flags, multicast_flags;

	if (!packet_info || (offset >= packet_info->packet_length_bytes))
	{
		return AUD_ERR_INVALIDPARAMETER;
	}
	result = ddp_packet_read_message(packet_info, &message_info, offset);
	if (result != AUD_SUCCESS)
	{
		return result;
	}
	result = ddp_packet_validate_message(&message_info, DDP_OP_ROUTING_PERFORMANCE_CONFIG, sizeof(ddp_routing_performance_config_response_t));
	if (result != AUD_SUCCESS)
	{
		return result;
	}
	header = (ddp_routing_performance_config_response_t *)message_info.header.header;

	if (out_request_id)
	{
		*out_request_id = ntohs(header->subheader.request_id);
	}
	if (out_status)
	{
		*out_status = ntohs(header->subheader.status);
	}

	element_flags = ntohs(header->payload.element_flags);
	if (element_flags & DDP_ROUTING_PERFORMANCE_FLAG__UNICAST)
	{
		unicast_flags = ntohs(header->payload.unicast.flags);
		if (out_unicast_fpp)
		{
			if (unicast_flags & DDP_ROUTING_PERFORMANCE_VALID_FLAG__FPP)
			{
				*out_unicast_fpp = ntohs(header->payload.unicast.fpp);
			}
			else {
				*out_unicast_fpp = 0;
			}
		}
		if (out_unicast_latency)
		{
			if (unicast_flags & DDP_ROUTING_PERFORMANCE_VALID_FLAG__LATENCY)
			{
				*out_unicast_latency = (ntohl(header->payload.unicast.latency) / 1000);
			}
			else {
				*out_unicast_latency = 0;
			}
		}
	}
	else
	{
		if (out_unicast_fpp) {
			*out_unicast_fpp = 0;
		}
		if (out_unicast_latency) {
			*out_unicast_latency = 0;
		}
	}

	if (element_flags & DDP_ROUTING_PERFORMANCE_FLAG__MULTICAST)
	{
		multicast_flags = ntohs(header->payload.multicast.flags);
		if (out_multicast_fpp)
		{
			if (multicast_flags & DDP_ROUTING_PERFORMANCE_VALID_FLAG__FPP)
			{
				*out_multicast_fpp = ntohs(header->payload.multicast.fpp);
			}
			else {
				*out_multicast_fpp = 0;
			}
		}
		if (out_multicast_latency)
		{
			if (multicast_flags & DDP_ROUTING_PERFORMANCE_VALID_FLAG__LATENCY)
			{
				*out_multicast_latency = (ntohl(header->payload.multicast.latency) / 1000);
			}
			else {
				*out_multicast_latency = 0;
			}
		}
	}
	else
	{
		if (out_multicast_fpp) {
			*out_multicast_fpp = 0;
		}
		if (out_multicast_latency) {
			*out_multicast_latency = 0;
		}
	}
	
	
	return AUD_SUCCESS;
}

aud_error_t
ddp_read_routing_rx_chan_config_state_response_header
(
	const ddp_packet_read_info_t * packet_info,
	uint16_t offset,
	ddp_message_read_info_t * out_message_info,
	ddp_request_id_t * out_request_id,
	ddp_status_t * out_status,
	uint16_t * out_num_chans
)
{
	aud_error_t result = AUD_SUCCESS;
	ddp_routing_rx_chan_config_state_response_t * header;

	if (!packet_info || (offset >= packet_info->packet_length_bytes) || !out_message_info)
	{
		return AUD_ERR_INVALIDPARAMETER;
	}
	result = ddp_packet_read_message(packet_info, out_message_info, offset);
	if (result != AUD_SUCCESS)
	{
		return result;
	}
	result = ddp_packet_validate_message(out_message_info, DDP_OP_ROUTING_RX_CHAN_CONFIG_STATE, sizeof(ddp_routing_rx_chan_config_state_response_t));
	if (result != AUD_SUCCESS)
	{
		return result;
	}
	header = (ddp_routing_rx_chan_config_state_response_t *)out_message_info->header.header;

	if (out_request_id)
	{
		*out_request_id = ntohs(header->subheader.request_id);
	}
	if (out_status)
	{
		*out_status = ntohs(header->subheader.status);
	}
	if (out_num_chans)
	{
		ddp_heap_read_info_t heap;
		result = ddp_offset_array_payload_init_read(out_message_info, &heap);
		if (result != AUD_SUCCESS) {
			return result;
		}

		*out_num_chans = heap.num_elements;
	}

	return AUD_SUCCESS;
}

aud_error_t
ddp_read_routing_rx_chan_config_state_response_chan_params
(
	ddp_message_read_info_t const * message_info,
	uint16_t chan_idx,
	ddp_rx_chan_state_params_t * out_rx_chan_data,
	uint16_t * out_num_custom_encodings
)
{
	return ddp_read_routing_rx_chan_state(message_info, chan_idx, out_rx_chan_data, out_num_custom_encodings);
}

aud_error_t
ddp_read_routing_tx_chan_config_state_response_header
(
	const ddp_packet_read_info_t * packet_info,
	uint16_t offset,
	ddp_message_read_info_t * out_message_info,
	ddp_request_id_t * out_request_id,
	ddp_status_t * out_status,
	uint16_t * out_num_chans
)
{
	aud_error_t result;
	ddp_routing_tx_chan_config_state_response_t * header;

	if (!packet_info || (offset >= packet_info->packet_length_bytes))
	{
		return AUD_ERR_INVALIDPARAMETER;
	}
	result = ddp_packet_read_message(packet_info, out_message_info, offset);
	if (result != AUD_SUCCESS)
	{
		return result;
	}
	result = ddp_packet_validate_message(out_message_info, DDP_OP_ROUTING_TX_CHAN_CONFIG_STATE, sizeof(ddp_routing_tx_chan_config_state_response_t));
	if (result != AUD_SUCCESS)
	{
		return result;
	}
	header = (ddp_routing_tx_chan_config_state_response_t *)out_message_info->header.header;

	if (out_request_id)
	{
		*out_request_id = ntohs(header->subheader.request_id);
	}
	if (out_status)
	{
		*out_status = ntohs(header->subheader.status);
	}

	if (out_num_chans)
	{
		ddp_heap_read_info_t heap;
		result = ddp_offset_array_payload_init_read(out_message_info, &heap);
		if (result != AUD_SUCCESS)
		{
			return result;
		}

		*out_num_chans = heap.num_elements;
	}

	return AUD_SUCCESS;
}

aud_error_t
ddp_read_routing_tx_chan_config_state_response_chan_params
(
	const ddp_message_read_info_t * message_info,
	uint16_t chan_idx,
	ddp_tx_chan_state_params_t * out_tx_chan_data,
	uint16_t * out_num_custom_encodings
)
{
	return ddp_read_routing_tx_chan_state(message_info, chan_idx, out_tx_chan_data, out_num_custom_encodings);
}

aud_error_t
ddp_read_routing_rx_chan_status_response_header
(
	const ddp_packet_read_info_t * packet_info,
	uint16_t offset,
	ddp_message_read_info_t * out_message_info,
	ddp_request_id_t * out_request_id,
	ddp_status_t * out_status,
	uint16_t * out_num_chans
)
{
	aud_error_t result;
	ddp_routing_rx_chan_status_response_t * header;

	if (!packet_info || offset >= packet_info->packet_length_bytes || !out_message_info)
	{
		return AUD_ERR_INVALIDPARAMETER;
	}
	result = ddp_packet_read_message(packet_info, out_message_info, offset);
	if (result != AUD_SUCCESS)
	{
		return result;
	}
	result = ddp_packet_validate_message(out_message_info, DDP_OP_ROUTING_RX_CHAN_STATUS, sizeof(ddp_routing_rx_chan_status_response_t));
	if (result != AUD_SUCCESS)
	{
		return result;
	}
	header = (ddp_routing_rx_chan_status_response_t *)out_message_info->header.header;

	if (out_request_id)
	{
		*out_request_id = ntohs(header->subheader.request_id);
	}
	if (out_status)
	{
		*out_status = ntohs(header->subheader.status);
	}

	if (out_num_chans)
	{
		uint16_t *payload;

		payload = (uint16_t *)out_message_info->payload.value16;
		*out_num_chans = ntohs(*payload);
	}

	return AUD_SUCCESS;
}

aud_error_t
ddp_read_routing_rx_chan_status_response_chan_params
(
	const ddp_message_read_info_t * message_info,
	uint16_t chan_idx,
	uint16_t * out_channel_id,
	uint16_t * out_status,
	uint8_t * out_avail,
	uint8_t * out_active
)
{
	uint16_t num_chans;
	uint16_t *payload;
	uint16_t status_array_offset;
	ddp_routing_rx_channel_status_t *rx_chan_status_ptr;

	if (!message_info)
	{
		return AUD_ERR_INVALIDPARAMETER;
	}

	payload = (uint16_t *)message_info->payload.value16;
	num_chans = ntohs(*payload);

	if (chan_idx >= num_chans)
	{
		return AUD_ERR_INVALIDPARAMETER;
	}

	payload += 2;

	status_array_offset = ntohs(*payload);
	rx_chan_status_ptr = (ddp_routing_rx_channel_status_t *)(message_info->header.value8 + status_array_offset);
	
	if (status_array_offset)
	{
		rx_chan_status_ptr += chan_idx;
		
		if (out_channel_id)
		{
			*out_channel_id = ntohs(rx_chan_status_ptr->channel_id);
		}
		if (out_status)
		{
			*out_status = ntohs(rx_chan_status_ptr->status);
		}
		if (out_avail)
		{
			*out_avail = rx_chan_status_ptr->avail;
		}
		if (out_active)
		{
			*out_active = rx_chan_status_ptr->active;
		}
	}
	else
	{
		return AUD_ERR_NODATA;
	}	

	return AUD_SUCCESS;
}

aud_error_t
ddp_read_routing_rx_chan_config_state_response_custom_encoding
(
	ddp_message_read_info_t const * message_info,
	uint16_t chan_idx,
	uint16_t custom_enc_idx,
	uint16_t * out_custom_encoding
)
{
	return ddp_read_routing_rx_chan_custom_encoding(message_info, chan_idx, custom_enc_idx, out_custom_encoding);
}

aud_error_t
ddp_read_routing_rx_flow_config_state_response_header
(
	const ddp_packet_read_info_t * packet_info,
	uint16_t offset,
	ddp_message_read_info_t * out_message_info,
	ddp_request_id_t * out_request_id,
	ddp_status_t * out_status,
	uint16_t * out_num_flows
)
{
	aud_error_t result;
	ddp_routing_rx_flow_config_state_response_t * header;

	if (!packet_info || (offset >= packet_info->packet_length_bytes))
	{
		return AUD_ERR_INVALIDPARAMETER;
	}
	result = ddp_packet_read_message(packet_info, out_message_info, offset);
	if (result != AUD_SUCCESS)
	{
		return result;
	}
	result = ddp_packet_validate_message(out_message_info, DDP_OP_ROUTING_RX_FLOW_CONFIG_STATE, sizeof(ddp_routing_rx_flow_config_state_response_t));
	if (result != AUD_SUCCESS)
	{
		return result;
	}
	header = (ddp_routing_rx_flow_config_state_response_t *)out_message_info->header.header;

	if (out_request_id)
	{
		*out_request_id = ntohs(header->subheader.request_id);
	}
	if (out_status)
	{
		*out_status = ntohs(header->subheader.status);
	}

	if (out_num_flows)
	{
		ddp_heap_read_info_t heap;
		result = ddp_offset_array_payload_init_read(out_message_info, &heap);
		if (result != AUD_SUCCESS)
		{
			return result;
		}

		*out_num_flows = heap.num_elements;
	}

	return AUD_SUCCESS;
}

aud_error_t
ddp_read_routing_rx_flow_config_state_response_flow_params
(
	const ddp_message_read_info_t * message_info,
	uint16_t flow_idx,
	uint16_t * out_num_slots,
	uint16_t * out_num_addresses,
	ddp_rx_flow_state_params_t * out_rx_flow
)
{
	return ddp_read_routing_rx_flow_state(message_info, flow_idx, out_num_slots, out_num_addresses, out_rx_flow);
}

aud_error_t
ddp_read_routing_rx_flow_config_state_response_flow_slot
(
	const ddp_message_read_info_t * message_info,
	uint16_t flow_idx,
	uint16_t slot_idx,
	uint16_t * out_num_chans
)
{
	return ddp_read_routing_rx_flow_state_slot(message_info, flow_idx, slot_idx, out_num_chans);
}

aud_error_t
ddp_read_routing_rx_flow_config_state_response_flow_slot_chans
(
	const ddp_message_read_info_t * message_info,
	uint16_t flow_idx,
	uint16_t slot_idx,
	uint16_t * out_chans
)
{
	return ddp_read_routing_rx_flow_state_slot_chans(message_info, flow_idx, slot_idx, out_chans);
}

aud_error_t
ddp_read_routing_rx_flow_config_state_response_flow_address_legacy
(
	const ddp_message_read_info_t * message_info,
	uint16_t flow_idx,
	uint16_t address_idx,
	uint32_t * out_ip_address,
	uint16_t * out_port
)
{
	aud_error_t result;
	ddp_ip_addr_nw_t ip_addr_nw;

	result = ddp_read_routing_rx_flow_config_state_response_flow_address_nw_ip(message_info, flow_idx, address_idx, &ip_addr_nw, out_port);
	*out_ip_address = ntohl(ip_addr_nw.ip_addr);

	return result;
}

aud_error_t
ddp_read_routing_rx_flow_config_state_response_flow_address_nw_ip
(
	const ddp_message_read_info_t * message_info,
	uint16_t flow_idx,
	uint16_t address_idx,
	ddp_ip_addr_nw_t * out_ip_address,
	uint16_t * out_port
)
{
	return ddp_read_routing_flow_state_address(message_info, flow_idx, address_idx, out_ip_address, out_port);
}

aud_error_t
ddp_read_routing_tx_flow_config_state_response_header
(
	const ddp_packet_read_info_t * packet_info,
	uint16_t offset,
	ddp_message_read_info_t * out_message_info,
	ddp_request_id_t * out_request_id,
	ddp_status_t * out_status,
	uint16_t * out_num_flows
)
{
	aud_error_t result;
	ddp_routing_tx_flow_config_state_response_t * header;

	if (!packet_info || (offset >= packet_info->packet_length_bytes))
	{
		return AUD_ERR_INVALIDPARAMETER;
	}
	result = ddp_packet_read_message(packet_info, out_message_info, offset);
	if (result != AUD_SUCCESS)
	{
		return result;
	}
	result = ddp_packet_validate_message(out_message_info, DDP_OP_ROUTING_TX_FLOW_CONFIG_STATE, sizeof(ddp_routing_tx_flow_config_state_response_t));
	if (result != AUD_SUCCESS)
	{
		return result;
	}
	header = (ddp_routing_tx_flow_config_state_response_t *)out_message_info->header.header;

	if (out_request_id)
	{
		*out_request_id = ntohs(header->subheader.request_id);
	}
	if (out_status)
	{
		*out_status = ntohs(header->subheader.status);
	}

	if (out_num_flows)
	{
		ddp_heap_read_info_t heap;
		result = ddp_offset_array_payload_init_read(out_message_info, &heap);
		if (result != AUD_SUCCESS)
		{
			return result;
		}

		*out_num_flows = heap.num_elements;
	}

	return AUD_SUCCESS;
}

aud_error_t
ddp_read_routing_tx_flow_config_state_response_flow_params
(
	const ddp_message_read_info_t * message_info,
	uint16_t flow_idx,
	uint16_t * out_num_slots,
	uint16_t * out_num_addresses,
	ddp_tx_flow_state_params_t * out_tx_flow
)
{
	return ddp_read_routing_tx_flow_state(message_info, flow_idx, out_num_slots, out_num_addresses, out_tx_flow);
}

aud_error_t
ddp_read_routing_tx_flow_config_state_response_flow_slots
(
	const ddp_message_read_info_t * message_info,
	uint16_t flow_idx,
	uint16_t * out_slots
)
{
	return ddp_read_routing_tx_flow_state_slots(message_info, flow_idx, out_slots);
}

aud_error_t
ddp_read_routing_tx_flow_config_state_response_flow_address_legacy
(
	const ddp_message_read_info_t * message_info,
	uint16_t flow_idx,
	uint16_t address_idx,
	uint32_t * out_ip_address,
	uint16_t * out_port
)
{
	aud_error_t result;
	ddp_ip_addr_nw_t ip_addr_nw_ip;

	result = ddp_read_routing_tx_flow_config_state_response_flow_address_nw_ip(message_info, flow_idx, address_idx, &ip_addr_nw_ip, out_port);
	*out_ip_address = ntohl(ip_addr_nw_ip.ip_addr);

	return result;
}

aud_error_t
ddp_read_routing_tx_flow_config_state_response_flow_address_nw_ip
(
	const ddp_message_read_info_t * message_info,
	uint16_t flow_idx,
	uint16_t address_idx,
	ddp_ip_addr_nw_t * out_ip_address,
	uint16_t * out_port
)
{
	return ddp_read_routing_flow_state_address(message_info, flow_idx, address_idx, out_ip_address, out_port);
}

aud_error_t
ddp_read_routing_rx_flow_status_response_header
(
	const ddp_packet_read_info_t * packet_info,
	uint16_t offset,
	ddp_message_read_info_t * out_message_info,
	ddp_request_id_t * out_request_id,
	ddp_status_t * out_status,
	uint16_t * out_num_flows
)
{
	aud_error_t result;
	ddp_routing_rx_flow_status_response_t * header;

	if (!packet_info || offset >= packet_info->packet_length_bytes || !out_message_info)
	{
		return AUD_ERR_INVALIDPARAMETER;
	}
	result = ddp_packet_read_message(packet_info, out_message_info, offset);
	if (result != AUD_SUCCESS)
	{
		return result;
	}
	result = ddp_packet_validate_message(out_message_info, DDP_OP_ROUTING_RX_FLOW_STATUS, sizeof(ddp_routing_rx_flow_status_response_t));
	if (result != AUD_SUCCESS)
	{
		return result;
	}
	header = (ddp_routing_rx_flow_status_response_t *)out_message_info->header.header;

	if (out_request_id)
	{
		*out_request_id = ntohs(header->subheader.request_id);
	}
	if (out_status)
	{
		*out_status = ntohs(header->subheader.status);
	}

	if (out_num_flows)
	{
		uint16_t *payload;

		payload = (uint16_t *)out_message_info->payload.value16;
		*out_num_flows = ntohs(*payload);
	}

	return AUD_SUCCESS;
}

aud_error_t
ddp_read_routing_rx_flow_status_response_flow_params
(
	const ddp_message_read_info_t * message_info,
	uint16_t flow_idx,
	uint16_t * out_flow_id,
	uint16_t * out_status,
	uint8_t * out_avail,
	uint8_t * out_active
)
{
	uint16_t num_flows;
	uint16_t *payload;
	uint16_t status_array_offset;
	ddp_routing_rx_flow_status_t *rx_flow_status_ptr;

	if (!message_info)
	{
		return AUD_ERR_INVALIDPARAMETER;
	}

	payload = (uint16_t *)message_info->payload.value16;
	num_flows = ntohs(*payload);

	if (flow_idx >= num_flows)
	{
		return AUD_ERR_INVALIDPARAMETER;
	}

	payload += 2;

	status_array_offset = ntohs(*payload);
	rx_flow_status_ptr = (ddp_routing_rx_flow_status_t *)(message_info->header.value8 + status_array_offset);

	if (status_array_offset)
	{
		rx_flow_status_ptr += flow_idx;

		if (out_flow_id)
		{
			*out_flow_id = ntohs(rx_flow_status_ptr->flow_id);
		}
		if (out_status)
		{
			*out_status = ntohs(rx_flow_status_ptr->status);
		}
		if (out_avail)
		{
			*out_avail = rx_flow_status_ptr->avail;
		}
		if (out_active)
		{
			*out_active = rx_flow_status_ptr->active;
		}
	}
	else
	{
		return AUD_ERR_NODATA;
	}

	return AUD_SUCCESS;
}

aud_error_t
ddp_read_routing_rx_chan_label_set_response_header
(
	const ddp_packet_read_info_t * packet_info,
	uint16_t offset,
	ddp_message_read_info_t * out_message_info,
	ddp_request_id_t * out_request_id,
	ddp_status_t * out_status,
	uint16_t * out_num_chans
)
{
	aud_error_t result = AUD_SUCCESS;
	ddp_routing_rx_chan_label_set_response_t * header;

	if (!packet_info || (offset >= packet_info->packet_length_bytes) || !out_message_info)
	{
		return AUD_ERR_INVALIDPARAMETER;
	}
	result = ddp_packet_read_message(packet_info, out_message_info, offset);
	if (result != AUD_SUCCESS)
	{
		return result;
	}
	result = ddp_packet_validate_message(out_message_info, DDP_OP_ROUTING_RX_CHAN_LABEL_SET, sizeof(ddp_routing_rx_chan_label_set_response_t));
	if (result != AUD_SUCCESS)
	{
		return result;
	}
	header = (ddp_routing_rx_chan_label_set_response_t *)out_message_info->header.header;

	if (out_request_id)
	{
		*out_request_id = ntohs(header->subheader.request_id);
	}
	if (out_status)
	{
		*out_status = ntohs(header->subheader.status);
	}
	if (out_num_chans)
	{
		ddp_heap_read_info_t heap;
		result = ddp_offset_array_payload_init_read(out_message_info, &heap);
		if (result != AUD_SUCCESS) {
			return result;
		}

		*out_num_chans = heap.num_elements;
	}

	return AUD_SUCCESS;
}

aud_error_t
ddp_read_routing_rx_chan_label_set_response_chan_params
(
	const ddp_message_read_info_t * message_info,
	uint16_t index,
	ddp_rx_chan_state_params_t * out_rx_chan_data,
	uint16_t * out_num_custom_encodings
)
{
	return ddp_read_routing_rx_chan_state(message_info, index, out_rx_chan_data, out_num_custom_encodings);
}

aud_error_t
ddp_read_routing_rx_chan_label_set_response_custom_encoding
(
	ddp_message_read_info_t const * message_info,
	uint16_t chan_idx,
	uint16_t custom_enc_idx,
	uint16_t * out_custom_encoding
)
{
	return ddp_read_routing_rx_chan_custom_encoding(message_info, chan_idx, custom_enc_idx, out_custom_encoding);
}

aud_error_t
ddp_read_routing_tx_chan_config_state_response_custom_encoding
(
	ddp_message_read_info_t const * message_info,
	uint16_t chan_idx,
	uint16_t custom_enc_idx,
	uint16_t * out_custom_encoding
)
{
	return ddp_read_routing_tx_chan_custom_encoding(message_info, chan_idx, custom_enc_idx, out_custom_encoding);
}

aud_error_t
ddp_read_routing_tx_chan_label_set_response_header
(
	const ddp_packet_read_info_t * packet_info,
	uint16_t offset,
	ddp_message_read_info_t * out_message_info,
	ddp_request_id_t * out_request_id,
	ddp_status_t * out_status,
	uint16_t * out_num_chans
)
{
	aud_error_t result;
	ddp_routing_tx_chan_label_set_response_t * header;

	if (!packet_info || (offset >= packet_info->packet_length_bytes))
	{
		return AUD_ERR_INVALIDPARAMETER;
	}
	result = ddp_packet_read_message(packet_info, out_message_info, offset);
	if (result != AUD_SUCCESS)
	{
		return result;
	}
	result = ddp_packet_validate_message(out_message_info, DDP_OP_ROUTING_TX_CHAN_LABEL_SET, sizeof(ddp_routing_tx_chan_label_set_response_t));
	if (result != AUD_SUCCESS)
	{
		return result;
	}
	header = (ddp_routing_tx_chan_label_set_response_t *)out_message_info->header.header;

	if (out_request_id)
	{
		*out_request_id = ntohs(header->subheader.request_id);
	}
	if (out_status)
	{
		*out_status = ntohs(header->subheader.status);
	}

	if (out_num_chans)
	{
		ddp_heap_read_info_t heap;
		result = ddp_offset_array_payload_init_read(out_message_info, &heap);
		if (result != AUD_SUCCESS)
		{
			return result;
		}

		*out_num_chans = heap.num_elements;
	}

	return AUD_SUCCESS;
}

aud_error_t
ddp_read_routing_tx_chan_label_set_response_chan_params
(
	const ddp_message_read_info_t * message_info,
	uint16_t index,
	ddp_tx_chan_state_params_t * out_tx_chan_data,
	uint16_t * out_num_custom_encodings
)
{
	return ddp_read_routing_tx_chan_state(message_info, index, out_tx_chan_data, out_num_custom_encodings);
}

aud_error_t
ddp_read_routing_tx_chan_label_set_response_custom_encoding
(
	ddp_message_read_info_t const * message_info,
	uint16_t chan_idx,
	uint16_t custom_enc_idx,
	uint16_t * out_custom_encoding
)
{
	return ddp_read_routing_tx_chan_custom_encoding(message_info, chan_idx, custom_enc_idx, out_custom_encoding);
}

aud_error_t
ddp_read_routing_rx_sub_set_response_header
(
	const ddp_packet_read_info_t * packet_info,
	uint16_t offset,
	ddp_message_read_info_t * out_message_info,
	ddp_request_id_t * out_request_id,
	ddp_status_t * out_status,
	uint16_t * out_num_chans
)
{
	aud_error_t result = AUD_SUCCESS;
	ddp_routing_rx_sub_set_response_t * header;

	if (!packet_info || (offset >= packet_info->packet_length_bytes) || !out_message_info)
	{
		return AUD_ERR_INVALIDPARAMETER;
	}
	result = ddp_packet_read_message(packet_info, out_message_info, offset);
	if (result != AUD_SUCCESS)
	{
		return result;
	}
	result = ddp_packet_validate_message(out_message_info, DDP_OP_ROUTING_RX_SUBSCRIBE_SET, sizeof(ddp_routing_rx_sub_set_response_t));
	if (result != AUD_SUCCESS)
	{
		return result;
	}
	header = (ddp_routing_rx_sub_set_response_t *)out_message_info->header.header;

	if (out_request_id)
	{
		*out_request_id = ntohs(header->subheader.request_id);
	}
	if (out_status)
	{
		*out_status = ntohs(header->subheader.status);
	}
	if (out_num_chans)
	{
		ddp_heap_read_info_t heap;
		result = ddp_offset_array_payload_init_read(out_message_info, &heap);
		if (result != AUD_SUCCESS) {
			return result;
		}

		*out_num_chans = heap.num_elements;
	}

	return AUD_SUCCESS;
}

aud_error_t
ddp_read_routing_rx_sub_set_response_chan_params
(
	const ddp_message_read_info_t * message_info,
	uint16_t index,
	ddp_rx_chan_state_params_t * out_rx_chan_data,
	uint16_t * out_num_custom_encodings
)
{
	return ddp_read_routing_rx_chan_state(message_info, index, out_rx_chan_data, out_num_custom_encodings);
}

aud_error_t
ddp_read_routing_rx_sub_set_response_custom_encoding
(
	ddp_message_read_info_t const * message_info,
	uint16_t chan_idx,
	uint16_t custom_enc_idx,
	uint16_t * out_custom_encoding
)
{
	return ddp_read_routing_rx_chan_config_state_response_custom_encoding(message_info, chan_idx, custom_enc_idx, out_custom_encoding);
}

aud_error_t
ddp_read_routing_rx_unsub_chan_response
(
	const ddp_packet_read_info_t * packet_info,
	uint16_t offset,
	ddp_request_id_t * out_request_id,
	ddp_status_t * out_status
)
{
	aud_error_t result;
	ddp_message_read_info_t message_info;
	ddp_routing_rx_unsub_chan_response_t * header;

	if (!packet_info || (offset >= packet_info->packet_length_bytes))
	{
		return AUD_ERR_INVALIDPARAMETER;
	}
	result = ddp_packet_read_message(packet_info, &message_info, offset);
	if (result != AUD_SUCCESS)
	{
		return result;
	}
	result = ddp_packet_validate_message(&message_info, DDP_OP_ROUTING_RX_UNSUB_CHAN, sizeof(ddp_routing_rx_unsub_chan_response_t));
	if (result != AUD_SUCCESS)
	{
		return result;
	}
	header = (ddp_routing_rx_unsub_chan_response_t *)message_info.header.header;

	if (out_request_id)
	{
		*out_request_id = ntohs(header->subheader.request_id);
	}
	if (out_status)
	{
		*out_status = ntohs(header->subheader.status);
	}

	return AUD_SUCCESS;
}

aud_error_t
ddp_read_routing_multicast_tx_flow_config_response_header
(
	const ddp_packet_read_info_t * packet_info,
	uint16_t offset,
	ddp_message_read_info_t * out_message_info,
	ddp_request_id_t * out_request_id,
	ddp_status_t * out_status,
	uint16_t * out_num_flows
)
{
	aud_error_t result;
	ddp_routing_multicast_tx_flow_config_response_t * header;

	if (!packet_info || (offset >= packet_info->packet_length_bytes))
	{
		return AUD_ERR_INVALIDPARAMETER;
	}
	result = ddp_packet_read_message(packet_info, out_message_info, offset);
	if (result != AUD_SUCCESS)
	{
		return result;
	}
	result = ddp_packet_validate_message(out_message_info, DDP_OP_ROUTING_MCAST_TX_FLOW_CONFIG_SET, sizeof(ddp_routing_multicast_tx_flow_config_response_t));
	if (result != AUD_SUCCESS)
	{
		return result;
	}
	header = (ddp_routing_multicast_tx_flow_config_response_t *)out_message_info->header.header;

	if (out_request_id)
	{
		*out_request_id = ntohs(header->subheader.request_id);
	}
	if (out_status)
	{
		*out_status = ntohs(header->subheader.status);
	}

	if (out_num_flows)
	{
		ddp_heap_read_info_t heap;
		result = ddp_offset_array_payload_init_read(out_message_info, &heap);
		if (result != AUD_SUCCESS)
		{
			return result;
		}

		*out_num_flows = heap.num_elements;
	}

	return AUD_SUCCESS;
}

aud_error_t
ddp_read_routing_multicast_tx_flow_config_response_flow_params
(
	const ddp_message_read_info_t * message_info,
	uint16_t flow_idx,
	uint16_t * out_num_slots,
	uint16_t * out_num_addresses,
	ddp_tx_flow_state_params_t * out_tx_flow
)
{
	return ddp_read_routing_tx_flow_state(message_info, flow_idx, out_num_slots, out_num_addresses, out_tx_flow);
}

aud_error_t
ddp_read_routing_multicast_tx_flow_config_response_flow_slots
(
	const ddp_message_read_info_t * message_info,
	uint16_t flow_idx,
	uint16_t * out_slots
)
{
	return ddp_read_routing_tx_flow_state_slots(message_info, flow_idx, out_slots);
}

aud_error_t
ddp_read_routing_multicast_tx_flow_config_state_response_flow_address_legacy
(
	const ddp_message_read_info_t * message_info,
	uint16_t flow_idx,
	uint16_t address_idx,
	uint32_t * out_ip_address,
	uint16_t * out_port
)
{
	aud_error_t result;
	ddp_ip_addr_nw_t ip_addr_nw_ip;

	result = ddp_read_routing_multicast_tx_flow_config_state_response_flow_address_nw_ip(message_info, flow_idx, address_idx, &ip_addr_nw_ip, out_port);
	*out_ip_address = ntohl(ip_addr_nw_ip.ip_addr);

	return result;
}

aud_error_t
ddp_read_routing_multicast_tx_flow_config_state_response_flow_address_nw_ip
(
	const ddp_message_read_info_t * message_info,
	uint16_t flow_idx,
	uint16_t address_idx,
	ddp_ip_addr_nw_t * out_ip_address,
	uint16_t * out_port
)
{
	return ddp_read_routing_flow_state_address(message_info, flow_idx, address_idx, out_ip_address, out_port);
}

aud_error_t
ddp_read_routing_flow_delete_response
(
	const ddp_packet_read_info_t * packet_info,
	uint16_t offset,
	ddp_request_id_t * out_request_id,
	ddp_status_t * out_status
)
{
	aud_error_t result;
	ddp_message_read_info_t message_info;
	ddp_routing_flow_delete_response_t * header;

	if (!packet_info || (offset >= packet_info->packet_length_bytes))
	{
		return AUD_ERR_INVALIDPARAMETER;
	}
	result = ddp_packet_read_message(packet_info, &message_info, offset);
	if (result != AUD_SUCCESS)
	{
		return result;
	}
	result = ddp_packet_validate_message(&message_info, DDP_OP_ROUTING_FLOW_DELETE, sizeof(ddp_routing_flow_delete_response_t));
	if (result != AUD_SUCCESS)
	{
		return result;
	}
	header = (ddp_routing_flow_delete_response_t *)message_info.header.header;

	if (out_request_id)
	{
		*out_request_id = ntohs(header->subheader.request_id);
	}
	if (out_status)
	{
		*out_status = ntohs(header->subheader.status);
	}

	return AUD_SUCCESS;
}

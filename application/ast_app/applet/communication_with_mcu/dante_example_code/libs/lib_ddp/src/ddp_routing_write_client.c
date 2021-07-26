/*
* File     : ddp_routing_write_client.c
* Created  : September 2014
* Author   : Chamira Perera <cperera@audinate.com>
* Synopsis : All routing DDP client write helper functions.
*
* Copyright 2005-2017, Audinate Pty Ltd and/or its licensors
Confidential
* Audinate Copyright Header Version 1
*/

#include "ddp/routing_client.h"
#include "ddp/routing_structures.h"
#include "ddp/routing_support.h"

aud_error_t
ddp_add_routing_basic_request
(
	ddp_packet_write_info_t * packet_info,
	ddp_request_id_t request_id
)
{
	aud_error_t result = AUD_SUCCESS;
	ddp_message_write_info_t message_info;

	if (!packet_info)
	{
		return AUD_ERR_INVALIDPARAMETER;
	}

	result = ddp_packet_allocate_request(packet_info, &message_info,
										DDP_OP_ROUTING_BASIC,
										request_id,
										0);
	return result;
}

aud_error_t
ddp_add_routing_ready_state_request
(
	ddp_packet_write_info_t * packet_info,
	ddp_request_id_t request_id
)
{
	aud_error_t result = AUD_SUCCESS;
	ddp_message_write_info_t message_info;

	if (!packet_info)
	{
		return AUD_ERR_INVALIDPARAMETER;
	}

	result = ddp_packet_allocate_request(packet_info, &message_info,
										DDP_OP_ROUTING_READY_STATE,
										request_id,
										0);
	return result;
}


aud_error_t
ddp_add_routing_performance_config_request
(
	ddp_packet_write_info_t * packet_info,
	ddp_request_id_t request_id,
	uint16_t unicast_control_flags,
	uint16_t unicast_fpp,
	uint32_t unicast_latency,
	uint16_t multicast_control_flags,
	uint16_t multicast_fpp,
	uint32_t multicast_latency
)
{
	aud_error_t result = AUD_SUCCESS;
	ddp_message_write_info_t message_info;
	ddp_routing_performance_config_request_t * raw_header;
	uint16_t element_flags = 0;

	if (!packet_info)
	{
		return AUD_ERR_INVALIDPARAMETER;
	}

	result = ddp_packet_allocate_request(packet_info, &message_info,
										DDP_OP_ROUTING_PERFORMANCE_CONFIG,
										request_id,
										sizeof(raw_header->payload));

	if (result != AUD_SUCCESS)
	{
		return result;
	}

	raw_header = (ddp_routing_performance_config_request_t *)message_info._.buf32;
	raw_header->payload.element_size = htons(sizeof(ddp_routing_performance_payload_performance_t));

	if (unicast_control_flags)
	{
		raw_header->payload.unicast.flags = htons(unicast_control_flags);
		element_flags |= DDP_ROUTING_PERFORMANCE_FLAG__UNICAST;

		if (unicast_control_flags & DDP_ROUTING_PERFORMANCE_VALID_FLAG__FPP)
		{
			raw_header->payload.unicast.fpp = htons(unicast_fpp);
		}
		else
		{
			raw_header->payload.unicast.fpp = 0;
		}
		if (unicast_control_flags & DDP_ROUTING_PERFORMANCE_VALID_FLAG__LATENCY)
		{
			raw_header->payload.unicast.latency = htonl(unicast_latency * 1000);
		}
		else
		{
			raw_header->payload.unicast.latency = 0;
		}
	}
	else
	{
		raw_header->payload.unicast.flags = 0;
		raw_header->payload.unicast.fpp = 0;
		raw_header->payload.unicast.latency = 0;
	}
	if (multicast_control_flags)
	{
		raw_header->payload.multicast.flags = htons(multicast_control_flags);
		element_flags |= DDP_ROUTING_PERFORMANCE_FLAG__MULTICAST;

		if (multicast_control_flags & DDP_ROUTING_PERFORMANCE_VALID_FLAG__FPP)
		{
			raw_header->payload.multicast.fpp = htons(multicast_fpp);
		}
		else
		{
			raw_header->payload.multicast.fpp = 0;
		}
		if (multicast_control_flags & DDP_ROUTING_PERFORMANCE_VALID_FLAG__LATENCY)
		{
			raw_header->payload.multicast.latency = htonl(multicast_latency * 1000);
		}
		else
		{
			raw_header->payload.multicast.latency = 0;
		}
	}
	else
	{
		raw_header->payload.multicast.flags = 0;
		raw_header->payload.multicast.fpp = 0;
		raw_header->payload.multicast.latency = 0;
	}

	raw_header->payload.element_flags = htons(element_flags);

	return AUD_SUCCESS;
}


aud_error_t
ddp_add_routing_rx_chan_config_state_request
(
	ddp_packet_write_info_t * packet_info,
	ddp_request_id_t request_id,
	uint16_t from,
	uint16_t to
)
{
	aud_error_t result = AUD_SUCCESS;
	ddp_routing_support_init_query_t range_info;

	if (!packet_info)
	{
		return AUD_ERR_INVALIDPARAMETER;
	}

	if (!from || !to || (from > to))
	{
		return AUD_ERR_INVALIDPARAMETER;
	}

	result = ddp_routing_support_init_query(packet_info, &range_info, DDP_OP_ROUTING_RX_CHAN_CONFIG_STATE, request_id, 0);
	if (result != AUD_SUCCESS)
	{
		return result;
	}

	result = ddp_routing_support_query_add_range(&range_info, from, to);
	if (result != AUD_SUCCESS)
	{
		return result;
	}

	return AUD_SUCCESS;
}

aud_error_t
ddp_add_routing_tx_chan_config_state_request
(
	ddp_packet_write_info_t * packet_info,
	ddp_request_id_t request_id,
	uint16_t from,
	uint16_t to
)
{
	aud_error_t result = AUD_SUCCESS;
	ddp_routing_support_init_query_t range_info;

	if (!packet_info)
	{
		return AUD_ERR_INVALIDPARAMETER;
	}

	if (!from || !to || (from > to))
	{
		return AUD_ERR_INVALIDPARAMETER;
	}

	result = ddp_routing_support_init_query(packet_info, &range_info, DDP_OP_ROUTING_TX_CHAN_CONFIG_STATE, request_id, 0);
	if (result != AUD_SUCCESS)
	{
		return result;
	}

	result = ddp_routing_support_query_add_range(&range_info, from, to);
	if (result != AUD_SUCCESS)
	{
		return result;
	}

	return AUD_SUCCESS;
}

aud_error_t
ddp_add_routing_rx_chan_status_request
(
	ddp_packet_write_info_t * packet_info,
	ddp_request_id_t request_id,
	uint16_t from,
	uint16_t to
)
{
	aud_error_t result = AUD_SUCCESS;
	ddp_routing_support_init_query_t range_info;

	if (!packet_info)
	{
		return AUD_ERR_INVALIDPARAMETER;
	}

	if (!from || !to || (from > to))
	{
		return AUD_ERR_INVALIDPARAMETER;
	}

	result = ddp_routing_support_init_query(packet_info, &range_info, DDP_OP_ROUTING_RX_CHAN_STATUS, request_id, 0);
	if (result != AUD_SUCCESS)
	{
		return result;
	}

	result = ddp_routing_support_query_add_range(&range_info, from, to);
	if (result != AUD_SUCCESS)
	{
		return result;
	}

	return AUD_SUCCESS;
}

aud_error_t
ddp_add_routing_rx_flow_config_state_request
(
	ddp_packet_write_info_t * packet_info,
	ddp_request_id_t request_id,
	uint16_t from,
	uint16_t to
)
{
	aud_error_t result = AUD_SUCCESS;
	ddp_routing_support_init_query_t range_info;

	if (!packet_info)
	{
		return AUD_ERR_INVALIDPARAMETER;
	}

	if (!from || !to || (from > to))
	{
		return AUD_ERR_INVALIDPARAMETER;
	}

	result = ddp_routing_support_init_query(packet_info, &range_info, DDP_OP_ROUTING_RX_FLOW_CONFIG_STATE, request_id, 0);
	if (result != AUD_SUCCESS)
	{
		return result;
	}

	result = ddp_routing_support_query_add_range(&range_info, from, to);
	if (result != AUD_SUCCESS)
	{
		return result;
	}

	return AUD_SUCCESS;
}

aud_error_t
ddp_add_routing_tx_flow_config_state_request
(
	ddp_packet_write_info_t * packet_info,
	ddp_request_id_t request_id,
	uint16_t from,
	uint16_t to
)
{
	aud_error_t result = AUD_SUCCESS;
	ddp_routing_support_init_query_t range_info;

	if (!packet_info)
	{
		return AUD_ERR_INVALIDPARAMETER;
	}

	if (!from || !to || (from > to))
	{
		return AUD_ERR_INVALIDPARAMETER;
	}

	result = ddp_routing_support_init_query(packet_info, &range_info, DDP_OP_ROUTING_TX_FLOW_CONFIG_STATE, request_id, 0);
	if (result != AUD_SUCCESS)
	{
		return result;
	}

	result = ddp_routing_support_query_add_range(&range_info, from, to);
	if (result != AUD_SUCCESS)
	{
		return result;
	}

	return AUD_SUCCESS;
}

aud_error_t
ddp_add_routing_rx_flow_status_request
(
	ddp_packet_write_info_t * packet_info,
	ddp_request_id_t request_id,
	uint16_t from,
	uint16_t to
)
{
	aud_error_t result = AUD_SUCCESS;
	ddp_routing_support_init_query_t range_info;

	if (!packet_info)
	{
		return AUD_ERR_INVALIDPARAMETER;
	}

	if (!from || !to || (from > to))
	{
		return AUD_ERR_INVALIDPARAMETER;
	}

	result = ddp_routing_support_init_query(packet_info, &range_info, DDP_OP_ROUTING_RX_FLOW_STATUS, request_id, 0);
	if (result != AUD_SUCCESS)
	{
		return result;
	}

	result = ddp_routing_support_query_add_range(&range_info, from, to);
	if (result != AUD_SUCCESS)
	{
		return result;
	}

	return AUD_SUCCESS;
}

aud_error_t
ddp_add_routing_flow_delete_request
(
	ddp_packet_write_info_t * packet_info,
	ddp_request_id_t request_id,
	uint16_t from,
	uint16_t to,
	uint16_t filter
)
{
	aud_error_t result = AUD_SUCCESS;
	ddp_routing_support_init_query_t range_info;

	if (!packet_info)
	{
		return AUD_ERR_INVALIDPARAMETER;
	}

	if (!from || !to || (from > to))
	{
		return AUD_ERR_INVALIDPARAMETER;
	}

	result = ddp_routing_support_init_flow_delete(packet_info, &range_info, request_id, 0, filter);
	if (result != AUD_SUCCESS)
	{
		return result;
	}

	result = ddp_routing_support_query_add_range(&range_info, from, to);
	if (result != AUD_SUCCESS)
	{
		return result;
	}

	return AUD_SUCCESS;
}


aud_error_t
ddp_add_routing_rx_chan_label_set_request
(
	ddp_packet_write_info_t * packet_info,
	ddp_request_id_t request_id,
	uint16_t num_chans,
	ddp_label_params_t * labels
)
{
	aud_error_t result = AUD_SUCCESS;
	ddp_routing_support_init_labels_t labels_info;
	uint32_t i;

	if (!packet_info || !num_chans || !labels)
	{
		return AUD_ERR_INVALIDPARAMETER;
	}

	result = ddp_routing_support_init_labels(packet_info, &labels_info, DDP_OP_ROUTING_RX_CHAN_LABEL_SET, request_id, num_chans);
	if (result != AUD_SUCCESS)
	{
		return result;
	}

	for (i = 0; i < num_chans; ++i)
	{
		result = ddp_routing_support_labels_add_label(&labels_info, labels[i].chan_id, labels[i].label);
		if (result != AUD_SUCCESS)
		{
			return result;
		}
	}

	return AUD_SUCCESS;
}

aud_error_t
ddp_add_routing_tx_chan_label_set_request
(
	ddp_packet_write_info_t * packet_info,
	ddp_request_id_t request_id,
	uint16_t num_chans,
	ddp_label_params_t * labels
)
{
	aud_error_t result = AUD_SUCCESS;
	ddp_routing_support_init_labels_t labels_info;
	uint32_t i;

	if (!packet_info || !num_chans || !labels)
	{
		return AUD_ERR_INVALIDPARAMETER;
	}

	result = ddp_routing_support_init_labels(packet_info, &labels_info, DDP_OP_ROUTING_TX_CHAN_LABEL_SET, request_id, num_chans);
	if (result != AUD_SUCCESS)
	{
		return result;
	}

	for (i = 0; i < num_chans; ++i)
	{
		result = ddp_routing_support_labels_add_label(&labels_info, labels[i].chan_id, labels[i].label);
		if (result != AUD_SUCCESS)
		{
			return result;
		}
	}

	return AUD_SUCCESS;
}

aud_error_t
ddp_add_routing_rx_sub_set_request
(
	ddp_packet_write_info_t * packet_info,
	ddp_request_id_t request_id,
	uint16_t num_chans,
	ddp_rx_chan_sub_params_t * chan_subs
)
{
	aud_error_t result = AUD_SUCCESS;
	ddp_routing_support_init_subs_t subs_info;
	uint32_t i;

	if (!packet_info || !num_chans || !chan_subs)
	{
		return AUD_ERR_INVALIDPARAMETER;
	}

	result = ddp_routing_support_init_subs(packet_info, &subs_info, DDP_OP_ROUTING_RX_SUBSCRIBE_SET, request_id, num_chans);
	if (result != AUD_SUCCESS)
	{
		return result;
	}

	for (i = 0; i < num_chans; ++i)
	{
		result = ddp_routing_support_subs_add_sub(&subs_info, chan_subs[i].rx_chan_id, chan_subs[i].tx_chan_id, chan_subs[i].tx_device);
		if (result != AUD_SUCCESS)
		{
			return result;
		}
	}

	return AUD_SUCCESS;
}

aud_error_t
ddp_add_routing_rx_unsub_chan_request
(
	ddp_packet_write_info_t * packet_info,
	ddp_request_id_t request_id,
	uint16_t from,
	uint16_t to
)
{
	aud_error_t result = AUD_SUCCESS;
	ddp_routing_support_init_query_t range_info;

	if (!packet_info)
	{
		return AUD_ERR_INVALIDPARAMETER;
	}

	if (!from || !to || (from > to))
	{
		return AUD_ERR_INVALIDPARAMETER;
	}

	result = ddp_routing_support_init_query(packet_info, &range_info, DDP_OP_ROUTING_RX_UNSUB_CHAN, request_id, 0);
	if (result != AUD_SUCCESS)
	{
		return result;
	}

	result = ddp_routing_support_query_add_range(&range_info, from, to);
	if (result != AUD_SUCCESS)
	{
		return result;
	}

	return AUD_SUCCESS;
}


void ddp_routing_multicast_tx_flow_config_add_slot_params
(
	ddp_multicast_tx_flow_params_t * mcast_flow,
	uint16_t * slots
)
{
	mcast_flow->slots = slots;
}

aud_error_t
ddp_add_routing_multicast_tx_flow_config_request
(
	ddp_packet_write_info_t * packet_info,
	ddp_request_id_t request_id,
	ddp_multicast_tx_flow_params_t * mcast_flow
)
{
	aud_error_t result = AUD_SUCCESS;
	ddp_routing_support_init_flow_t flow_info;
	uint32_t i;

	if (!packet_info || !mcast_flow)
	{
		return AUD_ERR_INVALIDPARAMETER;
	}

	result = ddp_routing_support_init_tx_flow_mcast_request(packet_info, &flow_info, request_id, mcast_flow->flow_id, mcast_flow->num_slots, 0);
	if (result != AUD_SUCCESS)
	{
		return result;
	}

	result = ddp_routing_support_flow_set_label(&flow_info, mcast_flow->label);
	if (result != AUD_SUCCESS)
	{
		return result;
	}

	flow_info.flow->encoding = htons(mcast_flow->encoding);
	flow_info.flow->sample_rate = htonl(mcast_flow->sample_rate);
	flow_info.flow->fpp = htons(mcast_flow->fpp);
	flow_info.flow->latency = htonl(mcast_flow->latency);
	flow_info.flow->transport = DDP_AUDIO_TRANSPORT_ATP;
	flow_info.flow->slot_size = htons(sizeof(uint16_t));
	flow_info.flow->num_slots = htons(mcast_flow->num_slots);

	for (i = 0; i < mcast_flow->num_slots; ++i)
	{
		flow_info.slots.tx[i] = htons(mcast_flow->slots[i]);
	}

	return AUD_SUCCESS;
}


/*
 * File     : routing_support.c
 * Created  : June 2014
 * Author   : Andrew White <andrew.white@audinate.com>
 * Synopsis : Wrappers for DDP routing handling
 *
 * Copyright 2005-2017, Audinate Pty Ltd and/or its licensors
	Confidential
 * Audinate Copyright Header Version 1
 */


//----------
// Include

#include "ddp/routing_support.h"
#include "ddp/audio_structures.h"


//----------
// Queries

aud_error_t
ddp_routing_support_init_query
(
	ddp_packet_write_info_t * packet_info,	// in
	ddp_routing_support_init_query_t * query_info,	// out
	uint16_t opcode,
	uint16_t request_id,
	ddp_size_t max_ranges
)
{
	aud_error_t result;

	assert(packet_info && query_info);

	result = ddp_packet_allocate_request_head(packet_info, &query_info->message, opcode, request_id);
	if (result != AUD_SUCCESS)
		return result;

	result = ddp_message_id_range_allocate_payload(&query_info->message, &query_info->ranges, max_ranges);
	if (result != AUD_SUCCESS)
		return result;

	return AUD_SUCCESS;
}


aud_error_t
ddp_routing_support_query_add_range
(
	ddp_routing_support_init_query_t * query_info,
	dante_id_t from, dante_id_t to
)
{
	assert(query_info);

	return ddp_message_id_range_add_range(&query_info->ranges, from, to);
}


// Delete

aud_error_t
ddp_routing_support_init_flow_delete
(
	ddp_packet_write_info_t * packet_info,	// in
	ddp_routing_support_init_query_t * query_info,	// out
	uint16_t request_id,
	ddp_size_t max_ranges,
		// may be zero for incrementally growing packet
	uint16_t filter
)
{
	aud_error_t result;

	result = ddp_routing_support_init_query (
		packet_info, query_info, DDP_OP_ROUTING_FLOW_DELETE, request_id, max_ranges
	);
	if (result != AUD_SUCCESS)
		return result;

	result = ddp_routing_support_flow_delete_set_filter(query_info, filter);
	return result;
}


aud_error_t
ddp_routing_support_flow_delete_set_filter
(
	ddp_routing_support_init_query_t * query_info,
	uint16_t filter
)
{
	ddp_routing_flow_delete_request_payload_t * p;

	assert(query_info && query_info->message.payload);

	p = (ddp_routing_flow_delete_request_payload_t *)query_info->message.payload;
	p->filter = htons(filter);
	return AUD_SUCCESS;
}


//----------
// Flows

// Writing

aud_error_t
init_flow_request_common
(
	ddp_packet_write_info_t * packet_info,
	ddp_routing_support_init_flow_t * flow_info,
	uint16_t request_id,
	uint16_t opcode,
	dante_id_t flow_id,	// in
	uint16_t num_addrs
)
{
	aud_error_t result;

	bzero(flow_info, sizeof(*flow_info));

	result = ddp_packet_allocate_request(
		packet_info, &flow_info->message,
		opcode, request_id,
		sizeof(*flow_info->flow)
	);
	if (result != AUD_SUCCESS)
		return result;

	flow_info->flow = (ddp_routing_flow_data_t *) flow_info->message.payload;

	bzero(flow_info->flow, sizeof(*flow_info->flow));
	flow_info->flow->size = htons(sizeof(*flow_info->flow));
	flow_info->flow->flow_id = htons(flow_id);

	// allocate addresses
	if (num_addrs)
	{
		flow_info->addrs = ddp_routing_data_array_overlay_allocate(
			&flow_info->message,
			(ddp_routing_data_array_overlay_t *) &flow_info->flow->num_addresses,
			num_addrs, sizeof(*flow_info->addrs)
		);
		if (! flow_info->addrs)
			return AUD_ERR_NOMEMORY;
	}

	return AUD_SUCCESS;
}

aud_error_t
init_tx_flow_common_request
(
	ddp_packet_write_info_t * packet_info,
	ddp_routing_support_init_flow_t * flow_info,
	uint16_t request_id,
	uint16_t opcode,
	dante_id_t flow_id,	// in
	uint16_t num_slots,
	uint16_t num_addrs
)
{
	aud_error_t result;


	result = init_flow_request_common(
		packet_info, flow_info, request_id, opcode, flow_id, num_addrs
	);
	if (result != AUD_SUCCESS)
		return result;

	flow_info->slots.tx = (uint16_t *)ddp_routing_data_array_overlay_allocate(
		&flow_info->message,
		(ddp_routing_data_array_overlay_t *) &flow_info->flow->num_slots,
		num_slots, sizeof(*flow_info->slots.tx)
	);
	if (! flow_info->slots.tx)
		return AUD_ERR_NOMEMORY;

	return AUD_SUCCESS;
}

aud_error_t
ddp_routing_support_init_tx_flow_mcast_request
(
	ddp_packet_write_info_t * packet_info,
	ddp_routing_support_init_flow_t * flow_info,
	uint16_t request_id,
	dante_id_t flow_id,	// in
	uint16_t num_slots,
	uint16_t num_addrs
)
{
	assert(packet_info && flow_info);

	return init_tx_flow_common_request(
		packet_info, flow_info, request_id, DDP_OP_ROUTING_MCAST_TX_FLOW_CONFIG_SET,
		flow_id, num_slots, num_addrs
	);
}

aud_error_t
ddp_routing_support_flow_set_label
(
	ddp_routing_support_init_flow_t * flow,
	const char * label
)
{
	assert(flow && flow->flow);

	if (label && label[0])
	{
		if (ddp_message_allocate_string(&flow->message, &flow->flow->label_offset, label))
			return AUD_SUCCESS;
		else
			return AUD_ERR_NOMEMORY;
	}
	else
	{
		flow->flow->label_offset = 0;
		return AUD_SUCCESS;
	}
}


// Reading

static aud_error_t
read_flow_at_index
(
	ddp_message_read_info_t * message_info,
	ddp_routing_support_read_flows_t * flows,
	ddp_routing_data_array_read_t * array_read,
	unsigned index
)
{
	aud_error_t result;
	const ddp_routing_flow_data_t * flow;

	bzero(flows, sizeof(*flows));

	result = ddp_offset_array_payload_init_read(message_info, &flows->read);
	if (result != AUD_SUCCESS)
		return result;

	if (index >= flows->read.num_elements)
		return AUD_ERR_NOTFOUND;

	flows->curr_flow.flow =
		ddp_offset_array_read_element_sized(
			&flows->read, index, 0
		);
	if (! flows->curr_flow.flow)
		return AUD_ERR_INVALIDDATA;

	flows->curr_flow.index = index;
	flow = flows->curr_flow.flow;

	if (flow->num_addresses)
	{
		result = ddp_routing_data_array_overlay_read(
			message_info,
			(ddp_routing_data_array_overlay_t *) &flow->num_addresses,
			array_read
		);
		if (result)
		{
			flows->curr_flow.num_addrs = array_read->num_entries;
			flows->curr_flow.addr = array_read->array;
		}
	}
	return AUD_SUCCESS;
}


aud_error_t
ddp_routing_support_read_tx_flow_at_index
(
	ddp_message_read_info_t * message_info,
	ddp_routing_support_read_flows_t * flows,
	unsigned index
)
{
	aud_error_t result;
	ddp_routing_data_array_read_t array_read;
	const ddp_routing_flow_data_t * flow;

	assert(message_info && flows);

	result = read_flow_at_index(message_info, flows, &array_read, index);
	if (result != AUD_SUCCESS)
		return result;

	// slots
	flow = flows->curr_flow.flow;
	if (flow->num_slots)
	{
		result = ddp_routing_data_array_overlay_read(
			message_info,
			(ddp_routing_data_array_overlay_t *) &flow->num_slots,
			&array_read
		);
		if (result)
		{
			flows->curr_flow.num_slots = array_read.num_entries;
			flows->curr_flow.slots.tx = array_read.array;
		}
	}

	return result;
}


aud_error_t
ddp_routing_support_read_rx_flow_at_index
(
	ddp_message_read_info_t * message_info,
	ddp_routing_support_read_flows_t * flows,
	unsigned index
)
{
	aud_error_t result;
	ddp_routing_data_array_read_t array_read;
	const ddp_routing_flow_data_t * flow;

	assert(message_info && flows);

	result = read_flow_at_index(message_info, flows, &array_read, index);
	if (result != AUD_SUCCESS)
		return result;

	// slots
	flow = flows->curr_flow.flow;
	if (flow->num_slots)
	{
		result = ddp_routing_rx_flow_slots_read(
			message_info,
			&flows->curr_flow.slots.rx,
			(ddp_routing_data_array_overlay_t *) &flow->num_slots
		);
	}

	return result;
}


//----------
// Channels

aud_error_t
ddp_routing_support_read_rx_channel_at_index
(
	ddp_message_read_info_t * message_info,
	ddp_routing_support_read_channels_t * channels_info,
	unsigned index
)
{
	aud_error_t result;
	const ddp_routing_rx_channel_data_t * channel;

	// Tx is subset of Rx
	result = ddp_routing_support_read_tx_channel_at_index(message_info, channels_info, index);
	if (result != AUD_SUCCESS)
		return result;

	channel = channels_info->curr_chan.channel.rx;

	// resolve subscriptions
	channels_info->curr_chan.sub_channel =
		ddp_routing_support_string_from_raw_offset(message_info, channel->sub_channel_offset);
	channels_info->curr_chan.sub_channel =
		ddp_routing_support_string_from_raw_offset(message_info, channel->sub_device_offset);

	return AUD_SUCCESS;
}

aud_error_t
ddp_routing_support_read_tx_channel_at_index
(
	ddp_message_read_info_t * message_info,
	ddp_routing_support_read_channels_t * channels_info,
	unsigned index
)
{
	aud_error_t result;
	const ddp_routing_tx_channel_data_t * channel;

	assert(message_info && channels_info);

	bzero(channels_info, sizeof(*channels_info));

	result = ddp_offset_array_payload_init_read(message_info, &channels_info->read);
	if (result != AUD_SUCCESS)
		return result;

	if (index >= channels_info->read.num_elements)
		return AUD_ERR_NOTFOUND;

	channels_info->curr_chan.channel.tx =
		ddp_offset_array_read_element_sized(
			&channels_info->read, index, 0
		);
	if (! channels_info->curr_chan.channel.tx)
		return AUD_ERR_INVALIDDATA;

	channels_info->curr_chan.index = index;
	channel = channels_info->curr_chan.channel.tx;

	// resolve the names
	channels_info->curr_chan.name =
		ddp_routing_support_string_from_raw_offset(message_info, channel->name_offset);
	channels_info->curr_chan.label =
		ddp_routing_support_string_from_raw_offset(message_info, channel->name_offset);

	// resolve format
	if (channel->format_offset)
	{
		channels_info->curr_chan.format =
			ddp_heap_resolve_sized(message_info, ntohs(channel->format_offset), 0);
	}

	return AUD_SUCCESS;
}


//----------
// Subscriptions

aud_error_t
ddp_routing_support_init_subs
(
	ddp_packet_write_info_t * packet_info,	// in
	ddp_routing_support_init_subs_t * labels,
	uint16_t opcode,
	uint16_t request_id,
	uint16_t num_labels
);

/*
	Add a subscription entry

	channel & device must either both be legal strings or both NULL
 */
aud_error_t
ddp_routing_support_subs_add_sub
(
	ddp_routing_support_init_subs_t * labels,
	dante_id_t channel_id,
	const char * channel,
	const char * device
);


//----------
// Subscriptions & Labels

aud_error_t
ddp_routing_support_init_array
(
	ddp_packet_write_info_t * packet_info,	// in
	ddp_routing_support_init_array_t * array,
	uint16_t opcode,
	uint16_t request_id,
	ddp_size_t elem_size,
	uint16_t num
)
{
	aud_error_t result;

	assert(packet_info && array);

	if (! num)
		return AUD_ERR_INVALIDPARAMETER;

	result = ddp_packet_allocate_request_head(packet_info, &array->message, opcode, request_id);
	if (result != AUD_SUCCESS)
		return result;

	result = ddp_array_payload_allocate(&array->message, elem_size);
	if (result != AUD_SUCCESS)
		return result;

	array->array.array = ddp_array_payload_allocate_elements(&array->message, num);
	if (! array->array.array)
		return AUD_ERR_NOMEMORY;

	array->max = num;
	array->num = 0;

	return AUD_SUCCESS;
}


aud_error_t
ddp_routing_support_subs_add_sub
(
	ddp_routing_support_init_subs_t * subs,
	dante_id_t channel_id,
	const char * channel,
	const char * device
)
{
	ddp_routing_subscription_config_t * sub;
	aud_bool_t have_channel = aud_str_is_non_empty(channel);
	aud_bool_t have_device = aud_str_is_non_empty(device);

	assert(subs && subs->array.subs && subs->max);

	if (subs->num >= subs->max)
	{
		return AUD_ERR_RANGE;
	}

	if (have_channel != have_device)
	{
		return AUD_ERR_INVALIDPARAMETER;
	}

	sub = subs->array.subs + subs->num;

	sub->local_channel.ident = htons(channel_id);
	sub->local_channel.name_offset = 0;
	if (have_channel)
	{
		if (! ddp_message_allocate_string(&subs->message, &sub->subscribed_channel_offset, channel))
		{
			return AUD_ERR_NOMEMORY;
		}
		if (! ddp_message_allocate_string(&subs->message, &sub->subscribed_device_offset, device))
		{
			sub->subscribed_channel_offset = 0;
			return AUD_ERR_NOMEMORY;
		}
	}
	else
	{
		sub->subscribed_channel_offset = 0;
		sub->subscribed_device_offset = 0;
	}
	subs->num++;
	return AUD_SUCCESS;
}


aud_error_t
ddp_routing_support_labels_add_label
(
	ddp_routing_support_init_labels_t * labels,
	dante_id_t channel_id,
	const char * label
)
{
	ddp_name_id_pair_t * pair;

	assert(labels && labels->array.labels && labels->max);

	if (labels->num >= labels->max)
	{
		return AUD_ERR_RANGE;
	}

	pair = labels->array.labels + labels->num;

	pair->ident = htons(channel_id);
	if (label && label[0])
	{
		if (! ddp_message_allocate_string(&labels->message, &pair->name_offset, label))
		{
			return AUD_ERR_NOMEMORY;
		}
	}
	else
	{
		pair->name_offset = 0;
	}
	labels->num++;
	return AUD_SUCCESS;
}

aud_error_t
ddp_read_routing_rx_chan_state
(
	ddp_message_read_info_t const * message_info,
	uint16_t  chan_idx,
	ddp_rx_chan_state_params_t * out_rx_chan_data,
	uint16_t * out_num_custom_encodings
)
{
	aud_error_t result;
	ddp_heap_read_info_t heap;
	uint16_t num_chans, element_size;
	ddp_routing_rx_channel_data_t * data;
	
	if (!message_info || !out_rx_chan_data) 
	{
		return AUD_ERR_INVALIDPARAMETER;
	}

	result =  ddp_offset_array_payload_init_read(message_info, &heap);
	if (result != AUD_SUCCESS)
	{
		return result;
	}

	num_chans = heap.num_elements;
	element_size = sizeof(ddp_routing_rx_channel_data_t);

	if (chan_idx >= num_chans) 
	{
		return AUD_ERR_INVALIDPARAMETER;
	}
	
	data = (ddp_routing_rx_channel_data_t *)ddp_offset_array_read_element_sized(&heap, chan_idx, element_size);
	if (data)
	{
		out_rx_chan_data->channel_id = ntohs(data->channel_id);

		if (data->name_offset) {
			out_rx_chan_data->channel_name = (char*)message_info->header.header + ntohs(data->name_offset);
		}
		else {
			out_rx_chan_data->channel_name = NULL;
		}

		if (data->format_offset)
		{
			ddp_heap_read_info_t heap_format;
			if (ddp_read_heap_arraystruct(message_info, &heap_format, ntohs(data->format_offset), 1, sizeof(ddp_routing_channel_format_t)) == AUD_SUCCESS)
			{
				ddp_routing_channel_format_t *format = (ddp_routing_channel_format_t*)heap_format._.array16;
				out_rx_chan_data->encoding = ntohs(format->preferred_encoding);
				out_rx_chan_data->sample_rate = ntohl(format->sample_rate);
				out_rx_chan_data->pcm_map = ntohs(format->pcm_map);
				*out_num_custom_encodings = ntohs(format->num_custom_encodings);
			}
			else
			{
				out_rx_chan_data->encoding = 0;
				out_rx_chan_data->sample_rate = 0;
				out_rx_chan_data->pcm_map = 0;
				*out_num_custom_encodings = 0;
			}
		}

		out_rx_chan_data->channel_flags = ntohs(data->channel_flags);
		out_rx_chan_data->channel_flags_mask = ntohs(data->channel_flags_mask);

		if (data->label_offset) {
			out_rx_chan_data->channel_label = (char*)message_info->header.header + ntohs(data->label_offset);
		}
		else {
			out_rx_chan_data->channel_label = NULL;
		}

		out_rx_chan_data->status = ntohs(data->status);
		out_rx_chan_data->avail = data->avail;
		out_rx_chan_data->active = data->active;

		if (data->sub_channel_offset) {
			out_rx_chan_data->subscribed_channel = (char*)message_info->header.header + ntohs(data->sub_channel_offset);
		}
		else {
			out_rx_chan_data->subscribed_channel = NULL;
		}

		if (data->sub_device_offset) {
			out_rx_chan_data->subcribed_device = (char*)message_info->header.header + ntohs(data->sub_device_offset);
		}
		else {
			out_rx_chan_data->subcribed_device = NULL;
		}

		out_rx_chan_data->flow_id = ntohs(data->flow_id);
		out_rx_chan_data->slot_id = ntohs(data->slot_id);
	}
	else
	{
		return AUD_ERR_NODATA;
	}

	return AUD_SUCCESS;
}

aud_error_t
ddp_read_routing_rx_chan_custom_encoding
(
	ddp_message_read_info_t const * message_info,
	uint16_t chan_idx,
	uint16_t custom_enc_idx,
	uint16_t * out_custom_encoding
)
{
	aud_error_t result;
	ddp_heap_read_info_t heap;
	uint16_t num_chans, element_size;
	ddp_routing_rx_channel_data_t * data;

	if (!message_info)
	{
		return AUD_ERR_INVALIDPARAMETER;
	}

	result = ddp_offset_array_payload_init_read(message_info, &heap);
	if (result != AUD_SUCCESS)
	{
		return result;
	}

	num_chans = heap.num_elements;
	element_size = sizeof(ddp_routing_rx_channel_data_t);

	if (chan_idx >= num_chans)
	{
		return AUD_ERR_INVALIDPARAMETER;
	}

	data = (ddp_routing_rx_channel_data_t *)ddp_offset_array_read_element_sized(&heap, chan_idx, element_size);
	if (data)
	{
		if (data->format_offset)
		{
			ddp_heap_read_info_t heap_format;
			if (ddp_read_heap_arraystruct(message_info, &heap_format, ntohs(data->format_offset), 1, sizeof(ddp_routing_channel_format_t)) == AUD_SUCCESS)
			{
				ddp_routing_channel_format_t *format = (ddp_routing_channel_format_t*)heap_format._.array16;

				if (ddp_read_heap_arraystruct(message_info, &heap_format, ntohs(format->custom_encodings_offset), 1, sizeof(ddp_custom_enc_array_t)) == AUD_SUCCESS)
				{
					ddp_custom_enc_array_t *custom_enc = (ddp_custom_enc_array_t *)heap_format._.array16;
					custom_enc += custom_enc_idx;

					*out_custom_encoding = ntohs(custom_enc->custom_encoding);
				}
				else
				{
					return AUD_ERR_NODATA;
				}
			}
		}
		else
		{
			return AUD_ERR_NODATA;
		}
	}

	return AUD_SUCCESS;
}

aud_error_t
ddp_read_routing_tx_chan_state
(
	ddp_message_read_info_t const * message_info,
	uint16_t chan_idx,
	ddp_tx_chan_state_params_t * out_tx_chan_data,
	uint16_t * out_num_custom_encodings
)
{
	aud_error_t result;
	ddp_heap_read_info_t heap;
	uint16_t num_chans, element_size;
	ddp_routing_tx_channel_data_t * data;

	if (!message_info || !out_tx_chan_data)
	{
		return AUD_ERR_INVALIDPARAMETER;
	}

	result = ddp_offset_array_payload_init_read(message_info, &heap);
	if (result != AUD_SUCCESS)
	{
		return result;
	}

	num_chans = heap.num_elements;
	element_size = sizeof(ddp_routing_tx_channel_data_t);

	if (chan_idx >= num_chans)
	{
		return AUD_ERR_INVALIDPARAMETER;
	}

	data = (ddp_routing_tx_channel_data_t *)ddp_offset_array_read_element_sized(&heap, chan_idx, element_size);
	if (data)
	{
		out_tx_chan_data->channel_id = ntohs(data->channel_id);

		if (data->name_offset) {
			out_tx_chan_data->channel_name = (char*)message_info->header.header + ntohs(data->name_offset);
		}
		else {
			out_tx_chan_data->channel_name = NULL;
		}

		if (data->format_offset)
		{
			ddp_heap_read_info_t heap_format;
			if (ddp_read_heap_arraystruct(message_info, &heap_format, ntohs(data->format_offset), 1, sizeof(ddp_routing_channel_format_t)) == AUD_SUCCESS)
			{
				ddp_routing_channel_format_t *format = (ddp_routing_channel_format_t*)heap_format._.array16;
				out_tx_chan_data->encoding = ntohs(format->preferred_encoding);
				out_tx_chan_data->sample_rate = ntohl(format->sample_rate);
				out_tx_chan_data->pcm_map = ntohs(format->pcm_map);
				*out_num_custom_encodings = ntohs(format->num_custom_encodings);
			}
			else
			{
				out_tx_chan_data->encoding = 0;
				out_tx_chan_data->sample_rate = 0;
				out_tx_chan_data->pcm_map = 0;
				*out_num_custom_encodings = 0;
			}
		}

		out_tx_chan_data->channel_flags = ntohs(data->channel_flags);
		out_tx_chan_data->channel_flags_mask = ntohs(data->channel_flags_mask);

		if (data->label_offset) {
			out_tx_chan_data->channel_label = (char*)message_info->header.header + ntohs(data->label_offset);
		}
		else {
			out_tx_chan_data->channel_label = NULL;
		}
	}
	else
	{
		return AUD_ERR_NODATA;
	}

	return AUD_SUCCESS;
}

aud_error_t
ddp_read_routing_tx_chan_custom_encoding
(
	ddp_message_read_info_t const * message_info,
	uint16_t chan_idx,
	uint16_t custom_enc_idx,
	uint16_t * out_custom_encoding
)
{
	aud_error_t result;
	ddp_heap_read_info_t heap;
	uint16_t num_chans, element_size;
	ddp_routing_tx_channel_data_t * data;

	if (!message_info)
	{
		return AUD_ERR_INVALIDPARAMETER;
	}

	result = ddp_offset_array_payload_init_read(message_info, &heap);
	if (result != AUD_SUCCESS)
	{
		return result;
	}

	num_chans = heap.num_elements;
	element_size = sizeof(ddp_routing_tx_channel_data_t);

	if (chan_idx >= num_chans)
	{
		return AUD_ERR_INVALIDPARAMETER;
	}

	data = (ddp_routing_tx_channel_data_t *)ddp_offset_array_read_element_sized(&heap, chan_idx, element_size);
	if (data)
	{
		if (data->format_offset)
		{
			ddp_heap_read_info_t heap_format;
			if (ddp_read_heap_arraystruct(message_info, &heap_format, ntohs(data->format_offset), 1, sizeof(ddp_routing_channel_format_t)) == AUD_SUCCESS)
			{
				ddp_routing_channel_format_t *format = (ddp_routing_channel_format_t*)heap_format._.array16;

				if (ddp_read_heap_arraystruct(message_info, &heap_format, ntohs(format->custom_encodings_offset), 1, sizeof(ddp_custom_enc_array_t)) == AUD_SUCCESS)
				{
					ddp_custom_enc_array_t *custom_enc = (ddp_custom_enc_array_t *)heap_format._.array16;
					custom_enc += custom_enc_idx;
					
					*out_custom_encoding = ntohs(custom_enc->custom_encoding);
				}
				else
				{
					return AUD_ERR_NODATA;
				}
			}
		}
		else
		{
			return AUD_ERR_NODATA;
		}
	}

	return AUD_SUCCESS;
}

aud_error_t
ddp_read_routing_rx_flow_state
(
	const ddp_message_read_info_t * message_info,
	uint16_t flow_idx,
	uint16_t * out_num_slots,
	uint16_t * out_num_addresses,
	ddp_rx_flow_state_params_t * out_rx_flow
)
{
	aud_error_t result;
	ddp_heap_read_info_t heap;
	uint16_t num_flows, element_size;
	ddp_routing_flow_data_t *fd;

	if (!message_info || !out_rx_flow)
	{
		return AUD_ERR_INVALIDPARAMETER;
	}

	result = ddp_offset_array_payload_init_read(message_info, &heap);
	if (result != AUD_SUCCESS)
	{
		return result;
	}

	num_flows = heap.num_elements;
	element_size = sizeof(ddp_routing_flow_data_t);

	if (flow_idx >= num_flows)
	{
		return AUD_ERR_INVALIDPARAMETER;
	}

	fd = (ddp_routing_flow_data_t *)ddp_offset_array_read_element_sized(&heap, flow_idx, element_size);
	if (fd)
	{
		out_rx_flow->flow_id = ntohs(fd->flow_id);

		if (fd->label_offset) {
			out_rx_flow->flow_label = ddp_heap_read_string(message_info, ntohs(fd->label_offset));
		}
		else {
			out_rx_flow->flow_label = NULL;
		}

		out_rx_flow->status = ntohs(fd->status);
		out_rx_flow->flow_flags = ntohs(fd->flow_flags);
		out_rx_flow->flow_flags_mask = ntohs(fd->flow_flags_mask);
		out_rx_flow->user_conf_flags = ntohl(fd->user_conf_flags);
		out_rx_flow->user_conf_mask = ntohl(fd->user_conf_mask);
		out_rx_flow->transport = fd->transport;
		out_rx_flow->avail_mask = fd->avail_mask;
		out_rx_flow->active_mask = fd->active_mask;
		out_rx_flow->sample_rate = ntohl(fd->sample_rate);
		out_rx_flow->latency = ntohl(fd->latency) / 1000;
		out_rx_flow->encoding = ntohs(fd->encoding);
		out_rx_flow->fpp = ntohs(fd->fpp);

		if (out_num_slots)
		{
			*out_num_slots = ntohs(fd->num_slots);
		}
		if (out_num_addresses)
		{
			*out_num_addresses = ntohs(fd->num_addresses);
		}
	}
	else
	{
		return AUD_ERR_NODATA;
	}

	return AUD_SUCCESS;
}

aud_error_t
ddp_read_routing_rx_flow_state_slot
(
	const ddp_message_read_info_t * message_info,
	uint16_t flow_idx,
	uint16_t slot_idx,
	uint16_t * out_num_chans
)
{
	aud_error_t result;
	ddp_heap_read_info_t heap;
	uint16_t num_flows, element_size;
	ddp_routing_flow_data_t *fd;

	if (!message_info)
	{
		return AUD_ERR_INVALIDPARAMETER;
	}

	result = ddp_offset_array_payload_init_read(message_info, &heap);
	if (result != AUD_SUCCESS)
	{
		return result;
	}

	num_flows = heap.num_elements;
	element_size = sizeof(ddp_routing_flow_data_t);

	if (flow_idx >= num_flows)
	{
		return AUD_ERR_INVALIDPARAMETER;
	}

	fd = (ddp_routing_flow_data_t *)ddp_offset_array_read_element_sized(&heap, flow_idx, element_size);
	if (fd)
	{
		ddp_routing_rx_flow_slots_read_t _slots;
		if (slot_idx >= ntohs(fd->num_slots))
		{
			return AUD_ERR_INVALIDPARAMETER;
		}
		
		result = ddp_routing_rx_flow_slots_read(message_info, &_slots, (ddp_routing_data_array_overlay_t *)&fd->num_slots);

		if (result != AUD_SUCCESS) 
		{
			return result;
		}
		
		if (out_num_chans)
		{
			const uint16_t * channels;
			*out_num_chans = (uint16_t)ddp_routing_rx_flow_slots_read_slot(&_slots, slot_idx, &channels);
		}
	}
	else
	{
		return AUD_ERR_NODATA;
	}

	return AUD_SUCCESS;
}

aud_error_t
ddp_read_routing_rx_flow_state_slot_chans
(
	const ddp_message_read_info_t * message_info,
	uint16_t flow_idx,
	uint16_t slot_idx,
	uint16_t * out_chans
)
{
	aud_error_t result;
	ddp_heap_read_info_t heap;
	uint16_t num_flows, element_size;
	ddp_routing_flow_data_t *fd;

	if (!message_info)
	{
		return AUD_ERR_INVALIDPARAMETER;
	}

	result = ddp_offset_array_payload_init_read(message_info, &heap);
	if (result != AUD_SUCCESS)
	{
		return result;
	}

	num_flows = heap.num_elements;
	element_size = sizeof(ddp_routing_flow_data_t);

	if (flow_idx >= num_flows)
	{
		return AUD_ERR_INVALIDPARAMETER;
	}

	fd = (ddp_routing_flow_data_t *)ddp_offset_array_read_element_sized(&heap, flow_idx, element_size);
	if (fd)
	{
		const uint16_t * channels;
		uint16_t num_chans, i;
		ddp_routing_rx_flow_slots_read_t _slots;
		if (slot_idx >= ntohs(fd->num_slots))
		{
			return AUD_ERR_INVALIDPARAMETER;
		}

		
		result = ddp_routing_rx_flow_slots_read(message_info, &_slots, (ddp_routing_data_array_overlay_t *)&fd->num_slots);

		if (result != AUD_SUCCESS)
		{
			return result;
		}

		num_chans = (uint16_t)ddp_routing_rx_flow_slots_read_slot(&_slots, slot_idx, &channels);

		for (i = 0; i < num_chans; ++i) 
		{
			out_chans[i] = ntohs(channels[i]);
		}
	}
	else
	{
		return AUD_ERR_NODATA;
	}

	return AUD_SUCCESS;
}

aud_error_t
ddp_read_routing_flow_state_address
(
	const ddp_message_read_info_t * message_info,
	uint16_t flow_idx,	
	uint16_t address_idx,
	ddp_ip_addr_nw_t * out_ip_address,
	uint16_t * out_port
)
{
	aud_error_t result;
	ddp_heap_read_info_t heap;
	uint16_t num_flows, element_size;
	ddp_routing_flow_data_t *fd;

	if (!message_info)
	{
		return AUD_ERR_INVALIDPARAMETER;
	}

	result = ddp_offset_array_payload_init_read(message_info, &heap);
	if (result != AUD_SUCCESS)
	{
		return result;
	}

	num_flows = heap.num_elements;
	element_size = sizeof(ddp_routing_flow_data_t);

	if (flow_idx >= num_flows)
	{
		return AUD_ERR_INVALIDPARAMETER;
	}

	fd = (ddp_routing_flow_data_t *)ddp_offset_array_read_element_sized(&heap, flow_idx, element_size);
	if (fd)
	{
		ddp_routing_data_array_read_t array_read;
		const ddp_addr_inet_t * addr;
		if (address_idx >= ntohs(fd->num_addresses))
		{
			return AUD_ERR_INVALIDPARAMETER;
		}
		
		result = ddp_routing_data_array_overlay_read(message_info, (const ddp_routing_data_array_overlay_t *)&fd->num_addresses, &array_read);
		if (result != AUD_SUCCESS) 
		{
			return result;
		}

		addr = ddp_routing_data_array_read_entry(&array_read, address_idx);
		if (addr)
		{
			out_ip_address->ip_addr = addr->in_addr.ip_addr;
			*out_port = ntohs(addr->port);
		}
		else
		{
			out_ip_address->ip_addr = 0;
			*out_port = 0;
		}
	}
	else
	{
		return AUD_ERR_NODATA;
	}

	return AUD_SUCCESS;
}

aud_error_t
ddp_read_routing_tx_flow_state
(
	const ddp_message_read_info_t * message_info,
	uint16_t flow_idx,
	uint16_t * out_num_slots,
	uint16_t * out_num_addresses,
	ddp_tx_flow_state_params_t * out_tx_flow
)
{
	aud_error_t result;
	ddp_heap_read_info_t heap;
	uint16_t num_flows, element_size;
	ddp_routing_flow_data_t *fd;

	if (!message_info || !out_tx_flow)
	{
		return AUD_ERR_INVALIDPARAMETER;
	}

	result = ddp_offset_array_payload_init_read(message_info, &heap);
	if (result != AUD_SUCCESS)
	{
		return result;
	}

	num_flows = heap.num_elements;
	element_size = sizeof(ddp_routing_flow_data_t);

	if (flow_idx >= num_flows)
	{
		return AUD_ERR_INVALIDPARAMETER;
	}

	fd = (ddp_routing_flow_data_t *)ddp_offset_array_read_element_sized(&heap, flow_idx, element_size);
	if (fd)
	{
		out_tx_flow->flow_id = ntohs(fd->flow_id);

		if (fd->label_offset) {
			out_tx_flow->flow_label = ddp_heap_read_string(message_info, ntohs(fd->label_offset));
		}
		else {
			out_tx_flow->flow_label = NULL;
		}

		out_tx_flow->status = ntohs(fd->status);
		out_tx_flow->flow_flags = ntohs(fd->flow_flags);
		out_tx_flow->flow_flags_mask = ntohs(fd->flow_flags_mask);
		out_tx_flow->user_conf_flags = ntohl(fd->user_conf_flags);
		out_tx_flow->user_conf_mask = ntohl(fd->user_conf_mask);
		out_tx_flow->peer_flow_id = ntohs(fd->peer_flow_id);
		
		if (fd->peer_flow_label_offset) {
			out_tx_flow->peer_flow_label = ddp_heap_read_string(message_info, ntohs(fd->peer_flow_label_offset));
		}
		else {
			out_tx_flow->peer_flow_label = NULL;
		}
		
		memcpy(out_tx_flow->peer_device_id, fd->peer_device_id, sizeof(out_tx_flow->peer_device_id));		
		out_tx_flow->peer_process_id = ntohs(fd->peer_process_id);
		
		if (fd->peer_device_label_offset) {
			out_tx_flow->peer_device_label = ddp_heap_read_string(message_info, ntohs(fd->peer_device_label_offset));
		}
		else {
			out_tx_flow->peer_device_label = NULL;
		}			
		
		out_tx_flow->transport = fd->transport;
		out_tx_flow->avail_mask = fd->avail_mask;
		out_tx_flow->active_mask = fd->active_mask;
		out_tx_flow->sample_rate = ntohl(fd->sample_rate);
		out_tx_flow->latency = ntohl(fd->latency) / 1000;
		out_tx_flow->encoding = ntohs(fd->encoding);
		out_tx_flow->fpp = ntohs(fd->fpp);

		if (out_num_slots)
		{
			*out_num_slots = ntohs(fd->num_slots);
		}
		if (out_num_addresses)
		{
			*out_num_addresses = ntohs(fd->num_addresses);
		}
	}
	else
	{
		return AUD_ERR_NODATA;
	}

	return AUD_SUCCESS;
}

aud_error_t
ddp_read_routing_tx_flow_state_slots
(
	const ddp_message_read_info_t * message_info,
	uint16_t flow_idx,
	uint16_t * out_slots
)
{
	aud_error_t result;
	ddp_heap_read_info_t heap;
	uint16_t num_flows, element_size, i;
	ddp_routing_flow_data_t *fd;

	if (!message_info)
	{
		return AUD_ERR_INVALIDPARAMETER;
	}

	result = ddp_offset_array_payload_init_read(message_info, &heap);
	if (result != AUD_SUCCESS)
	{
		return result;
	}

	num_flows = heap.num_elements;
	element_size = sizeof(ddp_routing_flow_data_t);

	if (flow_idx >= num_flows)
	{
		return AUD_ERR_INVALIDPARAMETER;
	}

	fd = (ddp_routing_flow_data_t *)ddp_offset_array_read_element_sized(&heap, flow_idx, element_size);
	if (fd)
	{
		ddp_routing_data_array_read_t slotsRead;
		result = ddp_routing_data_array_overlay_read(message_info, (const ddp_routing_data_array_overlay_t *)&fd->num_slots, &slotsRead);
		if (result != AUD_SUCCESS)
		{
			return result;
		}

		for (i = 0; i < slotsRead.num_entries; ++i)
		{
			uint16_t const * slot = ddp_routing_data_array_read_entry(&slotsRead, (uint16_t)i);
			if (slot)
			{
				out_slots[i] = ntohs(*slot);
			}
			else
			{
				out_slots[i] = 0;
			}
		}
	}
	else
	{
		return AUD_ERR_NODATA;
	}

	return AUD_SUCCESS;
}

//----------

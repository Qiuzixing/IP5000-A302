/*
 * File     : routing_structures.h
 * Created  : June 2014
 * Author   : Andrew White <andrew.white@audinate.com>
 * Synopsis : Routines for handling DDP routing structures
 *
 * Copyright 2005-2017, Audinate Pty Ltd and/or its licensors
	Confidential
 * Audinate Copyright Header Version 1
 */

//----------

#include "ddp/routing_support.h"

//----------
//

void *
ddp_routing_data_array_overlay_allocate
(
	ddp_message_write_info_t * message_info,
	ddp_routing_data_array_overlay_t * array,
	uint16_t num_entries,
	ddp_size_t entry_size
)
{
	assert(message_info && array);
	if (num_entries && entry_size)
	{
		void * result;
		result = ddp_message_allocate_on_heap(
			message_info, &array->offset, num_entries * entry_size
		);
		if (! result)
			return NULL;

		array->num_entries = htons(num_entries);
		array->entry_size = htons(entry_size);
		return result;
	}
	else
		return NULL;
}


aud_error_t
ddp_routing_data_array_overlay_read
(
	const ddp_message_read_info_t * message_info,
	const ddp_routing_data_array_overlay_t * array_info,
	ddp_routing_data_array_read_t * read
)
{
	ddp_size_t n;
	ddp_size_t entry_size, data_size;

	assert(message_info && read && array_info);

	if (! array_info->offset)
		return AUD_ERR_NOTFOUND;

	if (! (array_info->entry_size && array_info->num_entries))
		return AUD_ERR_INVALIDPARAMETER;
		// if offset is non-null, these should be also

	n = ntohs(array_info->num_entries);
	if (! n)
	{
		read->message_info = message_info;
		read->num_entries = 0;
			// nothing else matters
		return AUD_SUCCESS;
	}

	if (! array_info->offset)
		return AUD_ERR_INVALIDDATA;
	if (! array_info->entry_size)
		return AUD_ERR_INVALIDDATA;

	entry_size = ntohs(array_info->entry_size);
	data_size = entry_size * n;

	read->array = ddp_heap_resolve_sized(
		message_info, ntohs(array_info->offset), data_size
	);
	if (! read->array)
		return AUD_ERR_INVALIDDATA;

	read->message_info = message_info;
	read->num_entries = n;
	read->entry_size = entry_size;
	return AUD_SUCCESS;
}

const void *
ddp_routing_data_array_read_entry
(
	const ddp_routing_data_array_read_t * array_info,
	uint16_t index
)
{
	uint16_t offset;

	assert(array_info && array_info->message_info);

	if (index >= array_info->num_entries)
		return NULL;

	assert(array_info->entry_size && array_info->array);
		// check here as they can be unset if num_entries == 0

	offset = index * array_info->entry_size;
	return (const void *) (((const uint8_t *) array_info->array) + offset);
}



//----------
// Shared data

aud_error_t
ddp_routing_array_allocate_payload
(
	ddp_message_write_info_t * message_info,
	ddp_array_t **array_in,
	uint16_t max_elements
)
{
	aud_error_t result;
	ddp_routing_array_payload_t * r;
	ddp_array_t * array;

	result = ddp_message_allocate_payload(message_info, sizeof(*r));
	if (result != AUD_SUCCESS)
		return result;

	r = (ddp_routing_array_payload_t *) message_info->payload;

	array = ddp_array_allocate(message_info, &r->array_offset, max_elements);
	if (! array)
		return AUD_ERR_NOMEMORY;

	if (array_in)
	{
		*array_in = array;
	}
	return result;
}

//----------
// Routing performance configuration
aud_error_t
ddp_routing_performance_payload_response__set_performance
(
	ddp_routing_performance_response_payload_t * p,
	ddp_routing_performance__ptype_t ptype,
	uint32_t latency,
	uint16_t fpp
)
{
	uint16_t index;
	ddp_routing_performance_payload_performance_t * perf;
	uint16_t valid_flags = 0;

	if ((!p) || (!ptype) || (ptype >= DDP_ROUTING_PERFORMANCE__Count))
		return AUD_ERR_INVALIDPARAMETER;

	index = (uint16_t)ptype - 1;
	perf = (&p->unicast) + index;
	if (latency)
	{
		perf->latency = htonl(latency);
		valid_flags |= DDP_ROUTING_PERFORMANCE_VALID_FLAG__LATENCY;
	}
	if (fpp)
	{
		perf->fpp = htons(fpp);
		valid_flags |= DDP_ROUTING_PERFORMANCE_VALID_FLAG__FPP;
	}
	perf->flags = htons(valid_flags);
	AUD_FLAG_SET(p->element_flags, htons(1 << index));
	p->element_size = htons(sizeof(*perf));

	return AUD_SUCCESS;
}

//----------
// Channels

ddp_routing_rx_channel_data_t *
ddp_routing_add_rx_channel
(
	ddp_message_write_info_t * message_info,
	ddp_array_t * array,
	uint16_t channel_id,
	const char * name,
	const char * label
)
{
	ddp_routing_rx_channel_data_t * rxc =
		ddp_array_allocate_element(message_info, array, sizeof(*rxc));
	if (! rxc)
		return NULL;

	rxc->size = htons(sizeof(*rxc));
	rxc->channel_id = htons(channel_id);
	if (name)
	{
		if (! ddp_message_allocate_string(message_info, &rxc->name_offset, name))
			return NULL;
	}
	if (label)
	{
		//Case 8078 - fixed incorrect label / name for rx channel state
		//NOTE: we do a strcmp rather than a strcasecmp as we actually care about the case as this is
		//		what the customer sees, and Dante Device Protocol is a customer facing API
		if (name && strcmp(name, label) == 0){
			rxc->label_offset = rxc->name_offset;
		}
		else {
			if (!ddp_message_allocate_string(message_info, &rxc->label_offset, label))
				return NULL;
		}
	}


	return rxc;
}

ddp_routing_tx_channel_data_t *
ddp_routing_add_tx_channel
(
	ddp_message_write_info_t * message_info,
	ddp_array_t * array,
	uint16_t channel_id,
	const char * name
)
{
	ddp_routing_tx_channel_data_t * txc =
		ddp_array_allocate_element(message_info, array, sizeof(*txc));
	if (! txc)
		return NULL;

	txc->size = htons(sizeof(*txc));
	txc->channel_id = htons(channel_id);
	if (name)
	{
		if (! ddp_message_allocate_string(message_info, &txc->name_offset, name))
			return NULL;
	}

	return txc;
}

//----------
// Flows

ddp_routing_flow_data_t *
ddp_routing_add_flow
(
	ddp_message_write_info_t * message_info,
	ddp_array_t * array,
	uint16_t flow_id,
	const char * label
)
{
	ddp_routing_flow_data_t * fd =
		ddp_array_allocate_element(message_info, array, sizeof(*fd));
	if (! fd)
		return NULL;

	fd->size = htons(sizeof(*fd));
	fd->flow_id = htons(flow_id);
	if (label)
	{
		if (! ddp_message_allocate_string(message_info, &fd->label_offset, label))
			return NULL;
	}

	return fd;
}


aud_error_t
ddp_routing_rx_flow_slots_allocate
(
	ddp_message_write_info_t * message_info,
	ddp_routing_rx_flow_slots_write_t * slots,
	ddp_routing_data_array_overlay_t * array_info,
	uint16_t num_slots
)
{
	assert(message_info && slots && array_info);

	if (! num_slots)
		return AUD_ERR_INVALIDPARAMETER;

	slots->slots = ddp_routing_data_array_overlay_allocate(
		message_info, array_info, num_slots, sizeof(uint16_t)
	);
	if (slots->slots)
	{
		slots->message_info = message_info;
		slots->num_slots = (uint16_t) num_slots;
		return AUD_SUCCESS;
	}
	else
		return AUD_ERR_NOMEMORY;
}

uint16_t *
ddp_routing_rx_flow_slots_allocate_channels_at_slot
(
	ddp_routing_rx_flow_slots_write_t * slots,
	uint16_t slot_index,
	uint16_t num_channels
)
{
	uint16_t * chan_array;

	assert(slots && slots->message_info && slots->slots && slots->num_slots);

	if (slot_index >= slots->num_slots || ! num_channels)
		return NULL;

	chan_array = ddp_message_allocate_on_heap(
		slots->message_info,
		&slots->slots[slot_index],
		(num_channels + 1) * sizeof(slots->slots[slot_index])
	);
	if (! chan_array)
		return NULL;

	// mask in the flag bit
	slots->slots[slot_index] |= htons(DDP_ROUTING_RX_FLOW_SLOT__OFFSET_FLAG);

	// return pointer to the first channel
	chan_array[0] = htons(num_channels);
	return chan_array + 1;
}



aud_error_t
ddp_routing_rx_flow_slots_read
(
	const ddp_message_read_info_t * message_info,
	ddp_routing_rx_flow_slots_read_t * slots,
	const ddp_routing_data_array_overlay_t * array_info
)
{
	aud_error_t result;
	ddp_routing_data_array_read_t array_read;

	assert(message_info && slots && array_info);

	result = ddp_routing_data_array_overlay_read(message_info, array_info, &array_read);
	if (result != AUD_SUCCESS)
		return result;

	if (! array_read.num_entries)
		return AUD_ERR_INVALIDDATA;
	if (array_read.entry_size != sizeof(uint16_t))
		return AUD_ERR_INVALIDDATA;
			// not sure how to parse this

	slots->slots = array_read.array;
	slots->message_info = array_read.message_info;
	slots->num_slots = array_read.num_entries;
	return AUD_SUCCESS;
}


int
ddp_routing_rx_flow_slots_read_slot
(
	const ddp_routing_rx_flow_slots_read_t * slots,
	uint16_t slot_index,
	const uint16_t ** out_slot_array
)
{
	uint16_t slot;
	const uint16_t * slot_array;
	int result;
	assert(slots && slots->message_info && slots->num_slots && slots->slots);

	if (slot_index >= slots->num_slots)
		return -1;

	slot = ntohs(slots->slots[slot_index]);
	if (slot & DDP_ROUTING_RX_FLOW_SLOT__OFFSET_FLAG)
	{
		slot &= DDP_ROUTING_RX_FLOW_SLOT__MASK;
		if (! slot)
		{
			slot_array = NULL;
			result = 0;
			goto l__done;
		}
		slot_array = ddp_heap_resolve_sized(
			slots->message_info, slot, sizeof(slot_array[0])
		);
		if (! slot_array)
			return -1;
		if (slot_array[0])
		{
			result = ntohs(slot_array[0]);
			slot_array ++;
		}
		else
		{
			result = 0;
			slot_array = NULL;
		}
	}
	else if (slot)
	{
		slot_array = &slots->slots[slot_index];
		result = 1;
	}
	else
	{
		slot_array = NULL;
		result = 0;
	}

l__done:
	if (out_slot_array)
	{
		* out_slot_array = slot_array;
	}
	return result;
}


//----------

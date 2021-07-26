/*
 * File     : routing_support.h
 * Created  : June 2014
 * Author   : Andrew White <andrew.white@audinate.com>
 * Synopsis : Wrappers for DDP routing handling
 *
 * Copyright 2005-2017, Audinate Pty Ltd and/or its licensors
	Confidential
 * Audinate Copyright Header Version 1
 */
#ifndef _DDP_ROUTING_SUPPORT__H
#define _DDP_ROUTING_SUPPORT__H


//----------
// Include

#include "ddp/routing_client.h"
#include "ddp/ddp_shared.h"
#include "ddp/routing_structures.h"

#ifdef __cplusplus
extern "C" {
#endif

//! @cond Doxygen_Suppress

typedef ddp_offset_array_payload_t ddp_routing_array_payload_t;

//----------
// Utilities

AUD_INLINE const char *
ddp_routing_support_string_from_raw_offset
(
	const ddp_message_read_info_t * message_info,
	uint16_t raw_offset
)
{
	if (raw_offset)
		return ddp_heap_read_string(message_info, ntohs(raw_offset));
	else
		return NULL;
}


//----------
// Queries

typedef struct ddp_routing_support_init_query
{
	ddp_message_write_info_t message;
	ddp_message_id_ranges_write_t ranges;
} ddp_routing_support_init_query_t;

aud_error_t
ddp_routing_support_init_query
(
	ddp_packet_write_info_t * packet_info,	// in
	ddp_routing_support_init_query_t * query_info,	// out
	uint16_t opcode,
	uint16_t request_id,
	ddp_size_t max_ranges
		// may be zero for incrementally growing packet
);

aud_error_t
ddp_routing_support_query_add_range
(
	ddp_routing_support_init_query_t * query_info,
	dante_id_t from, dante_id_t to
);

AUD_INLINE aud_error_t
ddp_routing_support_query_add_ident
(
	ddp_routing_support_init_query_t * query_info,
	dante_id_t ident
)
{
	return ddp_routing_support_query_add_range(query_info, ident, ident);
}


// The same message can be used for delete.  When deleting flows, we need to
// provide a filter.

aud_error_t
ddp_routing_support_init_flow_delete
(
	ddp_packet_write_info_t * packet_info,	// in
	ddp_routing_support_init_query_t * query_info,	// out
	uint16_t request_id,
	ddp_size_t max_ranges,
		// may be zero for incrementally growing packet
	uint16_t filter
);


AUD_INLINE aud_error_t
ddp_routing_support_init_channel_delete
(
	ddp_packet_write_info_t * packet_info,	// in
	ddp_routing_support_init_query_t * query_info,	// out
	uint16_t request_id,
	ddp_size_t max_ranges
		// may be zero for incrementally growing packet
)
{
	return ddp_routing_support_init_query (
		packet_info, query_info, DDP_OP_ROUTING_RX_UNSUB_CHAN, request_id, max_ranges
	);
}

aud_error_t
ddp_routing_support_flow_delete_set_filter
(
	ddp_routing_support_init_query_t * query_info,
	uint16_t filter
);


typedef struct ddp_routing_rx_flow_slots_write
{
	ddp_message_write_info_t * message_info;
	uint16_t num_slots;
	uint16_t * slots;
} ddp_routing_rx_flow_slots_write_t;

// To read, call ddp_message_id_range_validate on the message_info
// A non-null return value is a header followed by an array of ranges.


//----------
// Flows

// Writing single flow config request packets

typedef struct ddp_routing_support_init_flow
{
	ddp_message_write_info_t message;
	ddp_routing_flow_data_t * flow;
	uint16_t num_slots, num_addrs;
	union
	{
		ddp_routing_rx_flow_slots_write_t rx;
		uint16_t * tx;
	} slots;
	ddp_addr_inet_t * addrs;
} ddp_routing_support_init_flow_t;


/*
	Input: an initialised packet, uninitialised message info
	Output: empty but initialised flow data structure
 */
aud_error_t
ddp_routing_support_init_tx_flow_mcast_request
(
	ddp_packet_write_info_t * packet_info,	// in
	ddp_routing_support_init_flow_t * flow_info,	// out
	uint16_t request_id,	// in
	dante_id_t flow_id,	// in
	uint16_t num_slots,	// in
	uint16_t num_addrs	// in
);

aud_error_t
ddp_routing_support_flow_set_label
(
	ddp_routing_support_init_flow_t * flow,
	const char * label
);


// Reading flow configs

typedef struct ddp_routing_rx_flow_slots_read
{
	const ddp_message_read_info_t * message_info;
	uint16_t num_slots;
	const uint16_t * slots;
} ddp_routing_rx_flow_slots_read_t;

typedef struct ddp_routing_support_read_flows
{
	ddp_heap_read_info_t read;
	struct
	{
		unsigned index;
		const ddp_routing_flow_data_t * flow;
		unsigned num_slots, num_addrs;
		union
		{
			ddp_routing_rx_flow_slots_read_t rx;
			const uint16_t * tx;
		} slots;
		const ddp_addr_inet_t * addr;
	} curr_flow;
} ddp_routing_support_read_flows_t;

aud_error_t
ddp_routing_support_read_tx_flow_at_index
(
	ddp_message_read_info_t * message_info,
	ddp_routing_support_read_flows_t * flows,
	unsigned index
);

aud_error_t
ddp_routing_support_read_rx_flow_at_index
(
	ddp_message_read_info_t * message_info,
	ddp_routing_support_read_flows_t * flows,
	unsigned index
);


//----------
// Channels

// Reading channel configs

typedef struct ddp_routing_support_read_channels
{
	ddp_heap_read_info_t read;
	struct
	{
		unsigned index;
		union
		{
			const ddp_routing_rx_channel_data_t * rx;
			const ddp_routing_tx_channel_data_t * tx;
		} channel;
		const ddp_routing_channel_format_t * format;
		const char * name;
		const char * label;
		// rx only
		const char * sub_channel;
		const char * sub_device;
	} curr_chan;
} ddp_routing_support_read_channels_t;

aud_error_t
ddp_routing_support_read_rx_channel_at_index
(
	ddp_message_read_info_t * message_info,
	ddp_routing_support_read_channels_t * channel_info,
	unsigned index
);

aud_error_t
ddp_routing_support_read_tx_channel_at_index
(
	ddp_message_read_info_t * message_info,
	ddp_routing_support_read_channels_t * channel_info,
	unsigned index
);


//----------
// Subscriptions & labels

typedef struct ddp_routing_subscription_config
{
	ddp_name_id_pair_t local_channel;			/*!< Non-zero Rx channel ID */
	uint16_t subscribed_channel_offset;			/*!< Offset from the start of the TLV0 header to the start of the non-zero Tx channel ID */
	uint16_t subscribed_device_offset;			/*!< Offset from the start of the TLV0 header to the start of the null-terminated Tx device name */
} ddp_routing_subscription_config_t;

typedef struct ddp_routing_support_init_array
{
	ddp_message_write_info_t message;
	union
	{
		void * array;
		ddp_routing_subscription_config_t * subs;
		ddp_name_id_pair_t * labels;
	} array;
	unsigned max;
	unsigned num;
} ddp_routing_support_init_array_t;

aud_error_t
ddp_routing_support_init_array
(
	ddp_packet_write_info_t * packet_info,	// in
	ddp_routing_support_init_array_t * array,
	uint16_t opcode,
	uint16_t request_id,
	ddp_size_t elem_size,
	uint16_t num_labels
);


// Subscriptions

typedef ddp_routing_support_init_array_t ddp_routing_support_init_subs_t;

AUD_INLINE aud_error_t
ddp_routing_support_init_subs
(
	ddp_packet_write_info_t * packet_info,	// in
	ddp_routing_support_init_subs_t * subs,
	uint16_t opcode,
	uint16_t request_id,
	uint16_t num_subs
)
{
	return ddp_routing_support_init_array(
		packet_info, subs, opcode, request_id, sizeof(subs->array.subs[0]), num_subs
	);
}

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


// Labels

typedef ddp_routing_support_init_array_t ddp_routing_support_init_labels_t;

AUD_INLINE aud_error_t
ddp_routing_support_init_labels
(
	ddp_packet_write_info_t * packet_info,	// in
	ddp_routing_support_init_labels_t * labels,
	uint16_t opcode,
	uint16_t request_id,
	uint16_t num_labels
)
{
	return ddp_routing_support_init_array(
		packet_info, labels, opcode, request_id, sizeof(labels->array.labels[0]), num_labels
	);
}

/*
	Add a label entry

	label can be null to clear an existing label on device
 */
aud_error_t
ddp_routing_support_labels_add_label
(
	ddp_routing_support_init_labels_t * labels,
	dante_id_t channel_id,
	const char * label
);

//----------
// Shared data

aud_error_t
ddp_routing_array_allocate_payload
(
	ddp_message_write_info_t * message_info,
	ddp_array_t **array_in,
	uint16_t max_elements
);

//----------
// Routing performance configuration

#if 0
typedef enum ddp_routing_performance__ptype
{
	DDP_ROUTING_PERFORMANCE__UNDEF = 0,
	DDP_ROUTING_PERFORMANCE__UNICAST,
	DDP_ROUTING_PERFORMANCE__MULTICAST,

	DDP_ROUTING_PERFORMANCE__Count
} ddp_routing_performance__ptype_t;

aud_error_t
ddp_routing_performance_payload_response__set_performance
(
	ddp_routing_performance_response_payload_t * p,
	ddp_routing_performance__ptype_t ptype,
	uint32_t latency,
	uint16_t fpp
);
#endif // 0

//----------
// Channels

ddp_routing_tx_channel_data_t *
ddp_routing_add_tx_channel
(
	ddp_message_write_info_t * message_info,
	ddp_array_t * array,
	uint16_t channel_id,
	const char * name
);

ddp_routing_rx_channel_data_t *
ddp_routing_add_rx_channel
(
	ddp_message_write_info_t * message_info,
	ddp_array_t * array,
	uint16_t channel_id,
	const char * name,
	const char * label
);

// RX flow slots
/*
RX flow slots need to be efficient in the case that we have only 0 or 1
channel per slot but handle a possibly indefinite number of channels.

The base structure is an array of uint16_t slots.  If the high bit of an
element is zero, the value is interpreted as a channel identifier.  If the
high bit is 1, the low 15 bits are interpreted as an offset to an array of
uint16_t channel identifiers.  The first element of this array is interpreted
as the length of the rest of the array (not as an identifier).

Example: 4 slots -> 4x uint16
0000 -> empty slot
0010 -> channel 16
1100 -> offset to location 0x100
0x100: 3 1 2 3 -> 3 channels: 1,2,3
1108 -> offset to location 0x108
0x108: 0 -> no channels
Note: the fourth entry is inefficient but legal.
*/
enum
{
	DDP_ROUTING_RX_FLOW_SLOT__OFFSET_FLAG = 0x8000,
	DDP_ROUTING_RX_FLOW_SLOT__MASK = 0x7fff
};

//----------
// Data arrays

/*
Overlay for use when dealing with nested arrays, such as in the flow_data
structure.  Cast the address of num_* to one of these.

Assumes the fields in the structure are laid out in exactly this order.

Do not instantiate this structure directly.  All fields are in network byte
order.
*/
typedef struct ddp_routing_data_array_overlay
{
	uint16_t num_entries;
	uint16_t entry_size;
	uint16_t offset;
} ddp_routing_data_array_overlay_t;


void *
ddp_routing_data_array_overlay_allocate
(
	ddp_message_write_info_t * message_info,
	ddp_routing_data_array_overlay_t * array_info,
	uint16_t num_entries,
	ddp_size_t entry_size
);


typedef struct ddp_routing_data_array_read
{
	const ddp_message_read_info_t * message_info;
	uint16_t num_entries;
	ddp_size_t entry_size;
	const void * array;
} ddp_routing_data_array_read_t;

/*
Prepare an array for reading.

This call does not verify alignment or entry size.  On success, caller
should verify the returned values before calling ddp_routing_data_array_read.
*/
aud_error_t
ddp_routing_data_array_overlay_read
(
	const ddp_message_read_info_t * message_info,
	const ddp_routing_data_array_overlay_t * array_info,
	ddp_routing_data_array_read_t * read
);

const void *
ddp_routing_data_array_read_entry
(
	const ddp_routing_data_array_read_t * array_info,
	uint16_t index
);

//----------
// Flows

ddp_routing_flow_data_t *
ddp_routing_add_flow
(
	ddp_message_write_info_t * message_info,
	ddp_array_t * array,
	uint16_t flow_id,
	const char * label
);

/*
Initialise a structure to add channels to slots.

All slots are initialised to contain no channels.
*/
aud_error_t
ddp_routing_rx_flow_slots_allocate
(
	ddp_message_write_info_t * message_info,
	ddp_routing_rx_flow_slots_write_t * slots,
	ddp_routing_data_array_overlay_t * array_info,
	uint16_t num_slots
);

// Set a slot to contain a single channel
AUD_INLINE aud_bool_t
ddp_routing_rx_flow_slots_set_channel_at_slot
(
	ddp_routing_rx_flow_slots_write_t * slots,
	uint16_t slot_index,
	uint16_t channel_id
)
{
	assert(slots && slots->message_info && slots->slots && slots->num_slots);

	if (slot_index < slots->num_slots &&
		!(channel_id & DDP_ROUTING_RX_FLOW_SLOT__OFFSET_FLAG)
		)
	{
		slots->slots[slot_index] = htons(channel_id);
		return AUD_TRUE;
	}
	else
		return AUD_FALSE;
}

/*
Set a slot to contain an array of channels

Return pointer to the channels portion of the array (skips the length element).
Array is num_channels * uint16_t in length.
Channels can be filled in by writing to array. Must use htons when doing so.
Array elements are initialised to the null channel.
*/
uint16_t *
ddp_routing_rx_flow_slots_allocate_channels_at_slot
(
	ddp_routing_rx_flow_slots_write_t * slots,
	uint16_t slot_index,
	uint16_t num_channels
);

aud_error_t
ddp_routing_rx_flow_slots_read
(
	const ddp_message_read_info_t * message_info,
	ddp_routing_rx_flow_slots_read_t * slots,
	const ddp_routing_data_array_overlay_t * array_info
);

/*
Get a pointer to the channels at a particular slot.

Returns number of channel entries in slot (on success),
or negative on failure.

If slot_array is null, will just return the number of entries available.
If result is zero & slot_array is non-null, slot_array will contain NULL.
If result is negative, slot_array will be unmodified.
*/
int
ddp_routing_rx_flow_slots_read_slot
(
	const ddp_routing_rx_flow_slots_read_t * slots,
	uint16_t slot_index,
	const uint16_t ** slot_array
);

aud_error_t
ddp_read_routing_rx_chan_state
(
	ddp_message_read_info_t const * message_info,
	uint16_t  chan_idx,
	ddp_rx_chan_state_params_t * out_rx_chan_data,
	uint16_t *out_num_custom_encodings
);

aud_error_t
ddp_read_routing_rx_chan_custom_encoding
(
	ddp_message_read_info_t const * message_info,
	uint16_t chan_idx,
	uint16_t custom_enc_idx,
	uint16_t * out_custom_encoding
);

aud_error_t
ddp_read_routing_tx_chan_state
(
	ddp_message_read_info_t const * message_info,
	uint16_t chan_idx,
	ddp_tx_chan_state_params_t * out_tx_chan_data,
	uint16_t * out_num_custom_encodings
);

aud_error_t
ddp_read_routing_tx_chan_custom_encoding
(
	ddp_message_read_info_t const * message_info, 
	uint16_t chan_idx, 
	uint16_t custom_enc_idx, 
	uint16_t * out_custom_encoding
);

aud_error_t
ddp_read_routing_rx_flow_state
(
	const ddp_message_read_info_t * message_info,
	uint16_t flow_idx,
	uint16_t * out_num_slots,
	uint16_t * out_num_addresses,
	ddp_rx_flow_state_params_t * out_flow
);

aud_error_t
ddp_read_routing_rx_flow_state_slot
(
	const ddp_message_read_info_t * message_info,
	uint16_t flow_idx,
	uint16_t slot_idx,
	uint16_t * out_num_chans
);

aud_error_t
ddp_read_routing_rx_flow_state_slot_chans
(
	const ddp_message_read_info_t * message_info,
	uint16_t flow_idx,
	uint16_t slot_idx,
	uint16_t * out_chans
);

aud_error_t
ddp_read_routing_flow_state_address
(
	const ddp_message_read_info_t * message_info,
	uint16_t flow_idx,
	uint16_t address_idx,
	ddp_ip_addr_nw_t * out_ip_address,
	uint16_t * out_port
);

aud_error_t
ddp_read_routing_tx_flow_state
(
	const ddp_message_read_info_t * message_info,
	uint16_t flow_idx,
	uint16_t * out_num_slots,
	uint16_t * out_num_addresses,
	ddp_tx_flow_state_params_t * out_tx_flow
);

aud_error_t
ddp_read_routing_tx_flow_state_slots
(
	const ddp_message_read_info_t * message_info,
	uint16_t flow_idx,
	uint16_t * out_slots
);

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
);

aud_error_t
init_flow_request_common
(
	ddp_packet_write_info_t * packet_info,
	ddp_routing_support_init_flow_t * flow_info,
	uint16_t request_id,
	uint16_t opcode,
	dante_id_t flow_id,	// in
	uint16_t num_addrs
);

typedef struct ddp_routing_flow_config_request_payload
{
	ddp_routing_flow_data_t flow;
} ddp_routing_flow_config_request_payload_t;

typedef ddp_routing_performance_payload_t
ddp_routing_performance_request_payload_t,
ddp_routing_performance_response_payload_t;

typedef enum ddp_routing_performance__ptype
{
	DDP_ROUTING_PERFORMANCE__UNDEF = 0,
	DDP_ROUTING_PERFORMANCE__UNICAST,
	DDP_ROUTING_PERFORMANCE__MULTICAST,

	DDP_ROUTING_PERFORMANCE__Count
} ddp_routing_performance__ptype_t;

aud_error_t
ddp_routing_performance_payload_response__set_performance
(
	ddp_routing_performance_response_payload_t * p,
	ddp_routing_performance__ptype_t ptype,
	uint32_t latency,
	uint16_t fpp
);

//! @endcond

#ifdef __cplusplus
}
#endif

#endif // _DDP_ROUTING_SUPPORT__H

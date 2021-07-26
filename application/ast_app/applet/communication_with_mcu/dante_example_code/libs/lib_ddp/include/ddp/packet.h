/*
* File     : packet.h
* Created  : August 2014
* Updated  : Date: 2014/08/22
* Author   : Michael Ung <michael.ung@audinate.com>
* Synopsis : All top level DDP packet client helper functions.
*
* Copyright 2005-2017, Audinate Pty Ltd and/or its licensors
Confidential
* Audinate Copyright Header Version 1
*/

#ifndef _DDP_PACKET_H
#define _DDP_PACKET_H

#include <arpa/inet.h>
#include "ddp/ddp.h"

#ifdef __cplusplus
extern "C" {
#endif

//! @cond Doxygen_Suppress

//----------------------------------------------------------
// DDP size
//----------------------------------------------------------
typedef uint16_t ddp_size_t;

AUD_INLINE ddp_size_t
ddp_size_bytes_to_uint32
(
	ddp_size_t bytes
)
{
	return bytes >> 2;
}

AUD_INLINE ddp_size_t
ddp_normalise_length_bytes
(
	ddp_size_t bytes
)
{
	if (bytes & 0x3)
	{
		bytes = (bytes + 3) & ~0x3;
	}
	return bytes;
}

//----------------------------------------------------------
// offset / raw offset conversion helpers helpers
//----------------------------------------------------------
AUD_INLINE ddp_offset_t
ddp_raw_offset_to_ddp_offset
(
	ddp_raw_offset_t raw_offset
)
{
	return (ddp_offset_t)(ntohs(raw_offset));
}

AUD_INLINE ddp_raw_offset_t
ddp_offset_to_ddp_raw_offset
(
	ddp_offset_t offset
)
{
	return (ddp_raw_offset_t)(htons(offset));
}

//----------------------------------------------------------
// Tweaking message headers
//----------------------------------------------------------

AUD_INLINE ddp_size_t
ddp_message_get_length_bytes
(
	const ddp_message_header_t * m
)
{
	return ntohs(m->block._.length_bytes);
}

AUD_INLINE void
ddp_message_set_length_bytes
(
	ddp_message_header_t * m,
	ddp_size_t length_bytes
)
{
	m->block._.length_bytes = htons(length_bytes);
}


AUD_INLINE ddp_size_t
ddp_message_get_length_uint32
(
	const ddp_message_header_t * m
)
{
	return ddp_size_bytes_to_uint32(ddp_message_get_length_bytes(m));
}

//----------------------------------------------------------
// Generic Packet functions
//----------------------------------------------------------

AUD_INLINE ddp_size_t
ddp_packet_header_get_length_bytes
(
	const ddp_packet_header_t * p
)
{
	return ntohs(p->block._.length_bytes);
}

AUD_INLINE ddp_opcode_t
ddp_packet_header_get_opcode
(
	const ddp_packet_header_t * p
)
{
	return ntohs(p->block._.opcode);
}

typedef struct ddp_packet_write_info
{
	union
	{
		uint8_t * buf8;
		uint16_t * buf16;
		uint32_t * buf32;
		ddp_packet_header_t * header;
	} _;

	ddp_size_t max_length_bytes, curr_length_bytes;
} ddp_packet_write_info_t;

aud_error_t
ddp_packet_init_write
(
	ddp_packet_write_info_t * packet_info,
	uint32_t * buf,
	ddp_size_t buf_len
);

aud_error_t
ddp_packet_init_write_requests
(
	ddp_packet_write_info_t * packet_info,
	uint32_t * buf,
	ddp_size_t buf_len
);

aud_error_t
ddp_packet_init_write_responses
(
	ddp_packet_write_info_t * packet_info,
	uint32_t * buf,
	ddp_size_t buf_len
);


// Manipulating length

AUD_INLINE ddp_size_t
ddp_packet_write_get_length_bytes
(
	const ddp_packet_write_info_t * packet_info
)
{
	return packet_info->curr_length_bytes;
}

AUD_INLINE ddp_size_t
ddp_packet_write_get_free_bytes
(
	const ddp_packet_write_info_t * packet_info
)
{
	return packet_info->max_length_bytes - ddp_packet_write_get_length_bytes(packet_info);
}

AUD_INLINE ddp_size_t
ddp_packet_write_get_length_uint32
(
	const ddp_packet_write_info_t * packet_info
)
{
	return ddp_size_bytes_to_uint32(ddp_packet_write_get_length_bytes(packet_info));
}

AUD_INLINE uint32_t *
ddp_packet_write_get_tail
(
	ddp_packet_write_info_t * packet_info
)
{
	unsigned offset = ddp_packet_write_get_length_uint32(packet_info);
	return packet_info->_.buf32 + offset;
}


/*
	Attempt to grow the packet by the indicated number of bytes.
	On success, returns pointer to previous tail.
	On failure, returns NULL.
 */
uint32_t *
ddp_packet_write_grow_packet
(
	ddp_packet_write_info_t * packet_info,
	ddp_size_t bytes
);


typedef struct ddp_packet_read_info
{
	uint16_t packet_length_bytes;
	union
	{
		const uint8_t * buf8;
		const uint16_t * buf16;
		const uint32_t * buf32;
		const ddp_packet_header_t * header;
	} _;
	ddp_version_t version;
} ddp_packet_read_info_t;

aud_error_t
ddp_packet_init_read
(
	ddp_packet_read_info_t * packet_info,
	const uint32_t * buf,
	ddp_size_t len
);

aud_error_t
ddp_packet_init_read_from_write
(
	ddp_packet_read_info_t * packet_info,
	const ddp_packet_write_info_t * packet_write_info
);


/*
	Init packet for reading, verify header, and return offset of first message.
 */
aud_error_t
ddp_packet_first_message_offset
(
	ddp_packet_read_info_t * packet_info,
	ddp_offset_t * first_message_offset_p,
	const uint32_t * buf,
	size_t buflen
);


aud_error_t
ddp_packet_next_message_offset
(
	const ddp_packet_read_info_t * packet_info,
	ddp_offset_t * message_offset_p,
	ddp_offset_t curr_offset
);


AUD_INLINE ddp_size_t
ddp_packet_read_get_header_length
(
	const ddp_packet_read_info_t * packet_info
)
{
	return ddp_packet_header_get_length_bytes(packet_info->_.header);
}

AUD_INLINE ddp_opcode_t
ddp_packet_read_get_opcode
(
	const ddp_packet_read_info_t * packet_info
)
{
	return ddp_packet_header_get_opcode(packet_info->_.header);
}


//----------------------------------------------------------
// Packet flags
//----------------------------------------------------------

// Direction

AUD_INLINE uint8_t
ddp_packet_write_get_direction
(
	const ddp_packet_write_info_t * packet_info
)
{
	return packet_info->_.header->_.ext.flags & DDP_PACKET_HEADER_FLAG_DIRECTION_MASK;
}


AUD_INLINE uint8_t
ddp_packet_read_get_direction
(
	const ddp_packet_read_info_t * packet_info
)
{
	return packet_info->_.header->_.ext.flags & DDP_PACKET_HEADER_FLAG_DIRECTION_MASK;
}


// Verifying direction

aud_bool_t
ddp_packet_write_direction_is_valid
(
	const ddp_packet_write_info_t * packet_info,
	uint8_t direction_flag
);


/**
	Check that direction either matches the input or is undefined.
	If undefined, update it.
 */
aud_bool_t
ddp_packet_write_apply_direction
(
	ddp_packet_write_info_t * packet_info,
	uint8_t direction_flag
);


// Infrastructure

AUD_INLINE aud_bool_t
ddp_packet_read_is_link_infrastructure
(
	const ddp_packet_read_info_t * packet_info
)
{
	return (packet_info->_.header->_.ext.flags & DDP_PACKET_HEADER_FLAG_LINK_INFRASTRUCTURE)
		? AUD_TRUE : AUD_FALSE;
}

void
ddp_packet_write_set_link_infrastructure
(
	ddp_packet_write_info_t * packet_info
);


//----------------------------------------------------------
// Generic heap put/get functions
//----------------------------------------------------------

typedef struct ddp_message_write_info
{
	ddp_packet_write_info_t * packet;

	union
	{
		uint32_t * buf32;
		ddp_message_header_t * header;
	} _;

	union
	{
		uint32_t * buf;
		ddp_request_subheader_t * request;
		ddp_response_subheader_t * response;
		ddp_monitoring_subheader_t * monitoring;
		ddp_local_subheader_t * local;
	} subheader;
	uint32_t * payload;
	uint32_t * heap;
} ddp_message_write_info_t;


AUD_INLINE uint32_t *
ddp_message_write_get_tail
(
	ddp_message_write_info_t * message_info
)
{
	unsigned offset = ddp_message_get_length_uint32(message_info->_.header);
	return message_info->_.buf32 + offset;
}

// Manipulating length

AUD_INLINE ddp_size_t
ddp_message_write_get_length_bytes
(
	const ddp_message_write_info_t * message_info
)
{
	return ddp_message_get_length_bytes(message_info->_.header);
}

AUD_INLINE ddp_size_t
ddp_message_write_get_free_bytes
(
	const ddp_message_write_info_t * message_info
)
{
	return message_info->packet->max_length_bytes -
		ddp_message_write_get_length_bytes(message_info);
}

/*
	Attempt to grow the message by the indicated number of bytes.
	On success, returns pointer to previous tail.
	On failure, returns NULL.
 */
uint32_t *
ddp_message_write_grow_packet
(
	ddp_message_write_info_t * message_info,
	ddp_size_t bytes
);


// Full message allocation for writing

aud_error_t
ddp_packet_allocate_message
(
	ddp_packet_write_info_t * packet_info,
	ddp_message_write_info_t * message_info,
	uint16_t opcode,
	uint16_t subheader_length_bytes,
	uint16_t payload_length_bytes
);

aud_error_t
ddp_packet_allocate_request
(
	ddp_packet_write_info_t * packet_info,
	ddp_message_write_info_t * message_info,
	uint16_t opcode,
	ddp_request_id_t request_id,
	uint16_t payload_length_bytes
);

aud_error_t
ddp_packet_allocate_response
(
	ddp_packet_write_info_t * packet_info,
	ddp_message_write_info_t * message_info,
	uint16_t opcode,
	ddp_request_id_t request_id,
	ddp_status_t status,
	uint16_t payload_length_bytes
);

aud_error_t
ddp_packet_allocate_monitoring
(
	ddp_packet_write_info_t * packet_info,
	ddp_message_write_info_t * message_info,
	uint16_t opcode,
	uint16_t monitoring_seqnum,
	uint16_t payload_length_bytes
);

aud_error_t
ddp_packet_allocate_local
(
	ddp_packet_write_info_t * packet_info,
	ddp_message_write_info_t * message_info,
	uint16_t opcode,
	const ddp_event_timestamp_t* event_timestamp,
	uint16_t payload_length_bytes
);


// Partial allocation for writing

aud_error_t
ddp_packet_allocate_message_head
(
	ddp_packet_write_info_t * packet_info,
	ddp_message_write_info_t * message_info,
	uint16_t opcode,
	ddp_size_t subheader_length_bytes
);

aud_error_t
ddp_packet_allocate_request_head
(
	ddp_packet_write_info_t * packet_info,
	ddp_message_write_info_t * message_info,
	uint16_t opcode,
	uint16_t request_id
);

aud_error_t
ddp_packet_allocate_response_head
(
	ddp_packet_write_info_t * packet_info,
	ddp_message_write_info_t * message_info,
	uint16_t opcode,
	uint16_t request_id
);

AUD_INLINE aud_error_t
ddp_packet_allocate_event_head
(
	ddp_packet_write_info_t * packet_info,
	ddp_message_write_info_t * message_info,
	uint16_t opcode
)
{
	return ddp_packet_allocate_response_head(packet_info, message_info, opcode, 0);
}

aud_error_t
ddp_packet_allocate_local_head
(
	ddp_packet_write_info_t * packet_info,
	ddp_message_write_info_t * message_info,
	uint16_t opcode,
	const ddp_event_timestamp_t* event_timestamp
);


/*
	Add space for a payload to an existing partially allocated message.
	Pointer to the payload can be extracted from the message info structure.
	This call also initialises the heap.
 */
aud_error_t
ddp_message_allocate_payload
(
	ddp_message_write_info_t * message_info,
	ddp_size_t payload_length_bytes
);


/*
	Allocate a block of memory on the heap, and zero it
	Offset to memory is written into 'offset'
	On failure, returns NULL and 0 is written to offset
 */
void *
ddp_message_allocate_on_heap
(
	ddp_message_write_info_t * message_info,
	ddp_raw_offset_t * offset,
	ddp_size_t heap_bytes
);

/*
	Allocate a block of memory on the heap, but don't zero it
	Offset to memory is written into 'offset'
	On failure, returns NULL and 0 is written to offset
 */
void *
ddp_message_allocate_on_heap_no_zero
(
	ddp_message_write_info_t * message_info,
	ddp_raw_offset_t * offset,
	ddp_size_t heap_bytes
);

/*
	Allocate a block of memory on the heap and copy the provided data
	Offset to memory is written into 'offset'
	On failure, returns NULL and 0 is written to offset
 */
void *
ddp_message_allocate_data
(
	ddp_message_write_info_t * message_info,
	ddp_raw_offset_t * offset,
	ddp_size_t data_len,
	const void * data
);

/*
	Allocate a block of memory on the heap and copy the provided string
	Offset to memory is written into 'offset'
	On failure, returns AUD_FALSE and 0 is written to offset.
	NULL string is considered an error.
 */
char *
ddp_message_allocate_string
(
	ddp_message_write_info_t * message_info,
	ddp_raw_offset_t * offset,
	const char * string
);


/*
aud_error_t
ddp_packet_write_message
(
	ddp_packet_write_info_t * packet_info,
	const ddp_block_header_t * header
);*/

typedef struct ddp_message_read_info
{
	const ddp_packet_read_info_t * packet_info;
	ddp_offset_t packet_offset_bytes;
	uint16_t length_bytes;
	uint16_t opcode;
	uint16_t subheader_length_bytes;
	uint16_t payload_length_bytes;
	uint16_t heap_length_bytes;
	union
	{
		const uint8_t  * value8;
		const uint16_t * value16;
		const uint32_t * value32;
		const ddp_message_header_t * header;
	} header;
	union
	{
		const uint8_t  * value8;
		const uint16_t * value16;
		const uint32_t * value32;
		const ddp_request_subheader_t * request;
		const ddp_response_subheader_t * response;
		const ddp_monitoring_subheader_t * monitoring;
		const ddp_local_subheader_t * local;
	} subheader;
	union
	{
		const uint8_t  * value8;
		const uint16_t * value16;
		const uint32_t * value32;
	} payload;
} ddp_message_read_info_t;

/*
 Initialise a message_read_info structure given a pointer to the start of the message.
 This function should not normally need to be called directly - use a packet_info structure and ddp_packet_read_message instead.
 This function is only required if the messages are not part of a buffer with a DDP packet header.
 NOTE: 
   Some of the functions in this file may assume that message_info->packet_info or
   message_info->packet_offset_bytes are valid, non-NULL(non-zero) values.
   You have been warned.
 */
aud_error_t
ddp_message_init_read
(
	ddp_message_read_info_t * message_info,
	const uint8_t * buf,
	size_t buflen
);

aud_error_t
ddp_packet_read_block_header
(
	const ddp_packet_read_info_t * packet_info,
	uint16_t packet_offset_bytes,
	ddp_block_header_t * block_header
);

aud_error_t
ddp_packet_read_message
(
	const ddp_packet_read_info_t * packet_info,
	ddp_message_read_info_t * message_info,
	uint16_t packet_offset_bytes
);

ddp_request_id_t
ddp_packet_read_response_request_id
(
	const ddp_message_read_info_t * message_info
);

ddp_status_t
ddp_packet_read_response_status
(
	const ddp_message_read_info_t * message_info
);

/*
	Given a packet, read the first block header, verify that it is a packet header,
	and then extract the first message.  Populates both the packet and message read
	structures.
 */
aud_error_t
ddp_packet_init_read_first_message
(
	ddp_packet_read_info_t * packet_info,
	ddp_message_read_info_t * message_info,
	const uint32_t * buf,
	size_t buflen
);


AUD_INLINE aud_error_t
ddp_packet_validate_message
(
	const ddp_message_read_info_t * message_info,
	ddp_opcode_t opcode,
	uint16_t min_length_bytes
) {
	// verify that the opcode is the one we expected
	if(ntohs(message_info->header.header->block._.opcode) != opcode)
	{
		return AUD_ERR_INVALIDDATA;
	}
	// verify that the message is big enough for what we expected...
	if (ddp_message_get_length_bytes(message_info->header.header) < min_length_bytes)
	{
		return AUD_ERR_INVALIDDATA;
	}
	return AUD_SUCCESS;
}

AUD_INLINE aud_error_t
ddp_message_validate_request_payload
(
	const ddp_message_read_info_t * message_info,
	uint16_t min_payload_length_bytes
) {
	if (message_info->subheader_length_bytes < DDP_REQUEST_SUBHEADER_MIN_SIZE)
	{
		return AUD_ERR_INVALIDDATA;
	}
	// verify that the message is big enough for what we expected...
	if (message_info->payload_length_bytes < min_payload_length_bytes)
	{
		return AUD_ERR_INVALIDDATA;
	}
	return AUD_SUCCESS;
}


/*
	Get a pointer to the end of the message.
	
	Any attempt to dereference memory beyond this pointer has exceeded the message bound.
 */
AUD_INLINE const void *
ddp_message_max_ptr
(
	const ddp_message_read_info_t * message_info
)
{
	return message_info->header.value8 + message_info->length_bytes;
}


/*
	Set the status on a ddp response_message
 */
AUD_INLINE void
ddp_message_response_set_status
(
	ddp_message_write_info_t * message_info,
	ddp_status_t status
)
{
	message_info->subheader.response->status = htons(status);
}

AUD_INLINE ddp_status_t
ddp_message_response_get_status
(
	const ddp_message_read_info_t * message_info
)
{
	return ntohs(message_info->subheader.response->status);
}


/* 
 * Sets the opcode to DDP_OP_EMPTY and clears out payload.
 * Used for clearing denied requests before forwarding.
 */
AUD_INLINE void
ddp_message_request_blank_out_content
(
	ddp_message_read_info_t* message_info
)
{
	ddp_message_header_t* header = (ddp_message_header_t *)message_info->header.header;
	header->block._.opcode = htons(DDP_OP_EMPTY);

	if (message_info->payload_length_bytes > 0)
	{
		bzero((uint8_t *)message_info->payload.value8, message_info->payload_length_bytes);
	}
}


// Quick response allocator

AUD_INLINE aud_error_t
ddp_packet_allocate_response_head_from_request
(
	ddp_packet_write_info_t * packet_info,
	ddp_message_write_info_t * message_info,
	const ddp_message_read_info_t * read_info
)
{
	return ddp_packet_allocate_response_head(
		packet_info, message_info, read_info->opcode,
		ntohs(read_info->subheader.request->request_id)
	);
}

//----------------------------------------------------------
// Heap control (for sub-message elements)
//----------------------------------------------------------

typedef struct ddp_heap_write_info
{
	ddp_offset_t msg_offset_bytes;
		// NOTE: msg_offset_bytes is in network byte order
	uint16_t num_elements;
	ddp_size_t element_size_bytes;

	ddp_size_t length_bytes;

	// The 'array' fields point the the heap data block.
	union
	{
		uint8_t  * array8;
		uint16_t * array16;
		uint32_t * array32;
	} _;
} ddp_heap_write_info_t;

aud_error_t
ddp_allocate_heap_array
(
	ddp_message_write_info_t * message_info,
	ddp_heap_write_info_t * heap_info,
	uint16_t num_elements,
	uint16_t element_size_bytes
);

AUD_INLINE aud_error_t
ddp_allocate_heap_array8
(
	ddp_message_write_info_t * message_info,
	ddp_heap_write_info_t * heap_info,
	uint16_t num_elements8
)
{
	return ddp_allocate_heap_array(message_info, heap_info, num_elements8, sizeof(uint8_t));
}

AUD_INLINE aud_error_t
ddp_allocate_heap_array16
(
	ddp_message_write_info_t * message_info,
	ddp_heap_write_info_t * heap_info,
	uint16_t num_elements16
)
{
	return ddp_allocate_heap_array(message_info, heap_info, num_elements16, sizeof(uint16_t));
}

AUD_INLINE aud_error_t
ddp_allocate_heap_array32
(
	ddp_message_write_info_t * message_info,
	ddp_heap_write_info_t * heap_info,
	uint16_t num_elements32
)
{
	return ddp_allocate_heap_array(message_info, heap_info, num_elements32, sizeof(uint32_t));
}

AUD_INLINE aud_error_t
ddp_allocate_heap_arraystruct
(
	ddp_message_write_info_t * message_info,
	ddp_heap_write_info_t * heap_info,
	uint16_t num_elements,
	uint16_t element_size_bytes
)
{
	return ddp_allocate_heap_array(message_info, heap_info, num_elements, element_size_bytes);
}


typedef struct ddp_heap_read_info
{
	const ddp_message_read_info_t * message_info;

	uint16_t num_elements;
	uint16_t element_size_bytes;
	uint16_t length_bytes;

	union
	{
		const uint8_t * array8;
		const uint16_t * array16;
		const uint32_t * array32;
	} _;
	const void * offset_array;
		// only used by offset array reading
} ddp_heap_read_info_t;

aud_error_t
ddp_read_heap_array
(
	const ddp_message_read_info_t * message_info,
	ddp_heap_read_info_t * heap_info,
	ddp_offset_t msg_payload_offset,
	uint16_t num_elements,
	uint16_t element_size_bytes
);

AUD_INLINE aud_error_t
ddp_read_heap_array8
(
	const ddp_message_read_info_t * message_info,
	ddp_heap_read_info_t * heap_info,
	ddp_offset_t msg_payload_offset,
	uint16_t num_elements
)
{
	return ddp_read_heap_array(message_info, heap_info, msg_payload_offset, num_elements, sizeof(uint8_t));
}

AUD_INLINE aud_error_t
ddp_read_heap_array16
(
	const ddp_message_read_info_t * message_info,
	ddp_heap_read_info_t * heap_info,
	ddp_offset_t msg_payload_offset,
	uint16_t num_elements
)
{
	return ddp_read_heap_array(message_info, heap_info, msg_payload_offset, num_elements, sizeof(uint16_t));
}

AUD_INLINE aud_error_t
ddp_read_heap_array32
(
	const ddp_message_read_info_t * message_info,
	ddp_heap_read_info_t * heap_info,
	ddp_offset_t msg_payload_offset,
	uint16_t num_elements
)
{
	return ddp_read_heap_array(message_info, heap_info, msg_payload_offset, num_elements, sizeof(uint32_t));
}

AUD_INLINE aud_error_t
ddp_read_heap_arraystruct
(
	const ddp_message_read_info_t * message_info,
	ddp_heap_read_info_t * heap_info,
	ddp_offset_t msg_payload_offset,
	uint16_t num_elements,
	uint16_t element_size_bytes
)
{
	// element sizes round up
	if (element_size_bytes > sizeof(uint16_t))
		element_size_bytes = ddp_normalise_length_bytes(element_size_bytes);
	return ddp_read_heap_array(
		message_info, heap_info, msg_payload_offset, num_elements, element_size_bytes
	);
}

const void *
ddp_read_heap_element_at_index
(
	const ddp_heap_read_info_t * heap,
	unsigned index
);


/*
	Resolve an offset on the heap.
	If min_size > 0, ensure that at least this many bytes are free between the
	offset and the end of the heap.
 */
const void *
ddp_heap_resolve_sized
(
	const ddp_message_read_info_t * message_info,
	ddp_offset_t message_offset,
	ddp_size_t min_size
);

AUD_INLINE const void *
ddp_heap_raw_resolve_sized
(
	const ddp_message_read_info_t * message_info,
	ddp_raw_offset_t message_offset_packet_order,
	ddp_size_t min_size
)
{
	return ddp_heap_resolve_sized(message_info, ntohs(message_offset_packet_order), min_size);
}


char *
ddp_heap_read_string
(
	const ddp_message_read_info_t * message_info,
	ddp_offset_t message_offset
);

AUD_INLINE const char *
ddp_heap_raw_read_string
(
	const ddp_message_read_info_t * message_info,
	ddp_raw_offset_t message_offset_packet_order
)
{
	return ddp_heap_read_string(message_info, ntohs(message_offset_packet_order));
}


//----------------------------------------------------------
// Message copying
//----------------------------------------------------------

/*
	Copy an existing message into a packet
 */
aud_error_t
ddp_packet_write_add_message
(
	ddp_packet_write_info_t * packet_info,
	const ddp_message_read_info_t * message_info
);


//----------------------------------------------------------
// Misc utility functions
//----------------------------------------------------------

AUD_INLINE ddp_size_t
ddp_calculate_offset_from_pointers
(
	void * low, void * hi
)
{
	ptrdiff_t low_p = (ptrdiff_t) low;
	ptrdiff_t hi_p = (ptrdiff_t) hi;
	if (low_p < hi_p && (hi_p - low_p < UINT16_MAX))
		return (ddp_size_t) (hi_p - low_p);
	else
		return 0;
}


AUD_INLINE ddp_size_t
ddp_packet_write_calculate_offset
(
	const ddp_packet_write_info_t * p,
	void * ptr
)
{
	return ddp_calculate_offset_from_pointers(p->_.header, ptr);
}

AUD_INLINE ddp_size_t
ddp_packet_write_calculate_offset_checked
(
	const ddp_packet_write_info_t * p,
	void * ptr
)
{
	ddp_size_t size = ddp_packet_write_calculate_offset(p, ptr);
	if (size <= p->max_length_bytes)
		return size;
	else
		return 0;
}


AUD_INLINE ddp_size_t
ddp_message_write_calculate_offset
(
	const ddp_message_write_info_t * m,
	void * ptr
)
{
	return ddp_calculate_offset_from_pointers(m->_.header, ptr);
}

AUD_INLINE ddp_size_t
ddp_message_write_calculate_offset_checked
(
	const ddp_message_write_info_t * m,
	void * ptr
)
{
	ddp_size_t size = ddp_message_write_calculate_offset(m, ptr);
	ddp_size_t m_offset = ddp_packet_write_calculate_offset(m->packet, m->_.header);
	if (m_offset + size <= m->packet->max_length_bytes)
		return size;
	else
		return 0;
}


/*
	Given a message_write_info, initialise a message_read_info for the same message.

	Mostly useful for logging or debugging.
 */
aud_error_t
ddp_packet_read_message_from_write_message
(
	ddp_packet_read_info_t * packet,
	ddp_message_read_info_t * message,
	const ddp_message_write_info_t * src
);


//----------------------------------------------------------
// Accessors
//----------------------------------------------------------

AUD_INLINE ddp_request_id_t
ddp_message_read_response_request_id
(
	const ddp_message_read_info_t * msg
)
{
	return ntohs(msg->subheader.response->request_id);
}

AUD_INLINE ddp_request_id_t
ddp_message_read_response_status
(
	const ddp_message_read_info_t * msg
)
{
	return ntohs(msg->subheader.response->status);
}


//----------------------------------------------------------
// Data aggregation
//----------------------------------------------------------

/*
	Data aggregation allows a single item of data to represent multiple
	strings, with all offsets pointing to the same value.

	To solve in the general case, we keep a table of all strings (or byte-bufs)
	and their matching offsets and compare each allocation to the existing data.
 */

typedef struct ddp_message_write_offset_table_entry
{
	union
	{
		const void * data;
		const char * string;
	} data;
	ddp_size_t len;
		// for string data, len == 0
	ddp_offset_t offset;
} ddp_message_write_offset_table_entry_t;

typedef struct ddp_message_write_offset_table_header
{
	size_t max, curr;
} ddp_message_write_offset_table_header_t;

typedef struct guppy_message_write_offset_table
{
	ddp_message_write_offset_table_header_t head;
	ddp_message_write_offset_table_entry_t entry[1];
} ddp_message_write_offset_table_t;

AUD_INLINE void
ddp_message_write_offset_table_init_header
(
	ddp_message_write_offset_table_header_t * head,
	size_t num_entries
)
{
	head->curr = 0;
	head->max = num_entries;
}


const void *
ddp_message_write_offset_table_get_entry
(
	const ddp_message_write_offset_table_t * table,
	ddp_offset_t * offset,
	const void * data,
	ddp_size_t data_len
);

AUD_INLINE const char *
ddp_message_write_offset_table_get_string
(
	const ddp_message_write_offset_table_t * table,
	ddp_offset_t * offset,
	const char * string
)
{
	return (const char*)ddp_message_write_offset_table_get_entry(table, offset, string, 0);
}

const char *
ddp_message_write_offset_table_get_strcase
(
	const ddp_message_write_offset_table_t * table,
	ddp_offset_t * offset,
	const char * string
);


const void *
ddp_message_write_offset_table_allocate_data
(
	ddp_message_write_info_t * msg_info,
	ddp_message_write_offset_table_t * table,
	ddp_offset_t * offset,
	const void * data,
	ddp_size_t size
);

const char *
ddp_message_write_offset_table_allocate_string
(
	ddp_message_write_info_t * msg_info,
	ddp_message_write_offset_table_t * table,
	ddp_offset_t * offset,
	const char * string
);

const char *
ddp_message_write_offset_table_allocate_string_case
(
	ddp_message_write_info_t * msg_info,
	ddp_message_write_offset_table_t * table,
	ddp_offset_t * offset,
	const char * string
);


//----------------------------------------------------------
// Generic message functions
//----------------------------------------------------------
/*
AUD_INLINE aud_error_t
ddp_packet_add_message_offset
(
	ddp_packet_write_info_t * packet_info,
	uint16_t message_offset
) {
	if (!packet_info)
	{
		return AUD_ERR_INVALIDPARAMETER;
	}
	if (packet_info->curr_messages >= packet_info->max_messages || message_offset >= packet_info->curr_length_bytes)
	{
		return AUD_ERR_INVALIDPARAMETER;
	}
	packet_info->message_offsets[packet_info->curr_messages++] = htons(message_offset);
	return AUD_SUCCESS;
}

AUD_INLINE aud_error_t
ddp_packet_message_at_index
(
	const ddp_packet_read_info_t * packet_info,
	ddp_heap_read_info_t * message_info,
	uint16_t n
) {
	if (!packet_info || !message_info || n >= packet_info->num_messages)
	{
		return AUD_ERR_INVALIDPARAMETER;
	}
	return ddp_read_array16(packet_info, message_info, ntohs(packet_info->message_offsets[n]));
}

// On output, the heap_info struct counts and pointers refer to the 'extra' data not the total heap array
aud_error_t
ddp_allocate_monitoring_header_extra16
(
	ddp_packet_write_info_t * packet_info,
	ddp_heap_write_info_t * heap_info,
	ddp_opcode_t opcode,
	const dante_rxflow_error_timestamp_t * timestamp,
	uint16_t extra_words
);

// On output, the heap_info struct counts and pointers refer to the 'extra' data not the total heap array
aud_error_t
ddp_allocate_monitoring_header_extra32
(
	ddp_packet_write_info_t * packet_info,
	ddp_heap_write_info_t * heap_info,
	ddp_opcode_t opcode,
	const dante_rxflow_error_timestamp_t * timestamp,
	uint16_t extra_dwords
);

aud_error_t
ddp_read_monitoring_header_extra16
(
	const ddp_packet_read_info_t * packet_info,
	ddp_heap_read_info_t * info,
	uint16_t offset,
	uint16_t * opcode,
	dante_rxflow_error_timestamp_t * timestamp
);

aud_error_t
ddp_read_monitoring_header_extra32
(
	const ddp_packet_read_info_t * packet_info,
	ddp_heap_read_info_t * info,
	uint16_t offset,
	uint16_t * opcode,
	dante_rxflow_error_timestamp_t * timestamp
);
*/

//! @endcond

#ifdef __cplusplus
}
#endif

#endif // _DDP_PACKET_H

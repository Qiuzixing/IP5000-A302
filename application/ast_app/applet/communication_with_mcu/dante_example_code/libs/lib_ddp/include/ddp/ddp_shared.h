/*
 * File     : ddp_shared.h
 * Created  : June 2014
 * Author   : Andrew White <andrew.white@audinate.com>
 * Synopsis : Shared structures and types
 *
 * Copyright 2005-2017, Audinate Pty Ltd and/or its licensors
	Confidential
 * Audinate Copyright Header Version 1
 */
#ifndef _DDP_SHARED__H
#define _DDP_SHARED__H

//----------
// Include

#include "ddp/shared_structures.h"
#include "ddp/routing_structures.h"
#include "ddp/packet.h"
#include "ddp/network_structures.h"

#ifdef __cplusplus
extern "C" {
#endif

//----------
// Addresses

AUD_INLINE void
ddp_addr_inet_init
(
	ddp_addr_inet_t * addr,
	uint32_t in_addr,
		// network byte order
	uint16_t port
		// host byte order
)
{
	addr->family = htons(DDP_ADDRESS_FAMILY_INET);
	addr->port = htons(port);
	addr->in_addr.ip_addr = in_addr;
}

// Offset arrays

ddp_addr_inet_t *
ddp_addr_inet_allocate
(
	ddp_message_write_info_t * message_info,
	uint32_t in_addr,
		// network byte order
	uint16_t port
		// host byte order
);

ddp_addr_inet_t *
ddp_addr_inet_add_to_array
(
	ddp_message_write_info_t * message_info,
	ddp_array_t * array,
	uint32_t in_addr,
		// network byte order
	uint16_t port
		// host byte order
);


//----------
// Fixed array

/*
	Allocate a payload for a ddp_array message.
	
	This does not allocate any elements
 */
aud_error_t
ddp_array_payload_allocate
(
	ddp_message_write_info_t * message_info,
	ddp_size_t element_size
);

/*
	Add a number of elements to array, and return a pointer to first new element

	NOTE: growing arrays must be uint32_t aligned
 */
void *
ddp_array_payload_allocate_elements
(
	ddp_message_write_info_t * message_info,
	uint16_t num_elements
);


//----------
// Offset array

ddp_array_t *
ddp_array_allocate
(
	ddp_message_write_info_t * message_info,
	ddp_raw_offset_t * offset,
	unsigned n
);

/*
	Allocate heap_bytes of data on the heap and assign the pointer to the next
	empty spot in the array.

	Returns NULL if allocation failed or there were no elements free in the array.
 */
void *
ddp_array_allocate_element
(
	ddp_message_write_info_t * message_info,
	ddp_array_t * array,
	ddp_size_t heap_bytes
);

/*
	Allocate a string on the heap and assign the pointer to the next
	empty spot in the array.

	Returns NULL if allocation failed or there were no elements free in the array.
	NULL string is consdered a failure.
 */
char *
ddp_array_allocate_string
(
	ddp_message_write_info_t * message_info,
	ddp_array_t * array,
	const char * string
);


// Reading

aud_error_t
ddp_array_init_read
(
	const ddp_message_read_info_t *,
	ddp_heap_read_info_t *,
	ddp_offset_t array_start_offset
);


/*
	Extract an offset from an array
 */
ddp_offset_t
ddp_array_get_offset
(
	ddp_heap_read_info_t * heap,
	unsigned index
);

/*
 	Dereference an element of an offset array
 */
const void *
ddp_offset_array_get_element
(
	ddp_heap_read_info_t * heap,
	unsigned index
);

/*
	Dereference an element of an offset array and check size

	If size > 0, then check that this size fits within message bounds.
	If size == 0, then assume the first uint16 is a size and check this.
 */
const void *
ddp_offset_array_read_element_sized
(
	ddp_heap_read_info_t * heap,
	unsigned index,
	ddp_size_t size
);


// Accessors

AUD_INLINE uint16_t
ddp_array_curr
(
	const ddp_array_t * array
)
{
	return (uint16_t) ntohs(array->header.curr);
}

AUD_INLINE unsigned
ddp_array_max
(
	const ddp_array_t * array
)
{
	return (unsigned) ntohs(array->header.max);
}

AUD_INLINE void
ddp_array_set_curr
(
	ddp_array_t * array,
	uint16_t n
)
{
	array->header.curr = htons((uint16_t)n);
}

//----------
// DDP offset array message

ddp_array_t *
ddp_offset_array_payload_allocate
(
	ddp_message_write_info_t * message_info,
	unsigned n
);


aud_error_t
ddp_offset_array_payload_init_read
(
	const ddp_message_read_info_t * message_info,
	ddp_heap_read_info_t *
);


//----------
// ID range request

/*
	Tests whether a pair of numbers form a valid range.
	The following ranges are valid:
		0,0 (no elements)
		X,0 (from X to end)
		X,Y (where 0 < X <= Y)
	The following are invalid:
		0,Y
		X,Y (where X > Y > 0)
 */
AUD_INLINE aud_bool_t
ddp_is_valid_range(uint16_t from, uint16_t to)
{
	return (to == 0) || (from && from <= to);
}

AUD_INLINE aud_bool_t
ddp_id_range_is_valid(const ddp_id_range_t * r)
{
	return ddp_is_valid_range(ntohs(r->from), ntohs(r->to));
}

/*
	Attempt to update an existing range to be the union of the existing and new
	range.  To do this requires that:
		- existing range is valid
		- new range is valid
		- ranges overlap
	Returns true on success, false if range could not be updated
 */
aud_bool_t
ddp_range_extend_range
(
	ddp_id_range_t * range,
	uint16_t from, uint16_t to
);

//! @cond Doxygen_Suppress
typedef struct ddp_message_id_ranges_write
{
	ddp_message_write_info_t * message_info;
	ddp_id_ranges_t * ranges;
} ddp_message_id_ranges_write_t;
//! @endcond

/*
	Allocate an id_ranges message.
	If max_ranges is non-zero, this sets an upper bound for the number of ranges.
	If it is zero, then the payload is treated like a heap and grows as ranges
	are added.
 */
aud_error_t
ddp_message_id_range_allocate_payload
(
	ddp_message_write_info_t * message_info,
	ddp_message_id_ranges_write_t * ranges_write,
	uint16_t max_ranges
);

/*
	Add another range to an existing message, initialising the payload if
	necessary.
	Where possible, ranges are automatically compressed: adding range 2-5 to
	range 1-3 will result in a single range 1-5.  This is only applied to the
	last range in the list.
 */
aud_error_t
ddp_message_id_range_extend_range
(
	ddp_message_id_ranges_write_t * ranges_write,
	uint16_t from, uint16_t to
);


/*
	As add_range, but without compressing ranges or checking them for correctness
 */
aud_error_t
ddp_message_id_range_add_range
(
	ddp_message_id_ranges_write_t * ranges_write,
	uint16_t from, uint16_t to
);


/*
	Validate an id_ranges message.
	On success, return pointer to the ranges structure
 */
const ddp_id_ranges_t *
ddp_message_id_range_validate
(
	const ddp_message_read_info_t * message_info
);


// Accessors

AUD_INLINE uint16_t
ddp_id_ranges_curr
(
	const ddp_id_ranges_t * ranges
)
{
	return (uint16_t) ntohs(ranges->curr);
}

AUD_INLINE uint16_t
ddp_id_ranges_max
(
	const ddp_id_ranges_t * ranges
)
{
	return (uint16_t) ntohs(ranges->max);
}

AUD_INLINE void
ddp_id_ranges_set_curr
(
	ddp_id_ranges_t * ranges,
	uint16_t n
)
{
	ranges->curr = htons(n);
}


//----------

#ifdef __cplusplus
}
#endif

#endif // _DDP_SHARED__H

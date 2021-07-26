/*
 * File     : ddp_shared.c
 * Created  : June 2014
 * Author   : Andrew White <andrew.white@audinate.com>
 * Synopsis : Shared structures and types
 *
 * Copyright 2005-2017, Audinate Pty Ltd and/or its licensors
	Confidential
 * Audinate Copyright Header Version 1
 */

//----------
// Include

#include "ddp/ddp_shared.h"


//----------
// Transports


//----------
// Addresses


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
// Array payload

aud_error_t
ddp_array_payload_allocate
(
	ddp_message_write_info_t * message_info,
	ddp_size_t element_size
)
{
	aud_error_t result;
	ddp_array_payload_t * r;

	assert(message_info);
	if (! element_size)
		return AUD_ERR_INVALIDPARAMETER;

	result = ddp_message_allocate_payload(message_info, sizeof(*r));
	if (result != AUD_SUCCESS)
		return result;

	r = (ddp_array_payload_t *) message_info->payload;
	r->element_size = htons(element_size);
	r->num_elements = 0;
	r->array_offset = 0;
	r->pad0 = 0;
	return AUD_SUCCESS;
}


void *
ddp_array_payload_allocate_elements
(
	ddp_message_write_info_t * message_info,
	uint16_t num_elements
)
{
	ddp_array_payload_t * r;
	uint8_t * array;
	ddp_size_t heap_bytes, elem_size;

	assert(message_info && message_info->payload);
	if (! num_elements)
		return NULL;

	r = (ddp_array_payload_t *) message_info->payload;
	elem_size = ntohs(r->element_size);
	heap_bytes = num_elements * elem_size;

	if (!r->array_offset)
	{
		array = ddp_message_allocate_on_heap(message_info, &r->array_offset, heap_bytes);
		if (array)
		{
			r->num_elements = htons(num_elements);
		}
	}
	else
	{
		uint16_t curr_elements = ntohs(r->num_elements);
		array = ((uint8_t *)message_info->_.buf32) + ntohs(r->array_offset);
		array += (curr_elements * elem_size);
		if (! ddp_message_write_grow_packet(message_info, heap_bytes))
			return NULL;

		bzero(array, heap_bytes);
		curr_elements += num_elements;
		r->num_elements = htons(curr_elements);
	}
	return (void *) array;
}


//----------
// Offset array

ddp_array_t *
ddp_array_allocate
(
	ddp_message_write_info_t * message_info,
	ddp_raw_offset_t * offset,
	unsigned n
)
{
	ddp_array_t * array;
	ddp_size_t array_bytes;

	if (! n)
		return NULL;

	array_bytes = (ddp_size_t) (sizeof(ddp_array_t) + (n - 1) * sizeof(array->offset[0]));
	array = ddp_message_allocate_on_heap(message_info, offset, array_bytes);
	if (array)
	{
		array->header.max = htons((uint16_t) n);
	}
	return array;
}


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
)
{
	uint16_t n;
	void * result;
	assert (message_info && array);

	if (! heap_bytes)
		return NULL;

	n = ddp_array_curr(array);
	if (n >= ddp_array_max(array))
		return NULL;

	result = ddp_message_allocate_on_heap(message_info, &array->offset[n], heap_bytes);
	if (result)
		ddp_array_set_curr(array, n+1);
	return result;
}

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
)
{
	uint16_t n;
	char * result;
	assert (message_info && array);

	if (! string)
		return NULL;

	n = ddp_array_curr(array);
	if (n >= ddp_array_max(array))
		return NULL;

	result = ddp_message_allocate_string(message_info, &array->offset[n], string);
	if (result)
		ddp_array_set_curr(array, n+1);
	return result;
}


aud_error_t
ddp_array_init_read
(
	const ddp_message_read_info_t * message_info,
	ddp_heap_read_info_t * heap,
	ddp_offset_t array_start_offset
)
{
	const ddp_array_t * array =
		ddp_heap_resolve_sized(message_info, array_start_offset, sizeof(ddp_array_t));

	heap->message_info = message_info;
	heap->offset_array = array;
	heap->_.array16 = (const uint16_t *) (const void *) (&array->header + 1);
	heap->num_elements = (uint16_t) ddp_array_curr(array);
	heap->element_size_bytes = sizeof(uint16_t);
	heap->length_bytes = (heap->num_elements - 1) * heap->element_size_bytes;

	if (array_start_offset + sizeof(array->header) > message_info->length_bytes)
		return AUD_ERR_RANGE;
	return AUD_SUCCESS;
}


/*
	Extract an offset from an array
 */
ddp_offset_t
ddp_array_get_offset
(
	ddp_heap_read_info_t * heap,
	unsigned index
)
{
	const ddp_array_t * array = (heap->offset_array);
	assert(array);
	if (index < ddp_array_curr(array))
	{
		return ntohs(array->offset[index]);
	}
	return 0;
}


const void *
ddp_offset_array_get_element
(
	ddp_heap_read_info_t * heap,
	unsigned index
)
{
	ddp_offset_t offset = ddp_array_get_offset(heap, index);
	if (offset)
	{
		return ddp_heap_resolve_sized(heap->message_info, offset, 0);
	}
	else
		return NULL;
}


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
)
{
	ddp_offset_t offset = ddp_array_get_offset(heap, index);
	if (! offset)
		return NULL;

	if (size)
		return ddp_heap_resolve_sized(heap->message_info, offset, size);
	else
	{
		const uint16_t * result =
			ddp_heap_resolve_sized(heap->message_info, offset, sizeof(*result));
		if (result)
		{
			size = ntohs(result[0]);
			if (offset + size <= heap->message_info->length_bytes)
				return result;
		}
		return NULL;
	}
}


// Offset array payload

ddp_array_t *
ddp_offset_array_payload_allocate
(
	ddp_message_write_info_t * message_info,
	unsigned n
)
{
	ddp_offset_array_payload_t * p;
	if (!(message_info && n))
		return NULL;

	if (ddp_message_allocate_payload(message_info, sizeof(*p)) != AUD_SUCCESS)
	{
		return NULL;
	}

	p = (ddp_offset_array_payload_t *) message_info->payload;
	return ddp_array_allocate(message_info, &p->array_offset, n);
}


aud_error_t
ddp_offset_array_payload_init_read
(
	const ddp_message_read_info_t * message_info,
	ddp_heap_read_info_t * heap
)
{
	aud_error_t result;

	assert(message_info && heap);

	if (! message_info->payload.value16)
		return AUD_ERR_INVALIDPARAMETER;

	result = ddp_array_init_read(message_info, heap, ntohs(message_info->payload.value16[0]));
	return result;
}


//----------
// ID range request

aud_bool_t
ddp_range_extend_range
(
	ddp_id_range_t * range,
	uint16_t from, uint16_t to
)
{
	uint16_t range_from = ntohs(range->from);
	uint16_t range_to = ntohs(range->to);

	if (! (ddp_is_valid_range(from, to) && ddp_is_valid_range(range_from, range_to)))
		return AUD_FALSE;

	if (from == 0)
		return AUD_TRUE;
			// don't need to test to == 0 since that's implied by previous check
	if (range_from == 0)
	{
		range->from = htons(from);
		range->to = htons(to);
		return AUD_TRUE;
	}

	// At this point, we have two valid non-empty ranges
	// Do they overlap?
	if (to && range_from > to + 1)
		return AUD_FALSE;
	if (range_to && from > range_to + 1)
		return AUD_FALSE;

	// overlap - just figure bounds
	if (range_to)
	{
		if (! to)
		{
			range_to = 0;
		}
		else if (to > range_to)
		{
			range_to = htons(to);
		}
	}
	if (from < range_from)
	{
		range->from = htons(from);
	}
	return AUD_TRUE;
}


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
)
{
	aud_error_t result;
	ddp_id_range_payload_t * p;
	ddp_id_ranges_t * ranges;
	ddp_size_t heap_bytes = sizeof(*ranges);

	assert(message_info && ranges_write);

	result = ddp_message_allocate_payload(message_info, sizeof(*p));
	if (result != AUD_SUCCESS)
		return result;
	p = (ddp_id_range_payload_t *) message_info->payload;
	p->pad0 = 0;

	if (max_ranges > 1)
	{
		heap_bytes += (ddp_size_t)sizeof(ddp_id_range_t) * (max_ranges - 1);
	}
	ranges = ddp_message_allocate_on_heap(message_info, &p->ranges_offset, heap_bytes);
	if (! ranges_write)
		return AUD_ERR_NOMEMORY;

	ranges_write->message_info = message_info;
	ranges_write->ranges = ranges;
	if (max_ranges)
	{
		ranges->max = htons(max_ranges);
	}

	return AUD_SUCCESS;
}


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
)
{
	ddp_id_ranges_t * ranges;
	unsigned curr;

	assert(ranges_write && ranges_write->message_info && ranges_write->ranges);

	if (from == 0 && to == 0)
	{
		return AUD_SUCCESS;
	}

	if (! ddp_is_valid_range(from, to))
	{
		return AUD_ERR_RANGE;
	}

	ranges = ranges_write->ranges;
	curr = ddp_id_ranges_curr(ranges);
	if (curr)
	{
		ddp_id_range_t * last_range = ranges->range + (curr - 1);
		if (ddp_range_extend_range(last_range, from, to))
			return AUD_SUCCESS;
	}

	return ddp_message_id_range_add_range(ranges_write, from, to);
}


/*
	As add_range, but without compressing ranges or checking them for correctness
 */
aud_error_t
ddp_message_id_range_add_range
(
	ddp_message_id_ranges_write_t * ranges_write,
	uint16_t from, uint16_t to
)
{
	ddp_id_ranges_t * ranges;
	unsigned max, curr;

	assert(ranges_write && ranges_write->message_info && ranges_write->ranges);

	ranges = ranges_write->ranges;
	max = ddp_id_ranges_max(ranges);
	curr = ddp_id_ranges_curr(ranges);
	if (max)
	{
		if (curr >= max)
		{
			return AUD_ERR_NOMEMORY;
		}
	}
	else if (curr)
	{
		// if p->ranges.curr == 0, we have already allocated one range
		if (! ddp_message_write_grow_packet(ranges_write->message_info, sizeof(ddp_id_range_t)))
		{
			return AUD_ERR_NOMEMORY;
		}
	}

	ranges->range[curr].from = htons(from);
	ranges->range[curr].to = htons(to);
	ddp_id_ranges_set_curr(ranges, (uint16_t)curr+1);

	return AUD_SUCCESS;
}


const ddp_id_ranges_t *
ddp_message_id_range_validate
(
	const ddp_message_read_info_t * message_info
)
{
	if (message_info && message_info->payload.value32)
	{
		unsigned curr;

		const ddp_id_range_payload_t * r =
			(const ddp_id_range_payload_t *) message_info->payload.value16;
		ddp_size_t bytes = sizeof(*r);
		const ddp_id_ranges_t * ranges;

		if (! r->ranges_offset)
			return NULL;
		ranges = ddp_heap_resolve_sized(message_info, ntohs(r->ranges_offset), sizeof(*ranges));
		if (! ranges)
			return NULL;

		curr = ddp_id_ranges_curr(ranges);
		if (curr > 1)
		{
			bytes += (uint16_t)(curr - 1) * sizeof(ranges->range[0]);
		}
		if (ntohs(r->ranges_offset) + bytes <= message_info->length_bytes)
			return ranges;
		else
			return NULL;
	}
	else
		return NULL;
}


//----------

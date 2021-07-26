/*
* File     : ddp_packet_write.c
* Created  : August 2014
* Updated  : Date: 2014/08/22
* Author   : Michael Ung <michael.ung@audinate.com>
* Synopsis : All top level DDP packet client write helper functions.
*
* Copyright 2005-2017, Audinate Pty Ltd and/or its licensors
Confidential
* Audinate Copyright Header Version 1
*/

#include "ddp/packet.h"

aud_error_t
ddp_packet_init_write
(
	ddp_packet_write_info_t * info,
	uint32_t * buf,
	ddp_size_t buf_len
) {
	const ddp_size_t length_bytes = 8;//sizeof(ddp_packet_header_t);

	if (!info || !buf || !buf_len)
	{
		return AUD_ERR_INVALIDPARAMETER;
	}

	if (buf_len < length_bytes)
	{
		return AUD_ERR_NOMEMORY;
	}

	info->_.buf32 = buf;
	info->max_length_bytes = buf_len;
	info->curr_length_bytes = 8;//sizeof(ddp_packet_header_t);
	info->_.header->block._.length_bytes = htons(8);//sizeof(ddp_packet_header_t));
	info->_.header->block._.opcode = htons(DDP_OP_PACKET_HEADER);
	info->_.header->version = htons(DDP_VERSION_1_0_0);
	info->_.header->_.padding0 = 0;

	return AUD_SUCCESS;
}

uint32_t *
ddp_packet_write_grow_packet
(
	ddp_packet_write_info_t * packet_info,
	ddp_size_t bytes
)
{
	ddp_size_t curr_len = ddp_packet_write_get_length_bytes(packet_info);

	curr_len += ddp_normalise_length_bytes(bytes);
	if (curr_len > packet_info->max_length_bytes)
		return 0;
	else
	{
		uint32_t * ptr = ddp_packet_write_get_tail(packet_info);
		packet_info->curr_length_bytes = curr_len;
		return ptr;
	}
}


aud_error_t
ddp_packet_init_write_requests
(
	ddp_packet_write_info_t * packet_info,
	uint32_t * buf,
	ddp_size_t buf_len
)
{
	aud_error_t result = ddp_packet_init_write(packet_info, buf, buf_len);
	if (result == AUD_SUCCESS)
		ddp_packet_write_apply_direction(packet_info, DDP_PACKET_HEADER_FLAG_DIRECTION_REQUEST);
	return result;
}

aud_error_t
ddp_packet_init_write_responses
(
	ddp_packet_write_info_t * packet_info,
	uint32_t * buf,
	ddp_size_t buf_len
)
{
	aud_error_t result = ddp_packet_init_write(packet_info, buf, buf_len);
	if (result == AUD_SUCCESS)
		ddp_packet_write_apply_direction(packet_info, DDP_PACKET_HEADER_FLAG_DIRECTION_RESPONSE);
	return result;
}


uint32_t *
ddp_message_write_grow_packet
(
	ddp_message_write_info_t * message_info,
	ddp_size_t bytes
)
{
	ddp_size_t curr_len = ddp_message_write_get_length_bytes(message_info);
	uint32_t * ptr = ddp_packet_write_grow_packet(message_info->packet, bytes);

	if (ptr)
	{
		curr_len += ddp_normalise_length_bytes(bytes);
		ddp_message_set_length_bytes(message_info->_.header, curr_len);
	}
	return ptr;
}


aud_bool_t
ddp_packet_write_direction_is_valid
(
	const ddp_packet_write_info_t * packet_info,
	uint8_t flag_in
)
{
	uint8_t flag_packet = ddp_packet_write_get_direction(packet_info);
	return flag_packet == 0 || flag_packet == flag_in;
}


aud_bool_t
ddp_packet_write_apply_direction
(
	ddp_packet_write_info_t * packet_info,
	uint8_t direction_flag
)
{
	uint8_t flag_packet = ddp_packet_write_get_direction(packet_info);
	if (flag_packet == 0)
	{
		AUD_FLAG_SET_MASKED(
			packet_info->_.header->_.ext.flags,
			DDP_PACKET_HEADER_FLAG_DIRECTION_MASK,
			direction_flag
		);
		return AUD_TRUE;
	}
	else
	{
		return (flag_packet == direction_flag);
	}
}


void
ddp_packet_write_set_link_infrastructure
(
	ddp_packet_write_info_t * packet_info
)
{
	AUD_FLAG_SET(
		packet_info->_.header->_.ext.flags,
		DDP_PACKET_HEADER_FLAG_LINK_INFRASTRUCTURE
	);
}


aud_error_t
ddp_packet_allocate_message_head
(
	ddp_packet_write_info_t * packet_info,
	ddp_message_write_info_t * message_info,
	uint16_t opcode,
	ddp_size_t subheader_length_bytes
)
{
	ddp_size_t length_bytes;
	uint32_t * packet_ptr;

	if (!packet_info || !message_info)
	{
		return AUD_ERR_INVALIDPARAMETER;
	}

	length_bytes = sizeof(ddp_message_header_t) + subheader_length_bytes;

	packet_ptr = ddp_packet_write_grow_packet(packet_info, length_bytes);
	if (! packet_ptr)
	{
		return AUD_ERR_NOMEMORY;
	}

	message_info->packet = packet_info;
	message_info->_.buf32 = packet_ptr;
	message_info->_.header->block._.length_bytes = htons(length_bytes);
	message_info->_.header->block._.opcode = htons(opcode);
	message_info->_.header->subheader_length_bytes = htons(subheader_length_bytes);
	message_info->_.header->payload_length_bytes = 0;

	message_info->subheader.buf = message_info->_.buf32 +
		ddp_size_bytes_to_uint32(sizeof(ddp_message_header_t));
	message_info->payload = NULL;
	message_info->heap = NULL;

	return AUD_SUCCESS;
}


aud_error_t
ddp_packet_allocate_request_head
(
	ddp_packet_write_info_t * packet_info,
	ddp_message_write_info_t * message_info,
	uint16_t opcode,
	uint16_t request_id
)
{
	ddp_request_subheader_t * sub;
	if (! ddp_packet_write_apply_direction(
		packet_info, DDP_PACKET_HEADER_FLAG_DIRECTION_REQUEST)
	)
	{
		return AUD_ERR_INVALIDDATA;
	}

	aud_error_t result =
		ddp_packet_allocate_message_head(
			packet_info, message_info, opcode, sizeof(*sub)
		);
	if (result != AUD_SUCCESS)
		return result;

	sub = message_info->subheader.request;
	sub->request_id = htons(request_id);
	sub->padding = 0;

	return AUD_SUCCESS;
}


aud_error_t
ddp_packet_allocate_response_head
(
	ddp_packet_write_info_t * packet_info,
	ddp_message_write_info_t * message_info,
	uint16_t opcode,
	uint16_t request_id
)
{
	ddp_response_subheader_t * sub;
	if (! ddp_packet_write_apply_direction(
		packet_info, DDP_PACKET_HEADER_FLAG_DIRECTION_RESPONSE)
	)
	{
		return AUD_ERR_INVALIDDATA;
	}

	aud_error_t result =
		ddp_packet_allocate_message_head(
			packet_info, message_info, opcode, sizeof(*sub)
		);
	if (result != AUD_SUCCESS)
		return result;

	sub = message_info->subheader.response;
	sub->request_id = htons(request_id);
	sub->status = 0;

	return AUD_SUCCESS;
}


aud_error_t
ddp_packet_allocate_local_head
(
	ddp_packet_write_info_t * packet_info,
	ddp_message_write_info_t * message_info,
	uint16_t opcode,
	const ddp_event_timestamp_t* event_timestamp
)
{
	ddp_local_subheader_t * sub;
	aud_error_t result =
		ddp_packet_allocate_message_head(
			packet_info, message_info, opcode, sizeof(*sub)
		);
	if (result != AUD_SUCCESS)
		return result;

	sub = message_info->subheader.local;
	sub->timestamp = *event_timestamp;

	return AUD_SUCCESS;
}


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
)
{
	uint32_t * packet_ptr;

	if (! (message_info && message_info->packet))
	{
		return AUD_ERR_INVALIDPARAMETER;
	}

	packet_ptr = ddp_message_write_grow_packet(message_info, payload_length_bytes);
	if (! packet_ptr)
	{
		return AUD_ERR_NOMEMORY;
	}

	message_info->payload = packet_ptr;
	message_info->_.header->payload_length_bytes = htons(payload_length_bytes);
	return AUD_SUCCESS;
}


/*
	Internal version of ddp_message_allocate_on_heap & friends.
	Assumes that all parameters are checked and valid.
 */
static void *
allocate_heap
(
	ddp_message_write_info_t * message_info,
	ddp_raw_offset_t * offset,
	ddp_size_t heap_bytes
)
{
	ddp_size_t curr_offset = ddp_message_write_get_length_bytes(message_info);
	void * ptr = ddp_message_write_grow_packet(message_info, heap_bytes);
	if (ptr && offset)
	{
		*offset = htons(curr_offset);
	}
	return ptr;
}

AUD_INLINE void *
allocate_heap_failed
(
	ddp_raw_offset_t * offset
)
{
	if (offset)
	{
		*offset = 0;
	}
	return NULL;
}


/*
	Allocate a block of memory on the heap
	Offset to memory is written into 'offset'
 */
void *
ddp_message_allocate_on_heap
(
	ddp_message_write_info_t * message_info,
	ddp_raw_offset_t * offset,
	ddp_size_t heap_bytes
)
{
	if (message_info && heap_bytes)
	{
		void * ptr = allocate_heap(message_info, offset, heap_bytes);
		if (ptr)
		{
			bzero(ptr, heap_bytes);
		}
		return ptr;
	}
	else
	{
		return allocate_heap_failed(offset);
	}
}

void *
ddp_message_allocate_on_heap_no_zero
(
	ddp_message_write_info_t * message_info,
	ddp_raw_offset_t * offset,
	ddp_size_t heap_bytes
) {
	if (message_info && heap_bytes)
	{
		return allocate_heap(message_info, offset, heap_bytes);
	}
	else
	{
		return allocate_heap_failed(offset);
	}
}


/*
	Allocate a block of memory on the heap and copy the provided data
	Offset to memory is written into 'offset'
 */
void *
ddp_message_allocate_data
(
	ddp_message_write_info_t * message_info,
	ddp_offset_t * offset,
	ddp_size_t data_len,
	const void * data
)
{
	if (data && data_len)
	{
		void * ptr = ddp_message_allocate_on_heap(message_info, offset, data_len);
		if (ptr)
		{
			memcpy(ptr, data, data_len);
		}
		return ptr;
	}
	else
	{
		return allocate_heap_failed(offset);
	}
}


/*
	Allocate a block of memory on the heap and copy the provided string
	Offset to memory is written into 'offset'
 */
char *
ddp_message_allocate_string
(
	ddp_message_write_info_t * message_info,
	ddp_offset_t * offset,
	const char * string
)
{
	if (string)
	{
		return (char *) ddp_message_allocate_data(message_info, offset, (ddp_size_t)strlen(string) + 1, string);
	}
	else
	{
		return (char *) allocate_heap_failed(offset);
	}
}


aud_error_t
ddp_packet_allocate_message
(
	ddp_packet_write_info_t * packet_info,
	ddp_message_write_info_t * message_info,
	uint16_t opcode,
	uint16_t subheader_length_bytes,
	uint16_t payload_length_bytes
) {
	aud_error_t result;

	result = ddp_packet_allocate_message_head(
		packet_info, message_info, opcode, subheader_length_bytes
	);
	if (result != AUD_SUCCESS)
		return result;

	return ddp_message_allocate_payload(message_info, payload_length_bytes);
}

aud_error_t
ddp_packet_allocate_request
(
	ddp_packet_write_info_t * packet_info,
	ddp_message_write_info_t * message_info,
	uint16_t opcode,
	ddp_request_id_t request_id,
	uint16_t payload_length_bytes
) {
	aud_error_t result;

	result = ddp_packet_allocate_request_head(
		packet_info, message_info, opcode, request_id
	);
	if (result != AUD_SUCCESS)
		return result;

	return ddp_message_allocate_payload(message_info, payload_length_bytes);
}

aud_error_t
ddp_packet_allocate_response
(
	ddp_packet_write_info_t * packet_info,
	ddp_message_write_info_t * message_info,
	uint16_t opcode,
	ddp_request_id_t request_id,
	ddp_status_t status,
	uint16_t payload_length_bytes
) {
	aud_error_t result;

	result = ddp_packet_allocate_response_head(
		packet_info, message_info, opcode, request_id
	);
	if (result != AUD_SUCCESS)
		return result;

	if (status)
		ddp_message_response_set_status(message_info, status);

	return ddp_message_allocate_payload(message_info, payload_length_bytes);
}

aud_error_t
ddp_packet_allocate_monitoring
(
	ddp_packet_write_info_t * packet_info,
	ddp_message_write_info_t * message_info,
	uint16_t opcode,
	uint16_t monitoring_seqnum,
	uint16_t payload_length_bytes
) {
	ddp_monitoring_subheader_t * subheader;
	aud_error_t result = ddp_packet_allocate_message(packet_info, message_info,
		opcode,
		sizeof(ddp_monitoring_subheader_t), payload_length_bytes);
	if (result != AUD_SUCCESS)
	{
		return result;
	}
	subheader = message_info->subheader.monitoring;
	subheader->monitor_seqnum = htons(monitoring_seqnum);
	subheader->padding = 0;

	return AUD_SUCCESS;
}

aud_error_t
ddp_packet_allocate_local
(
	ddp_packet_write_info_t * packet_info,
	ddp_message_write_info_t * message_info,
	uint16_t opcode,
	const ddp_event_timestamp_t* event_timestamp,
	uint16_t payload_length_bytes
) {
	ddp_local_subheader_t * subheader;
	aud_error_t result = ddp_packet_allocate_message(packet_info, message_info,
		opcode,
		sizeof(ddp_local_subheader_t), payload_length_bytes);
	if (result != AUD_SUCCESS)
	{
		return result;
	}
	subheader = message_info->subheader.local;
	subheader->timestamp.seconds = htonl(event_timestamp->seconds);
	subheader->timestamp.subseconds = htonl(event_timestamp->subseconds);

	return AUD_SUCCESS;
}


aud_error_t
ddp_packet_write_add_message
(
	ddp_packet_write_info_t * packet_info,
	const ddp_message_read_info_t * message_info
)
{
	uint32_t * msg_start = ddp_packet_write_grow_packet(
		packet_info, message_info->length_bytes
	);
	if (! msg_start)
		return AUD_ERR_NOMEMORY;

	memcpy(msg_start, message_info->header.value32, message_info->length_bytes);
	return AUD_SUCCESS;
}


aud_error_t
ddp_allocate_heap_array
(
	ddp_message_write_info_t * message_info,
	ddp_heap_write_info_t * heap_info,
	uint16_t num_elements,
	ddp_size_t element_size
)
{
	uint16_t heap_len_bytes;
	void * mem;

	if (!message_info || !heap_info || !num_elements || !element_size)
	{
		return AUD_ERR_INVALIDPARAMETER;
	}

	//calculate the base size of the heap to allocate must be in multiple of 4 bytes of alignment
	heap_len_bytes = ddp_normalise_length_bytes(num_elements * element_size);

	//check that there is still sufficient room in the packet buffer
	mem = ddp_message_allocate_on_heap(message_info, &heap_info->msg_offset_bytes, heap_len_bytes);
	if (! mem)
		return AUD_ERR_NOMEMORY;

	//populate the heap_info struct
	heap_info->num_elements = num_elements;
	heap_info->element_size_bytes = element_size;
	heap_info->length_bytes = heap_len_bytes;
	heap_info->_.array32 = (uint32_t *) mem;

	return AUD_SUCCESS;
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

const void *
ddp_message_write_offset_table_get_entry
(
	const ddp_message_write_offset_table_t * table,
	ddp_offset_t * offset_out,
	const void * data,
	ddp_size_t data_len
)
{
	unsigned i;
	if (data_len)
	{
		for (i = 0; i < table->head.curr; i++)
		{
			if (table->entry[i].len != data_len)
				continue;

			if (memcmp(table->entry[i].data.data, data, data_len) == 0)
			{
				* offset_out = table->entry[i].offset;
				return table->entry[i].data.data;
			}
		}
	}
	else
	{
		for (i = 0; i < table->head.curr; i++)
		{
			if (strcmp(table->entry[i].data.string, data) == 0)
			{
				* offset_out = table->entry[i].offset;
				return table->entry[i].data.data;
			}
		}
	}

	return 0;
}

const char *
ddp_message_write_offset_table_get_strcase
(
	const ddp_message_write_offset_table_t * table,
	ddp_offset_t * offset_out,
	const char * string
)
{
	unsigned i;
	for (i = 0; i < table->head.curr; i++)
	{
		if (strcasecmp(table->entry[i].data.string, string) == 0)
		{
			* offset_out = table->entry[i].offset;
			return table->entry[i].data.string;
		}
	}

	return 0;
}


const void *
ddp_message_write_offset_table_allocate_data
(
	ddp_message_write_info_t * msg_info,
	ddp_message_write_offset_table_t * table,
	ddp_offset_t * offset_out,
	const void * data,
	ddp_size_t size
)
{
	if (table)
	{
		const void * data_out = ddp_message_write_offset_table_get_entry(table, offset_out, data, size);
		if (data_out)
		{
			return data_out;
		}
		else if (table->head.curr < table->head.max)
		{
			ddp_message_write_offset_table_entry_t * entry = table->entry + table->head.curr;
			entry->data.data = ddp_message_allocate_data(msg_info, &entry->offset, size, data);
			if (entry->data.data)
			{
				table->head.curr++;
				*offset_out = entry->offset;
				return entry->data.data;
			}
		}
		return NULL;
	}
	else
	{
		return ddp_message_allocate_data(msg_info, offset_out, size, data);
	}
}


static const char *
ddp_message_write_offset_table_allocate_string_common
(
	ddp_message_write_info_t * msg_info,
	ddp_message_write_offset_table_t * table,
	ddp_offset_t * offset_out,
	const char * string,
	aud_bool_t check_case
)
{
	if (table)
	{
		const char * string_out;
		if (check_case)
			string_out = ddp_message_write_offset_table_get_string(table, offset_out, string);
		else
			string_out = ddp_message_write_offset_table_get_string(table, offset_out, string);
		if (string_out)
		{
			return string_out;
		}
		else if (table->head.curr < table->head.max)
		{
			ddp_message_write_offset_table_entry_t * entry = table->entry + table->head.curr;
			entry->data.string = ddp_message_allocate_string(msg_info, &entry->offset, string);
			if (entry->data.string)
			{
				table->head.curr++;
				*offset_out = entry->offset;
				return entry->data.string;
			}
		}
		return NULL;
	}
	else
	{
		return ddp_message_allocate_string(msg_info, offset_out, string);
	}
}

const char *
ddp_message_write_offset_table_allocate_string
(
	ddp_message_write_info_t * msg_info,
	ddp_message_write_offset_table_t * table,
	ddp_offset_t * offset_out,
	const char * string
)
{
	return ddp_message_write_offset_table_allocate_string_common(
		msg_info, table, offset_out, string, AUD_TRUE
	);
}


const char *
ddp_message_write_offset_table_allocate_string_case
(
	ddp_message_write_info_t * msg_info,
	ddp_message_write_offset_table_t * table,
	ddp_offset_t * offset_out,
	const char * string
)
{
	return ddp_message_write_offset_table_allocate_string_common(
		msg_info, table, offset_out, string, AUD_FALSE
	);
}


//----------------------------------------------------------


/*
aud_error_t
ddp_allocate_monitoring_header_extra16
(
	ddp_packet_write_info_t * packet_info,
	ddp_heap_write_info_t * heap_info,
	ddp_opcode_t opcode,
	const dante_rxflow_error_timestamp_t * timestamp,
	uint16_t extra_words
) {
	aud_error_t result;

	assert(sizeof(ddp_monitoring_header_t) % 4 == 0);

	result = ddp_allocate_array16_align32(packet_info, heap_info, (uint16_t) (sizeof(ddp_monitoring_header_t)/2 + extra_words));
	if (result == AUD_SUCCESS)
	{
		ddp_monitoring_header_t * header = (ddp_monitoring_header_t *) heap_info->_.array16;
		header->request_id = 0;
		header->opcode = htons(opcode);
		if (timestamp)
		{
			header->timestamp_seconds = htonl(timestamp->seconds);
			header->timestamp_subseconds = htonl(timestamp->subseconds);
		}

		// move array info to 'extra' block
		heap_info->num_elements8  = extra_words * 2;
		heap_info->num_elements16 = extra_words;
		heap_info->num_elements32 = 0;
		heap_info->_.array16 = heap_info->raw16 + sizeof(ddp_monitoring_header_t)/2;
	}
	return result;
}

aud_error_t
ddp_allocate_monitoring_header_extra32
(
	ddp_packet_write_info_t * packet_info,
	ddp_heap_write_info_t * heap_info,
	ddp_opcode_t opcode,
	const dante_rxflow_error_timestamp_t * timestamp,
	uint16_t extra_dwords
) {
	aud_error_t result;

	assert(sizeof(ddp_monitoring_header_t) % 4 == 0);

	result = ddp_allocate_array16_align32(packet_info, heap_info, (uint16_t) (sizeof(ddp_monitoring_header_t)/2 + extra_dwords*2));
	if (result == AUD_SUCCESS)
	{
		ddp_monitoring_header_t * header = (ddp_monitoring_header_t *) heap_info->_.array16;
		header->request_id = 0;
		header->opcode = htons(opcode);
		if (timestamp)
		{
			header->timestamp_seconds = htonl(timestamp->seconds);
			header->timestamp_subseconds = htonl(timestamp->subseconds);
		}

		// move array info to 'extra' block
		heap_info->num_elements8  = extra_dwords * 4;
		heap_info->num_elements16 = extra_dwords * 2;
		heap_info->num_elements32 = extra_dwords;
		heap_info->_.array16 = heap_info->raw16 + sizeof(ddp_monitoring_header_t)/2;
	}
	return result;
}

*/

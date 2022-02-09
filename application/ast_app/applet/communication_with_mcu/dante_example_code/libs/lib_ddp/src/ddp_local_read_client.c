/*
* File     : ddp_local_read_client.c
* Created  : August 2014
* Updated  : Date: 2014/08/22
* Author   : Michael Ung <michael.ung@audinate.com>
* Synopsis : All local DDP client read helper functions.
*
* Copyright 2005-2017, Audinate Pty Ltd and/or its licensors
Confidential
* Audinate Copyright Header Version 1
*/

#include "ddp/local_client.h"
#include "ddp/local_structures.h"

#if defined(AUD_PLATFORM_ULTIMO)

aud_error_t
ddp_read_local_audio_format
(
	const ddp_packet_read_info_t * packet_info,
	uint16_t offset,
	ddp_event_timestamp_t ** out_timestamp,
	dante_samplerate_t * out_current_samplerate,
	dante_samplerate_t * out_reboot_samplerate
)
{
	aud_error_t result;
	ddp_message_read_info_t message_info;
	ddp_local_audio_format_header_t * header;

	if (!packet_info || offset >= packet_info->packet_length_bytes)
	{
		return AUD_ERR_INVALIDPARAMETER;
	}
	result = ddp_packet_read_message(packet_info, &message_info, offset);
	if (result != AUD_SUCCESS)
	{
		return result;
	}
	result = ddp_packet_validate_message(&message_info, DDP_OP_LOCAL_AUDIO_FORMAT, sizeof(ddp_local_audio_format_header_t));
	if (result != AUD_SUCCESS)
	{
		return result;
	}
	header = (ddp_local_audio_format_header_t *)message_info.header.header;

	if (out_timestamp)
	{
		*out_timestamp = &header->subheader.timestamp;
	}
	if (out_current_samplerate)
	{
		*out_current_samplerate = ntohl(header->payload.current_samplerate);
	}
	if (out_reboot_samplerate)
	{
		*out_reboot_samplerate = ntohl(header->payload.reboot_samplerate);
	}

	return AUD_SUCCESS;
}

aud_error_t
ddp_read_local_clock_pullup
(
	const ddp_packet_read_info_t * packet_info,
	uint16_t offset,
	ddp_event_timestamp_t ** out_timestamp,
	uint16_t * out_current_pullup,
	uint16_t * out_reboot_pullup
)
{
	aud_error_t result;
	ddp_message_read_info_t message_info;
	ddp_local_clock_pullup_header_t * header;

	if (!packet_info || offset >= packet_info->packet_length_bytes)
	{
		return AUD_ERR_INVALIDPARAMETER;
	}
	result = ddp_packet_read_message(packet_info, &message_info, offset);
	if (result != AUD_SUCCESS)
	{
		return result;
	}
	result = ddp_packet_validate_message(&message_info, DDP_OP_LOCAL_CLOCK_PULLUP, sizeof(ddp_local_clock_pullup_header_t));
	if (result != AUD_SUCCESS)
	{
		return result;
	}
	header = (ddp_local_clock_pullup_header_t *)message_info.header.header;

	if (out_timestamp)
	{
		*out_timestamp = &header->subheader.timestamp;
	}
	if (out_current_pullup)
	{
		*out_current_pullup = ntohs(header->payload.current_pullup);
	}
	if (out_reboot_pullup)
	{
		*out_reboot_pullup = ntohs(header->payload.reboot_pullup);
	}

	return AUD_SUCCESS;
}

#endif // defined(AUD_PLATFORM_ULTIMO)

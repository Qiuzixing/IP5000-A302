/*
* File     : ddp_audio_write_client.c
* Created  : August 2014
* Updated  : Date: 2014/08/22
* Author   : Michael Ung <michael.ung@audinate.com>
* Synopsis : All audio DDP client write helper functions.
*
* Copyright 2005-2017, Audinate Pty Ltd and/or its licensors
Confidential
* Audinate Copyright Header Version 1
*/

#include "ddp/audio_client.h"
#include "ddp/audio_structures.h"

aud_error_t
ddp_add_audio_basic_request
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

	result = ddp_packet_allocate_request (packet_info, &message_info,
											DDP_OP_AUDIO_BASIC,
											request_id,
											0);
	return result;
}

aud_error_t
ddp_add_audio_sample_rate_config_request
(
	ddp_packet_write_info_t * packet_info,
	ddp_request_id_t request_id,
	uint16_t control_flags,
	uint32_t sample_rate
)
{
	aud_error_t result = AUD_SUCCESS;
	ddp_message_write_info_t message_info;
	ddp_audio_srate_config_request_t * raw_header;

	if (!packet_info)
	{
		return AUD_ERR_INVALIDPARAMETER;
	}

	result = ddp_packet_allocate_request(packet_info, &message_info,
										   DDP_OP_AUDIO_SRATE_CONFIG,
										   request_id,
										   sizeof(raw_header->payload));

	if (result != AUD_SUCCESS)
	{
		return result;
	}

	raw_header = (ddp_audio_srate_config_request_t *)message_info._.buf32;
	raw_header->payload.control_flags = htons(control_flags);
	
	if (control_flags & SET_SRATE_FLAGS)
	{
		raw_header->payload.sample_rate = htonl(sample_rate);
	}
	else
	{
		raw_header->payload.sample_rate = 0;
	}
	
	raw_header->payload.pad1 = 0;

	return AUD_SUCCESS;
}

aud_error_t
ddp_add_audio_encoding_config_request
(
	ddp_packet_write_info_t * packet_info,
	ddp_request_id_t request_id,
	uint16_t control_flags,
	uint16_t encoding
)
{
	aud_error_t result = AUD_SUCCESS;
	ddp_message_write_info_t message_info;
	ddp_audio_encoding_config_request_t * raw_header;
	if (!packet_info)
	{
		return AUD_ERR_INVALIDPARAMETER;
	}

	result = ddp_packet_allocate_request(packet_info, &message_info,
										   DDP_OP_AUDIO_ENC_CONFIG,
										   request_id,
										   sizeof(raw_header->payload));

	if (result != AUD_SUCCESS)
	{
		return result;
	}

	raw_header = (ddp_audio_encoding_config_request_t *)message_info._.buf32;

	raw_header->payload.control_flags = htons(control_flags);

	if (AUD_FLAG_ISSET(control_flags, AUDIO_ENC_CONFIG__SET_ENCODING_FLAGS))
	{
		raw_header->payload.encoding = htons(encoding);
	}
	else
	{
		raw_header->payload.encoding = 0;
	}

	return AUD_SUCCESS;
}

#if defined(AUD_PLATFORM_ULTIMO)

aud_error_t
ddp_add_audio_signal_presence_config_request
(
	ddp_packet_write_info_t * packet_info,
	ddp_request_id_t request_id,
	uint16_t control_flags,
	uint8_t mode
)
{
	aud_error_t result = AUD_SUCCESS;
	ddp_message_write_info_t message_info;
	ddp_audio_signal_presence_config_request_t * raw_header;
	if (!packet_info)
	{
		return AUD_ERR_INVALIDPARAMETER;
	}

	result = ddp_packet_allocate_request
			(
				packet_info,
				&message_info,
				DDP_OP_AUDIO_SIGNAL_PRESENCE_CONFIG,
				request_id,
				sizeof(raw_header->payload)
			);

	if (result != AUD_SUCCESS)
	{
		return result;
	}

	raw_header = (ddp_audio_signal_presence_config_request_t *)message_info._.buf32;

	raw_header->payload.control_flags = htons(control_flags);

	if (AUD_FLAG_ISSET(control_flags, AUDIO_SIGNAL_PRESENCE_CONFIG__SET_MODE_FLAGS))
	{
		raw_header->payload.mode = mode;
	}
	else
	{
		raw_header->payload.mode = 0;
	}

	return AUD_SUCCESS;
}

#endif // defined(AUD_PLATFORM_ULTIMO)

#if defined(AUD_PLATFORM_BROOKLYN_II)

aud_error_t
ddp_add_audio_interface_request
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

	result = ddp_packet_allocate_request
			(
				packet_info, 
				&message_info,
				DDP_OP_AUDIO_INTERFACE,
				request_id,
				0
			);

	return result;
}

#endif // #if defined(AUD_PLATFORM_BROOKLYN_II)

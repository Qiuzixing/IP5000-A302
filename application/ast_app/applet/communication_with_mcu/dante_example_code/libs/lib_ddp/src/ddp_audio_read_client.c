/*
* File     : ddp_audio_read_client.c
* Created  : August 2014
* Updated  : Date: 2014/08/22
* Author   : Michael Ung <michael.ung@audinate.com>
* Synopsis : All audio DDP client read helper functions.
*
* Copyright 2005-2017, Audinate Pty Ltd and/or its licensors
Confidential
* Audinate Copyright Header Version 1
*/

#include "ddp/audio_client.h"
#include "ddp/audio_structures.h"

aud_error_t
ddp_read_audio_basic_response
(
	const ddp_packet_read_info_t * packet_info,
	uint16_t offset,
	ddp_request_id_t * out_request_id,
	uint16_t * out_status,
	uint32_t * out_capability_flags,
	uint32_t * out_default_sample_rate,
	uint16_t * out_default_encoding,
	uint16_t * out_rx_chans,
	uint16_t * out_tx_chans,
	uint8_t * out_change_flags
)
{
	aud_error_t result;
	ddp_message_read_info_t message_info;
	ddp_audio_basic_response_t * header;

	if (!packet_info || offset >= packet_info->packet_length_bytes)
	{
		return AUD_ERR_INVALIDPARAMETER;
	}
	result = ddp_packet_read_message(packet_info, &message_info, offset);
	if (result != AUD_SUCCESS)
	{
		return result;
	}
	result = ddp_packet_validate_message(&message_info, DDP_OP_AUDIO_BASIC, sizeof(ddp_audio_basic_response_t));
	if (result != AUD_SUCCESS)
	{
		return result;
	}
	header = (ddp_audio_basic_response_t *)message_info.header.header;

	if (out_request_id)
	{
		*out_request_id = ntohs(header->subheader.request_id);
	}
	if (out_status)
	{
		*out_status = ntohs(header->subheader.status);
	}
	if (out_capability_flags)
	{
		*out_capability_flags = ntohl(header->payload.capability_flags);
	}
	if (out_default_sample_rate)
	{
		*out_default_sample_rate = ntohl(header->payload.default_sample_rate);
	}
	if (out_default_encoding)
	{
		*out_default_encoding = ntohs(header->payload.default_encoding);
	}
	if (out_rx_chans)
	{
		*out_rx_chans = ntohs(header->payload.rx_chans);
	}
	if (out_tx_chans)
	{
		*out_tx_chans = ntohs(header->payload.tx_chans);
	}
	if (out_change_flags)
	{
		*out_change_flags = header->payload.change_flags;
	}

	return AUD_SUCCESS;
}

aud_error_t
ddp_read_audio_sample_rate_config_response
(
	const ddp_packet_read_info_t * packet_info,
	uint16_t offset,
	ddp_message_read_info_t * out_message_info,
	ddp_request_id_t * out_request_id,
	uint16_t * out_status,
	uint32_t * out_current_samplerate,
	uint32_t * out_reboot_srate,
	uint16_t * out_num_supported_srates
)
{
	aud_error_t result;
	ddp_audio_srate_config_response_t * header;
	uint16_t valid_flags;

	if (!packet_info || offset >= packet_info->packet_length_bytes)
	{
		return AUD_ERR_INVALIDPARAMETER;
	}
	result = ddp_packet_read_message(packet_info, out_message_info, offset);
	if (result != AUD_SUCCESS)
	{
		return result;
	}
	result = ddp_packet_validate_message(out_message_info, DDP_OP_AUDIO_SRATE_CONFIG, sizeof(ddp_audio_srate_config_response_t));
	if (result != AUD_SUCCESS)
	{
		return result;
	}
	header = (ddp_audio_srate_config_response_t *)out_message_info->header.header;
	valid_flags = ntohs(header->payload.valid_flags);
	
	if (out_request_id)
	{
		*out_request_id = ntohs(header->subheader.request_id);
	}
	if (out_status)
	{
		*out_status = ntohs(header->subheader.status);
	}
	if (out_current_samplerate)
	{
		*out_current_samplerate = (valid_flags & CURR_SAMPLE_RATE_VALID_FLAGS) ? ntohl(header->payload.current_samplerate) : 0;
	}
	if (out_reboot_srate)
	{
		*out_reboot_srate = (valid_flags & REBOOT_SAMPLE_RATE_VALID_FLAGS) ? ntohl(header->payload.reboot_samplerate) : 0;
	}
	if (out_num_supported_srates)
	{
		*out_num_supported_srates = ntohs(header->payload.num_supported_srates);
	}

	return AUD_SUCCESS;
}

aud_error_t
ddp_read_audio_sample_rate_config_supported_srate
(
	const ddp_message_read_info_t * message_info,
	uint16_t srate_idx,
	dante_samplerate_t * out_supported_srate
)
{
	aud_error_t result;
	ddp_audio_srate_config_response_t * header;
	uint16_t valid_flags;

	header = (ddp_audio_srate_config_response_t *)message_info->header.header;
	valid_flags = ntohs(header->payload.valid_flags);

	if (out_supported_srate)
	{
		if (valid_flags & SUPPORTED_SAMPLE_RATES_VALID_FLAGS)
		{
			ddp_heap_read_info_t heap_info;

			result = ddp_read_heap_array32(message_info,
										   &heap_info,
										   ntohs(header->payload.supported_srates_offset),
										   ntohs(header->payload.num_supported_srates));

			if (result != AUD_SUCCESS)
			{
				return result;
			}

			*out_supported_srate = ntohl(heap_info._.array32[srate_idx]);
		}
		else
		{
			*out_supported_srate = 0;
		}
	}

	return AUD_SUCCESS;
}



aud_error_t
ddp_read_audio_encoding_config_response
(
	const ddp_packet_read_info_t * packet_info,
	uint16_t offset,
	ddp_message_read_info_t * out_message_info,
	ddp_request_id_t * out_request_id,
	uint16_t * out_status,
	uint16_t * out_current_encoding,
	uint16_t * out_reboot_encoding,
	uint16_t * out_num_supported_encodings
)
{
	aud_error_t result;
	ddp_audio_encoding_config_response_t * header;
	uint16_t valid_flags;

	if (!packet_info || offset >= packet_info->packet_length_bytes)
	{
		return AUD_ERR_INVALIDPARAMETER;
	}
	result = ddp_packet_read_message(packet_info, out_message_info, offset);
	if (result != AUD_SUCCESS)
	{
		return result;
	}
	result = ddp_packet_validate_message(out_message_info, DDP_OP_AUDIO_ENC_CONFIG, sizeof(ddp_audio_encoding_config_response_t));
	if (result != AUD_SUCCESS)
	{
		return result;
	}
	header = (ddp_audio_encoding_config_response_t *)out_message_info->header.header;
	valid_flags = ntohs(header->payload.valid_flags);

	if (out_request_id)
	{
		*out_request_id = ntohs(header->subheader.request_id);
	}
	if (out_status)
	{
		*out_status = ntohs(header->subheader.status);
	}
	if (out_current_encoding)
	{
		*out_current_encoding = (valid_flags & AUDIO_ENC_CONFIG__CURRENT_ENC_VALID) ? ntohs(header->payload.current_encoding) : 0;
	}
	if (out_reboot_encoding)
	{
		*out_reboot_encoding = (valid_flags & AUDIO_ENC_CONFIG__REBOOT_ENC_VALID) ? ntohs(header->payload.reboot_encoding) : 0;
	}
	if (*out_num_supported_encodings)
	{
		*out_num_supported_encodings = ntohs(header->payload.num_supported_encs);
	}

	return AUD_SUCCESS;
}

aud_error_t
ddp_read_audio_encoding_config_supported_encoding
(
	const ddp_message_read_info_t * message_info,
	uint16_t enc_idx,
	uint16_t * out_supported_enc
)
{
	aud_error_t result;
	ddp_audio_encoding_config_response_t * header;
	uint16_t valid_flags;
	ddp_supported_enc_array_t *enc_elem;

	header = (ddp_audio_encoding_config_response_t *)message_info->header.header;
	valid_flags = ntohs(header->payload.valid_flags);

	if (out_supported_enc)
	{
		if (valid_flags & AUDIO_ENC_CONFIG__SUPPORTED_ENC_VALID)
		{
			ddp_heap_read_info_t heap_info;

			result = ddp_read_heap_arraystruct(message_info,
											   &heap_info,
											   ntohs(header->payload.supported_encs_offset),
											   ntohs(header->payload.num_supported_encs),
											   ntohs(header->payload.supported_encs_size));

			if (result != AUD_SUCCESS)
			{
				return result;
			}

			enc_elem = (ddp_supported_enc_array_t *)heap_info._.array8;
			enc_elem += enc_idx;
			*out_supported_enc = ntohs(enc_elem->encoding);
		}
		else
		{
			*out_supported_enc = 0;
		}
	}

	return AUD_SUCCESS;
}

#if defined(AUD_PLATFORM_ULTIMO)

aud_error_t
ddp_read_audio_signal_presence_config_response
(
	const ddp_packet_read_info_t * packet_info,
	uint16_t offset,
	ddp_message_read_info_t * out_message_info,
	ddp_request_id_t * out_request_id,
	uint16_t * out_status,
	uint8_t * out_current_mode
)
{
	aud_error_t result;
	ddp_audio_signal_presence_config_response_t * header;
	uint16_t valid_flags;

	if (!packet_info || offset >= packet_info->packet_length_bytes)
	{
		return AUD_ERR_INVALIDPARAMETER;
	}
	result = ddp_packet_read_message(packet_info, out_message_info, offset);
	if (result != AUD_SUCCESS)
	{
		return result;
	}
	result = ddp_packet_validate_message(out_message_info, DDP_OP_AUDIO_SIGNAL_PRESENCE_CONFIG, sizeof(ddp_audio_signal_presence_config_response_t));
	if (result != AUD_SUCCESS)
	{
		return result;
	}
	header = (ddp_audio_signal_presence_config_response_t *)out_message_info->header.header;
	valid_flags = ntohs(header->payload.valid_flags);

	if (out_request_id)
	{
		*out_request_id = ntohs(header->subheader.request_id);
	}
	if (out_status)
	{
		*out_status = ntohs(header->subheader.status);
	}
	if (out_current_mode)
	{
		*out_current_mode = (valid_flags & AUDIO_SIGNAL_PRESENCE_CONFIG__CURRENT_MODE_VALID) ? header->payload.current_mode : 0;
	}

	return AUD_SUCCESS;
}

aud_error_t
ddp_read_audio_signal_presence_data_response
(
	const ddp_packet_read_info_t * packet_info,
	uint16_t offset,
	ddp_message_read_info_t * out_message_info,
	ddp_request_id_t * out_request_id,
	uint16_t * out_status,
	uint16_t * out_num_tx_chans,
	uint16_t * out_num_rx_chans
)
{
	aud_error_t result;
	ddp_audio_signal_presence_data_response_t * header;
	uint16_t valid_flags;

	if (!packet_info || offset >= packet_info->packet_length_bytes)
	{
		return AUD_ERR_INVALIDPARAMETER;
	}
	result = ddp_packet_read_message(packet_info, out_message_info, offset);
	if (result != AUD_SUCCESS)
	{
		return result;
	}
	result = ddp_packet_validate_message(out_message_info, DDP_OP_AUDIO_SIGNAL_PRESENCE_DATA, sizeof(ddp_audio_signal_presence_data_response_t));
	if (result != AUD_SUCCESS)
	{
		return result;
	}
	header = (ddp_audio_signal_presence_data_response_t *)out_message_info->header.header;
	valid_flags = ntohs(header->payload.valid_flags);

	if (out_request_id)
	{
		*out_request_id = ntohs(header->subheader.request_id);
	}
	if (out_status)
	{
		*out_status = ntohs(header->subheader.status);
	}
	if (out_num_tx_chans)
	{
		*out_num_tx_chans = (valid_flags & AUDIO_SIGNAL_PRESENCE_DATA__NUM_TX_CHANS_VALID) ? ntohs(header->payload.num_tx_chans) : 0;
	}
	if (out_num_rx_chans)
	{
		*out_num_rx_chans = (valid_flags & AUDIO_SIGNAL_PRESENCE_DATA__NUM_RX_CHANS_VALID) ? ntohs(header->payload.num_rx_chans) : 0;
	}

	return AUD_SUCCESS;
}

aud_error_t
ddp_read_audio_signal_presence_data_tx_chan_value
(
	const ddp_message_read_info_t * message_info,
	uint16_t tx_chan_index,
	uint8_t * out_signal_presence
)
{
	aud_error_t result;
	ddp_audio_signal_presence_data_response_t * header;
	uint16_t valid_flags;
	const uint8_t *signal_presence_data;

	header = (ddp_audio_signal_presence_data_response_t *)message_info->header.header;
	valid_flags = ntohs(header->payload.valid_flags);

	//check for valid flags
	if ((valid_flags & (AUDIO_SIGNAL_PRESENCE_DATA__NUM_TX_CHANS_VALID | AUDIO_SIGNAL_PRESENCE_DATA__TX_CHAN_SIZE_VALID)) == 0)
	{
		return AUD_ERR_INVALIDDATA;
	}

	if (tx_chan_index >= header->payload.num_tx_chans)
	{
		return AUD_ERR_INVALIDPARAMETER;
	}
	
	if (out_signal_presence)
	{
		ddp_heap_read_info_t heap_info;

		result = ddp_read_heap_arraystruct(message_info,
										   &heap_info,
										   ntohs(header->payload.tx_chan_array_offset),
										   ntohs(header->payload.num_tx_chans),
										   header->payload.tx_chan_array_element_size);

		if (result != AUD_SUCCESS)
		{
			return result;
		}

		signal_presence_data = heap_info._.array8;
		signal_presence_data += tx_chan_index;
		*out_signal_presence = *signal_presence_data;
	}

	return AUD_SUCCESS;
}

aud_error_t
ddp_read_audio_signal_presence_data_rx_chan_value
(
	const ddp_message_read_info_t * message_info,
	uint16_t rx_chan_index,
	uint8_t * out_signal_presence
)
{
	aud_error_t result;
	ddp_audio_signal_presence_data_response_t * header;
	uint16_t valid_flags;
	const uint8_t *signal_presence_data;

	header = (ddp_audio_signal_presence_data_response_t *)message_info->header.header;
	valid_flags = ntohs(header->payload.valid_flags);

	//check for valid flags
	if( (valid_flags & (AUDIO_SIGNAL_PRESENCE_DATA__NUM_RX_CHANS_VALID|AUDIO_SIGNAL_PRESENCE_DATA__RX_CHAN_SIZE_VALID)) == 0)
	{
		return AUD_ERR_INVALIDDATA;
	}

	if (rx_chan_index >= header->payload.num_rx_chans)
	{
		return AUD_ERR_INVALIDPARAMETER;
	}
	
	if (out_signal_presence)
	{
		ddp_heap_read_info_t heap_info;

		result = ddp_read_heap_arraystruct(message_info,
										   &heap_info,
										   ntohs(header->payload.rx_chan_array_offset),
										   ntohs(header->payload.num_rx_chans),
										   header->payload.rx_chan_array_element_size);

		if (result != AUD_SUCCESS)
		{
			return result;
		}

		signal_presence_data = heap_info._.array8;
		signal_presence_data += rx_chan_index;
		*out_signal_presence = *signal_presence_data;
	}

	return AUD_SUCCESS;
}

#endif // defined(AUD_PLATFORM_ULTIMO)

#if defined(AUD_PLATFORM_BROOKLYN_II)

aud_error_t
ddp_read_audio_interface_response
(
	const ddp_packet_read_info_t * packet_info,
	uint16_t offset,
	ddp_request_id_t * out_request_id,
	uint16_t * out_status,
	uint8_t * out_chans_per_tdm,
	uint8_t * out_frame_type,
	uint8_t * out_align_type,
	uint8_t * out_chan_map_type
)
{
	aud_error_t result;
	ddp_message_read_info_t message_info;
	ddp_audio_interface_response_t * header;

	if (!packet_info || offset >= packet_info->packet_length_bytes)
	{
		return AUD_ERR_INVALIDPARAMETER;
	}
	result = ddp_packet_read_message(packet_info, &message_info, offset);
	if (result != AUD_SUCCESS)
	{
		return result;
	}
	result = ddp_packet_validate_message(&message_info, DDP_OP_AUDIO_INTERFACE, sizeof(ddp_audio_interface_response_t));
	if (result != AUD_SUCCESS)
	{
		return result;
	}
	header = (ddp_audio_interface_response_t *)message_info.header.header;

	if (out_request_id)
	{
		*out_request_id = ntohs(header->subheader.request_id);
	}
	if (out_status)
	{
		*out_status = ntohs(header->subheader.status);
	}
	if (out_chans_per_tdm)
	{
		*out_chans_per_tdm = header->payload.chans_per_tdm;
	}
	if (out_frame_type)
	{
		*out_frame_type = header->payload.frame_type;
	}
	if (out_align_type)
	{
		*out_align_type = header->payload.align_type;
	}
	if (out_chan_map_type)
	{
		*out_chan_map_type = header->payload.chan_map_type;
	}

	return AUD_SUCCESS;
}

#endif // defined(AUD_PLATFORM_BROOKLYN_II)

/*
* File     : ddp_device_read_client.c
* Created  : August 2014
* Updated  : Date: 2014/08/22
* Author   : Michael Ung <michael.ung@audinate.com>
* Synopsis : All device DDP client read helper functions.
*
* Copyright 2005-2017, Audinate Pty Ltd and/or its licensors
Confidential
* Audinate Copyright Header Version 1
*/

#include "ddp/device_client.h"

aud_error_t
ddp_read_device_general_response
(
	const ddp_packet_read_info_t * packet_info,
	uint16_t offset,
	ddp_request_id_t* out_request_id,
	ddp_status_t* out_status,
	dante_id64_t** out_model_id,
	char** out_model_id_string,
	uint32_t* out_software_version,
	uint32_t* out_software_build,
	uint32_t* out_firmware_version,
	uint32_t* out_firmware_build,
	uint32_t* out_bootloader_version,
	uint32_t* out_bootloader_build,
	uint32_t* out_api_version,
	uint32_t* out_capability_flags,
	uint32_t* out_status_flags
)
{
	aud_error_t result;
	ddp_message_read_info_t message_info;
	ddp_device_general_response_t * header;

	if (!packet_info || offset >= packet_info->packet_length_bytes)
	{
		return AUD_ERR_INVALIDPARAMETER;
	}
	result = ddp_packet_read_message(packet_info, &message_info, offset);
	if (result != AUD_SUCCESS)
	{
		return result;
	}
	result = ddp_packet_validate_message(&message_info, DDP_OP_DEVICE_GENERAL, sizeof(ddp_device_general_response_t));
	if (result != AUD_SUCCESS)
	{
		return result;
	}
	header = (ddp_device_general_response_t *) message_info.header.header;
	if (out_request_id)
	{
		*out_request_id = ntohs(header->subheader.request_id);
	}
	if (out_status)
	{
		*out_status = ntohs(header->subheader.status);
	}
	if (out_model_id)
	{
		*out_model_id = &header->payload.model_id;
	}
	if (out_model_id_string)
	{
		if(ntohs(header->payload.model_id_string_offset) != 0)
		{
			*out_model_id_string = (char*)header + ntohs(header->payload.model_id_string_offset);
		}
		else
		{
			*out_model_id_string = NULL;
		}
	}
	if (out_software_version)
	{
		*out_software_version = ntohl(header->payload.software_version);
	}
	if (out_software_build)
	{
		*out_software_build = ntohl(header->payload.software_build);
	}
	if (out_firmware_version)
	{
		*out_firmware_version = ntohl(header->payload.firmware_version);
	}
	if (out_firmware_build)
	{
		*out_firmware_build = ntohl(header->payload.firmware_build);
	}
	if (out_bootloader_version)
	{
		*out_bootloader_version = ntohl(header->payload.bootloader_version);
	}
	if (out_bootloader_build)
	{
		*out_bootloader_build = ntohl(header->payload.bootloader_build);
	}
	if (out_api_version)
	{
		*out_api_version = ntohl(header->payload.api_version);
	}
	if (out_capability_flags)
	{
		*out_capability_flags = ntohl(header->payload.capability_flags);
	}
	if (out_status_flags)
	{
		*out_status_flags = ntohl(header->payload.status_flags);
	}

	return AUD_SUCCESS;
}

aud_error_t
ddp_read_device_manufacturer_response
(
	const ddp_packet_read_info_t * packet_info,
	uint16_t offset,
	ddp_request_id_t* out_request_id,
	ddp_status_t* out_status,
	dante_id64_t** out_manf_id,
	char** out_manf_name_string,
	dante_id64_t** out_model_id,
	char** out_model_id_string,
	uint32_t* out_software_version,
	uint32_t* out_software_build,
	uint32_t* out_firmware_version,
	uint32_t* out_firmware_build,
	uint32_t* out_capability_flags,
	uint32_t* out_model_version,
	char** out_model_version_string
)
{
	aud_error_t result;
	ddp_message_read_info_t message_info;
	ddp_device_manf_response_t * header;

	if (!packet_info || offset >= packet_info->packet_length_bytes)
	{
		return AUD_ERR_INVALIDPARAMETER;
	}
	result = ddp_packet_read_message(packet_info, &message_info, offset);
	if (result != AUD_SUCCESS)
	{
		return result;
	}
	result = ddp_packet_validate_message(&message_info, DDP_OP_DEVICE_MANF, sizeof(ddp_device_manf_response_t));
	if (result != AUD_SUCCESS)
	{
		return result;
	}
	header = (ddp_device_manf_response_t *) message_info.header.header;
	if (out_request_id)
	{
		*out_request_id = ntohs(header->subheader.request_id);
	}
	if (out_status)
	{
		*out_status = ntohs(header->subheader.status);
	}
	if (out_manf_id)
	{
		*out_manf_id = &header->payload.manf_id;
	}
	if (out_manf_name_string)
	{
		if(ntohs(header->payload.manf_name_string_offset) != 0)
		{
			*out_manf_name_string = (char*)header + ntohs(header->payload.manf_name_string_offset);
		}
		else
		{
			*out_manf_name_string = NULL;
		}
	}
	if (out_model_id)
	{
		*out_model_id = &header->payload.model_id;
	}
	if (out_model_id_string)
	{
		if(ntohs(header->payload.model_id_string_offset) != 0)
		{
			*out_model_id_string = (char*)header + ntohs(header->payload.model_id_string_offset);
		}
		else
		{
			*out_model_id_string = NULL;
		}
	}
	if (out_software_version)
	{
		*out_software_version = ntohl(header->payload.software_version);
	}
	if (out_software_build)
	{
		*out_software_build = ntohl(header->payload.software_build);
	}
	if (out_firmware_version)
	{
		*out_firmware_version = ntohl(header->payload.firmware_version);
	}
	if (out_firmware_build)
	{
		*out_firmware_build = ntohl(header->payload.firmware_build);
	}

	if (out_capability_flags)
	{
		*out_capability_flags = ntohl(header->payload.capability_flags);
	}
	if (out_model_version)
	{
		*out_model_version = ntohl(header->payload.model_version);
	}
	if (out_model_version_string)
	{
		if(ntohs(header->payload.model_version_string_offset) != 0)
		{
			*out_model_version_string = (char*)header + ntohs(header->payload.model_version_string_offset);
		}
		else
		{
			*out_model_version_string = NULL;
		}
	}

	return AUD_SUCCESS;
}

aud_error_t
ddp_read_device_erase_response
(
	const ddp_packet_read_info_t * packet_info,
	uint16_t offset,
	ddp_request_id_t* out_request_id,
	ddp_status_t* out_status,
	uint16_t* out_supported_flags,
	uint16_t* out_executed_flags
)
{
	aud_error_t result;
	ddp_message_read_info_t message_info;
	ddp_device_erase_response_t * header;

	if (!packet_info || offset >= packet_info->packet_length_bytes)
	{
		return AUD_ERR_INVALIDPARAMETER;
	}
	result = ddp_packet_read_message(packet_info, &message_info, offset);
	if (result != AUD_SUCCESS)
	{
		return result;
	}
	result = ddp_packet_validate_message(&message_info, DDP_OP_DEVICE_ERASE_CONFIG, sizeof(ddp_device_erase_response_t));
	if (result != AUD_SUCCESS)
	{
		return result;
	}
	header = (ddp_device_erase_response_t *) message_info.header.header;
	if (out_request_id)
	{
		*out_request_id = ntohs(header->subheader.request_id);
	}
	if (out_status)
	{
		*out_status = ntohs(header->subheader.status);
	}

	if (out_supported_flags)
	{
		*out_supported_flags = ntohs(header->payload.supported_flags);
	}
	if (out_executed_flags)
	{
		*out_executed_flags = ntohs(header->payload.executed_flags);
	}

	return AUD_SUCCESS;
}

aud_error_t
ddp_read_device_reboot_response
(
	const ddp_packet_read_info_t * packet_info,
	uint16_t offset,
	ddp_request_id_t* out_request_id,
	ddp_status_t* out_status,
	uint16_t* out_supported_flags,
	uint16_t* out_executed_flags
)
{
	aud_error_t result;
	ddp_message_read_info_t message_info;
	ddp_device_reboot_response_t * header;

	if (!packet_info || offset >= packet_info->packet_length_bytes)
	{
		return AUD_ERR_INVALIDPARAMETER;
	}
	result = ddp_packet_read_message(packet_info, &message_info, offset);
	if (result != AUD_SUCCESS)
	{
		return result;
	}
	result = ddp_packet_validate_message(&message_info, DDP_OP_DEVICE_REBOOT, sizeof(ddp_device_reboot_response_t));
	if (result != AUD_SUCCESS)
	{
		return result;
	}
	header = (ddp_device_reboot_response_t *) message_info.header.header;
	if (out_request_id)
	{
		*out_request_id = ntohs(header->subheader.request_id);
	}
	if (out_status)
	{
		*out_status = ntohs(header->subheader.status);
	}

	if (out_supported_flags)
	{
		*out_supported_flags = ntohs(header->payload.supported_flags);
	}
	if (out_executed_flags)
	{
		*out_executed_flags = ntohs(header->payload.executed_flags);
	}

	return AUD_SUCCESS;
}

aud_error_t
ddp_read_device_upgrade_response
(
	const ddp_packet_read_info_t * packet_info,
	uint16_t offset,
	ddp_request_id_t* out_request_id,
	ddp_status_t* out_status,
	ddp_upgrade_rsp_payload_t* out_payload
)
{
	aud_error_t result;
	ddp_message_read_info_t message_info;
	ddp_device_upgrade_response_t * header;

	if (!packet_info || offset >= packet_info->packet_length_bytes)
	{
		return AUD_ERR_INVALIDPARAMETER;
	}
	result = ddp_packet_read_message(packet_info, &message_info, offset);
	if (result != AUD_SUCCESS)
	{
		return result;
	}
	result = ddp_packet_validate_message(&message_info, DDP_OP_DEVICE_UPGRADE, sizeof(ddp_device_upgrade_response_t));
	if (result != AUD_SUCCESS)
	{
		return result;
	}
	header = (ddp_device_upgrade_response_t *) message_info.header.header;
	if (out_request_id)
	{
		*out_request_id = ntohs(header->subheader.request_id);
	}
	if (out_status)
	{
		*out_status = ntohs(header->subheader.status);
	}

	if (out_payload)
	{
		out_payload->valid_flags = ntohs(header->payload.valid_flags);
		out_payload->upgrade_stage = ntohs(header->payload.upgrade_stage);
		out_payload->upgrade_error = ntohl(header->payload.upgrade_error);
		out_payload->progress_current = ntohl(header->payload.progress_current);
		out_payload->progress_total = ntohl(header->payload.progress_total);
		out_payload->manf_id = &header->payload.manf_id;
		out_payload->model_id = &header->payload.model_id;
	}

	return AUD_SUCCESS;
}


aud_error_t
ddp_read_device_identity_response
(
	const ddp_packet_read_info_t * packet_info,
	uint16_t offset,
	ddp_request_id_t* out_request_id,
	ddp_status_t* out_status,
	uint16_t* out_identity_status_flags,
	uint16_t* out_process_id,
	dante_device_id_t** out_dante_device_id,
	const char** out_default_name_string,
	const char** out_friendly_name_string,
	const char** out_dante_domain_string,
	const char** out_advertised_name_string
)
{
	aud_error_t result;
	ddp_message_read_info_t message_info;
	ddp_device_identity_response_t * header;

	if (!packet_info || offset >= packet_info->packet_length_bytes)
	{
		return AUD_ERR_INVALIDPARAMETER;
	}
	result = ddp_packet_read_message(packet_info, &message_info, offset);
	if (result != AUD_SUCCESS)
	{
		return result;
	}
	result = ddp_packet_validate_message(&message_info, DDP_OP_DEVICE_IDENTITY, sizeof(ddp_device_identity_response_t));
	if (result != AUD_SUCCESS)
	{
		return result;
	}
	header = (ddp_device_identity_response_t *)message_info.header.header;
	if (out_request_id)
	{
		*out_request_id = ntohs(header->subheader.request_id);
	}
	if (out_status)
	{
		*out_status = ntohs(header->subheader.status);
	}
	if (out_identity_status_flags)
	{
		*out_identity_status_flags = ntohs(header->payload.identity_status_flags);
	}
	if (out_process_id)
	{
		*out_process_id = ntohs(header->payload.process_id);
	}
	if (out_dante_device_id)
	{
		*out_dante_device_id = &header->payload.device_id;
	}
	if (out_default_name_string)
	{
		uint16_t default_name_offset = ntohs(header->payload.default_name_offset);
		if (default_name_offset != 0)
		{
			*out_default_name_string = (char*)header + default_name_offset;
		}
		else
		{
			*out_default_name_string = NULL;
		}
	}
	if (out_friendly_name_string)
	{
		uint16_t friendly_name_offset = ntohs(header->payload.friendly_name_offset);
		if (friendly_name_offset != 0)
		{
			*out_friendly_name_string = (char*)header + friendly_name_offset;
		}
		else
		{
			*out_friendly_name_string = NULL;
		}
	}
	if (out_dante_domain_string)
	{
		uint16_t dante_domain_offset = ntohs(header->payload.dante_domain_offset);
		if (dante_domain_offset != 0)
		{
			*out_dante_domain_string = (char*)header + dante_domain_offset;
		}
		else
		{
			*out_dante_domain_string = NULL;
		}
	}
	if (out_advertised_name_string)
	{
		uint16_t advertised_name_offset = ntohs(header->payload.advertised_name_offset);
		if (advertised_name_offset != 0)
		{
			*out_advertised_name_string = (char*)header + advertised_name_offset;
		}
		else
		{
			*out_advertised_name_string = NULL;
		}
	}

	return AUD_SUCCESS;
}

aud_error_t
ddp_read_device_identify_response
(
	const ddp_packet_read_info_t * packet_info,
	uint16_t offset,
	ddp_request_id_t* out_request_id,
	ddp_status_t* out_status
)
{
	aud_error_t result;
	ddp_message_read_info_t message_info;
	ddp_device_identify_response_t * header;

	if (!packet_info || offset >= packet_info->packet_length_bytes)
	{
		return AUD_ERR_INVALIDPARAMETER;
	}
	result = ddp_packet_read_message(packet_info, &message_info, offset);
	if (result != AUD_SUCCESS)
	{
		return result;
	}
	result = ddp_packet_validate_message(&message_info, DDP_OP_DEVICE_IDENTIFY, sizeof(ddp_device_identify_response_t));
	if (result != AUD_SUCCESS)
	{
		return result;
	}
	header = (ddp_device_identify_response_t *)message_info.header.header;
	if (out_request_id)
	{
		*out_request_id = ntohs(header->subheader.request_id);
	}
	if (out_status)
	{
		*out_status = ntohs(header->subheader.status);
	}

	return AUD_SUCCESS;
}

aud_error_t
ddp_read_device_gpio_response
(
	const ddp_packet_read_info_t * packet_info,
	uint16_t offset,
	ddp_request_id_t * out_request_id,
	ddp_status_t * out_status,
	uint32_t * out_interrupt_trigger_mask,
	uint32_t * out_input_state_valid_mask,
	uint32_t * out_input_state_values,
	uint32_t * out_output_state_valid_mask,
	uint32_t * out_output_state_values
)
{
	aud_error_t result;
	ddp_message_read_info_t message_info;
	ddp_device_gpio_response_t * header;

	if (!packet_info || offset >= packet_info->packet_length_bytes)
	{
		return AUD_ERR_INVALIDPARAMETER;
	}
	result = ddp_packet_read_message(packet_info, &message_info, offset);
	if (result != AUD_SUCCESS)
	{
		return result;
	}
	result = ddp_packet_validate_message(&message_info, DDP_OP_DEVICE_GPIO, sizeof(ddp_device_gpio_response_t));
	if (result != AUD_SUCCESS)
	{
		return result;
	}
	header = (ddp_device_gpio_response_t *)message_info.header.header;
	if (out_request_id)
	{
		*out_request_id = ntohs(header->subheader.request_id);
	}
	if (out_status)
	{
		*out_status = ntohs(header->subheader.status);
	}
	if(out_interrupt_trigger_mask)
	{
		*out_interrupt_trigger_mask = ntohl(header->payload.interrupt_trigger_mask);
	}
	if(out_input_state_valid_mask)
	{
		*out_input_state_valid_mask = ntohl(header->payload.input_state_valid_mask);
	}
	if(out_input_state_values)
	{
		*out_input_state_values = ntohl(header->payload.input_state_values);
	}
	if(out_output_state_valid_mask)
	{
		*out_output_state_valid_mask = ntohl(header->payload.output_state_valid_mask);
	}
	if(out_output_state_values)
	{
		*out_output_state_values = ntohl(header->payload.output_state_values);
	}

	return AUD_SUCCESS;

}

#if defined(AUD_PLATFORM_ULTIMO)
aud_error_t
ddp_read_device_switch_led_response
(
	const ddp_packet_read_info_t * packet_info,
	uint16_t offset,
	ddp_request_id_t * out_request_id,
	ddp_status_t * out_status,
	uint8_t * out_switch_led_mode
)
{
	aud_error_t result;
	ddp_message_read_info_t message_info;
	ddp_device_switch_led_response_t * header;

	if (!packet_info || offset >= packet_info->packet_length_bytes)
	{
		return AUD_ERR_INVALIDPARAMETER;
	}
	result = ddp_packet_read_message(packet_info, &message_info, offset);
	if (result != AUD_SUCCESS)
	{
		return result;
	}
	result = ddp_packet_validate_message(&message_info, DDP_OP_DEVICE_SWITCH_LED, sizeof(ddp_device_switch_led_response_t));
	if (result != AUD_SUCCESS)
	{
		return result;
	}
	header = (ddp_device_switch_led_response_t *)message_info.header.header;
	if (out_request_id)
	{
		*out_request_id = ntohs(header->subheader.request_id);
	}
	if (out_status)
	{
		*out_status = ntohs(header->subheader.status);
	}
	if(out_switch_led_mode)
	{
		*out_switch_led_mode = header->payload.switch_led_mode;
	}
	return AUD_SUCCESS;
}

#endif // defined(AUD_PLATFORM_ULTIMO)

aud_error_t
ddp_read_device_aes67_response
(
	const ddp_packet_read_info_t * packet_info,
	uint16_t offset,
	ddp_request_id_t * out_request_id,
	ddp_status_t * out_status,
	uint8_t * out_aes67_support,
	uint8_t * out_aes67_current,
	uint8_t * out_aes67_reboot
)
{
	aud_error_t result;
	ddp_message_read_info_t message_info;
	ddp_device_aes67_response_t * header;

	if (!packet_info || offset >= packet_info->packet_length_bytes)
	{
		return AUD_ERR_INVALIDPARAMETER;
	}
	result = ddp_packet_read_message(packet_info, &message_info, offset);
	if (result != AUD_SUCCESS)
	{
		return result;
	}
	result = ddp_packet_validate_message(&message_info, DDP_OP_DEVICE_AES67, sizeof(ddp_device_aes67_response_t));
	if (result != AUD_SUCCESS)
	{
		return result;
	}
	header = (ddp_device_aes67_response_t *)message_info.header.header;
	if (out_request_id)
	{
		*out_request_id = ntohs(header->subheader.request_id);
	}
	if (out_status)
	{
		*out_status = ntohs(header->subheader.status);
	}
	if(out_aes67_support)
	{
		*out_aes67_support = header->payload.device_aes67_support;
	}
	if(out_aes67_current)
	{
		*out_aes67_current = header->payload.device_aes67_current;
	}
	if(out_aes67_reboot)
	{
		*out_aes67_reboot = header->payload.device_aes67_reboot;
	}

	return AUD_SUCCESS;
}

aud_error_t
ddp_read_device_lock_unlock_response
(
	const ddp_packet_read_info_t * packet_info,
	uint16_t offset,
	ddp_request_id_t * out_request_id,
	ddp_status_t * out_status,
	uint8_t * out_lock_unlock_status
)
{
	aud_error_t result;
	ddp_message_read_info_t message_info;
	ddp_device_lockunlock_response_t * header;

	if (!packet_info || offset >= packet_info->packet_length_bytes)
	{
		return AUD_ERR_INVALIDPARAMETER;
	}
	result = ddp_packet_read_message(packet_info, &message_info, offset);
	if (result != AUD_SUCCESS)
	{
		return result;
	}
	result = ddp_packet_validate_message(&message_info, DDP_OP_DEVICE_LOCK_UNLOCK, sizeof(ddp_device_lockunlock_response_t));
	if (result != AUD_SUCCESS)
	{
		return result;
	}
	header = (ddp_device_lockunlock_response_t *)message_info.header.header;
	if (out_request_id)
	{
		*out_request_id = ntohs(header->subheader.request_id);
	}
	if (out_status)
	{
		*out_status = ntohs(header->subheader.status);
	}
	if(out_lock_unlock_status)
	{
		*out_lock_unlock_status = header->payload.lock_unlock_status;
	}
	return AUD_SUCCESS;
}

#if defined(AUD_PLATFORM_BROOKLYN_II)

aud_error_t
ddp_read_device_switch_redundancy_response
(
	const ddp_packet_read_info_t * packet_info,
	uint16_t offset,
	ddp_request_id_t * out_request_id,
	ddp_status_t * out_status,
	uint8_t * out_device_switch_redundancy_support,
	uint8_t * out_device_switch_redundancy_current,
	uint8_t * out_device_switch_redundancy_reboot
)
{
	aud_error_t result;
	ddp_message_read_info_t message_info;
	ddp_device_switch_redundancy_response_t * header;

	if (!packet_info || offset >= packet_info->packet_length_bytes)
	{
		return AUD_ERR_INVALIDPARAMETER;
	}
	result = ddp_packet_read_message(packet_info, &message_info, offset);
	if (result != AUD_SUCCESS)
	{
		return result;
	}
	result = ddp_packet_validate_message(&message_info, DDP_OP_DEVICE_SWITCH_REDUNDANCY, sizeof(ddp_device_switch_redundancy_response_t));
	if (result != AUD_SUCCESS)
	{
		return result;
	}
	header = (ddp_device_switch_redundancy_response_t *)message_info.header.header;
	if (out_request_id)
	{
		*out_request_id = ntohs(header->subheader.request_id);
	}
	if (out_status)
	{
		*out_status = ntohs(header->subheader.status);
	}
	if(out_device_switch_redundancy_support)
	{
		*out_device_switch_redundancy_support = header->payload.device_switch_redundancy_support;
	}
	if(out_device_switch_redundancy_current)
	{
		*out_device_switch_redundancy_current = header->payload.device_switch_redundancy_current;
	}
	if(out_device_switch_redundancy_reboot)
	{
		*out_device_switch_redundancy_reboot = header->payload.device_switch_redundancy_reboot;
	}

	return AUD_SUCCESS;
}

aud_error_t
ddp_read_uart_config_response_header
(
	const ddp_packet_read_info_t * packet_info,
	uint16_t offset,
	ddp_message_read_info_t * out_message_info,
	ddp_request_id_t * out_request_id,
	ddp_status_t * out_status,
	uint8_t * out_uart_number
)
{
	aud_error_t result;
	ddp_device_uart_config_response_t * header;

	if (!packet_info || offset >= packet_info->packet_length_bytes)
	{
		return AUD_ERR_INVALIDPARAMETER;
	}

	result = ddp_packet_read_message(packet_info, out_message_info, offset);
	if (result != AUD_SUCCESS)
	{
		return result;
	}

	result = ddp_packet_validate_message(out_message_info, DDP_OP_DEVICE_UART_CONFIG, sizeof(ddp_device_uart_config_response_t));
	if (result != AUD_SUCCESS)
	{
		return result;
	}

	header = (ddp_device_uart_config_response_t *)out_message_info->header.header;

	if (out_request_id)
	{
		*out_request_id = ntohs(header->subheader.request_id);
	}
	if (out_status)
	{
		*out_status = ntohs(header->subheader.status);
	}

	if (out_uart_number)
	{
		*out_uart_number = header->payload.uart_number;
	}

	return AUD_SUCCESS;
}

aud_error_t
ddp_read_uart_config_response_uart_st_array
(
	const ddp_message_read_info_t * message_info,
	uint8_t uart_num,
	uint8_t uart_idx,
	uint8_t * out_uart_mode,
	uint8_t * out_uart_bits,
	uint8_t * out_uart_parity,
	uint8_t * out_uart_stop_bits,
	uint8_t * out_uart_user_configurable,
	uint32_t * out_uart_speed
)
{
	aud_error_t result;
	ddp_device_uart_config_response_t *header;
	ddp_heap_read_info_t heap_info;
	ddp_device_uart_config_response_payload_t *uart_config_elem;

	if (!message_info)
	{
		return AUD_ERR_INVALIDPARAMETER;
	}

	header = (ddp_device_uart_config_response_t *)message_info->header.header;

	if (uart_idx >= uart_num)
	{
		return AUD_ERR_INVALIDPARAMETER;
	}

	result = ddp_read_heap_arraystruct(message_info,
			&heap_info,
			ntohs(header->payload.uart_config_offset),
			uart_num,
			ntohs(header->payload.uart_config_size));

	if (result != AUD_SUCCESS)
	{
		return result;
	}

	uart_config_elem = ((ddp_device_uart_config_response_payload_t*)heap_info._.array32) + uart_idx;

	*out_uart_mode = uart_config_elem->uart_mode;
	*out_uart_bits = uart_config_elem->uart_bits;
	*out_uart_parity = uart_config_elem->uart_parity;
	*out_uart_stop_bits = uart_config_elem->uart_stop_bits;
	*out_uart_user_configurable = uart_config_elem->uart_user_configurable;
	*out_uart_speed = ntohl(uart_config_elem->uart_speed);
	return AUD_SUCCESS;
}

aud_error_t
ddp_read_device_meter_config_response
(
	const ddp_packet_read_info_t * packet_info,
	uint16_t offset,
	ddp_request_id_t * out_request_id,
	ddp_status_t * out_status,
	uint16_t * out_device_current_meter_config
)
{
	aud_error_t result;
	ddp_message_read_info_t message_info;
	ddp_device_meter_config_response_t * header;

	if (!packet_info || offset >= packet_info->packet_length_bytes)
	{
		return AUD_ERR_INVALIDPARAMETER;
	}
	result = ddp_packet_read_message(packet_info, &message_info, offset);
	if (result != AUD_SUCCESS)
	{
		return result;
	}
	result = ddp_packet_validate_message(&message_info, DDP_OP_DEVICE_METER_CONFIG, sizeof(ddp_device_meter_config_response_t));
	if (result != AUD_SUCCESS)
	{
		return result;
	}
	header = (ddp_device_meter_config_response_t *)message_info.header.header;
	if (out_request_id)
	{
		*out_request_id = ntohs(header->subheader.request_id);
	}
	if (out_status)
	{
		*out_status = ntohs(header->subheader.status);
	}
	if(out_device_current_meter_config)
	{
		*out_device_current_meter_config = ntohs(header->payload.current_meter_config_rate);
	}

	return AUD_SUCCESS;
}

#endif // defined(AUD_PLATFORM_BROOKLYN_II)

aud_error_t
ddp_read_device_vlan_config_response_header
(
	const ddp_packet_read_info_t * packet_info,
	uint16_t offset,
	ddp_message_read_info_t * out_message_info,
	ddp_request_id_t * out_request_id,
	ddp_status_t * out_status,
	uint8_t * out_vlan_max_num,
	uint8_t * out_vlan_num,
	uint8_t * out_vlan_current_id,
	uint8_t * out_vlan_reboot_id,
	uint16_t * out_vlan_config_port_mask
)
{
	aud_error_t result;
	ddp_device_vlan_config_response_t * header;

	if (!packet_info || offset >= packet_info->packet_length_bytes)
	{
		return AUD_ERR_INVALIDPARAMETER;
	}

	result = ddp_packet_read_message(packet_info, out_message_info, offset);
	if (result != AUD_SUCCESS)
	{
		return result;
	}

	result = ddp_packet_validate_message(out_message_info, DDP_OP_DEVICE_VLAN_CONFIG, sizeof(ddp_device_vlan_config_response_t));
	if (result != AUD_SUCCESS)
	{
		return result;
	}

	header = (ddp_device_vlan_config_response_t *)out_message_info->header.header;

	if (out_request_id)
	{
		*out_request_id = ntohs(header->subheader.request_id);
	}
	if (out_status)
	{
		*out_status = ntohs(header->subheader.status);
	}

	if (out_vlan_max_num)
	{
		*out_vlan_max_num = header->payload.vlan_max_number;
	}
	if (out_vlan_num)
	{
		*out_vlan_num = header->payload.vlan_config_number;
	}
	if (out_vlan_current_id)
	{
		*out_vlan_current_id = header->payload.vlan_current_config_id;
	}
	if (out_vlan_reboot_id)
	{
		*out_vlan_reboot_id = header->payload.vlan_reboot_config_id;
	}
	if (out_vlan_config_port_mask)
	{
		*out_vlan_config_port_mask = ntohs(header->payload.vlan_config_port_mask);
	}


	return AUD_SUCCESS;
}

aud_error_t
ddp_read_device_vlan_config_response_vlan_st_array
(
	const ddp_message_read_info_t * message_info,
	uint8_t vlan_num,
	uint8_t vlan_idx,
	uint32_t * out_vlan_primary_bitmap,
	uint32_t * out_vlan_secondary_bitmap,
	uint32_t * out_vlan_user_2_bitmap,
	uint32_t * out_vlan_user_3_bitmap
)
{
	aud_error_t result;
	ddp_device_vlan_config_response_t *header;
	ddp_heap_read_info_t heap_info;
	ddp_device_vlan_config_response_payload_t *vlan_config_elem;

	if (!message_info)
	{
		return AUD_ERR_INVALIDPARAMETER;
	}

	header = (ddp_device_vlan_config_response_t *)message_info->header.header;

	if (vlan_idx > vlan_num)
	{
		return AUD_ERR_INVALIDPARAMETER;
	}

	result = ddp_read_heap_arraystruct(message_info,
		&heap_info,
		ntohs(header->payload.vlan_config_structure_offset),
		vlan_num,
		ntohs(header->payload.vlan_config_structure_size));

	if (result != AUD_SUCCESS)
	{
		return result;
	}

	vlan_config_elem = ((ddp_device_vlan_config_response_payload_t*)heap_info._.array32) + vlan_idx;

	if (out_vlan_primary_bitmap)
	{
		*out_vlan_primary_bitmap = ntohl(vlan_config_elem->vlan_primary_bitmap);
	}
	if (out_vlan_secondary_bitmap)
	{
		*out_vlan_secondary_bitmap = ntohl(vlan_config_elem->vlan_secondary_bitmap);
	}
	if (out_vlan_user_2_bitmap)
	{
		*out_vlan_user_2_bitmap = ntohl(vlan_config_elem->vlan_user_2_bitmap);
	}
	if (out_vlan_user_3_bitmap)
	{
		*out_vlan_user_3_bitmap = ntohl(vlan_config_elem->vlan_user_3_bitmap);
	}

	return AUD_SUCCESS;
}

aud_error_t
ddp_read_device_vlan_config_response_name_string
(
	const ddp_message_read_info_t * message_info,
	uint16_t vlan_idx,
	char ** out_domain
)
{
	aud_error_t result;
	ddp_device_vlan_config_response_t *header;
	ddp_heap_read_info_t heap_info;
	ddp_vlan_name_offsets_t *offsets;

	if (!message_info)
	{
		return AUD_ERR_INVALIDPARAMETER;
	}

	header = (ddp_device_vlan_config_response_t *)message_info->header.header;

	if (vlan_idx > header->payload.vlan_config_number)
	{
		return AUD_ERR_INVALIDPARAMETER;
	}

	if (out_domain)
	{
		result = ddp_read_heap_arraystruct(message_info,
			&heap_info,
			ntohs(header->payload.vlan_config_name_string_offset),
			header->payload.vlan_config_number,
			sizeof(ddp_vlan_name_offsets_t));

		if (result != AUD_SUCCESS)
		{
			return result;
		}

		offsets = ((ddp_vlan_name_offsets_t*)heap_info._.array16) + vlan_idx;
		if (offsets->offset)
		{
			*out_domain = ddp_heap_read_string(message_info, ntohs(offsets->offset));
		}
		else
		{
			*out_domain = NULL;
		}

	}

	return  AUD_SUCCESS;
}

aud_error_t
ddp_read_device_dante_domain_response
(
	const ddp_packet_read_info_t * packet_info,
	uint16_t offset,
	ddp_request_id_t * out_request_id,
	ddp_status_t * out_status,
	uint8_t * out_dante_domain_enrollment,
	uint8_t * out_dante_domain_manager_connection,
	uint8_t * out_ddm_admin_local_access_control_policy,
	uint8_t * out_ddm_admin_remote_access_control_policy
)
{
	aud_error_t result;
	ddp_message_read_info_t message_info;
	ddp_device_dante_domain_response_t * header;

	if (!packet_info || offset >= packet_info->packet_length_bytes)
	{
		return AUD_ERR_INVALIDPARAMETER;
	}
	result = ddp_packet_read_message(packet_info, &message_info, offset);
	if (result != AUD_SUCCESS)
	{
		return result;
	}
	result = ddp_packet_validate_message(&message_info, DDP_OP_DEVICE_DANTE_DOMAIN, sizeof(ddp_device_dante_domain_response_t));
	if (result != AUD_SUCCESS)
	{
		return result;
	}
	header = (ddp_device_dante_domain_response_t *)message_info.header.header;
	if (out_request_id)
	{
		*out_request_id = ntohs(header->subheader.request_id);
	}
	if (out_status)
	{
		*out_status = ntohs(header->subheader.status);
	}

	//get the flags
	uint32_t valid_flags = ntohl(header->payload.valid_flags);
	uint16_t domain_flags_valid = ntohs(header->payload.dante_domain_flags_valid_mask);
	uint16_t domain_flags = ntohs(header->payload.dante_domain_flags);

	//get the enrollment state
	if(out_dante_domain_enrollment)
	{
		if( (domain_flags_valid & DEVICE_DANTE_DOMAIN_IS_ENROLLED) &&
			(domain_flags & DEVICE_DANTE_DOMAIN_IS_ENROLLED) )
		{
			*out_dante_domain_enrollment = DDP_DANTE_DOMAIN_ENROLLMENT_ENROLLED;
		}
		else if( (domain_flags_valid & DEVICE_DANTE_DOMAIN_IS_ENROLLED) &&
				!(domain_flags & DEVICE_DANTE_DOMAIN_IS_ENROLLED) )
		{
			*out_dante_domain_enrollment = DDP_DANTE_DOMAIN_ENROLLMENT_UNENROLLED;
		}
		else
		{
			*out_dante_domain_enrollment = DDP_DANTE_DOMAIN_ENROLLMENT_UNKNOWN;
		}
	}

	//get the DDM connection state
	if(out_dante_domain_manager_connection)
	{
		if( (domain_flags_valid & DEVICE_DANTE_DOMAIN_IS_DDM_CONNECTED) &&
			(domain_flags & DEVICE_DANTE_DOMAIN_IS_DDM_CONNECTED) )
		{
			*out_dante_domain_manager_connection = DDP_DDM_MANAGER_CONNECTION_CONNECTED;
		}
		else if( (domain_flags_valid & DEVICE_DANTE_DOMAIN_IS_DDM_CONNECTED) &&
				!(domain_flags & DEVICE_DANTE_DOMAIN_IS_DDM_CONNECTED) )
		{
			*out_dante_domain_manager_connection = DDP_DDM_MANAGER_CONNECTION_NOTCONNECTED;
		}
		else
		{
			*out_dante_domain_manager_connection = DDP_DDM_MANAGER_CONNECTION_UNKNOWN;
		}
	}

	//get the local policy
	if(out_ddm_admin_local_access_control_policy)
	{
		if(valid_flags & DEVICE_DANTE_DOMAIN_RESPONSE_INDEX_LOCAL_ACCESS_CONTROL_VALID)
		{
			*out_ddm_admin_local_access_control_policy = header->payload.local_access_control_policy;
		}
		else
		{
			*out_ddm_admin_local_access_control_policy = DEVICE_DANTE_DOMAIN_LOCAL_ACCESS_CONTROL_POLICY_UNKNOWN_INVALID;
		}
	}

	//get the remote policy
	if(out_ddm_admin_remote_access_control_policy)
	{
		if(valid_flags & DEVICE_DANTE_DOMAIN_RESPONSE_INDEX_REMOTE_ACCESS_CONTROL_VALID)
		{
			*out_ddm_admin_remote_access_control_policy = header->payload.remote_access_control_policy;
		}
		else
		{
			*out_ddm_admin_remote_access_control_policy = DEVICE_DANTE_DOMAIN_REMOTE_ACCESS_CONTROL_POLICY_UNKNOWN_INVALID;
		}
	}
	return AUD_SUCCESS;
}

#if defined(AUD_PLATFORM_ULTIMO)

aud_error_t
ddp_read_device_switch_status_response_header
(
	const ddp_packet_read_info_t * packet_info,
	uint16_t offset,
	ddp_message_read_info_t * out_message_info,
	ddp_request_id_t * out_request_id,
	ddp_status_t * out_status,
	uint16_t * out_enabled_ports_values,
	uint8_t * out_num_ports
)
{
	aud_error_t result;
	ddp_device_switch_status_response_t * header;

	if (!packet_info || offset >= packet_info->packet_length_bytes)
	{
		return AUD_ERR_INVALIDPARAMETER;
	}
	result = ddp_packet_read_message(packet_info, out_message_info, offset);
	if (result != AUD_SUCCESS)
	{
		return result;
	}
	result = ddp_packet_validate_message(out_message_info, DDP_OP_DEVICE_SWITCH_STATUS, sizeof(ddp_device_switch_status_response_t));
	if (result != AUD_SUCCESS)
	{
		return result;
	}
	header = (ddp_device_switch_status_response_t *)out_message_info->header.header;

	if (out_request_id)
	{
		*out_request_id = ntohs(header->subheader.request_id);
	}
	if (out_status)
	{
		*out_status = ntohs(header->subheader.status);
	}
	if (out_enabled_ports_values)
	{
		*out_enabled_ports_values = ntohs(header->payload.enabled_ports_values);
	}
	if (out_num_ports)
	{
		*out_num_ports = header->payload.num_ports;
	}

	return AUD_SUCCESS;
}

aud_error_t
ddp_read_device_switch_status_response_port
(
	const ddp_message_read_info_t * message_info,
	uint8_t port_idx,
	ddp_switch_port_status_t ** out_port_status
)
{
	aud_error_t result;
	ddp_device_switch_status_response_t * header;
	ddp_heap_read_info_t heap_info;

	if (!message_info)
	{
		return AUD_ERR_INVALIDPARAMETER;
	}

	header = (ddp_device_switch_status_response_t *)message_info->header.header;

	if (port_idx >= header->payload.num_ports)
	{
		return AUD_ERR_INVALIDPARAMETER;
	}

	result = ddp_read_heap_arraystruct(message_info,
		&heap_info,
		ntohs(header->payload.port_offset),
		header->payload.num_ports,
		header->payload.port_array_element_size);

	if (result != AUD_SUCCESS)
	{
		return result;
	}

	*out_port_status = ((ddp_switch_port_status_t *)heap_info._.array32) + port_idx;

	(*out_port_status)->valid_flags = ntohs((*out_port_status)->valid_flags);
	(*out_port_status)->link_speed = ntohs((*out_port_status)->link_speed);
	(*out_port_status)->error_count = ntohl((*out_port_status)->error_count);

	return AUD_SUCCESS;
}

#endif // #if defined(AUD_PLATFORM_ULTIMO)


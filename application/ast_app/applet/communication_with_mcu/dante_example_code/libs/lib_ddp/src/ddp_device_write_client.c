/*
* File     : ddp_device_write_client.c
* Created  : August 2014
* Updated  : Date: 2014/08/22
* Author   : Michael Ung <michael.ung@audinate.com>
* Synopsis : All device DDP client write helper functions.
*
* Copyright 2005-2017, Audinate Pty Ltd and/or its licensors
Confidential
* Audinate Copyright Header Version 1
*/

#include "ddp/device_client.h"
#include "ddp/util.h"


aud_error_t
ddp_add_device_general_request
(
	ddp_packet_write_info_t * packet_info,
	ddp_request_id_t request_id
)
{
	aud_error_t result;
	ddp_message_write_info_t message_info;

	if (!packet_info)
	{
		return AUD_ERR_INVALIDPARAMETER;
	}

	result = ddp_packet_allocate_request(packet_info, &message_info,
		DDP_OP_DEVICE_GENERAL, request_id,
		0);
	if (result != AUD_SUCCESS)
	{
		return result;
	}

	return AUD_SUCCESS;
}

aud_error_t
ddp_add_device_manufacturer_request
(
	ddp_packet_write_info_t * packet_info,
	ddp_request_id_t request_id,
	ddp_manf_override_values_t * override_values
)
{
	aud_error_t result;
	ddp_message_write_info_t message_info;
	ddp_device_manf_request_t * raw_header;
	uint32_t valid_flags = 0;

	if (!packet_info)
	{
		return AUD_ERR_INVALIDPARAMETER;
	}

	result = ddp_packet_allocate_request(packet_info, &message_info,
		DDP_OP_DEVICE_MANF, request_id,
		sizeof(raw_header->payload));
	if (result != AUD_SUCCESS)
	{
		return result;
	}

	raw_header = (ddp_device_manf_request_t *)message_info._.buf32;

	if (override_values)
	{
		if (override_values->manf_name_string)
		{
			if (strlen(override_values->manf_name_string) > 31)
			{
				return AUD_ERR_INVALIDPARAMETER;
			}

			if (ddp_message_allocate_string
					(&message_info, &raw_header->payload.manf_name_string_offset, override_values->manf_name_string) == NULL)

			{
				return AUD_ERR_NOMEMORY;
			}

			valid_flags |= MANF_OVERRIDE_MANF_NAME_STRING_VALID;
		}
		else
		{
			raw_header->payload.manf_name_string_offset = 0;
		}

		if (override_values->model_id_string)
		{
			if (strlen(override_values->model_id_string) > 31)
			{
				return AUD_ERR_INVALIDPARAMETER;
			}

			if (ddp_message_allocate_string
					(&message_info, &raw_header->payload.model_id_string_offset, override_values->model_id_string) == NULL)

			{
				return AUD_ERR_NOMEMORY;
			}

			valid_flags |= MANF_OVERRIDE_MODEL_ID_STRING_VALID;
		}
		if (override_values->software_version)
		{
			raw_header->payload.software_version = htonl(override_values->software_version);
			valid_flags |= MANF_OVERRIDE_SOFTWARE_VER_VALID;
		}
		if (override_values->software_build)
		{
			raw_header->payload.software_build = htonl(override_values->software_build);
			valid_flags |= MANF_OVERRIDE_SOFTWARE_BUILD_VALID;
		}
		if (override_values->firmware_version)
		{
			raw_header->payload.firmware_version = htonl(override_values->firmware_version);
			valid_flags |= MANF_OVERRIDE_FIRMWARE_VER_VALID;
		}
		if (override_values->firmware_build)
		{
			raw_header->payload.firmware_build = htonl(override_values->firmware_build);
			valid_flags |= MANF_OVERRIDE_FIMRWARE_BUILD_VALID;
		}
		if (override_values->capability_flags)
		{
			raw_header->payload.capability_flags = htonl(override_values->capability_flags);
			valid_flags |= MANF_OVERRIDE_CAPABILITY_FLAGS_VALID;
		}
		if (override_values->model_version)
		{
			raw_header->payload.model_version = htonl(override_values->model_version);
			valid_flags |= MANF_OVERRIDE_MODEL_VER_VALID;
		}
		if (override_values->model_version_string)
		{
			if (strlen(override_values->model_version_string) > 31)
			{
				return AUD_ERR_INVALIDPARAMETER;
			}

			if (ddp_message_allocate_string
					(&message_info, &raw_header->payload.model_version_string_offset, override_values->model_version_string) == NULL)

			{
				return AUD_ERR_NOMEMORY;
			}

			valid_flags |= MANF_OVERRIDE_MODEL_VER_STRING_VALID;
		}

		raw_header->payload.override_valid_flags = htonl(valid_flags);
	}
	else
	{
		raw_header->payload.override_valid_flags = 0;
	}

	return AUD_SUCCESS;
}

aud_error_t
ddp_add_device_upgrade_tftp_request
(
	ddp_packet_write_info_t * packet_info,
	ddp_request_id_t request_id,
	uint16_t upgrade_action_flags,
	uint32_t upgrade_flags,
	uint32_t upgrade_flags_mask,
	dante_id64_t* manf_override,
	dante_id64_t* model_override,
	ddp_upgrade_tftp_t* tftp_params
)
{
	aud_error_t result;
	ddp_message_write_info_t message_info;
	ddp_device_upgrade_request_t * raw_header;
	uint16_t valid_flags = 0;

	if (!packet_info)
	{
		return AUD_ERR_INVALIDPARAMETER;
	}

	result = ddp_packet_allocate_request(packet_info, &message_info,
			DDP_OP_DEVICE_UPGRADE, request_id, sizeof(raw_header->payload));
	if (result != AUD_SUCCESS)
	{
		return result;
	}
	raw_header = (ddp_device_upgrade_request_t *) message_info._.buf32;

	raw_header->payload.upgrade_action_flags = htons(upgrade_action_flags);
	raw_header->payload.upgrade_flags = htonl(upgrade_flags);
	raw_header->payload.upgrade_flags_mask = htonl(upgrade_flags_mask);
	raw_header->payload.protocol_type = htons(UPGRADE_PROTOCOL_TFTP_GET);

	// set local valid flags for mandatory options, otherwise the message is sent as a request to query the device state
	if (upgrade_action_flags) {
		valid_flags = (UPGRADE_REQUEST_ACTIONS_VALID | UPGRADE_REQUEST_FLAGS_VALID | UPGRADE_REQUEST_PROTOCOL_TYPE_VALID);
	}

	if(manf_override)
	{
		memcpy(&raw_header->payload.manf_override, manf_override, sizeof(dante_id64_t));
		valid_flags |= UPGRADE_REQUEST_MANF_OVERRIDE_VALID;
	}
	if (model_override)
	{
		memcpy(&raw_header->payload.model_override, model_override, sizeof(dante_id64_t));
		valid_flags |= UPGRADE_REQUEST_MODEL_OVERRIDE_VALID;
	}

	//allocate the tftp protocol params structure on the heap
	if (tftp_params)
	{
		int16_t tftp_valid_flags = 0;
		ddp_device_upgrade_tftp_protocol_params_t* tftp_prot_params =
			(ddp_device_upgrade_tftp_protocol_params_t*)
			ddp_message_allocate_on_heap(
			&message_info,
			&raw_header->payload.prot_params_offset,
			sizeof(*tftp_prot_params)
			);
		if (!tftp_prot_params)
			return AUD_ERR_NOMEMORY;

		valid_flags |= UPGRADE_REQUEST_PROTOCOL_PARAM_OFFSET_VALID;

		//populate the fields in the tftp protocol parameters structure
		if (tftp_params->file_length != 0)
		{
			tftp_prot_params->file_length = htonl(tftp_params->file_length);
			tftp_valid_flags |= UPGRADE_TFTP_PROT_PARAM_FILE_LEN_VALID;
		}
		tftp_prot_params->family = htons(tftp_params->family);
		tftp_prot_params->port = htons(tftp_params->port);
		tftp_prot_params->ip_address = tftp_params->ip_address_nw.ip_addr;
		tftp_valid_flags |= (UPGRADE_TFTP_PROT_PARAM_FAMILY_VALID | UPGRADE_TFTP_PROT_PARAM_PORT_VALID | UPGRADE_TFTP_PROT_PARAM_IPADDR_VALID);

		if (tftp_params->filename)
		{
			if (!ddp_message_allocate_string(&message_info, &tftp_prot_params->filename_offset, tftp_params->filename))
			{
				return AUD_ERR_NOMEMORY;
			}
			tftp_valid_flags |= UPGRADE_TFTP_PROT_PARAM_FILENAME_VALID;
		}

		//store the tftp valid flags
		tftp_prot_params->valid_flags = htons(tftp_valid_flags);
	}
	else
	{
		raw_header->payload.prot_params_offset = 0;
	}

	//store the valid flags
	raw_header->payload.valid_flags = htons(valid_flags);

	return AUD_SUCCESS;
}

#if defined(AUD_PLATFORM_ULTIMO)

aud_error_t
ddp_add_device_upgrade_xmodem_request
(
	ddp_packet_write_info_t * packet_info,
	ddp_request_id_t request_id,
	uint16_t upgrade_action_flags,
	uint32_t upgrade_flags,
	uint32_t upgrade_flags_mask,
	dante_id64_t* manf_override,
	dante_id64_t* model_override,
	ddp_upgrade_xmodem_t* xmodem_params
)
{
	aud_error_t result;
	ddp_message_write_info_t message_info;
	ddp_device_upgrade_request_t * raw_header;
	uint16_t valid_flags;
	ddp_device_upgrade_xmodem_protocol_params_t* xmodem_prot_params;
	uint16_t xmodem_valid_flags = 0;

	if (!packet_info)
	{
		return AUD_ERR_INVALIDPARAMETER;
	}

	result = ddp_packet_allocate_request(packet_info, &message_info,
		DDP_OP_DEVICE_UPGRADE, request_id, sizeof(raw_header->payload));
	if (result != AUD_SUCCESS)
	{
		return result;
	}
	raw_header = (ddp_device_upgrade_request_t *)message_info._.buf32;

	raw_header->payload.upgrade_action_flags = htons(upgrade_action_flags);
	raw_header->payload.upgrade_flags = htonl(upgrade_flags);
	raw_header->payload.upgrade_flags_mask = htonl(upgrade_flags_mask);

	//set local valid flags for mandatory options
	valid_flags = (UPGRADE_REQUEST_ACTIONS_VALID | UPGRADE_REQUEST_FLAGS_VALID | UPGRADE_REQUEST_PROTOCOL_TYPE_VALID);

	if (manf_override)
	{
		memcpy(&raw_header->payload.manf_override, manf_override, sizeof(dante_id64_t));
		valid_flags |= UPGRADE_REQUEST_MANF_OVERRIDE_VALID;
	}
	if (model_override)
	{
		memcpy(&raw_header->payload.model_override, model_override, sizeof(dante_id64_t));
		valid_flags |= UPGRADE_REQUEST_MODEL_OVERRIDE_VALID;
	}

	raw_header->payload.protocol_type = htons(UPGRADE_PROTOCOL_XMODEM);

	if (xmodem_params)
	{
		//allocate the xmodem protocol params structure on the heap
		xmodem_prot_params =
			(ddp_device_upgrade_xmodem_protocol_params_t*)
			ddp_message_allocate_on_heap(
			&message_info, &raw_header->payload.prot_params_offset, sizeof(*xmodem_prot_params)
			);
		valid_flags |= UPGRADE_REQUEST_PROTOCOL_PARAM_OFFSET_VALID;

		//store the valid flags
		raw_header->payload.valid_flags = htons(valid_flags);

		//populate the fields in the xmodem protocol parameters structure
		if (xmodem_params->file_length != 0)
		{
			xmodem_prot_params->file_length = htonl(xmodem_params->file_length);
			xmodem_valid_flags |= UPGRADE_XMODEM_PROT_PARAM_FILE_LEN_VALID;
		}
		else
		{
			//file length needs to be specified for XMODEM
			return AUD_ERR_INVALIDPARAMETER;
		}

		xmodem_prot_params->port = htons(xmodem_params->port);
		xmodem_valid_flags |= UPGRADE_XMODEM_PROT_PARAM_PORT_VALID;

		if (xmodem_params->port == UPGRADE_XMODEM_PORT_SPI0)
		{
			if (ddp_verify_spi_baud_rate(xmodem_params->baud_rate) != AUD_FALSE) {
				xmodem_prot_params->baudrate = htonl(xmodem_params->baud_rate);
				xmodem_valid_flags |= UPGRADE_XMODEM_PROT_PARAM_BAUDRATE_VALID;
			}
			else {
				return AUD_ERR_INVALIDPARAMETER;
			}

			if ((xmodem_params->s_params.spi[DDP_XMODEM_SPI_DEV_MASTER].polarity >= DDP_SPI_CPOL_IDLE_LOW) &&
				(xmodem_params->s_params.spi[DDP_XMODEM_SPI_DEV_MASTER].polarity <= DDP_SPI_CPOL_IDLE_HIGH)) {
				xmodem_prot_params->master_cpol = (uint8_t)xmodem_params->s_params.spi[DDP_XMODEM_SPI_DEV_MASTER].polarity;
				xmodem_valid_flags |= UPGRADE_XMODEM_PROT_PARAM_MASTER_CPOL_VALID;
			}
			else {
				return AUD_ERR_INVALIDPARAMETER;
			}

			if ((xmodem_params->s_params.spi[DDP_XMODEM_SPI_DEV_MASTER].phase >= DDP_SPI_CPHA_SAMPLE_LEADING_EDGE) &&
				(xmodem_params->s_params.spi[DDP_XMODEM_SPI_DEV_MASTER].phase <= DDP_SPI_CPHA_SAMPLE_TRAILING_EDGE)) {
				xmodem_prot_params->master_cpha = (uint8_t)xmodem_params->s_params.spi[DDP_XMODEM_SPI_DEV_MASTER].phase;
				xmodem_valid_flags |= UPGRADE_XMODEM_PROT_PARAM_MASTER_CPHA_VALID;
			}
			else {
				return AUD_ERR_INVALIDPARAMETER;
			}

			if ((xmodem_params->s_params.spi[DDP_XMODEM_SPI_DEV_SLAVE].polarity >= DDP_SPI_CPOL_IDLE_LOW) &&
				(xmodem_params->s_params.spi[DDP_XMODEM_SPI_DEV_SLAVE].polarity <= DDP_SPI_CPOL_IDLE_HIGH)) {
				xmodem_prot_params->slave_cpol = (uint8_t)xmodem_params->s_params.spi[DDP_XMODEM_SPI_DEV_SLAVE].polarity;
				xmodem_valid_flags |= UPGRADE_XMODEM_PROT_PARAM_SLAVE_CPOL_VALID;
			}
			else {
				return AUD_ERR_INVALIDPARAMETER;
			}

			if ((xmodem_params->s_params.spi[DDP_XMODEM_SPI_DEV_SLAVE].phase >= DDP_SPI_CPHA_SAMPLE_LEADING_EDGE) &&
				(xmodem_params->s_params.spi[DDP_XMODEM_SPI_DEV_SLAVE].phase <= DDP_SPI_CPHA_SAMPLE_TRAILING_EDGE)) {
				xmodem_prot_params->slave_cpha = (uint8_t)xmodem_params->s_params.spi[DDP_XMODEM_SPI_DEV_SLAVE].phase;
				xmodem_valid_flags |= UPGRADE_XMODEM_PROT_PARAM_SLAVE_CPHA_VALID;
			}
			else {
				return AUD_ERR_INVALIDPARAMETER;
			}
		}
		else if ((xmodem_params->port == UPGRADE_XMODEM_PORT_UARTA) || (xmodem_params->port == UPGRADE_XMODEM_PORT_UARTB))
		{
			if (ddp_verify_uart_baud_rate(xmodem_params->baud_rate) != AUD_FALSE) {
				xmodem_prot_params->baudrate = htonl(xmodem_params->baud_rate);
				xmodem_valid_flags |= UPGRADE_XMODEM_PROT_PARAM_BAUDRATE_VALID;
			}
			else {
				return AUD_ERR_INVALIDPARAMETER;
			}

			if ((xmodem_params->s_params.uart.flow_control >= DDP_UART_FLOWCONTROL_NONE) &&
				(xmodem_params->s_params.uart.flow_control <= DDP_UART_FLOWCONTROL_RTS_CTS)) {
				xmodem_prot_params->flow_control = (uint8_t)xmodem_params->s_params.uart.flow_control;
				xmodem_valid_flags |= UPGRADE_XMODEM_PROT_PARAM_FLOW_CONTROL_VALID;
			}
			else {
				return AUD_ERR_INVALIDPARAMETER;
			}

			if ((xmodem_params->s_params.uart.parity >= DDP_UART_PARITY_NONE) &&
				(xmodem_params->s_params.uart.parity <= DDP_UART_PARITY_ODD)) {
				xmodem_prot_params->parity = (uint8_t)xmodem_params->s_params.uart.parity;
				xmodem_valid_flags |= UPGRADE_XMODEM_PROT_PARAM_PARITY_VALID;
			}
			else {
				return AUD_ERR_INVALIDPARAMETER;
			}

		}

		xmodem_prot_params->pad0 = 0;

		//store the xmodem valid flags
		xmodem_prot_params->valid_flags = htons(xmodem_valid_flags);
	}
	else
	{
		raw_header->payload.prot_params_offset = 0;
	}

	return AUD_SUCCESS;
}

#endif // defined(AUD_PLATFORM_ULTIMO)

aud_error_t
ddp_add_device_erase_request
(
	ddp_packet_write_info_t * packet_info,
	ddp_request_id_t request_id,
	uint16_t mode,
	uint16_t mode_mask
)
{
	aud_error_t result;
	ddp_message_write_info_t message_info;
	ddp_device_erase_request_t * raw_header;

	if (!packet_info)
	{
		return AUD_ERR_INVALIDPARAMETER;
	}

	result = ddp_packet_allocate_request(packet_info, &message_info,
			DDP_OP_DEVICE_ERASE_CONFIG, request_id,
		sizeof(raw_header->payload));
	if (result != AUD_SUCCESS)
	{
		return result;
	}
	raw_header = (ddp_device_erase_request_t *) message_info._.buf32;

	raw_header->payload.mode = htons(mode);
	raw_header->payload.mode_mask = htons(mode_mask);

	return AUD_SUCCESS;
}

aud_error_t
ddp_add_device_reboot_request
(
	ddp_packet_write_info_t * packet_info,
	ddp_request_id_t request_id,
	uint16_t mode,
	uint16_t mode_mask
)
{
	aud_error_t result;
	ddp_message_write_info_t message_info;
	ddp_device_reboot_request_t * raw_header;

	if (!packet_info)
	{
		return AUD_ERR_INVALIDPARAMETER;
	}

	result = ddp_packet_allocate_request(packet_info, &message_info,
			DDP_OP_DEVICE_REBOOT, request_id,
		sizeof(raw_header->payload));
	if (result != AUD_SUCCESS)
	{
		return result;
	}
	raw_header = (ddp_device_reboot_request_t *) message_info._.buf32;

	raw_header->payload.mode = htons(mode);
	raw_header->payload.mode_mask = htons(mode_mask);

	return AUD_SUCCESS;
}

aud_error_t
ddp_add_device_identity_request
(
	ddp_packet_write_info_t * packet_info,
	ddp_request_id_t request_id,
	const char * name_change_string
)
{
	aud_error_t result;
	ddp_message_write_info_t message_info;
	ddp_device_identity_request_t * raw_header;

	if (!packet_info)
	{
		return AUD_ERR_INVALIDPARAMETER;
	}

	result = ddp_packet_allocate_request(packet_info, &message_info,
										   DDP_OP_DEVICE_IDENTITY, request_id,
										   sizeof(raw_header->payload));
	if (result != AUD_SUCCESS)
	{
		return result;
	}
	raw_header = (ddp_device_identity_request_t *)message_info._.buf32;

	if (name_change_string)
	{
		if (ddp_message_allocate_string(&message_info, &raw_header->payload.name_change_string_offset, name_change_string) == NULL)
		{
			return AUD_ERR_NOMEMORY;
		}
		raw_header->payload.valid_flags = htons(IDENTITY_REQUEST_NAME_CHANGE_VALID);
	}
	else
	{
		raw_header->payload.valid_flags = 0;
		raw_header->payload.name_change_string_offset = 0;
	}

	return AUD_SUCCESS;
}

aud_error_t
ddp_add_device_gpio_request
(
	ddp_packet_write_info_t * packet_info,
	ddp_request_id_t request_id,
	uint32_t output_state_valid_flags,
	uint32_t output_state_values
)
{
	aud_error_t result;
	ddp_message_write_info_t message_info;
	ddp_device_gpio_request_t * raw_header;

	if (!packet_info)
	{
		return AUD_ERR_INVALIDPARAMETER;
	}

	result = ddp_packet_allocate_request(packet_info, &message_info,
											DDP_OP_DEVICE_GPIO, request_id,
										   sizeof(raw_header->payload));
	if (result != AUD_SUCCESS)
	{
		return result;
	}
	raw_header = (ddp_device_gpio_request_t *)message_info._.buf32;

	raw_header->payload.output_state_valid_flags = htonl(output_state_valid_flags);
	raw_header->payload.output_state_values = htonl(output_state_values);

	return AUD_SUCCESS;
}

#if defined(AUD_PLATFORM_ULTIMO)
aud_error_t
ddp_add_device_switch_led_request
(
	ddp_packet_write_info_t * packet_info,
	ddp_request_id_t request_id,
	uint8_t switch_led_mode
)
{
	aud_error_t result;
	ddp_message_write_info_t message_info;
	ddp_device_switch_led_request_t * raw_header;

	if (!packet_info)
	{
		return AUD_ERR_INVALIDPARAMETER;
	}

	result = ddp_packet_allocate_request(packet_info, &message_info,
											DDP_OP_DEVICE_SWITCH_LED, request_id,
										   sizeof(raw_header->payload));
	if (result != AUD_SUCCESS)
	{
		return result;
	}
	raw_header = (ddp_device_switch_led_request_t *)message_info._.buf32;

	switch(switch_led_mode)
	{
		case SWITCH_LED_MODE_NONE:
			raw_header->payload.valid_flags = 0;
			raw_header->payload.switch_led_mode = switch_led_mode;
			break;

		case SWITCH_LED_MODE_FORCE_OFF:
		case SWITCH_LED_MODE_FORCE_ON:
		case SWITCH_LED_MODE_NORMAL:
		case SWITCH_LED_MODE_LINK_ONLY:
			raw_header->payload.valid_flags = htons(SWITCH_LED_REQUEST_SWITCH_LED_MODE_VALID);
			raw_header->payload.switch_led_mode = switch_led_mode;
			break;
		default:
			return AUD_ERR_INVALIDPARAMETER;
	}

	return AUD_SUCCESS;
}

#endif // defined(AUD_PLATFORM_ULTIMO)

aud_error_t
ddp_add_device_aes67_request
(
	ddp_packet_write_info_t * packet_info,
	ddp_request_id_t request_id,
	uint16_t valid_flag,
	uint8_t aes67_enabled
)
{
	aud_error_t result;
	ddp_message_write_info_t message_info;
	ddp_device_aes67_request_t * raw_header;

	if (!packet_info)
	{
		return AUD_ERR_INVALIDPARAMETER;
	}

	result = ddp_packet_allocate_request(packet_info, &message_info,
				DDP_OP_DEVICE_AES67, request_id, sizeof(raw_header->payload));
	if (result != AUD_SUCCESS)
	{
		return result;
	}
	raw_header = (ddp_device_aes67_request_t *)message_info._.buf32;

	if(valid_flag)
	{
		raw_header->payload.valid_flags = htons(valid_flag);
		raw_header->payload.aes67_mode = aes67_enabled;
		raw_header->payload.pad0 = 0;
	}
	else
	{
		raw_header->payload.valid_flags = 0;
		raw_header->payload.aes67_mode = 0;
		raw_header->payload.pad0 = 0;
	}

	return AUD_SUCCESS;
}

aud_error_t
ddp_add_device_lock_unlock_request
(
	ddp_packet_write_info_t * packet_info,
	ddp_request_id_t request_id
)
{
	aud_error_t result;
	ddp_message_write_info_t message_info;

	if (!packet_info)
	{
		return AUD_ERR_INVALIDPARAMETER;
	}

	result = ddp_packet_allocate_request(packet_info, &message_info,
			DDP_OP_DEVICE_LOCK_UNLOCK, request_id, 0);
	if (result != AUD_SUCCESS)
	{
		return result;
	}

	return AUD_SUCCESS;
}

#if defined(AUD_PLATFORM_BROOKLYN_II)

aud_error_t
ddp_add_device_switch_redundancy_request
(
	ddp_packet_write_info_t * packet_info,
	ddp_request_id_t request_id,
	uint16_t valid_flag,
	uint8_t switch_redundancy_mode
)
{
	aud_error_t result;
	ddp_message_write_info_t message_info;
	ddp_device_switch_redundancy_request_t * raw_header;

	if (!packet_info)
	{
		return AUD_ERR_INVALIDPARAMETER;
	}

	result = ddp_packet_allocate_request(packet_info, &message_info,
			DDP_OP_DEVICE_SWITCH_REDUNDANCY, request_id, sizeof(raw_header->payload));
	if (result != AUD_SUCCESS)
	{
		return result;
	}
	raw_header = (ddp_device_switch_redundancy_request_t *)message_info._.buf32;

	if(valid_flag)
	{
		raw_header->payload.valid_flags = htons(valid_flag);
		raw_header->payload.switch_redundancy_mode = switch_redundancy_mode;
		raw_header->payload.pad0 = 0;
	}
	else
	{
		raw_header->payload.valid_flags = 0;
		raw_header->payload.switch_redundancy_mode = 0;
		raw_header->payload.pad0 = 0;
	}

	return AUD_SUCCESS;
}

aud_error_t
ddp_add_device_uart_config_request
(
	ddp_packet_write_info_t * packet_info,
	ddp_request_id_t request_id,
	uint16_t valid_flag,
	uint8_t uart_index_number,
	uint8_t uart_bits,
	uint8_t uart_parity,
	uint8_t uart_stop_bits,
	uint32_t uart_speed
)
{
	aud_error_t result;
	ddp_message_write_info_t message_info;
	ddp_device_uart_config_request_t * raw_header;

	if (!packet_info)
	{
		return AUD_ERR_INVALIDPARAMETER;
	}

	result = ddp_packet_allocate_request(packet_info, &message_info,
			DDP_OP_DEVICE_UART_CONFIG, request_id, sizeof(raw_header->payload));
	if (result != AUD_SUCCESS)
	{
		return result;
	}
	raw_header = (ddp_device_uart_config_request_t *)message_info._.buf32;

	if(valid_flag)
	{
		raw_header->payload.valid_flags = htons(valid_flag);
		raw_header->payload.uart_index_number = uart_index_number;
		raw_header->payload.uart_bits = uart_bits;
		raw_header->payload.uart_parity = uart_parity;
		raw_header->payload.uart_stop_bits = uart_stop_bits;
		raw_header->payload.uart_speed = htonl(uart_speed);
	}
	else
	{
		raw_header->payload.valid_flags = 0;
		raw_header->payload.uart_index_number = 0;
		raw_header->payload.uart_bits = 0;
		raw_header->payload.uart_parity = 0;
		raw_header->payload.uart_stop_bits = 0;
		raw_header->payload.uart_speed = 0;
	}

	return AUD_SUCCESS;
}

aud_error_t
ddp_add_device_meter_config_request
(
	ddp_packet_write_info_t * packet_info,
	ddp_request_id_t request_id,
	uint16_t valid_flag,
	uint16_t meter_conig_rate
)
{
	aud_error_t result;
	ddp_message_write_info_t message_info;
	ddp_device_meter_config_request_t * raw_header;

	if (!packet_info)
	{
		return AUD_ERR_INVALIDPARAMETER;
	}

	result = ddp_packet_allocate_request(packet_info, &message_info,
			DDP_OP_DEVICE_METER_CONFIG, request_id, sizeof(raw_header->payload));
	if (result != AUD_SUCCESS)
	{
		return result;
	}
	raw_header = (ddp_device_meter_config_request_t *)message_info._.buf32;

	if(valid_flag)
	{
		raw_header->payload.valid_flags = htons(valid_flag);
		raw_header->payload.meter_config_rate = htons(meter_conig_rate);
	}
	else
	{
		raw_header->payload.valid_flags = 0;
		raw_header->payload.meter_config_rate = 0;
	}

	return AUD_SUCCESS;
}

#endif // defined(AUD_PLATFORM_BROOKLYN_II)

aud_error_t
ddp_add_device_vlan_config_request
(
	ddp_packet_write_info_t * packet_info,
	ddp_request_id_t request_id,
	uint16_t valid_flag,
	uint8_t vlan_config_id
)
{
	aud_error_t result;
	ddp_message_write_info_t message_info;
	ddp_device_vlan_config_request_t * raw_header;

	if (!packet_info)
	{
		return AUD_ERR_INVALIDPARAMETER;
	}

	result = ddp_packet_allocate_request(packet_info, &message_info,
		DDP_OP_DEVICE_VLAN_CONFIG, request_id, sizeof(raw_header->payload));
	if (result != AUD_SUCCESS)
	{
		return result;
	}
	raw_header = (ddp_device_vlan_config_request_t *)message_info._.buf32;

	if (valid_flag)
	{
		raw_header->payload.valid_flags = htons(valid_flag);
		raw_header->payload.vlan_config_id = vlan_config_id;
		raw_header->payload.pad0 = 0;
	}
	else
	{
		raw_header->payload.valid_flags = 0;
		raw_header->payload.vlan_config_id = 0;
		raw_header->payload.pad0 = 0;
	}

	return AUD_SUCCESS;
}

aud_error_t
ddp_add_device_dante_domain_request
(
	ddp_packet_write_info_t * packet_info,
	ddp_request_id_t request_id
)
{
	aud_error_t result;
	ddp_message_write_info_t message_info;

	if (!packet_info)
	{
		return AUD_ERR_INVALIDPARAMETER;
	}

	result = ddp_packet_allocate_request(packet_info, &message_info,
			DDP_OP_DEVICE_DANTE_DOMAIN, request_id, 0);
	if (result != AUD_SUCCESS)
	{
		return result;
	}

	return AUD_SUCCESS;
}

#if defined(AUD_PLATFORM_ULTIMO)

aud_error_t
ddp_add_device_switch_status_request
(
	ddp_packet_write_info_t * packet_info,
	ddp_request_id_t request_id
)
{
	aud_error_t result;
	ddp_message_write_info_t message_info;

	if (!packet_info)
	{
		return AUD_ERR_INVALIDPARAMETER;
	}

	result = ddp_packet_allocate_request(packet_info, &message_info,
		DDP_OP_DEVICE_SWITCH_STATUS, request_id,
		0);
	if (result != AUD_SUCCESS)
	{
		return result;
	}

	return AUD_SUCCESS;
}

#endif // defined(AUD_PLATFORM_ULTIMO)

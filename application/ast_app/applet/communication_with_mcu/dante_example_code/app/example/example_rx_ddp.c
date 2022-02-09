/*
* File     : example_rx_ddp.c
* Created  : October 2014
* Updated  : 2014/10/08
* Author   : Chamira Perera
* Synopsis : HostCPU Handling of received DDP responses and events and implementation of reading all DDP events
*
* This module handles all received valid DDP responses and events and it provides example code to read all DDP events
*
* Copyright 2005-2017, Audinate Pty Ltd and/or its licensors
* Confidential
* Audinate Copyright Header Version 1
*/


#include "aud_platform.h"
#include "example_rx_ddp.h"
#include "cobs.h"
#include "ddp/packet.h"
#include "ddp/audio_client.h"
#include "ddp/device_client.h"
#include "ddp/clocking_client.h"
#include "ddp/local_client.h"
#include "ddp/routing_client.h"
#include "ddp/network_client.h"
#include "ddp/util.h"
#include "dante/dante_common.h"
#include "ddp/clocking_util.h"
#include "../../../gb_commun_with_mcu.h"
extern int run_flag;
extern int dante_state;
#if (defined HOSTCPU_API_SUPPORT_DANTE_EVENT_AUDIO_FORMAT)
aud_error_t handle_dante_event_audio_format(ddp_packet_read_info_t *ddp_rinfo, uint16_t offset)
{
	aud_error_t result;
	dante_samplerate_t curr_srate, reboot_srate;
	ddp_event_timestamp_t *timestamp;

	result = ddp_read_local_audio_format(ddp_rinfo, offset, &timestamp, &curr_srate, &reboot_srate);
	if (result != AUD_SUCCESS)
	{
		return result;
	}

	AUD_PRINTF("Timestamp: %u:%u\n", timestamp->seconds, timestamp->subseconds);
	AUD_PRINTF("Current sample rate: %u\n", curr_srate);
	AUD_PRINTF("Reboot sample rate: %u\n", reboot_srate);
	
	AUD_PRINTF("CUSTOMER TODO: handle reconfiguration of the CODEC due to sample rate change here!!!!!\n");

	return AUD_SUCCESS;
}
#endif //HOSTCPU_API_SUPPORT_DANTE_EVENT_AUDIO_FORMAT

#if (defined HOSTCPU_API_SUPPORT_DANTE_EVENT_CLOCK_PULLUP)
aud_error_t handle_dante_event_clock_pullup(ddp_packet_read_info_t *ddp_rinfo, uint16_t offset)
{
	aud_error_t result;
	uint16_t curr_pullup, reboot_pullup;
	ddp_event_timestamp_t *timestamp;

	result = ddp_read_local_clock_pullup(ddp_rinfo, offset, &timestamp, &curr_pullup, &reboot_pullup);
	if (result != AUD_SUCCESS)
	{
		return result;
	}

	AUD_PRINTF("Timestamp: %u:%u\n", timestamp->seconds, timestamp->subseconds);
	AUD_PRINTF("Current pullup: %s\n", ddp_pullup_to_string(curr_pullup));
	AUD_PRINTF("Reboot pullup: %s\n", ddp_pullup_to_string(reboot_pullup));

	AUD_PRINTF("CUSTOMER TODO: handle reconfiguration of the CODEC due to pullup change here!!!!!\n");

	return AUD_SUCCESS;
}
#endif //HOSTCPU_API_SUPPORT_DANTE_EVENT_CLOCK_PULLUP



#if (defined HOSTCPU_API_SUPPORT_DDP_DEVICE_GENERAL)
aud_error_t handle_ddp_device_general(ddp_packet_read_info_t *ddp_rinfo, uint16_t offset)
{
	aud_error_t result;
	ddp_request_id_t request_id;
	ddp_status_t status;
	char *model_id_str;
	dante_id64_t *model_id;
	uint32_t sw_ver, sw_bld, fw_ver, fw_bld, btld_ver, btld_bld, api_ver, cap_flags, status_flags;
	char id64_t_string_buff[DANTE_ID64_DNSSD_BUF_LENGTH];

	result = ddp_read_device_general_response(ddp_rinfo, offset, &request_id, &status,
											&model_id, &model_id_str, &sw_ver, &sw_bld, &fw_ver, &fw_bld,
											&btld_ver, &btld_bld, &api_ver, &cap_flags, &status_flags);

	if (result != AUD_SUCCESS)
	{
		return result;
	}

	AUD_PRINTF("Model ID: 0x%s\n", dante_id64_to_dnssd_hex(model_id, &id64_t_string_buff[0]));
	if (model_id_str) {
		AUD_PRINTF("Model ID string: %s\n", model_id_str);
	}
	AUD_PRINTF("Software version: %d.%d.%d\n", sw_ver >> 24, (sw_ver >> 16) & 0xFF, sw_ver & 0xFFFF);
	AUD_PRINTF("Software build: %d\n", sw_bld);
	AUD_PRINTF("Firmware version: %d.%d.%d\n", fw_ver >> 24, (fw_ver >> 16) & 0xFF, fw_ver & 0xFFFF);
	AUD_PRINTF("Firmware build: %d\n", fw_bld);
	AUD_PRINTF("Bootloader version: %d\n", btld_ver);
	AUD_PRINTF("Bootloader build: %d\n", btld_bld);
	AUD_PRINTF("API version: %d\n", api_ver);
	AUD_PRINTF("Capability flags: 0x%08X\n", cap_flags);
	AUD_PRINTF("Status flags: 0x%08X\n", status_flags);

	return AUD_SUCCESS;
}
#endif //HOSTCPU_API_SUPPORT_DDP_DEVICE_GENERAL

#if (defined HOSTCPU_API_SUPPORT_DDP_DEVICE_MANF_INFO)
aud_error_t handle_ddp_device_manuf_info(ddp_packet_read_info_t *ddp_rinfo, uint16_t offset)
{
	aud_error_t result;
	ddp_request_id_t request_id;
	ddp_status_t status;
	char *manuf_str, *model_ver_str, *model_id_str;
	dante_id64_t *manuf_id, *model_id;
	char id64_t_string_buff[DANTE_ID64_DNSSD_BUF_LENGTH];
	uint32_t sw_ver, sw_bld, fw_ver, fw_bld, cap_flags, model_ver;	

	result = ddp_read_device_manufacturer_response(ddp_rinfo, offset, &request_id, &status,
												&manuf_id, &manuf_str, &model_id, &model_id_str, &sw_ver,
												&sw_bld, &fw_ver, &fw_bld, &cap_flags, &model_ver, &model_ver_str);
	if (result != AUD_SUCCESS)
	{
		return result;
	}

	AUD_PRINTF("Manufacturer ID: 0x%s\n", dante_id64_to_dnssd_hex(manuf_id, &id64_t_string_buff[0]));
	if (model_id_str) {
		AUD_PRINTF("Manufacturer ID string: %s\n", model_id_str);
	}
	AUD_PRINTF("Model ID: 0x%s\n", dante_id64_to_dnssd_hex(model_id, &id64_t_string_buff[0]));
	if (model_id_str) {
		AUD_PRINTF("Model ID string: %s\n", model_id_str);
	}
	AUD_PRINTF("Software version: %d.%d.%d\n", sw_ver >> 24, (sw_ver >> 16) & 0xFF, sw_ver & 0xFFFF);
	AUD_PRINTF("Software build: %d\n", sw_bld);
	AUD_PRINTF("Firmware version: %d.%d.%d\n", fw_ver >> 24, (fw_ver >> 16) & 0xFF, fw_ver & 0xFFFF);
	AUD_PRINTF("Firmware build: %d\n", fw_bld);
	AUD_PRINTF("Capability flags: 0x%08X\n", cap_flags);
	AUD_PRINTF("Model version: %d\n", model_ver);
	if (model_ver_str) {
		AUD_PRINTF("Model version string: %s\n", model_ver_str);
	}

	return AUD_SUCCESS;
}
#endif //HOSTCPU_API_SUPPORT_DDP_DEVICE_MANF_INFO

#if (defined HOSTCPU_API_SUPPORT_DDP_DEVICE_UPGRADE)
aud_error_t handle_ddp_device_upgrade(ddp_packet_read_info_t *ddp_rinfo, uint16_t offset)
{
	AUD_UNUSED(ddp_rinfo);
	AUD_UNUSED(offset);
	return AUD_SUCCESS;
}				
#endif //HOSTCPU_API_SUPPORT_DDP_DEVICE_UPGRADE

#if (defined HOSTCPU_API_SUPPORT_DDP_DEVICE_ERASE_CONFIG)
aud_error_t handle_ddp_device_erase_config(ddp_packet_read_info_t *ddp_rinfo, uint16_t offset)
{
	aud_error_t result;
	ddp_request_id_t request_id;
	uint16_t supported_flags, executed_flags;
	ddp_status_t status;

	result = ddp_read_device_erase_response(ddp_rinfo, offset, &request_id, &status, &supported_flags, &executed_flags);
	if (result != AUD_SUCCESS)
	{
		return result;
	}

	AUD_PRINTF("Supported flags: 0x%04X\n", supported_flags);
	AUD_PRINTF("Executed flags: 0x%04X\n", executed_flags);

	return AUD_SUCCESS;
}
#endif //HOSTCPU_API_SUPPORT_DDP_DEVICE_ERASE_CONFIG

#if (defined HOSTCPU_API_SUPPORT_DDP_DEVICE_REBOOT)
aud_error_t handle_ddp_device_reboot(ddp_packet_read_info_t *ddp_rinfo, uint16_t offset)
{
	aud_error_t result;
	ddp_request_id_t request_id;
	uint16_t supported_flags, executed_flags;
	ddp_status_t status;

	result = ddp_read_device_reboot_response(ddp_rinfo, offset, &request_id, &status, &supported_flags, &executed_flags);
	if (result != AUD_SUCCESS)
	{
		return result;
	}

	AUD_PRINTF("Supported flags: 0x%04X\n", supported_flags);
	AUD_PRINTF("Executed flags: 0x%04X\n", executed_flags);

	return AUD_SUCCESS;
}
#endif //HOSTCPU_API_SUPPORT_DDP_DEVICE_REBOOT

#if (defined HOSTCPU_API_SUPPORT_DDP_DEVICE_ID)
aud_error_t handle_ddp_device_id(ddp_packet_read_info_t *ddp_rinfo, uint16_t offset)
{
	aud_error_t result;
	ddp_request_id_t request_id;
	ddp_status_t status;
	uint16_t status_flags, process_id;	
	dante_device_id_t *device_id;
	dante_device_id_str_t device_id_string_buff;
	char system_buf[256] = {0};
	char *default_name, *friendly_name, *domain, *advertised_name;

	result = ddp_read_device_identity_response(ddp_rinfo, offset, &request_id, &status, &status_flags, &process_id, 
												&device_id, &default_name, &friendly_name, &domain, &advertised_name);
	if (result != AUD_SUCCESS)
	{
		return result;
	}

	AUD_PRINTF("Identity status flags: 0x%04X\n", status_flags);
	AUD_PRINTF("Process ID: %d\n", process_id);
	AUD_PRINTF("Dante device ID: 0x%s\n", dante_device_id_to_string(device_id, device_id_string_buff));
	if (default_name) {
		AUD_PRINTF("Default name: %s\n", default_name);
		sprintf(system_buf,"astparam s dante_default_name %s",default_name);
		system(system_buf);
	}
	if (friendly_name) {
		memset(system_buf,0,sizeof(system_buf));
		AUD_PRINTF("Friendly name: %s\n", friendly_name);
		sprintf(system_buf,"astparam s dante_friendly_name %s",friendly_name);
		system(system_buf);
	}
	if (domain) {
		AUD_PRINTF("Dante domain: %s\n", domain);
	}
	if (advertised_name) {
		AUD_PRINTF("Advertised name: %s\n", advertised_name);
	}

	return AUD_SUCCESS;
}
#endif //HOSTCPU_API_SUPPORT_DDP_DEVICE_ID

#if (defined HOSTCPU_API_SUPPORT_DDP_DEVICE_IDENTIFY)
aud_error_t handle_ddp_device_identify(ddp_packet_read_info_t *ddp_rinfo, uint16_t offset)
{
	aud_error_t result;
	ddp_request_id_t request_id;
	ddp_status_t status;

	result = ddp_read_device_identify_response(ddp_rinfo, offset, &request_id, &status);
	if (result != AUD_SUCCESS)
	{
		return result;
	}

	AUD_PRINTF("Identify message received!\n");

	return AUD_SUCCESS;
}
#endif //HOSTCPU_API_SUPPORT_DDP_DEVICE_IDENTIFY

#if (defined HOSTCPU_API_SUPPORT_DDP_DEVICE_GPIO)
aud_error_t handle_ddp_device_gpio(ddp_packet_read_info_t *ddp_rinfo, uint16_t offset)
{
	aud_error_t result;
	ddp_request_id_t request_id;
	ddp_status_t status;
	uint32_t interrupt_trigger_mask, input_state_valid_mask, input_state_values, output_state_valid_mask, output_state_values;

	result = ddp_read_device_gpio_response(ddp_rinfo, offset, &request_id, &status, &interrupt_trigger_mask, &input_state_valid_mask, &input_state_values, &output_state_valid_mask, &output_state_values);
	if (result != AUD_SUCCESS)
	{
		return result;
	}

	AUD_PRINTF("Interrupt Trigger Mask: 0x%08X\n", interrupt_trigger_mask);
	AUD_PRINTF("Input State Valid Mask: 0x%08X\n", input_state_valid_mask);
	AUD_PRINTF("Input State Values: 0x%08X\n", input_state_values);
	AUD_PRINTF("Output State Valid Mask: 0x%08X\n", output_state_valid_mask);
	AUD_PRINTF("Output State Values: 0x%08X\n", output_state_values);

	return AUD_SUCCESS;
}
#endif //HOSTCPU_API_SUPPORT_DDP_DEVICE_GPIO

#if (defined HOSTCPU_API_SUPPORT_DDP_DEVICE_SWITCH_LED)
aud_error_t handle_ddp_device_switch_led(ddp_packet_read_info_t *ddp_rinfo, uint16_t offset)
{
	aud_error_t result;
	ddp_request_id_t request_id;
	ddp_status_t status;
	uint8_t switch_led_mode;

	result = ddp_read_device_switch_led_response(ddp_rinfo, offset, &request_id, &status, &switch_led_mode);
	if (result != AUD_SUCCESS)
	{
		return result;
	}

	AUD_PRINTF("Switch LED Mode: ");

	switch(switch_led_mode)
	{
		case SWITCH_LED_MODE_FORCE_OFF :
			AUD_PRINTF("Forced OFF\n");
			break;
		case SWITCH_LED_MODE_FORCE_ON :
			AUD_PRINTF("Forced ON\n");
			break;
		case SWITCH_LED_MODE_NORMAL :
			AUD_PRINTF("Normal\n");
			break;
		case SWITCH_LED_MODE_LINK_ONLY :
			AUD_PRINTF("Link Only\n");
			break;
		default:
			AUD_PRINTF("UNKNOWN!\n");
			break;
	}

	return AUD_SUCCESS;
}
#endif //HOSTCPU_API_SUPPORT_DDP_DEVICE_SWITCH_LED

#if (defined HOSTCPU_API_SUPPORT_DDP_DEVICE_AES67)
aud_error_t handle_ddp_device_aes67(ddp_packet_read_info_t *ddp_rinfo, uint16_t offset)
{
	aud_error_t result;
	ddp_request_id_t request_id;
	ddp_status_t status;
	uint8_t aes67_support, aes67_current, aes67_reboot;

	result = ddp_read_device_aes67_response(ddp_rinfo, offset, &request_id, &status, &aes67_support, &aes67_current, &aes67_reboot);
	if (result != AUD_SUCCESS)
	{
		return result;
	}

	AUD_PRINTF("AES67 Support : %s, current mode : %s, reboot mode : %s\n",
			(aes67_support==AES67_MODE_SUPPORTED) ? "Supported" : "Not supported",
			(aes67_current==AES67_MODE_ENABLED) ? "Enabled" : "Disabled",
			(aes67_reboot==AES67_MODE_ENABLED) ? "Enabled" : "Disabled"	);

	return AUD_SUCCESS;
}
#endif //HOSTCPU_API_SUPPORT_DDP_DEVICE_AES67

#if (defined HOSTCPU_API_SUPPORT_DDP_DEVICE_SWITCH_REDUNDANCY)
aud_error_t handle_ddp_device_switch_redundancy(ddp_packet_read_info_t *ddp_rinfo, uint16_t offset)
{
	aud_error_t result;
	ddp_request_id_t request_id;
	ddp_status_t status;
	uint8_t switch_redundancy_support, switch_redundancy_current, switch_redundancy_reboot;

	result = ddp_read_device_switch_redundancy_response(ddp_rinfo, offset, &request_id, &status, &switch_redundancy_support, &switch_redundancy_current, &switch_redundancy_reboot);
	if (result != AUD_SUCCESS)
	{
		return result;
	}

	AUD_PRINTF("Switch Support : %s, current mode : %s, reboot mode : %s\n",
		(switch_redundancy_support == SWITCH_REDUNDANCY_MODE_SUPPORTED) ? "Supported" : "Not supported",
		(switch_redundancy_current == SWITCH_REDUNDANCY_SWITCH_MODE_ENABLED) ? "Switch_mode" : ((switch_redundancy_current == SWITCH_REDUNDANCY_REDUNDANCY_MODE_ENABLED) ? "Redundancy_mode" : "Disabled"),
		(switch_redundancy_reboot == SWITCH_REDUNDANCY_SWITCH_MODE_ENABLED) ? "Switch_mode" : ((switch_redundancy_reboot == SWITCH_REDUNDANCY_REDUNDANCY_MODE_ENABLED) ? "Redundancy_mode" : "Disabled")
					);

	return AUD_SUCCESS;
}
#endif //HOSTCPU_API_SUPPORT_DDP_DEVICE_SWITCH_REDUNDANCY

#if (defined HOSTCPU_API_SUPPORT_DDP_DEVICE_UART_CONFIG)
aud_error_t handle_ddp_device_uart_config(ddp_packet_read_info_t *ddp_rinfo, uint16_t offset)
{
	aud_error_t result;
	ddp_message_read_info_t msg_info;
	ddp_request_id_t request_id;
	ddp_status_t status;
	uint8_t uart_num = 0, i;
	uint8_t uart_mode, uart_user_configurable, uart_bits, uart_parity, uart_stop_bits;
	uint32_t uart_speed;

	result = ddp_read_uart_config_response_header(ddp_rinfo, offset, &msg_info, &request_id, &status, &uart_num);

	if (result != AUD_SUCCESS)
	{
		return result;
	}

	AUD_PRINTF("Number of UART - %d\n", uart_num);

	for(i= 0; i < uart_num; i++)
	{
		ddp_read_uart_config_response_uart_st_array(&msg_info, uart_num, i, &uart_mode,
			&uart_bits, &uart_parity, &uart_stop_bits, &uart_user_configurable, &uart_speed);

		AUD_PRINTF("UART index %d mode = ", i);

		switch (uart_mode)
		{
			case DEVICE_UART_USER_LOCKED:
				AUD_PRINTF("Locked\n");
				break;
			case DEVICE_UART_USER_CONFIGURABLE:
				AUD_PRINTF("user configurable\n");
				break;
			case DEVICE_UART_USER_CONFIGURED:
				AUD_PRINTF("user configured\n");
				break;
			default:
				AUD_PRINTF("NONE!\n");
				break;
		}

		AUD_PRINTF("uart out_uart_bits - %d\n", uart_bits);
		AUD_PRINTF("uart out_uart_parity - %d\n", uart_parity);
		AUD_PRINTF("uart out_uart_stop_bits - %d\n", uart_stop_bits);
		AUD_PRINTF("uart uart_user_configurable - %d\n", uart_user_configurable);
		AUD_PRINTF("uart uart_speed - %d\n\n", uart_speed);
	}


	return AUD_SUCCESS;
}
#endif //HOSTCPU_API_SUPPORT_DDP_DEVICE_UART_CONFIG

#if (defined HOSTCPU_API_SUPPORT_DDP_DEVICE_VLAN_CONFIG)
aud_error_t handle_ddp_device_vlan_config(ddp_packet_read_info_t *ddp_rinfo, uint16_t offset)
{
	aud_error_t result;
	ddp_message_read_info_t msg_info;
	ddp_request_id_t request_id;
	ddp_status_t status;
	int i;
	uint8_t vlan_max_num, vlan_num, vlan_current_id, vlan_reboot_id;
	uint16_t vlan_config_port_mask;
	uint32_t vlan_primary_bitmap, vlan_secondary_bitmap, vlan_user_2_bitmap, vlan_user_3_bitmap;
	char *vlan_name_string = NULL;

	result = ddp_read_device_vlan_config_response_header(ddp_rinfo, offset, &msg_info, &request_id, &status,
				&vlan_max_num, &vlan_num, &vlan_current_id, &vlan_reboot_id, &vlan_config_port_mask);

	if (result != AUD_SUCCESS)
	{
		return result;
	}

	AUD_PRINTF("number of vlan config - %d\n", vlan_num);
	AUD_PRINTF("max number of vlan config - %d\n", vlan_max_num);
	AUD_PRINTF("current vlan config id - %d\n", vlan_current_id);
	AUD_PRINTF("reboot vlan config id - %d\n", vlan_reboot_id);
	AUD_PRINTF("vlan config port mask - %d\n", vlan_config_port_mask);

	for(i= 0; i < vlan_num; i++)
	{
		ddp_read_device_vlan_config_response_vlan_st_array(&msg_info, vlan_num, i,
			&vlan_primary_bitmap, &vlan_secondary_bitmap, &vlan_user_2_bitmap, &vlan_user_3_bitmap);

		AUD_PRINTF("vlan id - %d, primary bitmap - %x\n", (i+1), vlan_primary_bitmap);
		AUD_PRINTF("vlan id - %d, secondary bitmap - %x\n", (i+1), vlan_secondary_bitmap);
		AUD_PRINTF("vlan id - %d, user vlan 2 bitmap - %x\n", (i+1), vlan_user_2_bitmap);
		AUD_PRINTF("vlan id - %d, user vlan 3 bitmap - %x\n", (i+1), vlan_user_3_bitmap);
	}

	for(i= 0; i < vlan_num; i++)
	{
		ddp_read_device_vlan_config_response_name_string(&msg_info, i, &vlan_name_string);

		AUD_PRINTF("vlan index - %d. name - %s\n", i, vlan_name_string);
	}

	return AUD_SUCCESS;
}
#endif //HOSTCPU_API_SUPPORT_DDP_DEVICE_VLAN_CONFIG

#if (defined HOSTCPU_API_SUPPORT_DDP_DEVICE_METER_CONFIG)
aud_error_t handle_ddp_device_meter_config(ddp_packet_read_info_t *ddp_rinfo, uint16_t offset)
{
	aud_error_t result;
	ddp_request_id_t request_id;
	ddp_status_t status;
	uint16_t out_current_meter_rate;

	result = ddp_read_device_meter_config_response(ddp_rinfo, offset, &request_id, &status, &out_current_meter_rate);
	if (result != AUD_SUCCESS)
	{
		return result;
	}

	AUD_PRINTF("Current meter config rate - %d\n", out_current_meter_rate);
	return AUD_SUCCESS;
}
#endif //HOSTCPU_API_SUPPORT_DDP_DEVICE_METER_CONFIG

#if (defined HOSTCPU_API_SUPPORT_DDP_DEVICE_LOCK_UNLOCK)
aud_error_t handle_ddp_device_lock_unlock(ddp_packet_read_info_t *ddp_rinfo, uint16_t offset)
{
	aud_error_t result;
	ddp_request_id_t request_id;
	ddp_status_t status;
	uint8_t lock_state;

	result = ddp_read_device_lock_unlock_response(ddp_rinfo, offset, &request_id, &status, &lock_state);
	if (result != AUD_SUCCESS)
	{
		return result;
	}

	AUD_PRINTF("Lock State: ");

	switch (lock_state)
	{
	case LOCK_UNLOCK_STATUS_LOCKED:
		AUD_PRINTF("Locked\n");
		break;
	case LOCK_UNLOCK_STATUS_UNLOCKED:
		AUD_PRINTF("Unlocked\n");
		break;
	case LOCK_UNLOCK_STATUS_NONE:
		AUD_PRINTF("Nine / Not Supported\n");
		break;
	default:
		AUD_PRINTF("UNKNOWN!\n");
		break;
	}

	return AUD_SUCCESS;
}
#endif //HOSTCPU_API_SUPPORT_DDP_DEVICE_LOCK_UNLOCK

#if (defined HOSTCPU_API_SUPPORT_DDP_DEVICE_DANTE_DOMAIN)
aud_error_t handle_ddp_device_dante_domain(ddp_packet_read_info_t *ddp_rinfo, uint16_t offset)
{
	aud_error_t result;
	ddp_request_id_t request_id;
	ddp_status_t status;
	uint8_t dante_domain_enrollment;
	uint8_t dante_domain_manager_connection;
	uint8_t ddm_admin_local_access_control_policy;
	uint8_t ddm_admin_remote_access_control_policy;

	result = ddp_read_device_dante_domain_response
		(
			ddp_rinfo, 
			offset, 
			&request_id, 
			&status,
			&dante_domain_enrollment, 
			&dante_domain_manager_connection, 
			&ddm_admin_local_access_control_policy, 
			&ddm_admin_remote_access_control_policy
		);

	if (result != AUD_SUCCESS)
	{
		return result;
	}

	//print the domain enrollment state
	switch (dante_domain_enrollment)
	{
	case DDP_DANTE_DOMAIN_ENROLLMENT_UNKNOWN:
		AUD_PRINTF("Dante Domain Enrollment state - UNKNOWN\n");
		break;

	case DDP_DANTE_DOMAIN_ENROLLMENT_UNENROLLED:
		AUD_PRINTF("Dante Domain Enrollment state - UNENROLLED\n");
		break;

	case DDP_DANTE_DOMAIN_ENROLLMENT_ENROLLED:
		AUD_PRINTF("Dante Domain Enrollment state - ENROLLED\n");
		break;

	default:
		AUD_PRINTF("Dante Domain Enrollment state - ERROR INVALID VALUE val=%d\n", dante_domain_enrollment);
		break;
	}

	//print the DDM connection state
	switch (dante_domain_manager_connection)
	{
	case DDP_DDM_MANAGER_CONNECTION_UNKNOWN:
		AUD_PRINTF("Dante Domain Manager Connection state - UNKNOWN\n");
		break;

	case DDP_DDM_MANAGER_CONNECTION_NOTCONNECTED:
		AUD_PRINTF("Dante Domain Manager Connection state - NOT CONNECTED\n");
		break;

	case DDP_DDM_MANAGER_CONNECTION_CONNECTED:
		AUD_PRINTF("Dante Domain Manager Connection state - CONNECTED\n");
		break;

	default:
		AUD_PRINTF("Dante Domain Manager Connection state - ERROR INVALID VALUE val=%d\n", dante_domain_manager_connection);
		break;
	}

	//print the DDM admiin local access control policy state
	switch (ddm_admin_local_access_control_policy)
	{
	case DEVICE_DANTE_DOMAIN_LOCAL_ACCESS_CONTROL_POLICY_UNKNOWN_INVALID:
		AUD_PRINTF("DDM Admin Local Access Control Policy - UNKNOWN\n");
		break;

	case DEVICE_DANTE_DOMAIN_LOCAL_ACCESS_CONTROL_POLICY_NONE:
		AUD_PRINTF("DDM Admin Local Access Control Policy - NONE\n");
		break;

	case DEVICE_DANTE_DOMAIN_LOCAL_ACCESS_CONTROL_POLICY_READ_ONLY:
		AUD_PRINTF("DDM Admin Local Access Control Policy - READ ONLY\n");
		break;

	case DEVICE_DANTE_DOMAIN_LOCAL_ACCESS_CONTROL_POLICY_READ_WRITE:
		AUD_PRINTF("DDM Admin Local Access Control Policy - READ+WRITE\n");
		break;

	default:
		AUD_PRINTF("DDM Admin Local Access Control Policy - ERROR INVALID VALUE val=%d\n", ddm_admin_local_access_control_policy);
		break;
	}

	switch (ddm_admin_remote_access_control_policy)
	{
	case DEVICE_DANTE_DOMAIN_REMOTE_ACCESS_CONTROL_POLICY_UNKNOWN_INVALID:
		AUD_PRINTF("DDM Admin Remote Access Control Policy - UNKNOWN\n");
		break;

	case DEVICE_DANTE_DOMAIN_REMOTE_ACCESS_CONTROL_POLICY_NONE:
		AUD_PRINTF("DDM Admin Remote Access Control Policy - NONE\n");
		break;

	case DEVICE_DANTE_DOMAIN_REMOTE_ACCESS_CONTROL_POLICY_READ_ONLY:
		AUD_PRINTF("DDM Admin Remote Access Control Policy - READ ONLY\n");
		break;

	case DEVICE_DANTE_DOMAIN_REMOTE_ACCESS_CONTROL_POLICY_READ_WRITE:
		AUD_PRINTF("DDM Admin Remote Access Control Policy - READ+WRITE\n");
		break;

	default:
		AUD_PRINTF("DDM Admin Remote Access Control Policy - ERROR INVALID VALUE val=%d\n", ddm_admin_remote_access_control_policy);
		break;
	}

	return AUD_SUCCESS;
}
#endif //HOSTCPU_API_SUPPORT_DDP_DEVICE_DANTE_DOMAIN

#if (defined HOSTCPU_API_SUPPORT_DDP_DEVICE_SWITCH_STATUS)
aud_error_t handle_ddp_device_switch_status(ddp_packet_read_info_t *ddp_rinfo, uint16_t offset)
{
	aud_error_t result;
	ddp_message_read_info_t msg_info;
	ddp_request_id_t request_id;
	ddp_status_t status;
	uint8_t i = 0;
	uint16_t enabled_ports_values;
	uint8_t num_ports;
	ddp_switch_port_status_t *port_status;

	result = ddp_read_device_switch_status_response_header(ddp_rinfo, offset, &msg_info, &request_id, &status, &enabled_ports_values, &num_ports);
	if (result != AUD_SUCCESS)
	{
		return result;
	}

	if (enabled_ports_values != 0)
	{
		AUD_PRINTF("Enabled ports:\n");

		while (enabled_ports_values)
		{
			if (enabled_ports_values & 0x1)
			{
				AUD_PRINTF(" Port %u\n", i);
			}
			enabled_ports_values >>= 1;
			++i;
		}

		for (i = 0; i < num_ports; ++i)
		{
			result = ddp_read_device_switch_status_response_port(&msg_info, i, &port_status);
			if (result != AUD_SUCCESS)
			{
				return result;
			}

			AUD_PRINTF("Port [%d] info:\n", port_status->port_no);
			if (port_status->valid_flags & DEVICE_SWITCH_PORT_STATUS_PORT_CMODE_VALID) {
				AUD_PRINTF("  CMode: %u\n", port_status->port_cmode);
			}
			if (port_status->valid_flags & DEVICE_SWITCH_PORT_STATUS_LINK_SPEED_VALID) {
				AUD_PRINTF("  Link speed: %u Mbps\n", port_status->link_speed);
			}
			if (port_status->link_flags_mask & DEVICE_SWITCH_PORT_STATUS_LINK_STATE)
			{
				uint8_t link_state = port_status->link_flags & DEVICE_SWITCH_PORT_STATUS_LINK_STATE;
				AUD_PRINTF("  Link state: %s\n", (link_state != 0 ? "Up" : "Down"));
			}
			if (port_status->link_flags_mask & DEVICE_SWITCH_PORT_STATUS_DUPLEX)
			{
				uint8_t duplex = port_status->link_flags & DEVICE_SWITCH_PORT_STATUS_DUPLEX;
				AUD_PRINTF("  Duplex state: %s\n", (duplex != 0 ? "Full" : "Half"));
			}
			if (port_status->valid_flags & DEVICE_SWITCH_PORT_STATUS_ERR_COUNT_VALID) {
				AUD_PRINTF("  Error count: %u\n", port_status->error_count);
			}
		}
	}

	return AUD_SUCCESS;
}
#endif // HOSTCPU_API_SUPPORT_DDP_DEVICE_SWITCH_STATUS

#if (defined HOSTCPU_API_SUPPORT_DDP_NETWORK_BASIC)
aud_error_t handle_ddp_network_basic(ddp_packet_read_info_t *ddp_rinfo, uint16_t offset)
{
	aud_error_t result;
	ddp_message_read_info_t msg_info;
	ddp_request_id_t request_id;
	ddp_status_t status;
	uint16_t i;
	network_basic_control_t nw_basic_info;

	result = ddp_read_network_basic_response_header(ddp_rinfo, offset, &msg_info, &request_id, &status, &nw_basic_info);
	if (result != AUD_SUCCESS)
	{
		return result;
	}

	AUD_PRINTF("Number of interfaces: %d\n", nw_basic_info.num_intfs);
	AUD_PRINTF("Number of DNS servers: %d\n", nw_basic_info.num_dns_srvrs);

	if (nw_basic_info.num_dns_srvrs)
	{
		uint16_t family;
		ddp_ip_addr_nw_t ip_addr;

		for (i = 0; i < nw_basic_info.num_dns_srvrs; ++i)
		{
			result = ddp_read_network_basic_response_dns_nw_ip(&msg_info, i, &family, &ip_addr);
			if (result != AUD_SUCCESS)
			{
				return result;
			}

			ip_addr.ip_addr = ntohl(ip_addr.ip_addr);

			AUD_PRINTF("DNS[%d]: Family: %d\n", i + 1, family);
			AUD_PRINTF("DNS[%d]: IP address: %d.%d.%d.%d\n", i + i,
				(ip_addr.ip_addr >> 24) & 0xFF, (ip_addr.ip_addr >> 16) & 0xFF, (ip_addr.ip_addr >> 8) & 0xFF, (ip_addr.ip_addr & 0xFF));
		}
	}
	if (nw_basic_info.num_intfs)
	{
		uint16_t flags, mode, num_addrs, j;
		uint32_t speed;
		ddp_network_etheraddr_t mac_addr;

		for (i = 0; i < nw_basic_info.num_intfs; ++i)
		{
			result = ddp_read_network_basic_response_interface(&msg_info, i, &flags, &mode, &speed, &mac_addr, &num_addrs);

			AUD_PRINTF("Interface[%d]: \n", i + i);
			AUD_PRINTF("  Flags: 0x%04X\n", flags);
			AUD_PRINTF("  Mode: 0x%d\n", mode);
			AUD_PRINTF("  Link speed: %d\n", speed);
			AUD_PRINTF("  MAC address: %02X:%02X:%02X:%02X:%02X:%02X\n", mac_addr.addr[0], mac_addr.addr[1],
																	mac_addr.addr[2], mac_addr.addr[3], mac_addr.addr[4], mac_addr.addr[5]);
			FILE* fp = fopen("/etc/dnt_mac", "w");
			if (fp)
			{
				fprintf(fp, "%02X:%02X:%02X:%02X:%02X:%02X\n", mac_addr.addr[0], mac_addr.addr[1],
						mac_addr.addr[2], mac_addr.addr[3], mac_addr.addr[4], mac_addr.addr[5]);
				fflush(fp);
				fclose(fp);
			}

			AUD_PRINTF("  Num addresses: %d\n", num_addrs);
			run_flag = 0xffff;

			if (num_addrs)
			{
				uint16_t family;
				ddp_ip_addr_nw_t ip_addr, net_mask, gateway;

				for (j = 0; j < num_addrs; ++j)
				{
					result = ddp_read_network_basic_response_interface_address_nw_ip(&msg_info, i, j, &family, &ip_addr, &net_mask, &gateway);

					// AUD_PRINTF("  Address[%d]: \n", j + i);
					// AUD_PRINTF("    Family: %d\n", family);
					ip_addr.ip_addr = ntohl(ip_addr.ip_addr);
					// AUD_PRINTF("    IP address: %d.%d.%d.%d\n", 
					//	(ip_addr.ip_addr >> 24) & 0xFF, (ip_addr.ip_addr >> 16) & 0xFF, (ip_addr.ip_addr >> 8) & 0xFF, (ip_addr.ip_addr & 0xFF));
					net_mask.ip_addr = ntohl(net_mask.ip_addr);
					//AUD_PRINTF("    Net mask: %d.%d.%d.%d\n", 
					//	(net_mask.ip_addr >> 24) & 0xFF, (net_mask.ip_addr >> 16) & 0xFF, (net_mask.ip_addr >> 8) & 0xFF, (net_mask.ip_addr & 0xFF));
					gateway.ip_addr = ntohl(gateway.ip_addr);
					//AUD_PRINTF("    Gateway: %d.%d.%d.%d\n",
					//	(gateway.ip_addr >> 24) & 0xFF, (gateway.ip_addr >> 16) & 0xFF, (gateway.ip_addr >> 8) & 0xFF, (gateway.ip_addr & 0xFF));
				}
			}
		}
	}
	if (nw_basic_info.num_domains)
	{
		char *domain;

		for (i = 0; i < nw_basic_info.num_domains; ++i)
		{
			result = ddp_read_network_basic_response_domain(&msg_info, i, &domain);
			if (domain)
			{
				AUD_PRINTF("Domain[%d]: %s\n", i + 1, domain);
			}
		}
	}

	return AUD_SUCCESS;
}
#endif //HOSTCPU_API_SUPPORT_DDP_NETWORK_BASIC

#if (defined HOSTCPU_API_SUPPORT_DDP_NETWORK_CONFIG)
aud_error_t handle_ddp_network_config(ddp_packet_read_info_t *ddp_rinfo, uint16_t offset)
{
	aud_error_t result;
	ddp_request_id_t request_id;
	ddp_status_t status;
	uint8_t curr_mode, reboot_mode, change_flags, interface_index;
	network_ip_config_nw_ip_t ip_config = {0};
	network_dns_config_nw_ip_t dns_config = {0};
	char *domain;

	result = ddp_read_network_config_response_nw_ip(ddp_rinfo, offset, &request_id, &interface_index, &status, &change_flags, &curr_mode, 
											&reboot_mode, &ip_config, &dns_config, &domain);
	if (result != AUD_SUCCESS)
	{
		return result;
	}

	AUD_PRINTF("Change flags: 0x%02X\n", change_flags);
	AUD_PRINTF("Current mode: %d\n", curr_mode);
	AUD_PRINTF("Reboot mode: %d\n", reboot_mode);
	if (ip_config.ip_addr.ip_addr)
	{
		ip_config.ip_addr.ip_addr = ntohl(ip_config.ip_addr.ip_addr);
		AUD_PRINTF("Reboot IP address: %d.%d.%d.%d\n", (ip_config.ip_addr.ip_addr >> 24) & 0xFF, (ip_config.ip_addr.ip_addr >> 16) & 0xFF,
			(ip_config.ip_addr.ip_addr >> 8) & 0xFF, ip_config.ip_addr.ip_addr & 0xFF);
	}
	if (ip_config.subnet_mask.ip_addr)
	{
		ip_config.subnet_mask.ip_addr = ntohl(ip_config.subnet_mask.ip_addr);
		AUD_PRINTF("Reboot subnet mask: %d.%d.%d.%d\n", (ip_config.subnet_mask.ip_addr >> 24) & 0xFF, (ip_config.subnet_mask.ip_addr >> 16) & 0xFF,
			(ip_config.subnet_mask.ip_addr >> 8) & 0xFF, ip_config.subnet_mask.ip_addr & 0xFF);
	}
	if (ip_config.gateway.ip_addr)
	{
		ip_config.gateway.ip_addr = ntohl(ip_config.gateway.ip_addr);
		AUD_PRINTF("Reboot gateway: %d.%d.%d.%d\n", (ip_config.gateway.ip_addr >> 24) & 0xFF, (ip_config.gateway.ip_addr >> 16) & 0xFF,
			(ip_config.gateway.ip_addr >> 8) & 0xFF, ip_config.gateway.ip_addr & 0xFF);
	}
	if (dns_config.ip_addr.ip_addr)
	{
		dns_config.ip_addr.ip_addr = ntohl(dns_config.ip_addr.ip_addr);
		AUD_PRINTF("Reboot static DNS server: %d.%d.%d.%d\n", (dns_config.ip_addr.ip_addr >> 24) & 0xFF, (dns_config.ip_addr.ip_addr >> 16) & 0xFF,
			(dns_config.ip_addr.ip_addr >> 8) & 0xFF, dns_config.ip_addr.ip_addr & 0xFF);
	}
	if (domain)
	{
		AUD_PRINTF("Reboot static domain: %s\n", domain);
	}

	return AUD_SUCCESS;
}
#endif //HOSTCPU_API_SUPPORT_DDP_NETWORK_CONFIG

#if (defined HOSTCPU_API_SUPPORT_DDP_CLOCK_BASIC_LEGACY)
aud_error_t handle_ddp_clock_basic_legacy(ddp_packet_read_info_t *ddp_rinfo, uint16_t offset)
{
	aud_error_t result = AUD_SUCCESS;
	ddp_request_id_t request_id;
	ddp_status_t status;
	uint8_t clock_state, mute_state, ext_wc_state, preferred;
	uint32_t cap_flags, drift;

	result = ddp_read_clock_basic_legacy_response(ddp_rinfo, offset, &request_id, &status, &cap_flags, &clock_state, 
										&mute_state, &ext_wc_state, &preferred, &drift);
	if (result != AUD_SUCCESS)
	{
		return result;
	}

	AUD_PRINTF("Capability flags: 0x%08X\n", cap_flags);
	AUD_PRINTF("Clock state: %d\n", clock_state);
	AUD_PRINTF("Mute state: %d\n", mute_state);
	AUD_PRINTF("External word clock state: %d\n", ext_wc_state);
	AUD_PRINTF("Preferred: %d\n", preferred);
	AUD_PRINTF("Drift: %d\n", drift);

	return AUD_SUCCESS;
}
#endif 

#if (defined HOSTCPU_API_SUPPORT_DDP_CLOCK_CONFIG)
aud_error_t handle_ddp_clock_config(ddp_packet_read_info_t *ddp_rinfo, uint16_t offset)
{
	aud_error_t result;
	ddp_request_id_t request_id;
	ddp_status_t status;
	ddp_clock_config_control_params_t params;
	uint16_t num_ports, i;
	ddp_message_read_info_t msg_info;
	ddp_clock_config_port_params_t port_params;

	result = ddp_read_clock_config_response(ddp_rinfo, offset, &msg_info, &request_id, &status, &params, &num_ports);
	if (result != AUD_SUCCESS)
	{
		return result;
	}

	AUD_PRINTF("Preferred: %d\n", params.preferred);
	AUD_PRINTF("Word clock sync: %d\n", params.ext_word_clock_sync);
	AUD_PRINTF("Logging: %d\n", params.logging);
	AUD_PRINTF("Multicast: %d\n", params.multicast);
	AUD_PRINTF("Slave only: %d\n", params.slave_only);
	AUD_PRINTF("Clock protocol: %04X\n", params.clock_protocol);
	AUD_PRINTF("Clock protocol mask: %04X\n", params.clock_protocol_mask);
	AUD_PRINTF("Unicast delay request clock protocol: %04X\n", params.unicast_delay_clock_protocol);
	AUD_PRINTF("Unicast delay request clock protocol mask: %04X\n", params.unicast_delay_clock_protocol_mask);

	if (num_ports != 0)
	{
		for (i = 0; i < num_ports; ++i)
		{
			result = ddp_read_clock_config_response_port(&msg_info, i, &port_params);
			if (result != AUD_SUCCESS)
			{
				return result;
			}

			AUD_PRINTF("Port[%u]: \n", i + 1);
			AUD_PRINTF("Port Id: %u\n", port_params.port_id);
			AUD_PRINTF("Enable port: %u\n", port_params.enable_port);
		}
	}

	return AUD_SUCCESS;
}
#endif //HOSTCPU_API_SUPPORT_DDP_CLOCK_CONFIG

#if (defined HOSTCPU_API_SUPPORT_DDP_CLOCK_PULLUP)
#define MAX_SUBDOMAIN_LENGTH 16
aud_error_t handle_ddp_clock_pullup(ddp_packet_read_info_t *ddp_rinfo, uint16_t offset)
{
	aud_error_t result;
	ddp_request_id_t request_id;
	ddp_status_t status;
	uint8_t current_pullup;
	uint8_t reboot_pullup;
	uint8_t * current_subdomain;
	uint16_t current_subdomain_length;
	uint8_t * reboot_subdomain;
	uint16_t reboot_subdomain_length;
	clock_supported_pullup_t * supported_pullups;
	uint16_t num_supported_pullups;

	char subdomain_string[MAX_SUBDOMAIN_LENGTH + 1];

	result = ddp_read_clock_pullup_response
		(ddp_rinfo,
		offset,
		&request_id,
		&status,
		&current_pullup,
		&reboot_pullup,
		&current_subdomain,
		&current_subdomain_length,
		&reboot_subdomain,
		&reboot_subdomain_length,
		&supported_pullups,
		&num_supported_pullups
		);

	if (result != AUD_SUCCESS)
	{
		return result;
	}

	AUD_PRINTF("Current Pullup: %s\n", ddp_pullup_to_string(current_pullup));
	AUD_PRINTF("Reboot Pullup: %s\n", ddp_pullup_to_string(current_pullup));

	//build and print the current subdomain
	if (current_subdomain != NULL) {
		memset(subdomain_string, 0x00, sizeof(subdomain_string));
		if (current_subdomain_length > MAX_SUBDOMAIN_LENGTH) {
			memcpy(subdomain_string, current_subdomain, MAX_SUBDOMAIN_LENGTH);
		}
		else {
			memcpy(subdomain_string, current_subdomain, current_subdomain_length);
		}
		AUD_PRINTF("Current Subddomain: %s\n", subdomain_string);
	}

	//build and print the reboot subdomain
	if (reboot_subdomain != NULL) {
		memset(subdomain_string, 0x00, sizeof(subdomain_string));
		if (reboot_subdomain_length > MAX_SUBDOMAIN_LENGTH) {
			memcpy(subdomain_string, reboot_subdomain, MAX_SUBDOMAIN_LENGTH);
		}
		else {
			memcpy(subdomain_string, reboot_subdomain, reboot_subdomain_length);
		}
		AUD_PRINTF("Reboot Subddomain: %s\n", subdomain_string);
	}

	//print the supported pull ups
	if (num_supported_pullups)
	{
		int i;
		AUD_PRINTF("Supported Pullups =");

		for (i = 0; i < num_supported_pullups; i++)
		{
			AUD_PRINTF(" [%s]", ddp_pullup_to_string(supported_pullups[i].pullup));
		}
		AUD_PRINTF("\n");
	}

	return AUD_SUCCESS;
}
#endif //HOSTCPU_API_SUPPORT_DDP_CLOCK_PULLUP

#if (defined HOSTCPU_API_SUPPORT_DDP_CLOCK_BASIC2)
aud_error_t handle_ddp_clock_basic2(ddp_packet_read_info_t *ddp_rinfo, uint16_t offset)
{
	aud_error_t result = AUD_SUCCESS;
	ddp_message_read_info_t msg_info;
	ddp_request_id_t request_id;
	ddp_status_t status;
	uint8_t clock_source, clock_state, servo_state, mute_state, ext_wc_state, stratum, preferred, num_ports, port_size;
	uint8_t *uuid, *master_uuid, *gm_uuid;
	uint32_t cap_flags, i; 
	uint32_t drift, max_drift;
	ddp_clock_port_t *port;

	result = ddp_read_clock_basic2_response_header(ddp_rinfo, offset, &msg_info, &request_id, &status, &cap_flags, 
		&clock_source, &clock_state, &servo_state, &preferred, &mute_state, &ext_wc_state, &stratum, &drift, &max_drift, 
		&uuid, &master_uuid, &gm_uuid, &num_ports, &port_size);
	
	if (result != AUD_SUCCESS)
	{
		return result;
	}

	AUD_PRINTF("Capability flags: 0x%08X\n", cap_flags);
	AUD_PRINTF("Clock source: %d\n", clock_source);
	AUD_PRINTF("Clock state: %d\n", clock_state);
	AUD_PRINTF("Servo state: %d\n", servo_state);
	AUD_PRINTF("Preferred: %d\n", preferred);
	AUD_PRINTF("Mute state: %d\n", mute_state);
	AUD_PRINTF("External word clock state: %d\n", ext_wc_state);
	AUD_PRINTF("Stratum: %d\n", stratum);
	AUD_PRINTF("Drift: %d\n", drift);
	AUD_PRINTF("Max drift: %d\n", max_drift);
	AUD_PRINTF("UUID: %02X:%02X:%02X:%02X:%02X:%02X\n", uuid[0], uuid[1], uuid[2], uuid[3], uuid[4], uuid[5]);
	AUD_PRINTF("Master UUID: %02X:%02X:%02X:%02X:%02X:%02X\n", master_uuid[0], master_uuid[1], master_uuid[2], master_uuid[3], master_uuid[4], master_uuid[5]);
	AUD_PRINTF("Grandmaster UUID: %02X:%02X:%02X:%02X:%02X:%02X\n", gm_uuid[0], gm_uuid[1], gm_uuid[2], gm_uuid[3], gm_uuid[4], gm_uuid[5]);
	AUD_PRINTF("Num ports: %d\n", num_ports);
	AUD_PRINTF("Port size: %d\n", port_size);

	for (i = 0; i < num_ports; ++i)
	{
		result = ddp_read_clock_basic2_response_port(&msg_info, i, &port);
		if (result != AUD_SUCCESS)
		{
			return result;
		}

		AUD_PRINTF("Port[%d]: \n", i+1);
		AUD_PRINTF("Flags: 0x%04X\n", port->flags);
		AUD_PRINTF("Port id: %d\n", port->port_id);
		AUD_PRINTF("Port protocol: %02X\n", port->port_protocol);
		AUD_PRINTF("Castness: %d\n", port->castness);
		AUD_PRINTF("Interface index: %d\n", port->interface_index);
		AUD_PRINTF("Port state: %d\n", port->port_state);
	}

	return AUD_SUCCESS;
}
#endif //HOSTCPU_API_SUPPORT_DDP_CLOCK_BASIC2

#if (defined HOSTCPU_API_SUPPORT_DDP_AUDIO_BASIC)
aud_error_t handle_ddp_audio_basic(ddp_packet_read_info_t *ddp_rinfo, uint16_t offset)
{
	aud_error_t result = AUD_SUCCESS;
	ddp_request_id_t request_id;
	ddp_status_t status;
	uint8_t change_flags;
	uint16_t default_enc, rx_chans, tx_chans;
	uint32_t cap_flags, default_srate;

	result = ddp_read_audio_basic_response(ddp_rinfo, offset, &request_id, &status, &cap_flags, &default_srate, 
										&default_enc, &rx_chans, &tx_chans, &change_flags);
	if (result != AUD_SUCCESS)
	{
		return result;
	}

	AUD_PRINTF("Capability flags: 0x%08X\n", cap_flags);
	AUD_PRINTF("Default sample rate: %d\n", default_srate);
	AUD_PRINTF("Default encoding: %d\n", default_enc);
	AUD_PRINTF("Rx chans: %d\n", rx_chans);
	AUD_PRINTF("Tx chans: %d\n", tx_chans);
	AUD_PRINTF("Change flags: %d\n", change_flags);

	return AUD_SUCCESS;
}
#endif //HOSTCPU_API_SUPPORT_DDP_AUDIO_BASIC

#if (defined HOSTCPU_API_SUPPORT_DDP_AUDIO_SRATE_CONFIG)
aud_error_t handle_ddp_audio_srate(ddp_packet_read_info_t *ddp_rinfo, uint16_t offset)
{
	aud_error_t result = AUD_SUCCESS;
	ddp_message_read_info_t msg_info;
	ddp_request_id_t request_id;
	ddp_status_t status;
	uint32_t current_srate, reboot_srate;
	uint16_t num_supported_srates, i;
	dante_samplerate_t supported_srate;

	result = ddp_read_audio_sample_rate_config_response(ddp_rinfo, offset, &msg_info, &request_id, &status,
													&current_srate, &reboot_srate, &num_supported_srates);
	if (result != AUD_SUCCESS)
	{
		return result;
	}

	AUD_PRINTF("Current sample rate: %d\n", current_srate);
	AUD_PRINTF("Reboot sample rate: %d\n", reboot_srate);
	AUD_PRINTF("Num supported sample rates: %d\n", num_supported_srates);

	if (num_supported_srates)
	{
		for (i = 0; i < num_supported_srates; ++i)
		{
			result = ddp_read_audio_sample_rate_config_supported_srate(&msg_info, i, &supported_srate);
			AUD_PRINTF("Supported sample rate[%d]: %d\n", i + 1, supported_srate);
		}
	}

	hostcpu_ddp_srate_update(current_srate);

	return AUD_SUCCESS;
}
#endif //HOSTCPU_API_SUPPORT_DDP_AUDIO_SRATE_CONFIG

#if (defined HOSTCPU_API_SUPPORT_DDP_AUDIO_ENC_CONFIG)
aud_error_t handle_ddp_audio_enc(ddp_packet_read_info_t *ddp_rinfo, uint16_t offset)
{
	aud_error_t result = AUD_SUCCESS;
	ddp_message_read_info_t msg_info;
	ddp_request_id_t request_id;
	ddp_status_t status;
	uint16_t current_enc, reboot_enc, num_supported_encs, i, supported_enc;

	result = ddp_read_audio_encoding_config_response(ddp_rinfo, offset, &msg_info, &request_id, &status,
													&current_enc, &reboot_enc, &num_supported_encs);
	if (result != AUD_SUCCESS)
	{
		return result;
	}

	AUD_PRINTF("Current encoding: %d\n", current_enc);
	AUD_PRINTF("Reboot encoding: %d\n", reboot_enc);
	AUD_PRINTF("Num supported encodings: %d\n", num_supported_encs);

	for (i = 0; i < num_supported_encs; ++i)
	{
		result = ddp_read_audio_encoding_config_supported_encoding(&msg_info, i, &supported_enc);
		AUD_PRINTF("Supported encoding[%d]: %d\n", i + 1, supported_enc);
	}

	return AUD_SUCCESS;
}
#endif //HOSTCPU_API_SUPPORT_DDP_AUDIO_ENC_CONFIG

#if (defined HOSTCPU_API_SUPPORT_DDP_AUDIO_SIGNAL_PRESENCE)
aud_error_t handle_ddp_audio_signal_presence_config(ddp_packet_read_info_t *ddp_rinfo, uint16_t offset)
{
	aud_error_t result = AUD_SUCCESS;
	ddp_message_read_info_t msg_info;
	ddp_request_id_t request_id;
	ddp_status_t status;
	uint8_t current_mode;

	result = ddp_read_audio_signal_presence_config_response(ddp_rinfo, offset, &msg_info, &request_id, &status, &current_mode);
	if (result != AUD_SUCCESS)
	{
		return result;
	}

	switch (current_mode)
	{
	case AUDIO_SIGNAL_PRESENCE_MODE__NONE:
		AUD_PRINTF("Current mode: NONE\n");
		break;

	case AUDIO_SIGNAL_PRESENCE_MODE__DISABLED:
		AUD_PRINTF("Current mode: DISABLED\n");
		break;

	case AUDIO_SIGNAL_PRESENCE_MODE__ENABLED:
		AUD_PRINTF("Current mode: ENABLED\n");
		break;

	default:
		AUD_PRINTF("Current mode: %d\n", current_mode);
		break;
	}

	return AUD_SUCCESS;
}

aud_error_t handle_ddp_audio_signal_presence_data(ddp_packet_read_info_t *ddp_rinfo, uint16_t offset)
{
	aud_error_t result = AUD_SUCCESS;
	ddp_message_read_info_t msg_info;
	ddp_request_id_t request_id;
	ddp_status_t status;
	uint16_t num_tx_chans, num_rx_chans, i;

	result = ddp_read_audio_signal_presence_data_response(ddp_rinfo, offset, &msg_info, &request_id, &status, &num_tx_chans, &num_rx_chans);
	if (result != AUD_SUCCESS)
	{
		return result;
	}

	AUD_PRINTF("Num TX channels: %d\n", num_tx_chans);
	AUD_PRINTF("Num RX channels: %d\n", num_rx_chans);

	//print the TX channels
	uint8_t signal;
	for (i = 0; i < num_tx_chans; i++) 
	{
		if (ddp_read_audio_signal_presence_data_tx_chan_value(&msg_info, i, &signal) == AUD_SUCCESS) 
		{
			switch (signal)
			{
				case AUDIO_SIGNAL_PRESENCE_VALUE__CLIP:
					AUD_PRINTF("TX chan[%d]: %s\n", i, "CLIP");
					break;
				case AUDIO_SIGNAL_PRESENCE_VALUE__HAS_SIGNAL:
					AUD_PRINTF("TX chan[%d]: %s\n", i, "HAS SIGNAL");
					break;
				case AUDIO_SIGNAL_PRESENCE_VALUE__NO_SIGNAL:
					AUD_PRINTF("TX chan[%d]: %s\n", i, "NO SIGNAL");
					break;
				default:
					AUD_PRINTF("TX chan[%d]: %d\n", i, signal);
					break;
			}
		}
	}

	//print the RX channels
	for (i = 0; i < num_rx_chans; i++) 
	{
		if (ddp_read_audio_signal_presence_data_rx_chan_value(&msg_info, i, &signal) == AUD_SUCCESS) 
		{
			switch (signal)
			{
			case AUDIO_SIGNAL_PRESENCE_VALUE__CLIP:
				AUD_PRINTF("RX chan[%d]: %s\n", i, "CLIP");
				break;
			case AUDIO_SIGNAL_PRESENCE_VALUE__HAS_SIGNAL:
				AUD_PRINTF("RX chan[%d]: %s\n", i, "HAS SIGNAL");
				break;
			case AUDIO_SIGNAL_PRESENCE_VALUE__NO_SIGNAL:
				AUD_PRINTF("RX chan[%d]: %s\n", i, "NO SIGNAL");
				break;
			default:
				AUD_PRINTF("RX chan[%d]: %d\n", i, signal);
					break;
			}
		}
	}

	return AUD_SUCCESS;
}
#endif //HOSTCPU_API_SUPPORT_DDP_AUDIO_SIGNAL_PRESENCE

#if (defined HOSTCPU_API_SUPPORT_DDP_AUDIO_INTERFACE)
aud_error_t handle_ddp_audio_interface(ddp_packet_read_info_t *ddp_rinfo, uint16_t offset)
{
	aud_error_t result = AUD_SUCCESS;
	ddp_request_id_t request_id;
	ddp_status_t status;
	uint8_t chans_per_tdm, frame_type, align_type, chan_map_type;

	result = ddp_read_audio_interface_response(ddp_rinfo, offset, &request_id, &status, &chans_per_tdm, &frame_type, &align_type, &chan_map_type);
	if (result != AUD_SUCCESS)
	{
		return result;
	}

	AUD_PRINTF("Channels per TDM: %u\n", chans_per_tdm);
	AUD_PRINTF("Frame type: %u\n", frame_type);
	AUD_PRINTF("Align type: %u\n", align_type);
	AUD_PRINTF("Chan map type: %u\n", chan_map_type);

	return AUD_SUCCESS;
}
#endif //HOSTCPU_API_SUPPORT_DDP_AUDIO_INTERFACE

#if (defined HOSTCPU_API_SUPPORT_DDP_ROUTING_BASIC)
aud_error_t handle_ddp_routing_basic(ddp_packet_read_info_t *ddp_rinfo, uint16_t offset)
{
	AUD_UNUSED(ddp_rinfo);
	AUD_UNUSED(offset);
	return AUD_SUCCESS;
}
#endif //HOSTCPU_API_SUPPORT_DDP_ROUTING_BASIC

#if (defined HOSTCPU_API_SUPPORT_DDP_ROUTING_READY_STATE)
aud_error_t handle_ddp_routing_ready_state(ddp_packet_read_info_t *ddp_rinfo, uint16_t offset)
{
	aud_error_t result;
	ddp_request_id_t request_id;
	ddp_status_t status;
	uint8_t network_ready_state, routing_ready_state;

	result = ddp_read_routing_ready_state_response(ddp_rinfo, offset, &request_id, &status, &network_ready_state, &routing_ready_state);

	if (result != AUD_SUCCESS) 
	{
		return result;
	}

	AUD_PRINTF("Network ready state: %d\n", network_ready_state);
	AUD_PRINTF("Routing ready state: %d\n", routing_ready_state);

	hostcpu_ddp_route_ready(routing_ready_state);

	return AUD_SUCCESS;
}
#endif //HOSTCPU_API_SUPPORT_DDP_ROUTING_READY_STATE

#if (defined HOSTCPU_API_SUPPORT_DDP_ROUTING_PERF_CONFIG)
aud_error_t handle_ddp_routing_performance_config(ddp_packet_read_info_t *ddp_rinfo, uint16_t offset)
{
	aud_error_t result;
	ddp_request_id_t request_id;
	ddp_status_t status;
	uint16_t unicast_fpp, multicast_fpp;
	uint32_t unicast_latency, multicast_latency;

	result = ddp_read_routing_performance_config_response(ddp_rinfo, offset, &request_id, &status,
														&unicast_fpp, &unicast_latency, &multicast_fpp, &multicast_latency);

	if (result != AUD_SUCCESS) 
	{
		return result;
	}

	AUD_PRINTF("Unicast fpp: %d\n", unicast_fpp);
	AUD_PRINTF("Unicast latency %d \n", unicast_latency);
	AUD_PRINTF("Multicast fpp %d\n", multicast_fpp);
	AUD_PRINTF("Multicast latency %d\n", multicast_latency);

	return AUD_SUCCESS;
}
#endif

#if (defined HOSTCPU_API_SUPPORT_DDP_ROUTING_RX_CHAN_CONFIG_STATE)
aud_error_t handle_ddp_routing_rx_chan_config_state(ddp_packet_read_info_t *ddp_rinfo, uint16_t offset)
{
	aud_error_t result;
	ddp_request_id_t request_id;
	ddp_status_t status;
	uint16_t i, num_chans, num_custom_encs;
	ddp_rx_chan_state_params_t rx_chan_state;
	ddp_message_read_info_t message_info;

	result = ddp_read_routing_rx_chan_config_state_response_header(ddp_rinfo, offset, &message_info, &request_id, &status, &num_chans);
	if (result != AUD_SUCCESS)
	{		
		return result;
	}

	for (i = 0; i < num_chans; ++i)
	{
		result = ddp_read_routing_rx_chan_config_state_response_chan_params(&message_info, i, &rx_chan_state, &num_custom_encs);
		if (result != AUD_SUCCESS)
		{
			return result;
		}

		AUD_PRINTF("Channel Id %d: Flags 0x%04x ", rx_chan_state.channel_id, rx_chan_state.channel_flags);
		AUD_PRINTF("rate=%d enc=%d pcm_map=0x%04x  ", rx_chan_state.sample_rate, rx_chan_state.encoding, rx_chan_state.pcm_map);

		if (rx_chan_state.channel_name)
		{
			AUD_PRINTF(" Name=%s", rx_chan_state.channel_name);
		}
		else
		{
			AUD_PRINTF(" Name=''");
		}
		if (rx_chan_state.channel_label)
		{
			AUD_PRINTF(" Label %s", rx_chan_state.channel_label);
		}
		else
		{
			AUD_PRINTF(" Label=''");
		}

		AUD_PRINTF(" in Flow Id %d slot id %d status=0x%04x ", rx_chan_state.flow_id, rx_chan_state.slot_id, rx_chan_state.status);


		if (rx_chan_state.subscribed_channel)
		{
			AUD_PRINTF("sub=%s", rx_chan_state.subscribed_channel);
		}
		else
		{
			AUD_PRINTF("sub='' ");
		}
		if (rx_chan_state.subcribed_device)
		{
			AUD_PRINTF("@%s ", rx_chan_state.subcribed_device);
		}

		if (num_custom_encs != 0)
		{
			AUD_PRINTF("Custom Encs= ");

			uint16_t j, custom_enc = 0;
			for (j = 0; j < num_custom_encs; ++j)
			{
				result = ddp_read_routing_rx_chan_config_state_response_custom_encoding(&message_info, i, j, &custom_enc);
				if (result != AUD_SUCCESS)
				{
					return result;
				}
				AUD_PRINTF("%u ", custom_enc);
			}
		}

		AUD_PRINTF("\n");
	}

	return AUD_SUCCESS;
}
#endif //HOSTCPU_API_SUPPORT_DDP_ROUTING_RX_CHAN_CONFIG_STATE

#if (defined HOSTCPU_API_SUPPORT_DDP_ROUTING_RX_SUBSCRIBE_SET)
aud_error_t handle_ddp_routing_rx_subscribe_set(ddp_packet_read_info_t *ddp_rinfo, uint16_t offset)
{
	AUD_UNUSED(ddp_rinfo);
	AUD_UNUSED(offset);
	return AUD_SUCCESS;
}
#endif //HOSTCPU_API_SUPPORT_DDP_ROUTING_RX_SUBSCRIBE_SET

#if (defined HOSTCPU_API_SUPPORT_DDP_ROUTING_RX_CHAN_LABEL_SET)
aud_error_t handle_ddp_routing_rx_chan_label_set(ddp_packet_read_info_t *ddp_rinfo, uint16_t offset)
{
	AUD_UNUSED(ddp_rinfo);
	AUD_UNUSED(offset);
	return AUD_SUCCESS;
}
#endif //HOSTCPU_API_SUPPORT_DDP_ROUTING_RX_CHAN_LABEL_SET

#if (defined HOSTCPU_API_SUPPORT_DDP_ROUTING_TX_CHAN_CONFIG_STATE)
aud_error_t handle_ddp_routing_tx_chan_config_state(ddp_packet_read_info_t *ddp_rinfo, uint16_t offset)
{
	aud_error_t result;
	ddp_request_id_t request_id;
	ddp_status_t status;
	uint16_t num_chans, i, num_custom_encs;
	ddp_tx_chan_state_params_t tx_chan_state;
	ddp_message_read_info_t message_info;

	result = ddp_read_routing_tx_chan_config_state_response_header(ddp_rinfo, offset, &message_info, &request_id, &status, &num_chans);
	if (result != AUD_SUCCESS)
	{
		return result;
	}

	for (i = 0; i < num_chans; ++i)
	{
		result = ddp_read_routing_tx_chan_config_state_response_chan_params(&message_info, i, &tx_chan_state, &num_custom_encs);
		if (result != AUD_SUCCESS)
		{
			return result;
		}

		AUD_PRINTF("Channel Id %d: flags=0x%04x", tx_chan_state.channel_id, tx_chan_state.channel_flags);
		AUD_PRINTF(" rate=%d  enc=%d pcm map=0x%04x  ", tx_chan_state.sample_rate, tx_chan_state.encoding, tx_chan_state.pcm_map);

		if (tx_chan_state.channel_name)
		{
			AUD_PRINTF(" Name=%s", tx_chan_state.channel_name);
		}
		else
		{
			AUD_PRINTF(" Name=''");
		}
		if (tx_chan_state.channel_label)
		{
			AUD_PRINTF("Label=%s ", tx_chan_state.channel_label);
		}
		else
		{
			AUD_PRINTF("Label='' ");
		}

		if (num_custom_encs != 0)
		{
			AUD_PRINTF("Custom Encs= ");

			uint16_t j, custom_enc = 0;
			for (j = 0; j < num_custom_encs; ++j)
			{
				result = ddp_read_routing_tx_chan_config_state_response_custom_encoding(&message_info, i, j, &custom_enc);
				if (result != AUD_SUCCESS)
				{
					return result;
				}
				AUD_PRINTF("%u ", custom_enc);
			}
		}
		AUD_PRINTF("\n");
	}

	return AUD_SUCCESS;
}
#endif //HOSTCPU_API_SUPPORT_DDP_ROUTING_TX_CHAN_CONFIG_STATE

#if (defined HOSTCPU_API_SUPPORT_DDP_ROUTING_TX_CHAN_LABEL_SET)
aud_error_t handle_ddp_routing_tx_chan_label_set(ddp_packet_read_info_t *ddp_rinfo, uint16_t offset)
{
	AUD_UNUSED(ddp_rinfo);
	AUD_UNUSED(offset);
	return AUD_SUCCESS;
}
#endif //HOSTCPU_API_SUPPORT_DDP_ROUTING_TX_CHAN_LABEL_SET

#if (defined HOSTCPU_API_SUPPORT_DDP_ROUTING_RX_FLOW_CONFIG_STATE)
aud_error_t handle_ddp_routing_rx_flow_config_state(ddp_packet_read_info_t *ddp_rinfo, uint16_t offset)
{
	aud_error_t result;
	ddp_request_id_t request_id;
	ddp_status_t status;
	uint16_t i, j, num_flows;
	ddp_rx_flow_state_params_t rx_flow_state;
	uint16_t channels[4];
	ddp_message_read_info_t message_info;
	
	result = ddp_read_routing_rx_flow_config_state_response_header(ddp_rinfo, offset, &message_info, &request_id, &status, &num_flows);
	if (result != AUD_SUCCESS)
	{
		return result;
	}

	for (i = 0; i < num_flows; ++i)
	{
		uint16_t num_slots, num_addrs;

		result = ddp_read_routing_rx_flow_config_state_response_flow_params(&message_info, i, &num_slots, &num_addrs, &rx_flow_state);
		if (result != AUD_SUCCESS)
		{
			return result;
		}

		char * flow_label = "";
		if (rx_flow_state.flow_label)
		{
			flow_label = rx_flow_state.flow_label;
		}

		AUD_PRINTF(" %d: '%s' flags=%04x rate=%d enc=%u",
			rx_flow_state.flow_id, flow_label, rx_flow_state.flow_flags, rx_flow_state.sample_rate, rx_flow_state.encoding);

		AUD_PRINTF(" chans=[(%d)", num_slots);

		if (num_slots)
		{
			uint16_t num_chans;

			for (j = 0; j < num_slots; ++j)
			{
				result = ddp_read_routing_rx_flow_config_state_response_flow_slot(&message_info, i, j, &num_chans);
				if (result != AUD_SUCCESS)
				{
					return result;
				}

				if (num_chans >= 1)
				{
					uint16_t k;
					
					result = ddp_read_routing_rx_flow_config_state_response_flow_slot_chans(&message_info, i, j, channels);
					if (result != AUD_SUCCESS)
					{
						return result;
					}

					AUD_PRINTF(" {");
					for (k = 0; k < num_chans; ++k)
					{
						if (k > 0)
						{
							AUD_PRINTF(",");
						}
						AUD_PRINTF("%d", channels[k]);
					}
					AUD_PRINTF("}");
				}
				else
				{
					AUD_PRINTF(" {0}");
				}
			}
		}
		AUD_PRINTF("]");

		uint16_t port;
		ddp_ip_addr_nw_t ip_addr;

		AUD_PRINTF(" addrs=[(%d)", num_addrs);
		for (j = 0; j < num_addrs; ++j)
		{
			result = ddp_read_routing_rx_flow_config_state_response_flow_address_nw_ip(&message_info, i, j, &ip_addr, &port);
			if (result != AUD_SUCCESS)
			{
				return result;
			}

			if (ip_addr.ip_addr && port)
			{
				ip_addr.ip_addr = ntohl(ip_addr.ip_addr);
				AUD_PRINTF(" %d.%d.%d.%d:%d", (ip_addr.ip_addr >> 24) & 0xFF, (ip_addr.ip_addr >> 16) & 0xFF, (ip_addr.ip_addr >> 8) & 0xFF, (ip_addr.ip_addr & 0xFF), port);
			}
			else
			{
				AUD_PRINTF(" <none>");
			}

			AUD_PRINTF("]");
		}
		AUD_PRINTF("]");

		AUD_PRINTF(" status=%d 0x%x|0x%x l=%d user=%04x|%04x", rx_flow_state.status, rx_flow_state.avail_mask, 
			rx_flow_state.active_mask, rx_flow_state.latency, rx_flow_state.user_conf_flags, rx_flow_state.user_conf_mask);
		AUD_PRINTF("\n");
	}

	return AUD_SUCCESS;
}
#endif //HOSTCPU_API_SUPPORT_DDP_ROUTING_RX_FLOW_CONFIG_STATE

#if (defined HOSTCPU_API_SUPPORT_DDP_ROUTING_MCAST_TX_FLOW_CONFIG_SET)
aud_error_t handle_ddp_routing_multicast_tx_flow_config(ddp_packet_read_info_t *ddp_rinfo, uint16_t offset)
{
	AUD_UNUSED(ddp_rinfo);
	AUD_UNUSED(offset);
	return AUD_SUCCESS;
}
#endif //HOSTCPU_API_SUPPORT_DDP_ROUTING_MCAST_TX_FLOW_CONFIG_SET

#if (defined HOSTCPU_API_SUPPORT_DDP_ROUTING_TX_FLOW_CONFIG_STATE)
aud_error_t handle_ddp_routing_tx_flow_config_state(ddp_packet_read_info_t *ddp_rinfo, uint16_t offset)
{
	aud_error_t result;
	ddp_request_id_t request_id;
	ddp_status_t status;
	ddp_tx_flow_state_params_t tx_flow_state;
	uint16_t num_flows, i, j;
	uint16_t slots[4];
	ddp_message_read_info_t message_info;

	result = ddp_read_routing_tx_flow_config_state_response_header(ddp_rinfo, offset, &message_info, &request_id, &status, &num_flows);
	if (result != AUD_SUCCESS)
	{
		return result;
	}

	for (i = 0; i < num_flows; ++i)
	{
		uint16_t num_slots, num_addrs;

		result = ddp_read_routing_tx_flow_config_state_response_flow_params(&message_info, i, &num_slots, &num_addrs, &tx_flow_state);
		if (result != AUD_SUCCESS)
		{
			return result;
		}

		char * flow_label = "";
		if (tx_flow_state.flow_label)
		{
			flow_label = tx_flow_state.flow_label;
		}

		AUD_PRINTF(" %d: '%s' flags=%04x rate=%d enc=%u",
			tx_flow_state.flow_id, flow_label, tx_flow_state.flow_flags, tx_flow_state.sample_rate, tx_flow_state.encoding);

		AUD_PRINTF(" chans=[(%d)", num_slots);
		if (num_slots)
		{
			result = ddp_read_routing_tx_flow_config_state_response_flow_slots(&message_info, i, slots);
			if (result != AUD_SUCCESS)
			{
				return result;
			}

			for (j = 0; j < num_slots; ++j)
			{
				if (slots[j])
				{
					AUD_PRINTF(" %u", slots[j]);
				}
				else
				{
					AUD_PRINTF(" ?");
				}
			}
		}
		AUD_PRINTF("]");

		uint16_t port;
		ddp_ip_addr_nw_t ip_addr;

		AUD_PRINTF(" addrs=[(%d)", num_addrs);
		for (j = 0; j < num_addrs; ++j)
		{
			result = ddp_read_routing_tx_flow_config_state_response_flow_address_nw_ip(&message_info, i, j, &ip_addr, &port);
			if (result != AUD_SUCCESS)
			{
				return result;
			}

			if (ip_addr.ip_addr && port)
			{
				ip_addr.ip_addr = ntohl(ip_addr.ip_addr);
				AUD_PRINTF(" %d.%d.%d.%d:%d", (ip_addr.ip_addr >> 24) & 0xFF, (ip_addr.ip_addr >> 16) & 0xFF, (ip_addr.ip_addr >> 8) & 0xFF, (ip_addr.ip_addr & 0xFF), port);
			}
			else
			{
				AUD_PRINTF(" <none>");
			}

			AUD_PRINTF("]");
		}
		AUD_PRINTF("]");

		AUD_PRINTF(" active=0x%02x fpp=%d ", tx_flow_state.active_mask & tx_flow_state.avail_mask, tx_flow_state.fpp);
		if (tx_flow_state.peer_device_label && tx_flow_state.peer_flow_label)
		{
			AUD_PRINTF("to='%s@%s' ", tx_flow_state.peer_flow_label, tx_flow_state.peer_device_label);
			AUD_PRINTF("\n peer device id: process id %d device id %04x %04x user=%04x|%04x\n",
				tx_flow_state.peer_process_id, tx_flow_state.peer_device_id[0], tx_flow_state.peer_device_id[1],
				tx_flow_state.user_conf_flags, tx_flow_state.user_conf_mask);
		}
		AUD_PRINTF("\n");
	}

	return AUD_SUCCESS;
}
#endif //HOSTCPU_API_SUPPORT_DDP_ROUTING_TX_FLOW_CONFIG_STATE

#if (defined HOSTCPU_API_SUPPORT_DDP_ROUTING_RX_CHAN_STATUS)
aud_error_t handle_ddp_routing_rx_chan_status(ddp_packet_read_info_t *ddp_rinfo, uint16_t offset)
{
	aud_error_t result;
	ddp_request_id_t request_id;
	ddp_status_t status;
	uint16_t i;
	uint16_t num_chans, channel_id, chan_status;
	uint8_t avail, active;
	ddp_message_read_info_t message_info;

	result = ddp_read_routing_rx_chan_status_response_header(ddp_rinfo, offset, &message_info, &request_id, &status, &num_chans);
	if (result != AUD_SUCCESS)
	{
		return result;
	}

	for (i = 0; i < num_chans; ++i)
	{
		result = ddp_read_routing_rx_chan_status_response_chan_params(&message_info, i, &channel_id, &chan_status, &avail, &active);
		if (result != AUD_SUCCESS)
		{
			return result;
		}

		AUD_PRINTF("Channel %u status %u 0x%x|0x%x\n", channel_id, chan_status, avail, active);
	}

	return AUD_SUCCESS;
}
#endif //HOSTCPU_API_SUPPORT_DDP_ROUTING_RX_CHAN_STATUS

#if (defined HOSTCPU_API_SUPPORT_DDP_ROUTING_RX_FLOW_STATUS)
aud_error_t handle_ddp_routing_rx_flow_status(ddp_packet_read_info_t *ddp_rinfo, uint16_t offset)
{
	aud_error_t result;
	ddp_request_id_t request_id;
	ddp_status_t status;
	uint16_t i;
	uint16_t num_flows, flow_id, flow_status;
	uint8_t avail, active;
	ddp_message_read_info_t message_info;

	result = ddp_read_routing_rx_flow_status_response_header(ddp_rinfo, offset, &message_info, &request_id, &status, &num_flows);
	if (result != AUD_SUCCESS)
	{
		return result;
	}

	for (i = 0; i < num_flows; ++i)
	{
		result = ddp_read_routing_rx_chan_status_response_chan_params(&message_info, i, &flow_id, &flow_status, &avail, &active);
		if (result != AUD_SUCCESS)
		{
			return result;
		}

		AUD_PRINTF("Flow %u status %u 0x%x|0x%x\n", flow_id, flow_status, avail, active);
 		if(flow_status == 0)
		{
			dante_state = DANTE_AUDIO_DISCONNECT;
		}
		else
		{
			dante_state = DANTE_AUDIO_DETECTED;
		}  
	}

	
	
	return AUD_SUCCESS;
}
#endif //HOSTCPU_API_SUPPORT_DDP_ROUTING_RX_FLOW_STATUS

#if (defined HOSTCPU_API_SUPPORT_DDP_ROUTING_RX_UNSUB_CHAN)
aud_error_t handle_ddp_routing_rx_unsub_chan(ddp_packet_read_info_t *ddp_rinfo, uint16_t offset)
{
	AUD_UNUSED(ddp_rinfo);
	AUD_UNUSED(offset);
	return AUD_SUCCESS;
}
#endif //HOSTCPU_API_SUPPORT_DDP_ROUTING_RX_UNSUB_CHAN

#if (defined HOSTCPU_API_SUPPORT_DDP_ROUTING_FLOW_DELETE)
aud_error_t handle_ddp_routing_flow_delete(ddp_packet_read_info_t *ddp_rinfo, uint16_t offset)
{
	AUD_UNUSED(ddp_rinfo);
	AUD_UNUSED(offset);
	return AUD_SUCCESS;
}
#endif //HOSTCPU_API_SUPPORT_DDP_ROUTING_FLOW_DELETE


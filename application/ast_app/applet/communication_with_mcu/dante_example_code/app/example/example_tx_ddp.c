/*
* File     : example_tx_ddp.c
* Created  : October 2014
* Updated  : 2014/10/08
* Author   : Chamira Perera
* Synopsis : HostCPU Implementation of sending various example DDP requests
*
* This module implements the following DDP requests
* 1. Querying the Dante device's network interface state 
* 2. Configuring static IP address for a Dante device
* 3. Subscribe the Rx audio channels of a Dante device to another Dante device (transmitter, Tx)
* 4. Creating a multicast Tx for flow for a Dante device
* 5. Delete the multicast Tx flow for a Dante device created in example 4
* 6. Upgrade the application firmware for a Dante device using XMODEM over SPI
* 7. Change the audio sample rate of a Dante device
* 8. Change the clock pullup of a Dante device
* 9. Set a GPIO output of a Dante device
*
* Copyright 2005-2017, Audinate Pty Ltd and/or its licensors
* Confidential
* Audinate Copyright Header Version 1
*/

#include "example_tx_ddp.h"

#include "hostcpu_transport.h"
#include "cobs.h"

#include "ddp/audio_client.h"
#include "ddp/clocking_client.h"
#include "ddp/device_client.h"
#include "ddp/local_client.h"
#include "ddp/network_client.h"
#include "ddp/routing_client.h"

extern uint8_t tx_buffer[];
extern uint8_t cobs_tx_buffer[];

aud_error_t ddp_write_network_interface_query(uint8_t *ddp_buf, ddp_size_t *ddp_packet_len, size_t ddp_packet_buf_max, ddp_request_id_t network_if_request_id)
{
	aud_error_t result;
	ddp_packet_write_info_t ddp_winfo;

	*ddp_packet_len = 0;

	AUD_PRINTF("Sending DDP network basic request to query network interface\n");

	// Intialize the DDP packet buffer
	result = ddp_packet_init_write(&ddp_winfo, (uint32_t *)ddp_buf, (ddp_size_t)ddp_packet_buf_max);
	if (result != AUD_SUCCESS)
	{
		return result;
	}

	// Create the DDP netwok basic request message
	result = ddp_add_network_basic_request(&ddp_winfo, network_if_request_id);
	if (result != AUD_SUCCESS)
	{
		return result;
	}

	// return ddp packet length.
	*ddp_packet_len = ddp_packet_write_get_length_bytes(&ddp_winfo);

	return AUD_SUCCESS;
}

aud_error_t ddp_write_configure_static_ip(uint8_t *ddp_buf, ddp_size_t *ddp_packet_len, size_t ddp_packet_buf_max, ddp_request_id_t config_static_ip_request_id)
{
	aud_error_t result;
	ddp_packet_write_info_t ddp_winfo;
	network_ip_config_nw_ip_t ip_config;

	*ddp_packet_len = 0;

	AUD_PRINTF("Sending DDP network config request to configure a static IP address\n");

	// 2. Intialize the DDP packet buffer
	result = ddp_packet_init_write(&ddp_winfo, (uint32_t *)ddp_buf, (ddp_size_t)ddp_packet_buf_max);
	if (result != AUD_SUCCESS)
	{
		return result;
	}

	// 3. Populate the static IP parameters
	ip_config.ip_addr.ip_addr = htonl((169 << 24) | (254 << 16) | (44 << 8) | 33);	// The static IP is 169.254.44.33
	ip_config.subnet_mask.ip_addr = htonl((255 << 24) | (255 << 16) | (0 << 8) | 0);	// The subnet mask is 255.255.0.0
	ip_config.gateway.ip_addr = htonl((169 << 24) | (254 << 16) | (0 << 8) | 1);	// The gateway IP is 169.254.0.1

	// 4. Create the DDP netwok basic request message, we will not be setting a DNS and domain name
	result = ddp_add_network_config_request_nw_ip(&ddp_winfo, config_static_ip_request_id, 0, NETWORK_INTERFACE_MODE_STATIC, &ip_config, NULL, NULL);
	if (result != AUD_SUCCESS)
	{
		return result;
	}

	// return ddp packet length.
	*ddp_packet_len = ddp_packet_write_get_length_bytes(&ddp_winfo);

	return AUD_SUCCESS;
}

aud_error_t ddp_write_subscribe_channels(uint8_t *ddp_buf, ddp_size_t *ddp_packet_len, size_t ddp_packet_buf_max, ddp_request_id_t sub_chans_request_id)
{
	aud_error_t result;
	ddp_packet_write_info_t ddp_winfo;
	ddp_rx_chan_sub_params_t sub_params[2];

	*ddp_packet_len = 0;

	AUD_PRINTF("Sending a DDP routing Rx subscribe set request to subscribe channels\n");

	// Intialize the DDP packet buffer
	result = ddp_packet_init_write(&ddp_winfo, (uint32_t *)ddp_buf, (ddp_size_t)ddp_packet_buf_max);
	if (result != AUD_SUCCESS)
	{
		return result;
	}

	// Populate parameters for the subscription request
	sub_params[0].rx_chan_id = 1;
	sub_params[0].tx_chan_id = "01";
	sub_params[0].tx_device = "BKLYN-II-0615b2";
	sub_params[1].rx_chan_id = 2;
	sub_params[1].tx_chan_id = "02";
	sub_params[1].tx_device = "BKLYN-II-0615b2";

	// Create the DDP routing Rx subscribe request message
	result = ddp_add_routing_rx_sub_set_request(&ddp_winfo, sub_chans_request_id, 2, sub_params);
	if (result != AUD_SUCCESS)
	{
		return result;
	}

	// return ddp packet length.
	*ddp_packet_len = ddp_packet_write_get_length_bytes(&ddp_winfo);

	return AUD_SUCCESS;
}

aud_error_t ddp_write_create_tx_multicast_flow(uint8_t *ddp_buf, ddp_size_t *ddp_packet_len, size_t ddp_packet_buf_max, ddp_request_id_t tx_multicast_request_id)
{
	aud_error_t result;
	ddp_packet_write_info_t ddp_winfo;
	ddp_multicast_tx_flow_params_t mcast_params;
	uint16_t slots[2];

	*ddp_packet_len = 0;

	AUD_PRINTF("Sending a DDP routing multicast Tx flow configuration request to create a Tx multicast flow\n");

	// Intialize the DDP packet buffer
	result = ddp_packet_init_write(&ddp_winfo, (uint32_t *)ddp_buf, (ddp_size_t)ddp_packet_buf_max);
	if (result != AUD_SUCCESS)
	{
		return result;
	}

	// Populate the parameters for the multicast Tx
	mcast_params.encoding = DANTE_ENCODING_PCM24;
	mcast_params.flow_id = 2;
	mcast_params.fpp = 32;
	mcast_params.label = "Tx Mcast Flow";
	mcast_params.latency = 0;
	mcast_params.num_slots = 2;
	mcast_params.sample_rate = 0;

	slots[0] = 1;										// Slot 1 transmits Tx channel 1
	slots[1] = 2;										// Slot 2 transmits Tx channel 2

	// Add the slot array to the multicast Tx parameters
	ddp_routing_multicast_tx_flow_config_add_slot_params(&mcast_params, slots);

	// Create the DDP routing Rx subscribe request message
	result = ddp_add_routing_multicast_tx_flow_config_request(&ddp_winfo, tx_multicast_request_id, &mcast_params);
	if (result != AUD_SUCCESS)
	{
		return result;
	}

	// return ddp packet length.
	*ddp_packet_len = ddp_packet_write_get_length_bytes(&ddp_winfo);

	return AUD_SUCCESS;
}

aud_error_t ddp_write_delete_flow(uint8_t *ddp_buf, ddp_size_t *ddp_packet_len, size_t ddp_packet_buf_max, ddp_request_id_t flow_delete_request_id)
{
	aud_error_t result;
	ddp_packet_write_info_t ddp_winfo;

	*ddp_packet_len = 0;

	AUD_PRINTF("Sending a DDP routing flow delete request to delete a flow\n");

	// Intialize the DDP packet buffer
	result = ddp_packet_init_write(&ddp_winfo, (uint32_t *)ddp_buf, (ddp_size_t)ddp_packet_buf_max);
	if (result != AUD_SUCCESS)
	{
		return result;
	}

	// Create the DDP routing delete flow request message, the multicast Tx flow created in create_tx_multicast_flow has an ID
	// of 2. Therefore we are only deleting flow ID 2
	result = ddp_add_routing_flow_delete_request(&ddp_winfo, flow_delete_request_id, 2, 2, DDP_ROUTING_FLOW_FILTER__TX_MANUAL);
	if (result != AUD_SUCCESS)
	{
		return result;
	}

	// return ddp packet length.
	*ddp_packet_len = ddp_packet_write_get_length_bytes(&ddp_winfo);

	return AUD_SUCCESS;
}

#if defined(AUD_PLATFORM_ULTIMO)
aud_error_t ddp_write_upgrade_via_xmodem(uint8_t *ddp_buf, ddp_size_t *ddp_packet_len, size_t ddp_packet_buf_max, ddp_request_id_t upgrade_request_id)
{
	aud_error_t result;
	ddp_packet_write_info_t ddp_winfo;
	ddp_upgrade_xmodem_t xmodem_params;

	*ddp_packet_len = 0;

	AUD_PRINTF("Sending a DDP device upgrade request to upgrade the firmware on the device\n");

	// Intialize the DDP packet buffer
	result = ddp_packet_init_write(&ddp_winfo, (uint32_t *)ddp_buf, (ddp_size_t)ddp_packet_buf_max);
	if (result != AUD_SUCCESS)
	{
		return result;
	}

	// Populate the XMODEM related parameters for the upgrade
	xmodem_params.port = UPGRADE_XMODEM_PORT_SPI0;														// Use the SPI serial interface
	xmodem_params.baud_rate = DDP_SPI_BAUD_3125000;														// Dante device SPI master clock rate is 3.125 MHz
	xmodem_params.file_length = 410004;																	// File size of the firmware DNT file (typical Ultimo application DNT size)
	xmodem_params.s_params.spi[DDP_XMODEM_SPI_DEV_MASTER].polarity = DDP_SPI_CPOL_IDLE_HIGH;			// SPI master polarity of the Dante device
	xmodem_params.s_params.spi[DDP_XMODEM_SPI_DEV_MASTER].phase = DDP_SPI_CPHA_SAMPLE_TRAILING_EDGE;	// SPI master phase of the Dante device
	xmodem_params.s_params.spi[DDP_XMODEM_SPI_DEV_SLAVE].polarity = DDP_SPI_CPOL_IDLE_HIGH;				// SPI slave polarity of the Dante device
	xmodem_params.s_params.spi[DDP_XMODEM_SPI_DEV_SLAVE].phase = DDP_SPI_CPHA_SAMPLE_TRAILING_EDGE;		// SPI slave phase of the Dante device

	// Create the DDP device upgrade request message
	result = ddp_add_device_upgrade_xmodem_request(&ddp_winfo, upgrade_request_id, (UPGRADE_ACTION_DOWNLOAD | UPGRADE_ACTION_FLASH), 0, 0, NULL, NULL, &xmodem_params);
	if (result != AUD_SUCCESS)
	{
		return result;
	}

	// return ddp packet length.
	*ddp_packet_len = ddp_packet_write_get_length_bytes(&ddp_winfo);

	return AUD_SUCCESS;
}
#endif // defined(AUD_PLATFORM_ULTIMO)

aud_error_t ddp_write_configure_sample_rate(uint8_t *ddp_buf, ddp_size_t *ddp_packet_len, size_t ddp_packet_buf_max, ddp_request_id_t config_srate_request_id)
{
	aud_error_t result;
	ddp_packet_write_info_t ddp_winfo;

	*ddp_packet_len = 0;

	AUD_PRINTF("Sending a DDP audio sample rate configuration request to configure the sample rate on the device\n");

	// Intialize the DDP packet buffer
	result = ddp_packet_init_write(&ddp_winfo, (uint32_t *)ddp_buf, (ddp_size_t)ddp_packet_buf_max);
	if (result != AUD_SUCCESS)
	{
		return result;
	}

	// Create the DDP audio sample rate query request message
	result = ddp_add_audio_sample_rate_config_request(&ddp_winfo, config_srate_request_id, 0, 0);
	if (result != AUD_SUCCESS)
	{
		return result;
	}

	// return ddp packet length.
	*ddp_packet_len = ddp_packet_write_get_length_bytes(&ddp_winfo);

	return AUD_SUCCESS;
}

aud_error_t ddp_write_query_sample_rate(uint8_t *ddp_buf, ddp_size_t *ddp_packet_len, size_t ddp_packet_buf_max, ddp_request_id_t query_srate_request_id)
{
	aud_error_t result;
	ddp_packet_write_info_t ddp_winfo;

	*ddp_packet_len = 0;

	AUD_PRINTF("Sending a DDP audio sample rate query request to obtain the sample rate on the device\n");

	// Intialize the DDP packet buffer
	result = ddp_packet_init_write(&ddp_winfo, (uint32_t *)ddp_buf, (ddp_size_t)ddp_packet_buf_max);
	if (result != AUD_SUCCESS)
	{
		return result;
	}

	// Create the DDP audio sample rate query request message
	result = ddp_add_audio_sample_rate_config_request(&ddp_winfo, query_srate_request_id, 0, 0);
	if (result != AUD_SUCCESS)
	{
		return result;
	}

	// return ddp packet length.
	*ddp_packet_len = ddp_packet_write_get_length_bytes(&ddp_winfo);

	return AUD_SUCCESS;
}

aud_error_t ddp_write_configure_clock_pullup(uint8_t *ddp_buf, ddp_size_t *ddp_packet_len, size_t ddp_packet_buf_max, ddp_request_id_t config_clock_pullup_request_id)
{
	aud_error_t result;
	ddp_packet_write_info_t ddp_winfo;
	clock_pullup_control_fields_t clock_pullup_params;

	*ddp_packet_len = 0;

	AUD_PRINTF("Sending a DDP clock pullup request to configure the clock pullup on the device\n");

	// Intialize the DDP packet buffer
	result = ddp_packet_init_write(&ddp_winfo, (uint32_t *)ddp_buf, (ddp_size_t)ddp_packet_buf_max);
	if (result != AUD_SUCCESS)
	{
		return result;
	}

	// Populate the clock pullup parameters
	clock_pullup_params.valid_flags = CLOCK_PULLUP_SET_PULLUP;	// Configuring both the clock pullup and subdomain
	clock_pullup_params.pullup = CLOCK_PULLUP_POS_4_1667_PCENT;

	// Create the DDP clock pullup request message
	result = ddp_add_clock_pullup_request(&ddp_winfo, config_clock_pullup_request_id, &clock_pullup_params);
	if (result != AUD_SUCCESS)
	{
		return result;
	}

	// return ddp packet length.
	*ddp_packet_len = ddp_packet_write_get_length_bytes(&ddp_winfo);

	return AUD_SUCCESS;
}

aud_error_t ddp_write_set_gpio_output(uint8_t *ddp_buf, ddp_size_t *ddp_packet_len, size_t ddp_packet_buf_max, ddp_request_id_t gpio_request_id)
{
	aud_error_t result;
	ddp_packet_write_info_t ddp_winfo;
	uint32_t const output_mask = (0x01 << 6) | (0x01 << 5) | (0x01 << 4);
	uint32_t const output_values = (0x01 << 5) | (0x01 << 4);

	*ddp_packet_len = 0;

	AUD_PRINTF("Sending DDP GPIO control message to set GPIO outputs\n");

	// Intialize the DDP packet buffer
	result = ddp_packet_init_write(&ddp_winfo, (uint32_t *)ddp_buf, (ddp_size_t)ddp_packet_buf_max);
	if (result != AUD_SUCCESS)
	{
		return result;
	}

	// Create the DDP device GPIO request message
	// The output valid state flags are 0x70, so we are setting the values on GPIO 6, 5, 4
	// The output values are 0x30 so we are setting GPIO 6 = LOW, GPIO5 = HIGH, GPIO4 = HIGH
	result = ddp_add_device_gpio_request(&ddp_winfo, gpio_request_id, output_mask, output_values);
	if (result != AUD_SUCCESS)
	{
		return result;
	}

	// return ddp packet length.
	*ddp_packet_len = ddp_packet_write_get_length_bytes(&ddp_winfo);

	return AUD_SUCCESS;
}
#if defined(AUD_PLATFORM_ULTIMO)
aud_error_t ddp_write_turn_off_switch_leds(uint8_t *ddp_buf, ddp_size_t *ddp_packet_len, size_t ddp_packet_buf_max, ddp_request_id_t sw_led_request_id)
{
	aud_error_t result;
	ddp_packet_write_info_t ddp_winfo;

	*ddp_packet_len = 0;

	AUD_PRINTF("Sending DDP Switch LED control message to turn off Switch LEDs\n");

	// Intialize the DDP packet buffer
	result = ddp_packet_init_write(&ddp_winfo, (uint32_t *)ddp_buf, (ddp_size_t)ddp_packet_buf_max);
	if (result != AUD_SUCCESS)
	{
		return result;
	}

	// Create the DDP device Switch LED request message
	result = ddp_add_device_switch_led_request(&ddp_winfo, sw_led_request_id, SWITCH_LED_MODE_FORCE_OFF);
	if (result != AUD_SUCCESS)
	{
		return result;
	}

	// return ddp packet length.
	*ddp_packet_len = ddp_packet_write_get_length_bytes(&ddp_winfo);

	return AUD_SUCCESS;
}
#endif // defined(AUD_PLATFORM_ULTIMO)

aud_error_t ddp_write_device_general_info(uint8_t *ddp_buf, ddp_size_t *ddp_packet_len, size_t ddp_packet_buf_max, ddp_request_id_t device_general_request_id)
{
	aud_error_t result;
	ddp_packet_write_info_t ddp_winfo;

	*ddp_packet_len = 0;

	AUD_PRINTF("Sending DDP device general request to query device basic information\n");

	// Intialize the DDP packet buffer
	result = ddp_packet_init_write(&ddp_winfo, (uint32_t *)ddp_buf, (ddp_size_t)ddp_packet_buf_max);
	if (result != AUD_SUCCESS)
	{
		return result;
	}

	// Create the DDP netwok basic request message
	result = ddp_add_device_general_request(&ddp_winfo, device_general_request_id);
	if (result != AUD_SUCCESS)
	{
		return result;
	}

	// return ddp packet length.
	*ddp_packet_len = ddp_packet_write_get_length_bytes(&ddp_winfo);

	return AUD_SUCCESS;
}

/**
* Query the lock/unlock state the Dante device
* @return AUD_SUCCESS if the packet was sucessfully created and transmitted, otherwise a value other than AUD_SUCCESS is returned
*/
aud_error_t ddp_write_device_lock_unlock_query(uint8_t *ddp_buf, ddp_size_t *ddp_packet_len, size_t ddp_packet_buf_max, ddp_request_id_t lock_unlock_request_id)
{
	aud_error_t result;
	ddp_packet_write_info_t ddp_winfo;

	*ddp_packet_len = 0;

	AUD_PRINTF("Sending DDP Device Lock/Unlock query message\n");

	// Intialize the DDP packet buffer
	result = ddp_packet_init_write(&ddp_winfo, (uint32_t *)ddp_buf, (ddp_size_t)ddp_packet_buf_max);
	if (result != AUD_SUCCESS)
	{
		return result;
	}

	// Create the DDP device lock unlock request message
	result = ddp_add_device_lock_unlock_request(&ddp_winfo, lock_unlock_request_id);
	if (result != AUD_SUCCESS)
	{
		return result;
	}

	// return ddp packet length.
	*ddp_packet_len = ddp_packet_write_get_length_bytes(&ddp_winfo);

	return AUD_SUCCESS;
}

#if defined(AUD_PLATFORM_ULTIMO)
/**
* Enable audio signal presence on the Dante device
* @return AUD_SUCCESS if the packet was sucessfully created and transmitted, otherwise a value other than AUD_SUCCESS is returned
*/
aud_error_t ddp_write_enable_audio_signal_presence(uint8_t *ddp_buf, ddp_size_t *ddp_packet_len, size_t ddp_packet_buf_max, ddp_request_id_t signal_presence_config_request_id)
{
	aud_error_t result;
	ddp_packet_write_info_t ddp_winfo;

	*ddp_packet_len = 0;

	AUD_PRINTF("Sending DDP Audio Signal Presence Config control message to turn on signal presence\n");

	// Intialize the DDP packet buffer
	result = ddp_packet_init_write(&ddp_winfo, (uint32_t *)ddp_buf, (ddp_size_t)ddp_packet_buf_max);
	if (result != AUD_SUCCESS)
	{
		return result;
	}

	// Create the DDP signal presence config request message
	result = ddp_add_audio_signal_presence_config_request(&ddp_winfo, signal_presence_config_request_id, AUDIO_SIGNAL_PRESENCE_CONFIG__SET_MODE_FLAGS, AUDIO_SIGNAL_PRESENCE_MODE__ENABLED);
	if (result != AUD_SUCCESS)
	{
		return result;
	}

	// return ddp packet length.
	*ddp_packet_len = ddp_packet_write_get_length_bytes(&ddp_winfo);

	return AUD_SUCCESS;
}

#endif // defined(AUD_PLATFORM_ULTIMO)

/**
* Query the Dante Domain state the Dante device
* @return AUD_SUCCESS if the packet was sucessfully created and transmitted, otherwise a value other than AUD_SUCCESS is returned
*/
aud_error_t ddp_write_device_dante_domain_query(uint8_t *ddp_buf, ddp_size_t *ddp_packet_len, size_t ddp_packet_buf_max, ddp_request_id_t dante_domain_request_id)
{
	aud_error_t result;
	ddp_packet_write_info_t ddp_winfo;

	*ddp_packet_len = 0;

	AUD_PRINTF("Sending DDP Device Dante Domain query message\n");

	// Intialize the DDP packet buffer
	result = ddp_packet_init_write(&ddp_winfo, (uint32_t *)ddp_buf, (ddp_size_t)ddp_packet_buf_max);
	if (result != AUD_SUCCESS)
	{
		return result;
	}

	// Create the DDP device Dante Domain request message
	result = ddp_add_device_dante_domain_request(&ddp_winfo, dante_domain_request_id);
	if (result != AUD_SUCCESS)
	{
		return result;
	}

	// return ddp packet length.
	*ddp_packet_len = ddp_packet_write_get_length_bytes(&ddp_winfo);

	return AUD_SUCCESS;
}

#if defined(AUD_PLATFORM_ULTIMO)

aud_error_t ddp_write_device_switch_status_query(uint8_t *ddp_buf, ddp_size_t *ddp_packet_len, size_t ddp_packet_buf_max, ddp_request_id_t switch_status_request_id)
{
	aud_error_t result;
	ddp_packet_write_info_t ddp_winfo;

	*ddp_packet_len = 0;

	AUD_PRINTF("Sending DDP Device switch status query message\n");

	// Intialize the DDP packet buffer
	result = ddp_packet_init_write(&ddp_winfo, (uint32_t *)ddp_buf, (ddp_size_t)ddp_packet_buf_max);
	if (result != AUD_SUCCESS)
	{
		return result;
	}

	// Create the DDP device switch status request message
	result = ddp_add_device_switch_status_request(&ddp_winfo, switch_status_request_id);
	if (result != AUD_SUCCESS)
	{
		return result;
	}

	// return ddp packet length.
	*ddp_packet_len = ddp_packet_write_get_length_bytes(&ddp_winfo);

	return AUD_SUCCESS;
}

#endif // defined(AUD_PLATFORM_ULTIMO)

aud_error_t ddp_write_device_manufacturer_info(uint8_t *ddp_buf, ddp_size_t *ddp_packet_len, size_t ddp_packet_buf_max, ddp_request_id_t manufacturer_request_id)
{
	aud_error_t result;
	ddp_packet_write_info_t ddp_winfo;
	ddp_manf_override_values_t overrides = {0};

	*ddp_packet_len = 0;

	AUD_PRINTF("Sending DDP Device manufacturer message\n");

	// Intialize the DDP packet buffer
	result = ddp_packet_init_write(&ddp_winfo, (uint32_t *)ddp_buf, (ddp_size_t)ddp_packet_buf_max);
	if (result != AUD_SUCCESS)
	{
		return result;
	}

#if defined(AUD_PLATFORM_ULTIMO)
	// Set manufacturer overrride values - These overrides can be viewed in DC. To clear the overrides, reset the device
	overrides.manf_name_string = "MyTestManf";
	overrides.model_id_string = "MyTestModel";
	overrides.software_version = 10 << 24 | 20 << 16 | 30;
	overrides.software_build = 40;
	overrides.firmware_version = 50 << 24 | 60 << 16 | 70;
	overrides.firmware_build = 80;
	overrides.capability_flags = 0x00000003;
	overrides.model_version_string = "MyTestVersion";

	// Create the DDP device manufacturer request message
	result = ddp_add_device_manufacturer_request(&ddp_winfo, manufacturer_request_id, &overrides);
#else
	result = ddp_add_device_manufacturer_request(&ddp_winfo, manufacturer_request_id, NULL);
#endif // defined(AUD_PLATFORM_ULTIMO)

	if (result != AUD_SUCCESS)
	{
		return result;
	}

	// return ddp packet length.
	*ddp_packet_len = ddp_packet_write_get_length_bytes(&ddp_winfo);

	return AUD_SUCCESS;
}

/*
* File     : example_tx_ddp.h
* Created  : October 2014
* Updated  : 2014/10/08
* Author   : Chamira Perera
* Synopsis : HostCPU Implementation of various example DDP requests
*
* This module implements the following DDP requests
* 1.  Querying the Dante device's network interface state
* 2.  Configuring static IP address for a Dante device
* 3.  Subscribe the Rx audio channels of a Dante device to another Dante device (transmitter, Tx)
* 4.  Creating a multicast Tx for flow for a Dante device
* 5.  Delete the multicast Tx flow for a Dante device created in example 4
* 6.  Upgrade the application firmware for a Dante device using XMODEM over SPI
* 7.  Change the audio sample rate of a Dante device
* 8.  Change the clock pullup of a Dante device
* 9.  Set a GPIO output of a Dante device
* 10. Query the general info of a Dante device
* 11. Query the lock status of a Dante device
* 12. Enable signal presence for Ultimo devices
* 13. Query the Dante Domain status of a Dante device
* 14. Query the status of the switch of an Ultimo device
* 15. Query the manufacturer information for Brooklyn and Broadway devices and override the manufacturer info on Ultimo devices
*
* Copyright 2005-2017, Audinate Pty Ltd and/or its licensors
Confidential
* Audinate Copyright Header Version 1
*/

#ifndef __EXAMPLE_TX_DDP_H__
#define __EXAMPLE_TX_DDP_H__

#include <aud_platform.h>
#include "ddp/packet.h"
/**
* Send a request to retreive information about the network interface of the Dante device
* @return AUD_SUCCESS if the packet was sucessfully created and transmitted, otherwise a value other than AUD_SUCCESS is returned
*/
aud_error_t ddp_write_network_interface_query(uint8_t *ddp_buf, ddp_size_t *ddp_packet_len, size_t ddp_packet_buf_max, ddp_request_id_t network_if_request_id);

/**
* Configures a Dante device with a static IP address
* @return AUD_SUCCESS if the packet was sucessfully created and transmitted, otherwise a value other than AUD_SUCCESS is returned
*/
aud_error_t ddp_write_configure_static_ip(uint8_t *ddp_buf, ddp_size_t *ddp_packet_len, size_t ddp_packet_buf_max, ddp_request_id_t config_static_ip_request_id);

/**
* Susbscribe two Rx audio channels to two Tx audio channels
* @return AUD_SUCCESS if the packet was sucessfully created and transmitted, otherwise a value other than AUD_SUCCESS is returned
*/
aud_error_t ddp_write_subscribe_channels(uint8_t *ddp_buf, ddp_size_t *ddp_packet_len, size_t ddp_packet_buf_max, ddp_request_id_t sub_chans_request_id);

/**
* Create a multicast Tx flow with two slots (two Tx channels)
* @return AUD_SUCCESS if the packet was sucessfully created and transmitted, otherwise a value other than AUD_SUCCESS is returned
*/
aud_error_t ddp_write_create_tx_multicast_flow(uint8_t *ddp_buf, ddp_size_t *ddp_packet_len, size_t ddp_packet_buf_max, ddp_request_id_t tx_multicast_request_id);

/**
* Deletes the multicast Tx flow created in create_tx_multicast_flow
* @return AUD_SUCCESS if the packet was sucessfully created and transmitted, otherwise a value other than AUD_SUCCESS is returned
*/
aud_error_t ddp_write_delete_flow(uint8_t *ddp_buf, ddp_size_t *ddp_packet_len, size_t ddp_packet_buf_max, ddp_request_id_t flow_delete_request_id);

#if defined(AUD_PLATFORM_ULTIMO)

/**
* Upgrade the firmware on the Dante device using SPI and XMODEM
* @return AUD_SUCCESS if the packet was sucessfully created and transmitted, otherwise a value other than AUD_SUCCESS is returned
*/
aud_error_t ddp_write_upgrade_via_xmodem(uint8_t *ddp_buf, ddp_size_t *ddp_packet_len, size_t ddp_packet_buf_max, ddp_request_id_t upgrade_request_id);

#endif // deinfed(AUD_PLATFORM_ULTIMO)

/**
* Configures a Dante device with a new audio sampling rate
* @return AUD_SUCCESS if the packet was sucessfully created and transmitted, otherwise a value other than AUD_SUCCESS is returned
*/
aud_error_t ddp_write_configure_sample_rate(uint8_t *ddp_buf, ddp_size_t *ddp_packet_len, size_t ddp_packet_buf_max, ddp_request_id_t config_srate_request_id);

/**
* Query a current Dante device a audio sampling rate
* @return AUD_SUCCESS if the packet was sucessfully created and transmitted, otherwise a value other than AUD_SUCCESS is returned
*/
aud_error_t ddp_write_query_sample_rate(uint8_t *ddp_buf, ddp_size_t *ddp_packet_len, size_t ddp_packet_buf_max, ddp_request_id_t query_srate_request_id);

/**
* Configures a Dante device with a new clock pullup and subdomain
* @return AUD_SUCCESS if the packet was sucessfully created and transmitted, otherwise a value other than AUD_SUCCESS is returned
*/
aud_error_t ddp_write_configure_clock_pullup(uint8_t *ddp_buf, ddp_size_t *ddp_packet_len, size_t ddp_packet_buf_max, ddp_request_id_t config_clock_pullup_request_id);

/**
* Set the GPIO output Dante device to a particular state
* @return AUD_SUCCESS if the packet was sucessfully created and transmitted, otherwise a value other than AUD_SUCCESS is returned
*/
aud_error_t ddp_write_set_gpio_output(uint8_t *ddp_buf, ddp_size_t *ddp_packet_len, size_t ddp_packet_buf_max, ddp_request_id_t gpio_request_id);

#if defined(AUD_PLATFORM_ULTIMO)
/**
* Turn off Switch LEDs on the Dante device
* @return AUD_SUCCESS if the packet was sucessfully created and transmitted, otherwise a value other than AUD_SUCCESS is returned
*/
aud_error_t ddp_write_turn_off_switch_leds(uint8_t *ddp_buf, ddp_size_t *ddp_packet_len, size_t ddp_packet_buf_max, ddp_request_id_t sw_led_request_id);

#endif // defined(AUD_PLATFORM_ULTIMO)

/**
* Send a request to retreive information about Dante device general information
* @return AUD_SUCCESS if the packet was sucessfully created and transmitted, otherwise a value other than AUD_SUCCESS is returned
*/
aud_error_t ddp_write_device_general_info(uint8_t *ddp_buf, ddp_size_t *ddp_packet_len, size_t ddp_packet_buf_max, ddp_request_id_t device_general_request_id);

/**
* Query the lock/unlock state the Dante device
* @return AUD_SUCCESS if the packet was sucessfully created and transmitted, otherwise a value other than AUD_SUCCESS is returned
*/
aud_error_t ddp_write_device_lock_unlock_query(uint8_t *ddp_buf, ddp_size_t *ddp_packet_len, size_t ddp_packet_buf_max, ddp_request_id_t lock_unlock_request_id);

#if defined(AUD_PLATFORM_ULTIMO)

/**
* Enable audio signal presence on the Dante device
* @return AUD_SUCCESS if the packet was sucessfully created and transmitted, otherwise a value other than AUD_SUCCESS is returned
*/
aud_error_t ddp_write_enable_audio_signal_presence(uint8_t *ddp_buf, ddp_size_t *ddp_packet_len, size_t ddp_packet_buf_max, ddp_request_id_t signal_presence_config_request_id);

#endif // defined(AUD_PLATFORM_ULTIMO)

/**
* Query the Dante Domain state the Dante device
* @return AUD_SUCCESS if the packet was sucessfully created and transmitted, otherwise a value other than AUD_SUCCESS is returned
*/
aud_error_t ddp_write_device_dante_domain_query(uint8_t *ddp_buf, ddp_size_t *ddp_packet_len, size_t ddp_packet_buf_max, ddp_request_id_t lock_unlock_request_id);

#if defined(AUD_PLATFORM_ULTIMO)

/**
* Query the switch status state the Dante device
* @return AUD_SUCCESS if the packet was sucessfully created and transmitted, otherwise a value other than AUD_SUCCESS is returned
*/
aud_error_t ddp_write_device_switch_status_query(uint8_t *ddp_buf, ddp_size_t *ddp_packet_len, size_t ddp_packet_buf_max, ddp_request_id_t switch_status_request_id);

#endif // defined(AUD_PLATFORM_ULTIMO)

/**
* Override the manufacturer info which is supported only on the Ultimo. On the Brookly2 and Broadway the manufacturer info set in the capability is returned
* @return AUD_SUCCESS if the packet was sucessfully created and transmitted, otherwise a value other than AUD_SUCCESS is returned
*/
aud_error_t ddp_write_device_manufacturer_info(uint8_t *ddp_buf, ddp_size_t *ddp_packet_len, size_t ddp_packet_buf_max, ddp_request_id_t manufacturer_request_id);

#endif // __EXAMPLE_TX_DDP_H__

/**
+* Set the friendly name of Ultimo
+* @return AUD_SUCCESS if the packet was sucessfully created and transmitted, otherwise a value other than AUD_SUCCESS is returned
+*/
aud_error_t ddp_write_device_identity_request(uint8_t *ddp_buf, ddp_size_t *ddp_packet_len, size_t ddp_packet_buf_max, ddp_request_id_t identity_request_id, const char *friendly_name_string);

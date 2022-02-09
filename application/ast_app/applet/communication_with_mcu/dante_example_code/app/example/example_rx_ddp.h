/*
* File     : example_rx_ddp.h
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

#ifndef __EXAMPLE_RX_DDP_H__
#define __EXAMPLE_RX_DDP_H__

#include "aud_platform.h"
#include "ddp/packet.h"

#if (defined HOSTCPU_API_SUPPORT_DANTE_EVENT_AUDIO_FORMAT)
/**
* Handles the Dante Local Audio Format event
* @param ddp_rinfo [in] Pointer to a structure which has information about the DDP packet buffer, it is assumed that the ddp_packet_init_read function is called beforehand @see ddp_packet_init_read
* @param offset [in] Current offset into the packet
* @return AUD_SUCCESS if the Dante local audio format event was successfully read, otherwise a value other than AUD_SUCCESS is returned
*/
aud_error_t handle_dante_event_audio_format(ddp_packet_read_info_t *ddp_rinfo, uint16_t offset);
#endif //HOSTCPU_API_SUPPORT_DANTE_EVENT_AUDIO_FORMAT

#if (defined HOSTCPU_API_SUPPORT_DANTE_EVENT_CLOCK_PULLUP)
/**
* Handles the Dante Local Clock Pullup event
* @param ddp_rinfo [in] Pointer to a structure which has information about the DDP packet buffer, it is assumed that the ddp_packet_init_read function is called beforehand @see ddp_packet_init_read
* @param offset [in] Current offset into the packet
* @return AUD_SUCCESS if the Dante local clock pullup event was successfully read, otherwise a value other than AUD_SUCCESS is returned
*/
aud_error_t handle_dante_event_clock_pullup(ddp_packet_read_info_t *ddp_rinfo, uint16_t offset);
#endif //HOSTCPU_API_SUPPORT_DANTE_EVENT_CLOCK_PULLUP

#if (defined HOSTCPU_API_SUPPORT_DDP_DEVICE_GENERAL)
/**
* Handles the DDP device general response
* @param ddp_rinfo [in] Pointer to a structure which has information about the DDP packet buffer, it is assumed that the ddp_packet_init_read function is called beforehand @see ddp_packet_init_read
* @param offset [in] Current offset into the packet
* @return AUD_SUCCESS if the DDP device general response was successfully read, otherwise a value other than AUD_SUCCESS is returned
*/
aud_error_t handle_ddp_device_general(ddp_packet_read_info_t *ddp_rinfo, uint16_t offset);
#endif //HOSTCPU_API_SUPPORT_DDP_DEVICE_GENERAL

#if (defined HOSTCPU_API_SUPPORT_DDP_DEVICE_MANF_INFO)

/**
* Handles the DDP device manufacturer response
* @param ddp_rinfo [in] Pointer to a structure which has information about the DDP packet buffer, it is assumed that the ddp_packet_init_read function is called beforehand @see ddp_packet_init_read
* @param offset [in] Current offset into the packet
* @return AUD_SUCCESS if the DDP device manufacturer response was successfully read, otherwise a value other than AUD_SUCCESS is returned
*/
aud_error_t handle_ddp_device_manuf_info(ddp_packet_read_info_t *ddp_rinfo, uint16_t offset);
#endif //HOSTCPU_API_SUPPORT_DDP_DEVICE_MANF_INFO

#if (defined HOSTCPU_API_SUPPORT_DDP_DEVICE_UPGRADE)
/**
* Handles the DDP device upgrade response
* @param ddp_rinfo [in] Pointer to a structure which has information about the DDP packet buffer, it is assumed that the ddp_packet_init_read function is called beforehand @see ddp_packet_init_read
* @param offset [in] Current offset into the packet
* @return AUD_SUCCESS if the DDP device upgrade response was successfully read, otherwise a value other than AUD_SUCCESS is returned
*/
aud_error_t handle_ddp_device_upgrade(ddp_packet_read_info_t *ddp_rinfo, uint16_t offset);
#endif //HOSTCPU_API_SUPPORT_DDP_DEVICE_UPGRADE

#if (defined HOSTCPU_API_SUPPORT_DDP_DEVICE_ERASE_CONFIG)
/**
* Handles the DDP device erase response and event
* @param ddp_rinfo [in] Pointer to a structure which has information about the DDP packet buffer, it is assumed that the ddp_packet_init_read function is called beforehand @see ddp_packet_init_read
* @param offset [in] Current offset into the packet
* @return AUD_SUCCESS if the DDP device erase response and event was successfully read, otherwise a value other than AUD_SUCCESS is returned
*/
aud_error_t handle_ddp_device_erase_config(ddp_packet_read_info_t *ddp_rinfo, uint16_t offset);
#endif //HOSTCPU_API_SUPPORT_DDP_DEVICE_ERASE_CONFIG

#if (defined HOSTCPU_API_SUPPORT_DDP_DEVICE_REBOOT)
/**
* Handles the DDP device reboot response and event
* @param ddp_rinfo [in] Pointer to a structure which has information about the DDP packet buffer, it is assumed that the ddp_packet_init_read function is called beforehand @see ddp_packet_init_read
* @param offset [in] Current offset into the packet
* @return AUD_SUCCESS if the DDP device reboot response and event was successfully read, otherwise a value other than AUD_SUCCESS is returned
*/
aud_error_t handle_ddp_device_reboot(ddp_packet_read_info_t *ddp_rinfo, uint16_t offset);
#endif //HOSTCPU_API_SUPPORT_DDP_DEVICE_REBOOT

#if (defined HOSTCPU_API_SUPPORT_DDP_DEVICE_ID)
/**
* Handles the DDP device identity response and event
* @param ddp_rinfo [in] Pointer to a structure which has information about the DDP packet buffer, it is assumed that the ddp_packet_init_read function is called beforehand @see ddp_packet_init_read
* @param offset [in] Current offset into the packet
* @return AUD_SUCCESS if the DDP device id response and event was successfully read, otherwise a value other than AUD_SUCCESS is returned
*/
aud_error_t handle_ddp_device_id(ddp_packet_read_info_t *ddp_rinfo, uint16_t offset);
#endif //HOSTCPU_API_SUPPORT_DDP_DEVICE_ID

#if (defined HOSTCPU_API_SUPPORT_DDP_DEVICE_IDENTIFY)
/**
* Handles the DDP device identify event
* @param ddp_rinfo [in] Pointer to a structure which has information about the DDP packet buffer, it is assumed that the ddp_packet_init_read function is called beforehand @see ddp_packet_init_read
* @param offset [in] Current offset into the packet
* @return AUD_SUCCESS if the DDP device identify response and event was successfully read, otherwise a value other than AUD_SUCCESS is returned
*/
aud_error_t handle_ddp_device_identify(ddp_packet_read_info_t *ddp_rinfo, uint16_t offset);
#endif //HOSTCPU_API_SUPPORT_DDP_DEVICE_IDENTIFY

#if (defined HOSTCPU_API_SUPPORT_DDP_DEVICE_GPIO)
/**
* Handles the DDP device gpio response and event
* @param ddp_rinfo [in] Pointer to a structure which has information about the DDP packet buffer, it is assumed that the ddp_packet_init_read function is called beforehand @see ddp_packet_init_read
* @param offset [in] Current offset into the packet
* @return AUD_SUCCESS if the DDP device identify response and event was successfully read, otherwise a value other than AUD_SUCCESS is returned
*/
aud_error_t handle_ddp_device_gpio(ddp_packet_read_info_t *ddp_rinfo, uint16_t offset);
#endif //HOSTCPU_API_SUPPORT_DDP_DEVICE_GPIO

#if (defined HOSTCPU_API_SUPPORT_DDP_DEVICE_SWITCH_LED)
/**
* Handles the DDP device Switch LED response and event
* @param ddp_rinfo [in] Pointer to a structure which has information about the DDP packet buffer, it is assumed that the ddp_packet_init_read function is called beforehand @see ddp_packet_init_read
* @param offset [in] Current offset into the packet
* @return AUD_SUCCESS if the DDP device switch led response and event was successfully read, otherwise a value other than AUD_SUCCESS is returned
*/
aud_error_t handle_ddp_device_switch_led(ddp_packet_read_info_t *ddp_rinfo, uint16_t offset);
#endif //HOSTCPU_API_SUPPORT_DDP_DEVICE_SWITCH_LED

#if (defined HOSTCPU_API_SUPPORT_DDP_DEVICE_LOCK_UNLOCK)
/**
* Handles the DDP device lock unlock response and event
* @param ddp_rinfo [in] Pointer to a structure which has information about the DDP packet buffer, it is assumed that the ddp_packet_init_read function is called beforehand @see ddp_packet_init_read
* @param offset [in] Current offset into the packet
* @return AUD_SUCCESS if the DDP device lock unlock response and event was successfully read, otherwise a value other than AUD_SUCCESS is returned
*/
aud_error_t handle_ddp_device_lock_unlock(ddp_packet_read_info_t *ddp_rinfo, uint16_t offset);
#endif //HOSTCPU_API_SUPPORT_DDP_DEVICE_LOCK_UNLOCK

#if (defined HOSTCPU_API_SUPPORT_DDP_DEVICE_AES67)
/**
 * Handles the DDP device AES67 response and event
 * @param ddp_rinfo [in] Pointer to a structure which has information about the DDP packet buffer, it is assumed that the ddp_packet_init_read function is called beforehand @see ddp_packet_init_read
 * @param offset [in] Current offset into the packet
 * @return AUD_SUCCESS if the DDP device AES67 response and event was successfully read, otherwise a value other than AUD_SUCCESS is returned
 */
aud_error_t handle_ddp_device_aes67(ddp_packet_read_info_t *ddp_rinfo, uint16_t offset);
#endif //HOSTCPU_API_SUPPORT_DDP_DEVICE_AES67

#if (defined HOSTCPU_API_SUPPORT_DDP_DEVICE_SWITCH_REDUNDANCY)
/**
* Handles the DDP device switch redundancy response and event
* @param ddp_rinfo [in] Pointer to a structure which has information about the DDP packet buffer, it is assumed that the ddp_packet_init_read function is called beforehand @see ddp_packet_init_read
* @param offset [in] Current offset into the packet
* @return AUD_SUCCESS if the DDP device switch redundancy response and event was successfully read, otherwise a value other than AUD_SUCCESS is returned
*/
aud_error_t handle_ddp_device_switch_redundancy(ddp_packet_read_info_t *ddp_rinfo, uint16_t offset);
#endif //HOSTCPU_API_SUPPORT_DDP_DEVICE_SWITCH_REDUNDANCY

#if (defined HOSTCPU_API_SUPPORT_DDP_DEVICE_UART_CONFIG)
/**
* Handles the DDP device UART config response and event
* @param ddp_rinfo [in] Pointer to a structure which has information about the DDP packet buffer, it is assumed that the ddp_packet_init_read function is called beforehand @see ddp_packet_init_read
* @param offset [in] Current offset into the packet
* @return AUD_SUCCESS if the DDP device UART config response and event was successfully read, otherwise a value other than AUD_SUCCESS is returned
*/
aud_error_t handle_ddp_device_uart_config(ddp_packet_read_info_t *ddp_rinfo, uint16_t offset);
#endif //HOSTCPU_API_SUPPORT_DDP_DEVICE_UART_CONFIG

#if (defined HOSTCPU_API_SUPPORT_DDP_DEVICE_VLAN_CONFIG)
/**
* Handles the DDP device VLAN config response and event
* @param ddp_rinfo [in] Pointer to a structure which has information about the DDP packet buffer, it is assumed that the ddp_packet_init_read function is called beforehand @see ddp_packet_init_read
* @param offset [in] Current offset into the packet
* @return AUD_SUCCESS if the DDP device VLAN config response and event was successfully read, otherwise a value other than AUD_SUCCESS is returned
*/
aud_error_t handle_ddp_device_vlan_config(ddp_packet_read_info_t *ddp_rinfo, uint16_t offset);
#endif //HOSTCPU_API_SUPPORT_DDP_DEVICE_VLAN_CONFIG

#if (defined HOSTCPU_API_SUPPORT_DDP_DEVICE_METER_CONFIG)
/**
* Handles the DDP device meter response and event
* @param ddp_rinfo [in] Pointer to a structure which has information about the DDP packet buffer, it is assumed that the ddp_packet_init_read function is called beforehand @see ddp_packet_init_read
* @param offset [in] Current offset into the packet
* @return AUD_SUCCESS if the DDP device meter config response and event was successfully read, otherwise a value other than AUD_SUCCESS is returned
*/
aud_error_t handle_ddp_device_meter_config(ddp_packet_read_info_t *ddp_rinfo, uint16_t offset);
#endif //HOSTCPU_API_SUPPORT_DDP_DEVICE_METER_CONFIG

#if (defined HOSTCPU_API_SUPPORT_DDP_DEVICE_SWITCH_STATUS)
/**
* Handles the DDP device switch status response and event
* @param ddp_rinfo [in] Pointer to a structure which has information about the DDP packet buffer, it is assumed that the ddp_packet_init_read function is called beforehand @see ddp_packet_init_read
* @param offset [in] Current offset into the packet
* @return AUD_SUCCESS if the DDP device switch status response and event was successfully read, otherwise a value other than AUD_SUCCESS is returned
*/
aud_error_t handle_ddp_device_switch_status(ddp_packet_read_info_t *ddp_rinfo, uint16_t offset);
#endif //HOSTCPU_API_SUPPORT_DDP_DEVICE_SWITCH_STATUS

#if (defined HOSTCPU_API_SUPPORT_DDP_DEVICE_DANTE_DOMAIN)
/**
* Handles the DDP device Dante Domain response and event
* @param ddp_rinfo [in] Pointer to a structure which has information about the DDP packet buffer, it is assumed that the ddp_packet_init_read function is called beforehand @see ddp_packet_init_read
* @param offset [in] Current offset into the packet
* @return AUD_SUCCESS if the DDP device identify response and event was successfully read, otherwise a value other than AUD_SUCCESS is returned
*/
aud_error_t handle_ddp_device_dante_domain(ddp_packet_read_info_t *ddp_rinfo, uint16_t offset);
#endif //HOSTCPU_API_SUPPORT_DDP_DEVICE_DANTE_DOMAIN

#if (defined HOSTCPU_API_SUPPORT_DDP_NETWORK_BASIC)
/**
* Handles the DDP network basic response and event
* @param ddp_rinfo [in] Pointer to a structure which has information about the DDP packet buffer, it is assumed that the ddp_packet_init_read function is called beforehand @see ddp_packet_init_read
* @param offset [in] Current offset into the packet
* @return AUD_SUCCESS if the DDP network basic response and event was successfully read, otherwise a value other than AUD_SUCCESS is returned
*/
aud_error_t handle_ddp_network_basic(ddp_packet_read_info_t *ddp_rinfo, uint16_t offset);
#endif //HOSTCPU_API_SUPPORT_DDP_NETWORK_BASIC

#if (defined HOSTCPU_API_SUPPORT_DDP_NETWORK_CONFIG)
/**
* Handles the DDP network config response and event
* @param ddp_rinfo [in] Pointer to a structure which has information about the DDP packet buffer, it is assumed that the ddp_packet_init_read function is called beforehand @see ddp_packet_init_read
* @param offset [in] Current offset into the packet
* @return AUD_SUCCESS if the DDP network config response and event was successfully read, otherwise a value other than AUD_SUCCESS is returned
*/
aud_error_t handle_ddp_network_config(ddp_packet_read_info_t *ddp_rinfo, uint16_t offset);
#endif //HOSTCPU_API_SUPPORT_DDP_NETWORK_CONFIG

#if (defined HOSTCPU_API_SUPPORT_DDP_CLOCK_BASIC_LEGACY)
/**
* Handles the DDP clock basic legacy response and event
* @param ddp_rinfo [in] Pointer to a structure which has information about the DDP packet buffer, it is assumed that the ddp_packet_init_read function is called beforehand @see ddp_packet_init_read
* @param offset [in] Current offset into the packet
* @deprecated - use handle_ddp_clock_basic2 instead
* @return AUD_SUCCESS if the DDP clock basic legacy response and event was successfully read, otherwise a value other than AUD_SUCCESS is returned
*/
aud_error_t handle_ddp_clock_basic_legacy(ddp_packet_read_info_t *ddp_rinfo, uint16_t offset);
#endif //HOSTCPU_API_SUPPORT_DDP_CLOCK_BASIC_LEGACY

#if (defined HOSTCPU_API_SUPPORT_DDP_CLOCK_CONFIG)
/**
* Handles the DDP clock config response and event
* @param ddp_rinfo [in] Pointer to a structure which has information about the DDP packet buffer, it is assumed that the ddp_packet_init_read function is called beforehand @see ddp_packet_init_read
* @param offset [in] Current offset into the packet
* @return AUD_SUCCESS if the DDP clock config response and event was successfully read, otherwise a value other than AUD_SUCCESS is returned
*/
aud_error_t handle_ddp_clock_config(ddp_packet_read_info_t *ddp_rinfo, uint16_t offset);
#endif //HOSTCPU_API_SUPPORT_DDP_CLOCK_CONFIG

#if (defined HOSTCPU_API_SUPPORT_DDP_CLOCK_PULLUP)
/**
* Handles the DDP clock pullup response
* @param ddp_rinfo [in] Pointer to a structure which has information about the DDP packet buffer, it is assumed that the ddp_packet_init_read function is called beforehand @see ddp_packet_init_read
* @param offset [in] Current offset into the packet
* @return AUD_SUCCESS if the DDP clock pullup response was successfully read, otherwise a value other than AUD_SUCCESS is returned
*/
aud_error_t handle_ddp_clock_pullup(ddp_packet_read_info_t *ddp_rinfo, uint16_t offset);
#endif //HOSTCPU_API_SUPPORT_DDP_CLOCK_PULLUP

#if (defined HOSTCPU_API_SUPPORT_DDP_CLOCK_BASIC2)
/**
* Handles the DDP clock basic 2 response and event
* @param ddp_rinfo [in] Pointer to a structure which has information about the DDP packet buffer, it is assumed that the ddp_packet_init_read function is called beforehand @see ddp_packet_init_read
* @param offset [in] Current offset into the packet
* @return AUD_SUCCESS if the DDP clock basic 2 response and event was successfully read, otherwise a value other than AUD_SUCCESS is returned
*/
aud_error_t handle_ddp_clock_basic2(ddp_packet_read_info_t *ddp_rinfo, uint16_t offset);
#endif //HOSTCPU_API_SUPPORT_DDP_CLOCK_BASIC2

#if (defined HOSTCPU_API_SUPPORT_DDP_AUDIO_BASIC)
/**
* Handles the DDP audio basic response and event
* @param ddp_rinfo [in] Pointer to a structure which has information about the DDP packet buffer, it is assumed that the ddp_packet_init_read function is called beforehand @see ddp_packet_init_read
* @param offset [in] Current offset into the packet
* @return AUD_SUCCESS if the DDP audio basic response and event was successfully read, otherwise a value other than AUD_SUCCESS is returned
*/
aud_error_t handle_ddp_audio_basic(ddp_packet_read_info_t *ddp_rinfo, uint16_t offset);
#endif //HOSTCPU_API_SUPPORT_DDP_AUDIO_BASIC

#if (defined HOSTCPU_API_SUPPORT_DDP_AUDIO_SRATE_CONFIG)
/**
* Handles the DDP audio sample rate config response and event
* @param ddp_rinfo [in] Pointer to a structure which has information about the DDP packet buffer, it is assumed that the ddp_packet_init_read function is called beforehand @see ddp_packet_init_read
* @param offset [in] Current offset into the packet
* @return AUD_SUCCESS if the DDP audio sample rate config response and event was successfully read, otherwise a value other than AUD_SUCCESS is returned
*/
aud_error_t handle_ddp_audio_srate(ddp_packet_read_info_t *ddp_rinfo, uint16_t offset);
#endif //HOSTCPU_API_SUPPORT_DDP_AUDIO_SRATE_CONFIG

#if (defined HOSTCPU_API_SUPPORT_DDP_AUDIO_ENC_CONFIG)
/**
* Handles the DDP audio encoding config response and event
* @param ddp_rinfo [in] Pointer to a structure which has information about the DDP packet buffer, it is assumed that the ddp_packet_init_read function is called beforehand @see ddp_packet_init_read
* @param offset [in] Current offset into the packet
* @return AUD_SUCCESS if the DDP audio encoding config response and event was successfully read, otherwise a value other than AUD_SUCCESS is returned
*/
aud_error_t handle_ddp_audio_enc(ddp_packet_read_info_t *ddp_rinfo, uint16_t offset);
#endif //HOSTCPU_API_SUPPORT_DDP_AUDIO_ENC_CONFIG

#if (defined HOSTCPU_API_SUPPORT_DDP_AUDIO_SIGNAL_PRESENCE)
/**
* Handles the DDP audio signal presence config response
* @param ddp_rinfo [in] Pointer to a structure which has information about the DDP packet buffer, it is assumed that the ddp_packet_init_read function is called beforehand @see ddp_packet_init_read
* @param offset [in] Current offset into the packet
* @return AUD_SUCCESS if the DDP audio encoding config response and event was successfully read, otherwise a value other than AUD_SUCCESS is returned
*/
aud_error_t handle_ddp_audio_signal_presence_config(ddp_packet_read_info_t *ddp_rinfo, uint16_t offset);

/**
* Handles the DDP audio signal presence data event
* @param ddp_rinfo [in] Pointer to a structure which has information about the DDP packet buffer, it is assumed that the ddp_packet_init_read function is called beforehand @see ddp_packet_init_read
* @param offset [in] Current offset into the packet
* @return AUD_SUCCESS if the DDP audio encoding config response and event was successfully read, otherwise a value other than AUD_SUCCESS is returned
*/
aud_error_t handle_ddp_audio_signal_presence_data(ddp_packet_read_info_t *ddp_rinfo, uint16_t offset);
#endif //HOSTCPU_API_SUPPORT_DDP_AUDIO_SIGNAL_PRESENCE

#if (defined HOSTCPU_API_SUPPORT_DDP_AUDIO_INTERFACE)
/**
* Handles the DDP audio interface response and event
* @param ddp_rinfo [in] Pointer to a structure which has information about the DDP packet buffer, it is assumed that the ddp_packet_init_read function is called beforehand @see ddp_packet_init_read
* @param offset [in] Current offset into the packet
* @return AUD_SUCCESS if the DDP audio interface response and event was successfully read, otherwise a value other than AUD_SUCCESS is returned
*/
aud_error_t handle_ddp_audio_interface(ddp_packet_read_info_t *ddp_rinfo, uint16_t offset);
#endif //HOSTCPU_API_SUPPORT_DDP_AUDIO_INTERFACE

#if (defined HOSTCPU_API_SUPPORT_DDP_ROUTING_BASIC)
/**
* Handles the DDP routing basic response
* @param ddp_rinfo [in] Pointer to a structure which has information about the DDP packet buffer, it is assumed that the ddp_packet_init_read function is called beforehand @see ddp_packet_init_read
* @param offset [in] Current offset into the packet
* @return AUD_SUCCESS if the DDP routing basic response was successfully read, otherwise a value other than AUD_SUCCESS is returned
*/
aud_error_t handle_ddp_routing_basic(ddp_packet_read_info_t *ddp_rinfo, uint16_t offset);
#endif //HOSTCPU_API_SUPPORT_DDP_ROUTING_BASIC

#if (defined HOSTCPU_API_SUPPORT_DDP_ROUTING_READY_STATE)
/**
* Handles the DDP routing basic ready response and event
* @param ddp_rinfo [in] Pointer to a structure which has information about the DDP packet buffer, it is assumed that the ddp_packet_init_read function is called beforehand @see ddp_packet_init_read
* @param offset [in] Current offset into the packet
* @return AUD_SUCCESS if the DDP routing ready response and event was successfully read, otherwise a value other than AUD_SUCCESS is returned
*/
aud_error_t handle_ddp_routing_ready_state(ddp_packet_read_info_t *ddp_rinfo, uint16_t offset);
#endif //HOSTCPU_API_SUPPORT_DDP_ROUTING_READY_STATE

#if (defined HOSTCPU_API_SUPPORT_DDP_ROUTING_PERF_CONFIG)
/**
* Handles the DDP routing performance config response and event
* @param ddp_rinfo [in] Pointer to a structure which has information about the DDP packet buffer, it is assumed that the ddp_packet_init_read function is called beforehand @see ddp_packet_init_read
* @param offset [in] Current offset into the packet
* @return AUD_SUCCESS if the DDP routing performance config response and event was successfully read, otherwise a value other than AUD_SUCCESS is returned
*/
aud_error_t handle_ddp_routing_performance_config(ddp_packet_read_info_t *ddp_rinfo, uint16_t offset);
#endif //HOSTCPU_API_SUPPORT_DDP_ROUTING_PERF_CONFIG

#if (defined HOSTCPU_API_SUPPORT_DDP_ROUTING_RX_CHAN_CONFIG_STATE)
/**
* Handles the DDP routing Rx chan config state response and event
* @param ddp_rinfo [in] Pointer to a structure which has information about the DDP packet buffer, it is assumed that the ddp_packet_init_read function is called beforehand @see ddp_packet_init_read
* @param offset [in] Current offset into the packet
* @return AUD_SUCCESS if the DDP routing Rx chan config state response and event was successfully read, otherwise a value other than AUD_SUCCESS is returned
*/
aud_error_t handle_ddp_routing_rx_chan_config_state(ddp_packet_read_info_t *ddp_rinfo, uint16_t offset);
#endif //HOSTCPU_API_SUPPORT_DDP_ROUTING_RX_CHAN_CONFIG_STATE

#if (defined HOSTCPU_API_SUPPORT_DDP_ROUTING_RX_SUBSCRIBE_SET)
/**
* Handles the DDP routing Rx subscribe set response
* @param ddp_rinfo [in] Pointer to a structure which has information about the DDP packet buffer, it is assumed that the ddp_packet_init_read function is called beforehand @see ddp_packet_init_read
* @param offset [in] Current offset into the packet
* @return AUD_SUCCESS if the DDP routing Rx subscribe set response was successfully read, otherwise a value other than AUD_SUCCESS is returned
*/
aud_error_t handle_ddp_routing_rx_subscribe_set(ddp_packet_read_info_t *ddp_rinfo, uint16_t offset);
#endif //HOSTCPU_API_SUPPORT_DDP_ROUTING_RX_SUBSCRIBE_SET

#if (defined HOSTCPU_API_SUPPORT_DDP_ROUTING_RX_CHAN_LABEL_SET)
/**
* Handles the DDP routing Rx chan label set response
* @param ddp_rinfo [in] Pointer to a structure which has information about the DDP packet buffer, it is assumed that the ddp_packet_init_read function is called beforehand @see ddp_packet_init_read
* @param offset [in] Current offset into the packet
* @return AUD_SUCCESS if the DDP routing Rx chan label set response was successfully read, otherwise a value other than AUD_SUCCESS is returned
*/
aud_error_t handle_ddp_routing_rx_chan_label_set(ddp_packet_read_info_t *ddp_rinfo, uint16_t offset);
#endif //HOSTCPU_API_SUPPORT_DDP_ROUTING_RX_CHAN_LABEL_SET

#if (defined HOSTCPU_API_SUPPORT_DDP_ROUTING_TX_CHAN_CONFIG_STATE)
/**
* Handles the DDP routing Tx chan config state response and event
* @param ddp_rinfo [in] Pointer to a structure which has information about the DDP packet buffer, it is assumed that the ddp_packet_init_read function is called beforehand @see ddp_packet_init_read
* @param offset [in] Current offset into the packet
* @return AUD_SUCCESS if the DDP routing Tx chan config state response and event was successfully read, otherwise a value other than AUD_SUCCESS is returned
*/
aud_error_t handle_ddp_routing_tx_chan_config_state(ddp_packet_read_info_t *ddp_rinfo, uint16_t offset);
#endif //HOSTCPU_API_SUPPORT_DDP_ROUTING_TX_CHAN_CONFIG_STATE

#if (defined HOSTCPU_API_SUPPORT_DDP_ROUTING_TX_CHAN_LABEL_SET)
/**
* Handles the DDP routing Tx chan label set response
* @param ddp_rinfo [in] Pointer to a structure which has information about the DDP packet buffer, it is assumed that the ddp_packet_init_read function is called beforehand @see ddp_packet_init_read
* @param offset [in] Current offset into the packet
* @return AUD_SUCCESS if the DDP routing Tx chan label set response was successfully read, otherwise a value other than AUD_SUCCESS is returned
*/
aud_error_t handle_ddp_routing_tx_chan_label_set(ddp_packet_read_info_t *ddp_rinfo, uint16_t offset);
#endif //HOSTCPU_API_SUPPORT_DDP_ROUTING_TX_CHAN_LABEL_SET

#if (defined HOSTCPU_API_SUPPORT_DDP_ROUTING_RX_FLOW_CONFIG_STATE)
/**
* Handles the DDP routing Rx flow config state response and event
* @param ddp_rinfo [in] Pointer to a structure which has information about the DDP packet buffer, it is assumed that the ddp_packet_init_read function is called beforehand @see ddp_packet_init_read
* @param offset [in] Current offset into the packet
* @return AUD_SUCCESS if the DDP routing Rx flow config state response and event was successfully read, otherwise a value other than AUD_SUCCESS is returned
*/
aud_error_t handle_ddp_routing_rx_flow_config_state(ddp_packet_read_info_t *ddp_rinfo, uint16_t offset);
#endif //HOSTCPU_API_SUPPORT_DDP_ROUTING_RX_FLOW_CONFIG_STATE

#if (defined HOSTCPU_API_SUPPORT_DDP_ROUTING_MCAST_TX_FLOW_CONFIG_SET)
/**
* Handles the DDP routing multicast Tx flow config response
* @param ddp_rinfo [in] Pointer to a structure which has information about the DDP packet buffer, it is assumed that the ddp_packet_init_read function is called beforehand @see ddp_packet_init_read
* @param offset [in] Current offset into the packet
* @return AUD_SUCCESS if the DDP routing multicast Tx flow config response was successfully read, otherwise a value other than AUD_SUCCESS is returned
*/
aud_error_t handle_ddp_routing_multicast_tx_flow_config(ddp_packet_read_info_t *ddp_rinfo, uint16_t offset);
#endif //HOSTCPU_API_SUPPORT_DDP_ROUTING_MCAST_TX_FLOW_CONFIG_SET

#if (defined HOSTCPU_API_SUPPORT_DDP_ROUTING_TX_FLOW_CONFIG_STATE)
/**
* Handles the DDP routing Tx flow config state response and event
* @param ddp_rinfo [in] Pointer to a structure which has information about the DDP packet buffer, it is assumed that the ddp_packet_init_read function is called beforehand @see ddp_packet_init_read
* @param offset [in] Current offset into the packet
* @return AUD_SUCCESS if the DDP routing Tx flow config state response and event was successfully read, otherwise a value other than AUD_SUCCESS is returned
*/
aud_error_t handle_ddp_routing_tx_flow_config_state(ddp_packet_read_info_t *ddp_rinfo, uint16_t offset);
#endif //HOSTCPU_API_SUPPORT_DDP_ROUTING_TX_FLOW_CONFIG_STATE

#if (defined HOSTCPU_API_SUPPORT_DDP_ROUTING_RX_CHAN_STATUS)
/**
* Handles the DDP routing Rx chan status response and event
* @param ddp_rinfo [in] Pointer to a structure which has information about the DDP packet buffer, it is assumed that the ddp_packet_init_read function is called beforehand @see ddp_packet_init_read
* @param offset [in] Current offset into the packet
* @return AUD_SUCCESS if the DDP routing Rx chan status response and event was successfully read, otherwise a value other than AUD_SUCCESS is returned
*/
aud_error_t handle_ddp_routing_rx_chan_status(ddp_packet_read_info_t *ddp_rinfo, uint16_t offset);
#endif //HOSTCPU_API_SUPPORT_DDP_ROUTING_RX_CHAN_STATUS

#if (defined HOSTCPU_API_SUPPORT_DDP_ROUTING_RX_FLOW_STATUS)
/**
* Handles the DDP routing Rx flow status response and event
* @param ddp_rinfo [in] Pointer to a structure which has information about the DDP packet buffer, it is assumed that the ddp_packet_init_read function is called beforehand @see ddp_packet_init_read
* @param offset [in] Current offset into the packet
* @return AUD_SUCCESS if the DDP routing Rx flow status response and event was successfully read, otherwise a value other than AUD_SUCCESS is returned
*/
aud_error_t handle_ddp_routing_rx_flow_status(ddp_packet_read_info_t *ddp_rinfo, uint16_t offset);
#endif //HOSTCPU_API_SUPPORT_DDP_ROUTING_RX_FLOW_STATUS

#if (defined HOSTCPU_API_SUPPORT_DDP_ROUTING_RX_UNSUB_CHAN)
/**
* Handles the DDP routing Rx unsubscribe chan response
* @param ddp_rinfo [in] Pointer to a structure which has information about the DDP packet buffer, it is assumed that the ddp_packet_init_read function is called beforehand @see ddp_packet_init_read
* @param offset [in] Current offset into the packet
* @return AUD_SUCCESS if the DDP routing Rx unsubscribe chan response was successfully read, otherwise a value other than AUD_SUCCESS is returned
*/
aud_error_t handle_ddp_routing_rx_unsub_chan(ddp_packet_read_info_t *ddp_rinfo, uint16_t offset);
#endif //HOSTCPU_API_SUPPORT_DDP_ROUTING_RX_UNSUB_CHAN

#if (defined HOSTCPU_API_SUPPORT_DDP_ROUTING_FLOW_DELETE)
/**
* Handles the DDP routing flow delete response
* @param ddp_rinfo [in] Pointer to a structure which has information about the DDP packet buffer, it is assumed that the ddp_packet_init_read function is called beforehand @see ddp_packet_init_read
* @param offset [in] Current offset into the packet
* @return AUD_SUCCESS if the DDP flow delete response was successfully read, otherwise a value other than AUD_SUCCESS is returned
*/
aud_error_t handle_ddp_routing_flow_delete(ddp_packet_read_info_t *ddp_rinfo, uint16_t offset);
#endif //HOSTCPU_API_SUPPORT_DDP_ROUTING_FLOW_DELETE

#if (defined HOSTCPU_API_SUPPORT_DDP_STATUS_ERROR)
/**
* Handles a DDP status code which indicates an error
* @param ddp_rinfo [in] Pointer to a structure which has information about the DDP packet buffer, it is assumed that the ddp_packet_init_read function is called beforehand @see ddp_packet_init_read
* @param offset [in] Current offset into the packet
*/
void handle_ddp_status_error(ddp_packet_read_info_t *ddp_rinfo, uint16_t offset);
#endif // HOSTCPU_API_SUPPORT_DDP_STATUS_ERROR

#endif // __EXAMPLE_RX_DDP_H__

/*
 * File     : example_tx_uhip.c
 * Created  : October 2014
 * Updated  : 2014/10/02
 * Author   : Michael Ung, Chamira Perera
 * Synopsis : UHIP TX helper functions
 *
 * Copyright 2005-2017, Audinate Pty Ltd and/or its licensors
 * Confidential
 * Audinate Copyright Header Version 1
 */

#include "example_uhip_common.h"
#include "example_tx_uhip.h"
#include "hostcpu_transport.h"
#include "cobs.h"
#include "frame.h"
#include "uhip_structures.h"
#include "uhip_packet_write.h"
#include "uhip_hostcpu_tx_timer.h"
#include "example_tx_ddp.h"
#include "example_tx_pb.h"

// Use only one this buffer for DDP & UHIP structure wrapping. The Data is COBS encoded in-place before sending it to the Ultimo device. 
uint8_t tx_buffer[_DMA_CHUNK_ROUND(_COBS_PAD(UHIP_PACKET_SIZE_MAX) + UHIP_PACKET_SIZE_MAX + COBS_ZERO_PAIR_NUM)];

// Forward declarations
static aud_error_t send_uhip_protocol_control(UhipProtocolControlType type);


/*
 * Wrapping DDP packet in uhip packet structure & send over physical layer
 * 1. Initialize tx buffer in UHIP structure
 * 2. Creating the UHIP structure with DDP packet
 * 3. Cobs encoding UHIP + DDP packet
 * 4. Add delimeter data and align packet size in 32bytes
 * 5. Send over SPI/UART
 */
aud_error_t prepare_uhip_packet_and_send(uint8_t *tx_buf, ddp_size_t ddp_packet_len)
{
	aud_error_t result;
	uhip_packet_write_info_t uhip_winfo;
	uint8_t *ddp_tx_buffer_ptr;
	size_t cobs_encoded_size, total_data_to_send, bytes_sent;

	// Allow room to hold the start of packet delimiter and data inserted after the COBS encode
	uint8_t * tx_buf_ptr = tx_buf + 1 + _COBS_PAD(UHIP_PACKET_SIZE_MAX);

	ddp_tx_buffer_ptr = &tx_buf_ptr[sizeof(uhip_header_t)];

	// 1. Initialize the UHIP packet buffer
	result = uhip_packet_init_write(&uhip_winfo, (uint32_t *)tx_buf_ptr, UHIP_PACKET_SIZE_MAX);
	if (result != AUD_SUCCESS)
	{
		return result;
	}

	// 2. Create the UHIP packet with the DDP packet. Reusing same buffer. Pass no memory pointer and length as 0.
	result = uhip_packet_write_dante_device_protcol(&uhip_winfo, (uint32_t *)ddp_tx_buffer_ptr, ddp_packet_len);
	if (result != AUD_SUCCESS)
	{
		return result;
	}

	// 3. COBS encode the UHIP+DDP packet from step 6. Ensure to keep the first byte free in the buffer that
	// stores the COBS encoded data to add the first delimiter byte
	cobs_encoded_size = cobs_encode(tx_buf_ptr, uhip_winfo.curr_length_bytes, tx_buf + 1);

	// 4. Add the delimeter bytes to frame and add pad bytes to make the data to be sent a multiple of 32-bytes
	total_data_to_send = prepare_tx_frame(tx_buf, cobs_encoded_size);
	if (total_data_to_send == 0)
	{
		return AUD_ERR_SYSTEM;
	}

	// 5. Send the data to the Dante device
	bytes_sent = hostcpu_transport_write(tx_buf, total_data_to_send);
	if (bytes_sent != total_data_to_send)
	{
		return AUD_ERR_SYSTEM;
	}

	return AUD_SUCCESS;
}



static void tx_success()
{
	//change the state - we are now waiting on an ACK
	hostcpu_uhip_set_state(HOSTCPU_UHIP_STATE_NO_TX_WAIT_ACK);

	//start the ACK timer
	uhip_hostcpu_tx_timer_start(TX_ACK_TIMEOUT_MILLISECONDS);
}

/**
 * Handles any pending TX messages
 */
void handle_uhip_tx()
{
	ddp_size_t ddp_packet_len;
	size_t ddp_buf_max_len, ddp_start_offset;
	uint8_t *ddp_tx_buffer_ptr;

	// Allocate extra room to store the start of packet delimiter and data after the COBS encode
	ddp_start_offset = sizeof(uhip_header_t) + 1 + _COBS_PAD(UHIP_PACKET_SIZE_MAX);
	
	// ddp tx buffer pointer
	ddp_tx_buffer_ptr = &tx_buffer[ddp_start_offset];

	ddp_buf_max_len = UHIP_PACKET_SIZE_MAX - ddp_start_offset;

	//send any pending ACK's
	if (hostcpu_uhip_is_tx_flag_set(HOSTCPU_UHIP_TX_PROT_CTRL_ACK_FLAG))
	{
		AUD_PRINTF("Sending UHIP ACK\n");
		
		if (send_uhip_protocol_control(UhipProtocolControlType_ACK_SUCCESS) == AUD_SUCCESS)
		{
			hostcpu_uhip_increment_num_tx_prot_ctrl_pkts();

			//clear the flag
			hostcpu_uhip_clear_tx_flag(HOSTCPU_UHIP_TX_PROT_CTRL_ACK_FLAG);
		}
	}

	//if we are not in a normal state we can't send a non-protocl control message
	if (hostcpu_uhip_get_state() != HOSTCPU_UHIP_STATE_NORMAL) {
		return;
	}
	
	//send packets based on the tx flags bitmask
	if (hostcpu_uhip_is_tx_flag_set(HOSTCPU_UHIP_TX_CMC_PB_FLAG))
	{
		if (send_conmon_packetbridge_packet() == AUD_SUCCESS)
		{
			hostcpu_uhip_increment_num_tx_cmc_pb_pkts();

			//clear the flag
			hostcpu_uhip_clear_tx_flag(HOSTCPU_UHIP_TX_CMC_PB_FLAG);

			//change the state and start the tx timer
			tx_success();
		}
	}
	else if (hostcpu_uhip_is_tx_flag_set(HOSTCPU_UHIP_TX_UDP_PB_FLAG))
	{
		if (send_udp_packetbridge_packet() == AUD_SUCCESS)
		{
			hostcpu_uhip_increment_num_tx_udp_pb_pkts();

			//clear the flag
			hostcpu_uhip_clear_tx_flag(HOSTCPU_UHIP_TX_UDP_PB_FLAG);

			//change the state and start the tx timer
			tx_success();
		}
	}
	else if (hostcpu_uhip_is_tx_flag_set(HOSTCPU_UHIP_TX_DDP_NET_IF_QUERY_FLAG))
	{
		ddp_request_id_t network_if_request_id = 1;

		// build message
		if (ddp_write_network_interface_query(ddp_tx_buffer_ptr, &ddp_packet_len, ddp_buf_max_len, network_if_request_id) == AUD_SUCCESS)
		{
			// wrap ddp packet in uhip structure & cobs & sending over transport
			if(prepare_uhip_packet_and_send(tx_buffer, ddp_packet_len) == AUD_SUCCESS)
			{
				// remember ddp tx request id
				hostcpu_uhip_set_ddp_tx_request_id(network_if_request_id);
				//clear the flag
				hostcpu_uhip_clear_tx_flag(HOSTCPU_UHIP_TX_DDP_NET_IF_QUERY_FLAG);

				//change the state and start the tx timer
				tx_success();
			}
		}
	}
	else if (hostcpu_uhip_is_tx_flag_set(HOSTCPU_UHIP_TX_DDP_CONFIG_STATIC_IP_FLAG))
	{
		ddp_request_id_t config_static_ip_request_id = 2;

		// build message
		if (ddp_write_configure_static_ip(ddp_tx_buffer_ptr, &ddp_packet_len, ddp_buf_max_len, config_static_ip_request_id) == AUD_SUCCESS)
		{
			// wrap ddp packet in uhip structure & cobs & sending over transport
			if(prepare_uhip_packet_and_send(tx_buffer, ddp_packet_len) == AUD_SUCCESS)
			{
				// remember ddp tx request id
				hostcpu_uhip_set_ddp_tx_request_id(config_static_ip_request_id);
				//clear the flag
				hostcpu_uhip_clear_tx_flag(HOSTCPU_UHIP_TX_DDP_CONFIG_STATIC_IP_FLAG);

				//change the state and start the tx timer
				tx_success();
			}
		}
	}
	else if (hostcpu_uhip_is_tx_flag_set(HOSTCPU_UHIP_TX_DDP_SUB_RX_CHANS_FLAG))
	{
		ddp_request_id_t sub_chans_request_id = 3;

		// build message
		if (ddp_write_subscribe_channels(ddp_tx_buffer_ptr, &ddp_packet_len, ddp_buf_max_len, sub_chans_request_id) == AUD_SUCCESS)
		{
			// wrap ddp packet in uhip structure & cobs & sending over transport
			if(prepare_uhip_packet_and_send(tx_buffer, ddp_packet_len) == AUD_SUCCESS)
			{
				// remember ddp tx request id
				hostcpu_uhip_set_ddp_tx_request_id(sub_chans_request_id);
				//clear the flag
				hostcpu_uhip_clear_tx_flag(HOSTCPU_UHIP_TX_DDP_SUB_RX_CHANS_FLAG);

				//change the state and start the tx timer
				tx_success();
			}
		}
	}
	else if (hostcpu_uhip_is_tx_flag_set(HOSTCPU_UHIP_TX_DDP_ADD_MCAST_TX_FLAG))
	{
		ddp_request_id_t tx_multicast_request_id = 4;

		// build message
		if (ddp_write_create_tx_multicast_flow(ddp_tx_buffer_ptr, &ddp_packet_len, ddp_buf_max_len, tx_multicast_request_id) == AUD_SUCCESS)
		{
			// wrap ddp packet in uhip structure & cobs & sending over transport
			if(prepare_uhip_packet_and_send(tx_buffer, ddp_packet_len) == AUD_SUCCESS)
			{
				// remember ddp tx request id
				hostcpu_uhip_set_ddp_tx_request_id(tx_multicast_request_id);
				//clear the flag
				hostcpu_uhip_clear_tx_flag(HOSTCPU_UHIP_TX_DDP_ADD_MCAST_TX_FLAG);

				//change the state and start the tx timer
				tx_success();
			}
		}
	}
	else if (hostcpu_uhip_is_tx_flag_set(HOSTCPU_UHIP_TX_DDP_DELETE_FLOW_FLAG))
	{
		ddp_request_id_t flow_delete_request_id = 5;

		// build message
		if (ddp_write_delete_flow(ddp_tx_buffer_ptr, &ddp_packet_len, ddp_buf_max_len, flow_delete_request_id) == AUD_SUCCESS)
		{
			// wrap ddp packet in uhip structure & cobs & sending over transport
			if(prepare_uhip_packet_and_send(tx_buffer, ddp_packet_len) == AUD_SUCCESS)
			{
				// remember ddp tx request id
				hostcpu_uhip_set_ddp_tx_request_id(flow_delete_request_id);
				//clear the flag
				hostcpu_uhip_clear_tx_flag(HOSTCPU_UHIP_TX_DDP_DELETE_FLOW_FLAG);

				//change the state and start the tx timer
				tx_success();
			}
		}
	}
	else if (hostcpu_uhip_is_tx_flag_set(HOSTCPU_UHIP_TX_DDP_UPGRADE_XMODEM_FLAG))
	{
		ddp_request_id_t upgrade_request_id = 6;

		// build message
		if (ddp_write_upgrade_via_xmodem(ddp_tx_buffer_ptr, &ddp_packet_len, ddp_buf_max_len, upgrade_request_id) == AUD_SUCCESS)
		{
			// wrap ddp packet in uhip structure & cobs & sending over transport
			if(prepare_uhip_packet_and_send(tx_buffer, ddp_packet_len) == AUD_SUCCESS)
			{
				// remember ddp tx request id
				hostcpu_uhip_set_ddp_tx_request_id(upgrade_request_id);
				//clear the flag
				hostcpu_uhip_clear_tx_flag(HOSTCPU_UHIP_TX_DDP_UPGRADE_XMODEM_FLAG);

				//change the state and start the tx timer
				tx_success();
			}
		}
	}
	else if (hostcpu_uhip_is_tx_flag_set(HOSTCPU_UHIP_TX_DDP_CONFIG_SRATE_FLAG))
	{
		ddp_request_id_t config_srate_request_id = 7;

		// build message
		if (ddp_write_configure_sample_rate(ddp_tx_buffer_ptr, &ddp_packet_len, ddp_buf_max_len, config_srate_request_id) == AUD_SUCCESS)
		{
			// wrap ddp packet in uhip structure & cobs & sending over transport
			if(prepare_uhip_packet_and_send(tx_buffer, ddp_packet_len) == AUD_SUCCESS)
			{
				// remember ddp tx request id
				hostcpu_uhip_set_ddp_tx_request_id(config_srate_request_id);
				//clear the flag
				hostcpu_uhip_clear_tx_flag(HOSTCPU_UHIP_TX_DDP_CONFIG_SRATE_FLAG);

				//change the state and start the tx timer
				tx_success();
			}
		}
	}
	else if (hostcpu_uhip_is_tx_flag_set(HOSTCPU_UHIP_TX_DDP_CONFIG_CLOCK_PULLUP_FLAG))
	{
		ddp_request_id_t config_clock_pullup_request_id = 8;

		// build message
		if (ddp_write_configure_clock_pullup(ddp_tx_buffer_ptr, &ddp_packet_len, ddp_buf_max_len, config_clock_pullup_request_id) == AUD_SUCCESS)
		{
			// wrap ddp packet in uhip structure & cobs & sending over transport
			if(prepare_uhip_packet_and_send(tx_buffer, ddp_packet_len) == AUD_SUCCESS)
			{
				// remember ddp tx request id
				hostcpu_uhip_set_ddp_tx_request_id(config_clock_pullup_request_id);
				//clear the flag
				hostcpu_uhip_clear_tx_flag(HOSTCPU_UHIP_TX_DDP_CONFIG_CLOCK_PULLUP_FLAG);

				//change the state and start the tx timer
				tx_success();
			}
		}
	}
	else if (hostcpu_uhip_is_tx_flag_set(HOSTCPU_UHIP_TX_DDP_DEVICE_GPIO_FLAG))
	{
		ddp_request_id_t gpio_request_id = 9;

		// build message
		if (ddp_write_set_gpio_output(ddp_tx_buffer_ptr, &ddp_packet_len, ddp_buf_max_len, gpio_request_id) == AUD_SUCCESS)
		{
			// wrap ddp packet in uhip structure & cobs & sending over transport
			if(prepare_uhip_packet_and_send(tx_buffer, ddp_packet_len) == AUD_SUCCESS)
			{
				// remember ddp tx request id
				hostcpu_uhip_set_ddp_tx_request_id(gpio_request_id);
				//clear the flag
				hostcpu_uhip_clear_tx_flag(HOSTCPU_UHIP_TX_DDP_DEVICE_GPIO_FLAG);

				//change the state and start the tx timer
				tx_success();
			}
		}
	}
	else if (hostcpu_uhip_is_tx_flag_set(HOSTCPU_UHIP_TX_DDP_DEVICE_SWITCH_LED_FLAG))
	{
		ddp_request_id_t sw_led_request_id = 10;

		// build message
		if (ddp_write_turn_off_switch_leds(ddp_tx_buffer_ptr, &ddp_packet_len, ddp_buf_max_len, sw_led_request_id) == AUD_SUCCESS)
		{
			// wrap ddp packet in uhip structure & cobs & sending over transport
			if(prepare_uhip_packet_and_send(tx_buffer, ddp_packet_len) == AUD_SUCCESS)
			{
				// remember ddp tx request id
				hostcpu_uhip_set_ddp_tx_request_id(sw_led_request_id);

				//clear the flag
				hostcpu_uhip_clear_tx_flag(HOSTCPU_UHIP_TX_DDP_DEVICE_SWITCH_LED_FLAG);

				//change the state and start the tx timer
				tx_success();
			}
		}
	}
	else if (hostcpu_uhip_is_tx_flag_set(HOSTCPU_UHIP_TX_DDP_DEVICE_LOCK_UNLOCK_FLAG))
	{
		ddp_request_id_t lock_unlock_request_id = 11;

		// build message
		if (ddp_write_device_lock_unlock_query(ddp_tx_buffer_ptr, &ddp_packet_len, ddp_buf_max_len, lock_unlock_request_id) == AUD_SUCCESS)
		{
			// wrap ddp packet in uhip structure & cobs & sending over transport
			if (prepare_uhip_packet_and_send(tx_buffer, ddp_packet_len) == AUD_SUCCESS)
			{
				// remember ddp tx request id
				hostcpu_uhip_set_ddp_tx_request_id(lock_unlock_request_id);

				//clear the flag
				hostcpu_uhip_clear_tx_flag(HOSTCPU_UHIP_TX_DDP_DEVICE_LOCK_UNLOCK_FLAG);

				//change the state and start the tx timer
				tx_success();
			}
		}
	}
	else if (hostcpu_uhip_is_tx_flag_set(HOSTCPU_UHIP_TX_DDP_AUDIO_SIGNAL_PRESENCE_FLAG))
	{
		ddp_request_id_t audio_signal_presence_request_id = 12;

		// build message
		if (ddp_write_enable_audio_signal_presence(ddp_tx_buffer_ptr, &ddp_packet_len, ddp_buf_max_len, audio_signal_presence_request_id) == AUD_SUCCESS)
		{
			// wrap ddp packet in uhip structure & cobs & sending over transport
			if (prepare_uhip_packet_and_send(tx_buffer, ddp_packet_len) == AUD_SUCCESS)
			{
				// remember ddp tx request id
				hostcpu_uhip_set_ddp_tx_request_id(audio_signal_presence_request_id);

				//clear the flag
				hostcpu_uhip_clear_tx_flag(HOSTCPU_UHIP_TX_DDP_AUDIO_SIGNAL_PRESENCE_FLAG);

				//change the state and start the tx timer
				tx_success();
			}
		}
	}
	else if (hostcpu_uhip_is_tx_flag_set(HOSTCPU_UHIP_TX_DDP_DEVICE_DANTE_DOMAIN_FLAG))
	{
		ddp_request_id_t device_dante_domain_request_id = 13;

		// build message
		if (ddp_write_device_dante_domain_query(ddp_tx_buffer_ptr, &ddp_packet_len, ddp_buf_max_len, device_dante_domain_request_id) == AUD_SUCCESS)
		{
			// wrap ddp packet in uhip structure & cobs & sending over transport
			if (prepare_uhip_packet_and_send(tx_buffer, ddp_packet_len) == AUD_SUCCESS)
			{
				// remember ddp tx request id
				hostcpu_uhip_set_ddp_tx_request_id(device_dante_domain_request_id);

				//clear the flag
				hostcpu_uhip_clear_tx_flag(HOSTCPU_UHIP_TX_DDP_DEVICE_DANTE_DOMAIN_FLAG);

				//change the state and start the tx timer
				tx_success();
			}
		}
	}
	else if (hostcpu_uhip_is_tx_flag_set(HOSTCPU_UHIP_TX_DDP_DEVICE_SWITCH_STATUS_FLAG))
	{
		ddp_request_id_t device_switch_status_request_id = 14;

		// build message
		if (ddp_write_device_switch_status_query(ddp_tx_buffer_ptr, &ddp_packet_len, ddp_buf_max_len, device_switch_status_request_id) == AUD_SUCCESS)
		{
			// wrap ddp packet in uhip structure & cobs & sending over transport
			if (prepare_uhip_packet_and_send(tx_buffer, ddp_packet_len) == AUD_SUCCESS)
			{
				// remember ddp tx request id
				hostcpu_uhip_set_ddp_tx_request_id(device_switch_status_request_id);

				//clear the flag
				hostcpu_uhip_clear_tx_flag(HOSTCPU_UHIP_TX_DDP_DEVICE_SWITCH_STATUS_FLAG);

				//change the state and start the tx timer
				tx_success();
			}
		}
	}
	else if (hostcpu_uhip_is_tx_flag_set(HOSTCPU_UHIP_TX_DDP_DEVICE_MANF_OVERRIDE_FLAG))
	{
		ddp_request_id_t device_manufacturer_request_id = 15;

		// build message
		if (ddp_write_device_manufacturer_info(ddp_tx_buffer_ptr, &ddp_packet_len, ddp_buf_max_len, device_manufacturer_request_id) == AUD_SUCCESS)
		{
			// wrap ddp packet in uhip structure & cobs & sending over transport
			if (prepare_uhip_packet_and_send(tx_buffer, ddp_packet_len) == AUD_SUCCESS)
			{
				// remember ddp tx request id
				hostcpu_uhip_set_ddp_tx_request_id(device_manufacturer_request_id);

				//clear the flag
				hostcpu_uhip_clear_tx_flag(HOSTCPU_UHIP_TX_DDP_DEVICE_MANF_OVERRIDE_FLAG);

				//change the state and start the tx timer
				tx_success();
			}
		}
	}
}

/**
* Sends a UHIP protocol control message 
* @param type [in] The type of protocol control message to send @see UhipProtocolControlType
* @return AUD_SUCCESS if the packet was sucessfully created and transmitted, otherwise a value other than AUD_SUCCESS is returned
*/
static aud_error_t send_uhip_protocol_control(UhipProtocolControlType type)
{
	aud_error_t result;
	uhip_packet_write_info_t uhip_winfo;
	size_t cobs_encoded_size, total_data_to_send, bytes_sent;

	// Allocate extra room to store the start of packet delimiter and data inserted after the COBS encode
	uint8_t * ctrl_buf_ptr = tx_buffer + 1 + _COBS_PAD(UHIP_PACKET_SIZE_MAX);

	// 1. Initialize the UHIP packet buffer
	result = uhip_packet_init_write(&uhip_winfo, (uint32_t *)ctrl_buf_ptr, UHIP_PACKET_SIZE_MAX);
	if (result != AUD_SUCCESS)
	{
		return result;
	}

	// 2. Create the UHIP packet with the DDP packet created from step 3
	result = uhip_packet_write_protocol_control(&uhip_winfo, type);
	if (result != AUD_SUCCESS)
	{
		return result;
	}

	// 3. COBS encode the UHIP packet from step 2. Ensure to keep the first byte free in the buffer that
	// stores the COBS encoded data to add the first delimiter byte
	cobs_encoded_size = cobs_encode(ctrl_buf_ptr, uhip_winfo.curr_length_bytes, tx_buffer + 1);

	// 4. Add the delimeter bytes to frame and add pad bytes to make the data to be sent a multiple of 32-bytes
	total_data_to_send = prepare_tx_frame(tx_buffer, cobs_encoded_size);
	if (total_data_to_send == 0)
	{
		return AUD_ERR_SYSTEM;
	}

	// 5. Send the data to the Dante device
	bytes_sent = hostcpu_transport_write(tx_buffer, total_data_to_send);
	if (bytes_sent != total_data_to_send)
	{
		return AUD_ERR_SYSTEM;
	}

	return AUD_SUCCESS;
}

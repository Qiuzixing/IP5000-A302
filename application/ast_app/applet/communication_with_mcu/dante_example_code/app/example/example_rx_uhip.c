/*
 * File     : example_rx_uhip.h
 * Created  : October 2014
 * Updated  : 2014/10/02
 * Author   : Michael Ung
 * Synopsis : UHIP RX helper functions
 *
 * Copyright 2005-2017, Audinate Pty Ltd and/or its licensors
 * Confidential
 * Audinate Copyright Header Version 1
 */

#include "example_uhip_common.h"
#include "example_rx_uhip.h"

#include "cobs.h"
#include "ddp/packet.h"
#include "ddp/util.h"
#include "example_rx_ddp.h"
#include "frame.h"
#include "uhip_structures.h"
#include "uhip_packet_read.h"
#include "uhip_util.h"

#include "hostcpu_transport.h"
#include "uhip_hostcpu_rx_timer.h"
#include "uhip_hostcpu_tx_timer.h"

//raw rx cobs encoded buffer. This buffer is passed in to the cobs decoder to hold the cobs decoded data. We decode the cobs data in place. 
static uint8_t cobs_rx_buffer[_COBS_PAD(UHIP_PACKET_SIZE_MAX) + UHIP_PACKET_SIZE_MAX + COBS_ZERO_PAIR_NUM];

/**
* Resets the RX buffers
*/
void reset_uhip_rx_buffers(void)
{
	memset(cobs_rx_buffer, 0x00, _COBS_PAD(UHIP_PACKET_SIZE_MAX));
}

//process the rx packet
void process_cobs_rx_packet(uint8_t* cobs_rx_buffer, size_t rx_packet_size_bytes)
{
	aud_error_t result;
	size_t uncobs_size = 0;
	uhip_packet_read_info_t uhip_read_info;
	UhipPacketType uhip_packet_type;

	uint8_t * rx_buffer = cobs_rx_buffer;

	//uncobs the packet
	uncobs_size = cobs_decode(cobs_rx_buffer, rx_packet_size_bytes, rx_buffer, UHIP_PACKET_SIZE_MAX);

	//check for a corrupted packet
	if (uncobs_size == 0) 
	{
		hostcpu_uhip_increment_num_rx_cobs_decode_errs();
		AUD_PRINTF("ERROR COBS decode error - discarding packet!\n");
		goto l__cleanup_return;
	}

	//read the UHIP header
	result = uhip_packet_init_read(&uhip_read_info, (uint32_t *)rx_buffer, (uint16_t)uncobs_size);
	if (result != AUD_SUCCESS) 
	{
		hostcpu_uhip_increment_num_rx_malformed_errs();
		AUD_PRINTF("ERROR UHIP malformed [uhip_packet_init_read()] - discarding packet!\n");
		goto l__cleanup_return;
	}

	//parse based on packet type
	result = uhip_packet_read_get_type(&uhip_read_info, &uhip_packet_type);
	if (result != AUD_SUCCESS) 
	{
		hostcpu_uhip_increment_num_rx_malformed_errs();
		AUD_PRINTF("ERROR UHIP malformed [uhip_packet_read_get_type()] - discarding packet!\n");
		goto l__cleanup_return;
	}

	switch (uhip_packet_type)
	{
		case UhipPacketType_PROTOCOL_CONTROL:
		{
			UhipProtocolControlType uhip_prot_control_type;

			//parse the protocol control packet
			if (uhip_packet_read_protocol_control(&uhip_read_info, &uhip_prot_control_type) == AUD_SUCCESS) 
			{
				hostcpu_uhip_increment_num_rx_prot_ctrl_pkts();
				AUD_PRINTF("Received UHIP Protocol Control type=%s\n\n", uhip_protocol_control_type_to_string(uhip_prot_control_type));

				switch (uhip_prot_control_type)
				{
					case UhipProtocolControlType_ACK_SUCCESS:
					case UhipProtocolControlType_ERROR_OTHER:
					case UhipProtocolControlType_ERROR_MALFORMED_PACKET:
					case UhipProtocolControlType_ERROR_UNKNOWN_TYPE:
					case UhipProtocolControlType_ERROR_NETWORK:
					case UhipProtocolControlType_ERROR_TIMEOUT:
					{
						//stop the tx timer as we have received an ACK and set the UHIP state back to normal
						uhip_hostcpu_tx_timer_stop();
						hostcpu_uhip_set_state(HOSTCPU_UHIP_STATE_NORMAL);
					}
					break;
					case UhipProtocolControlType_MAX:
					default:
					break;
				}
			}
		}
		break;

		case UhipPacketType_CONMON_PACKET_BRIDGE:
		{
			//trigger an ACK
			hostcpu_uhip_set_tx_flag(HOSTCPU_UHIP_TX_PROT_CTRL_ACK_FLAG);

#if (defined HOSTCPU_API_SUPPORT_PACKETBRIDGE_CONMON)
			UhipConMonPacketBridgeChannel cmc_pb_chan;
			uint8_t* payload;
			uint16_t payload_length;
			int i;

			//parse the packet
			if (uhip_packet_read_cmc_packet_bridge(&uhip_read_info, &cmc_pb_chan, &payload, &payload_length) == AUD_SUCCESS)
			{
				hostcpu_uhip_increment_num_rx_cmc_pb_pkts();

				//print header
				AUD_PRINTF
				(
					"Received ConMon Packet Bridge Packet chan=%s payload_length=%u payload=\n\n", 
					uhip_cmc_pb_chan_to_string(cmc_pb_chan), 
					payload_length
				);

				//print payload in hex ascii
				for (i = 0; i<payload_length; i += 16)
				{
					int j;
					for (j = 0; j<16; j++)
					{
						if ((i + j) < payload_length)
						{
							AUD_PRINTF("[%.2x] ", payload[i + j]);
						}
					}
					AUD_PRINTF("\n");
				}
			}
#endif //#if (defined HOSTCPU_API_SUPPORT_PACKETBRIDGE_CONMON)
		}
		break;

		case UhipPacketType_UDP_PACKET_BRIDGE:
		{
			//trigger an ACK
			hostcpu_uhip_set_tx_flag(HOSTCPU_UHIP_TX_PROT_CTRL_ACK_FLAG);

#if (defined HOSTCPU_API_SUPPORT_PACKETBRIDGE_UDP)
			UhipUDPPacketBridgeChannel udp_pb_chan;
			uint16_t remote_port;
			uint32_t remote_ip;
			uint8_t* payload;
			uint16_t payload_length;
			int i;

			//parse the packet
			if (uhip_packet_read_udp_packet_bridge(&uhip_read_info, &udp_pb_chan, &remote_port, &remote_ip, &payload, &payload_length) == AUD_SUCCESS)
			{
				hostcpu_uhip_increment_num_rx_udp_pb_pkts();

				//print header
				AUD_PRINTF
				(
					"Received UDP Packet Bridge Packet chan=%s remote_port=%u remote_ip=%u.%u.%u.%u payload_length=%u payload=\n\n", 
					uhip_udp_pb_chan_to_string(udp_pb_chan),
					remote_port,
					(remote_ip & 0xFF000000) >> 24,
					(remote_ip & 0x00FF0000) >> 16,
					(remote_ip & 0x0000FF00) >> 8,
					(remote_ip & 0x000000FF),
					payload_length
				);

				//print payload in hex ascii
				for (i = 0; i<payload_length; i += 16)
				{
					int j;
					for (j = 0; j<16; j++)
					{
						if ((i + j) < payload_length)
						{
							AUD_PRINTF("[%.2x] ", payload[i + j]);
						}
					}
					AUD_PRINTF("\n");
				}
			}
#endif //#if (defined HOSTCPU_API_SUPPORT_PACKETBRIDGE_UDP)
		}
		break;

		case UhipPacketType_DANTE_EVENTS:
		{
			//trigger an ACK
			hostcpu_uhip_set_tx_flag(HOSTCPU_UHIP_TX_PROT_CTRL_ACK_FLAG);

			//parse the Dante Event message
#if ( (defined HOSTCPU_API_SUPPORT_DANTE_EVENT_AUDIO_FORMAT) || (defined HOSTCPU_API_SUPPORT_DANTE_EVENT_CLOCK_PULLUP) )
			ddp_packet_read_info_t ddp_rinfo;
			ddp_message_read_info_t ddp_msg_info;
			uint16_t packet_offset = 0;

			result = ddp_packet_init_read(&ddp_rinfo, (uint32_t *)(rx_buffer + sizeof(uhip_header_t)+8), // sizeof(ddp_packet_header_t)),
											uncobs_size - sizeof(uhip_header_t));
			if (result != AUD_SUCCESS)
			{
				goto l__cleanup_return;
			}
			hostcpu_uhip_increment_num_rx_dante_evnt_pkts();
			result = ddp_packet_read_message(&ddp_rinfo, &ddp_msg_info, packet_offset);

			//read every TLV block in the packet
			while (result == AUD_SUCCESS)
			{
				AUD_PRINTF("Received Dante Event %s\n\n", ddp_opcode_to_string(ddp_msg_info.opcode));

				switch (ddp_msg_info.opcode)
				{
#if (defined HOSTCPU_API_SUPPORT_DANTE_EVENT_AUDIO_FORMAT)
					case DDP_OP_LOCAL_AUDIO_FORMAT:
						result = handle_dante_event_audio_format(&ddp_rinfo, packet_offset);
						break;
#endif //HOSTCPU_API_SUPPORT_DANTE_EVENT_AUDIO_FORMAT
#if (defined HOSTCPU_API_SUPPORT_DANTE_EVENT_CLOCK_PULLUP)
					case DDP_OP_LOCAL_CLOCK_PULLUP:
						result = handle_dante_event_clock_pullup(&ddp_rinfo, packet_offset);
						break;
#endif //HOSTCPU_API_SUPPORT_DANTE_EVENT_CLOCK_PULLUP
					default:
						AUD_PRINTF("Unsupported / Unknown Dante Event: opcode=0x%04x\n", ddp_msg_info.opcode);
						break;
				}//switch (ddp_msg_info.opcode)

				//jump to the next TLV block
				packet_offset += ddp_msg_info.length_bytes;
				result = ddp_packet_read_message(&ddp_rinfo, &ddp_msg_info, packet_offset);
			} //while (result == AUD_SUCCESS)
#endif //HOSTCPU_API_SUPPORT_DANTE_EVENT_AUDIO_FORMAT  || HOSTCPU_API_SUPPORT_DANTE_EVENT_CLOCK_PULLUP
		}
		break;
		
		case UhipPacketType_DDP:
		{
			ddp_packet_read_info_t ddp_rinfo;
			ddp_message_read_info_t ddp_msg_info;
			ddp_request_id_t response_request_id;
			ddp_status_t response_status;
			uint16_t packet_offset = 0;
			ddp_block_header_t block_header;

			//trigger an ACK
			hostcpu_uhip_set_tx_flag(HOSTCPU_UHIP_TX_PROT_CTRL_ACK_FLAG);

			//parse the DDP message
			result = ddp_packet_init_read(&ddp_rinfo, (uint32_t *)(rx_buffer + sizeof(uhip_header_t)), 
											uncobs_size - sizeof(uhip_header_t));
			
			if (result != AUD_SUCCESS)
			{
				goto l__cleanup_return;
			}

			result = ddp_packet_read_block_header(&ddp_rinfo, packet_offset, &block_header);
			if (result != AUD_SUCCESS)
			{
				AUD_PRINTF("Failed to read block header\n");
				goto l__cleanup_return;
			}
			if (block_header._.opcode != DDP_OP_PACKET_HEADER)
			{
				AUD_PRINTF("Unexpected opcode: 0x%04X in the block header, expected 0x0001\n", block_header._.opcode);
				goto l__cleanup_return;
			}

			hostcpu_uhip_increment_num_rx_ddp_pkts();

			// Jump past the block header to the start of the message in this DDP packet
			packet_offset += block_header._.length_bytes;
			result = ddp_packet_read_message(&ddp_rinfo, &ddp_msg_info, packet_offset);

			//read every TLV block in the packet
			while (result == AUD_SUCCESS)
			{
				//check the status for any errors
				response_status = ddp_packet_read_response_status(&ddp_msg_info);
				if (response_status != DDP_STATUS_NOERROR) 
				{
					AUD_PRINTF("ERROR DDP message received with status %s for opcode %s - discarding packet\n", ddp_status_to_string(response_status), ddp_opcode_to_string(ddp_msg_info.opcode));
#if (defined HOSTCPU_API_SUPPORT_DDP_STATUS_ERROR)
					handle_ddp_status_error(&ddp_rinfo, packet_offset);
#endif //HOSTCPU_API_SUPPORT_DDP_STATUS_ERROR
					goto l__cleanup_return;
				}

				//read the request id
				response_request_id = ddp_packet_read_response_request_id(&ddp_msg_info);
				AUD_PRINTF("Received %s for %s\n\n", (response_request_id) ? "response" : "event", ddp_opcode_to_string(ddp_msg_info.opcode));
				if (response_request_id)
				{
					if (response_request_id != hostcpu_uhip_get_ddp_tx_request_id())
					{
						AUD_PRINTF("ERROR DDP request ID in response does not match with request message - discarding packet\n");
						goto l__cleanup_return;
					}

					hostcpu_uhip_set_ddp_tx_request_id(0);
				}

				switch (ddp_msg_info.opcode)
				{
#if (defined HOSTCPU_API_SUPPORT_DDP_DEVICE_GENERAL)
				case DDP_OP_DEVICE_GENERAL:
					result = handle_ddp_device_general(&ddp_rinfo, packet_offset);
					break;
#endif //HOSTCPU_API_SUPPORT_DDP_DEVICE_GENERAL
#if (defined HOSTCPU_API_SUPPORT_DDP_DEVICE_MANF_INFO)
				case DDP_OP_DEVICE_MANF:
					result = handle_ddp_device_manuf_info(&ddp_rinfo, packet_offset);
					break;
#endif //HOSTCPU_API_SUPPORT_DDP_DEVICE_MANF_INFO
#if (defined HOSTCPU_API_SUPPORT_DDP_DEVICE_UPGRADE)
				case DDP_OP_DEVICE_UPGRADE:
					result = handle_ddp_device_upgrade(&ddp_rinfo, packet_offset);
					break;
#endif //HOSTCPU_API_SUPPORT_DDP_DEVICE_UPGRADE
#if (defined HOSTCPU_API_SUPPORT_DDP_DEVICE_ERASE_CONFIG)
				case DDP_OP_DEVICE_ERASE_CONFIG:
					result = handle_ddp_device_erase_config(&ddp_rinfo, packet_offset);
					break;
#endif //HOSTCPU_API_SUPPORT_DDP_DEVICE_ERASE_CONFIG
#if (defined HOSTCPU_API_SUPPORT_DDP_DEVICE_REBOOT)
				case DDP_OP_DEVICE_REBOOT:
					result = handle_ddp_device_reboot(&ddp_rinfo, packet_offset);
					break;
#endif //HOSTCPU_API_SUPPORT_DDP_DEVICE_REBOOT
#if (defined HOSTCPU_API_SUPPORT_DDP_DEVICE_ID)
				case DDP_OP_DEVICE_IDENTITY:
					result = handle_ddp_device_id(&ddp_rinfo, packet_offset);
					break;
#endif //HOSTCPU_API_SUPPORT_DDP_DEVICE_ID
#if (defined HOSTCPU_API_SUPPORT_DDP_DEVICE_IDENTIFY)
				case DDP_OP_DEVICE_IDENTIFY:
					result = handle_ddp_device_identify(&ddp_rinfo, packet_offset);
					break;
#endif //HOSTCPU_API_SUPPORT_DDP_DEVICE_IDENTIFY
#if (defined HOSTCPU_API_SUPPORT_DDP_DEVICE_GPIO)
				case DDP_OP_DEVICE_GPIO:
					result = handle_ddp_device_gpio(&ddp_rinfo, packet_offset);
					break;
#endif //HOSTCPU_API_SUPPORT_DDP_DEVICE_GPIO
#if (defined HOSTCPU_API_SUPPORT_DDP_DEVICE_SWITCH_LED)
				case DDP_OP_DEVICE_SWITCH_LED:
					result = handle_ddp_device_switch_led(&ddp_rinfo, packet_offset);
					break;
#endif //HOSTCPU_API_SUPPORT_DDP_DEVICE_SWITCH_LED
#if (defined HOSTCPU_API_SUPPORT_DDP_DEVICE_LOCK_UNLOCK)
				case DDP_OP_DEVICE_LOCK_UNLOCK:
					result = handle_ddp_device_lock_unlock(&ddp_rinfo, packet_offset);
					break;
#endif //HOSTCPU_API_SUPPORT_DDP_DEVICE_LOCK_UNLOCK

#if (defined HOSTCPU_API_SUPPORT_DDP_DEVICE_VLAN_CONFIG)
				case DDP_OP_DEVICE_VLAN_CONFIG:
					result = handle_ddp_device_vlan_config(&ddp_rinfo, packet_offset);
					break;
#endif // HOSTCPU_API_SUPPORT_DDP_DEVICE_VLAN_CONFIG

#if (defined HOSTCPU_API_SUPPORT_DDP_DEVICE_DANTE_DOMAIN)
				case DDP_OP_DEVICE_DANTE_DOMAIN:
					result = handle_ddp_device_dante_domain(&ddp_rinfo, packet_offset);
					break;
#endif // HOSTCPU_API_SUPPORT_DDP_DEVICE_DANTE_DOMAIN

#if (defined HOSTCPU_API_SUPPORT_DDP_DEVICE_SWITCH_STATUS)
				case DDP_OP_DEVICE_SWITCH_STATUS:
					result = handle_ddp_device_switch_status(&ddp_rinfo, packet_offset);
					break;
#endif // HOSTCPU_API_SUPPORT_DDP_DEVICE_SWITCH_STATUS

#if (defined HOSTCPU_API_SUPPORT_DDP_DEVICE_AES67)
				case DDP_OP_DEVICE_AES67:
					result = handle_ddp_device_aes67(&ddp_rinfo, packet_offset);
					break;
#endif // HOSTCPU_API_SUPPORT_DDP_DEVICE_AES67

#if (defined HOSTCPU_API_SUPPORT_DDP_NETWORK_BASIC)
				case DDP_OP_NETWORK_BASIC:
					result = handle_ddp_network_basic(&ddp_rinfo, packet_offset);
					break;
#endif //HOSTCPU_API_SUPPORT_DDP_NETWORK_BASIC
#if (defined HOSTCPU_API_SUPPORT_DDP_NETWORK_CONFIG)
				case DDP_OP_NETWORK_CONFIG:
					result = handle_ddp_network_config(&ddp_rinfo, packet_offset);
					break;
#endif //HOSTCPU_API_SUPPORT_DDP_NETWORK_CONFIG

#if (defined HOSTCPU_API_SUPPORT_DDP_CLOCK_BASIC_LEGACY)
				case DDP_OP_CLOCK_BASIC_LEGACY:
					result = handle_ddp_clock_basic_legacy(&ddp_rinfo, packet_offset);
					break;
#endif //HOSTCPU_API_SUPPORT_DDP_CLOCK_BASIC_LEGACY
#if (defined HOSTCPU_API_SUPPORT_DDP_CLOCK_CONFIG)
				case DDP_OP_CLOCK_CONFIG:
					result = handle_ddp_clock_config(&ddp_rinfo, packet_offset);
					break;
#endif //UHIP_HOSTCPU_API_SUPPORT_DDP_CLOCK_CONFIG
#if (defined HOSTCPU_API_SUPPORT_DDP_CLOCK_PULLUP)
				case DDP_OP_CLOCK_PULLUP:
					result = handle_ddp_clock_pullup(&ddp_rinfo, packet_offset);
					break;
#endif //UHIP_HOSTCPU_API_SUPPORT_DDP_CLOCK_PULLUP
#if (defined HOSTCPU_API_SUPPORT_DDP_CLOCK_BASIC2)
				case DDP_OP_CLOCK_BASIC2:
					result = handle_ddp_clock_basic2(&ddp_rinfo, packet_offset);
					break;
#endif //HOSTCPU_API_SUPPORT_DDP_CLOCK_BASIC2

#if (defined HOSTCPU_API_SUPPORT_DDP_AUDIO_BASIC)
				case DDP_OP_AUDIO_BASIC:
					result = handle_ddp_audio_basic(&ddp_rinfo, packet_offset);
					break;
#endif //HOSTCPU_API_SUPPORT_DDP_AUDIO_BASIC
#if (defined HOSTCPU_API_SUPPORT_DDP_AUDIO_SRATE_CONFIG)
				case DDP_OP_AUDIO_SRATE_CONFIG:
					result = handle_ddp_audio_srate(&ddp_rinfo, packet_offset);
					break;
#endif //HOSTCPU_API_SUPPORT_DDP_AUDIO_SRATE_CONFIG
#if (defined HOSTCPU_API_SUPPORT_DDP_AUDIO_ENC_CONFIG)
				case DDP_OP_AUDIO_ENC_CONFIG:
					result = handle_ddp_audio_enc(&ddp_rinfo, packet_offset);
					break;
#endif //HOSTCPU_API_SUPPORT_DDP_AUDIO_ENC_CONFIG
#if (defined HOSTCPU_API_SUPPORT_DDP_AUDIO_SIGNAL_PRESENCE)
				case DDP_OP_AUDIO_SIGNAL_PRESENCE_CONFIG:
					result = handle_ddp_audio_signal_presence_config(&ddp_rinfo, packet_offset);
					break;
				case DDP_OP_AUDIO_SIGNAL_PRESENCE_DATA:
					result = handle_ddp_audio_signal_presence_data(&ddp_rinfo, packet_offset);
					break;
#endif //HOSTCPU_API_SUPPORT_DDP_AUDIO_SIGNAL_PRESENCE

#if (defined HOSTCPU_API_SUPPORT_DDP_ROUTING_BASIC)
				case DDP_OP_ROUTING_BASIC:
					result = handle_ddp_routing_basic(&ddp_rinfo, packet_offset);
					break;
#endif //HOSTCPU_API_SUPPORT_DDP_ROUTING_BASIC
#if (defined HOSTCPU_API_SUPPORT_DDP_ROUTING_READY_STATE)
				case DDP_OP_ROUTING_READY_STATE:
					result = handle_ddp_routing_ready_state(&ddp_rinfo, packet_offset);
					break;
#endif //HOSTCPU_API_SUPPORT_DDP_ROUTING_READY_STATE
#if (defined HOSTCPU_API_SUPPORT_DDP_ROUTING_PERF_CONFIG)
				case DDP_OP_ROUTING_PERFORMANCE_CONFIG:
					result = handle_ddp_routing_performance_config(&ddp_rinfo, packet_offset);
					break;
#endif //HOSTCPU_API_SUPPORT_DDP_ROUTING_PERF_CONFIG
#if (defined HOSTCPU_API_SUPPORT_DDP_ROUTING_RX_CHAN_CONFIG_STATE)
				case DDP_OP_ROUTING_RX_CHAN_CONFIG_STATE:
					result = handle_ddp_routing_rx_chan_config_state(&ddp_rinfo, packet_offset);
					break;
#endif //HOSTCPU_API_SUPPORT_DDP_ROUTING_RX_CHAN_CONFIG_STATE
#if (defined HOSTCPU_API_SUPPORT_DDP_ROUTING_RX_SUBSCRIBE_SET)
				case DDP_OP_ROUTING_RX_SUBSCRIBE_SET:
					result = handle_ddp_routing_rx_subscribe_set(&ddp_rinfo, packet_offset);
					break;
#endif //HOSTCPU_API_SUPPORT_DDP_ROUTING_RX_SUBSCRIBE_SET
#if (defined HOSTCPU_API_SUPPORT_DDP_ROUTING_RX_CHAN_LABEL_SET)
				case DDP_OP_ROUTING_RX_CHAN_LABEL_SET:
					result = handle_ddp_routing_rx_chan_label_set(&ddp_rinfo, packet_offset);
					break;
#endif //HOSTCPU_API_SUPPORT_DDP_ROUTING_RX_CHAN_LABEL_SET
#if (defined HOSTCPU_API_SUPPORT_DDP_ROUTING_TX_CHAN_CONFIG_STATE)
				case DDP_OP_ROUTING_TX_CHAN_CONFIG_STATE:
					result = handle_ddp_routing_tx_chan_config_state(&ddp_rinfo, packet_offset);
					break;
#endif //HOSTCPU_API_SUPPORT_DDP_ROUTING_TX_CHAN_CONFIG_STATE
#if (defined HOSTCPU_API_SUPPORT_DDP_ROUTING_TX_CHAN_LABEL_SET)
				case DDP_OP_ROUTING_TX_CHAN_LABEL_SET:
					result = handle_ddp_routing_tx_chan_label_set(&ddp_rinfo, packet_offset);
					break;
#endif //HOSTCPU_API_SUPPORT_DDP_ROUTING_TX_CHAN_LABEL_SET
#if (defined HOSTCPU_API_SUPPORT_DDP_ROUTING_RX_FLOW_CONFIG_STATE)
				case DDP_OP_ROUTING_RX_FLOW_CONFIG_STATE:
					result = handle_ddp_routing_rx_flow_config_state(&ddp_rinfo, packet_offset);
					break;
#endif //HOSTCPU_API_SUPPORT_DDP_ROUTING_RX_FLOW_CONFIG_STATE
#if (defined HOSTCPU_API_SUPPORT_DDP_ROUTING_MCAST_TX_FLOW_CONFIG_SET)
				case DDP_OP_ROUTING_MCAST_TX_FLOW_CONFIG_SET:
					result = handle_ddp_routing_multicast_tx_flow_config(&ddp_rinfo, packet_offset);
					break;
#endif //HOSTCPU_API_SUPPORT_DDP_ROUTING_MCAST_TX_FLOW_CONFIG_SET
#if (defined HOSTCPU_API_SUPPORT_DDP_ROUTING_TX_FLOW_CONFIG_STATE)
				case DDP_OP_ROUTING_TX_FLOW_CONFIG_STATE:
					result = handle_ddp_routing_tx_flow_config_state(&ddp_rinfo, packet_offset);
					break;
#endif //HOSTCPU_API_SUPPORT_DDP_ROUTING_TX_FLOW_CONFIG_STATE
#if (defined HOSTCPU_API_SUPPORT_DDP_ROUTING_RX_CHAN_STATUS)
				case DDP_OP_ROUTING_RX_CHAN_STATUS:
					result = handle_ddp_routing_rx_chan_status(&ddp_rinfo, packet_offset);
					break;
#endif //HOSTCPU_API_SUPPORT_DDP_ROUTING_RX_CHAN_STATUS
#if (defined HOSTCPU_API_SUPPORT_DDP_ROUTING_RX_FLOW_STATUS)
				case DDP_OP_ROUTING_RX_FLOW_STATUS:
					result = handle_ddp_routing_rx_flow_status(&ddp_rinfo, packet_offset);
					break;
#endif //HOSTCPU_API_SUPPORT_DDP_ROUTING_RX_FLOW_STATUS
#if (defined HOSTCPU_API_SUPPORT_DDP_ROUTING_RX_UNSUB_CHAN)
				case DDP_OP_ROUTING_RX_UNSUB_CHAN:
					result = handle_ddp_routing_rx_unsub_chan(&ddp_rinfo, packet_offset);
					break;
#endif //HOSTCPU_API_SUPPORT_DDP_ROUTING_RX_UNSUB_CHAN
#if (defined HOSTCPU_API_SUPPORT_DDP_ROUTING_FLOW_DELETE)
				case DDP_OP_ROUTING_FLOW_DELETE:
					result = handle_ddp_routing_flow_delete(&ddp_rinfo, packet_offset);
					break;
#endif //HOSTCPU_API_SUPPORT_DDP_ROUTING_FLOW_DELETE

				default:
					AUD_PRINTF("Unsupported / Unknown DDP msg: opcode=0x%04x - %s\n", ddp_msg_info.opcode, ddp_opcode_to_string(ddp_msg_info.opcode));
					break;
				} //switch (ddp_msg_info.opcode)

				//jump to the next TLV block
				packet_offset += ddp_msg_info.length_bytes;
				result = ddp_packet_read_message(&ddp_rinfo, &ddp_msg_info, packet_offset);
			} //while (result == AUD_SUCCESS)
		}
		break;
		case UhipPacketType_MAX:
		default:
		break;
	}

l__cleanup_return:
	//reset the uhip buffers
	reset_uhip_rx_buffers();
}

/**
 * Handles any pending RX messages
 */
void handle_uhip_rx(unsigned char *dante_date,unsigned short int data_len)
{
	uint8_t temp_rx_buffer[UHIP_CHUNK_SIZE];
	size_t bytes_read = 0;
	aud_error_t result;
	size_t extracted_size;
	unsigned int i;
	unsigned int j = 0;
	unsigned int k = 0;
	//read bytes received from the transport layer on UHIP_CHUNK_SIZE at a time
	//bytes_read = hostcpu_transport_read(temp_rx_buffer, UHIP_CHUNK_SIZE);
	while (1)
	{
		for(j = 0;j + k < data_len;j++)
		{
			if(j == UHIP_CHUNK_SIZE)
			{
				break;
			}
			temp_rx_buffer[j] = dante_date[j + k];
		}

		for (i = 0; i < j; ++i)
		{
			result = extract_cobs_from_rx_frame(temp_rx_buffer[i], cobs_rx_buffer, &extracted_size, uhip_hostcpu_rx_timer_start, uhip_hostcpu_rx_timer_stop, RX_TIMEOUT_MILLISECONDS);

			if (result == AUD_ERR_NOMEMORY)
			{
				hostcpu_uhip_increment_num_rx_overflows();
				reset_uhip_rx_buffers();
			}
			else if (result == AUD_SUCCESS)
			{
				//process the rx packet
				process_cobs_rx_packet(cobs_rx_buffer, extracted_size);
			}
		}
		k = k + UHIP_CHUNK_SIZE;
		if( k >= data_len)
		{
			break;
		}
		//get the next chunk
		//bytes_read = hostcpu_transport_read(temp_rx_buffer, UHIP_CHUNK_SIZE);
	}
}


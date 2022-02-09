/*
* File     : example_tx_pb.c
* Created  : October 2014
* Updated  : 2014/10/28
* Author   : Michael Ung
* Synopsis : HostCPU Implementation of sending example packet bridge requests
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

extern uint8_t tx_buffer[];

static uint8_t packetbridge_payload[UHIP_PACKET_BRIDGE_PAYLOAD_MAX] =
{
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
	0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F,
	0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F,
	0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F,
	0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F,
	0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5A, 0x5B, 0x5C, 0x5D, 0x5E, 0x5F,
	0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F,
	0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7A, 0x7B, 0x7C, 0x7D, 0x7E, 0x7F,
	0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8A, 0x8B, 0x8C, 0x8D, 0x8E, 0x8F,
	0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9A, 0x9B, 0x9C, 0x9D, 0x9E, 0x9F,
	0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7, 0xA8, 0xA9, 0xAA, 0xAB, 0xAC, 0xAD, 0xAE, 0xAF,
	0xB0, 0xB1, 0xB2, 0xB3, 0xB4, 0xB5, 0xB6, 0xB7, 0xB8, 0xB9, 0xBA, 0xBB, 0xBC, 0xBD, 0xBE, 0xBF,
	0xC0, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7, 0xC8, 0xC9, 0xCA, 0xCB, 0xCC, 0xCD, 0xCE, 0xCF,
	0xD0, 0xD1, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6, 0xD7, 0xD8, 0xD9, 0xDA, 0xDB, 0xDC, 0xDD, 0xDE, 0xDF,
	0xE0, 0xE1, 0xE2, 0xE3, 0xE4, 0xE5, 0xE6, 0xE7, 0xE8, 0xE9, 0xEA, 0xEB, 0xEC, 0xED, 0xEE, 0xEF,
	0xF0, 0xF1, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7, 0xF8, 0xF9, 0xFA, 0xFB, 0xFC, 0xFD, 0xFE, 0xFF,
	0xFF, 0xFE, 0xFD, 0xFC, 0xFB, 0xFA, 0xF9, 0xF8, 0xF7, 0xF6, 0xF5, 0xF4, 0xF3, 0xF2, 0xF1, 0xF0,
	0xEF, 0xEE, 0xED, 0xEC, 0xEB, 0xEA, 0xE9, 0xE8, 0xE7, 0xE6, 0xE5, 0xE4, 0xE3, 0xE2, 0xE1, 0xE0,
	0xDF, 0xDE, 0xDD, 0xDC, 0xDB, 0xDA, 0xD9, 0xD8, 0xD7, 0xD6, 0xD5, 0xD4, 0xD3, 0xD2, 0xD1, 0xD0,
	0xCF, 0xCE, 0xCD, 0xCC, 0xCB, 0xCA, 0xC9, 0xC8, 0xC7, 0xC6, 0xC5, 0xC4, 0xC3, 0xC2, 0xC1, 0xC0,
	0xBF, 0xBE, 0xBD, 0xBC, 0xBB, 0xBA, 0xB9, 0xB8, 0xB7, 0xB6, 0xB5, 0xB4, 0xB3, 0xB2, 0xB1, 0xB0,
	0xAF, 0xAE, 0xAD, 0xAC, 0xAB, 0xAA, 0xA9, 0xA8, 0xA7, 0xA6, 0xA5, 0xA4, 0xA3, 0xA2, 0xA1, 0xA0,
	0x9F, 0x9E, 0x9D, 0x9C, 0x9B, 0x9A, 0x99, 0x98, 0x97, 0x96, 0x95, 0x94, 0x93, 0x92, 0x91, 0x90,
	0x8F, 0x8E, 0x8D, 0x8C, 0x8B, 0x8A, 0x89, 0x88, 0x87, 0x86, 0x85, 0x84, 0x83, 0x82, 0x81, 0x80,
	0x7F, 0x7E, 0x7D, 0x7C, 0x7B, 0x7A, 0x79, 0x78, 0x77, 0x76, 0x75, 0x74, 0x73, 0x72, 0x71, 0x70,
	0x6F, 0x6E, 0x6D, 0x6C, 0x6B, 0x6A, 0x69, 0x68, 0x67, 0x66, 0x65, 0x64, 0x63, 0x62, 0x61, 0x60,
	0x5F, 0x5E, 0x5D, 0x5C, 0x5B, 0x5A, 0x59, 0x58, 0x57, 0x56, 0x55, 0x54, 0x53, 0x52, 0x51, 0x50,
	0x4F, 0x4E, 0x4D, 0x4C, 0x4B, 0x4A, 0x49, 0x48, 0x47, 0x46, 0x45, 0x44, 0x43, 0x42, 0x41, 0x40,
	0x3F, 0x3E, 0x3D, 0x3C, 0x3B, 0x3A, 0x39, 0x38, 0x37, 0x36, 0x35, 0x34, 0x33, 0x32, 0x31, 0x30,
	0x2F, 0x2E, 0x2D, 0x2C, 0x2B, 0x2A, 0x29, 0x28, 0x27, 0x26, 0x25, 0x24, 0x23, 0x22, 0x21, 0x20,
	0x1F, 0x1E, 0x1D, 0x1C, 0x1B, 0x1A, 0x19, 0x18, 0x17, 0x16, 0x15, 0x14, 0x13, 0x12, 0x11, 0x10,
	0x0F, 0x0E, 0x0D, 0x0C
};

aud_error_t send_conmon_packetbridge_packet(void)
{
	aud_error_t result;
	uhip_packet_write_info_t uhip_winfo;
	size_t cobs_encoded_size, total_data_to_send, bytes_sent;

	// Allocate extra room to store the start of packet delimiter and data inserted after the COBS encode
	uint8_t * tx_buf_ptr = tx_buffer + 1 + _COBS_PAD(UHIP_PACKET_SIZE_MAX);

	// 1. Initialize the UHIP packet buffer
	result = uhip_packet_init_write(&uhip_winfo, (uint32_t *)tx_buf_ptr, UHIP_PACKET_SIZE_MAX);
	if (result != AUD_SUCCESS)
	{
		return result;
	}

	// 2. Add the UHIP packet bridge ConMon message
	result = uhip_packet_write_cmc_packet_bridge(&uhip_winfo, UhipConMonPacketBridgeChannel_CONMON_STATUS_CH, packetbridge_payload, UHIP_PACKET_BRIDGE_PAYLOAD_MAX);
	if (result != AUD_SUCCESS)
	{
		return result;
	}

	// 3. COBS encode the UHIP packet from step 2. Ensure to keep the first byte free in the buffer that
	// stores the COBS encoded data to add the first delimiter byte
	cobs_encoded_size = cobs_encode(tx_buf_ptr, uhip_winfo.curr_length_bytes, tx_buffer + 1);

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

aud_error_t send_udp_packetbridge_packet(void)
{
	aud_error_t result;
	uhip_packet_write_info_t uhip_winfo;
	size_t cobs_encoded_size, total_data_to_send, bytes_sent;
	uint16_t port = 9000; //port = 9000
	uint32_t ip = 0xEFFE327B; //ip = 239.254.50.123

	// Allocate extra room to store the start of packet delimiter and data inserted after the COBS encode
	uint8_t * tx_buf_ptr = tx_buffer + 1 + _COBS_PAD(UHIP_PACKET_SIZE_MAX);

	// 1. Initialize the UHIP packet buffer
	result = uhip_packet_init_write(&uhip_winfo, (uint32_t *)tx_buf_ptr, UHIP_PACKET_SIZE_MAX);
	if (result != AUD_SUCCESS)
	{
		return result;
	}

	// 2. Add the UHIP packet bridge UDP message
	result = uhip_packet_write_udp_packet_bridge(&uhip_winfo, UhipUDPPacketBridgeChannel_UDP_BROADCAST_CH, port, ip, packetbridge_payload, UHIP_PACKET_BRIDGE_PAYLOAD_MAX);
	if (result != AUD_SUCCESS)
	{
		return result;
	}

	// 3. COBS encode the UHIP packet from step 2. Ensure to keep the first byte free in the buffer that
	// stores the COBS encoded data to add the first delimiter byte
	cobs_encoded_size = cobs_encode(tx_buf_ptr, uhip_winfo.curr_length_bytes, tx_buffer + 1);

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

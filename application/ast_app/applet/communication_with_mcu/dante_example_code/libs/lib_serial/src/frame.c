/*
* File     : frame.c
* Created  : October 2014
* Updated  : 2014/10/30
* Author   : Chamira Perera
* Synopsis : Implementation of utility functions to handle Ultimo serial frames for UHIP
*
* Copyright 2005-2017, Audinate Pty Ltd and/or its licensors
Confidential
* Audinate Copyright Header Version 1
*/

#include "frame.h"

#include "cobs.h"
#include "uhip_structures.h"

/**
* @brief UHIP RX state
*/
typedef enum uhip_rx_state 
{
	UHIP_RX_STATE_IDLE_WAIT_SOF = 0,	/*!< Waiting for the first 0x00 COBS start of frame */
	UHIP_RX_STATE_FIRST_COBS,			/*!< First 0x00 COBS bytes received */
	UHIP_RX_STATE_PAYLOAD,				/*!< Receiving the payload (between the 0x00, and final 0x00 COBS bytes) */
} uhip_rx_state_t;

static uhip_rx_state_t rx_state = UHIP_RX_STATE_IDLE_WAIT_SOF;	/*!< Current RX state */
static uint16_t rx_packet_size_bytes = 0;						/*!< Size of the rx packet */


size_t prepare_tx_frame(uint8_t *tx_buffer, size_t cobs_encoded_size)
{
	size_t total_data_size = 0, loop_test;

	// Add delimeters at the start and end of the COBS encoded data
	tx_buffer[0] = COBS_DELIM_BYTE;
	tx_buffer[cobs_encoded_size + 1] = COBS_DELIM_BYTE;

	// Add pad bytes until the total data is a multiple of 32 bytes (UHIP_CHUNK_SIZE)
	total_data_size = (cobs_encoded_size + 2);
	loop_test = total_data_size % UHIP_CHUNK_SIZE;
	while (loop_test)
	{
		tx_buffer[total_data_size] = UHIP_PAD_BYTE;
		total_data_size++;
		loop_test = total_data_size % UHIP_CHUNK_SIZE;
	}

	return total_data_size;
}

aud_error_t extract_cobs_from_rx_frame(uint8_t current_byte, uint8_t *out_buffer, size_t *num_extracted, timer_start_t rx_timer_start, timer_stop_t rx_timer_stop, int timeout)
{
	//check for an overflow
	if (rx_packet_size_bytes >= _COBS_PAD(UHIP_PACKET_SIZE_MAX) + UHIP_PACKET_SIZE_MAX + COBS_ZERO_PAIR_NUM)
	{
		AUD_PRINTF("ERROR overflow of RX buffer - loss of synchronisation and/or dropped bytes in the hostcpu transport layer!\n");
		return AUD_ERR_NOMEMORY;
	}

	//process the byte
	switch (rx_state)
	{
		//Waiting for the first 0x00 COBS start of frame
		case UHIP_RX_STATE_IDLE_WAIT_SOF:
		{
			if (current_byte == COBS_DELIM_BYTE)
			{
				rx_state = UHIP_RX_STATE_FIRST_COBS;

				//start the RX timer
				if (rx_timer_start) {
					rx_timer_start(timeout);
				}
			}
		}
		break;

		//First 0x00 COBS bytes received
		case UHIP_RX_STATE_FIRST_COBS:
		{
			//if a non 0x00 bytes - this is payload
			if (current_byte != COBS_DELIM_BYTE)
			{
				out_buffer[rx_packet_size_bytes] = current_byte;
				rx_packet_size_bytes++;
				*num_extracted = rx_packet_size_bytes;
				rx_state = UHIP_RX_STATE_PAYLOAD;
			}
			//discard as we have received a 0x00, followed by a 0x00
			else
			{
				AUD_PRINTF("WARNING: [0x00] [0x00] received, possible loss of sync - dropping byte!\n");
			}
		}
		break;

		//Receiving the payload (between the 0x00, and final 0x00 COBS bytes)
		case UHIP_RX_STATE_PAYLOAD:
		{
			//if a non 0x00 bytes - this is payload
			if (current_byte != COBS_DELIM_BYTE)
			{
				out_buffer[rx_packet_size_bytes] = current_byte;
				rx_packet_size_bytes++;
				*num_extracted = rx_packet_size_bytes;

				//stop and restart the RX timer
				if (rx_timer_start && rx_timer_stop) {
					rx_timer_stop();
					rx_timer_start(timeout);
				}
			}
			//otherwise we have a complete frame
			else
			{
				if (rx_timer_stop) {
					rx_timer_stop();
				}

				*num_extracted = rx_packet_size_bytes;				

				rx_state = UHIP_RX_STATE_IDLE_WAIT_SOF;
				rx_packet_size_bytes = 0;

				return AUD_SUCCESS;
			}
		}
		break;
	}

	return AUD_ERR_INPROGRESS;
}

/* Copyright 2011, Jacques Fortier. All rights reserved.
 *
 * From https://github.com/jacquesf/COBS-Consistent-Overhead-Byte-Stuffing
 *
 * Redistribution and use in source and binary forms are permitted, with or without modification.
 */
#ifndef __COBS_H__
#define __COBS_H__

#include <stdint.h>
#include <stddef.h>

#define COBS_DELIM_BYTE 		0x00									/*!< Delimiter byte to frame COBS encoded data */
#define COBS_ZERO_PAIR_NUM		2										/*!< Number of 0x00 pairs for a packet */
#define _COBS_PAD(n) 			(((n + 254 - 1) & ~(254 - 1))/254) 		/*!< macro to calculate the maximum number of COBS pad bytes with a given payload 'n' */

/**
 * Decodes a COBS encoded message
 * @param input [in] pointer to the COBS encoded message
 * @param input_length [in] the length of the COBS encoded message
 * @param output [in] pointer to the decode buffer
 * @param output_length [in] length of the decode buffer
 * @return the number of bytes written to "output" if "input" was successfully
 * unstuffed, and 0 if there was an error unstuffing "input".
 */
size_t cobs_decode(const uint8_t * input, size_t input_length, uint8_t * output, size_t output_length);

/**
 * COBS encodes a message
 * @param input [in] pointer to the raw message
 * @param input_length [in] the length of the raw message
 * @param output [out] pointer to the output encode buffer
 * @return the number of bytes written to "output".
 */
size_t cobs_encode(const uint8_t * input, size_t input_length, uint8_t * output);

#endif // __COBS_H__

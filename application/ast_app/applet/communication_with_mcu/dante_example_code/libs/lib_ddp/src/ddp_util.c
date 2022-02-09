/*
* File     : ddp_util.c
* Created  : August 2014
* Updated  : Date: 2014/08/22
* Author   : Michael Ung <michael.ung@audinate.com>
* Synopsis : Utillity helper functions for DDP.
*
* Copyright 2005-2017, Audinate Pty Ltd and/or its licensors
Confidential
* Audinate Copyright Header Version 1
*/

#include "ddp/util.h"
#include "ddp/clocking_structures.h"


const char * ddp_status_to_string(ddp_status_t status)
{
	switch (status)
	{
	case DDP_STATUS_NOERROR:				return "NO_ERROR";
	case DDP_STATUS_ERROR_OTHER:				return "ERROR_OTHER";
	case DDP_STATUS_ERROR_UNSUPPORTED_OPCODE:		return "ERROR_UNSUPPORTED_OPCODE";
	case DDP_STATUS_ERROR_UNSUPPORTED_VERSION:		return "ERROR_UNSUPPORTED_VERSION";
	case DDP_STATUS_ERROR_INVALID_FORMAT:			return "ERROR_INVALID_FORMAT";
	case DDP_STATUS_ERROR_INVALID_DATA:			return "ERROR_INVALID_DATA";
	case DDP_STATUS_ERROR_PERMISSION_DENIED:		return "ERROR_PERMISSION_DENIED";
	case DDP_STATUS_ERROR_REBOOT_REQUIRED:		return "ERROR_REBOOT_REQUIRED";
	}
	return "?";
}

const char *
ddp_packet_header_flag_direction_to_string(uint8_t dir)
{
	switch(dir)
	{
	case 0: return "None";
	case DDP_PACKET_HEADER_FLAG_DIRECTION_REQUEST:
		return "Request";
	case DDP_PACKET_HEADER_FLAG_DIRECTION_RESPONSE:
		return "Response";
	default:
		return "?";
	}
}


aud_bool_t
ddp_verify_spi_baud_rate(uint32_t baud)
{
	aud_bool_t result = AUD_TRUE;

	switch (baud)
	{
		case 25000000:
		case 12500000:
		case 6250000:
		case 3125000:
		case 1562500:
		case 781250:
		case 390625:
		case 195312:
		case 97656:
		case 48828:
			break;
		default:
			result = AUD_FALSE;
			break;
	}

	return result;
}

aud_bool_t
ddp_verify_uart_baud_rate(uint32_t baud)
{
	aud_bool_t result = AUD_TRUE;

	switch (baud)
	{
		case 115200:
		case 57600:
		case 38400:
		case 19200:
		case 9600:
			break;
		default:
			result = AUD_FALSE;
			break;
	}

	return result;
}


/*
 * File     : uhip_util.c
 * Created  : Oct 2014
 * Updated  : 2014/10/08
 * Author   : Michael Ung
 * Synopsis : Ultimo Host Interface Protocol (UHIP) Utility functions
 *
 * Copyright 2005-2017, Audinate Pty Ltd and/or its licensors
	Confidential
 * Audinate Copyright Header Version 1
 */

#include "uhip_util.h"

/**
 * Converts a UHIP packet type to a string
 * @param type [in] the UHIP packet type
 * @return A string representation of this packet type
 */
const char * uhip_packet_type_to_string(UhipPacketType type)
{
	switch(type)
	{
		case UhipPacketType_PROTOCOL_CONTROL:			return "PROTOCOL_CONTROL";
		case UhipPacketType_CONMON_PACKET_BRIDGE: 		return "CONMON_PACKET_BRIDGE";
		case UhipPacketType_UDP_PACKET_BRIDGE:			return "UDP_PACKET_BRIDGE";
		case UhipPacketType_DANTE_EVENTS:				return "DANTE_EVENT";
		case UhipPacketType_DDP:						return "DANTE_DEVICE_PROTOCOL";
		case UhipPacketType_MAX:						return "?";
		default:										return "?";
	}
	
	return "?";
}

/**
 * Converts a UHIP protocol control type to a string
 * @param type [in] the UHIP protocol control type
 * @return A string representation of this type
 */
const char * uhip_protocol_control_type_to_string(UhipProtocolControlType type)
{
	switch(type)
	{
		case UhipProtocolControlType_ACK_SUCCESS:				return "ACK_SUCCESS";
		case UhipProtocolControlType_ERROR_OTHER: 				return "ERROR_OTHER";
		case UhipProtocolControlType_ERROR_MALFORMED_PACKET:	return "ERROR_MALFORMED_PACKET";
		case UhipProtocolControlType_ERROR_UNKNOWN_TYPE:		return "ERROR_UNKNOWN_TYPE";
		case UhipProtocolControlType_ERROR_NETWORK:				return "ERROR_NETWORK";
		case UhipProtocolControlType_ERROR_TIMEOUT:				return "ERROR_TIMEOUT";
		case UhipProtocolControlType_MAX:						return "?";
		default:												return "?";
	}
	
	return "?";
}

/**
 * Converts a UHIP ConMon packet bridge channel to a string
 * @param channel [in] the UHIP ConMon packet bridge channel
 * @return A string representation of this channel
 */
const char * uhip_cmc_pb_chan_to_string(UhipConMonPacketBridgeChannel channel)
{
	switch(channel)
	{
		case UhipConMonPacketBridgeChannel_CONMON_CONTROL_CH:		return "CONTROL_CHANNEL";
		case UhipConMonPacketBridgeChannel_CONMON_STATUS_CH: 		return "STATUS_CHANNEL";
		case UhipConMonPacketBridgeChannel_CONMON_VENDOR_BCAST_CH:	return "VENDOR_BROADCAST_CHANNEL";
		case UhipConMonPacketBridgeChannel_MAX:						return "?";
		default:													return "?";
	}
	
	return "?";
}

/**
 * Converts a UHIP UDP packet bridge channel to a string
 * @param channel [in] the UHIP UDP packet bridge channel
 * @return A string representation of this channel
 */
const char * uhip_udp_pb_chan_to_string(UhipUDPPacketBridgeChannel channel)
{
	switch (channel)
	{
		case UhipUDPPacketBridgeChannel_UDP_BROADCAST_CH:		return "BROADCAST_CHANNEL";
		case UhipUDPPacketBridgeChannel_UDP_UNICAST_CH: 		return "UNICAST_CHANNEL";
		case UhipUDPPacketBridgeChannel_MAX:					return "?";
		default:												return "?";
	}
	
	return "?";
}


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

const char * ddp_opcode_to_string(ddp_opcode_t opcode)
{
	switch (opcode)
	{
	case DDP_OP_DEVICE_GENERAL:						return "DEVICE_GENERAL";
	case DDP_OP_DEVICE_MANF:						return "DEVICE_MANF";
	case DDP_OP_DEVICE_SECURITY:					return "DEVICE_SECURITY";
	case DDP_OP_DEVICE_UPGRADE:						return "DEVICE_UPGRADE";
	case DDP_OP_DEVICE_ERASE_CONFIG:				return "ERASE_CONFIG";
	case DDP_OP_DEVICE_REBOOT:						return "DEVICE_REBOOT";
	case DDP_OP_DEVICE_IDENTITY:					return "DEVICE_IDENTITY";
	case DDP_OP_DEVICE_IDENTIFY:					return "DEVICE_IDENTIFY";
	case DDP_OP_DEVICE_GPIO:						return "DEVICE_GPIO";
	case DDP_OP_DEVICE_SWITCH_LED:					return "DEVICE_SWITCH_LED";
	case DDP_OP_DEVICE_AES67:						return "DEVICE_AES67";
	case DDP_OP_DEVICE_LOCK_UNLOCK:					return "DEVICE_LOCK_UNLOCK";
	case DDP_OP_DEVICE_SWITCH_REDUNDANCY:			return "DEVICE_SWITCH_REDUNDANCY";
	case DDP_OP_DEVICE_UART_CONFIG:					return "DEVICE_UART_CONFIG";
	case DDP_OP_DEVICE_VLAN_CONFIG:					return "DEVICE_VLAN_CONFIG";
	case DDP_OP_DEVICE_METER_CONFIG:				return "DEVICE_METER_CONFIG";
	case DDP_OP_DEVICE_DANTE_DOMAIN:				return "DEVICE_DANTE_DOMAIN";
	case DDP_OP_DEVICE_SWITCH_STATUS:				return "DEVICE_SWITCH_STATUS";

	case DDP_OP_NETWORK_BASIC:						return "NETWORK_BASIC";
	case DDP_OP_NETWORK_CONFIG:						return "NETWORK_CONFIG";

	case DDP_OP_CLOCK_BASIC_LEGACY:					return "CLOCK_BASIC_LEGACY";
	case DDP_OP_CLOCK_CONFIG:						return "CLOCK_CONFIG";
	case DDP_OP_CLOCK_PULLUP:						return "CLOCK_PULLUP";
	case DDP_OP_CLOCK_BASIC2:						return "CLOCK_BASIC2";

	case DDP_OP_ROUTING_BASIC:						return "ROUTING_BASIC";
	case DDP_OP_ROUTING_READY_STATE:				return "ROUTING_READY_STATE";
	case DDP_OP_ROUTING_PERFORMANCE_CONFIG:			return "ROUTING_PERFORMANCE_CONFIG";
	case DDP_OP_ROUTING_RX_CHAN_CONFIG_STATE:		return "ROUTING_RX_CHAN_CONFIG_STATE";
	case DDP_OP_ROUTING_TX_CHAN_CONFIG_STATE:		return "ROUTING_TX_CHAN_CONFIG_STATE";
	case DDP_OP_ROUTING_RX_FLOW_CONFIG_STATE:		return "ROUTING_RX_FLOW_CONFIG_STATE";
	case DDP_OP_ROUTING_TX_FLOW_CONFIG_STATE:		return "ROUTING_TX_FLOW_CONFIG_STATE";
	case DDP_OP_ROUTING_RX_CHAN_STATUS:				return "ROUTING_RX_CHAN_STATUS";
	case DDP_OP_ROUTING_RX_FLOW_STATUS:				return "ROUTING_RX_FLOW_STATUS";
	case DDP_OP_ROUTING_RX_SUBSCRIBE_SET:			return "ROUTING_RX_SUBSCRIBE_SET";
	case DDP_OP_ROUTING_RX_UNSUB_CHAN:				return "ROUTING_RX_UNSUB_CHAN";
	case DDP_OP_ROUTING_RX_CHAN_LABEL_SET:			return "ROUTING_RX_CHAN_LABEL_SET";
	case DDP_OP_ROUTING_TX_CHAN_LABEL_SET:			return "ROUTING_TX_CHAN_LABEL_SET";
	case DDP_OP_ROUTING_MCAST_TX_FLOW_CONFIG_SET:	return "ROUTING_MCAST_TX_FLOW_CONFIG_SET";
	case DDP_OP_ROUTING_MANUAL_TX_FLOW_CONFIG_SET:	return "ROUTING_MANUAL_TX_FLOW_CONFIG_SET";
	case DDP_OP_ROUTING_MANUAL_RX_FLOW_CONFIG_SET:	return "ROUTING_MANUAL_RX_FLOW_CONFIG_SET";
	case DDP_OP_ROUTING_FLOW_DELETE:				return "ROUTING_FLOW_DELETE";

	case DDP_OP_AUDIO_BASIC:						return "AUDIO_BASIC";
	case DDP_OP_AUDIO_SRATE_CONFIG:					return "AUDIO_SRATE_CONFIG";
	case DDP_OP_AUDIO_ENC_CONFIG:					return "AUDIO_ENC_CONFIG";
	case DDP_OP_AUDIO_ERROR:						return "AUDIO_ERROR";
	case DDP_OP_AUDIO_SIGNAL_PRESENCE_CONFIG:		return "AUDIO_SIGNAL_PRESENCE_CONFIG";
	case DDP_OP_AUDIO_SIGNAL_PRESENCE_DATA:			return "AUDIO_SIGNAL_PRESENCE_DATA";
	case DDP_OP_AUDIO_INTERFACE:					return "AUDIO_INTERFACE";

	case DDP_OP_MDNS_CONFIG:						return "MDNS_CONFIG";
	case DDP_OP_MDNS_REGISTER_SERVICE:				return "MDNS_REGISTER_SERVICE";
	case DDP_OP_MDNS_DEREGISTER_SERVICE:			return "MDNS_DEREGISTER_SERVICE";

	case DDP_OP_LOCAL_AUDIO_FORMAT:					return "LOCAL_AUDIO_FORMAT";
	case DDP_OP_LOCAL_CLOCK_PULLUP:					return "LOCAL_CLOCK_PULLUP";

	case DDP_OP_MONITOR_INTERFACE_STATISTICS:		return "MONITOR_INTERFACE_STATISTICS";
	case DDP_OP_MONITOR_CLOCK:						return "MONITOR_CLOCK";
	case DDP_OP_MONITOR_SIGNAL_PRESENCE:			return "MONITOR_SIGNAL_PRESENCE";
	case DDP_OP_MONITOR_RXFLOW_MAX_LATENCY:			return "MONITOR_RXFLOW_MAX_LATENCY";
	case DDP_OP_MONITOR_RXFLOW_LATE_PACKETS:		return "MONITOR_LATE_PACKETS";
	case DDP_OP_MONITOR_TIMER_ACCURACY:				return "MONITOR_TIMER_ACCURACY";
	}
	return "?";
}


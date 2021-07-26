/*
* File     : ddp.h
* Created  : August 2014
* Updated  : Date: 2014/08/22
* Author   : Michael Ung <michael.ung@audinate.com>
* Synopsis : Top level DDP definitions
*
* Copyright 2005-2017, Audinate Pty Ltd and/or its licensors
Confidential
* Audinate Copyright Header Version 1
*/

#ifndef _DDP_H
#define _DDP_H

#include "dante/dante_common.h"

// DDP versions are uint 16 in 4.4.8 format
enum
{
	DDP_VERSION_1_0_0		= 0x1000,
	DDP_VERSION_CURRENT		= DDP_VERSION_1_0_0
};
typedef uint16_t ddp_version_t;

#define DDP_VERSION_MAJOR(V) (((V) & 0xF000)  >> 12)
#define DDP_VERSION_MINOR(V) (((V) & 0x0F00)  >> 8)
#define DDP_VERSION_BUGFIX(V) (((V) & 0x000F) >> 0)

// offsets with in a DDP packet in host order
typedef uint16_t ddp_offset_t;

// offsets with in a DDP packet in network order
typedef uint16_t ddp_raw_offset_t;

// DDP request ids are a uint16_t, where 0 represents an event
typedef uint16_t ddp_request_id_t;

// DDP opcodes are a uint16_t, however the MSB is reserved.
typedef uint16_t ddp_opcode_t;

// DDP status codes are a uint16_t
enum ddp_status
{
	DDP_STATUS_NOERROR						= 0,	//[0] no error
	DDP_STATUS_ERROR_OTHER					= 1,	//[1] error - other (undefined error)
	DDP_STATUS_ERROR_UNSUPPORTED_OPCODE		= 2,	//[2] error - unsupported opcode
	DDP_STATUS_ERROR_UNSUPPORTED_VERSION	= 3,	//[3] error - unsupported DDP version
	DDP_STATUS_ERROR_INVALID_FORMAT			= 4,	//[4] error - invalid packet/message format
	DDP_STATUS_ERROR_INVALID_DATA			= 5,	//[5] error - invalid data
	DDP_STATUS_ERROR_PERMISSION_DENIED		= 6,	//[6] error - operation is not allowed / permitted
	DDP_STATUS_ERROR_REBOOT_REQUIRED		= 7 	//[7] error - operation cannot be performed because device is in read only mode and requires a reboot
	//OTHER RESERVED
};
typedef uint16_t ddp_status_t;

/**
* @struct ddp_event_timestamp
* @brief Structure format for a the DDP event
*/
typedef struct ddp_event_timestamp {
	uint32_t seconds;				/*!< seconds */
	uint32_t subseconds;			/*!< subseconds (no defined unit) */
} ddp_event_timestamp_t;

//----------
// OPCODES

#define DDP_OP_DEVICE_BASE			0x1000
#define DDP_OP_DEVICE_END			0x10FF

#define DDP_OP_NETWORK_BASE			0x1100
#define DDP_OP_NETWORK_END			0x11FF

#define DDP_OP_CLOCK_BASE			0x1200
#define DDP_OP_CLOCK_END			0x12FF

#define DDP_OP_ROUTING_BASE			0x1300
#define DDP_OP_ROUTING_END			0x13FF

#define DDP_OP_AUDIO_BASE			0x1400
#define DDP_OP_AUDIO_END			0x14FF

#define DDP_OP_MDNS_BASE			0x1500
#define DDP_OP_MDNS_END				0x15FF

// Range 0x1800 - 0x1FFF is reserved
#define DDP_OP_RESERVED_BIT_01 0x800


#define DDP_OP_RESERVED_BASE		0x2000
#define DDP_OP_RESERVED_END			0x2FFF

#define DDP_OP_LOCAL_BASE			0x4000
#define DDP_OP_LOCAL_END			0x40FF

#define DDP_OP_MONITOR_BASE			0x8000
#define DDP_OP_MONITOR_END			0x80FF

// Device message opcodes
typedef enum
{
	DDP_DEVICE_TYPE_GENERAL,
	DDP_DEVICE_TYPE_MANF,
	DDP_DEVICE_TYPE_SECURITY,
	DDP_DEVICE_TYPE_UPGRADE,
	DDP_DEVICE_TYPE_ERASE_CONFIG,
	DDP_DEVICE_TYPE_REBOOT,
	DDP_DEVICE_TYPE_IDENTITY,
	DDP_DEVICE_TYPE_IDENTIFY,
	DDP_DEVICE_TYPE_GPIO,
	DDP_DEVICE_TYPE_SWITCH_LED,
	DDP_DEVICE_TYPE_AES67,
	DDP_DEVICE_TYPE_LOCK_UNLOCK,
	DDP_DEVICE_TYPE_SWITCH_REDUNDANCY,
	DDP_DEVICE_TYPE_UART_CONFIG,
	DDP_DEVICE_TYPE_VLAN_CONFIG,
	DDP_DEVICE_TYPE_METER_CONFIG,
	DDP_DEVICE_TYPE_DANTE_DOMAIN,
	DDP_DEVICE_TYPE_SWITCH_STATUS,
	DDP_NUM_DEVICE_TYPES
} ddp_device_type_t;

// network message opcodes
typedef enum
{
	DDP_NETWORK_TYPE_BASIC,
	DDP_NETWORK_TYPE_CONFIG,
	DDP_NUM_NETWORK_TYPES
} ddp_network_type_t;

// clocking message opcodes
typedef enum
{
	DDP_CLOCK_TYPE_BASIC_LEGACY,
	DDP_CLOCK_TYPE_CONFIG,
	DDP_CLOCK_TYPE_PULLUP,
	DDP_CLOCK_TYPE_BASIC2,
	DDP_NUM_CLOCK_TYPES,
} ddp_clock_type_t;

// Routing message opcodes
typedef enum
{
	DDP_ROUTING_TYPE_BASIC,
	DDP_ROUTING_TYPE_READY_STATE,
	DDP_ROUTING_TYPE_PERFORMANCE_CONFIG,

	DDP_ROUTING_TYPE_RX_CHAN_CONFIG_STATE,
	DDP_ROUTING_TYPE_TX_CHAN_CONFIG_STATE,
	DDP_ROUTING_TYPE_RX_FLOW_CONFIG_STATE,
	DDP_ROUTING_TYPE_TX_FLOW_CONFIG_STATE,

	DDP_ROUTING_TYPE_RX_CHAN_STATUS,
	DDP_ROUTING_TYPE_RX_FLOW_STATUS,

	DDP_ROUTING_TYPE_RX_SUBSCRIBE_SET,
	DDP_ROUTING_TYPE_RX_UNSUB_CHAN,
	DDP_ROUTING_TYPE_RX_CHAN_LABEL_SET,
	DDP_ROUTING_TYPE_TX_CHAN_LABEL_SET,

	DDP_ROUTING_TYPE_MCAST_TX_FLOW_CONFIG_SET,
	DDP_ROUTING_TYPE_MANUAL_RX_FLOW_CONFIG_SET,
	DDP_ROUTING_TYPE_MANUAL_TX_FLOW_CONFIG_SET,
	DDP_ROUTING_TYPE_FLOW_DELETE,

	DDP_NUM_ROUTING_TYPES
} ddp_routing_type_t;

// audio message opcodes
typedef enum
{
	DDP_AUDIO_TYPE_BASIC,
	DDP_AUDIO_TYPE_SRATE_CONFIG,
	DDP_AUDIO_TYPE_ENC_CONFIG,
	DDP_AUDIO_TYPE_ERROR,
	DDP_AUDIO_TYPE_SIGNAL_PRESENCE_CONFIG,
	DDP_AUDIO_TYPE_SIGNAL_PRESENCE_DATA,
	DDP_AUDIO_TYPE_INTERFACE,
	DDP_NUM_AUDIO_TYPES
} ddp_audio_type_t;

// mdns message opcodes
typedef enum
{
	DDP_MDNS_TYPE_CONFIG,
	DDP_MDNS_TYPE_REGISTER_SERVICE,
	DDP_MDNS_TYPE_DEREGISTER_SERVICE,
	DDP_NUM_MDNS_TYPES
} ddp_mdns_type_t;

//local message opcodes
typedef enum
{
	DDP_LOCAL_TYPE_AUDIO_FORMAT,
	DDP_LOCAL_TYPE_CLOCK_PULLUP,
	DDP_NUM_LOCAL_TYPES
} ddp_local_type_t;

// Monitoring message opcodes are packed and incremental
// to make it easy to describe expected messages via a bitmask
typedef enum
{
	DDP_MONITORING_TYPE_INTERFACE_STATISTICS,
	DDP_MONITORING_TYPE_CLOCK,
	DDP_MONITORING_TYPE_SIGNAL_PRESENCE,
	DDP_MONITORING_TYPE_RXFLOW_MAX_LATENCY,
	DDP_MONITORING_TYPE_RXFLOW_LATE_PACKETS,
	DDP_MONITORING_TYPE_TIMER_ACCURACY,
	DDP_NUM_MONITORING_TYPES

	//DDP_MONITORING_TYPE_RXFLOW_EARLY_PACKETS,
	//DDP_MONITORING_TYPE_RXFLOW_OUT_OF_ORDER_PACKETS,
	//DDP_MONITORING_TYPE_RXFLOW_DROPPED_PACKETS,

} ddp_monitoring_type_t;

// DDP opcodes
enum
{
	DDP_OP_NONE = 0x0000,

	DDP_OP_PACKET_HEADER = 0x0001,

	DDP_OP_EMPTY = 0x0002,
		// Indicates a block with no content - only the block header is meaningful
		// Should be skipped during packet parsing
		// Useful to blank out a message in an existing packet

	//device opcodes = base 0x1000
	DDP_OP_DEVICE_GENERAL						= DDP_OP_DEVICE_BASE + DDP_DEVICE_TYPE_GENERAL,
	DDP_OP_DEVICE_MANF							= DDP_OP_DEVICE_BASE + DDP_DEVICE_TYPE_MANF,
	DDP_OP_DEVICE_SECURITY						= DDP_OP_DEVICE_BASE + DDP_DEVICE_TYPE_SECURITY,
	DDP_OP_DEVICE_UPGRADE						= DDP_OP_DEVICE_BASE + DDP_DEVICE_TYPE_UPGRADE,
	DDP_OP_DEVICE_ERASE_CONFIG					= DDP_OP_DEVICE_BASE + DDP_DEVICE_TYPE_ERASE_CONFIG,
	DDP_OP_DEVICE_REBOOT						= DDP_OP_DEVICE_BASE + DDP_DEVICE_TYPE_REBOOT,
	DDP_OP_DEVICE_IDENTITY						= DDP_OP_DEVICE_BASE + DDP_DEVICE_TYPE_IDENTITY,
	DDP_OP_DEVICE_IDENTIFY						= DDP_OP_DEVICE_BASE + DDP_DEVICE_TYPE_IDENTIFY,
	DDP_OP_DEVICE_GPIO							= DDP_OP_DEVICE_BASE + DDP_DEVICE_TYPE_GPIO,
	DDP_OP_DEVICE_SWITCH_LED					= DDP_OP_DEVICE_BASE + DDP_DEVICE_TYPE_SWITCH_LED,
	DDP_OP_DEVICE_AES67							= DDP_OP_DEVICE_BASE + DDP_DEVICE_TYPE_AES67,
	DDP_OP_DEVICE_LOCK_UNLOCK					= DDP_OP_DEVICE_BASE + DDP_DEVICE_TYPE_LOCK_UNLOCK,
	DDP_OP_DEVICE_SWITCH_REDUNDANCY				= DDP_OP_DEVICE_BASE + DDP_DEVICE_TYPE_SWITCH_REDUNDANCY,
	DDP_OP_DEVICE_UART_CONFIG					= DDP_OP_DEVICE_BASE + DDP_DEVICE_TYPE_UART_CONFIG,
	DDP_OP_DEVICE_VLAN_CONFIG					= DDP_OP_DEVICE_BASE + DDP_DEVICE_TYPE_VLAN_CONFIG,
	DDP_OP_DEVICE_METER_CONFIG					= DDP_OP_DEVICE_BASE + DDP_DEVICE_TYPE_METER_CONFIG,
	DDP_OP_DEVICE_DANTE_DOMAIN					= DDP_OP_DEVICE_BASE + DDP_DEVICE_TYPE_DANTE_DOMAIN,
	DDP_OP_DEVICE_SWITCH_STATUS					= DDP_OP_DEVICE_BASE + DDP_DEVICE_TYPE_SWITCH_STATUS,

	//network opcodes = base 0x1100
	DDP_OP_NETWORK_BASIC						= DDP_OP_NETWORK_BASE + DDP_NETWORK_TYPE_BASIC,
	DDP_OP_NETWORK_CONFIG						= DDP_OP_NETWORK_BASE + DDP_NETWORK_TYPE_CONFIG,

	//clocking opcodes = base 0x1200
	DDP_OP_CLOCK_BASIC_LEGACY					= DDP_OP_CLOCK_BASE + DDP_CLOCK_TYPE_BASIC_LEGACY,
	DDP_OP_CLOCK_CONFIG							= DDP_OP_CLOCK_BASE + DDP_CLOCK_TYPE_CONFIG,
	DDP_OP_CLOCK_PULLUP							= DDP_OP_CLOCK_BASE + DDP_CLOCK_TYPE_PULLUP,
	DDP_OP_CLOCK_BASIC2							= DDP_OP_CLOCK_BASE + DDP_CLOCK_TYPE_BASIC2,

	//routing opcodes = base 0x1300
	DDP_OP_ROUTING_BASIC						= DDP_OP_ROUTING_BASE + DDP_ROUTING_TYPE_BASIC,
	DDP_OP_ROUTING_READY_STATE					= DDP_OP_ROUTING_BASE + DDP_ROUTING_TYPE_READY_STATE,
	DDP_OP_ROUTING_PERFORMANCE_CONFIG			= DDP_OP_ROUTING_BASE + DDP_ROUTING_TYPE_PERFORMANCE_CONFIG,
	DDP_OP_ROUTING_RX_CHAN_CONFIG_STATE			= DDP_OP_ROUTING_BASE + DDP_ROUTING_TYPE_RX_CHAN_CONFIG_STATE,
	DDP_OP_ROUTING_TX_CHAN_CONFIG_STATE			= DDP_OP_ROUTING_BASE + DDP_ROUTING_TYPE_TX_CHAN_CONFIG_STATE,
	DDP_OP_ROUTING_RX_FLOW_CONFIG_STATE			= DDP_OP_ROUTING_BASE + DDP_ROUTING_TYPE_RX_FLOW_CONFIG_STATE,
	DDP_OP_ROUTING_TX_FLOW_CONFIG_STATE			= DDP_OP_ROUTING_BASE + DDP_ROUTING_TYPE_TX_FLOW_CONFIG_STATE,
	DDP_OP_ROUTING_RX_CHAN_STATUS				= DDP_OP_ROUTING_BASE + DDP_ROUTING_TYPE_RX_CHAN_STATUS,
	DDP_OP_ROUTING_RX_FLOW_STATUS				= DDP_OP_ROUTING_BASE + DDP_ROUTING_TYPE_RX_FLOW_STATUS,
	DDP_OP_ROUTING_RX_SUBSCRIBE_SET				= DDP_OP_ROUTING_BASE + DDP_ROUTING_TYPE_RX_SUBSCRIBE_SET,
	DDP_OP_ROUTING_RX_UNSUB_CHAN				= DDP_OP_ROUTING_BASE + DDP_ROUTING_TYPE_RX_UNSUB_CHAN,
	DDP_OP_ROUTING_RX_CHAN_LABEL_SET			= DDP_OP_ROUTING_BASE + DDP_ROUTING_TYPE_RX_CHAN_LABEL_SET,
	DDP_OP_ROUTING_TX_CHAN_LABEL_SET			= DDP_OP_ROUTING_BASE + DDP_ROUTING_TYPE_TX_CHAN_LABEL_SET,
	DDP_OP_ROUTING_MCAST_TX_FLOW_CONFIG_SET		= DDP_OP_ROUTING_BASE + DDP_ROUTING_TYPE_MCAST_TX_FLOW_CONFIG_SET,
	DDP_OP_ROUTING_MANUAL_RX_FLOW_CONFIG_SET	= DDP_OP_ROUTING_BASE + DDP_ROUTING_TYPE_MANUAL_RX_FLOW_CONFIG_SET,
	DDP_OP_ROUTING_MANUAL_TX_FLOW_CONFIG_SET	= DDP_OP_ROUTING_BASE + DDP_ROUTING_TYPE_MANUAL_TX_FLOW_CONFIG_SET,
	DDP_OP_ROUTING_FLOW_DELETE					= DDP_OP_ROUTING_BASE + DDP_ROUTING_TYPE_FLOW_DELETE,

	//audio opcodes = base 0x1400
	DDP_OP_AUDIO_BASIC							= DDP_OP_AUDIO_BASE + DDP_AUDIO_TYPE_BASIC,
	DDP_OP_AUDIO_SRATE_CONFIG					= DDP_OP_AUDIO_BASE + DDP_AUDIO_TYPE_SRATE_CONFIG,
	DDP_OP_AUDIO_ENC_CONFIG						= DDP_OP_AUDIO_BASE + DDP_AUDIO_TYPE_ENC_CONFIG,
	DDP_OP_AUDIO_ERROR							= DDP_OP_AUDIO_BASE + DDP_AUDIO_TYPE_ERROR,
	DDP_OP_AUDIO_SIGNAL_PRESENCE_CONFIG			= DDP_OP_AUDIO_BASE + DDP_AUDIO_TYPE_SIGNAL_PRESENCE_CONFIG,
	DDP_OP_AUDIO_SIGNAL_PRESENCE_DATA			= DDP_OP_AUDIO_BASE + DDP_AUDIO_TYPE_SIGNAL_PRESENCE_DATA,
	DDP_OP_AUDIO_INTERFACE						= DDP_OP_AUDIO_BASE + DDP_AUDIO_TYPE_INTERFACE,

	//mdns opcodes = base 0x1500
	DDP_OP_MDNS_CONFIG							= DDP_OP_MDNS_BASE + DDP_MDNS_TYPE_CONFIG,
	DDP_OP_MDNS_REGISTER_SERVICE				= DDP_OP_MDNS_BASE + DDP_MDNS_TYPE_REGISTER_SERVICE,
	DDP_OP_MDNS_DEREGISTER_SERVICE				= DDP_OP_MDNS_BASE + DDP_MDNS_TYPE_DEREGISTER_SERVICE,

	//local opcodes = base 0x4000
	DDP_OP_LOCAL_AUDIO_FORMAT					= DDP_OP_LOCAL_BASE + DDP_LOCAL_TYPE_AUDIO_FORMAT,
	DDP_OP_LOCAL_CLOCK_PULLUP					= DDP_OP_LOCAL_BASE + DDP_LOCAL_TYPE_CLOCK_PULLUP,

	//monitoring opcodes = base 0x8000
	DDP_OP_MONITOR_INTERFACE_STATISTICS			= DDP_OP_MONITOR_BASE + DDP_MONITORING_TYPE_INTERFACE_STATISTICS,
	DDP_OP_MONITOR_CLOCK						= DDP_OP_MONITOR_BASE + DDP_MONITORING_TYPE_CLOCK,
	DDP_OP_MONITOR_SIGNAL_PRESENCE				= DDP_OP_MONITOR_BASE + DDP_MONITORING_TYPE_SIGNAL_PRESENCE,
	DDP_OP_MONITOR_RXFLOW_MAX_LATENCY			= DDP_OP_MONITOR_BASE + DDP_MONITORING_TYPE_RXFLOW_MAX_LATENCY,
	DDP_OP_MONITOR_RXFLOW_LATE_PACKETS			= DDP_OP_MONITOR_BASE + DDP_MONITORING_TYPE_RXFLOW_LATE_PACKETS,
	DDP_OP_MONITOR_TIMER_ACCURACY				= DDP_OP_MONITOR_BASE + DDP_MONITORING_TYPE_TIMER_ACCURACY
};

/**
* @union ddp_block_header
* @brief Structure format for a the DDP block header
*/
typedef union ddp_block_header
{
	struct
	{
		uint16_t length_bytes; 	//the total length of this block
		uint16_t opcode;		//opcode for this block
	} _;						/*!< ddp_block_header structure definition */

	uint32_t __alignment;		/*!< enforce 32-bit alignment for all packet blocks */
} ddp_block_header_t;


/**
* @struct ddp_packet_header
* @brief Structure format for a the DDP packet header, must be the first block in a packet
*/
typedef struct ddp_packet_header
{
	ddp_block_header_t block;		/*!< The block header */
	uint16_t version;				/*!< version */

	union {
		uint16_t padding0;				//if no extension header set padding0 to 0x0000

		struct {
			uint8_t flags;
			uint8_t extension_offset;	//offset to the extension
		} ext;
	} _; /*!< ddp_packet_header extension definition */
} ddp_packet_header_t;

enum
{
	// Bits 0-1 of flag field indicate 'direction' of packet.

		// Useful for bi-directional pipes where each end of the pipe sometimes
		// functions as a client (issuing requests) and sometimes as a server
		// (sending responses and events)

		// NOTE: request and response messages MUST NOT be put in the same packet

	DDP_PACKET_HEADER_FLAG_DIRECTION_MASK = 0x3,
	DDP_PACKET_HEADER_FLAG_DIRECTION_NONE = 0x0,
		// Implicit or undefined direction
	DDP_PACKET_HEADER_FLAG_DIRECTION_REQUEST = 0x2,
		// Packet is a request (client -> server)
	DDP_PACKET_HEADER_FLAG_DIRECTION_RESPONSE = 0x3,
		// Packet is a response or event (server -> client)

	DDP_PACKET_HEADER_FLAG_LINK_INFRASTRUCTURE = 0x80,
		// Packet is for link endpoint and not client code
};


/**
* @struct ddp_message_header
* @brief Structure format for a DDP message
*/
typedef struct ddp_message_header
{
	ddp_block_header_t block;			/*!< The block header */
	uint16_t subheader_length_bytes;	/*!< Length of the subheader in bytes */
	uint16_t payload_length_bytes;		/*!< Length of the payload in bytes */

	// message header is followed by subheader and then by payload
} ddp_message_header_t;

/**
* @struct ddp_request_subheader
* @brief Structure format for the DDP request subheader
*/
typedef struct ddp_request_subheader
{
	ddp_request_id_t request_id;	/*!< request id for this message, must be non-zero */
	uint16_t padding;				/*!< pad */
} ddp_request_subheader_t;

#define DDP_REQUEST_SUBHEADER_MIN_SIZE 4 // subheader might grow one day (unlikely but posible) but will never shrink

/**
* @struct ddp_response_subheader
* @brief Structure format for the DDP response subheader
*/
typedef struct ddp_response_subheader
{
	ddp_request_id_t request_id;	/*!< request id for that is response is for, 0 if an event */
	ddp_status_t status;			/*!< status @see ddp_status */
} ddp_response_subheader_t;

/**
* @struct ddp_local_subheader
* @brief Structure format for the DDP local subheader
*/
typedef struct ddp_local_subheader
{
	ddp_event_timestamp_t timestamp;	/*!< timestamp */
} ddp_local_subheader_t;

/**
* @struct ddp_monitoring_subheader
* @brief Structure format for the DDP monitoring subheader
*/
typedef struct ddp_monitoring_subheader
{
	uint16_t monitor_seqnum;		/*!< monitoring sequence number */
	uint16_t padding;				/*!< pad */
} ddp_monitoring_subheader_t;

#endif // _DDP_H

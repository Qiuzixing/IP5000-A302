/*
 * File     : routing_structures.h
 * Created  : June 2014
 * Author   : Andrew White <andrew.white@audinate.com>, Chamira Perera <cperera@audinate.com>
 * Synopsis : All routing DDP messages structure definitions.
 *
 * Copyright 2005-2017, Audinate Pty Ltd and/or its licensors
	Confidential
 * Audinate Copyright Header Version 1
 */
#ifndef _DDP_ROUTING_STRUCTURES_H
#define _DDP_ROUTING_STRUCTURES_H


//----------

#include "ddp/ddp.h"

#include "ddp/shared_structures.h"

/**
* @file routing_structures.h
* @brief All routing DDP messages structure definitions.
*/

/** \addtogroup DDP
*  @{
*/

/** \addtogroup Routing
*  @{
*/

/**
* \defgroup RoutingEnums DDP Routing Enums
* @{
*/

/**
* @enum routing_basic_fpp_modes
* @brief Frames per packet modes for ddp_routing_basic_response
*/
enum routing_basic_fpp_modes
{
	DDP_ROUTING_FPP_MODE__UNDEF = 0,		/*!< Undefined mode */
	DDP_ROUTING_FPP_MODE__ALL,				/*!< All values in range are valid */
	DDP_ROUTING_FPP_MODE__POW2,				/*!< Only powers of two (1,2,4,8,16,32,...) in range are valid */
};

/**
* @enum routing_ready_state
* @brief Routing ready state values for ddp_routing_ready_state_response
*/
enum routing_ready_state
{
	ROUTING_READY_STATE__INVALID = 0,		/*!< Invalid routing ready state */
	ROUTING_READY_STATE__READY = 1,			/*!< Device is ready to route audio data */
	ROUTING_READY_STATE__NOT_READY = 2,		/*!< Device is not ready to route audio data */
};

/**
* @enum routing_ready_state_network_state
* @brief Network ready state values for ddp_routing_ready_state_response
*/
enum routing_ready_state_network_state
{
	NETWORK_READY_STATE__INVALID = 0,		/*!< Invalid network ready state */
	NETWORK_READY_STATE__READY = 1,			/*!< Network interface is ready */
	NETWORK_READY_STATE__NOT_READY = 2,		/*!< Network interface is not ready */
	NETWORK_READY_STATE__NO_IP = 3,			/*!< Network interface has not acquired an IP address */
	NETWORK_READY_STATE__LINK_DOWN = 4		/*!< The device is not connected to the network */
};

/**
* @enum routing_performance_config_flags
* @brief Routing performance configuration flags values for ddp_routing_performance_payload_performance
*/
enum routing_performance_config_flags
{
	DDP_ROUTING_PERFORMANCE_VALID_FLAG__FPP = 1 << 0,	/*!< fpp is valid */
	DDP_ROUTING_PERFORMANCE_VALID_FLAG__LATENCY = 1 << 1	/*!< latency is valid */
};

/**
* @enum routing_performance_config_element_flags
* @brief Routing performance configuration element_flags values for ddp_routing_performance_payload
*/
enum routing_performance_config_element_flags
{
	DDP_ROUTING_PERFORMANCE_FLAG__UNICAST = 1 << 0,	/*!< unicast element is valid */
	DDP_ROUTING_PERFORMANCE_FLAG__MULTICAST = 1 << 1	/*!< multicast element is valid */
};

/**
* @enum ddp_rx_channel_flags
* @brief Routing Rx channel channel_flags values for ddp_routing_rx_channel_data_t
*/
enum ddp_rx_channel_flags
{
	DDP_RX_CHANNEL_FLAG__EXISTS = 1 << 0,	/*!< The Rx channel exists */
	DDP_RX_CHANNEL_FLAG__ENABLED = 1 << 1,	/*!< The Rx channel has been enabled */
	DDP_RX_CHANNEL_FLAG__MUTED = 1 << 3,	/*!< The audio of the Rx channel has been muted */
};

/**
* @enum ddp_routing_rx_chan_status
* @brief Routing Rx channel status values for ddp_routing_rx_channel_data_t
*/
enum ddp_routing_rx_chan_status
{
	DDP_RX_CHAN_STATUS_NONE = 0,						/*!< Channel is not subscribed or otherwise doing anything interesting */
	DDP_RX_CHAN_STATUS_UNRESOLVED = 1,					/*!< Name not yet found */
	DDP_RX_CHAN_STATUS_RESOLVED = 2,					/*!< Name has been found, but not yet processed. This is an transient state */
	DDP_RX_CHAN_STATUS_RESOLVE_FAIL = 3,				/*!< Error: an error occurred while trying to resolve name */
	DDP_RX_CHAN_STATUS_SUBSCRIBE_SELF = 4,				/*!< Channel is successfully subscribed to own TX channels (local loopback mode) */
	DDP_RX_CHAN_STATUS_IDLE = 7,						/*!< A flow has been configured but does not have sufficient information to establish an audio connection. For example, configuring a template with no associations. */
	DDP_RX_CHAN_STATUS_IN_PROGRESS = 8,					/*!< Name has been found and processed; setting up flow. This is an transient state */
	DDP_RX_CHAN_STATUS_DYNAMIC = 9,						/*!< Active subscription to an automatically configured source flow */
	DDP_RX_CHAN_STATUS_STATIC = 10,						/*!< Active subscription to a manually configured source flow */

	DDP_RX_CHAN_STATUS_MANUAL = 14,						/*!< Manual flow configuration bypassing the standard subscription process */
	DDP_RX_CHAN_STATUS_NO_CONNECTION = 15,				/*!< Error: The name was found but the connection process failed (the receiver could not communicate with the transmitter) */
	DDP_RX_CHAN_STATUS_CHANNEL_FORMAT = 16,				/*!< Error: Channel formats do not match */
	DDP_RX_CHAN_STATUS_BUNDLE_FORMAT = 17,				/*!< Error: Flow formats do not match, e.g. Multicast flow with more slots than receiving device can handle */
	DDP_RX_CHAN_STATUS_NO_RX = 18,						/*!< Error: Receiver is out of resources (e.g. flows) */
	DDP_RX_CHAN_STATUS_RX_FAIL = 19,					/*!< Error: Receiver couldn't set up the flow */
	DDP_RX_CHAN_STATUS_NO_TX = 20,						/*!< Error: Transmitter is out of resources (e.g. flows) */
	DDP_RX_CHAN_STATUS_TX_FAIL = 21,					/*!< Error: Transmitter couldn't set up the flow */
	DDP_RX_CHAN_STATUS_QOS_FAIL_RX = 22,				/*!< Error: Receiver got a QoS failure (too much data) when setting up the flow */
	DDP_RX_CHAN_STATUS_QOS_FAIL_TX = 23,				/*!< Error: Transmitter got a QoS failure (too much data) when setting up the flow */
	DDP_RX_CHAN_STATUS_TX_REJECTED_ADDR = 24,			/*!< Error: TX rejected the address given by rx (usually indicates an arp failure) */
	DDP_RX_CHAN_STATUS_INVALID_MSG = 25,				/*!< Error: Transmitter rejected the flow request as invalid */
	DDP_RX_CHAN_STATUS_CHANNEL_LATENCY = 26,			/*!< Error: TX channel latency higher than maximum supported RX latency */
	DDP_RX_CHAN_STATUS_CLOCK_DOMAIN = 27,				/*!< Error: TX and RX and in different clock subdomains */
	DDP_RX_CHAN_STATUS_UNSUPPORTED = 28,				/*!< Error: Attempt to use an unsupported feature */
	DDP_RX_CHAN_STATUS_RX_LINK_DOWN = 29,				/*!< Error: All rx links are down */
	DDP_RX_CHAN_STATUS_TX_LINK_DOWN = 30,				/*!< Error: All tx links are down */
	DDP_RX_CHAN_STATUS_DYNAMIC_PROTOCOL = 31,			/*!< Error: can't find suitable protocol for dynamic connection */
	DDP_RX_CHAN_STATUS_INVALID_CHANNEL = 32,			/*!< Channel does not exist (eg no such local channel) */
	DDP_RX_CHAN_STATUS_TX_SCHEDULER_FAILURE = 33,		/*!< Tx Scheduler failure */

	DDP_RX_CHAN_STATUS_TEMPLATE_MISMATCH_DEVICE = 64,	/*!< Template-based subscription failed: template and subscription device names don't match */
	DDP_RX_CHAN_STATUS_TEMPLATE_MISMATCH_FORMAT = 65,	/*!< Template-based subscription failed: flow and channel formats don't match */
	DDP_RX_CHAN_STATUS_TEMPLATE_MISSING_CHANNEL = 66,	/*!< Template-based subscription failed: the channel is not part of the given multicast flow */
	DDP_RX_CHAN_STATUS_TEMPLATE_MISMATCH_CONFIG = 67,	/*!< Template-based subscription failed: something else about the template configuration made it impossible to complete the subscription using the given flow */
	DDP_RX_CHAN_STATUS_TEMPLATE_FULL = 68,				/*!< Template-based subscription failed: the unicast template is full */

	DDP_RX_CHAN_STATUS_TX_ACCESS_CONTROL_DENIED = 96,	/*!< TX access control denied the request */
	DDP_RX_CHAN_STATUS_TX_ACCESS_CONTROL_PENDING = 97,	/*!< TX access control request is in progress */

	DDP_RX_CHAN_STATUS_SYSTEM_FAIL = 255				/*!< Unexpected system failure. */
};

/**
* @enum ddp_tx_channel_flags
* @brief Routing Tx channel channel_flags values for ddp_routing_tx_channel_data
*/
enum ddp_tx_channel_flags
{
	DDP_TX_CHANNEL_FLAG__EXISTS = 1 << 0,	/*!< The Tx channel exists */
	DDP_TX_CHANNEL_FLAG__ENABLED = 1 << 1,	/*!< The Tx channel has been enabled */
	DDP_TX_CHANNEL_FLAG__ADVERT = 1 << 2,	/*!< The Tx channel has been advertised */
	DDP_TX_CHANNEL_FLAG__MUTED = 1 << 3,	/*!< The audio of the Tx channel has been muted */
};

/**
* @enum ddp_routing_tx_flow_flags
* @brief Routing Tx flow_flags values for ddp_routing_flow_data
*/
enum ddp_routing_tx_flow_flags
{
	DDP_ROUTING_TXFLOW_FLAG__PERSISTENT = 1 << 0,	/*!< Indicates that this Tx flow is persistent */
	DDP_ROUTING_TXFLOW_FLAG__AUTO = 1 << 1,	/*!< Indicates that this is an auto Tx flow */
	DDP_ROUTING_TXFLOW_FLAG__ADVERTISED = 1 << 2,	/*!< Indicates that this Tx flow has been advertised */
};

/**
* @enum ddp_routing_rx_flow_flags
* @brief Routing Rx flow_flags values for ddp_routing_flow_data
*/
enum ddp_routing_rx_flow_flags
{
	DDP_ROUTING_RXFLOW_FLAG__PERSISTENT = 1 << 0,	/*!< Indcates that this Rx flow is persistent */
	DDP_ROUTING_RXFLOW_FLAG__AUTO = 1 << 1,	/*!< Indicates that this is an auto Rx flow */
};

/**
* @enum ddp_routing_flow_user_flags
* @brief Routing flow user flag values
*
*	Flow user configuration flags only apply on responses, and indicate which
*	fields were explicitly set by the user and which have been configured from
*	defaults.  The mask indicates which flags are recognised by the device.
*
*	Interpreting flags:
*		mask = 1, flag = 1: field value was explicitly set by user
*		mask = 1, flag = 0: field value was generated by device
*		mask = 0, flag = *: device doesn't consider field to be user-configurable
*
*	Flow user configuration flags are ignored on request.  On request, the device
*	will treat any non-null fields as explicit configuration, and infer legal
*	values for any null fields.  The new configuration completely replaces any
*	existing configuration for that flow ID, although if there are no significant
*	changes then audio will not be interrupted (eg setting a flow from 'default'
*	PCM-24 to explicit PCM-24 will not disrupt audio as the net flow configuration
*	is unchanged, but the user flags will be changed in the response).
*/
enum ddp_routing_flow_user_flags
{
	DDP_ROUTING_FLOW_USER_FLAG__NUM_ADDRS = 1 << 0,	/*!< Number of addresses can be set independently of the value of the addresses */
	DDP_ROUTING_FLOW_USER_FLAG__ADDRS = 1 << 1,	/*!< Indicates that address of the flow is set */
	DDP_ROUTING_FLOW_USER_FLAG__SLOTS = 1 << 3,	/*!< Indicates that the slots of the flow is set */
	DDP_ROUTING_FLOW_USER_FLAG__SAMPLE_RATE = 1 << 4,	/*!< Indicates that the sample rate of the flow is set */
	DDP_ROUTING_FLOW_USER_FLAG__ENCODING = 1 << 5,	/*!< Indicates that the encoding of the flow is set */
	DDP_ROUTING_FLOW_USER_FLAG__LATENCY = 1 << 6,	/*!< Indicates that the latency of the flow is set */
	DDP_ROUTING_FLOW_USER_FLAG__FPP = 1 << 7	/*!< Indicates that the FPP of the flow is set */
};

/**
* @enum ddp_routing_flow_status
* @brief Routing status values for Rx flows
*/
enum ddp_routing_flow_status
{
	FLOW_STATUS_NONE = 0,	/*!< No flow exists */
	FLOW_STATUS_ACTIVE = 1,	/*!< A flow is configured and audio data is beinng sent or received */
	FLOW_STATUS_INACTIVE = 2,	/*!< A flow is configured but no audio is being received (rx only) */
	FLOW_STATUS_BAD_ADDRESS = 3,	/*!< A flow could not be created as the address was bad */
	FLOW_STATUS_QOS = 4,	/*!< A flow could not be created due to QoS restrictions */
	FLOW_STATUS_INVALID = 5,	/*!< Get status for non-existent flow */
	FLOW_STATUS_PENDING = 6,	/*!< A flow is partially configured but is waiting on device processes to complete */
};

/**
* @enum ddp_routing_flow_filter
* @brief Routing flow delete filter values for ddp_routing_flow_delete_request_payload
*/
enum ddp_routing_flow_filter
{
	DDP_ROUTING_FLOW_FILTER__RX_MANUAL = 0x1,		/*!< Rx manual flows */
	DDP_ROUTING_FLOW_FILTER__RX_AUTO = 0x2,		/*!< Rx auto flows */
	DDP_ROUTING_FLOW_FILTER__TX_MANUAL = 0x100,	/*!< Tx manual flows */
	DDP_ROUTING_FLOW_FILTER__TX_AUTO = 0x200,	/*!< Tx auto flows */
};

/** @cond */
typedef enum ddp_routing_flow_filter ddp_routing_flow_filter_t;
/** @endcond */

/**@}*/

/**
* \defgroup RoutingStructs DDP Routing Structures
* @{
*/

/**
* @struct ddp_routing_basic_response_payload
* @brief Structure format for the payload of the "routing basic response" message
*/
struct ddp_routing_basic_response_payload
{
	uint32_t cap_flags;						/*!< Currently none defined use zero */
	uint16_t num_intf;						/*!< Number of network interfaces */
	uint16_t pad0;							/*!< Pad for alignment - must be '0' */
	uint16_t max_rx_flows;					/*!< Maximum number of audio receive flows */
	uint16_t max_tx_flows;					/*!< Maximum number of audio transmit flows */
	uint16_t max_rx_slots;					/*!< Maximum number of audio receive slots in a packet */
	uint16_t max_tx_slots;					/*!< Maximum number of audio transmit slots in a packet */
	uint32_t min_rx_latency;				/*!< Minimum audio receive latency */
	uint32_t max_rx_latency;				/*!< Maximum audio receive latency */
	uint16_t min_rx_fpp;					/*!< Minimum audio receive frames per packet  */
	uint16_t max_rx_fpp;					/*!< Maximum audio receive frames per packet */
	uint16_t min_tx_fpp;					/*!< Minimum audio transmit frames per packet */
	uint16_t max_tx_fpp;					/*!< Maximum audio transmit frames per packet */
	uint16_t fpp_mode;						/*!< The frames per packet mode @see routing_basic_fpp_modes */
	uint16_t pad1;							/*!< Pad for alignment - must be '0' */
	uint16_t min_rx_dynamic_port;			/*!< Minimum dynamic UDP/IP port */
	uint16_t max_rx_dynamic_port;			/*!< Maximum dynamic UDP/IP port */
	uint16_t min_rx_static_port;			/*!< Minimum static UDP/IP port */
	uint16_t max_rx_static_port;			/*!< Maximum static UDP/IP port */
	uint16_t max_tx_label_per_channel;		/*!< Maximum number of Tx labels that can be set for a channel */
	uint16_t max_tx_label_total;			/*!< Number of labels per channel times the number of channels */
	uint8_t rx_chan_state_per_ddp_msg;		/*!< Maximum number of channels that can be accommodated in a single Rx channel config state DDP message. This sets the upper limit on the query */
	uint8_t tx_chan_state_per_ddp_msg;		/*!< Maximum number of channels that can be accommodated in a single Tx channel config state DDP message. This sets the upper limit on the query */
	uint8_t rx_flow_state_per_ddp_msg;		/*!< Maximum number of Rx flows that can be accommodated in a single Rx flow config state DDP message. This sets the upper limit on the query */
	uint8_t tx_flow_state_per_ddp_msg;		/*!< Maximum number of Tx flows that can be accommodated in a single Tx flow config state DDP message. This sets the upper limit on the query */
	uint8_t rx_chan_status_per_ddp_msg;		/*!< Maximum number of Rx channels that can be accommodated in a single Rx channel status DDP message. This sets the upper limit on the query */
	uint8_t rx_flow_status_per_ddp_msg;		/*!< Maximum number of Rx flows that can be accommodated in a single Rx flow status DDP message. This sets the upper limit on the query */
	uint16_t pad2;							/*!< Pad for alignment - must be '0' */
};

//! @cond Doxygen_Suppress
typedef struct ddp_routing_basic_response_payload ddp_routing_basic_response_payload_t;
//! @endcond Doxygen_Suppress

/**
* @struct ddp_routing_basic_request
* @brief Structure format for the "routing basic request" message
*/
struct ddp_routing_basic_request
{
	ddp_message_header_t header;			/*!< Message header */
	ddp_request_subheader_t subheader;		/*!< Request message subheader */
};

/**
* @struct ddp_routing_basic_response
* @brief Structure format for the "routing basic response" message
*/
struct ddp_routing_basic_response
{
	ddp_message_header_t header;					/*!< Message header */
	ddp_response_subheader_t subheader;				/*!< Response message subheader */

	struct ddp_routing_basic_response_payload payload;	/*!< Payload @see ddp_routing_basic_response_payload */
};

/**
* @struct ddp_routing_ready_state_request
* @brief Structure format for the "routing ready state request" message
*/
struct ddp_routing_ready_state_request
{
	ddp_message_header_t header;			/*!< Message header */
	ddp_request_subheader_t subheader;		/*!< Request message subheader */
};

/**
* @struct ddp_routing_ready_response_payload
* @brief Structure format for the payload of the "routing ready state response" message
*/
struct ddp_routing_ready_response_payload
{
	int8_t network_ready_state;				/*!< Current state of the device network interface @see routing_ready_state_network_state */
	int8_t routing_ready_state;				/*!< Current state of the device routing software @see routing_ready_state */
	uint16_t pad;							/*!< Pad for alignment - must be '0' */
};

/** @cond */
typedef struct ddp_routing_ready_response_payload ddp_routing_ready_response_payload_t;
/** @endcond */

/**
* @struct ddp_routing_ready_state_response
* @brief Structure format for the "routing ready state response" message
*/
struct ddp_routing_ready_state_response
{
	ddp_message_header_t header;					/*!< Message header */
	ddp_response_subheader_t subheader;				/*!< Response message subheader */

	ddp_routing_ready_response_payload_t payload;	/*!< Payload @see ddp_routing_ready_response_payload */
};

/**
* @struct ddp_routing_performance_payload_performance
* @brief Structure format for the unicast and multicast performance data of the "routing performance configuration request" message
*/
struct ddp_routing_performance_payload_performance
{
	uint16_t flags;					/*!< Bitwise OR'd flags @see routing_performance_config_flags */
	uint16_t fpp;					/*!< Audio frames per packet, valid only if the DDP_ROUTING_PERFORMANCE_VALID_FLAG__FPP flag is set in the flags field */
	uint32_t latency;				/*!< Audio latency in nano seconds, valid only if the DDP_ROUTING_PERFORMANCE_VALID_FLAG__LATENCY flag is set in the flags field */
};

/** @cond */
typedef struct ddp_routing_performance_payload_performance ddp_routing_performance_payload_performance_t;
/** @endcond */

/**
* @struct ddp_routing_performance_payload
* @brief Structure format for the payload of the "routing performance configuration request" message
*/
struct ddp_routing_performance_payload
{
	uint16_t element_flags;										/*!< Bitwise OR'd element flags @see routing_performance_config_element_flags */
	uint16_t element_size;										/*!< Size of the performance information structure in bytes @see ddp_routing_performance_payload_performance, currently the size of the structure is 8 bytes */

	ddp_routing_performance_payload_performance_t unicast;		/*!< Unicast flow performance information @see ddp_routing_performance_payload_performance, valid only if the DDP_ROUTING_PERFORMANCE_FLAG__UNICAST flag is set in the element_flags */
	ddp_routing_performance_payload_performance_t multicast;	/*!< Multicast flow performance information @see ddp_routing_performance_payload_performance, valid only if the DDP_ROUTING_PERFORMANCE_FLAG__MULTICAST flag is set in the element_flags */
};

/** @cond */
typedef struct ddp_routing_performance_payload ddp_routing_performance_payload_t;
/** @endcond */

/**
* @struct ddp_routing_performance_config_request
* @brief Structure format for the "routing performance config request" message
*/
struct ddp_routing_performance_config_request
{
	ddp_message_header_t header;						/*!< Message header */
	ddp_request_subheader_t subheader;					/*!< Request message subheader */

	ddp_routing_performance_payload_t payload;			/*!< Payload @see ddp_routing_performance_payload */
};

/**
* @struct ddp_routing_performance_config_response
* @brief Structure format for the "routing performance config response" message
*/
struct ddp_routing_performance_config_response
{
	ddp_message_header_t header;						/*!< Message header */
	ddp_response_subheader_t subheader;					/*!< Response message subheader */

	ddp_routing_performance_payload_t payload;			/*!< Payload @see ddp_routing_performance_payload */
};

/**
* @struct ddp_id_range
* @brief Structure format for the ID ranges used for "routing Rx/Tx channel configuration state/status request" messages
*/
struct ddp_id_range
{
	uint16_t from;				/*!< Starting ID value, this value should be non-zero */
	uint16_t to;				/*!< Ending ID value, this value should be non-zero */
};

/** @cond */
typedef struct ddp_id_range ddp_id_range_t;
/** @endcond */

/**
* @struct ddp_id_ranges
* @brief Structure format for the heap section of the payload of the "routing Rx/Tx channel configuration state/status request" messages
*/
struct ddp_id_ranges
{
	uint16_t curr;				/*!< Number of valid elements in the array */
	uint16_t max;				/*!< Maximum number of elements in the array (fixed at creation) */
	ddp_id_range_t range[1];	/*!< Pair of range values @see ddp_id_range */
};

/** @cond */
typedef struct ddp_id_ranges ddp_id_ranges_t;
/** @endcond */

/**
* @struct ddp_id_range_payload
* @brief Structure format for the payload of the "routing Rx/Tx channel configuration state/status request" messages
*/
struct ddp_id_range_payload
{
	ddp_raw_offset_t ranges_offset;		/*!< Offset to a structure that contains information about the channel ID or flow ID range */
	uint16_t pad0;						/*!< Pad for alignment - must be '0' */
};

/** @cond */
typedef struct ddp_id_range_payload ddp_id_range_payload_t;
/** @endcond */

/**
* @struct ddp_routing_rx_chan_config_state_request
* @brief Structure format for the "routing Rx channel configuration state request" message
*/
struct ddp_routing_rx_chan_config_state_request
{
	ddp_message_header_t header;						/*!< Message header */
	ddp_request_subheader_t subheader;					/*!< Request message subheader */

	ddp_id_range_payload_t payload;						/*!< Payload @see ddp_id_range_payload_t */

	// Heap goes here
};

/**
* @struct ddp_offset_array_payload
* @brief Structure format for the payload of the "routing Rx/Tx channel config state response" message
*/
struct ddp_offset_array_payload
{
	ddp_raw_offset_t array_offset;		/*!< Offset from the start of the TLV0 header to the start of the a ddp_array structure @see ddp_array_t */
	uint16_t pad0;						/*!< Pad for alignment - must be '0' */
};

//! @cond Doxygen_Suppress
typedef struct ddp_offset_array_payload ddp_offset_array_payload_t;
//! @endcond Doxygen_Suppress

/**
* @struct ddp_array
* @brief Structure format for part of the heap section of the "routing Rx channel config state response" message
*/
struct ddp_array
{
	/*!< @struct ddp_array_header */
	struct ddp_array_header
	{
		uint16_t curr;				/*!< Number of valid elements in the array */
		uint16_t max;				/*!< Maximum number of elements in the array (fixed at creation) */
	} header;  /*!< header */
	ddp_raw_offset_t offset[1];		/*!< Array of offsets to elements */
};

/** @cond */
typedef struct ddp_array ddp_array_t;
/** @endcond */

/**
* @struct ddp_routing_channel_format
* @brief Structure format for the channel format data poplulated in the heap of the "routing Rx/Tx channel config state response" message
*/
struct ddp_routing_channel_format
{
	uint16_t size;									/*!< Size of this structure */
	uint16_t preferred_encoding;					/*!< Preferred audio encoding value @see audio_supported_encoding used for the Rx/Tx channel */
	uint32_t sample_rate;							/*!< Sample rate used for the Rx/Tx channel */
	uint16_t pcm_map;								/*!< Bitmask of supported PCM encodings, in bytes (eg 0x4 => 24 bits) */
	uint16_t num_custom_encodings;					/*!< Number of custom non-PCM encodings supported by this channel */
	ddp_raw_offset_t custom_encodings_offset;		/*!< Offset from the start of the TLV0 header to the start of the custom encodings array @see ddp_custom_enc_array_t */
	uint16_t pad0;									/*!< Pad for alignment - must be '0' */
};

/** @cond */
typedef struct ddp_routing_channel_format ddp_routing_channel_format_t;
/** @endcond */

/**
* @struct ddp_routing_rx_channel_data
* @brief Structure format for the Rx channel state information populated in the heap of the "routing Rx channel config state response" message
*/
struct ddp_routing_rx_channel_data
{
	uint16_t size;							/*!< Size of this structure */
	uint16_t channel_id;					/*!< ID of this Rx channel */
	ddp_raw_offset_t name_offset;			/*!< Offset from the start of the TLV0 header to the start of the null-terminated default Rx channel name string */
	ddp_raw_offset_t format_offset;			/*!< Offset from the start of the TLV0 header to the start of the structure that has information about the audio format associated with the Rx channel @see ddp_routing_channel_format_t */
	uint16_t channel_flags;					/*!< Channel flags @see ddp_rx_channel_flags */
	uint16_t channel_flags_mask;			/*!< Bitwise OR'd mask which specify which channel_flags are valid */
	ddp_raw_offset_t label_offset;			/*!< Offset from the start of the TLV0 header to the start of the null-terminated channel label string */
	uint16_t pad0;							/*!< Pad for alignment - must be '0' */
	uint16_t status;						/*!< Status of the Rx channel @see ddp_routing_rx_chan_status */
	uint8_t avail;							/*!< A non-zero value indicates that this channel is available to receive audio */
	uint8_t active;							/*!< A non-zero value indicates that this channel is active */
	ddp_raw_offset_t sub_channel_offset;	/*!< Offset from the start of the TLV0 header to the start of the null-terminated Tx subscribed channel string */
	ddp_raw_offset_t sub_device_offset;		/*!< Offset from the start of the TLV0 header to the start of the null-terminated Tx subscribed device string */
	uint16_t flow_id;						/*!< Flow ID associated with this Rx channel */
	uint16_t slot_id;						/*!< Slot ID associated with this Rx channel */
};

/** @cond */
typedef struct ddp_routing_rx_channel_data ddp_routing_rx_channel_data_t;
/** @endcond */

/**
* @struct ddp_routing_rx_chan_config_state_response
* @brief Structure format for the "routing Rx channel configuration state response" message
* @note Due to buffer size limitations there is an upper bound on the amount of channels that be populated in this message. The rx_chan_state_per_ddp_msg field in the DDP routing basic response message specifies the upper bound. @see ddp_routing_basic_response_payload
*/
struct ddp_routing_rx_chan_config_state_response
{
	ddp_message_header_t header;						/*!< Message header */
	ddp_response_subheader_t subheader;					/*!< Response message subheader */

	ddp_offset_array_payload_t payload;					/*!< @see ddp_offset_array_payload and @see ddp_routing_rx_channel_data */

	// Heap goes here
};

/**
* @struct ddp_routing_tx_chan_config_state_request
* @brief Structure format for the "routing Tx channel configuration state request" message
*/
struct ddp_routing_tx_chan_config_state_request
{
	ddp_message_header_t header;						/*!< Message header */
	ddp_request_subheader_t subheader;					/*!< Request message subheader */

	ddp_id_range_payload_t payload;						/*!< Payload @see ddp_id_range_payload_t */

	// Heap goes here
};

/**
* @struct ddp_routing_tx_channel_data
* @brief Structure format for the Tx channel state information populated in the heap of the "routing Tx channel config state response" message
*/
struct ddp_routing_tx_channel_data
{
	uint16_t size;							/*!< Size of this structure */
	uint16_t channel_id;					/*!< ID of this Tx channel */
	ddp_raw_offset_t name_offset;			/*!< Offset from the start of the TLV0 header to the start of the null-terminated default Tx channel name string */
	ddp_raw_offset_t format_offset;			/*!< Offset from the start of the TLV0 header to the start of the structure that has information about the audio format associated with the Tx channel @see ddp_routing_channel_format_t */
	uint16_t channel_flags;					/*!< Channel flags @see ddp_tx_channel_flags */
	uint16_t channel_flags_mask;			/*!< Bitwise OR'd mask which specify which channel_flags are valid */
	ddp_raw_offset_t label_offset;			/*!< Offset from the start of the TLV0 header to the start of the null-terminated channel label string */
	uint16_t pad0;							/*!< Pad for alignment - must be '0' */
};

/** @cond */
typedef struct ddp_routing_tx_channel_data ddp_routing_tx_channel_data_t;
/** @endcond */

/**
* @struct ddp_routing_tx_chan_config_state_response
* @brief Structure format for the "routing Tx channel config state response" message
* @note Due to buffer size limitations there is an upper bound on the amount of channels that be populated in this message. The tx_chan_state_per_ddp_msg field in the DDP routing basic response message specifies the upper bound. @see ddp_routing_basic_response_payload
*/
struct ddp_routing_tx_chan_config_state_response
{
	ddp_message_header_t header;						/*!< Message header */
	ddp_response_subheader_t subheader;					/*!< Response message subheader */

	ddp_offset_array_payload_t payload;					/*!< @see ddp_offset_array_payload and @see ddp_routing_tx_channel_data */

	// Heap goes here
};

/**
* @struct ddp_routing_rx_chan_config_status_request
* @brief Structure format for the "routing Rx channel configuration status request" message
*/
struct ddp_routing_rx_chan_config_status_request
{
	ddp_message_header_t header;						/*!< Message header */
	ddp_request_subheader_t subheader;					/*!< Request message subheader */

	ddp_id_range_payload_t payload;						/*!< Payload @see ddp_id_range_payload */

	// Heap goes here
};

/**
* @struct ddp_routing_rx_channel_status
* @brief Structure format for the Rx channel status information populated in the heap of the "routing Rx channel config status response" message
*/
struct ddp_routing_rx_channel_status
{
	uint16_t channel_id;						/*!< ID of this Rx channel */
	uint16_t status;							/*!< Status of the Rx channel @see ddp_routing_rx_chan_status */
	uint8_t avail;								/*!< A non-zero value indicates that this channel is available to receive audio */
	uint8_t active;								/*!< A non-zero value indicates that this channel is active */
	uint16_t pad;								/*!< Pad for alignment - must be '0' */
};

/** @cond */
typedef struct ddp_routing_rx_channel_status ddp_routing_rx_channel_status_t;
/** @endcond */

/**
* @struct ddp_routing_rx_chan_status_response
* @brief Structure format for the "routing Rx channel config status response" message
* @note Due to buffer size limitations there is an upper bound on the amount of channels that be populated in this message. The rx_chan_status_per_ddp_msg field in the DDP routing basic response message specifies the upper bound. @see ddp_routing_basic_response_payload
*/
struct ddp_routing_rx_chan_status_response
{
	ddp_message_header_t header;						/*!< Message header */
	ddp_response_subheader_t subheader;					/*!< Response message subheader */

	ddp_offset_array_payload_t payload;					/*!< @see ddp_offset_array_payload_t and @see ddp_routing_rx_channel_status */

	// Heap goes here
};

/**
* @struct ddp_routing_rx_flow_config_state_request
* @brief Structure format for the "routing Rx flow configuration state request" message
*/
struct ddp_routing_rx_flow_config_state_request
{
	ddp_message_header_t header;						/*!< Message header */
	ddp_request_subheader_t subheader;					/*!< Request message subheader */

	ddp_id_range_payload_t payload;						/*!< Payload @see ddp_id_range_payload */

	// Heap goes here
};

/**
* @struct ddp_routing_flow_data
* @brief Structure format for the Rx/Tx flow config state information populated in the heap of the "routing Rx/Tx flow config state response" message
*/
struct ddp_routing_flow_data
{
	// Flow properties
	uint16_t size;								/*!< Size of this structure in bytes */
	uint16_t flow_id;							/*!< Non-zero ID of this flow */
	ddp_raw_offset_t label_offset;				/*!< Offset from the start of the TLV0 header to the start of the null-terminated flow label string */
	uint16_t status;							/*!< Current status of the flow @see ddp_routing_flow_status */

	uint16_t flow_flags;						/*!< Flow flags for Rx @see ddp_routing_rx_flow_flags or Tx @see ddp_routing_tx_flow_flags */
	uint16_t flow_flags_mask;					/*!< Bitwise OR'd mask which specify which channel_flags are valid */

	// User configuration information
	uint32_t user_conf_flags;					/*!< Bitwise OR'd user flow flags @see ddp_routing_flow_user_flags */
	uint32_t user_conf_mask;					/*!< Bitwise OR'd mask to indicate which of the flow flags are valid */

	// Peer properties
	uint16_t peer_flow_id;						/*!< Flow ID of the peer, e.g. if this is a Rx flow, then this is the Tx flow ID */
	ddp_raw_offset_t peer_flow_label_offset;	/*!< Offset from the start of the TLV0 header to the start of the null-terminated peer flow label string */
	uint32_t peer_device_id[2];					/*!< 8-byte Dante device ID */
	uint16_t peer_process_id;					/*!< Process ID of the peer device */
	ddp_raw_offset_t peer_device_label_offset;	/*!< Offset from the start of the TLV0 header to the start of the null-terminated peer device label string */

	// Transport & channel properties
	uint8_t transport;					/*!< Audio transport protocol, currently only DDP_AUDIO_TRANSPORT_ATP is supported */
	uint8_t pad0;						/*!< Pad for alignment - must be '0' */
	uint8_t avail_mask;					/*!< Bitmask of configured interfaces (see also addresses) */
	uint8_t active_mask;				/*!< Bitmask of active interfaces */
	uint32_t sample_rate;				/*!< Audio sample rate used for this flow */
	uint32_t latency;					/*!< Audio latency in nano seconds used for this flow */
	uint16_t encoding;					/*!< Audio encoding used for this flows @see audio_supported_encoding */
	uint16_t fpp;						/*!< Frames per packet used for this flow */
	uint16_t num_slots;					/*!< Number of slots */
	uint16_t slot_size;					/*!< Size of each slot in bytes, this value currently is 2 bytes */
	ddp_raw_offset_t slots_offset;		/*!< Offset from the start of the TLV0 header to the start of the array of audio channel slots */
	uint16_t num_addresses;				/*!< Number of IP addresses and ports associated with this flow */
	uint16_t addr_size;					/*!< Size of each IP address and port element in bytes */
	ddp_raw_offset_t addr_offset;		/*!< Offset from the start of the TLV0 header to the start of the array of IP addresses and ports */
};

/** @cond */
typedef struct ddp_routing_flow_data ddp_routing_flow_data_t;
/** @endcond */

/**
* @struct ddp_routing_rx_flow_config_state_response
* @brief Structure format for the "routing Rx flow config state response" message
* @note Due to buffer size limitations there is an upper bound on the amount of channels that be populated in this message. The rx_flow_state_per_ddp_msg field in the DDP routing basic response message specifies the upper bound. @see ddp_routing_basic_response_payload
*/
struct ddp_routing_rx_flow_config_state_response
{
	ddp_message_header_t header;						/*!< Message header */
	ddp_response_subheader_t subheader;					/*!< Response message subheader */

	ddp_offset_array_payload_t payload;					/*!< @see ddp_offset_array_payload_t and @see ddp_routing_flow_data */

	// Heap goes here
};

/**
* @struct ddp_routing_tx_flow_config_state_request
* @brief Structure format for the "routing Tx flow configuration state request" message
*/
struct ddp_routing_tx_flow_config_state_request
{
	ddp_message_header_t header;						/*!< Message header */
	ddp_request_subheader_t subheader;					/*!< Request message subheader */

	ddp_id_range_payload_t payload;						/*!< Payload @see ddp_id_range_payload_t */

	// Heap goes here
};

/**
* @struct ddp_routing_tx_flow_config_state_response
* @brief Structure format for the "routing Tx flow config state response" message
* @note Due to buffer size limitations there is an upper bound on the amount of flows that be populated in this message. The tx_flow_state_per_ddp_msg field in the DDP routing basic response message specifies the upper bound. @see ddp_routing_basic_response_payload
*/
struct ddp_routing_tx_flow_config_state_response
{
	ddp_message_header_t header;						/*!< Message header */
	ddp_response_subheader_t subheader;					/*!< Response message subheader */

	ddp_offset_array_payload_t payload;					/*!< @see ddp_offset_array_payload_t and @see ddp_routing_flow_data */

	// Heap goes here
};

/**
* @struct ddp_routing_rx_flow_status_request
* @brief Structure format for the "routing Rx flow configuration status request" message
*/
struct ddp_routing_rx_flow_status_request
{
	ddp_message_header_t header;						/*!< Message header */
	ddp_request_subheader_t subheader;					/*!< Request message subheader */

	ddp_id_range_payload_t payload;						/*!< Payload @see ddp_id_range_payload_t */

	// Heap goes here
};

/**
* @struct ddp_routing_rx_flow_status
* @brief Structure format for the Rx channel status information populated in the heap of the "routing Rx channel config status response" message
*/
struct ddp_routing_rx_flow_status
{
	uint16_t flow_id;						/*!< ID of this Rx flow */
	uint16_t status;						/*!< Status of the Rx flow @see ddp_routing_rx_flow_status */
	uint8_t avail;							/*!< A non-zero value indicates that this flow is available to receive audio */
	uint8_t active;							/*!< A non-zero value indicates that this flow is active */
	uint16_t pad;							/*!< Pad for alignment - must be '0' */
};

/** @cond */
typedef struct ddp_routing_rx_flow_status ddp_routing_rx_flow_status_t;
/** @endcond */

/**
* @struct ddp_routing_rx_flow_status_response
* @brief Structure format for the "routing Rx flow config status response" message
* @note Due to buffer size limitations there is an upper bound on the amount of flows that be populated in this message. The rx_flow_status_per_ddp_msg field in the DDP routing basic response message specifies the upper bound. @see ddp_routing_basic_response_payload
*/
struct ddp_routing_rx_flow_status_response
{
	ddp_message_header_t header;						/*!< Message header */
	ddp_response_subheader_t subheader;					/*!< Response message subheader */

	ddp_offset_array_payload_t payload;					/*!< @see ddp_offset_array_payload_t and @see ddp_routing_rx_flow_status */

	// Heap goes here
};

/**
* @struct ddp_name_id_pair
* @brief Structure format of the heap payload section for the "routing Rx or Tx channel request" messages
*/
struct ddp_name_id_pair
{
	uint16_t ident;						/*!< Rx or Tx channel ID */
	ddp_raw_offset_t name_offset;		/*!< Offset from the start of the TLV0 header to the start of the Rx or Tx channel label name */
};

/** @cond */
typedef struct ddp_name_id_pair ddp_name_id_pair_t;
/** @endcond */

/**
* @struct ddp_array_payload
* @brief Structure format of the payload for the "routing Rx or Tx channel request" and "routng subscribe request" messages
*/
struct ddp_array_payload
{
	uint16_t num_elements;			/*!< Number of Rx or Tx channels to modify */
	uint16_t element_size;			/*!< Size of each label structure in bytes @see ddp_name_id_pair_t, this field should be set to 4 bytes */
	uint16_t array_offset;			/*!< Offset from the start of the TLV0 header to the start of the label structure @see ddp_name_id_pair_t */
	uint16_t pad0;					/*!< Pad for alignment - must be '0' */
};

/** @cond */
typedef struct ddp_array_payload ddp_array_payload_t;
/** @endcond */

/**
* @struct ddp_routing_rx_chan_label_set_request
* @brief Structure format for the "routing Rx channel label request" message
* @note The maximum amount channels specified in this message is dictated by the rx_chan_state_per_ddp_msg field in the DDP routing basic message. @see ddp_routing_basic_response_payload
*/
struct ddp_routing_rx_chan_label_set_request
{
	ddp_message_header_t header;						/*!< Message header */
	ddp_request_subheader_t subheader;					/*!< Request message subheader */

	ddp_array_payload_t payload;						/*!< Channel label payload @see ddp_array_payload */

	// Heap goes here
};

/**
* @struct ddp_routing_rx_chan_label_set_response
* @brief Structure format for the "routing Rx channel label response" message
*/
struct ddp_routing_rx_chan_label_set_response
{
	ddp_message_header_t header;					/*!< Message header */
	ddp_response_subheader_t subheader;				/*!< Response message subheader */

	ddp_offset_array_payload_t payload;				/*!< @see ddp_offset_array_payload */

	// Heap goes here
};

/**
* @struct ddp_routing_tx_chan_label_set_request
* @brief Structure format for the "routing Tx channel label request" message
* @note The maximum amount channels specified in this message is dictated by the tx_chan_state_per_ddp_msg field in the DDP routing basic message. @see ddp_routing_basic_response_payload
*/
struct ddp_routing_tx_chan_label_set_request
{
	ddp_message_header_t header;						/*!< Message header */
	ddp_request_subheader_t subheader;					/*!< Request message subheader */

	ddp_array_payload_t payload;						/*!< Channel label payload @see ddp_array_payload_t */

	// Heap goes here
};

/**
* @struct ddp_routing_tx_chan_label_set_response
* @brief Structure format for the "routing Tx channel label response" message
*/
struct ddp_routing_tx_chan_label_set_response
{
	ddp_message_header_t header;						/*!< Message header */
	ddp_response_subheader_t subheader;					/*!< Response message subheader */

	ddp_offset_array_payload_t payload;					/*!< @see ddp_offset_array_payload_t */

	// Heap goes here
};

/**
* @struct ddp_routing_sub_data
* @brief Structure format of the heap section for the "routing Rx subscribe channel request" message
*/
struct ddp_routing_sub_data
{
	uint16_t chan_id;						/*!< Non-zero Rx channel ID */
	uint16_t pad0;							/*!< Pad for alignment - must be '0' */
	ddp_raw_offset_t sub_chan_offset;		/*!< Offset from the start of the TLV0 header to the start of the non-zero Tx channel ID */
	ddp_raw_offset_t sub_device_offset;		/*!< Offset from the start of the TLV0 header to the start of the null-terminated Tx device name */
};

/** @cond */
typedef struct ddp_routing_sub_data ddp_routing_sub_data_t;
/** @endcond */

/**
* @struct ddp_routing_rx_sub_set_request
* @brief Structure format for the "routing Tx channel label request" message
* @note The maximum amount of channels that can be subscribed at once is dictated by the rx_chan_state_per_ddp_msg field in the DDP routing basic message. @see ddp_routing_basic_response_payload
*/
struct ddp_routing_rx_sub_set_request
{
	ddp_message_header_t header;						/*!< Message header */
	ddp_request_subheader_t subheader;					/*!< Request message subheader */

	ddp_array_payload_t payload;						/*!< Channel label payload @see ddp_array_payload_t */

	// Heap goes here
};

/**
* @struct ddp_routing_rx_sub_set_response
* @brief Structure format for the "routing Rx subscribe channel response" message
*/
struct ddp_routing_rx_sub_set_response
{
	ddp_message_header_t header;					/*!< Message header */
	ddp_response_subheader_t subheader;				/*!< Response message subheader */

	ddp_offset_array_payload_t payload;				/*!< @see ddp_offset_array_payload_t */

	// Heap goes here
};

/**
* @struct ddp_routing_rx_unsub_chan_request
* @brief Structure format for the "routing Rx unsubscribe channel request" message
*/
struct ddp_routing_rx_unsub_chan_request
{
	ddp_message_header_t header;						/*!< Message header */
	ddp_request_subheader_t subheader;					/*!< Request message subheader */

	ddp_id_range_payload_t payload;						/*!< Payload @see ddp_id_range_payload_t */

	// Heap goes here
};

/**
* @struct ddp_routing_rx_unsub_chan_response
* @brief Structure format for the "routing Rx unsubscribe channel request" message
*/
struct ddp_routing_rx_unsub_chan_response
{
	ddp_message_header_t header;					/*!< Message header */
	ddp_response_subheader_t subheader;				/*!< Response message subheader */
};

/**
* @struct ddp_routing_multicast_tx_flow_config_request
* @brief Structure format for the "routing multicast Tx flow configuration request" message
*/
struct ddp_routing_multicast_tx_flow_config_request
{
	ddp_message_header_t header;							/*!< Message header */
	ddp_request_subheader_t subheader;						/*!< Request message subheader */

	struct
	{
		uint16_t size;										/*!< Size of this structure in bytes */
		uint16_t flow_id;									/*!< Non-zero ID of this flow */
		ddp_raw_offset_t label_offset;						/*!< Offset from the start of the TLV0 header to the start of the null-terminated flow label string */
		uint16_t pad0[13];									/*!< Pad for alignment - must be '0' */
		uint8_t transport;
		uint8_t pad1;
		uint32_t sample_rate;								/*!< Audio sample rate used for this flow */
		uint32_t latency;									/*!< Audio latency in nano seconds used for this flow */
		uint16_t encoding;									/*!< Audio encoding used for this flows @see audio_supported_encoding */
		uint16_t fpp;										/*!< Frames per packet used for this flow */
		uint16_t num_slots;									/*!< Number of audio transmit channels for this flow */
		uint16_t slot_size;									/*!< Pad for alignment - must be '0' */
		ddp_raw_offset_t slots_offset;						/*!< Offset from the start of the TLV0 header to the start of the array of slots */
		uint16_t pad2[3];									/*!< Pad for alignment - must be '0' */
	} payload;  /*!< fixed payload */

	// Heap goes here
};

/**
* @struct ddp_routing_multicast_tx_flow_config_response
* @brief Structure format for the "routing multicast Tx flow configuration response" message
*/
struct ddp_routing_multicast_tx_flow_config_response
{
	ddp_message_header_t header;						/*!< Message header */
	ddp_response_subheader_t subheader;					/*!< Response message subheader */

	ddp_offset_array_payload_t payload;					/*!< @see ddp_offset_array_payload */

	// Heap goes here
};

/**
* @struct ddp_routing_flow_delete_request_payload
* @brief Structure format for the payload of the "routing flow delete request" message
*/
struct ddp_routing_flow_delete_request_payload
{
	ddp_raw_offset_t flow_ids_offset;				/*!< Offset to flow ID ranges structure @see ddp_id_range_payload_t */
	uint16_t filter;								/*!< Bitwise OR'd flags that specify which types of flows to delete @see ddp_routing_flow_filter */
};

/** @cond */
typedef struct ddp_routing_flow_delete_request_payload ddp_routing_flow_delete_request_payload_t;
/** @endcond */

/**
* @struct ddp_routing_flow_delete_request
* @brief Structure format for the "routing flow delete request" message
*/
struct ddp_routing_flow_delete_request
{
	ddp_message_header_t header;							/*!< Message header */
	ddp_request_subheader_t subheader;						/*!< Request message subheader */

	ddp_routing_flow_delete_request_payload_t payload;		/*!< Flow ID range payload @see ddp_id_range_payload_t */

	// Heap goes here
};

/**
* @brief Structure format for the "routing flow delete response" message
*/
struct ddp_routing_flow_delete_response
{
	ddp_message_header_t header;				/*!< Message header */
	ddp_response_subheader_t subheader;			/*!< Request message subheader */
};


//! @cond Doxygen_Suppress
typedef struct ddp_routing_basic_request ddp_routing_basic_request_t;
typedef struct ddp_routing_basic_response ddp_routing_basic_response_t;
typedef struct ddp_routing_ready_state_request ddp_routing_ready_state_request_t;
typedef struct ddp_routing_ready_state_response ddp_routing_ready_state_response_t;
typedef struct ddp_routing_performance_config_request ddp_routing_performance_config_request_t;
typedef struct ddp_routing_performance_config_response ddp_routing_performance_config_response_t;
typedef struct ddp_routing_rx_chan_config_state_request ddp_routing_rx_chan_config_state_request_t;
typedef struct ddp_routing_rx_chan_config_state_response ddp_routing_rx_chan_config_state_response_t;
typedef struct ddp_routing_tx_chan_config_state_request ddp_routing_tx_chan_config_state_request_t;
typedef struct ddp_routing_tx_chan_config_state_response ddp_routing_tx_chan_config_state_response_t;
typedef struct ddp_routing_rx_chan_config_status_request ddp_routing_rx_chan_config_status_request_t;
typedef struct ddp_routing_rx_chan_status_response ddp_routing_rx_chan_status_response_t;
typedef struct ddp_routing_rx_flow_config_state_request ddp_routing_rx_flow_config_state_request_t;
typedef struct ddp_routing_rx_flow_config_state_response ddp_routing_rx_flow_config_state_response_t;
typedef struct ddp_routing_tx_flow_config_state_response ddp_routing_tx_flow_config_state_response_t;
typedef struct ddp_routing_rx_flow_status_request ddp_routing_rx_flow_status_request_t;
typedef struct ddp_routing_rx_flow_status_response ddp_routing_rx_flow_status_response_t;
typedef struct ddp_routing_rx_chan_label_set_request ddp_routing_rx_chan_label_set_request_t;
typedef struct ddp_routing_rx_chan_label_set_response ddp_routing_rx_chan_label_set_response_t;
typedef struct ddp_routing_tx_chan_label_set_request ddp_routing_tx_chan_label_set_request_t;
typedef struct ddp_routing_tx_chan_label_set_response ddp_routing_tx_chan_label_set_response_t;
typedef struct ddp_routing_rx_sub_set_request ddp_routing_rx_sub_set_request_t;
typedef struct ddp_routing_rx_sub_set_response ddp_routing_rx_sub_set_response_t;
typedef struct ddp_routing_rx_unsub_chan_request ddp_routing_rx_unsub_chan_request_t;
typedef struct ddp_routing_rx_unsub_chan_response ddp_routing_rx_unsub_chan_response_t;
typedef struct ddp_routing_multicast_tx_flow_config_request ddp_routing_multicast_tx_flow_config_request_t;
typedef struct ddp_routing_multicast_tx_flow_config_response ddp_routing_multicast_tx_flow_config_response_t;
typedef struct ddp_routing_flow_delete_request ddp_routing_flow_delete_request_t;
typedef struct ddp_routing_flow_delete_response ddp_routing_flow_delete_response_t;
//! @endcond

/**@}*/
/**@}*/
/**@}*/

#endif // _DDP_ROUTING_STRUCTURES_H

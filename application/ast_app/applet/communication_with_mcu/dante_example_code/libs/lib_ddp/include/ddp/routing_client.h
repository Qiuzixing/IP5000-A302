/*
 * File     : routing_client.h
 * Created  : September 2014
 * Author   : Chamira Perera <cperera@audinate.com>
 * Synopsis : All routing DDP client helper functions.
 *
 * Copyright 2005-2017, Audinate Pty Ltd and/or its licensors
	Confidential
 * Audinate Copyright Header Version 1
 */
#ifndef _DDP_ROUTING_CLIENT__H
#define _DDP_ROUTING_CLIENT__H

#include "ddp/packet.h"
#include "ddp/routing_structures.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
* @file routing_client.h
* @brief All routing DDP client helper functions.
*/

/** \addtogroup DDP
*  @{
*/

/**
* \defgroup Routing DDP Routing
* @{
*/

/**
* \defgroup RoutingStructs DDP Routing Structures
* @{
*/

/**
* @struct ddp_rx_channel_params
* @brief Structure used to retreive the Rx channel state related parameters
*/
struct ddp_rx_channel_params
{
	uint16_t channel_id;				/*!< ID of this Rx channel */
	char * channel_name;				/*!< Null-terminated default Rx channel name string, the caller of the function does not need to allocate memory for this field */
	uint16_t encoding;					/*!< Audio encoding used by this Rx channel @see audio_supported_encoding */
	uint32_t sample_rate;				/*!< Audio sample rate for this Rx channel */
	uint16_t pcm_map;					/*!< Bitmask of supported PCM encodings, in bytes (eg 0x4 => 24 bits) */
	uint16_t channel_flags;				/*!< Channel flags @see ddp_rx_channel_flags */
	uint16_t channel_flags_mask;		/*!< Bitwise OR'd mask which specify which channel_flags are valid */
	char * channel_label;				/*!< Null-terminated Rx channel label string, the caller of the function does not need to allocate memory for this field */
	uint16_t status;					/*!< Subscription status of this Rx channel @see ddp_routing_rx_chan_status */
	uint8_t avail;						/*!< A non-zero value indicates that this channel is available to receive audio */
	uint8_t active;						/*!< A non-zero value indicates that this channel is active */
	char * subscribed_channel;			/*!< Null-terminated string of the Tx channel that this Rx channel is subscribed to, if no subscription is active then this field is NULL, the caller of the function does not need to allocate memory for this field */
	char * subcribed_device;			/*!< Null-terminated string of the Tx device that this Rx channel is subscribed to, if no subscription is active then this field is NULL, the caller of the function does not need to allocate memory for this field */
	uint16_t flow_id;					/*!< Flow ID associted with this Rx channel, only valid if this channel is subscribed */
	uint16_t slot_id;					/*!< Slot ID associated this this Rx channel */
};

/** @cond */
typedef struct ddp_rx_channel_params ddp_rx_chan_state_params_t;
/** @endcond */

/**
* @struct ddp_tx_chan_state_params
* @brief Structure used to retreive the Tx channel state related parameters
*/
struct ddp_tx_chan_state_params
{
	uint16_t channel_id;				/*!< ID of this Tx channel */
	char * channel_name;				/*!< Null-terminated default Tx channel name string, the caller of the function does not need to allocate memory for this field */
	uint16_t encoding;					/*!< Audio encoding used by this Rx channel @see audio_supported_encoding */
	uint32_t sample_rate;				/*!< Audio sample rate in nano seconds for this Rx channel */
	uint16_t pcm_map;					/*!< Bitmask of supported PCM encodings, in bytes (eg 0x4 => 24 bits) */
	uint16_t channel_flags;				/*!< Channel flags @see ddp_rx_channel_flags */
	uint16_t channel_flags_mask;		/*!< Bitwise OR'd mask which specify which channel_flags are valid */
	char * channel_label;				/*!< Null-terminated Tx channel label string, the caller of the function does not need to allocate memory for this field */
};

/** @cond */
typedef struct ddp_tx_chan_state_params ddp_tx_chan_state_params_t;
/** @endcond */

/**
* @struct ddp_rx_flow_state_params
* @brief Structure used to retreive the Rx flow state related parameters
*/
struct ddp_rx_flow_state_params
{
	uint16_t flow_id;					/*!< Non-zero ID of this flow */
	char * flow_label;					/*!< Null-terminated flow label string, the caller of the function does not need to allocate memory for this field */
	uint16_t status;					/*!< Current status of the flow @see ddp_routing_flow_status */
	uint16_t flow_flags;				/*!< Flow flags for Rx @see ddp_routing_rx_flow_flags or Tx @see ddp_routing_tx_flow_flags */
	uint16_t flow_flags_mask;			/*!< Bitwise OR'd mask which specify which channel_flags are valid */
	uint32_t user_conf_flags;			/*!< Bitwise OR'd user flow flags @see ddp_routing_flow_user_flags */
	uint32_t user_conf_mask;			/*!< Bitwise OR'd mask to indicate which of the flow flags are valid */
	uint8_t transport;					/*!< Audio transport protocol, currently only DDP_AUDIO_TRANSPORT_ATP is supported */
	uint8_t avail_mask;					/*!< Bitmask of configured interfaces (see also addresses) */
	uint8_t active_mask;				/*!< Bitmask of active interfaces */
	uint32_t sample_rate;				/*!< Audio sample rate used for this flow */
	uint32_t latency;					/*!< Audio latency in microseconds used for this flow */
	uint16_t encoding;					/*!< Audio encoding used for this flows @see audio_supported_encoding */
	uint16_t fpp;						/*!< Frames per packet used for this flow */
};

/** @cond */
typedef struct ddp_rx_flow_state_params ddp_rx_flow_state_params_t;
/** @endcond */

/**
* @brief Structure used to retreive the Tx flow state related parameters
*/
struct ddp_tx_flow_state_params
{
	uint16_t flow_id;					/*!< Non-zero ID of this flow */
	char * flow_label;						/*!< Offset from the start of the TLV0 header to the start of the null-terminated flow label string */
	uint16_t status;					/*!< Current status of the flow @see ddp_routing_flow_status */
	uint16_t flow_flags;				/*!< Flow flags for Rx @see ddp_routing_rx_flow_flags or Tx @see ddp_routing_tx_flow_flags */
	uint16_t flow_flags_mask;			/*!< Bitwise OR'd mask which specify which channel_flags are valid */
	uint32_t user_conf_flags;			/*!< Bitwise OR'd user flow flags @see ddp_routing_flow_user_flags */
	uint32_t user_conf_mask;			/*!< Bitwise OR'd mask to indicate which of the flow flags are valid */
	uint16_t peer_flow_id;				/*!< Flow ID of the peer, e.g. if this is a Rx flow, then this is the Tx flow ID */
	char * peer_flow_label;				/*!< Offset from the start of the TLV0 header to the start of the null-terminated peer flow label string */
	uint32_t peer_device_id[2];			/*!< 8-byte Dante device ID */
	uint16_t peer_process_id;			/*!< Process ID of the peer device */
	char * peer_device_label;			/*!< Offset from the start of the TLV0 header to the start of the null-terminated peer device label string */
	uint8_t transport;					/*!< Audio transport protocol, currently only DDP_AUDIO_TRANSPORT_ATP is supported */
	uint8_t avail_mask;					/*!< Bitmask of configured interfaces (see also addresses) */
	uint8_t active_mask;				/*!< Bitmask of active interfaces */
	uint32_t sample_rate;				/*!< Audio sample rate used for this flow */
	uint32_t latency;					/*!< Audio latency in microseconds used for this flow */
	uint16_t encoding;					/*!< Audio encoding used for this flows @see audio_supported_encoding */
	uint16_t fpp;						/*!< Frames per packet used for this flow */
};

/** @cond */
typedef struct ddp_tx_flow_state_params ddp_tx_flow_state_params_t;
/** @endcond */

/**
* @struct ddp_label_params
* @brief Structure used to set the Rx or Tx channel label related parameters
*/
struct ddp_label_params
{
	uint16_t chan_id;			/*!< Non-zero ID of the channel which will be undergo a label update */
	char * label;				/*!< Pointer to a null-terminated new channel label, if a label is to be deleted then set this pointer to zero (null) */
};

/** @cond */
typedef struct ddp_label_params ddp_label_params_t;
/** @endcond */

/**
* @struct ddp_rx_chan_sub_params
* @brief Structure used to set the Rx channel subscription related parameters
*/
struct ddp_rx_chan_sub_params
{
	uint16_t rx_chan_id;				/*!< Channel ID that will receive audio */
	char * tx_chan_id;					/*!< Null-terminal channel ID that will transmit audio, use NULL to unsubscribe */
	char * tx_device;					/*!< Null-terminated name of the audio transmitter, use NULL to unsubscribe */
};

/** @cond */
typedef struct ddp_rx_chan_sub_params ddp_rx_chan_sub_params_t;
/** @endcond */

/**
* @struct ddp_multicast_tx_flow_params
* @brief Structure used to set the multicast Tx flow configuration parameters
*/
struct ddp_multicast_tx_flow_params
{
	uint16_t flow_id;					/*!< Non-zero ID of this flow */
	char * label;						/*!< Null-terminated flow label string */
	uint32_t sample_rate;				/*!< Audio sample rate used for this flow */
	uint32_t latency;					/*!< Audio latency in microseconds used for this flow */
	uint16_t encoding;					/*!< Audio encoding used for this flows @see audio_supported_encoding */
	uint16_t fpp;						/*!< Frames per packet used for this flow */
	uint16_t num_slots;					/*!< Number of audio transmit channels in this flow */
	uint16_t * slots;					/*!< Array of audio transmit channels in this flow */
};

/** @cond */
typedef struct ddp_multicast_tx_flow_params ddp_multicast_tx_flow_params_t;
/** @endcond */

/**@}*/

/**
* \defgroup RoutingFuncs DDP Routing Functions
* @{
*/

/**
* Creates a DDP routing basic request message. This message can only be used to query the current state of the device.
* @param packet_info [in] Pointer to a structure which has information about the DDP packet buffer
* @param request_id [in] A non-zero sequence number which is populated in this message
* @return AUD_SUCCESS if the DDP routing basic request message was successfully created, else a non AUD_SUCCESS value is returned
*/
aud_error_t
ddp_add_routing_basic_request
(
	ddp_packet_write_info_t * packet_info,
	ddp_request_id_t request_id
);


/**
* Reads data fields from a DDP routing basic response message
* @param packet_info [in] Pointer to a structure which has information about the DDP packet buffer
* @param offset [in] Starting point in the packet buffer to read this message
* @param out_request_id [out optional] Pointer to the sequence number of this message
* @param out_status [out optional] Pointer to the status of this message
* @param out_rsp_payload [out optional] Pointer to a structure to hold the routing basic information @see ddp_routing_basic_response_payload_t
* @return AUD_SUCCESS if the DDP routing basic response message was successfully read, else a non AUD_SUCCESS value is returned
*/
aud_error_t
ddp_read_routing_basic_response
(
	const ddp_packet_read_info_t * packet_info,
	uint16_t offset,
	ddp_request_id_t * out_request_id,
	ddp_status_t * out_status,
	ddp_routing_basic_response_payload_t *out_rsp_payload
);

/**
* Creates a DDP routing ready state request message. This message can only be used to query the current state of the device.
* @param packet_info [in] Pointer to a structure which has information about the DDP packet buffer
* @param request_id [in] A non-zero sequence number which is populated in this message
* @return AUD_SUCCESS if the DDP routing ready state request message was successfully created, else a non AUD_SUCCESS value is returned
*/
aud_error_t
ddp_add_routing_ready_state_request
(
	ddp_packet_write_info_t * packet_info,
	ddp_request_id_t request_id
);


/**
* Reads data fields from a DDP routing ready state response message
* @param packet_info [in] Pointer to a structure which has information about the DDP packet buffer
* @param offset [in] Starting point in the packet buffer to read this message
* @param out_request_id [out optional] Pointer to the sequence number of this message
* @param out_status [out optional] Pointer to the status of this message
* @param out_network_ready_state [out optional] Pointer to the network interface state of the device @see routing_ready_network_state
* @param out_routing_ready_state [out optional] Pointer to the ready state of the audio routing software of the device @see routing_ready_state
* @return AUD_SUCCESS if the DDP routing ready state response message was successfully read, else a non AUD_SUCCESS value is returned
*/
aud_error_t
ddp_read_routing_ready_state_response
(
	const ddp_packet_read_info_t * packet_info,
	uint16_t offset,
	ddp_request_id_t * out_request_id,
	ddp_status_t * out_status,
	uint8_t * out_network_ready_state,
	uint8_t * out_routing_ready_state
);

/**
* Creates a DDP routing performance configuration request message. This message can be used as a command to change the state of the device and as a request to query the current state of the device.
* @param packet_info [in] Pointer to a structure which has information about the DDP packet buffer
* @param request_id [in] A non-zero sequence number which is populated in this message
* @param unicast_control_flags [in] Bitwise OR's control flags for the unicast element @see routing_performance_config_flags, use zero to query the unicast performance values
* @param unicast_fpp [in] Frames per packet used for unicast flows
* @param unicast_latency [in] Latency in microseconds used for unicast flows
* @param multicast_control_flags [in] Bitwise OR's control flags for the multicast element @see routing_performance_config_flags, use zero to query the multicast performance values
* @param multicast_fpp [in] Frames per packet used for multicast flows 
* @param multicast_latency [in] Latency in microseconds used for multicast flows
* @return AUD_SUCCESS if the DDP routing performance configuration request message was successfully created, else a non AUD_SUCCESS value is returned
*/
aud_error_t
ddp_add_routing_performance_config_request
(
	ddp_packet_write_info_t * packet_info,
	ddp_request_id_t request_id,
	uint16_t unicast_control_flags,
	uint16_t unicast_fpp,
	uint32_t unicast_latency,
	uint16_t multicast_control_flags,
	uint16_t multicast_fpp,
	uint32_t multicast_latency
);

/**
* Reads data fields from a DDP routing performance configuration response message
* @param packet_info [in] Pointer to a structure which has information about the DDP packet buffer
* @param offset [in] Starting point in the packet buffer to read this message
* @param out_request_id [out optional] Pointer to the sequence number of this message
* @param out_status [out optional] Pointer to the status of this message
* @param out_unicast_fpp [in] Pointer to the frames per packet used by the device for unicast flows
* @param out_unicast_latency [in] Pointer to the latency in microseconds used by the device for unicast flows
* @param out_multicast_fpp [in] Pointer to the frames per packet used by the device for multicast flows
* @param out_multicast_latency [in] Pointer to the latency in microseconds used by the device for multicast flows
* @return AUD_SUCCESS if the DDP routing performance configuration response message was successfully read, else a non AUD_SUCCESS value is returned
*/
aud_error_t
ddp_read_routing_performance_config_response
(
	const ddp_packet_read_info_t * packet_info,
	uint16_t offset,
	ddp_request_id_t * out_request_id,
	ddp_status_t * out_status,
	uint16_t * out_unicast_fpp,
	uint32_t * out_unicast_latency,
	uint16_t * out_multicast_fpp,
	uint32_t * out_multicast_latency
);

/**
* Creates a DDP routing Rx channel configuration state request message. This message can only be used to query the current state of the device.
* @param packet_info [in] Pointer to a structure which has information about the DDP packet buffer
* @param request_id [in] A non-zero sequence number which is populated in this message
* @param from [in] Starting Rx channel ID, this value should be non-zero
* @param to [in] Ending Rx channel ID, this value should be non-zero and greater than the from value
* @return AUD_SUCCESS if the DDP routing Rx channel configuration state request message was successfully created, else a non AUD_SUCCESS value is returned
* @note The maximum amount of channels that can be specifed (i.e. from-to+1) is dictated by the rx_chan_state_per_ddp_msg field in the DDP routing basic response message. @see ddp_routing_basic_response_payload
*/
aud_error_t
ddp_add_routing_rx_chan_config_state_request
(
	ddp_packet_write_info_t * packet_info,
	ddp_request_id_t request_id,
	uint16_t from,
	uint16_t to
);

/**
* Reads fixed data fields from a DDP routing Rx channel configuration state response message
* @param packet_info [in] Pointer to a structure which has information about the DDP packet buffer
* @param offset [in] Starting point in the packet buffer to read this message
* @param out_message_info [in out] Pointer to a structure which contains information about the payload and heap for this DDP packet (memory should be allocated for this structure by the caller of this function)
* @param out_request_id [out optional] Pointer to the sequence number for this request message
* @param out_status [out optional] Pointer to the status of this message
* @param out_num_chans [out optional] Pointer to the number of Rx channels that have state information in this response
* @return AUD_SUCCESS if the DDP routing Rx channel configuration state response message was successfully read, else a non AUD_SUCCESS value is returned
*/
aud_error_t
ddp_read_routing_rx_chan_config_state_response_header
(
	const ddp_packet_read_info_t * packet_info,
	uint16_t offset,
	ddp_message_read_info_t * out_message_info,
	ddp_request_id_t * out_request_id,
	ddp_status_t * out_status,
	uint16_t * out_num_chans	
);

/**
* Reads data fields from a DDP routing Rx channel configuration state response message for a particular flow. N.B. This function should only be called after calling the ddp_read_routing_rx_chan_config_state_response_header function @see ddp_read_routing_rx_chan_config_state_response_header
* @param message_info [in] Pointer to a structure which has information about the DDP packet buffer, use the out_message_info parameter after calling the ddp_read_routing_rx_chan_config_state_response_header function
* @param chan_idx [in] Index into the collection of Rx channel config state data blocks in this response message. Values for this parameter range from 0 to number of channels - 1. 
* @param out_rx_chan_data [out optional] Pointer to a structure of Rx channel state (memory should be allocated for this structure by the caller of this function) indexed by chan_idx @see ddp_rx_channel_params_t
* @param out_num_custom_encodings [out optional] Pointer to the number of custom non-PCM encodings available for this channel
* @return AUD_SUCCESS if the data fields of a particular channel for the DDP routing Rx channel configuration state response message was successfully read, else a non AUD_SUCCESS value is returned
*/
aud_error_t
ddp_read_routing_rx_chan_config_state_response_chan_params
(
	ddp_message_read_info_t const * message_info,
	uint16_t chan_idx,
	ddp_rx_chan_state_params_t * out_rx_chan_data,
	uint16_t * out_num_custom_encodings
);

/**
* Reads a custom encoding field from a DDP routing Rx channel configuration state response message for a particular flow. N.B. This function should only be called after calling the ddp_read_routing_rx_chan_config_state_response_header function @see ddp_read_routing_rx_chan_config_state_response_header
* @param message_info [in] Pointer to a structure which has information about the DDP packet buffer, use the out_message_info parameter after calling the ddp_read_routing_rx_chan_config_state_response_header function
* @param chan_idx [in] Index into the collection of Rx channel config state data blocks in this response message. Values for this parameter range from 0 to number of channels - 1.
* @param custom_enc_idx [in] Index in the collection of custom non-PCM encoding data blocks in this response message. Values for this parameter range from 0 to number of custom encodings - 1.
* @param out_custom_encoding [out optional] Custom non-PCM encoding index at custom_enc_idx
* @return AUD_SUCCESS if the data fields of a particular channel for the DDP routing Rx channel configuration state response message was successfully read, else a non AUD_SUCCESS value is returned
*/
aud_error_t
ddp_read_routing_rx_chan_config_state_response_custom_encoding
(
	ddp_message_read_info_t const * message_info,
	uint16_t chan_idx,
	uint16_t custom_enc_idx,
	uint16_t * out_custom_encoding
);

/**
* Creates a DDP routing Tx channel configuration state request message. This message can only be used to query the current state of the device.
* @param packet_info [in] Pointer to a structure which has information about the DDP packet buffer
* @param request_id [in] A non-zero sequence number which is populated in this message
* @param from [in] Starting Tx channel ID, this value should be non-zero
* @param to [in] Ending Tx channel ID, this value should be non-zero and greater than the from value
* @return AUD_SUCCESS if the DDP routing Tx channel configuration state request message was successfully created, else a non AUD_SUCCESS value is returned
* @note The maximum amount of channels that can be specifed (i.e. from-to+1) is dictated by the tx_chan_state_per_ddp_msg field in the DDP routing basic response message. @see ddp_routing_basic_response_payload
*/
aud_error_t
ddp_add_routing_tx_chan_config_state_request
(
	ddp_packet_write_info_t * packet_info,
	ddp_request_id_t request_id,
	uint16_t from,
	uint16_t to
);

/**
* Reads header data fields from a DDP routing Tx channel configuration state response message
* @param packet_info [in] Pointer to a structure which has information about the DDP packet buffer
* @param offset [in] Starting point in the packet buffer to read this message
* @param out_message_info [in out] Pointer to a structure which contains information about the payload and heap for this DDP packet (memory should be allocated for this structure by the caller of this function)
* @param out_request_id [out optional] Pointer to the sequence number for this request message
* @param out_status [out optional] Pointer to the status of this message
* @param out_num_chans [out optional] Pointer to the number of Tx channels that have state information in this response
* @return AUD_SUCCESS if the DDP routing Tx channel configuration state response message was successfully read, else a non AUD_SUCCESS value is returned
*/
aud_error_t
ddp_read_routing_tx_chan_config_state_response_header
(
	const ddp_packet_read_info_t * packet_info,
	uint16_t offset,
	ddp_message_read_info_t * out_message_info,
	ddp_request_id_t * out_request_id,
	ddp_status_t * out_status,
	uint16_t * out_num_chans
);

/**
* Reads data fields from a DDP routing Tx channel configuration state response message for a particular flow. N.B. This function should only be called after calling the ddp_read_routing_tx_chan_config_state_response_header function @see ddp_read_routing_tx_chan_config_state_response_header
* @param message_info [in] Pointer to a structure which has information about the DDP packet buffer, use the out_message_info parameter after calling the ddp_read_routing_tx_chan_config_state_response_header function
* @param chan_idx [in] Index into the collection of Tx channel config state data blocks in this response message. Values for this parameter range from 0 to number of channels - 1. 
* @param out_tx_chan_data [out optional] Pointer to a structure of Tx channel state (memory should be allocated for this structure by the caller of this function) indexed by chan_idx @see ddp_tx_channel_params_t
* @param out_num_custom_encodings [out optional] Pointer to the number of custom non-PCM encodings available for this channel
* @return AUD_SUCCESS if the data fields of a particular channel for the DDP routing Tx channel configuration state response message was successfully read, else a non AUD_SUCCESS value is returned
*/
aud_error_t
ddp_read_routing_tx_chan_config_state_response_chan_params
(
	const ddp_message_read_info_t * message_info,
	uint16_t chan_idx,
	ddp_tx_chan_state_params_t * out_tx_chan_data,
	uint16_t * out_num_custom_encodings
);

/**
* Reads a custom encoding field from a DDP routing Tx channel configuration state response message for a particular flow. N.B. This function should only be called after calling the ddp_read_routing_tx_chan_config_state_response_header function @see ddp_read_routing_rx_chan_config_state_response_header
* @param message_info [in] Pointer to a structure which has information about the DDP packet buffer, use the out_message_info parameter after calling the ddp_read_routing_tx_chan_config_state_response_header function
* @param chan_idx [in] Index into the collection of Rx channel config state data blocks in this response message. Values for this parameter range from 0 to number of channels - 1.
* @param custom_enc_idx [in] Index in the collection of custom non-PCM encoding data blocks in this response message. Values for this parameter range from 0 to number of custom encodings - 1.
* @param out_custom_encoding [out optional] Custom non-PCM encoding index at custom_enc_idx
* @return AUD_SUCCESS if the data fields of a particular channel for the DDP routing Tx channel configuration state response message was successfully read, else a non AUD_SUCCESS value is returned
*/
aud_error_t
ddp_read_routing_tx_chan_config_state_response_custom_encoding
(
	ddp_message_read_info_t const * message_info,
	uint16_t chan_idx,
	uint16_t custom_enc_idx,
	uint16_t * out_custom_encoding
);

/**
* Creates a DDP routing Rx channel configuration status request message. This message can only be used to query the current state of the device.
* @param packet_info [in] Pointer to a structure which has information about the DDP packet buffer
* @param request_id [in] A non-zero sequence number which is populated in this message
* @param from [in] Starting Rx channel ID, this value should be non-zero
* @param to [in] Ending Rx channel ID, this value should be non-zero and greater than the from value
* @return AUD_SUCCESS if the DDP routing Rx channel configuration status request message was successfully created, else a non AUD_SUCCESS value is returned
* @note The maximum amount of flows that can be specifed (i.e. from-to+1) is dictated by the rx_flow_status_per_ddp_msg field in the DDP routing basic response message. @see ddp_routing_basic_response_payload
*/
aud_error_t
ddp_add_routing_rx_chan_status_request
(
	ddp_packet_write_info_t * packet_info,
	ddp_request_id_t request_id,
	uint16_t from,
	uint16_t to
);

/**
* Reads fixed data fields from a DDP routing Rx channel status response message
* @param packet_info [in] Pointer to a structure which has information about the DDP packet buffer
* @param offset [in] Starting point in the packet buffer to read this message
* @param out_message_info [in out] Pointer to a structure which contains information about the payload and heap for this DDP packet (memory should be allocated for this structure by the caller of this function)
* @param out_request_id [out optional] Pointer to the sequence number for this request message
* @param out_status [out optional] Pointer to the status of this message
* @param out_num_chans [out optional] Pointer to the number of Rx channels that have status information in this response
* @return AUD_SUCCESS if the DDP routing Rx channel status response message was successfully read, else a non AUD_SUCCESS value is returned
*/
aud_error_t
ddp_read_routing_rx_chan_status_response_header
(
	const ddp_packet_read_info_t * packet_info,
	uint16_t offset,
	ddp_message_read_info_t * out_message_info,
	ddp_request_id_t * out_request_id,
	ddp_status_t * out_status,
	uint16_t * out_num_chans
);

/**
* Reads data fields from a DDP routing Rx channel configuration state response message for a particular flow. N.B. This function should only be called after calling the ddp_read_routing_rx_chan_status_response_header function
* @param message_info [in] Pointer to a structure which has information about the DDP packet buffer, use the out_message_info parameter after calling the ddp_read_routing_rx_chan_status_response_header function
* @param chan_idx [in] Index into the channel status blocks of information, values range from 0 to number of channels - 1
* @param out_channel_id [out optional] Pointer to the ID of the Rx channel
* @param out_status [out optional] Pointer ot the status of the Rx channel @see ddp_routing_rx_chan_status indexed by chan_idx
* @param out_avail [out optional] Pointer to the availabilty of this channel
* @param out_active [out optional] Pointer to the active status of this channel
* @return AUD_SUCCESS if the data fields of a particular channel for the DDP routing Rx channel configuration state response message was successfully read, else a non AUD_SUCCESS value is returned
*/
aud_error_t
ddp_read_routing_rx_chan_status_response_chan_params
(
	const ddp_message_read_info_t * message_info,
	uint16_t chan_idx,
	uint16_t * out_channel_id,
	uint16_t * out_status,
	uint8_t * out_avail,
	uint8_t * out_active
);

/**
* Creates a DDP routing Rx flow configuration state request message. This message can only be used to query the current state of the device.
* @param packet_info [in] Pointer to a structure which has information about the DDP packet buffer
* @param request_id [in] A non-zero sequence number which is populated in this message
* @param from [in] Starting Rx flow ID, this value should be non-zero
* @param to [in] Ending Rx flow ID, this value should be non-zero and greater than the from value
* @return AUD_SUCCESS if the DDP routing Rx flow configuration state request message was successfully created, else a non AUD_SUCCESS value is returned
* @note The maximum amount of flows that can be specifed (i.e. from-to+1) is dictated by the rx_flow_state_per_ddp_msg field in the DDP routing basic response message. @see ddp_routing_basic_response_payload
*/
aud_error_t
ddp_add_routing_rx_flow_config_state_request
(
	ddp_packet_write_info_t * packet_info,
	ddp_request_id_t request_id,
	uint16_t from,
	uint16_t to
);

/**
* Reads fixed data fields from a DDP routing Rx flow configuration state response message
* @param packet_info [in] Pointer to a structure which has information about the DDP packet buffer
* @param offset [in] Starting point in the packet buffer to read this message
* @param out_message_info [in out] Pointer to a structure which contains information about the payload and heap for this DDP packet (memory should be allocated for this structure by the caller of this function)
* @param out_request_id [out optional] Pointer to the sequence number for this request message
* @param out_status [out optional] Pointer to the status of this message
* @param out_num_flows [out optional] Pointer to the number of Rx flows that have state information in this response
* @return AUD_SUCCESS if the DDP routing Rx flow configuration state response message was successfully read, else a non AUD_SUCCESS value is returned
*/
aud_error_t
ddp_read_routing_rx_flow_config_state_response_header
(
	const ddp_packet_read_info_t * packet_info,
	uint16_t offset,
	ddp_message_read_info_t * out_message_info,
	ddp_request_id_t * out_request_id,
	ddp_status_t * out_status,
	uint16_t * out_num_flows
);

/**
* Reads data fields from a DDP routing Rx flow configuration state response message for a particular flow. N.B. This function should only be called after calling the ddp_read_routing_rx_flow_config_state_response_header function
* @param message_info [in] Pointer to a structure which has information about the DDP packet buffer, use the out_message_info parameter after calling the ddp_read_routing_rx_flow_config_state_response_header
* @param flow_idx [in] Index into the collection of flow config state data blocks in this response message. Values for this parameter range from 0 to number of flows - 1. 
* @param out_num_slots [out optional] Pointer to the number of Rx slots associated with this flow
* @param out_num_addresses [out optional] Pointer to number of IP address and port pairs associated with this flow
* @param out_rx_flow [out] Pointer to a Rx flow state information structure (memory should be allocated by the caller of this function) @see ddp_rx_flow_state_params_t for the flow indexed by flow_idx
* @return AUD_SUCCESS if the DDP routing Rx flow configuration state response message was successfully read, else a non AUD_SUCCESS value is returned
*/
aud_error_t
ddp_read_routing_rx_flow_config_state_response_flow_params
(
	const ddp_message_read_info_t * message_info,
	uint16_t flow_idx,
	uint16_t * out_num_slots,
	uint16_t * out_num_addresses,
	ddp_rx_flow_state_params_t * out_rx_flow	
);

/**
* Reads a slot related data fields from a DDP routing Rx flow configuration state response message for a particular flow. N.B. This function should only be called after calling the ddp_read_routing_rx_flow_config_state_response_header function @see ddp_read_routing_rx_flow_config_state_response_header
* @param message_info [in] Pointer to a structure which has information about the DDP packet buffer, use the out_message_info parameter after calling the ddp_read_routing_rx_flow_config_state_response_header function
* @param flow_idx [in] Index into the collection of flow config state data blocks in this response message. Values for this parameter range from 0 to number of flows - 1.
* @param slot_idx [in] Index into the collection of slot data blocks for the flow indexed by flow_idx. Values for this parameter range from 0 to number of slots - 1.
* @param out_num_chans [out optional] Pointer to the number of channels contained within the slot index by slot_idx
* @return AUD_SUCCESS if the DDP routing Rx flow configuration state response message was successfully read, else a non AUD_SUCCESS value is returned
*/
aud_error_t
ddp_read_routing_rx_flow_config_state_response_flow_slot
(
	const ddp_message_read_info_t * message_info,
	uint16_t flow_idx,
	uint16_t slot_idx,
	uint16_t * out_num_chans
);

/**
* Reads the channels data field in a slot from a DDP routing Rx flow configuration state response message for a particular flow. N.B. This function should only be called after calling the ddp_read_routing_rx_flow_config_state_response_header function @see ddp_read_routing_rx_flow_config_state_response_header
* @param message_info [in] Pointer to a structure which has information about the DDP packet buffer, use the out_message_info parameter after calling the ddp_read_routing_rx_flow_config_state_response_header function
* @param flow_idx [in] Index into the collection of flow config state data blocks in this response message. Values for this parameter range from 0 to number of flows - 1.
* @param slot_idx [in] Index into the collection of slot data blocks for the flow indexed by flow_idx. Values for this parameter range from 0 to number of slots - 1.
* @param out_chans [in out optional] Pointer to the array of channels (memory should be allocated by the caller of this function) contained within the slot index by slot_idx 
* @return AUD_SUCCESS if the DDP routing Rx flow configuration state response message was successfully read, else a non AUD_SUCCESS value is returned
*/
aud_error_t
ddp_read_routing_rx_flow_config_state_response_flow_slot_chans
(
	const ddp_message_read_info_t * message_info,
	uint16_t flow_idx,
	uint16_t slot_idx,
	uint16_t * out_chans
);

/**
* Reads an address data field from a DDP routing Rx flow configuration state response message for a particular flow. N.B. This function should only be called after calling the ddp_read_routing_rx_flow_config_state_response_header function @see ddp_read_routing_rx_flow_config_state_response_header
* @param message_info [in] Pointer to a structure which has information about the DDP packet buffer, use the out_message_info parameter after calling the ddp_read_routing_rx_flow_config_state_response_header function
* @param flow_idx [in] Index into the collection of flow config state data blocks in this response message. Values for this parameter range from 0 to number of flows - 1.
* @param address_idx [in] Index into the collection of IP address and port data blocks for the flow indexed by flow_idx. Values for this parameter range from 0 to number of slots - 1.
* @param out_ip_address [out optional] Pointer to the IP address indexed by address_idx in a flow indexed by flow_idx
* @param out_port [out optional] Pointer to the port indexed by address_idx in a flow indexed by flow_idx
* @return AUD_SUCCESS if the DDP routing Rx flow configuration state response message was successfully read, else a non AUD_SUCCESS value is returned
* @deprecated Use \ref ddp_read_routing_rx_flow_config_state_response_flow_address_nw_ip instead
*/
aud_error_t
ddp_read_routing_rx_flow_config_state_response_flow_address_legacy
(
	const ddp_message_read_info_t * message_info,
	uint16_t flow_idx,
	uint16_t address_idx,
	uint32_t * out_ip_address,
	uint16_t * out_port
);

/**
* Reads an address data field from a DDP routing Rx flow configuration state response message for a particular flow. N.B. This function should only be called after calling the ddp_read_routing_rx_flow_config_state_response_header function @see ddp_read_routing_rx_flow_config_state_response_header
* @param message_info [in] Pointer to a structure which has information about the DDP packet buffer, use the out_message_info parameter after calling the ddp_read_routing_rx_flow_config_state_response_header function
* @param flow_idx [in] Index into the collection of flow config state data blocks in this response message. Values for this parameter range from 0 to number of flows - 1.
* @param address_idx [in] Index into the collection of IP address and port data blocks for the flow indexed by flow_idx. Values for this parameter range from 0 to number of slots - 1.
* @param out_ip_address [out optional] Pointer to the IP address in network byte order indexed by address_idx in a flow indexed by flow_idx
* @param out_port [out optional] Pointer to the port indexed by address_idx in a flow indexed by flow_idx
* @return AUD_SUCCESS if the DDP routing Rx flow configuration state response message was successfully read, else a non AUD_SUCCESS value is returned
*/
aud_error_t
ddp_read_routing_rx_flow_config_state_response_flow_address_nw_ip
(
	const ddp_message_read_info_t * message_info,
	uint16_t flow_idx,
	uint16_t address_idx,
	ddp_ip_addr_nw_t * out_ip_address,
	uint16_t * out_port
	);

/**
* Creates a DDP routing Tx flow configuration state request message. This message can only be used to query the current state of the device.
* @param packet_info [in] Pointer to a structure which has information about the DDP packet buffer
* @param request_id [in] A non-zero sequence number which is populated in this message
* @param from [in] Starting Tx flow ID, this value should be non-zero
* @param to [in] Ending Tx flow ID, this value should be non-zero and greater than the from value
* @return AUD_SUCCESS if the DDP routing Tx flow configuration state request message was successfully created, else a non AUD_SUCCESS value is returned
* @note The maximum amount of flows that can be specifed (i.e. from-to+1) is dictated by the tx_flow_state_per_ddp_msg field in the DDP routing basic response message. @see ddp_routing_basic_response_payload
*/
aud_error_t
ddp_add_routing_tx_flow_config_state_request
(
	ddp_packet_write_info_t * packet_info,
	ddp_request_id_t request_id,
	uint16_t from,
	uint16_t to
);

/**
* Reads fixed data fields from a DDP routing Tx flow configuration state response message
* @param packet_info [in] Pointer to a structure which has information about the DDP packet buffer
* @param offset [in] Starting point in the packet buffer to read this message
* @param out_message_info [in out] Pointer to a structure which contains information about the payload and heap for this DDP packet (memory should be allocated for this structure by the caller of this function)
* @param out_request_id [out optional] Pointer to the sequence number for this request message
* @param out_status [out optional] Pointer to the status of this message
* @param out_num_flows [out optional] Pointer to the number of Tx flows that have state information in this response
* @return AUD_SUCCESS if the DDP routing Tx flow configuration state response message was successfully read, else a non AUD_SUCCESS value is returned
*/
aud_error_t
ddp_read_routing_tx_flow_config_state_response_header
(
	const ddp_packet_read_info_t * packet_info,
	uint16_t offset,
	ddp_message_read_info_t * out_message_info,
	ddp_request_id_t * out_request_id,
	ddp_status_t * out_status,
	uint16_t * out_num_flows
);

/**
* Reads data fields from a DDP routing Tx flow configuration state response message for a particular flow. N.B. This function should only be called after calling the ddp_read_routing_tx_flow_config_state_response_header function @see ddp_read_routing_tx_flow_config_state_response_header
* @param message_info [in] Pointer to a structure which has information about the DDP packet buffer, use the out_message_info parameter after calling the ddp_read_routing_tx_flow_config_state_response_header function
* @param flow_idx [in] Index into the collection of flow config state data blocks in this response message. Values for this parameter range from 0 to number of flows - 1.
* @param out_num_slots [out optional] Pointer to the number of Tx slots associated with this flow
* @param out_num_addresses [out optional] Pointer to number of IP address and port pairs associated with this flow
* @param out_tx_flow [out] Pointer to a Tx flow state information structure (memory should be allocated by the caller of this function) @see ddp_tx_flow_state_params_t for the flow indexed by flow_idx
* @return AUD_SUCCESS if the DDP routing Tx flow configuration state response message was successfully read, else a non AUD_SUCCESS value is returned
*/
aud_error_t
ddp_read_routing_tx_flow_config_state_response_flow_params
(
	const ddp_message_read_info_t * message_info,
	uint16_t flow_idx,
	uint16_t * out_num_slots,
	uint16_t * out_num_addresses,
	ddp_tx_flow_state_params_t * out_tx_flow
);

/**
* Reads the slots data fields from a DDP routing Tx flow configuration state response message for a particular flow. N.B. This function should only be called after calling the ddp_read_routing_tx_flow_config_state_response_header function @see ddp_read_routing_tx_flow_config_state_response_header
* @param message_info [in] Pointer to a structure which has information about the DDP packet buffer, use the out_message_info parameter after calling the ddp_read_routing_tx_flow_config_state_response_header function
* @param flow_idx [in] Index into the collection of flow config state data blocks in this response message. Values for this parameter range from 0 to number of flows - 1.
* @param out_slots [out optional] Pointer to an array of slots for the flow indexed by flow_idx
* @return AUD_SUCCESS if the DDP routing Tx flow configuration state response message was successfully read, else a non AUD_SUCCESS value is returned
*/
aud_error_t
ddp_read_routing_tx_flow_config_state_response_flow_slots
(
	const ddp_message_read_info_t * message_info,
	uint16_t flow_idx,
	uint16_t * out_slots
);

/**
* Reads an address data field from a DDP routing Tx flow configuration state response message for a particular flow. N.B. This function should only be called after calling the ddp_read_routing_tx_flow_config_state_response_header function @see ddp_read_routing_tx_flow_config_state_response_header
* @param message_info [in] Pointer to a structure which has information about the DDP packet buffer, use the out_message_info parameter after calling the ddp_read_routing_tx_flow_config_state_response_header function
* @param flow_idx [in] Index into the collection of flow config state data blocks in this response message. Values for this parameter range from 0 to number of flows - 1.
* @param address_idx [in] Index into the collection of IP address and port data blocks for the flow indexed by flow_idx. Values for this parameter range from 0 to number of slots - 1.
* @param out_ip_address [out optional] Pointer to the IP address indexed by address_idx in a flow indexed by flow_idx
* @param out_port [out optional] Pointer to the port indexed by address_idx in a flow indexed by flow_idx
* @return AUD_SUCCESS if the DDP routing Tx flow configuration state response message was successfully read, else a non AUD_SUCCESS value is returned
* @deprecated Use \ref ddp_read_routing_tx_flow_config_state_response_flow_address_nw_ip instead
*/
aud_error_t
ddp_read_routing_tx_flow_config_state_response_flow_address_legacy
(
	const ddp_message_read_info_t * message_info,
	uint16_t flow_idx,
	uint16_t address_idx,
	uint32_t * out_ip_address,
	uint16_t * out_port
);

/**
* Reads an address data field from a DDP routing Tx flow configuration state response message for a particular flow. N.B. This function should only be called after calling the ddp_read_routing_tx_flow_config_state_response_header function @see ddp_read_routing_tx_flow_config_state_response_header
* @param message_info [in] Pointer to a structure which has information about the DDP packet buffer, use the out_message_info parameter after calling the ddp_read_routing_tx_flow_config_state_response_header function
* @param flow_idx [in] Index into the collection of flow config state data blocks in this response message. Values for this parameter range from 0 to number of flows - 1.
* @param address_idx [in] Index into the collection of IP address and port data blocks for the flow indexed by flow_idx. Values for this parameter range from 0 to number of slots - 1.
* @param out_ip_address [out optional] Pointer to the IP address in network byte order indexed by address_idx in a flow indexed by flow_idx
* @param out_port [out optional] Pointer to the port indexed by address_idx in a flow indexed by flow_idx
* @return AUD_SUCCESS if the DDP routing Tx flow configuration state response message was successfully read, else a non AUD_SUCCESS value is returned
*/
aud_error_t
ddp_read_routing_tx_flow_config_state_response_flow_address_nw_ip
(
	const ddp_message_read_info_t * message_info,
	uint16_t flow_idx,
	uint16_t address_idx,
	ddp_ip_addr_nw_t * out_ip_address,
	uint16_t * out_port
);

/**
* Creates a DDP routing Rx flow status request message. This message can only be used to query the current state of the device.
* @param packet_info [in] Pointer to a structure which has information about the DDP packet buffer
* @param request_id [in] A non-zero sequence number which is populated in this message
* @param from [in] Starting Rx flow ID, this value should be non-zero
* @param to [in] Ending Rx flow ID, this value should be non-zero and greater than the from value
* @return AUD_SUCCESS if the DDP routing Rx flow status request message was successfully created, else a non AUD_SUCCESS value is returned
* @note The maximum amount of flows that can be specifed (i.e. from-to+1) is dictated by the rx_flow_status_per_ddp_msg field in the DDP routing basic response message. @see ddp_routing_basic_response_payload
*/
aud_error_t
ddp_add_routing_rx_flow_status_request
(
	ddp_packet_write_info_t * packet_info,
	ddp_request_id_t request_id,
	uint16_t from,
	uint16_t to
);

/**
* Reads header data fields from a DDP routing Rx flow status response message. 
* @param packet_info [in] Pointer to a structure which has information about the DDP packet buffer
* @param offset [in] Starting point in the packet buffer to read this message
* @param out_message_info [in out] Pointer to a structure which contains information about the payload and heap for this DDP packet (memory should be allocated for this structure by the caller of this function)
* @param out_request_id [out optional] Pointer to the sequence number for this request message
* @param out_status [out optional] Pointer to the status of this message
* @param out_num_flows [out optional] Pointer to the number of Rx flows that have status information in this response
* @return AUD_SUCCESS if the DDP routing Rx flow status response message was successfully read, else a non AUD_SUCCESS value is returned
*/
aud_error_t
ddp_read_routing_rx_flow_status_response_header
(
	const ddp_packet_read_info_t * packet_info,
	uint16_t offset,
	ddp_message_read_info_t * out_message_info,
	ddp_request_id_t * out_request_id,
	ddp_status_t * out_status,
	uint16_t * out_num_flows
);

/**
* Reads data fields from a DDP routing Rx flow status response message for a particular flow. N.B. This function should only be called after calling the ddp_read_routing_rx_flow_status_response_header function @see ddp_read_routing_rx_flow_status_response_header
* @param message_info [in] Pointer to a structure which has information about the DDP packet buffer, use the out_message_info parameter after calling the ddp_read_routing_rx_flow_status_response_header function
* @param flow_idx [in] Index into the channel status blocks of information, values range from 0 to number of flows - 1
* @param out_flow_id [out optional] Pointer to the ID of the Rx flow
* @param out_status [out optional] Pointer ot the status of the Rx channel @see ddp_routing_rx_flow_status indexed by flow_idx
* @param out_avail [out optional] Pointer to the
* @param out_active [out optional] Pointer to the
* @return AUD_SUCCESS if the data fields of a particular flow for the DDP routing Rx flow status response message was successfully read, else a non AUD_SUCCESS value is returned
*/
aud_error_t
ddp_read_routing_rx_flow_status_response_flow_params
(
	const ddp_message_read_info_t * message_info,
	uint16_t flow_idx,
	uint16_t * out_flow_id,
	uint16_t * out_status,
	uint8_t * out_avail,
	uint8_t * out_active
);

/**
* Creates a DDP routing Rx channel label request message. This message can only be used to change the state of the device.
* @param packet_info [in] Pointer to a structure which has information about the DDP packet buffer
* @param request_id [in] A non-zero sequence number which is populated in this message
* @param num_chans [in] Number of channels which will undergo a label update
* @param labels [in] Pointer to an array of structures @see ddp_label_params_t which has the channel ID and new channel label
* @return AUD_SUCCESS if the DDP routing Rx channel label request message was successfully created, else a non AUD_SUCCESS value is returned
* @note The maximum amount channels for which their labels can be set is dictated by the rx_chan_state_per_ddp_msg field in the DDP routing basic message. @see ddp_routing_basic_response_payload
*/
aud_error_t
ddp_add_routing_rx_chan_label_set_request
(
	ddp_packet_write_info_t * packet_info,
	ddp_request_id_t request_id,
	uint16_t num_chans,
	ddp_label_params_t * labels
);

/**
* Reads header data fields from a DDP routing Rx channel label set response message
* @param packet_info [in] Pointer to a structure which has information about the DDP packet buffer
* @param offset [in] Starting point in the packet buffer to read this message
* @param out_message_info [in out] Pointer to a structure which contains information about the payload and heap for this DDP packet (memory should be allocated for this structure by the caller of this function)
* @param out_request_id [out optional] Pointer to the sequence number for this request message
* @param out_status [out optional] Pointer to the status of this message
* @param out_num_chans [out optional] Pointer to the number of Rx channels that have state information in this response
* @return AUD_SUCCESS if the DDP routing Rx channel label set response message was successfully read, else a non AUD_SUCCESS value is returned
*/
aud_error_t
ddp_read_routing_rx_chan_label_set_response_header
(
	const ddp_packet_read_info_t * packet_info,
	uint16_t offset,
	ddp_message_read_info_t * out_message_info,
	ddp_request_id_t * out_request_id,
	ddp_status_t * out_status,
	uint16_t * out_num_chans
);

/**
* Reads data fields from a DDP routing Rx channel label set response message for a particular channel
* @param message_info [in] Pointer to a structure which has information about the DDP packet buffer
* @param index [in] Index into the blocks of channel config state structures @see ddp_rx_chan_state_params_t array, values range from 0 to number of channels - 1
* @param out_rx_chan_data [out optional] Pointer to a structure of Rx channel state (memory should be allocated for this structure by the caller of this function) @see ddp_rx_channel_params_t
* @param out_num_custom_encodings [out optional] Pointer to the number of custom non-PCM encodings available for this channel
* @return AUD_SUCCESS if the data fields of a particular channel for the DDP routing Rx channel label set response message was successfully read, else a non AUD_SUCCESS value is returned
*/
aud_error_t
ddp_read_routing_rx_chan_label_set_response_chan_params
(
	const ddp_message_read_info_t * message_info,
	uint16_t index,
	ddp_rx_chan_state_params_t * out_rx_chan_data,
	uint16_t * out_num_custom_encodings
);

/**
* Reads a custom encoding field from a DDP routing Rx channel label set response message for a particular flow. N.B. This function should only be called after calling the ddp_read_routing_rx_chan_label_set_response_header function @see ddp_read_routing_rx_chan_config_state_response_header
* @param message_info [in] Pointer to a structure which has information about the DDP packet buffer, use the out_message_info parameter after calling the ddp_read_routing_rx_chan_label_set_response_header function
* @param chan_idx [in] Index into the collection of Rx channel config state data blocks in this response message. Values for this parameter range from 0 to number of channels - 1.
* @param custom_enc_idx [in] Index in the collection of custom non-PCM encoding data blocks in this response message. Values for this parameter range from 0 to number of custom encodings - 1.
* @param out_custom_encoding [out optional] Custom non-PCM encoding index at custom_enc_idx
* @return AUD_SUCCESS if the data fields of a particular channel for the DDP routing Rx channel label set response message was successfully read, else a non AUD_SUCCESS value is returned
*/
aud_error_t
ddp_read_routing_rx_chan_label_set_response_custom_encoding
(
	ddp_message_read_info_t const * message_info,
	uint16_t chan_idx,
	uint16_t custom_enc_idx,
	uint16_t * out_custom_encoding
);

/**
* Creates a DDP routing Tx channel label request message. This message can only be used to change the state of the device.
* @param packet_info [in] Pointer to a structure which has information about the DDP packet buffer
* @param request_id [in] A non-zero sequence number which is populated in this message
* @param num_chans [in] Number of channels which will undergo a label update
* @param labels [in] Pointer to an array of structures @see ddp_label_params_t which has the channel ID and new channel label
* @return AUD_SUCCESS if the DDP routing Tx channel label request message was successfully created, else a non AUD_SUCCESS value is returned
* @note The maximum amount channels for which their labels can be set is dictated by the tx_chan_state_per_ddp_msg field in the DDP routing basic message. @see ddp_routing_basic_response_payload
*/
aud_error_t
ddp_add_routing_tx_chan_label_set_request
(
	ddp_packet_write_info_t * packet_info,
	ddp_request_id_t request_id,
	uint16_t num_chans,
	ddp_label_params_t * labels
);

/**
* Reads header data fields from a DDP routing Tx channel label set response message
* @param packet_info [in] Pointer to a structure which has information about the DDP packet buffer
* @param offset [in] Starting point in the packet buffer to read this message
* @param out_message_info [in out] Pointer to a structure which contains information about the payload and heap for this DDP packet (memory should be allocated for this structure by the caller of this function)
* @param out_request_id [out optional] Pointer to the sequence number for this request message
* @param out_status [out optional] Pointer to the status of this message
* @param out_num_chans [out optional] Pointer to the number of Tx channels that have state information in this response
* @return AUD_SUCCESS if the DDP routing Tx channel label set response message was successfully read, else a non AUD_SUCCESS value is returned
*/
aud_error_t
ddp_read_routing_tx_chan_label_set_response_header
(
	const ddp_packet_read_info_t * packet_info,
	uint16_t offset,
	ddp_message_read_info_t * out_message_info,
	ddp_request_id_t * out_request_id,
	ddp_status_t * out_status,
	uint16_t * out_num_chans
);

/**
* Reads data fields from a DDP routing Tx channel label set response message for a particular channel
* @param message_info [in] Pointer to a structure which has information about the DDP packet buffer
* @param index [in] Index into the blocks of channel config state structures @see ddp_tx_chan_state_params_t array, values range from 0 to number of channels - 1
* @param out_tx_chan_data [out optional] Pointer to a structure of Tx channel state (memory should be allocated for this structure by the caller of this function) @see ddp_tx_channel_params_t
* @param out_num_custom_encodings [out optional] Pointer to the number of custom non-PCM encodings available for this channel
* @return AUD_SUCCESS if the data fields of a particular channel for the DDP routing Tx label set response message was successfully read, else a non AUD_SUCCESS value is returned
*/
aud_error_t
ddp_read_routing_tx_chan_label_set_response_chan_params
(
	const ddp_message_read_info_t * message_info,		
	uint16_t index,
	ddp_tx_chan_state_params_t * out_tx_chan_data,
	uint16_t * out_num_custom_encodings
);

/**
* Reads a custom encoding field from a DDP routing Tx channel label set response message for a particular flow. N.B. This function should only be called after calling the ddp_read_routing_tx_chan_label_set_response_header function @see ddp_read_routing_rx_chan_config_state_response_header
* @param message_info [in] Pointer to a structure which has information about the DDP packet buffer, use the out_message_info parameter after calling the ddp_read_routing_tx_chan_label_set_response_header function
* @param chan_idx [in] Index into the collection of Rx channel config state data blocks in this response message. Values for this parameter range from 0 to number of channels - 1.
* @param custom_enc_idx [in] Index in the collection of custom non-PCM encoding data blocks in this response message. Values for this parameter range from 0 to number of custom encodings - 1.
* @param out_custom_encoding [out optional] Custom non-PCM encoding index at custom_enc_idx
* @return AUD_SUCCESS if the data fields of a particular channel for the DDP routing Tx channel label set response message was successfully read, else a non AUD_SUCCESS value is returned
*/
aud_error_t
ddp_read_routing_tx_chan_label_set_response_custom_encoding
(
	ddp_message_read_info_t const * message_info,
	uint16_t chan_idx,
	uint16_t custom_enc_idx,
	uint16_t * out_custom_encoding
);

/**
* Creates a DDP routing Rx subscribe set request message. This message can only be used to change the state of the device.
* @param packet_info [in] Pointer to a structure which has information about the DDP packet buffer
* @param request_id [in] A non-zero sequence number which is populated in this message
* @param num_chans [in] Number of channels which will be subscribed
* @param chan_subs [in] Pointer to an array of structures @see ddp_rx_chan_sub_params_t which has parameters related to how to setup the subscription
* @return AUD_SUCCESS if the DDP routing Rx subscribe channel request message was successfully created, else a non AUD_SUCCESS value is returned
* @note The maximum amount of channels that can be subscribed at once is dictated by the rx_chan_state_per_ddp_msg field in the DDP routing basic message. @see ddp_routing_basic_response_payload
*/
aud_error_t
ddp_add_routing_rx_sub_set_request
(
	ddp_packet_write_info_t * packet_info,
	ddp_request_id_t request_id,
	uint16_t num_chans,
	ddp_rx_chan_sub_params_t * chan_subs
);

/**
* Reads header data fields from a DDP routing Rx subscribe set response message
* @param packet_info [in] Pointer to a structure which has information about the DDP packet buffer
* @param offset [in] Starting point in the packet buffer to read this message
* @param out_message_info [in out] Pointer to a structure which contains information about the payload and heap for this DDP packet (memory should be allocated for this structure by the caller of this function)
* @param out_request_id [out optional] Pointer to the sequence number for this request message
* @param out_status [out optional] Pointer to the status of this message
* @param out_num_chans [out optional] Pointer to the number of Rx channels that have state information in this response
* @return AUD_SUCCESS if the DDP routing Rx subscribe response message was successfully read, else a non AUD_SUCCESS value is returned
*/
aud_error_t
ddp_read_routing_rx_sub_set_response_header
(
	const ddp_packet_read_info_t * packet_info,
	uint16_t offset,
	ddp_message_read_info_t * out_message_info,
	ddp_request_id_t * out_request_id,
	ddp_status_t * out_status,
	uint16_t * out_num_chans
);

/**
* Reads data fields from a DDP routing Rx subscribe set response message for a particular channel
* @param message_info [in] Pointer to a structure which has information about the DDP packet buffer
* @param index [in] Index into the blocks of channel config state structures @see ddp_rx_chan_state_params_t array, values range from 0 to number of channels - 1
* @param out_rx_chan_data [out optional] Pointer to a structure of Rx channel state (memory should be allocated for this structure by the caller of this function) @see ddp_rx_channel_params_t
* @param out_num_custom_encodings [out optional] Pointer to the number of custom non-PCM encodings available for this channel
* @return AUD_SUCCESS if the data fields of a particular channel for the DDP routing Rx subscribe set response message was successfully read, else a non AUD_SUCCESS value is returned
*/
aud_error_t
ddp_read_routing_rx_sub_set_response_chan_params
(
	const ddp_message_read_info_t * message_info,
	uint16_t index,
	ddp_rx_chan_state_params_t * out_rx_chan_data,
	uint16_t * out_num_custom_encodings
);

/**
* Reads a custom encoding field from a DDP routing Rx subscribe set response message for a particular flow. N.B. This function should only be called after calling the ddp_read_routing_rx_sub_set_response_header function @see ddp_read_routing_rx_chan_config_state_response_header
* @param message_info [in] Pointer to a structure which has information about the DDP packet buffer, use the out_message_info parameter after calling the ddp_read_routing_rx_sub_set_response_header function
* @param chan_idx [in] Index into the collection of Rx channel config state data blocks in this response message. Values for this parameter range from 0 to number of channels - 1.
* @param custom_enc_idx [in] Index in the collection of custom non-PCM encoding data blocks in this response message. Values for this parameter range from 0 to number of custom encodings - 1.
* @param out_custom_encoding [out optional] Custom non-PCM encoding index at custom_enc_idx
* @return AUD_SUCCESS if the data fields of a particular channel for the DDP routing Rx channel subscribe set response message was successfully read, else a non AUD_SUCCESS value is returned
*/
aud_error_t
ddp_read_routing_rx_sub_set_response_custom_encoding
(
	ddp_message_read_info_t const * message_info,
	uint16_t chan_idx,
	uint16_t custom_enc_idx,
	uint16_t * out_custom_encoding
);

/**
* Creates a DDP routing Rx unsubscribe channel request message. This message can only be used to change the state of the device.
* @param packet_info [in] Pointer to a structure which has information about the DDP packet buffer
* @param request_id [in] A non-zero sequence number which is populated in this message
* @param from [in] Starting Rx channel ID, this value should be non-zero
* @param to [in] Ending Rx channel ID, this value should be non-zero and greater than the from value
* @return AUD_SUCCESS if the DDP routing Rx unsubscribe channel request message was successfully created, else a non AUD_SUCCESS value is returned
*/
aud_error_t
ddp_add_routing_rx_unsub_chan_request
(
	ddp_packet_write_info_t * packet_info,
	ddp_request_id_t request_id,
	uint16_t from,
	uint16_t to
);

/**
* Reads data fields from a DDP routing Rx unsubscribe channel response message
* @param packet_info [in] Pointer to a structure which has information about the DDP packet buffer
* @param offset [in] Starting point in the packet buffer to read this message
* @param out_request_id [out optional] Pointer to the sequence number for this message
* @param out_status [out optional] Pointer to the status of this message
* @return AUD_SUCCESS if the DDP routing Rx unsubscribe channel response message was successfully read, else a non AUD_SUCCESS value is returned
*/
aud_error_t
ddp_read_routing_rx_unsub_chan_response
(
	const ddp_packet_read_info_t * packet_info,
	uint16_t offset,
	ddp_request_id_t * out_request_id,
	ddp_status_t * out_status
);

/**
* Adds an array of slots to the multicast tx flow config parameters structure which will be passed into the ddp_add_routing_multicast_tx_flow_config_request function
* @param mcast_flow [in] Pointer to a structure which has parameters related to how to create the multicast Tx flow (memory should be allocated for this structure by the caller of this function) @see ddp_multicast_tx_flow_params_t
* @param slots [in] Pointer to an array of slots which will be used to build the multicast Tx flow config message (memory should be allocated for this structure by the caller of this function)
*/
void ddp_routing_multicast_tx_flow_config_add_slot_params
(
	ddp_multicast_tx_flow_params_t * mcast_flow,
	uint16_t * slots
);

/**
* Creates a DDP routing multicast Tx flow configuration request message. This message can only be used to change the state of the device.
* @param packet_info [in] Pointer to a structure which has information about the DDP packet buffer
* @param request_id [in] A non-zero sequence number which is populated in this message
* @param mcast_flow [in] Pointer to a structure which has parameters related to how to create the multicast Tx flow (memory should be allocated for this structure by the caller of this function) @see ddp_multicast_tx_flow_params_t
* @return AUD_SUCCESS if the DDP routing multicast Tx flow configuration request message was successfully created, else a non AUD_SUCCESS value is returned
*/
aud_error_t
ddp_add_routing_multicast_tx_flow_config_request
(
	ddp_packet_write_info_t * packet_info,
	ddp_request_id_t request_id,
	ddp_multicast_tx_flow_params_t * mcast_flow
);

/**
* Reads fixed data fields from a DDP routing multicast Tx flow configuration response message
* @param packet_info [in] Pointer to a structure which has information about the DDP packet buffer
* @param offset [in] Starting point in the packet buffer to read this message
* @param out_message_info [in out] Pointer to a structure which contains information about the payload and heap for this DDP packet (memory should be allocated for this structure by the caller of this function)
* @param out_request_id [out optional] Pointer to the sequence number for this request message
* @param out_status [out optional] Pointer to the status of this message
* @param out_num_flows [out optional] Pointer to the number of Tx flows that have state information in this response
* @return AUD_SUCCESS if the DDP routing multicast Tx flow configuration response message was successfully read, else a non AUD_SUCCESS value is returned
*/
aud_error_t
ddp_read_routing_multicast_tx_flow_config_response_header
(
	const ddp_packet_read_info_t * packet_info,
	uint16_t offset,
	ddp_message_read_info_t * out_message_info,
	ddp_request_id_t * out_request_id,
	ddp_status_t * out_status,
	uint16_t * out_num_flows
);

/**
* Reads data fields from a DDP routing multicast Tx flow configuration response message for a particular flow. N.B. This function should only be called after calling the ddp_read_routing_multicast_tx_flow_config_response_header function @see ddp_read_routing_multicast_tx_flow_config_response_header
* @param message_info [in] Pointer to a structure which has information about the DDP packet buffer, use the out_message_info parameter after calling the ddp_read_routing_multicast_tx_flow_config_response_header function
* @param flow_idx [in] Index into the collection of flow config state data blocks in this response message. Values for this parameter range from 0 to number of flows - 1.
* @param out_num_slots [out optional] Pointer to the number of Tx slots associated with this flow
* @param out_num_addresses [out optional] Pointer to number of IP address and port pairs associated with this flow
* @param out_tx_flow [out] Pointer to a Tx flow state information structure (memory should be allocated by the caller of this function) @see ddp_tx_flow_state_params_t for the flow indexed by flow_idx
* @return AUD_SUCCESS if the DDP routing multicast Tx flow configuration response message was successfully read, else a non AUD_SUCCESS value is returned
*/
aud_error_t
ddp_read_routing_multicast_tx_flow_config_response_flow_params
(
	const ddp_message_read_info_t * message_info,
	uint16_t flow_idx,
	uint16_t * out_num_slots,
	uint16_t * out_num_addresses,
	ddp_tx_flow_state_params_t * out_tx_flow
);

/**
* Reads the slots data fields from a DDP routing multicast Tx flow configuration response message for a particular flow. N.B. This function should only be called after calling the ddp_read_routing_tx_flow_config_state_response_header function @see ddp_read_routing_tx_flow_config_state_response_header
* @param message_info [in] Pointer to a structure which has information about the DDP packet buffer, use the out_message_info parameter after calling the ddp_read_routing_multicast_tx_flow_config_response_header function
* @param flow_idx [in] Index into the collection of flow config state data blocks in this response message. Values for this parameter range from 0 to number of flows - 1.
* @param out_slots [out optional] Pointer to an array of slots for the flow indexed by flow_idx
* @return AUD_SUCCESS if the DDP routing multicast Tx flow configuration response message was successfully read, else a non AUD_SUCCESS value is returned
*/
aud_error_t
ddp_read_routing_multicast_tx_flow_config_response_flow_slots
(
	const ddp_message_read_info_t * message_info,
	uint16_t flow_idx,
	uint16_t * out_slots
);

/**
* Reads an address data field from a DDP routing Tx flow configuration response message for a particular flow. N.B. This function should only be called after calling the ddp_read_routing_multicast_tx_flow_config_response_header function @see ddp_read_routing_multicast_tx_flow_config_response_header
* @param message_info [in] Pointer to a structure which has information about the DDP packet buffer, use the out_message_info parameter after calling the ddp_read_routing_multicast_tx_flow_config_response_header function
* @param flow_idx [in] Index into the collection of flow config state data blocks in this response message. Values for this parameter range from 0 to number of flows - 1.
* @param address_idx [in] Index into the collection of IP address and port data blocks for the flow indexed by flow_idx. Values for this parameter range from 0 to number of slots - 1.
* @param out_ip_address [out optional] Pointer to the IP address indexed by address_idx in a flow indexed by flow_idx
* @param out_port [out optional] Pointer to the port indexed by address_idx in a flow indexed by flow_idx
* @return AUD_SUCCESS if the DDP routing mulitcast Tx flow configuration response message was successfully read, else a non AUD_SUCCESS value is returned
* @deprecated Use \ref ddp_read_routing_multicast_tx_flow_config_state_response_flow_address_nw_ip instead
*/
aud_error_t
ddp_read_routing_multicast_tx_flow_config_state_response_flow_address_legacy
(
	const ddp_message_read_info_t * message_info,
	uint16_t flow_idx,
	uint16_t address_idx,
	uint32_t * out_ip_address,
	uint16_t * out_port
);

/**
* Reads an address data field from a DDP routing Tx flow configuration response message for a particular flow. N.B. This function should only be called after calling the ddp_read_routing_multicast_tx_flow_config_response_header function @see ddp_read_routing_multicast_tx_flow_config_response_header
* @param message_info [in] Pointer to a structure which has information about the DDP packet buffer, use the out_message_info parameter after calling the ddp_read_routing_multicast_tx_flow_config_response_header function
* @param flow_idx [in] Index into the collection of flow config state data blocks in this response message. Values for this parameter range from 0 to number of flows - 1.
* @param address_idx [in] Index into the collection of IP address and port data blocks for the flow indexed by flow_idx. Values for this parameter range from 0 to number of slots - 1.
* @param out_ip_address [out optional] Pointer to the IP address indexed by address_idx in a flow indexed by flow_idx
* @param out_port [out optional] Pointer to the port indexed by address_idx in a flow indexed by flow_idx
* @return AUD_SUCCESS if the DDP routing mulitcast Tx flow configuration response message was successfully read, else a non AUD_SUCCESS value is returned
*/
aud_error_t
ddp_read_routing_multicast_tx_flow_config_state_response_flow_address_nw_ip
(
	const ddp_message_read_info_t * message_info,
	uint16_t flow_idx,
	uint16_t address_idx,
	ddp_ip_addr_nw_t * out_ip_address,
	uint16_t * out_port
);

/**
* Creates a DDP routing flow delete request message. This message can only be used to change the state of the device.
* @param packet_info [in] Pointer to a structure which has information about the DDP packet buffer
* @param request_id [in] A non-zero sequence number which is populated in this message
* @param from [in] Starting flow ID, this value should be non-zero
* @param to [in] Ending flow ID, this value should be non-zero and greater than the from value
* @param filter [in] Bitwise OR'd flags which specify the types of flows to delete @see ddp_routing_flow_filter
* @return AUD_SUCCESS if the DDP routing flow delete request message was successfully created, else a non AUD_SUCCESS value is returned
*/
aud_error_t
ddp_add_routing_flow_delete_request
(
	ddp_packet_write_info_t * packet_info,
	ddp_request_id_t request_id,
	uint16_t from,
	uint16_t to,
	uint16_t filter
);

/**
* Reads data fields from a DDP routing flow delete response message
* @param packet_info [in] Pointer to a structure which has information about the DDP packet buffer
* @param offset [in] Starting point in the packet buffer to read this message
* @param out_request_id [out optional] Pointer to the sequence number for this request message
* @param out_status [out optional] Pointer to the status of this message
* @return AUD_SUCCESS if the DDP routing flow delete response message was successfully read, else a non AUD_SUCCESS value is returned
*/
aud_error_t
ddp_read_routing_flow_delete_response
(
	const ddp_packet_read_info_t * packet_info,
	uint16_t offset,
	ddp_request_id_t * out_request_id,
	ddp_status_t * out_status
);

/**@}*/
/**@}*/
/**@}*/

#ifdef __cplusplus
}
#endif

#endif // _DDP_ROUTING_CLIENT__H

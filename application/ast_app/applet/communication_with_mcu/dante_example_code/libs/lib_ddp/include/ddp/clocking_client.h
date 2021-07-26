/*
 * File     : clocking_client.h
 * Created  : August 2014
 * Updated  : Date: 2014/08/22
 * Author   : Michael Ung <michael.ung@audinate.com>
 * Synopsis : All clocking DDP client helper functions.
 *
 * Copyright 2005-2017, Audinate Pty Ltd and/or its licensors
	Confidential
 * Audinate Copyright Header Version 1
 */
#ifndef _DDP_CLOCKING_CLIENT_H
#define _DDP_CLOCKING_CLIENT_H

#include "ddp/packet.h"
#include "ddp/clocking_structures.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
* @file clocking_client.h
* @brief All clocking DDP client helper functions.
*/

/** \addtogroup DDP
*  @{
*/

/**
* \defgroup Clocking DDP Clocking
* @{
*/

/**
* \defgroup ClockingStructs DDP Clocking Structures
* @{
*/

/**
* @struct clock_pullup_control_fields
* @brief Structure used to set clock pullup related information
*/
struct clock_pullup_control_fields
{
	uint16_t 			valid_flags;		/*!< Bitwise OR'd valid flags @see clock_pullup_control_flags, set this to zero to query the clock pullup related information */
	uint8_t 			pullup;				/*!< Pullup value which needs to be applied, valid only if CLOCK_PULLUP_SET_PULLUP is set in the valid_flags field */
	const uint8_t * 	subdomain;			/*!< Subdomain string. This value cannot be directly set in the ddp_add_clock_pullup_request function but can be used to read the subdomain string in the request message */
	uint16_t 			subdomain_length;	/*!< Length of the subdomain. This value cannot be directly set in the ddp_add_clock_pullup_request function but can be used to read the subdomain string length in the request message */
};

/** @cond */
typedef struct clock_pullup_control_fields clock_pullup_control_fields_t;
/** @endcond */

/**
* @struct clock_supported_pullup
* @brief Structure format of the supported pullups
*/
struct clock_supported_pullup
{
	uint8_t pullup;		/*!< A supported pullup value @see clock_pullup */
	uint8_t pad0;		/*!< Pad for alignment - must be '0' */
	uint16_t pad1;		/*!< Pad for alignment - must be '0' */
};

/** @cond */
typedef struct clock_supported_pullup clock_supported_pullup_t;
/** @endcond */

/**
* @struct ddp_clock_config_control_params
* @brief Parameters passed in for ddp_add_clock_config_request and ddp_read_clock_config_response
*/
struct ddp_clock_config_control_params
{
	uint16_t control_flags;						/*!< Control flags @see clock_config_control_flags */
	uint8_t preferred;							/*!< Preferred PTP clock master setting @see clock_preferred */
	uint8_t ext_word_clock_sync;				/*!< External word clock sync setting @see clock_ext_wc_sync */
	uint8_t logging;							/*!< PTP logging setting @see clock_logging */
	uint8_t multicast;							/*!< Multicast PTP setting @see clock_multicast */
	uint8_t slave_only;							/*!< Slave only setting @see clock_slave_only */
	uint8_t clock_protocol;						/*!< Bitmap of clock protocols to enable/disable @see clock_protocol, must be bit-wise AND-ed with clock_protocol_mask for validity */
	uint8_t clock_protocol_mask;				/*!< Bitmask of which bits in the clock_protocol are valid @see clock_protocol */
	uint8_t unicast_delay_clock_protocol;		/*!< Bitmamp of clock protocols to enable/disable unicast delay requests @see clock_protocol, must be bit-wise AND-ed with unicast_delay_clock_protocol_mask for validity */
	uint8_t unicast_delay_clock_protocol_mask;	/*!< Bitmask of which bits in the unicast_delay_clock_protocol_mask are valid @see unicast_delay_clock_protocol_mask */
};

/** @cond */
typedef struct ddp_clock_config_control_params ddp_clock_config_control_params_t;
/** @endcond */

/**
* @struct ddp_clock_config_port_params
* @brief Clock port related parameters passed in for ddp_add_clock_config_request and ddp_read_clock_config_response_port
*/
struct ddp_clock_config_port_params
{
	uint16_t port_id;		/*!< The ID of the port, starts from 1 */
	uint8_t enable_port;	/*!< Port enable setting @see clock_port_enable */
};

/** @cond */
typedef struct ddp_clock_config_port_params ddp_clock_config_port_params_t;
/** @endcond */

/**@}*/

/**
* \defgroup ClockFuncs DDP Clocking Functions
* @{
*/

#if defined(AUD_PLATFORM_ULTIMO)

/**
* Creates a DDP clock basic legacy request message. This message can only be used to query the current state of the device. 
* @param packet_info [in] Pointer to a structure which has information about the DDP packet buffer
* @param request_id [in] A non-zero sequence number which is populated in this message
* @return AUD_SUCCESS if the DDP clock basic legacy request message was successfully created, else a non AUD_SUCCESS value is returned
* @deprecated Use \ref ddp_add_clock_basic2_request instead
*/
aud_error_t
ddp_add_clock_basic_legacy_request
(
	ddp_packet_write_info_t * packet_info,
	ddp_request_id_t request_id
);

/**
* Reads data fields from a DDP clock basic legacy response message
* @param packet_info [in] Pointer to a structure which has information about the DDP packet buffer
* @param offset [in] Starting point in the packet buffer to read this message
* @param out_request_id [out optional] Pointer to the sequence number for this request message
* @param out_status [out optional] Pointer to the status of this message
* @param out_capability_flags [out optional] Bitwise OR'd capability flags of this response @see clock_basic_capability_flags
* @param out_clock_state [out optional] Pointer to the clock state of the device
* @param out_mute_state [out optional] Pointer to the mute state of the device
* @param out_ext_wc_state [out optional] Pointer to the external mute state of the device
* @param out_preferred [out optional] Pointer to the preferred clock master state of the device
* @param out_drift [out optional] Pointer to the clock drift value of the device in PPM
* @return AUD_SUCCESS if the DDP clock basic legacy response message was successfully read, else a non AUD_SUCCESS value is returned
* @deprecated Use \ref ddp_read_clock_basic2_response_header and \ref ddp_read_clock_basic2_response_port instead
*/
aud_error_t
ddp_read_clock_basic_legacy_response
(
	const ddp_packet_read_info_t * packet_info,
	uint16_t offset,
	ddp_request_id_t * out_request_id,
	ddp_status_t * out_status,
	uint32_t* out_capability_flags,
	uint8_t* out_clock_state,
	uint8_t* out_mute_state,
	uint8_t* out_ext_wc_state,
	uint8_t* out_preferred,
	uint32_t* out_drift
);

#endif // defined(AUD_PLATFORM_ULTIMO)

/**
* Creates a DDP clock config request message. This message can be used as a command to change the state of the device and as a request to query the current state of the device.
* @param packet_info [in] Pointer to a structure which has information about the DDP packet buffer
* @param request_id [in] A non-zero sequence number which is populated in this message
* @param config_params [in] Pointer to the clock config related information that needs to be applied on the device @see clock_config_control_params
* @param num_clock_ports [in] Number of clock ports to configure, if none needs to be configured then set this parameter to zero
* @param port_params [in optional] Pointer to an array of clock port related configuration information @see clock_config_port_params_t
* @return AUD_SUCCESS if the DDP clock config request message was successfully created, else a non AUD_SUCCESS value is returned
*/
aud_error_t
ddp_add_clock_config_request
(
	ddp_packet_write_info_t * packet_info,
	ddp_request_id_t request_id,
	ddp_clock_config_control_params_t * config_params,
	uint16_t num_clock_ports,
	ddp_clock_config_port_params_t * port_params
);

/**
* Reads data fields from a DDP clock config response message
* @param packet_info [in] Pointer to a structure which has information about the DDP packet buffer
* @param offset [in] Starting point in the packet buffer to read this message
* @param out_message_info [in out] Pointer to the structure which has information about the DDP clock config message (memory should be allocated for this structure by the caller of this function)
* @param out_request_id [out optional] Pointer to the sequence number for this request message
* @param out_status [out optional] Pointer to the status of this message
* @param out_config_params [out optional] Pointer to the clock configuration params structure @see clock_config_control_params
* @param out_num_clock_ports [out optional] Pointer to the number of clock port structures contained within this message
* @return AUD_SUCCESS if the DDP clock config response message was successfully read, else a non AUD_SUCCESS value is returned
*/
aud_error_t
ddp_read_clock_config_response
(
	const ddp_packet_read_info_t * packet_info,
	uint16_t offset,
	ddp_message_read_info_t * out_message_info,
	ddp_request_id_t * out_request_id,
	ddp_status_t * out_status,
	ddp_clock_config_control_params_t * out_config_params,
	uint16_t * out_num_clock_ports
);

/**
* Reads data fields for a clock config port within a DDP clock config response message. Please call this function after calling ddp_read_clock_config_response
* @param message_info [in] Pointer to a structure which has information about the DDP clock config message
* @param port_idx [in] Index into the collection of clock ports in the DDP clock config message
* @param out_port_params [out optional] Pointer to the clock port configuration params structure @see clock_config_port_params
* @return AUD_SUCCESS if the DDP clock config response message was successfully read, else a non AUD_SUCCESS value is returned
*/
aud_error_t
ddp_read_clock_config_response_port
(
	const ddp_message_read_info_t * message_info,
	uint16_t port_idx,
	ddp_clock_config_port_params_t * out_port_params
);

/**
* Creates a DDP clock pullup request message. This message can be used as a command to change the state of the device and as a request to query the current state of the device.
* @param packet_info [in] Pointer to a structure which has information about the DDP packet buffer
* @param request_id [in] A non-zero sequence number which is populated in this message
* @param control_fields [in] The clock pullup related information that needs to be applied on the device @see clock_pullup_control_fields
* @return AUD_SUCCESS if the DDP clock pullup request message was successfully created, else a non AUD_SUCCESS value is returned
*/
aud_error_t
ddp_add_clock_pullup_request
(
	ddp_packet_write_info_t * packet_info,
	ddp_request_id_t request_id,
	clock_pullup_control_fields_t * control_fields
);

/**
* Reads data fields from a DDP clock pullup response message
* @param packet_info [in] Pointer to a structure which has information about the DDP packet buffer
* @param offset [in] Starting point in the packet buffer to read this message
* @param out_request_id [out optional] Pointer to the sequence number for this request message
* @param out_status [out optional] Pointer to the status of this message
* @param out_current_pullup [out optional] Pointer to the current pullup value of the device @see clock_pullup
* @param out_reboot_pullup [out optional] Pointer to the pullup value used after the device reboots @see clock_pullup
* @param out_current_subdomain [out optional] Pointer to a pointer to the current subdomain string of the device
* @param out_current_subdomain_length [out optional] Pointer to the length of the current subdomain string 
* @param out_reboot_subdomain [out optional] Pointer to a pointer to the subdomain string used after the device reboots
* @param out_reboot_subdomain_length [out optional] Pointer to the length of the reboot subdomain string 
* @param out_supported_pullups [out optional] Pointer to a pointer to an array of structures which has information about the supported pullups of the device @see clock_supported_pullup
* @param out_num_supported_pullups [out optional] Pointer to the number of pullups supported by the device
* @return AUD_SUCCESS if the DDP clock pullup response message was successfully read, else a non AUD_SUCCESS value is returned
*/
aud_error_t
ddp_read_clock_pullup_response
(
	const ddp_packet_read_info_t * packet_info,
	uint16_t offset,
	ddp_request_id_t * out_request_id,
	ddp_status_t * out_status,
	uint8_t * out_current_pullup,
	uint8_t * out_reboot_pullup,
	const uint8_t ** out_current_subdomain,
	uint16_t * out_current_subdomain_length,
	const uint8_t ** out_reboot_subdomain,
	uint16_t* out_reboot_subdomain_length,
	const clock_supported_pullup_t ** out_supported_pullups,
	uint16_t * out_num_supported_pullups
);

/**
* Creates a DDP clock basic 2 request message. This message can only be used to query the current state of the device.
* @param packet_info [in] Pointer to a structure which has information about the DDP packet buffer
* @param request_id [in] A non-zero sequence number which is populated in this message
* @return AUD_SUCCESS if the DDP clock basic 2 request message was successfully created, else a non AUD_SUCCESS value is returned
*/
aud_error_t
ddp_add_clock_basic2_request
(
	ddp_packet_write_info_t * packet_info,
	ddp_request_id_t request_id
);

/**
* Reads data fields from a DDP clock basic legacy response message
* @param packet_info [in] Pointer to a structure which has information about the DDP packet buffer
* @param offset [in] Starting point in the packet buffer to read this message
* @param out_message_info [in out] Pointer to a structure which contains information about the payload and heap for this DDP packet (memory should be allocated for this structure by the caller of this function)
* @param out_request_id [out optional] Pointer to the sequence number for this request message
* @param out_status [out optional] Pointer to the status of this message
* @param out_capability_flags [out optional] Bitwise OR'd capability flags of this response @see clock_basic_capability_flags
* @param out_clock_source [out optional] Pointer to the clock source of the device
* @param out_clock_state [out optional] Pointer to the clock state of the device
* @param out_servo_state [out optional] Pointer to the servo state of the device
* @param out_preferred [out optional] Pointer to the preferred clock master state of the device
* @param out_mute_state [out optional] Pointer to the mute state of the device
* @param out_ext_wc_state [out optional] Pointer to the external mute state of the device
* @param out_clock_stratum [out optional] Pointer to the clock stratum of the device
* @param out_drift [out optional] Pointer to the clock drift value of the device in PPM
* @param out_max_drift [out optional] Pointer to the max clock drift value of the device in PPM
* @param out_uuid [out optional] Pointer to a pointer to the uuid of the device
* @param out_master_uuid [out optional] Pointer to the pointer to the clock master uuid
* @param out_grandmaster_uuid [out optional] Pointer to the pointer to the clock grandmaster uuid
* @param out_num_ports [out optional] Pointer to the number of port structures
* @param out_port_size [out optional] Pointer to the size of each ptp port structure
* @return AUD_SUCCESS if the DDP clock basic 2 response message was successfully read, else a non AUD_SUCCESS value is returned
*/
aud_error_t
ddp_read_clock_basic2_response_header
(
	const ddp_packet_read_info_t * packet_info,
	uint16_t offset,
	ddp_message_read_info_t * out_message_info,
	ddp_request_id_t * out_request_id,
	ddp_status_t * out_status,
	uint32_t * out_capability_flags,
	uint8_t * out_clock_source,
	uint8_t * out_clock_state,
	uint8_t * out_servo_state,
	uint8_t * out_preferred,
	uint8_t * out_mute_state,
	uint8_t * out_ext_wc_state,
	uint8_t * out_clock_stratum,
	uint32_t * out_drift,
	uint32_t * out_max_drift,
	uint8_t const ** out_uuid,
	uint8_t const ** out_master_uuid,
	uint8_t const ** out_grandmaster_uuid,
	uint8_t * out_num_ports,
	uint8_t * out_port_size
);

/**
* Reads the clock port related data fields from a DDP clock basic2 response message for a particular clock port. N.B. This function should only be called after calling the ddp_read_clock_basic2_response_header function @see ddp_read_clock_basic2_response_header
* @param message_info [in] Pointer to a structure which has information about the DDP packet buffer, use the out_message_info parameter after calling the ddp_read_clock_basic2_response_header function
* @param port_idx [in] Index into the clock port blocks of information, values range from 0 to number of ports - 1
* @param out_port [out optional] Pointer to the pointer to the clock port structure @see ddp_clock_port_t
* @return AUD_SUCCESS if the data fields of a particular network interface for the DDP network basic response message was successfully read, else a non AUD_SUCCESS value is returned
*/
aud_error_t
ddp_read_clock_basic2_response_port
(
	const ddp_message_read_info_t * message_info,
	uint8_t port_idx,
	ddp_clock_port_t ** out_port
);

/**@}*/
/**@}*/
/**@}*/

#ifdef __cplusplus
}
#endif


#endif // _DDP_CLOCKING_CLIENT_H

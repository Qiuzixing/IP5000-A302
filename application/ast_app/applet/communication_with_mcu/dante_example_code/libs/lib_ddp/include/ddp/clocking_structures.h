/*
 * File     : clocking_structures.h
 * Created  : May 2014
 * Updated  : Date: 2014/05/16
 * Author   : Michael Ung <michael.ung@audinate.com>
 * Synopsis : All clocking DDP messages structure definitions.
 *
 * Copyright 2005-2017, Audinate Pty Ltd and/or its licensors
	Confidential
 * Audinate Copyright Header Version 1
 */
#ifndef _DDP_CLOCKING_STRUCTURES_H
#define _DDP_CLOCKING_STRUCTURES_H

#include "ddp/ddp.h"

/**
* @file clocking_structures.h
* @brief All clocking ddp messages structure definitions.
*/

/** \addtogroup DDP
*  @{
*/

/** \addtogroup Clocking
*  @{
*/

/**
* \defgroup ClockingEnums DDP Clocking Enums
* @{
*/

/**
* @enum clock_basic_capability_flags
* @brief Clock basic capability_flags values for ddp_clock_basic2_response
*/
enum clock_basic_capability_flags
{
	CAPABILITY_SUPPORT_PULLUP_SUBDOMAINS = 0x00000001,				/*!< Support for clock pullup/subdomains is available */
	CAPABILITY_SUPPORT_UNICAST_PTPV1 = 0x00000002,					/*!< Support for clock unicast PTP version 1 ports is available */
	CAPABILITY_SUPPORT_DYNAMIC_CLOCK_LOGGING = 0x00000004,			/*!< Support for dynamic clock logging is available */
	CAPABILITY_SUPPORT_EXTERNAL_WC = 0x00000008,					/*!< Support for using an external word clock is available */
	CAPABILITY_SLAVE_ONLY = 0x00000010,								/*!< Support for slave only operation is available */
	CAPABILITY_PER_PROTOCOL_UNICAST_DELAY_REQUESTS = 0x00000020,	/*!< Support for enable/disable unicast delay requests on a per protocol basis */
	CAPABILITY_CLOCK_PORT_USER_DISABLE = 0x00000040,				/*!< Support for enable/disable individual PTP clock ports */
	CAPABILITY_SUPPORT_UNICAST_PTPV2 = 0x00000080					/*!< Support for clock unicast PTP version 2 ports is available */
};

/**
* @enum clock_source
* @brief Clock basic clock source values for ddp_clock_basic2_response
*/
enum clock_source
{
	CLOCK_SOURCE_INVALID = 0,		/*!< Clock source invalid */
	CLOCK_SOURCE_INTERNAL = 1,		/*!< Internal clock source */
	CLOCK_SOURCE_EXTERNAL_WC = 2,	/*!< External word clock */
};

/**
 * @enum clock_state_legacy
 * @brief Clock basic clock state values for ddp_clock_basic_legacy_response
 */
enum clock_state_legacy
{
	CLOCK_STATE_LEGACY_DISABLED = 0,					/*!< PTP clock is in a disabled state  */
	CLOCK_STATE_LEGACY_MASTER_INTERNAL_CLOCK = 1,		/*!< PTP clock is in master state and the device is using an internal clock source */
	CLOCK_STATE_LEGACY_MASTER_EXTERNAL_CLOCK = 2,		/*!< PTP clock is in master state and the device is using an external clock source */
	CLOCK_STATE_LEGACY_SLAVE_IN_SYNC = 3,				/*!< PTP clock is in slave state and it is in sync */
	CLOCK_STATE_LEGACY_SLAVE_NO_SYNC = 4,				/*!< PTP clock is in slave state and it is not in sync */
};

/**
 * @enum clock_state
 * @brief Clock basic clock state values for ddp_clock_basic2_response
 */
enum clock_state
{
	CLOCK_STATE_INVALID = 0,		/*!< Clock state is invalid */
	CLOCK_STATE_NONE = 1,			/*!< Clock state is not being used */
	CLOCK_STATE_PASSIVE = 2,		/*!< Clock state is inactive */
	CLOCK_STATE_UNDISCIPLINED = 3,	/*!< Clock state is active but not disciplined */
	CLOCK_STATE_DISCIPLINED = 4,	/*!< Clock state is active and disciplined */
};

/**
 * @enum clock_servo_state
 * @brief Clock basic servo state values for ddp_clock_basic2_response
 */
enum clock_servo_state
{
	CLOCK_SERVO_STATE_INVALID = 0,		/*!< servo state is invalid */
	CLOCK_SERVO_STATE_UNKNOWN = 1,		/*!< servo state is unknown */
	CLOCK_SERVO_STATE_RESET = 2,		/*!< servo state is reset */
	CLOCK_SERVO_STATE_SYNCING = 3,		/*!< servo state is "locking" */
	CLOCK_SERVO_STATE_SYNC = 4,			/*!< servo state is "locked" */
	CLOCK_SERVO_STATE_DELAY_RESET = 5,	/*!< servo state is delay reset */
};

/**
 * @enum clock_mute_state
 * @brief Clock basic clock mute state values for ddp_clock_basic_response
 */
enum clock_mute_state
{
	MUTE_STATE_NOT_MUTED = 0,				/*!< PTP clocking is operating normally and audio is not muted */
	MUTE_STATE_MUTED_EXTERNAL_CLOCK = 1,	/*!< PTP clocking is not operating normally, the device is using an external clock source, and audio is muted */
	MUTE_STATE_MUTED_INTERNAL_CLOCK = 2,	/*!< PTP clocking is not operating normally, the device is using an internal clock source, and audio is muted */
	MUTE_STATE_MUTED_USER = 3,				/*!< user requested audio mute */
};

/**
 * @enum clock_ext_wc_state
 * @brief Clock basic external word clock values for ddp_clock_basic_response
 */
enum clock_ext_wc_state
{
	EXT_WC_STATE_NOT_SUPPORTED = 0,	/*!< An external word clocksource is not supported */
	EXT_WC_STATE_UNKNOWN = 1,	/*!< External word clock is in an unknown state */
	EXT_WC_STATE_NONE = 2,	/*!< External word clock state is none */
	EXT_WC_STATE_INVALID = 3,	/*!< External word clock state is invalid */
	EXT_WC_STATE_VALID = 4,	/*!< External word clock state is valid */
	EXT_WC_STATE_MISSING = 5,	/*!< External word clock state is missing */
};

/**
 * @enum clock_preferred
 * @brief Clock basic preferred state values for ddp_clock_basic_response
 */
enum clock_preferred
{
	CLOCK_NOT_PREFERRED = 0,	/*!< PTP clock is not set to preferred master */
	CLOCK_PREFERRED = 1,		/*!< PTP clock is set to preferred master */
};

/**
 * @enum clock_castness
 * @brief Clock port castness preferred values for ddp_clock_port
 */
enum clock_castness
{
	CLOCK_CASTNESS_INVALID = 0,		/*!< clock castness is invalid */
	CLOCK_CASTNESS_UNICAST,			/*!< clock castness is unicast */
	CLOCK_CASTNESS_MULTICAST,		/*!< clock castness is multicast */
};

/**
 * @enum clock_port_state
 * @brief Clock port state values for ddp_clock_port
 */
enum clock_port_state
{
	CLOCK_PORT_STATE_INVALID = 0,	/*!< clock port state is invalid */
	CLOCK_PORT_STATE_STARTUP,		/*!< clock port state is startup */
	CLOCK_PORT_STATE_INITIALIZING,	/*!< clock port state is initialising */
	CLOCK_PORT_STATE_FAULTY,		/*!< clock port state is faulty */
	CLOCK_PORT_STATE_DISABLED,		/*!< clock port state is disabled */
	CLOCK_PORT_STATE_LISTENING,		/*!< clock port state is listening */
	CLOCK_PORT_STATE_PRE_MASTER,	/*!< clock port state is pre-master */
	CLOCK_PORT_STATE_MASTER,		/*!< clock port state is master */
	CLOCK_PORT_STATE_PASSIVE,		/*!< clock port state is passive */
	CLOCK_PORT_STATE_UNCALIBRATED,	/*!< clock port state is uncalibrated */
	CLOCK_PORT_STATE_SLAVE,			/*!< clock port state is slave */
	CLOCK_PORT_STATE_STANDBY	/*!< clock port state is unicast standby */
};

/**
 * @enum clock_port_flags
 * @brief Clock port bit flags for ddp_clock_port
 */
enum clock_port_flags
{
	CLOCK_PORT_FLAG_USER_DISABLED =			0x01,	/*!< Port is disabled */
	CLOCK_PORT_FLAG_LINK_DOWN = 			0x02,	/*!< Network link is down */
	CLOCK_PORT_FLAG_UNICAST_DELAY_ENABLED =	0x04,	/*!< Port has unicast delay enabled */
};

/**
* @enum clock_config_control_flags
* @brief Clock config control_flags values for ddp_clock_config_request
*/
enum clock_config_control_flags
{
	CONTROL_FLAG_SET_PREFERRED =						0x0001,	/*!< Set device to be a preferred master */
	CONTROL_FLAG_SET_SLAVE_TO_WC =						0x0002,	/*!< Set slave device to use an external word clock */
	CONTROL_FLAGS_SET_LOGGING =							0x0004,	/*!< Set PTP logging */
	CONTROL_FLAGS_SET_MULTICAST =						0x0008,	/*!< Set multicast */
	CONTROL_FLAGS_SET_SLAVE_ONLY =						0x0010,	/*!< Set slave only */
	CONTROL_FLAGS_SET_CLOCK_PROTOCOL =					0x0020,	/*!< Set clock protocols */
	CONTROL_FLAGS_SET_UNICAST_DELAY_CLOCK_PROTOCOL =	0x0040,	/*!< Set unicast delay for clock protocols */
};

/**
 * @enum clock_ext_wc_sync
 * @brief Clock config ext_word_clock_sync values for ddp_clock_config_request / ddp_clock_config_response
 */
enum clock_ext_wc_sync
{
	EXT_WC_SYNC_DISABLED = 0,	/*!< External word clock syncing disabled */
	EXT_WC_SYNC_ENABLED = 1,	/*!< External word clock syncing enabled */
};

/**
 * @enum clock_logging
 * @brief Clock config logging values for ddp_clock_config_request / ddp_clock_config_response
 */
enum clock_logging
{
	LOGGING_DISABLED = 0,	/*!< PTP logging disabled */
	LOGGING_ENABLED = 1,	/*!< PTP logging enabled */
};

/**
* @enum clock_port_enable
* @brief Clock port enable/disable setting for ddp_clock_config_request / ddp_clock_config_response
*/
enum clock_port_enable
{
	PORT_ENABLE_INVALID = 0,	
	PORT_ENABLE_DISABLED,
	PORT_ENABLE_ENABLED,
};

/**
 * @enum clock_multicast
 * @brief Clock config multicast values for ddp_clock_config_request / ddp_clock_config_response
 */
enum clock_multicast
{
	MULTICAST_INVALID = 0,		/*!< multicast invalid */
	MULTICAST_DISABLED,			/*!< multicast disabled */
	MULTICAST_ENABLED,			/*!< multicast enabled */
};

/**
 * @enum clock_slave_only
 * @brief Clock config slave only values for ddp_clock_config_request / ddp_clock_config_response
 */
enum clock_slave_only
{
	SLAVE_ONLY_INVALID = 0,		/*!< slave only invalid */
	SLAVE_ONLY_DISABLED,		/*!< slave only disabled */
	SLAVE_ONLY_ENABLED,			/*!< slave only enabled */
};

/**
 * @enum clock_protocol
 * @brief Clock config protocol bitmask for ddp_clock_config_request / ddp_clock_config_response
 */
enum clock_protocol
{
	CLOCK_PROTOCOL_LAYER_3_PTP_VERSION_1 = 0x01,		/*!< Layer 3 PTP version 1 */
	CLOCK_PROTOCOL_LAYER_3_PTP_VERSION_2 = 0x02,		/*!< Layer 3 PTP version 2 */
};

/**
* @enum clock_config_response_valid_flags
* @brief Clock config valid_flags values for ddp_clock_config_response
*/
enum clock_config_response_valid_flags
{
	CLOCK_CONFIG_RSP_PREFERRED_VALID =						0x0001,		/*!< preferred field is valid */
	CLOCK_CONFIG_RSP_WC_SYNC_VALID =						0x0002,		/*!< word_clock_sync is valid */
	CLOCK_CONFIG_RSP_LOGGING_VALID =						0x0004,		/*!< logging is valid */
	CLOCK_CONFIG_RSP_MULTICAST_VALID =						0x0008,		/*!< multicast is valid */
	CLOCK_CONFIG_RSP_SLAVE_ONLY_VALID =						0x0010,		/*!< slave only is valid */
	CLOCK_CONFIG_RSP_CLOCK_PROTOCOL_VALID =					0x0020,		/*!< clock protocol enable/disable is valid */
	CLOCK_CONFIG_RSP_UNICAST_DELAY_CLOCK_PROTOCOL_VALID =	0x0040,		/*!< unicast delay for clock protocol is valid */
};

/**
* @enum clock_config_port_response_valid_flags
* @brief Clock port response flags for ddp_clock_port
*/
enum clock_config_port_response_valid_flags
{
	CLOCK_PORT_CONTROL_RSP_ENABLE_VALID =		0x0001,
	CLOCK_PORT_CONTROL_RSP_UNICAST_DELAY_VALID = 0x0002,
};

/**
* @enum clock_pullup_control_flags
* @brief Clock pullup control_flags values for ddp_clock_pullup_request
*/
enum clock_pullup_control_flags
{
	CLOCK_PULLUP_SET_PULLUP = 0x0001,		/*!< Set a pullup */
	CLOCK_PULLUP_SET_SUBDOMAIN = 0x0002		/*!< Set the subdomain */
};

/**
* @enum clock_pullup
* @brief Clock pullup values for ddp_clock_pullup_request
*/
enum clock_pullup
{
	CLOCK_PULLUP_NO_PULLUP = 0,		/*!< No pullup value */
	CLOCK_PULLUP_POS_4_1667_PCENT,	/*!< Pullup of +4.1667% */
	CLOCK_PULLUP_POS_0_1_PCENT,		/*!< Pullup of +0.1% */
	CLOCK_PULLUP_NEG_0_1_PCENT,		/*!< Pullup of -0.1% */
	CLOCK_PULLUP_NEG_4_0_PCENT4		/*!< Pullup of -4.0% */
};

/**
* @enum clock_pullup_valid_flags
* @brief Clock pullup valid_flags values for ddp_clock_pullup_response
*/
enum clock_pullup_valid_flags
{
	CLOCK_PULLUP_CURR_PULLUP_VALID = 		0x0001,	/*!< current_pullup field is valid */
	CLOCK_PULLUP_REBOOT_PULLUP_VALID = 		0x0002,	/*!< reboot_pullup field is valid */
	CLOCK_PULLUP_SUPPORTED_PULLUPS_VALID = 	0x0004,	/*!< num_supported_pullups, supported_pullups_size, and supported_pullups_offset fields are valid */
	CLOCK_PULLUP_CURR_SUBDOMAIN_VALID = 	0x0008,	/*!< current_subdomain_offset field is valid */
	CLOCK_PULLUP_REBOOT_SUBDOMAIN_VALID = 	0x0010	/*!< reboot_subdomain_offset field is valid */
};

/**@}*/

/**
* \defgroup ClockingMacros DDP Clocking Macros
* @{
*/

/**
* The length of an Audinate clock ID
*/
#define DDP_CLOCK_UUID_LENGTH 6	

/**@}*/

/**
* \defgroup ClockingStructs DDP Clocking Structures
* @{
*/

/**
* @struct ddp_clock_basic_legacy_request
* @brief Structure format for the "clock basic legacy request" message
* @deprecated - use ddp_clock_basic2_request instead
*/
struct ddp_clock_basic_legacy_request
{
	ddp_message_header_t header;		/*!< message header */
	ddp_request_subheader_t subheader;	/*!< request message subheader */
};

/**
* @struct ddp_clock_basic_legacy_response
* @brief Structure format for the "clock basic legacy response" message
* @deprecated - use ddp_clock_basic2_response instead
*/
struct ddp_clock_basic_legacy_response
{
	ddp_message_header_t header;			/*!< message header */
	ddp_response_subheader_t subheader;		/*!< response message subheader */

	//! Clock basic response data
	struct ddp_clock_basic_legacy_response__payload
	{
		uint32_t capability_flags;			/*!< Capability flags @see clock_basic_capability_flags */
		uint8_t clock_state_legacy;			/*!< Current PTP clock state @see clock_state_legacy */
		uint8_t mute_state;					/*!< Current mute state of audio which is related to the PTP clocking @see clock_mute_state */
		uint8_t ext_wc_state;				/*!< Current external word clock state @see clock_ext_wc_state */
		uint8_t preferred;					/*!< Current PTP clocking master preferred setting @see clock_preferred */
		uint32_t drift;						/*!< Current clock drift in parts per million (PPM) */
	} payload;	/*!< fixed payload */
};

/**
 * @struct ddp_clock_basic2_request
 * @brief Structure format for the "clock basic2 request" message
 */
struct ddp_clock_basic2_request
{
	ddp_message_header_t header;		/*!< message header */
	ddp_request_subheader_t subheader;	/*!< request message subheader */
};

/**
 * @struct ddp_clock_uuid
 * @brief Structure format for the ddp clock UUID's
 */
struct ddp_clock_uuid
{
	uint8_t data[DDP_CLOCK_UUID_LENGTH];	/*!< The UUID */
	uint16_t padding;						/*!< Pad for alignment - must be '0' */
};

//! @cond Doxygen_Suppress
typedef struct ddp_clock_uuid ddp_clock_uuid_t;
//! @cond

/**
 * @struct ddp_clock_port
 * @brief Structure format for each clock port message
 */
struct ddp_clock_port
{
	uint16_t flags;				/*!< per port bit flags @see clock_port_flags */
	uint16_t port_id;			/*!< unique port id for each port */
	uint8_t port_protocol;		/*!< ptp port protocol @see clock_protocol */
	uint8_t castness;			/*!< unicast or multicast @see clock_castness */
	uint8_t interface_index;	/*!< port interface index */
	uint8_t port_state;			/*!< port state @see clock_port_state */
};

//! @cond Doxygen_Suppress
typedef struct ddp_clock_port ddp_clock_port_t;
//! @endcond

/**
 * @struct ddp_clock_basic2_response
 * @brief Structure format for the "clock basic2 response" message
 */
struct ddp_clock_basic2_response
{
	ddp_message_header_t header;			/*!< message header */
	ddp_response_subheader_t subheader;		/*!< response message subheader */

	//! Clock basic2 response data
	struct ddp_clock_basic2_response__payload
	{
		uint32_t capability_flags;					/*!< Capability flags @see clock_basic_capability_flags */
		uint8_t clock_source;						/*!< Current clock source @see clock_source */
		uint8_t clock_state;						/*!< Current clock state @see clock_state */
		uint8_t servo_state;						/*!< Current servo state @see servo_state */
		uint8_t preferred;							/*!< Current PTP clocking master preferred setting @see clock_preferred */
		uint8_t mute_state;							/*!< Current mute state of audio which is related to the PTP clocking @see clock_mute_state */
		uint8_t ext_wc_state;						/*!< Current external word clock state @see clock_ext_wc_state */
		uint8_t clock_stratum;						/*!< clock stratum */
		uint8_t pad1;								/*!< Pad for alignment - must be '0' */
		uint32_t drift;								/*!< Current clock drift in parts per million (PPM) */
		uint32_t max_drift;							/*!< Maximum clock drift in parts per million (PPM) */
		uint8_t uuid_size;							/*!< Size of the uuid element @see ddp_clock_uuid @see uuid_offset */
		uint8_t master_uuid_size;					/*!< Size of the master uuid element @see ddp_clock_uuid @see master_uuid_offset */
		uint8_t grandmaster_uuid_size;				/*!< Size of the grandmaster uuid element @see ddp_clock_uuid @see grandmaster_uuid_offset */
		uint8_t pad2;								/*!< Pad for alignment - must be '0' */
		ddp_raw_offset_t uuid_offset;				/*!< Offset from the start of the TLV0 header to the uuid of this device @see ddp_clock_uuid */
		ddp_raw_offset_t master_uuid_offset;		/*!< Offset from the start of the TLV0 header to the master uuid @see ddp_clock_uuid */
		ddp_raw_offset_t grandmaster_uuid_offset;	/*!< Offset from the start of the TLV0 header to the grandmaster uuid @see ddp_clock_uuid */
		uint8_t num_ports;							/*!< Number of port structures @see port_array_offset */
		uint8_t port_size;							/*!< Size of each ptp port structure @see ddp_clock_port @see port_array_offset */
		ddp_raw_offset_t port_array_offset;			/*!< Offset from the start of the TLV0 header to the first clock port structure @see ddp_clock_port */
		uint16_t pad3;								/*!< Pad for alignment - must be '0' */
	} payload;	/*!< fixed payload */
	
	// Heap goes here	
};

/**
* @struct ddp_clock_config_port
* @brief Configuration for each clock port
*/
struct ddp_clock_config_port
{
	uint16_t port_id;		/*!< The ID of the port, starts from 1 */
	uint8_t enable_port;	/*!< Port enable setting @see clock_port_enable */
	uint8_t unicast_delay;	/*!< Unicast delay setting. N.B. Please do not use this field */
};

//! @cond Doxygen_Suppress
typedef struct ddp_clock_config_port ddp_clock_config_port_t;
//! @endcond Doxygen_Suppress

/**
* @struct ddp_clock_config_request
* @brief Structure format for the "clock config request" message
*/
struct ddp_clock_config_request
{
	ddp_message_header_t header;				/*!< message header */
	ddp_request_subheader_t subheader;			/*!< request message subheader */

	struct
	{
		uint16_t control_flags;						/*!< Control flags @see clock_config_control_flags */
		uint8_t preferred;							/*!< Preferred PTP clock master setting @see clock_preferred */
		uint8_t ext_word_clock_sync;				/*!< External word clock sync setting @see clock_ext_wc_sync */
		uint8_t logging;							/*!< PTP logging setting @see clock_logging */
		uint8_t multicast;							/*!< Multicast PTP setting @see clock_multicast */
		uint8_t slave_only;							/*!< Slave only setting @see clock_slave_only */
		uint8_t pad0;								/*!< Pad for alignment - must be '0' */
		uint16_t num_clock_port_structures;			/*!< Number of structures that correspond to the number of ports that need to be configured */
		ddp_raw_offset_t clock_port_structs_offset;	/*!< Offset from the start of the TLV0 header to the first clock port structure @see ddp_clock_config_port */
		uint16_t clock_port_structure_size;			/*!< Size of clock_port_params structure */
		uint8_t clock_protocol;						/*!< Bitmap of clock protocols to enable/disable @see clock_protocol, must be bit-wise AND-ed with clock_protocol_mask for validity */
		uint8_t clock_protocol_mask;				/*!< Bitmask of which bits in the clock_protocol are valid @see clock_protocol */
		uint8_t unicast_delay_clock_protocol;		/*!< Bitmap of clock protocols to enable/disable unicast delay requests @see clock_protocol, must be bit-wise AND-ed with unicast_delay_clock_protocol_mask for validity */
		uint8_t unicast_delay_clock_protocol_mask;	/*!< Bitmask of which bits in the unicast_delay_clock_protocol_mask are valid @see unicast_delay_clock_protocol_mask */
		uint16_t pad1;								/*!< Pad for alignment - must be '0' */
	} payload;										/*!< fixed payload */

	// Heap goes here
};

/**
* @struct ddp_clock_config_response
* @brief Structure format for the "clock config response" message
*/
struct ddp_clock_config_response
{
	ddp_message_header_t header;			/*!< message header */
	ddp_response_subheader_t subheader;		/*!< response message subheader */

	//! Clock config data
	struct ddp_clock_config_response__payload
	{
		uint16_t valid_flags;						/*!< Valid flags - which fields in this message is valid @see clock_config_response_valid_flags */
		uint8_t preferred;							/*!< Preferred PTP clock master setting @see clock_preferred, valid only if CLOCK_CONFIG_RSP_PREFERRED_VALID is set in the valid_flags */
		uint8_t word_clock_sync;					/*!< Word clock sync setting @see clock_ext_wc_sync, valid only if CLOCK_CONFIG_RSP_WC_SYNC_VALID is set in the valid_flags */
		uint8_t logging;							/*!< PTP logging setting  @see clock_logging, valid only if CLOCK_CONFIG_RSP_LOGGING_VALID is set in the valid_flags */
		uint8_t multicast;							/*!< Multicast PTP setting @see clock_multicast, valid only if CLOCK_CONFIG_RSP_MULTICAST_VALID is set in the valid_flags */
		uint8_t slave_only;							/*!< Slave only setting @see clock_slave_only, valid only if CLOCK_CONFIG_RSP_SLAVE_ONLY_VALID is set in the valid_flags */
		uint8_t pad0;								/*!< Pad for alignment - must be '0' */
		uint16_t num_clock_port_structures;			/*!< Number of structures that correspond to the number of ports that have been configured */
		ddp_raw_offset_t clock_port_structs_offset;	/*!< Offset from the start of the TLV0 header to the first clock port structure @see clock_port_params */
		uint16_t clock_port_structure_size;			/*!< Size of clock_port_params structure */
		uint8_t clock_protocol;						/*!< Bitmap of clock protocols which are enabled/disabled @see clock_protocol, must be bit-wise AND-ed with clock_protocol_mask for validity, valid only if CLOCK_CONFIG_RSP_CLOCK_PROTOCOL_VALID is set in the valid_flags */
		uint8_t clock_protocol_mask;				/*!< Bitmask of which bits in the clock_protocol are valid @see clock_protocol, valid only if CLOCK_CONFIG_RSP_CLOCK_PROTOCOL_VALID is set in the valid_flags */
		uint8_t unicast_delay_clock_protocol;		/*!< Bitmamp of clock protocols which have unicast delay enabled/disabled @see clock_protocol, must be bit-wise AND-ed with unicast_delay_clock_protocol_mask for validity, valid only if CLOCK_CONFIG_RSP_UNICAST_DELAY_CLOCK_PROTOCOL_VALID */
		uint8_t unicast_delay_clock_protocol_mask;	/*!< Bitmask of which bits in the unicast_delay_clock_protocol_mask are valid @see unicast_delay_clock_protocol_mask */
		uint16_t pad1;								/*!< Pad for alignment - must be '0' */
	} payload;	/*!< fixed payload */
};

/**
* @struct ddp_clock_pullup_request
* @brief Structure format for the "clock pullup request" message
*/
struct ddp_clock_pullup_request
{
	ddp_message_header_t header;		/*!< message header */
	ddp_request_subheader_t subheader;	/*!< request message subheader */

	struct
	{
		uint16_t control_flags;					/*!< Control flags @see clock_pullup_control_flags */
		uint8_t pullup;							/*!< Pullup value which is to be applied @see clock_pullup */
		uint8_t pad1;							/*!< Pad for alignment - must be '0' */
		ddp_raw_offset_t subdomain_offset;		/*!< Offset from the start of the TLV0 header to the first subdomain field */
		uint16_t subdomain_size;				/*!< Size of the subdomain field */
	} payload;	/*!< fixed payload */

	// Heap goes here
};

/**
* @struct ddp_clock_pullup_response
* @brief Structure format for the "clock pullup response" message
*/
struct ddp_clock_pullup_response
{
	ddp_message_header_t header;			/*!< message header */
	ddp_response_subheader_t subheader;		/*!< response message subheader */

	struct
	{
		uint16_t valid_flags;						/*!< Valid flags - which fields in this message is valid @see clock_pullup_valid_flags */
		uint8_t current_pullup;						/*!< Currently used pullup, valid only if CLOCK_PULLUP_CURR_PULLUP_VALID is set in the valid_flags */
		uint8_t reboot_pullup;						/*!< Pullup used after reboot, valid only if CLOCK_PULLUP_REBOOT_PULLUP_VALID is set in the valid_flags */
		ddp_raw_offset_t current_subdomain_offset;	/*!< Offset from the start of the TLV0 header to the first current subdomain field, valid only if CLOCK_PULLUP_CURR_SUBDOMAIN_VALID is set in the valid_flags */
		ddp_raw_offset_t reboot_subdomain_offset;	/*!< Offset from the start of the TLV0 header to the first reboot subdomain field, valid only if CLOCK_PULLUP_REBOOT_SUBDOMAIN_VALID is set in the valid_flags */
		uint16_t current_subdomain_size;			/*!< Size of each current subdomain field, valid only if CLOCK_PULLUP_CURR_SUBDOMAIN_VALID is set in the valid_flags */
		uint16_t reboot_subdomain_size;				/*!< Size of each reboot subdomain field, valid only if CLOCK_PULLUP_REBOOT_SUBDOMAIN_VALID is set in the valid_flags */
		uint16_t num_supported_pullups;				/*!< Number of supported pullups, valid only if CLOCK_PULLUP_SUPPORTED_PULLUPS_VALID is set in the valid_flags */
		uint16_t supported_pullups_size;			/*!< Size of each supported pullup field size, valid only if CLOCK_PULLUP_SUPPORTED_PULLUPS_VALID is set in the valid_flags */
		ddp_raw_offset_t supported_pullups_offset;	/*!< Offset from the start of the TLV0 header to the first supported pullup field */
		uint16_t pad0;								/*!< Pad for alignment - must be '0' */
	} payload;	/*!< fixed payload */

	// Heap goes here
};

//! @cond Doxygen_Suppress
typedef struct ddp_clock_basic_legacy_request ddp_clock_basic_legacy_request_t;
typedef struct ddp_clock_basic_legacy_response ddp_clock_basic_legacy_response_t;
typedef struct ddp_clock_basic2_request ddp_clock_basic2_request_t;
typedef struct ddp_clock_basic2_response ddp_clock_basic2_response_t;
typedef struct ddp_clock_config_request ddp_clock_config_request_t;
typedef struct ddp_clock_config_response ddp_clock_config_response_t;
typedef struct ddp_clock_pullup_request ddp_clock_pullup_request_t;
typedef struct ddp_clock_pullup_response ddp_clock_pullup_response_t;
//! @endcond

/**@}*/
/**@}*/
/**@}*/

#endif // _DDP_CLOCKING_STRUCTURES_H

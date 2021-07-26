/*
 * File     : network_structures.h
 * Created  : May 2014
 * Updated  : Date: 2014/05/16
 * Author   : Michael Ung <michael.ung@audinate.com>
 * Synopsis : All network DDP messages structure definitions.
 *
 * Copyright 2005-2017, Audinate Pty Ltd and/or its licensors
	Confidential
 * Audinate Copyright Header Version 1
 */
#ifndef _DDP_NETWORK_STRUCTURES_H
#define _DDP_NETWORK_STRUCTURES_H

#include "ddp/ddp.h"

#include "ddp/shared_structures.h"

/**
* @file network_structures.h
* @brief All network DDP messages structure definitions.
*/

/** \addtogroup DDP
*  @{
*/

/** \addtogroup Network
*  @{
*/

/**
* \defgroup NetworkEnums DDP Network Enums
* @{
*/

/**
* @enum ddp_network_intf_flags
* @brief Network configuration interface config flag values for ddp_network_intf_config_t
*/
enum ddp_network_intf_flags
{
	NETWORK_INTERFACE_FLAGS_LINK_UP = 0x0001,	/*!< Network interface link is currently up */
	NETWORK_INTERFACE_FLAGS_HALF_DUPLEX = 0x0002,	/*!< Network interface is currently operating in half duplex mode */
};

/**
* @enum network_intf_mode
* @brief Network interface mode values for ddp_network_basic_response_t, ddp_network_config_request_t and ddp_network_config_response_t
*/
enum network_intf_mode
{
	NETWORK_INTERFACE_MODE_LINKLOCAL_ONLY = 1,	/*!< Network interface mode is set to link local only (Zero-conf) */
	NETWORK_INTERFACE_MODE_LINKLOCAL_DHCP = 2,	/*!< Network interface mode is set to link local and dynamic IP */
	NETWORK_INTERFACE_MODE_DHCP_ONLY = 3,	/*!< Network interface mode is set to dynamic IP only */
	NETWORK_INTERFACE_MODE_STATIC = 4,	/*!< Network interface mode is set to static IP */
};

/**
* @enum network_intf_index (expandable)
* @brief Network interface index value for ddp_network_config_request_t and ddp_network_config_response_t
*/
enum network_intf_index
{
	NETWORK_INTERFACE_INDEX_PRIMARY	= 0,	/*!< Network interface index primary or first interface index*/
	NETWORK_INTERFACE_INDEX_SECONDARY = 1	/*!< Network interface index secondary or second interface index*/
};

/**
* @enum ddp_network_basic_response_capability_flags
* @brief Network basic capability_flags values for ddp_network_basic_response_t
*/
enum ddp_network_basic_response_capability_flags
{
	NETWORK_BASIC_CAPABILITY_SUPPORT_STATIC_IP_ON_REBOOT = 0x00000001,	/*!< Setting static IP after a reboot is supported */
	NETWORK_BASIC_CAPABILITY_SUPPORT_STATIC_IP_DYNAMICALLY = 0x00000002,	/*!< Setting static IP dynamically is supported */
	NETWORK_BASIC_CAPABILITY_SUPPORT_SWITCH_VLAN = 0x00000004,	/*!< Switch VLAN config is supported */
	NETWORK_BASIC_CAPABILITY_SUPPORT_SWITCH_REDUNDANCY = 0x00000008,	/*!< Switch redundancy config is supported */
	NETWORK_BASIC_CAPABILITY_CAN_DISABLE_LINKLOCAL = 0x00000010,	/*!< Disabling link local is supported */
	NETWORK_BASIC_CAPABILITY_CAN_DELAY_LINKLOCAL = 0x00000020,	/*!< Delayed link local is supported */
	NETWORK_BASIC_CAPABILITY_CAN_DISABLE_DHCP = 0x00000040,	/*!< Disabling DHCP mode is supported */
};

/**
* @enum network_config_response_valid_flags
* @brief Network configuration valid_flags values for ddp_network_config_response
*/
enum network_config_response_valid_flags
{
	NETWORK_CONFIG_CURRENT_MODE_VALID = 0x00000001,	/*!< current_mode field is valid */
	NETWORK_CONFIG_REBOOT_MODE_VALID = 0x00000002,	/*!< reboot_mode field is valid */
	NETWORK_CONFIG_REBOOT_STATIC_IP_ADDR_VALID = 0x00000004,	/*!< ip_addr field in _reboot_static_ip_config field is valid */
	NETWORK_CONFIG_REBOOT_STATIC_SUBNET_MASK_VALID = 0x00000008,	/*!< subnet_mask field in _reboot_static_ip_config field is valid */
	NETWORK_CONFIG_REBOOT_STATIC_GATEWAY_ADDR_VALID = 0x00000010,	/*!< gateway field in _reboot_static_ip_config field is valid */
	NETWORK_CONFIG_REBOOT_STATIC_DNS_SRVR_VALID = 0x00000020,	/*!< dns_srv_size and dns_srv_offset fields are valid */
	NETWORK_CONFIG_REBOOT_STATIC_DOMAIN_VALID = 0x00000040,	/*!< domain_offset is valid */
};

/**
* @enum network_config_request_control_flags
* @brief Network configuration control_flags values for ddp_network_config_request
*/
enum network_config_request_control_flags
{
	NETWORK_CONFIG_REQ_SET_MODE = 0x0001,	/*!< mode field is valid */
	NETWORK_CONFIG_REQ_SET_STATIC_IP = 0x0002,	/*!< ip_addr field in _ip_config field is valid */
	NETWORK_CONFIG_REQ_SET_STATIC_SUBNET_MASK = 0x0004,	/*!< subnet_mask field in _ip_config field is valid */
	NETWORK_CONFIG_REQ_SET_STATIC_GATEWAY = 0x0008,	/*!< gateway field in _ip_config field is valid */
	NETWORK_CONFIG_REQ_SET_DNS_SERVER = 0x0010,	/*!< dns_srv_size and dns_srv_offset fields are valids */
	NETWORK_CONFIG_REQ_SET_STATIC_DOMAIN = 0x0020,	/*!< domain_offset field is valid */
};

/**
* @enum network_config_response_change_flags
* @brief Network configuration change_flags values for ddp_network_config_response_t
*/
enum network_config_response_change_flags
{
	NETWORK_CONFIG_CHANGE_FLAGS_CHANGE_ON_REBOOT = 0x01,
};

/**@}*/

/**
* \defgroup NetworkMacros DDP Network Macros
* @{
*/

/**
* Size of the hardware MAC address in bytes
*/
#define DDP_NETWORK_ETHERADDR_LEN		6

/**@}*/

/**
* \defgroup NetworkStructs DDP Network Structures
* @{
*/

/**
* @struct network_ip_config_legacy
* @brief Structure format of the IP address information for "network config request" and "network config response" messages
*/
struct network_ip_config_legacy
{
	uint32_t ip_addr;		/*!< IP address */
	uint32_t subnet_mask;	/*!< Subnet mask */
	uint32_t gateway;		/*!< Gateway IP address*/
};

//! @cond Doxygen_Suppress
typedef struct network_ip_config_legacy network_ip_config_legacy_t;
//! @endcond

/**
* @struct ddp_network_addr_config_legacy
* @brief Structure format of the network address information @see network_ip_config_t
* @deprecated Use ddp_network_addr_config_nw_ip instead
*/
struct ddp_network_addr_config_legacy
{
	uint16_t family;						/*!< IP address family, currently only DDP_ADDRESS_FAMILY_INET is supported @see ddp_address_family */
	uint16_t pad0;							/*!< Pad for alignment - must be '0' */
	network_ip_config_legacy_t _ip_config;	/*!< Static IP address information @see network_ip_config_t */
};

//! @cond Doxygen_Suppress
typedef struct ddp_network_addr_config_legacy ddp_network_addr_config_legacy_t;
//! @endcon

/**
* @struct network_ip_config_nw_ip
* @brief Structure format of the IP address information for "network config request" and "network config response" messages
*/
struct network_ip_config_nw_ip
{
	ddp_ip_addr_nw_t ip_addr;		/*!< IP address */
	ddp_ip_addr_nw_t subnet_mask;	/*!< Subnet mask */
	ddp_ip_addr_nw_t gateway;		/*!< Gateway IP address*/
};

//! @cond Doxygen_Suppress
typedef struct network_ip_config_nw_ip network_ip_config_nw_ip_t;
//! @endcond

/**
* @struct ddp_network_addr_config_nw_ip
* @brief Structure format of the network address information @see network_ip_config_t
*/
struct ddp_network_addr_config_nw_ip
{
	uint16_t family;						/*!< IP address family, currently only DDP_ADDRESS_FAMILY_INET is supported @see ddp_address_family */
	uint16_t pad0;							/*!< Pad for alignment - must be '0' */
	network_ip_config_nw_ip_t _ip_config;	/*!< Static IP address information @see network_ip_config_t */
};

//! @cond Doxygen_Suppress
typedef struct ddp_network_addr_config_nw_ip ddp_network_addr_config_nw_ip_t;
//! @endcond

/**
* @struct ddp_network_etheraddr
* @brief Structure format of the ethernet MAC address
*/
struct ddp_network_etheraddr
{
	uint8_t addr[DDP_NETWORK_ETHERADDR_LEN];		/*!< MAC address */
};

//! @cond Doxygen_Suppress
typedef struct ddp_network_etheraddr ddp_network_etheraddr_t;
//! @endcond

/**
* @struct ddp_network_intf_config
* @brief Structure format of the device network interface information for the "network basic response" message
*/
struct ddp_network_intf_config
{
	uint16_t flags;							/*!< Network interface flags @see ddp_network_intf_flags */
	uint16_t mode;							/*!< Current operating mode @see network_intf_mode */
	uint32_t link_speed;					/*!< Network interface link speed */
	ddp_network_etheraddr_t mac_address;	/*!< 6-byte network interface MAC address (layer 2) */
	uint16_t num_addr;						/*!< Number of network interface address structures @see ddp_network_addr_config_t */
	uint16_t addr_size;						/*!< Size of the network interface address structure @see ddp_network_addr_config_t */
	ddp_raw_offset_t adr_offset;			/*!< Offset from the start of the TLV0 header to the first network interface address structure @see ddp_network_addr_config_t */
};

//! @cond Doxygen_Suppress
typedef struct ddp_network_intf_config ddp_network_intf_config_t;
//! @endcond

/**
* @struct ddp_network_intf_in
* @brief Structure used to set the DNS server related information
*/
struct ddp_network_intf_in
{
	ddp_network_intf_config_t _intf_config;		/*!< Information about the network interface of the device @see ddp_network_intf_config  */
	ddp_network_addr_config_legacy_t * paddr_config;	/*!< IP address information of the network interface of the device @see ddp_network_addr_config */
};

//! @cond Doxygen_Suppress
typedef struct ddp_network_intf_in ddp_network_intf_write_t;
//! @endcond

/**
* @struct ddp_network_basic_control
* @brief Structure used to set/indicate information related to the DDP network basic response message
*/
struct ddp_network_basic_control
{
	uint32_t capability_flags;		/*!< Bitwise OR'd capability flags @see ddp_network_basic_response_capability_flags */
	uint16_t num_intfs;				/*!< Number of network interface structures included in the network basic response message @see ddp_network_intf_config */
	uint16_t num_dns_srvrs;			/*!< Number of DNS server structures included in the network basic response message @see network_dns_config */
	uint16_t num_domains;			/*!< Number of domain name strings include in the network basic response message */
};

//! @cond Doxygen_Suppress
typedef struct ddp_network_basic_control network_basic_control_t;
//! @endcond

/**
* @struct ddp_dns_srvr_config
* @brief Structure format of the DNS server configuration information for "network basic response" messages
*/
struct ddp_dns_srvr_config
{
	uint16_t family;	/*!< IP address family, currently only DDP_ADDRESS_FAMILY_INET is supported @see ddp_address_family */
	uint16_t pad;		/*!< Pad for alignment - must be '0' */
	uint32_t ip_addr;	/*!< The IP address of the DNS server */
};

//! @cond Doxygen_Suppress
typedef struct ddp_dns_srvr_config ddp_dns_srvr_config_t;
//! @endcond

/**
* @struct ddp_network_basic_request
* @brief Structure format for the "network basic request" message
*/
struct ddp_network_basic_request
{
	ddp_message_header_t header;			/*!< Message header */
	ddp_request_subheader_t subheader;		/*!< Request message subheader */	
};

/**
* @struct ddp_domain_name_offsets
* @brief Structure format of an element of the array of offsets to domain name strings for the "network basic response" message
*/
struct ddp_domain_name_offsets
{
	ddp_raw_offset_t offset;	/*!< Offset from the start of the TLV0 header to the domain name string */
};

//! @cond Doxygen_Suppress
typedef struct ddp_domain_name_offsets ddp_domain_name_offsets_t;
//! @endcond

/**
* @struct ddp_network_basic_response
* @brief Structure format for the "network basic response" message
*/
struct ddp_network_basic_response
{
	ddp_message_header_t header;			/*!< Message header */
	ddp_response_subheader_t subheader;		/*!< Response message subheader */

	struct
	{
		uint32_t capability_flags;					/*!< Capability flags - which fields in this message is valid @see ddp_network_basic_response_capability_flags */
		uint16_t num_intfs;							/*!< Number of interfaces structures @see ddp_network_intf_config_t in this message */
		uint16_t intf_size;							/*!< Size of each interface structure @see ddp_network_intf_config_t in this message */
		ddp_raw_offset_t intf_offset;				/*!< Offset from the start of the TLV0 header to the first interface structure @see ddp_network_intf_config_t */
		uint16_t num_dns_srvrs;						/*!< Number of DNS servers structures @see ddp_dns_srvr_config_t */
		uint16_t dns_srv_size;						/*!< Size of each DNS server structure @see ddp_dns_srvr_config_t */
		ddp_raw_offset_t dns_srv_offset;			/*!< Offset from the start of the TLV0 header to the first DNS server structure @see ddp_dns_srvr_config_t */
		uint16_t num_domains;						/*!< Number of domain name string offset structures @see ddp_domain_name_offsets_t */
		ddp_raw_offset_t domain_offset_array;		/*!< Offset from the start of the TLV0 header to the first domain name string offset @see ddp_domain_name_offsets_t */
	} payload; /*!< fixed payload */
	
	// Heap goes here
};

/**
* @struct ddp_dns_srvr_config_net_config
* @brief Structure format of the DNS server configuration information for "network config request" messages
*/
struct ddp_dns_srvr_config_net_config
{
	uint32_t ip_addr;	/*!< The IP address of the DNS server */
	uint16_t family;	/*!< IP address family, currently only DDP_ADDRESS_FAMILY_INET is supported @see ddp_address_family */
	uint16_t pad;		/*!< Pad for alignment - must be '0' */
};

//! @cond Doxygen_Suppress
typedef struct ddp_dns_srvr_config_net_config ddp_dns_srvr_config_net_config_t;
//! @endcond

/**
* @struct ddp_network_config_request
* @brief Structure format for the "network config request" message
*/
struct ddp_network_config_request
{
	ddp_message_header_t header;			/*!< Message header */
	ddp_request_subheader_t subheader;		/*!< Request message subheader */
	
	struct
	{
		uint16_t control_flags;					/*!< Control flags for this message @see network_config_request_control_flags */
		uint8_t mode;							/*!< Network configuration mode to be applied on a device, only valid if NETWORK_CONFIG_REQ_SET_MODE is set in the control_flags field @see ddp_network_intf_mode */
		uint8_t itf_index;						/*!< Interface index */
		network_ip_config_nw_ip_t _ip_config;	/*!< Static IP address information to be applied on a device, only valid if NETWORK_CONFIG_REQ_SET_STATIC_IP, NETWORK_CONFIG_REQ_SET_STATIC_SUBNET_MASK, and NETWORK_CONFIG_REQ_SET_STATIC_GATEWAY are set in the control_flags field */
		uint16_t dns_srv_size;					/*!< Size of the DNS server structure @see ddp_dns_srvr_config_t, only valid if NETWORK_CONFIG_REQ_SET_DNS_SERVER is set in the control_flags field */
		uint16_t dns_srv_offset;				/*!< Offset from the start of the TLV0 header to the DNS server structure @see ddp_dns_srvr_config_t, only valid if NETWORK_CONFIG_REQ_SET_DNS_SERVER is set in the control_flags field */
		ddp_raw_offset_t domain_offset;			/*!< Offset from the start of the TLV0 header to the domain name string, only valid if NETWORK_CONFIG_REQ_SET_STATIC_DOMAIN is set in the control_flags field */
		uint16_t pad1;							/*!< Pad for alignment - must be '0' */
	} payload; /*!< fixed payload */
	
	// Heap goes here
};

/**
* @struct ddp_network_config_response
* @brief Structure format for the "network config response" message
*/
struct ddp_network_config_response
{
	ddp_message_header_t header;						/*!< Message header */
	ddp_response_subheader_t subheader;					/*!< Response message subheader */
	
	struct
	{
		uint32_t valid_flags;								/*!< Valid flags - which fields in this message is valid @see network_config_response_valid_flags */
		uint8_t change_flags;								/*!< Change flags @see network_config_response_change_flags */
		uint8_t current_mode;								/*!< Current mode of the device @see network_intf_mode, valid only if NETWORK_CONFIG_CURRENT_MODE_VALID is set in the valid_flags field */
		uint8_t reboot_mode;								/*!< Mode that the device will have after a reboot @see network_intf_mode, valid only if NETWORK_CONFIG_REBOOT_MODE_VALID is set in the valid_flags field */
		uint8_t itf_index;									/*!< Interface index - based on number of interfaces from network basic message 0,1,2 */
		network_ip_config_nw_ip_t _reboot_static_ip_config;	/*!< Static IP address information applied after a reboot @see network_ip_config_t, valid only if NETWORK_CONFIG_REBOOT_STATIC_IP_ADDR_VALID, NETWORK_CONFIG_REQ_SET_STATIC_SUBNET_MASK, or NETWORK_CONFIG_REQ_SET_STATIC_GATEWAY is set in the valid_flags field */
		uint32_t reboot_static_dns_ip_addr;					/*!< Static DNS IP address applied after a reboot, valid only if NETWORK_CONFIG_REBOOT_STATIC_DNS_SRVR_VALID is set in the valid_flags field */
		ddp_raw_offset_t reboot_static_domain_offset;		/*!< Static domain string applied after a reboot, valid only if NETWORK_CONFIG_REBOOT_STATIC_DOMAIN_VALID is set in the valid flags field */
		uint16_t pad1;										/*!< Pad for alignment - must be '0' */
	} payload; /*!< fixed payload */
	
	// Heap goes here
};

//! @cond Doxygen_Suppress
typedef struct ddp_network_basic_response ddp_network_basic_response_t;
typedef struct ddp_network_basic_request ddp_network_basic_request_t;
typedef struct ddp_network_config_request ddp_network_config_request_t;
typedef struct ddp_network_config_response ddp_network_config_response_t;
//! @endcond

/**@}*/
/**@}*/
/**@}*/

#endif // _DDP_NETWORK_STRUCTURES_H

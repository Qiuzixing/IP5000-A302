/*
 * File     : network_client.h
 * Created  : August 2014
 * Updated  : Date: 2014/08/22
 * Author   : Michael Ung <michael.ung@audinate.com>
 * Synopsis : All network DDP client helper functions.
 *
 * Copyright 2005-2017, Audinate Pty Ltd and/or its licensors
	Confidential
 * Audinate Copyright Header Version 1
 */
#ifndef _DDP_NETWORK_CLIENT_H
#define _DDP_NETWORK_CLIENT_H

#include "ddp/packet.h"
#include "ddp/network_structures.h"
#include "ddp/shared_structures.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
* @file network_client.h
* @brief All network DDP client helper functions.
*/

/** \addtogroup DDP
*  @{
*/

/**
* \defgroup Network DDP Network
* @{
*/

/** \addtogroup NetworkStructs
*  @{
*/

/**
* @struct network_dns_config_legacy
* @brief Helper structure used to set the DNS server related information
* @deprecated Use network_dns_config_nw_ip instead
*/
struct network_dns_config_legacy
{
	uint32_t ip_addr;		/*!< IP address of the DNS server */
	uint16_t family;		/*!< Address family of the DNS IP address @see ddp_address_family */
};

//! @cond Doxygen_Suppress
typedef struct network_dns_config_legacy network_dns_config_legacy_t;
//! @endcond

/**
* @struct network_dns_config_nw_ip
* @brief Helper structure used to set the DNS server related information
*/
struct network_dns_config_nw_ip
{
	ddp_ip_addr_nw_t ip_addr;	/*!< IP address of the DNS server in network order */
	uint16_t family;			/*!< Address family of the DNS IP address @see ddp_address_family */
};

//! @cond Doxygen_Suppress
typedef struct network_dns_config_nw_ip network_dns_config_nw_ip_t;
//! @endcond

/**@}*/

/**
* \defgroup NetworkFuncs DDP Network Functions
* @{
*/

/**
* Creates a DDP network basic request message. This message can only query the state of the device.
* @param packet_info [in] Pointer to a structure which has information about the DDP packet buffer
* @param request_id [in] A non-zero sequence number which is populated in this message
* @return AUD_SUCCESS if the DDP network basic request message was successfully created, else a non AUD_SUCCESS value is returned
*/
aud_error_t
ddp_add_network_basic_request
(
	ddp_packet_write_info_t * packet_info,
	ddp_request_id_t request_id
);

/**
* Reads fixed data fields from a DDP network basic response message
* @param packet_info [in] Pointer to a structure which has information about the DDP packet buffer
* @param offset [in] Starting point in the packet buffer to read this message
* @param out_message_info [in out] Pointer to a structure which contains information about the payload and heap for this DDP packet (memory should be allocated for this structure by the caller of this function)
* @param out_request_id [out optional] Pointer to the sequence number of this message
* @param out_status [out optional] Pointer to the status of this message
* @param out_basic_control [in out optional] Pointer to a structure which has information associated with the fields of the DDP network basic response message @see network_basic_control
* @return AUD_SUCCESS if the DDP network basic response message was successfully read, else a non AUD_SUCCESS value is returned
*/
aud_error_t
ddp_read_network_basic_response_header
(
	const ddp_packet_read_info_t * packet_info,
	uint16_t offset,
	ddp_message_read_info_t * out_message_info,
	ddp_request_id_t * out_request_id,
	ddp_status_t * out_status,
	network_basic_control_t * out_basic_control
);

/**
* Reads the network interface related data fields from a DDP network basic response message for a particular network interface. N.B. This function should only be called after calling the ddp_read_network_basic_response_header function @see ddp_read_network_basic_response_header
* @param message_info [in] Pointer to a structure which has information about the DDP packet buffer, use the out_message_info parameter after calling the ddp_read_network_basic_response_header function
* @param interface_idx [in] Index into the network interface blocks of information, values range from 0 to number of interfaces - 1
* @param out_flags [out optional] Pointer to the network interface flags @see ddp_network_intf_flags
* @param out_mode [out optional] Pointer to the network interface mode @see network_intf_mode
* @param out_link_speed [out optional] Pointer to the network interface link speed 
* @param out_mac_address [out optional] Pointer to the 6-byte MAC (physical) address
* @param out_num_addresses [out optional] Pointer to the number of addresses assigned to this network interface
* @return AUD_SUCCESS if the data fields of a particular network interface for the DDP network basic response message was successfully read, else a non AUD_SUCCESS value is returned
*/
aud_error_t
ddp_read_network_basic_response_interface
(
	const ddp_message_read_info_t * message_info,
	uint16_t interface_idx,
	uint16_t * out_flags,
	uint16_t * out_mode,
	uint32_t * out_link_speed,
	ddp_network_etheraddr_t * out_mac_address,
	uint16_t * out_num_addresses
);

/**
* Reads the address related data fields from a DDP network basic response message for a particular network interface and address. N.B. This function should only be called after calling the ddp_read_network_basic_response_header function @see ddp_read_network_basic_response_header
* @param message_info [in] Pointer to a structure which has information about the DDP packet buffer, use the out_message_info parameter after the calling ddp_read_network_basic_response_header function
* @param interface_idx [in] Index into the network interface blocks of information, values range from 0 to number of interfaces - 1
* @param address_idx [in] Index into the address blocks of information for the network interface indexed by interface_idx, values range from 0 to number of addresses - 1
* @param out_family [out optional] Pointer to the IP address family @see ddp_address_family
* @param out_ip_address [out optional] Pointer to the IP address
* @param out_netmask [out optional] Pointer to the netmask
* @param out_gateway [out optional] Pointer to the gateway
* @return AUD_SUCCESS if the data fields of a particular network interface and address for the DDP network basic response message was successfully read, else a non AUD_SUCCESS value is returned
* @deprecated Use \ref ddp_read_network_basic_response_interface_address_nw_ip instead
*/
aud_error_t
ddp_read_network_basic_response_interface_address_legacy
(
	const ddp_message_read_info_t * message_info,
	uint16_t interface_idx,
	uint16_t address_idx,
	uint16_t * out_family,
	uint32_t * out_ip_address,
	uint32_t * out_netmask,
	uint32_t * out_gateway
);

/**
* Reads the address related data fields from a DDP network basic response message for a particular network interface and address. N.B. This function should only be called after calling the ddp_read_network_basic_response_header function @see ddp_read_network_basic_response_header
* @param message_info [in] Pointer to a structure which has information about the DDP packet buffer, use the out_message_info parameter after the calling ddp_read_network_basic_response_header function
* @param interface_idx [in] Index into the network interface blocks of information, values range from 0 to number of interfaces - 1
* @param address_idx [in] Index into the address blocks of information for the network interface indexed by interface_idx, values range from 0 to number of addresses - 1
* @param out_family [out optional] Pointer to the IP address family @see ddp_address_family
* @param out_ip_address [out optional] Pointer to the IP address in network byte order
* @param out_netmask [out optional] Pointer to the netmask in network byte order
* @param out_gateway [out optional] Pointer to the gateway in network byte order
* @return AUD_SUCCESS if the data fields of a particular network interface and address for the DDP network basic response message was successfully read, else a non AUD_SUCCESS value is returned
*/
aud_error_t
ddp_read_network_basic_response_interface_address_nw_ip
(
	const ddp_message_read_info_t * message_info,
	uint16_t interface_idx,
	uint16_t address_idx,
	uint16_t * out_family,
	ddp_ip_addr_nw_t * out_ip_address,
	ddp_ip_addr_nw_t * out_netmask,
	ddp_ip_addr_nw_t * out_gateway
	);

/**
* Reads the DNS server related data fields from a DDP network basic response message. N.B. This function should only be called after calling the ddp_read_network_basic_response_header function @see ddp_read_network_basic_response_header
* @param message_info [in] Pointer to a structure which has information about the DDP packet buffer, use the out_message_info parameter after calling the ddp_read_network_basic_response_header function
* @param dns_idx [in] Index into the collection of DNS server address blocks of information indexed by dns_idx, values range from 0 to number of addresses - 1
* @param out_family [out optional] Pointer to the DNS server IP address family @see ddp_address_family
* @param out_ip_address [out optional] Pointer to the DNS server IP address
* @return AUD_SUCCESS if the data fields of a particular network interface and address for the DDP network basic response message was successfully read, else a non AUD_SUCCESS value is returned
* @deprecated Use \ref ddp_read_network_basic_response_dns_nw_ip instead
*/
aud_error_t
ddp_read_network_basic_response_dns_legacy
(
	const ddp_message_read_info_t * message_info,
	uint16_t dns_idx,
	uint16_t * out_family,
	uint32_t * out_ip_address
);

/**
* Reads the DNS server related data fields from a DDP network basic response message. N.B. This function should only be called after calling the ddp_read_network_basic_response_header function @see ddp_read_network_basic_response_header
* @param message_info [in] Pointer to a structure which has information about the DDP packet buffer, use the out_message_info parameter after calling the ddp_read_network_basic_response_header function
* @param dns_idx [in] Index into the collection of DNS server address blocks of information indexed by dns_idx, values range from 0 to number of addresses - 1
* @param out_family [out optional] Pointer to the DNS server IP address family @see ddp_address_family
* @param out_ip_address [out optional] Pointer to the DNS server IP address in network byte order
* @return AUD_SUCCESS if the data fields of a particular network interface and address for the DDP network basic response message was successfully read, else a non AUD_SUCCESS value is returned
*/
aud_error_t
ddp_read_network_basic_response_dns_nw_ip
(
	const ddp_message_read_info_t * message_info,
	uint16_t dns_idx,
	uint16_t * out_family,
	ddp_ip_addr_nw_t * out_ip_address
);

/**
* Reads a domain name data field from a DDP network basic response message. N.B. This function should only be called after calling the ddp_read_network_basic_response_header function @see ddp_read_network_basic_response_header
* @param message_info [in] Pointer to a structure which has information about the DDP packet buffer, use the out_message_info parameter after calling the ddp_read_network_basic_response_header function
* @param domain_idx [in] Index into the collection of domain names, values range from 0 to number of domains - 1
* @param out_domain [out optional] Pointer to a pointer to the null terminated domain name string
* @return AUD_SUCCESS if the domain name data field for the DDP network basic response message was successfully read, else a non AUD_SUCCESS value is returned
*/
aud_error_t
ddp_read_network_basic_response_domain
(
	const ddp_message_read_info_t * message_info,
	uint16_t domain_idx,
	char ** out_domain
);

/**
* Creates a DDP network config request message. This message can be used as a command to change the state of the device and as a request to query the current state of the device.
* @param packet_info [in] Pointer to a structure which has information about the DDP packet buffer
* @param request_id [in] A non-zero sequence number which is populated in this message
* @param intf_index [in] The network interface index number to that needs to be applied on the device (0,1 ... )
* @param mode [in] The network interface mode to that needs to be applied on the device @see network_intf_mode, use zero when querying the current mode
* @param ip_config [in optional] Pointer to a structure that contains information about the network interface of the device @see ddp_network_intf_config, use NULL when querying the current network interface information
* @param dns_config [in optional] Pointer to the DNS server configuration structure @see network_dns_config_legacy_t, use NULL when querying the current DNS server information
* @param domain_name [in optional] Pointer to a domain name string that needs to be applied on the device, use NULL when querying the current domain name
* @return AUD_SUCCESS if the DDP network config request message was successfully created, else a non AUD_SUCCESS value is returned
* @deprecated Use \ref ddp_add_network_config_request_nw_ip instead
*/
aud_error_t
ddp_add_network_config_request_legacy
(
	ddp_packet_write_info_t * packet_info,
	ddp_request_id_t request_id,
	uint8_t intf_index,
	uint8_t mode,
	const network_ip_config_legacy_t * ip_config,
	const network_dns_config_legacy_t * dns_config,
	const char * domain_name
);

/**
* Creates a DDP network config request message. This message can be used as a command to change the state of the device and as a request to query the current state of the device.
* @param packet_info [in] Pointer to a structure which has information about the DDP packet buffer
* @param request_id [in] A non-zero sequence number which is populated in this message
* @param intf_index [in] The network interface index number to that needs to be applied on the device (0,1 ... )
* @param mode [in] The network interface mode to that needs to be applied on the device @see network_intf_mode, use zero when querying the current mode
* @param ip_config [in optional] Pointer to a structure that contains information about the network interface of the device @see ddp_network_intf_config, use NULL when querying the current network interface information
* @param dns_config [in optional] Pointer to the DNS server configuration structure @see network_dns_config_nw_ip_t, use NULL when querying the current DNS server information
* @param domain_name [in optional] Pointer to a domain name string that needs to be applied on the device, use NULL when querying the current domain name
* @return AUD_SUCCESS if the DDP network config request message was successfully created, else a non AUD_SUCCESS value is returned
*/
aud_error_t
ddp_add_network_config_request_nw_ip
(
	ddp_packet_write_info_t * packet_info,
	ddp_request_id_t request_id,
	uint8_t intf_index,
	uint8_t mode,
	const network_ip_config_nw_ip_t * ip_config,
	const network_dns_config_nw_ip_t * dns_config,
	const char * domain_name
);


/**
* Reads data fields from a DDP network config response message
* @param packet_info [in] Pointer to a structure which has information about the DDP packet buffer
* @param offset [in] Starting point in the packet buffer to read this message
* @param out_request_id [out optional] Pointer to the sequence number of this message
* @param out_intf_index [out optional] Pointer to the interface index number of this message
* @param out_status [out optional] Pointer to the status of this message
* @param out_change_flags [out optional] Pointer to the bitwise OR'd change flags of this response @see network_config_response_change_flags 
* @param out_current_mode [out optional] Pointer to the current network interface mode of the device @see network_intf_mode
* @param out_reboot_mode [out optional] Pointer to the network interface mode used after the device reboots @see network_intf_mode
* @param out_ip_config [in out optional] Pointer to a structure that contains information about the network interface of the device @see ddp_network_intf_config
* @param out_dns_config [in out optional] Pointer to the DNS server configuration structure @see network_dns_config_t
* @param out_domain_name [out optional] Pointer to a pointer to the null terminated domain name string of the device 
* @return AUD_SUCCESS if the DDP network config response message was successfully read, else a non AUD_SUCCESS value is returned
* @deprecated Use \ref ddp_read_network_config_response_nw_ip instead
*/
aud_error_t
ddp_read_network_config_response_legacy
(
	const ddp_packet_read_info_t * packet_info,
	uint16_t offset,
	ddp_request_id_t * out_request_id,
	uint8_t * out_intf_index,
	ddp_status_t * out_status,
	uint8_t * out_change_flags,
	uint8_t * out_current_mode,
	uint8_t * out_reboot_mode,
	network_ip_config_legacy_t * out_ip_config,
	network_dns_config_legacy_t * out_dns_config,
	char ** out_domain_name
);

/**
* Reads data fields from a DDP network config response message
* @param packet_info [in] Pointer to a structure which has information about the DDP packet buffer
* @param offset [in] Starting point in the packet buffer to read this message
* @param out_request_id [out optional] Pointer to the sequence number of this message
* @param out_intf_index [out optional] Pointer to the interface index number of this message
* @param out_status [out optional] Pointer to the status of this message
* @param out_change_flags [out optional] Pointer to the bitwise OR'd change flags of this response @see network_config_response_change_flags
* @param out_current_mode [out optional] Pointer to the current network interface mode of the device @see network_intf_mode
* @param out_reboot_mode [out optional] Pointer to the network interface mode used after the device reboots @see network_intf_mode
* @param out_ip_config [in out optional] Pointer to a structure that contains information about the network interface of the device @see ddp_network_intf_config
* @param out_dns_config [in out optional] Pointer to the DNS server configuration structure @see network_dns_config_t
* @param out_domain_name [out optional] Pointer to a pointer to the null terminated domain name string of the device
* @return AUD_SUCCESS if the DDP network config response message was successfully read, else a non AUD_SUCCESS value is returned
*/
aud_error_t
ddp_read_network_config_response_nw_ip
(
	const ddp_packet_read_info_t * packet_info,
	uint16_t offset,
	ddp_request_id_t * out_request_id,
	uint8_t * out_intf_index,
	ddp_status_t * out_status,
	uint8_t * out_change_flags,
	uint8_t * out_current_mode,
	uint8_t * out_reboot_mode,
	network_ip_config_nw_ip_t * out_ip_config,
	network_dns_config_nw_ip_t * out_dns_config,
	char ** out_domain_name
);

/**@}*/
/**@}*/
/**@}*/

#ifdef __cplusplus
}
#endif

#endif // _DDP_NETWORK_CLIENT_H

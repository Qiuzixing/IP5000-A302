/*
* File     : ddp_network_read_client.c
* Created  : August 2014
* Updated  : Date: 2014/08/22
* Author   : Michael Ung <michael.ung@audinate.com>
* Synopsis : All network DDP client read helper functions.
*
* Copyright 2005-2017, Audinate Pty Ltd and/or its licensors
Confidential
* Audinate Copyright Header Version 1
*/

#include "ddp/network_client.h"

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
)
{
	aud_error_t result;
	network_ip_config_nw_ip_t ip_config_nw_ip;
	network_dns_config_nw_ip_t dns_config_nw_ip;

	result = ddp_read_network_config_response_nw_ip(packet_info,
													offset,
													out_request_id,
													out_intf_index,
													out_status,
													out_change_flags,
													out_current_mode,
													out_reboot_mode,
													&ip_config_nw_ip,
													&dns_config_nw_ip,
													out_domain_name);

	out_ip_config->ip_addr = ntohl(ip_config_nw_ip.ip_addr.ip_addr);
	out_ip_config->gateway = ntohl(ip_config_nw_ip.gateway.ip_addr);
	out_ip_config->subnet_mask = ntohl(ip_config_nw_ip.gateway.ip_addr);

	out_dns_config->family = dns_config_nw_ip.family;
	out_dns_config->ip_addr = ntohl(dns_config_nw_ip.ip_addr.ip_addr);

	return result;
}

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
)
{
	aud_error_t result;
	ddp_message_read_info_t message_info;
	ddp_network_config_response_t * header;
	uint32_t valid_flags;

	if (!packet_info || offset >= packet_info->packet_length_bytes)
	{
		return AUD_ERR_INVALIDPARAMETER;
	}

	result = ddp_packet_read_message(packet_info, &message_info, offset);
	if (result != AUD_SUCCESS)
	{
		return result;
	}

	result = ddp_packet_validate_message(&message_info, DDP_OP_NETWORK_CONFIG, sizeof(ddp_network_config_response_t));
	if (result != AUD_SUCCESS)
	{
		return result;
	}

	header = (ddp_network_config_response_t *)message_info.header.header;

	if (out_request_id)
	{
		*out_request_id = ntohs(header->subheader.request_id);
	}

	if (out_status)
	{
		*out_status = ntohs(header->subheader.status);
	}

	if (out_intf_index)
	{
		*out_intf_index = header->payload.itf_index;
	}

	valid_flags = ntohl(header->payload.valid_flags);

	if (out_change_flags)
	{
		*out_change_flags = header->payload.change_flags;
	}
	if (out_current_mode && AUD_FLAG_ISSET(valid_flags, NETWORK_CONFIG_CURRENT_MODE_VALID))
	{
		*out_current_mode = header->payload.current_mode;
	}
	if (out_reboot_mode && AUD_FLAG_ISSET(valid_flags, NETWORK_CONFIG_REBOOT_MODE_VALID))
	{
		*out_reboot_mode = header->payload.reboot_mode;
	}

	if (out_ip_config)
	{
		if (AUD_FLAG_ISSET(valid_flags, NETWORK_CONFIG_REBOOT_STATIC_IP_ADDR_VALID))
		{
			out_ip_config->ip_addr = header->payload._reboot_static_ip_config.ip_addr;
		}
		if (AUD_FLAG_ISSET(valid_flags, NETWORK_CONFIG_REBOOT_STATIC_SUBNET_MASK_VALID))
		{
			out_ip_config->subnet_mask = header->payload._reboot_static_ip_config.subnet_mask;
		}
		if (AUD_FLAG_ISSET(valid_flags, NETWORK_CONFIG_REBOOT_STATIC_GATEWAY_ADDR_VALID))
		{
			out_ip_config->gateway = header->payload._reboot_static_ip_config.gateway;
		}

	}
	if (out_dns_config)
	{
		if (AUD_FLAG_ISSET(valid_flags, NETWORK_CONFIG_REBOOT_STATIC_DNS_SRVR_VALID))
		{
			out_dns_config->ip_addr.ip_addr = header->payload.reboot_static_dns_ip_addr;
			out_dns_config->family = DDP_ADDRESS_FAMILY_INET;	//assume IPv4
		}

	}
	if (AUD_FLAG_ISSET(valid_flags, NETWORK_CONFIG_REBOOT_STATIC_DOMAIN_VALID) &&
		ntohs(header->payload.reboot_static_domain_offset))
	{
		*out_domain_name = (char*)header + ntohs(header->payload.reboot_static_domain_offset);
	}
	else
	{
		*out_domain_name = NULL;
	}
	return AUD_SUCCESS;
}

aud_error_t
ddp_read_network_basic_response_header
(
	const ddp_packet_read_info_t * packet_info,
	uint16_t offset,
	ddp_message_read_info_t * out_message_info,
	ddp_request_id_t * out_request_id,
	ddp_status_t * out_status,
	network_basic_control_t * out_basic_control
)
{
	aud_error_t result;
	ddp_network_basic_response_t * header;

	if (!packet_info || offset >= packet_info->packet_length_bytes)
	{
		return AUD_ERR_INVALIDPARAMETER;
	}

	result = ddp_packet_read_message(packet_info, out_message_info, offset);
	if (result != AUD_SUCCESS)
	{
		return result;
	}

	result = ddp_packet_validate_message(out_message_info, DDP_OP_NETWORK_BASIC, sizeof(ddp_network_basic_response_t));
	if (result != AUD_SUCCESS)
	{
		return result;
	}

	header = (ddp_network_basic_response_t *)out_message_info->header.header;

	if (out_request_id)
	{
		*out_request_id = ntohs(header->subheader.request_id);
	}
	if (out_status)
	{
		*out_status = ntohs(header->subheader.status);
	}

	if (out_basic_control)
	{
		out_basic_control->capability_flags = ntohl(header->payload.capability_flags);
		out_basic_control->num_intfs = ntohs(header->payload.num_intfs);
		out_basic_control->num_dns_srvrs = ntohs(header->payload.num_dns_srvrs);
		out_basic_control->num_domains = ntohs(header->payload.num_domains);
	}

	return AUD_SUCCESS;
}

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
)
{
	aud_error_t result;
	ddp_network_basic_response_t *header;
	ddp_heap_read_info_t heap_info;
	ddp_network_intf_config_t *intf_config;

	if (!message_info)
	{
		return AUD_ERR_INVALIDPARAMETER;
	}

	header = (ddp_network_basic_response_t *)message_info->header.header;

	if (interface_idx >= ntohs(header->payload.num_intfs))
	{
		return AUD_ERR_INVALIDPARAMETER;
	}

	result = ddp_read_heap_arraystruct(message_info,
									&heap_info,
									ntohs(header->payload.intf_offset),
									ntohs(header->payload.num_intfs),
									ntohs(header->payload.intf_size));

	if (result != AUD_SUCCESS)
	{
		return result;
	}

	intf_config = ((ddp_network_intf_config_t*)heap_info._.array32) + interface_idx;

	if (out_flags)
	{
		*out_flags = ntohs(intf_config->flags);
	}
	if (out_mode)
	{
		*out_mode = ntohs(intf_config->mode);
	}
	if (out_link_speed)
	{
		*out_link_speed = ntohl(intf_config->link_speed);
	}
	if (out_mac_address)
	{
		memcpy(out_mac_address, &intf_config->mac_address, sizeof(intf_config->mac_address));
	}
	if (out_num_addresses)
	{
		*out_num_addresses = ntohs(intf_config->num_addr);
	}

	return AUD_SUCCESS;
}

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
)
{
	aud_error_t result;
	ddp_ip_addr_nw_t ip_addr_nw, netmask_nw, gateway_nw;

	result = ddp_read_network_basic_response_interface_address_nw_ip(message_info,
																	interface_idx,
																	address_idx,
																	out_family,
																	&ip_addr_nw,
																	&netmask_nw,
																	&gateway_nw);

	*out_ip_address = ntohl(ip_addr_nw.ip_addr);
	*out_netmask = ntohl(netmask_nw.ip_addr);
	*out_gateway = ntohl(gateway_nw.ip_addr);

	return result;
}

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
)
{
	aud_error_t result;
	ddp_network_basic_response_t *header;
	ddp_heap_read_info_t heap_info;
	ddp_network_intf_config_t *intf_config;
	ddp_network_addr_config_legacy_t *addr_config;

	if (!message_info)
	{
		return AUD_ERR_INVALIDPARAMETER;
	}

	header = (ddp_network_basic_response_t *)message_info->header.header;

	if (interface_idx >= ntohs(header->payload.num_intfs))
	{
		return AUD_ERR_INVALIDPARAMETER;
	}

	result = ddp_read_heap_arraystruct(message_info,
									&heap_info,
									ntohs(header->payload.intf_offset),
									ntohs(header->payload.num_intfs),
									ntohs(header->payload.intf_size));

	if (result != AUD_SUCCESS)
	{
		return result;
	}

	intf_config = ((ddp_network_intf_config_t*)heap_info._.array32) + interface_idx;

	result = ddp_read_heap_arraystruct(message_info,
									&heap_info,
									ntohs(intf_config->adr_offset),
									ntohs(intf_config->num_addr),
									ntohs(intf_config->addr_size));

	if (result != AUD_SUCCESS)
	{
		return result;
	}

	addr_config = ((ddp_network_addr_config_legacy_t*)heap_info._.array32) + address_idx;

	if (out_family)
	{
		*out_family = ntohs(addr_config->family);
	}
	if (out_ip_address)
	{
		out_ip_address->ip_addr = addr_config->_ip_config.ip_addr;
	}
	if (out_gateway)
	{
		out_gateway->ip_addr = addr_config->_ip_config.gateway;
	}
	if (out_netmask)
	{
		out_netmask->ip_addr = addr_config->_ip_config.subnet_mask;
	}

	return AUD_SUCCESS;
}

aud_error_t
ddp_read_network_basic_response_dns_legacy
(
	const ddp_message_read_info_t * message_info,
	uint16_t dns_idx,
	uint16_t * out_family,
	uint32_t * out_ip_address
)
{
	aud_error_t result;
	ddp_ip_addr_nw_t ip_addr_nw;

	result = ddp_read_network_basic_response_dns_nw_ip(message_info, dns_idx, out_family, &ip_addr_nw);
	*out_ip_address = ntohl(ip_addr_nw.ip_addr);

	return result;
}

aud_error_t
ddp_read_network_basic_response_dns_nw_ip
(
	const ddp_message_read_info_t * message_info,
	uint16_t dns_idx,
	uint16_t * out_family,
	ddp_ip_addr_nw_t * out_ip_address
)
{
	aud_error_t result;
	ddp_network_basic_response_t *header;
	ddp_heap_read_info_t heap_info;
	ddp_dns_srvr_config_t *dns_config;
	
	if (!message_info)
	{
		return AUD_ERR_INVALIDPARAMETER;
	}

	header = (ddp_network_basic_response_t *)message_info->header.header;

	if (dns_idx >= ntohs(header->payload.num_dns_srvrs))
	{
		return AUD_ERR_INVALIDPARAMETER;
	}

	if (out_family && out_ip_address)
	{
		result = ddp_read_heap_arraystruct(message_info,
										&heap_info,
										ntohs(header->payload.dns_srv_offset),
										ntohs(header->payload.num_dns_srvrs),
										ntohs(header->payload.dns_srv_size));
		
		if (result != AUD_SUCCESS)
		{
			return result;
		}

		dns_config = ((ddp_dns_srvr_config_t*)heap_info._.array32) + dns_idx;
		*out_family = ntohs(dns_config->family);
		out_ip_address->ip_addr = dns_config->ip_addr;
	}

	return AUD_SUCCESS;
}

aud_error_t
ddp_read_network_basic_response_domain
(
	const ddp_message_read_info_t * message_info,
	uint16_t domain_idx,
	char ** out_domain
)
{
	aud_error_t result;
	ddp_network_basic_response_t *header;
	ddp_heap_read_info_t heap_info;
	ddp_domain_name_offsets_t *offsets;
	
	if (!message_info)
	{
		return AUD_ERR_INVALIDPARAMETER;
	}

	header = (ddp_network_basic_response_t *)message_info->header.header;

	if (domain_idx >= ntohs(header->payload.num_domains))
	{
		return AUD_ERR_INVALIDPARAMETER;
	}

	if (out_domain)
	{
		result = ddp_read_heap_arraystruct(message_info,
										&heap_info,
										ntohs(header->payload.domain_offset_array),
										ntohs(header->payload.num_domains),
										sizeof(ddp_domain_name_offsets_t));

		if (result != AUD_SUCCESS)
		{
			return result;
		}

		offsets = ((ddp_domain_name_offsets_t*)heap_info._.array16) + domain_idx;
		if (offsets->offset)
		{
			*out_domain = ddp_heap_read_string(message_info, ntohs(offsets->offset));
		}
		else
		{
			*out_domain = NULL;
		}

	}

	return  AUD_SUCCESS;
}




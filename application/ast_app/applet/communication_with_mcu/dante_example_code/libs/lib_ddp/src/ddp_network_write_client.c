/*
* File     : ddp_network_write_client.c
* Created  : August 2014
* Updated  : Date: 2014/08/22
* Author   : Michael Ung <michael.ung@audinate.com>
* Synopsis : All network DDP client write helper functions.
*
* Copyright 2005-2017, Audinate Pty Ltd and/or its licensors
Confidential
* Audinate Copyright Header Version 1
*/

#include "ddp/network_client.h"

aud_error_t
ddp_add_network_basic_request
(
		ddp_packet_write_info_t * packet_info,
		ddp_request_id_t request_id
)
{
	aud_error_t result;
	ddp_message_write_info_t message_info;

	if (!packet_info)
	{
		return AUD_ERR_INVALIDPARAMETER;
	}

	result = ddp_packet_allocate_request(packet_info, &message_info,
		DDP_OP_NETWORK_BASIC, request_id,
		0);
	if (result != AUD_SUCCESS)
	{
		return result;
	}

	return AUD_SUCCESS;
}


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
)
{
	aud_error_t result;
	network_ip_config_nw_ip_t ip_config_nw;
	network_dns_config_nw_ip_t dns_config_nw;

	ip_config_nw.ip_addr.ip_addr = htonl(ip_config->ip_addr);
	ip_config_nw.gateway.ip_addr = htonl(ip_config->gateway);
	ip_config_nw.subnet_mask.ip_addr = htonl(ip_config->subnet_mask);

	dns_config_nw.family = dns_config->family;
	dns_config_nw.ip_addr.ip_addr = htonl(dns_config->ip_addr);

	result = ddp_add_network_config_request_nw_ip(packet_info, request_id, intf_index, mode, &ip_config_nw, &dns_config_nw, domain_name);

	return result;
}

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
	)
{
	aud_error_t result;
	ddp_message_write_info_t message_info;
	ddp_network_config_request_t * raw_header;
	uint16_t control_flags = 0;

	result = ddp_packet_allocate_request(packet_info, &message_info,
			DDP_OP_NETWORK_CONFIG, request_id, sizeof(raw_header->payload));
	if (result != AUD_SUCCESS)
	{
		return result;
	}

	raw_header = (ddp_network_config_request_t *) message_info._.buf32;

	if(mode)
	{
		raw_header->payload.mode = mode;
		control_flags |= NETWORK_CONFIG_REQ_SET_MODE;
	}
	raw_header->payload.itf_index = intf_index;
	if(ip_config)
	{
		if(ip_config->ip_addr.ip_addr)
		{
			raw_header->payload._ip_config.ip_addr.ip_addr = ip_config->ip_addr.ip_addr;
			control_flags |= NETWORK_CONFIG_REQ_SET_STATIC_IP;
		}
		if(ip_config->subnet_mask.ip_addr)
		{
			raw_header->payload._ip_config.subnet_mask.ip_addr = ip_config->subnet_mask.ip_addr;
			control_flags |= NETWORK_CONFIG_REQ_SET_STATIC_SUBNET_MASK;
		}
		if(ip_config->gateway.ip_addr)
		{
			raw_header->payload._ip_config.gateway.ip_addr = ip_config->gateway.ip_addr;
			control_flags |= NETWORK_CONFIG_REQ_SET_STATIC_GATEWAY;
		}
	}
	else
	{
		raw_header->payload._ip_config.ip_addr.ip_addr = 0;
		raw_header->payload._ip_config.subnet_mask.ip_addr = 0;
		raw_header->payload._ip_config.gateway.ip_addr = 0;
	}

	if(dns_config)
	{
		//allocate onto the heap
		ddp_dns_srvr_config_net_config_t* dns_conf_heap;
		ddp_heap_write_info_t heap_info;
		result = ddp_allocate_heap_arraystruct(&message_info, &heap_info,
						1, sizeof(ddp_dns_srvr_config_net_config_t));
		if (result != AUD_SUCCESS)
		{
			return result;
		}

		raw_header->payload.dns_srv_offset = heap_info.msg_offset_bytes;
		raw_header->payload.dns_srv_size = htons(sizeof(ddp_dns_srvr_config_net_config_t));

		dns_conf_heap = (ddp_dns_srvr_config_net_config_t*)heap_info._.array8;

		if (dns_config->ip_addr.ip_addr && dns_config->family)
		{
			dns_conf_heap->ip_addr = dns_config->ip_addr.ip_addr;
			dns_conf_heap->family = htons(dns_config->family);
			dns_conf_heap->pad = 0;
			control_flags |= NETWORK_CONFIG_REQ_SET_DNS_SERVER;
		}
		else
		{
			return AUD_ERR_INVALIDPARAMETER;
		}
	}
	else
	{
		raw_header->payload.dns_srv_offset = 0;
		raw_header->payload.dns_srv_size = 0;
	}

	raw_header->payload.pad1 = 0;

	if(domain_name)
	{
		if (!
			ddp_message_allocate_string(
				&message_info,
				&raw_header->payload.domain_offset,
				domain_name
		))
		{
			return AUD_ERR_NOMEMORY;
		}
		control_flags |= NETWORK_CONFIG_REQ_SET_STATIC_DOMAIN;
	}
	else
	{
		raw_header->payload.domain_offset = 0;
	}
	raw_header->payload.control_flags = htons(control_flags);
	return AUD_SUCCESS;
}


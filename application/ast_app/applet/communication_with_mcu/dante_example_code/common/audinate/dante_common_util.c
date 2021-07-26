/*
 * File     : $RCSfile$
 * Created  : January 2007
 * Updated  : $Date$
 * Author   : James Westendorp <james.westendorp@audinate.com>
 * Synopsis : dante comon util implementation
 *
 *  * This software is copyright (c) 2004-2017 Audinate Pty Ltd and/or its licensors
 *
 *
 * Audinate Copyright Header Version 1
 *
 */

#include <string.h>
#include <ctype.h>
#include "dante/dante_common.h"

 
char *
dante_device_id_to_string
(
	const dante_device_id_t * id,
	dante_device_id_str_t str
) {
	sprintf(str, 
			"%02x%02x%02x%02x%02x%02x%02x%02x",
			id->data[0], id->data[1], 
			id->data[2], id->data[3], 
			id->data[4], id->data[5], 
			id->data[6], id->data[7]);
	return str;
}


char *
dante_instance_id_to_string
(
	const dante_instance_id_t * id,
	dante_instance_id_str_t str
) {
	sprintf(str,
			"%02x%02x%02x%02x%02x%02x%02x%02x:%d",
			id->device_id.data[0], id->device_id.data[1], 
			id->device_id.data[2], id->device_id.data[3], 
			id->device_id.data[4], id->device_id.data[5], 
			id->device_id.data[6], id->device_id.data[7], 
			id->process_id);
	return str;
}

AUD_INLINE int
aud_nibble_to_hex(int nibble)
{
	if (nibble <= 9)
		return '0' + nibble;
	else
		return 'a' + (nibble - 10);
}

static int is_dnssd_text(int ch)
{
	return isalnum(ch) || ch == '-' || ch == '_';
}

static char *
dante_id64_to_dnssd_text_body
(
	const dante_id64_t * id64,
	char * buf,
	aud_bool_t hex_only
)
{
	unsigned i, j, len;
	uint8_t ch;

	// drop trailling zero
	len = DANTE_ID64_LEN;
	for (;;)
	{
		ch = id64->data[len - 1];
		if (ch)
		{
			break;
		}
		len--;
		if (len == 0)
		{
			buf[0] = '_';
			buf[1] = '0';
			buf[2] = '0';
			return buf;
		}
	}

	if (hex_only)
		goto l__hex;

	// try to process as string
	ch = id64->data[0];
	if (ch == '_' || !is_dnssd_text(ch))
		goto l__hex;
	buf[0] = ch;
	j = 1;
	for (i = 1; i < len; i++)
	{
		ch = id64->data[i];
		if (is_dnssd_text(ch))
		{
			buf[j++] = ch;
		}
		else
			goto l__hex;
	}
	buf[j] = 0;
	return buf;

l__hex:
	buf[0] = '_';
	j = 1;
	for (i = 0; i < len; i++)
	{
		ch = id64->data[i];
		buf[j++] = (char)aud_nibble_to_hex(ch >> 4);
		buf[j++] = (char)aud_nibble_to_hex(ch & 0xf);
	}
	buf[j] = 0;
	return buf;
}


char *
dante_id64_to_dnssd_text
(
	const dante_id64_t * id64,
	char * buf
)
{
	return dante_id64_to_dnssd_text_body(id64, buf, AUD_FALSE);
}

char *
dante_id64_to_dnssd_hex
(
	const dante_id64_t * id64,
	char * buf
)
{
	return dante_id64_to_dnssd_text_body(id64, buf, AUD_TRUE);
}

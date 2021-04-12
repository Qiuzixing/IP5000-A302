/*
 * Copyright (c) 2004-2012
 * ASPEED Technology Inc. All Rights Reserved
 * Proprietary and Confidential
 *
 * By using this code you acknowledge that you have signed and accepted
 * the terms of the ASPEED SDK license agreement.
 */

#ifndef _I2S_EVENT_H
#define _I2S_EVENT_H

#define EVENT_AUDIO_SETTINGS_CHANGED 0x00000000
//#define EVENT_CLIENT_DISCONNECTED 0x00000001
#define EVENT_CODEC_SETTINGS_CHANGED 0x00000002
#define EVENT_HDMI_SETTINGS_CHANGED 0x00000003

typedef struct _event_struct
{
	struct list_head	list;
	u32	event;
	void *pdata;
	struct socket *pfrom; //NULL means from host itself. Used by host.
}event_struct, *pevent_struct;

int add_event(u32 event, void *pdata, struct socket *pfrom);

#endif

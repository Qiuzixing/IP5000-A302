/*
 * File     : aud_platform.h
 * Created  : September 2014
 * Updated  : Date: 2014/09/30
 * Author   : Michael Ung <michael.ung@audinate.com>
 * Synopsis : UHIP HostCPU platform definitions for Windows
 *
 * Copyright 2005-2017, Audinate Pty Ltd and/or its licensors
 * Confidential
 * Audinate Copyright Header Version 1
 */

#ifndef __AUD_PLATFORM_H__
#define __AUD_PLATFORM_H__

/**
 * Copy and modify this file to support your platform
**/

#define AUD_PLATFORM "HostCPU"

/* Definition required to compile DDP message helpers for messages processed only by an Ultimo */
#define AUD_PLATFORM_ULTIMO 1

#ifdef _WIN32
	#pragma comment(lib, "Ws2_32.lib")
	#include <winsock2.h> // For ntohs, ntohl, htons, and htonl
	#define AUD_INLINE __inline
#else
	/** set inline for compiler */
	#define AUD_INLINE static inline
#endif //_WIN32 

//aud environment sizes
enum
{
	AUD_ENV_ERROR_STRLEN = 128,
	AUD_ENV_SOCKADDR_ADDRSTRLEN = 16
};

#include <string.h> //strings
#include <stdlib.h> //NULL, etc
#include <stdio.h>	//for printf
#include <stddef.h> //ptrdiff_t, size_t, offsetof etc
#include <stdint.h> //uint8_t/int8_t/uint16_t/int16_t/etc integers
#include <audinate/platform_types.h>		//audinate platform types
#include <audinate/platform_error.h>		//audinate error types
#include <audinate/platform_string_util.h>	//audinate string macro's
#include <audinate/platform_flags_util.h>	//audinate flag macro's
#include <audinate/strcasecmp.h>			//audinate string comparison

//define bzero
#ifndef bzero
#define bzero(a, b) memset(a, 0, b)
#endif

//define assert
#define assert(X)
/*
#define assert(cond) if(!(cond)) \
		(printf("assertion failed: %s, file %s,line %d \n",#cond,\
		__FILE__,__LINE__))
*/

//define AUD_PRINTF
#define AUD_PRINTF(...)	printf(__VA_ARGS__)
//#define AUD_PRINTF(...) NULL
//enable / disable support for specific UHIP messages
#define HOSTCPU_API_SUPPORT_PACKETBRIDGE_CONMON						1	//support ConMon packet bridge messages and callbacks
#define HOSTCPU_API_SUPPORT_PACKETBRIDGE_UDP						1	//support UDP packet bridge messages and callbacks

#define HOSTCPU_API_SUPPORT_DANTE_EVENT_AUDIO_FORMAT					1	//support Dante event audio format messages and callbacks
#define HOSTCPU_API_SUPPORT_DANTE_EVENT_CLOCK_PULLUP					1	//support Dante event clock pullup messages and callbacks

#define HOSTCPU_API_SUPPORT_DDP_DEVICE_GENERAL						1	//support the DDP device general messages and callbacks
#define HOSTCPU_API_SUPPORT_DDP_DEVICE_MANF_INFO					1	//support the DDP device manufacturer information messages and callbacks
#define HOSTCPU_API_SUPPORT_DDP_DEVICE_UPGRADE						1	//support the DDP device upgrade messages and callbacks
#define HOSTCPU_API_SUPPORT_DDP_DEVICE_ERASE_CONFIG					1	//support the DDP device erase configuration messages and callbacks
#define HOSTCPU_API_SUPPORT_DDP_DEVICE_REBOOT						1	//support the DDP device reboot messages and callbacks
#define HOSTCPU_API_SUPPORT_DDP_DEVICE_ID						1	//support the DDP device ID messages and callbacks
#define HOSTCPU_API_SUPPORT_DDP_DEVICE_IDENTIFY						1	//support the DDP device identify messages and callbacks
#define HOSTCPU_API_SUPPORT_DDP_DEVICE_GPIO						1	//support the DDP device gpio messages and callbacks
#define HOSTCPU_API_SUPPORT_DDP_DEVICE_SWITCH_LED					1	//support the DDP device switch LED messages and callbacks
#define HOSTCPU_API_SUPPORT_DDP_DEVICE_LOCK_UNLOCK					1	//support the DDP device lock unlock messages and callbacks
#define HOSTCPU_API_SUPPORT_DDP_DEVICE_VLAN_CONFIG					1	//support the DDP device VLAN config messages and callbacks
#define HOSTCPU_API_SUPPORT_DDP_DEVICE_DANTE_DOMAIN					1	//support the DDP device Dante Domain messages and callbacks
#define HOSTCPU_API_SUPPORT_DDP_DEVICE_SWITCH_STATUS				1	//support the DDP device switch status messages and callbacks
#define HOSTCPU_API_SUPPORT_DDP_DEVICE_AES67						1	//support the DDP device AES67 messages and callbacks - Only supported on UltimoX platoform

#define HOSTCPU_API_SUPPORT_DDP_NETWORK_BASIC						1	//support the DDP network basic messages and callbacks
#define HOSTCPU_API_SUPPORT_DDP_NETWORK_CONFIG						1	//support the DDP network config messages and callbacks

#define HOSTCPU_API_SUPPORT_DDP_CLOCK_BASIC_LEGACY					1	//support the DDP clock basic legacy messages and callbacks
#define HOSTCPU_API_SUPPORT_DDP_CLOCK_CONFIG						1	//support the DDP clock config messages and callback
#define HOSTCPU_API_SUPPORT_DDP_CLOCK_PULLUP						1	//support the DDP clock pullup messages and callback
#define HOSTCPU_API_SUPPORT_DDP_CLOCK_BASIC2						1	//support the DDP clock basic 2 messages and callbacks

#define HOSTCPU_API_SUPPORT_DDP_AUDIO_BASIC						1	//support the DDP audio basic messages and callbacks
#define HOSTCPU_API_SUPPORT_DDP_AUDIO_SRATE_CONFIG					1	//support the DDP audio srate config messages and callback
#define HOSTCPU_API_SUPPORT_DDP_AUDIO_ENC_CONFIG					1	//support the DDP audio encoding messages and callback
#define HOSTCPU_API_SUPPORT_DDP_AUDIO_SIGNAL_PRESENCE					1	//support the DDP audio signal presence config and audio signal presence data messages and callbacks

#define HOSTCPU_API_SUPPORT_DDP_ROUTING_BASIC						1	//support the DDP routing basic messages and callbacks
#define HOSTCPU_API_SUPPORT_DDP_ROUTING_READY_STATE					1	//support the DDP routing ready state messages and callback
#define HOSTCPU_API_SUPPORT_DDP_ROUTING_PERF_CONFIG					1	//support the DDP routing performance config messages and callback
#define HOSTCPU_API_SUPPORT_DDP_ROUTING_RX_CHAN_CONFIG_STATE				1	//support the DDP routing RX channel config state messages and callback
#define HOSTCPU_API_SUPPORT_DDP_ROUTING_RX_SUBSCRIBE_SET				1	//support the DDP routing RX subscribe set messages and callback
#define HOSTCPU_API_SUPPORT_DDP_ROUTING_RX_CHAN_LABEL_SET				1	//support the DDP routing RX channel label set messages and callback
#define HOSTCPU_API_SUPPORT_DDP_ROUTING_TX_CHAN_CONFIG_STATE				1	//support the DDP routing TX channel config state messages and callback
#define HOSTCPU_API_SUPPORT_DDP_ROUTING_TX_CHAN_LABEL_SET				1	//support the DDP routing TX channel label set messages and callback
#define HOSTCPU_API_SUPPORT_DDP_ROUTING_RX_FLOW_CONFIG_STATE				1	//support the DDP routing RX flow config state messages and callback
#define HOSTCPU_API_SUPPORT_DDP_ROUTING_MCAST_TX_FLOW_CONFIG_SET			1	//support the DDP routing multicast TX flow config set messages and callback
#define HOSTCPU_API_SUPPORT_DDP_ROUTING_TX_FLOW_CONFIG_STATE				1	//support the DDP routing TX flow config state messages and callback
#define HOSTCPU_API_SUPPORT_DDP_ROUTING_RX_CHAN_STATUS					1	//support the DDP routing RX channel status messages and callback
#define HOSTCPU_API_SUPPORT_DDP_ROUTING_RX_FLOW_STATUS					1	//support the DDP routing RX flow status messages and callback
#define HOSTCPU_API_SUPPORT_DDP_ROUTING_RX_UNSUB_CHAN					1	//support the DDP routing RX un-subscribe channel messages and callback
#define HOSTCPU_API_SUPPORT_DDP_ROUTING_FLOW_DELETE					1	//support the DDP routing flow delete messages and callback

/*
 * Include platform common header file, BK2/Broadway BHIP, Ultimo
 */
#include "example_ddp_common.h"

#endif // __AUD_PLATFORM_H__


/*
 * File     : local_client.h
 * Created  : August 2014
 * Updated  : Date: 2014/08/22
 * Author   : Michael Ung <michael.ung@audinate.com>
 * Synopsis : All local DDP client helper functions.
 *
 * Copyright 2005-2017, Audinate Pty Ltd and/or its licensors
	Confidential
 * Audinate Copyright Header Version 1
 */
#ifndef _DDP_LOCAL_CLIENT_H
#define _DDP_LOCAL_CLIENT_H

#include "ddp/packet.h"
#include "ddp/local_structures.h"

#ifdef __cplusplus
extern "C" {
#endif

/** \addtogroup Ultimo
*  @{
*/

/**
* \defgroup DanteEvents Dante Events
* @{
*/

/**
* \defgroup DanteEventsFuncs Dante Events Functions
* @{
*/

#if defined(AUD_PLATFORM_ULTIMO)

/**
* Reads data fields from a DDP local audio format event message
* @param packet_info [out] Pointer to a structure which has information about the DDP packet buffer
* @param offset [in] Starting point in the packet buffer to read this message
* @param out_timestamp [out optional] Pointer to the timestamp when this event occurred
* @param out_current_samplerate [out optional] Pointer to the current samplerate
* @param out_reboot_samplerate [out optional] Pointer to the reboot samplerate
* @return AUD_SUCCESS if the DDP local audio format message was successfully read, else a non AUD_SUCCESS value is returned
* @note Only supported on the Ultimo platform
*/
aud_error_t
ddp_read_local_audio_format
(
	const ddp_packet_read_info_t * packet_info,
	uint16_t offset,
	ddp_event_timestamp_t ** out_timestamp,
	dante_samplerate_t * out_current_samplerate,
	dante_samplerate_t * out_reboot_samplerate
);


/**
* Reads data fields from a DDP local clock pullup event message
* @param packet_info [out] Pointer to a structure which has information about the DDP packet buffer
* @param offset [in] Starting point in the packet buffer to read this message
* @param out_timestamp [out optional] Pointer to the timestamp when this event occurred
* @param out_current_pullup [out optional] Pointer to the current pullup @see clock_pullup
* @param out_reboot_pullup [out optional] Pointer to the reboot pullup @see clock_pullup
* @return AUD_SUCCESS if the DDP local clock pullup message was successfully read, else a non AUD_SUCCESS value is returned
* @note Only supported on the Ultimo platform
*/
aud_error_t
ddp_read_local_clock_pullup
(
	const ddp_packet_read_info_t * packet_info,
	uint16_t offset,
	ddp_event_timestamp_t ** out_timestamp,
	uint16_t * out_current_pullup,
	uint16_t * out_reboot_pullup
);

#endif // defined(AUD_PLATFORM_ULTIMO)

/**@}*/
/**@}*/
/**@}*/

#ifdef __cplusplus
}
#endif

#endif //_DDP_LOCAL_CLIENT_H

/*
* File     : frame.h
* Created  : October 2014
* Updated  : 2014/10/30
* Author   : Chamira Perera
* Synopsis : Implementation of utility functions to handle Ultimo serial frames for UHIP
*
* Copyright 2005-2017, Audinate Pty Ltd and/or its licensors
Confidential
* Audinate Copyright Header Version 1
*/

#ifndef __FRAME_H__
#define __FRAME_H__

#include "aud_platform.h"

/**
* \defgroup Ultimo Ultimo
* @{
*/

/** \addtogroup UHIP
*  @{
*/

/**
* \defgroup UltimoSerial Transport Helpers
* @{
*/

//! @cond Doxygen_Suppress

/**
* \typedef timer_start_t
* Timer function definition used for extract_cobs_from_rx_frame @see extract_cobs_from_rx_frame
*/
typedef void(*timer_start_t)(int);

/**
* \typedef timer_stop_t
* Timer function definition used for extract_cobs_from_rx_frame @see extract_cobs_from_rx_frame
*/
typedef void(*timer_stop_t)(void);

//! @endcond Doxygen_Suppress

/**
* \defgroup SerialFuncs Transport Helper Functions
* @{
*/

/**
* Adds the delimiter and pad bytes to a COBS encoded data packet
* @param tx_buffer [in] Pointer to the COBS encoded message
* @param cobs_encoded_size [in] Size of the COBS encoded message
* @return The total size of the prepared frame which includes the COBS delimiters and pad bytes
* @note Should only be used with the Ultimo platform
*/
size_t prepare_tx_frame(uint8_t *tx_buffer, size_t cobs_encoded_size);

/**
* Extracts the COBS encoded data within a frame iteratively. The function should
* be called for all of the received data. 
* @param current_byte [in] the current received serial data byte
* @param out_buffer [in out] pointer to the buffer which contains the COBS data
* @param num_extracted [out] pointer to the amount of data extracted
* @param rx_timer_start [in optional] function pointer to the function to start the data receive timer
* @param rx_timer_stop [in optional] function pointer to the function to stop the data receive timer
* @param timeout [in optional] timeout value passed into the rx_timer_start function
* @return AUD_SUCCESS if the COBS encoded packet was successfully extracted, AUD_ERR_INPROGRESS if more data is requried to extract a complete packet, else an error in the extraction
* @note Should only be used with the Ultimo platform
*/
aud_error_t extract_cobs_from_rx_frame(uint8_t current_byte, uint8_t *out_buffer, size_t *num_extracted, timer_start_t rx_timer_start, timer_stop_t rx_timer_stop, int timeout);

/**@}*/

/**@}*/
/**@}*/
/**@}*/

#endif // __FRAME_H__

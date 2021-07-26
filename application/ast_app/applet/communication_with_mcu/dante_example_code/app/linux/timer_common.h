/*
* File     : timer_common.h
* Created  : October 2014
* Updated  : 2014/10/08
* Author   : Chamira Perera
* Synopsis : Timer definitions for Windows
*
* Copyright 2005-2017, Audinate Pty Ltd and/or its licensors
* Confidential
* Audinate Copyright Header Version 1
*/

#ifndef __TIMER_COMMON_H__
#define __TIMER_COMMON_H__

#include "aud_platform.h"

/**
* Initialises the HOST CPU TX timer
* @param timer [in] Pointer to the timer data structure
*/
typedef struct timer_data
{
	aud_bool_t has_fired;			/*!< Flag to indicate whether the timer has fired */
	aud_bool_t is_running;			/*!< Flag to indicate whether the timer is running */
	uint64_t timeout;				/*!< The timeout value in milli seconds */
	uint64_t start_ticks;			/*!< The number of ticks of the system clock when the timer was started */
} timer_data_t;

/**
* Initialises the HOST CPU TX timer
* @param timer [in] Pointer to the timer data structure
*/
void timer_common_init(timer_data_t *timer);

/**
* Starts the HOST CPU (one shot) timer
* @param timer [in] Pointer to the timer data structure 
* @param num_milliseconds [in] The number of milliseconds till the timer expires
*/
void timer_common_start(timer_data_t *timer, int num_milliseconds);

/**
* Stops the HOST CPU timer
* @param timer [in] Pointer to the timer data structure
*/
void timer_common_stop(timer_data_t *timer);

/**
* Checks whether the HOST CPU timer is running
* @param timer [in] Pointer to the timer data structure
* @return AUD_TRUE if the timer is running, otherwise AUD_FALSE
*/
aud_bool_t timer_common_is_running(timer_data_t *timer);

/**
* Checks whether the HOST CPU timer has expired / fired
* @param timer [in] Pointer to the timer data structure
* @return AUD_TRUE if the timer has expired/fired, otherwise AUD_FALSE
*/
aud_bool_t timer_common_has_fired(timer_data_t *timer);

#endif // __TIMER_COMMON_H__

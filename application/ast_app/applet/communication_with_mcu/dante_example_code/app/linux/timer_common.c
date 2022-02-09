/*
* File     : timer_common.c
* Created  : October 2014
* Updated  : 2014/10/08
* Author   : Chamira Perera
* Synopsis : HostCPU UHIP common timers for Windows
*
* Copyright 2005-2017, Audinate Pty Ltd and/or its licensors
* Confidential
* Audinate Copyright Header Version 1
*/

#include <time.h>
#include "timer_common.h"

static uint64_t GetTickCount()
{
	struct timespec tp;
    clock_gettime(CLOCK_MONOTONIC, &tp);
	return tp.tv_sec * 1000 + tp.tv_nsec / 1000 / 1000;
}

void timer_common_init(timer_data_t *timer)
{
	memset(timer, 0, sizeof(*timer));
	timer->has_fired = AUD_FALSE;
	timer->is_running = AUD_FALSE;
}

void timer_common_start(timer_data_t *timer, int num_milliseconds)
{
	if (!num_milliseconds || num_milliseconds < 0) {
		return;
	}

	timer->start_ticks = GetTickCount();
	timer->timeout = num_milliseconds;
	timer->is_running = AUD_TRUE;
	timer->has_fired = AUD_FALSE;
}

void timer_common_stop(timer_data_t *timer)
{
	timer->is_running = AUD_FALSE;
	timer->has_fired = AUD_FALSE;
}

aud_bool_t timer_common_is_running(timer_data_t *timer)
{
	return timer->is_running;
}

aud_bool_t timer_common_has_fired(timer_data_t *timer)
{
	uint64_t curr_ticks = GetTickCount();

	if (timer->is_running)
	{
		if ((curr_ticks - timer->start_ticks) >= timer->timeout) 
		{
			timer->has_fired = AUD_TRUE;
			timer->is_running = AUD_FALSE;
		}
	}

	return timer->has_fired;
}

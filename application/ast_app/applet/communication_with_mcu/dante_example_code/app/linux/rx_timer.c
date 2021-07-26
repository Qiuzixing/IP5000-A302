/*
* File     : rx_timer.c
* Created  : October 2014
* Updated  : 2014/10/08
* Author   : Chamira Perera
* Synopsis : HostCPU UHIP Rx timer implementation for Windows
*
* Copyright 2005-2017, Audinate Pty Ltd and/or its licensors
* Confidential
* Audinate Copyright Header Version 1
*/

#include "uhip_hostcpu_rx_timer.h"
#include "timer_common.h"

static timer_data_t rx_timer;

void uhip_hostcpu_rx_timer_init()
{
	timer_common_init(&rx_timer);
}

void uhip_hostcpu_rx_timer_start(int num_milliseconds)
{
	timer_common_start(&rx_timer, num_milliseconds);
}

void uhip_hostcpu_rx_timer_stop()
{
	timer_common_stop(&rx_timer);
}

aud_bool_t uhip_hostcpu_rx_timer_is_running()
{
	return timer_common_is_running(&rx_timer);
}

aud_bool_t uhip_hostcpu_rx_timer_has_fired()
{
	return timer_common_has_fired(&rx_timer);
}

/*
* File     : tx_timer.c
* Created  : October 2014
* Updated  : 2014/10/08
* Author   : Chamira Perera
* Synopsis : HostCPU UHIP Tx timer implementation for Windows
*
* Copyright 2005-2017, Audinate Pty Ltd and/or its licensors
* Confidential
* Audinate Copyright Header Version 1
*/

#include "uhip_hostcpu_tx_timer.h"
#include "timer_common.h"

static timer_data_t tx_timer;

void uhip_hostcpu_tx_timer_init()
{
	timer_common_init(&tx_timer);
}

void uhip_hostcpu_tx_timer_start(int num_milliseconds)
{
	timer_common_start(&tx_timer, num_milliseconds);
}

void uhip_hostcpu_tx_timer_stop()
{
	timer_common_stop(&tx_timer);
}

aud_bool_t uhip_hostcpu_tx_timer_is_running()
{
	return timer_common_is_running(&tx_timer);
}

aud_bool_t uhip_hostcpu_tx_timer_has_fired()
{
	return timer_common_has_fired(&tx_timer);
}

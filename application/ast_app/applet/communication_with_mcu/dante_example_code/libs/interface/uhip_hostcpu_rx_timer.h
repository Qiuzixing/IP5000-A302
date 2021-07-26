/*
 * File     : uhip_hostcpu_rx_timer.h
 * Created  : October 2014
 * Updated  : 2014/10/02
 * Author   : Michael Ung
 * Synopsis : UHIP HostCPU RX Timer Interface
 *
 * This is a polled one-shot timer, that is used for the UHIP protocol RX timeouts.
 * A high timer resolution is not necessary, as long the timer is fairly accurate (+/- 100ms)
 *
 * The implementation of this interface must be provided on the HostCPU platform by the customer.
 *
 * The RX timer is used to recover if COBS synchronisation is every lost on the receive path on the
 * host CPU. 
 * It is used as follows:
 * a) It is started when we receive the initial 0x00 at the start of the COBS encoded UHIP packet
 * b) It is stopped when a full COBS encoded packet is received by the HostCPU. 
 * c) It is reset / restarted every  time a "32-byte chunk" is received. 
 * d) If the timer has fired/expired this means that we have lost COBS synchronisation, and should
 *	  reset our RX buffer
 *
 * Copyright 2005-2017, Audinate Pty Ltd and/or its licensors
	Confidential
 * Audinate Copyright Header Version 1
 */
#ifndef __UHIP_HOSTCPU_RX_TIMER_H__
#define __UHIP_HOSTCPU_RX_TIMER_H__

#include "aud_platform.h"

 /** \addtogroup Ultimo
 *  @{
 */

 /** \addtogroup UHIP
 *  @{
 */

 /** \addtogroup Timer
 *  @{
 */

 /** \addtogroup TimerMacros
 *  @{
 */

/**
* Timeout for receiving another 32-byte chunk whilst receiving a UHIP COBS packet
*/
#define RX_TIMEOUT_MILLISECONDS		1000

/**@}*/

/** \addtogroup TimerFuncs
*  @{
*/

/**
 * Initialises the UHIP HOST CPU RX timer
 */
void uhip_hostcpu_rx_timer_init(void);

/**
 * Starts the UHIP HOST CPU RX (one shot) timer
 * @param num_milliseconds [in] The number of milliseconds till the timer expires
 */
void uhip_hostcpu_rx_timer_start(int num_milliseconds);

/**
 * Stops the UHIP HOST CPU RX timer
 */
void uhip_hostcpu_rx_timer_stop(void);

/**
 * Checks whether the UHIP HOST CPU RX timer is running
 * @return AUD_TRUE if the timer is running, otherwise AUD_FALSE
 */
aud_bool_t uhip_hostcpu_rx_timer_is_running(void);

/**
 * Checks whether the UHIP HOST CPU RX timer has expired / fired
 * @return AUD_TRUE if the timer has expired/fired, otherwise AUD_FALSE
 */
aud_bool_t uhip_hostcpu_rx_timer_has_fired(void);

/**@}*/

/**@}*/
/**@}*/
/**@}*/

#endif // __UHIP_HOSTCPU_RX_TIMER_H__

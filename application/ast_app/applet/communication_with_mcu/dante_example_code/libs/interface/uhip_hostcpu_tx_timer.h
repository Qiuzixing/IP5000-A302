/*
 * File     : uhip_hostcpu_tx_timer.h
 * Created  : October 2014
 * Updated  : 2014/10/02
 * Author   : Michael Ung
 * Synopsis : UHIP HostCPU TX Timer Interface
 *
 * This is a polled one-shot timer, that is used for the UHIP protocol TX timeouts.
 * A high timer resolution is not necessary, as long the timer is fairly accurate (+/- 100ms)
 *
 * The implementation of this interface must be provided on the HostCPU platform by the customer.
 *
 * The TX timer is used for two purposes. 
 * 1) To ensure that only a single UHIP data transaction is pending at any time.
 *    When used for this purpose it is controlled as follows:
 *    a) It is started when the HostCPU transmits a non protocl control packet (i.e. a UHIP data packet)
 *    b) It is stopped when a ACK / NACK is received.
 *    c) If the timer has fired/expired this means that we may have lost COBS synchronisation. At this point
 *	     the TX timer the used for the following purpose
 * 
 * 2) To handle COBS resynchronsiation with a receiver.
 *    When used for this purpose it is controlled as follows:
 *    a) Start the TX timer when an ACK/NACK is not received, and the the timer expires (1c)
 *	  b) Until the timer fires/expires do not send any more packets to the Ultimo from the Host CPU. This allows
 *		 the Ultimo time to reset its' RX buffers, and regain COBS synchronisation
 *
 * Copyright 2005-2017, Audinate Pty Ltd and/or its licensors
	Confidential
 * Audinate Copyright Header Version 1
 */
#ifndef __UHIP_HOSTCPU_TX_TIMER_H__
#define __UHIP_HOSTCPU_TX_TIMER_H__

#include "aud_platform.h"

 /** \addtogroup Ultimo
 *  @{
 */

 /** \addtogroup UHIP
 *  @{
 */

 /**
 * \defgroup Timer Timer
 * @{
 */

 /**
 * \defgroup TimerMacros Timer Macros
 * @{
 */

/**
 * Timeout when waiting for an ACK/NACK
 */
#define TX_ACK_TIMEOUT_MILLISECONDS			1000

/**
* Timeout after an ACK timeout failure, before sending another packet
*/
#define TX_RESYNC_STOP_TIMEOUT_MILLISECONDS		1000

/**@}*/

/**
* \defgroup TimerFuncs Timer Functions
* @{
*/

/**
 * Initialises the UHIP HOST CPU TX timer
 */
void uhip_hostcpu_tx_timer_init(void);

/**
 * Starts the UHIP HOST CPU TX (one shot) timer
 * @param num_milliseconds [in] The number of milliseconds till the timer expires
 */
void uhip_hostcpu_tx_timer_start(int num_milliseconds);

/**
 * Stops the UHIP HOST CPU TX timer
 */
void uhip_hostcpu_tx_timer_stop(void);

/**
 * Checks whether the UHIP HOST CPU TX timer is running
 * @return AUD_TRUE if the timer is running, otherwise AUD_FALSE
 */
aud_bool_t uhip_hostcpu_tx_timer_is_running(void);

/**
 * Checks whether the UHIP HOST CPU TX timer has expired / fired
 * @return AUD_TRUE if the timer has expired/fired, otherwise AUD_FALSE
 */
aud_bool_t uhip_hostcpu_tx_timer_has_fired(void);

/**@}*/

/**@}*/
/**@}*/
/**@}*/

#endif // __UHIP_HOSTCPU_TX_TIMER_H__

/*
 * File     : hostcpu_transport.h
 * Created  : October 2014
 * Updated  : 2014/10/02
 * Author   : Michael Ung
 * Synopsis : HostCPU Transport (physical) Interface
 *
 * This is an interface into the SPI / UART peripheral on the Host CPU
 *
 * The implementation of this interface must be provided on the HostCPU platform by the customer.
 *
 * Copyright 2005-2017, Audinate Pty Ltd and/or its licensors
 * Confidential
 * Audinate Copyright Header Version 1
 */
#ifndef __HOSTCPU_TRANSPORT_H__
#define __HOSTCPU_TRANSPORT_H__

#include <aud_platform.h>

 /*! \mainpage Audinate Host CPU API Documenation
 *
 * This documentation is the API reference for the core functionality provided by the Audinate Host CPU SDK. This reference can be used for
 * interfacing with a Brooklyn-II / Broadway or an Ultimo over a SPI or UART serial interface. Please consult the Host CPU SDK Programmer's Guide
 * Documentation before using this reference. In addtion, If you are interfacing with an Ultimo device refer to the 
 * Ultimo Programmer's Guide. For a Brooklyn-II / Broadway please refer to the Brooklyn-II Programmer's Guide / Broadway Programmer's Guide..
 *
 * The best way to get started is by navigating through the <a href='modules.html'>Modules</a> section. All modules necessary
 * to implement software for a Host CPU to interface with a Brooklyn-II / Broadway or an Ultimo using a SPI or UART serial interface 
 * is contained within the Modules section. The examples code in the src/app/example directory in the Host CPU SDK illustrate 
 * how each module is used for the Brooklyn-II / Broadway and Ultimo platforms.
 *
 * Following a brief summary of each module:
 * <ul>
 *	<li><a href=group___d_d_p.html>DDP</a>: Dante Device Protocol packet definitions and routines to compose and read DDP packets</li>
 *	<li><a href=group___b_k2.html>Brooklyn-II / Broadway</a>: Functionality specific to Host CPUs interfacing with the Brooklyn-II / Broadway platform</li>
 *	<ul>
 *		<li><a href=group___b_h_i_p_p_b.html>Packet Bridge</a>: Packet Bridge packet definitions and routines to compose and read Packet Bridge packets</li>
 *		<li><a href=group___b_h_i_p.html>BHIP Helpers</a>: Functionality specific to the Brooklyn-II / Broadway Host Interface Protocol </li>
 *		<ul>
 *			<li><a href=group___b_h_i_p_pkt_helpers.html>Packet Helpers</a>: BHIP packet definitions and routines to compose and read BHIP packets</li>
 *			<li><a href=group___b_k2_serial.html>Transport Helpers</a>: Routines to prepare a BHIP packet to send over a serial interface to the Brooklyn-II / Broadway or read data from a serial interface connected to a Brooklyn-II / Broadway and extract a BHIP packet</li>
 *		</ul>
 *	</ul>
 *	<li><a href=group___ultimo.html>Ultimo</a>: Functionality specific to Host CPUs intefacing with the Ultimo platform</li>
 *	<ul>
 *		<li><a href=group___dante_events.html>Dante Events</a>: Dante Event packet definitions and routies to compose and read Dante Event packets</li>
 *		<li><a href=group___u_h_i_p_p_b.html>Packet Bridge</a>: Packet Bridge packet definitions and routines to compose and read Packet Bridge packets</li>
 *		<li><a href=group___u_h_i_p.html>UHIP Helpers</a>: Functionality specific to the Ultimo Host Interface Protocol</li>
 *		<ul>
 *			<li><a href=group___timer.html>Timer</a>: Routies to initialise a timer, and start, stop, and query the state of a timer</li>
 *			<li><a href=group___u_h_i_p_pkt_helpers.html>Packet Helpers</a>: UHIP packet definitions and routines to compose and read UHIP packets</li>
 *			<li><a href=group___ultimo_serial.html>Transport Helpers</a>: Routines to prepare a UHIP packet to send over a serial interface to the Ultimo or read data from a serial interface connected to a Ultimo and extract a UHIP packet</li>
 *		</ul>
 *	</ul>
 *	<li><a href=group___transport.html>Transport</a>: Routines to initialise a serial interface (SPI/UART), and read and write from the serial interface</li>
 * </ul>
 *
 */


 /** \addtogroup Transport
 *  @{
 */

 /**
 * \defgroup TransportFuncs Transport Functions
 * @{
 */

/**
 * Initialises the HostCPU transport peripheral
 * @return AUD_TRUE if the init was successful, otherwise AUD_FALSE
 */
aud_bool_t hostcpu_transport_init(void);

/**
 * Read bytes from the transport interface
 * @param buffer [in] Pointer to the buffer to read into
 * @param max_bytes_to_read [in] The maximum number of bytes to read
 * @return the number of bytes read
 */
size_t hostcpu_transport_read(uint8_t* buffer, size_t max_bytes_to_read);

/**
 * Write bytes to the transport interface
 * @param buffer [in] Pointer to the buffer to write
 * @param num_bytes [in] The number of bytes to write
 * @return the number of bytes written
 */
size_t hostcpu_transport_write(uint8_t const * buffer, size_t num_bytes);

/**@}*/
/**@}*/

#endif // __HOSTCPU_TRANSPORT_H__

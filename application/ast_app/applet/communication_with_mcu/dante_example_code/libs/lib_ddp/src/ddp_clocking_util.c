/*
* File     : clock_util.c
* Created  : May 2016
* Updated  : Date: 2016/05/19
* Author   : Naeem Bacha <naeem.bacha@audinate.com>
* Synopsis : Utillity helper functions for clocking DDP.
*
* Copyright 2005-2017, Audinate Pty Ltd and/or its licensors
Confidential
* Audinate Copyright Header Version 1
*/

#include "ddp/clocking_util.h"
#include "ddp/clocking_structures.h"


const char * ddp_port_state_to_string(uint8_t port_state)
{
	switch(port_state){
	case CLOCK_PORT_STATE_INVALID: return "INVALID";
	case CLOCK_PORT_STATE_STARTUP: return "STARTUP";
	case CLOCK_PORT_STATE_INITIALIZING: return "INITIALIZING";
	case CLOCK_PORT_STATE_FAULTY: return "FAULTY";
	case CLOCK_PORT_STATE_DISABLED: return "DISABLED";
	case CLOCK_PORT_STATE_LISTENING: return "LISTENING";
	case CLOCK_PORT_STATE_PRE_MASTER: return "PRE_MASTER";
	case CLOCK_PORT_STATE_MASTER: return "MASTER";
	case CLOCK_PORT_STATE_PASSIVE: return "PASSIVE";
	case CLOCK_PORT_STATE_UNCALIBRATED: return "UNCALIBRATED";
	case CLOCK_PORT_STATE_SLAVE: return "SLAVE";
	case CLOCK_PORT_STATE_STANDBY: return "STANDBY";
	}
	return "?";

}

const char * ddp_servo_state_to_string(uint8_t servo_state)
{
	switch(servo_state){
	case CLOCK_SERVO_STATE_INVALID:	return "INVALID";
	case CLOCK_SERVO_STATE_UNKNOWN:	return "UNKNOWN";
	case CLOCK_SERVO_STATE_RESET:	return "RESET";
	case CLOCK_SERVO_STATE_SYNCING:	return "SYNCING";
	case CLOCK_SERVO_STATE_SYNC:	return "SYNC";
	case CLOCK_SERVO_STATE_DELAY_RESET:	return "DELAY_RESET";
	}
	return "?";
}


const char * ddp_pullup_to_string(uint32_t pullup)
{
	switch (pullup)
	{
	case CLOCK_PULLUP_NO_PULLUP:			return "PULLUP_NONE";
	case CLOCK_PULLUP_POS_4_1667_PCENT:		return "PULLUP_+4.1667%";
	case CLOCK_PULLUP_POS_0_1_PCENT:		return "PULLUP_+0.1%";
	case CLOCK_PULLUP_NEG_0_1_PCENT:		return "PULLUP_-0.1%";
	case CLOCK_PULLUP_NEG_4_0_PCENT4:		return "PULLUP_-4.0%";
	}
	return "?";
}


const char * ddp_port_path_type_to_string(uint8_t castness)
{
	switch(castness){
	case CLOCK_CASTNESS_INVALID: return "INVALID";
	case CLOCK_CASTNESS_UNICAST: return "UNICAST";
	case CLOCK_CASTNESS_MULTICAST: return "MULTICAST";
	}
	return "?";
}

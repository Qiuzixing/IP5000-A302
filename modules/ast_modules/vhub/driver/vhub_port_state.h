/*
 * Copyright (c) 2004-2012
 * ASPEED Technology Inc. All Rights Reserved
 * Proprietary and Confidential
 *
 * By using this code you acknowledge that you have signed and accepted
 * the terms of the ASPEED SDK license agreement.
 */


#ifndef _VHUB_PORT_STATE_
#define _VHUB_PORT_STATE_

enum _port_state {
    NOT_CONFIGURED = 0,
    POWERED_OFF,
    DISCONNECTED,
    DISABLED,
    RESETTING,
    ENABLED,
    SUSPENDED,
    RESUMING
};

/* Spec. Figure 11-10 */
enum _port_event {
    NOT_CONFIGURE = 0,
    HUB_CONFIGED,
    SET_PORT_POWER,
    CLEAR_PORT_POWER,
    DISCONNECT,
    TRY_CONNECT, //detect device attach
    EOI, //End Of time Interval
    CLEAR_PORT_ENABLE,
    SET_PORT_RESET,
    SET_PORT_SUSPEND,
    CLEAR_PORT_SUSPEND,
    //Added by myself
    RESET_SUCCESS,
    RESET_FAILED,
    CLEAR_PORT_C_CONNECTION,
    CLEAR_PORT_C_RESET,
    CLEAR_PORT_C_SUSPEND
};


u32 handle_port_event(struct vhub_hcd *vhub, u32 port, enum _port_event event);
u32 get_port_status(struct vhub_hcd *vhub, u32 port);
enum _port_state get_port_state(struct vhub_hcd *vhub, u32 port);


#endif //_VHUB_PORT_STATE_


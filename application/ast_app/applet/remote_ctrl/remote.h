/*
 * Copyright (c) 2004-2012
 * ASPEED Technology Inc. All Rights Reserved
 * Proprietary and Confidential
 *
 * By using this code you acknowledge that you have signed and accepted
 * the terms of the ASPEED SDK license agreement.
 */

#ifndef __REMOTE_H__
#define __REMOTE_H__

#define REMOTE_VER_1    (0xAE010000)
#define REMOTE_VER      (REMOTE_VER_1)
#define REMOTE_VER_MASK (0xFFFF0000)

#define RC_TCP_PORT 6750
#define RC_TCP_PORT_STR "6750"

#define RC_MAX_SIZE 128 // bytes

/* rc_types */
#define RC_CMD_ACCEPTED       (REMOTE_VER | 0x0000) // Client send this cmd back to host after accepted host's command.
#define RC_CMD_REG            (REMOTE_VER | 0x0001) // Register the host ip
#define RC_CMD_EVENT          (REMOTE_VER | 0x0002) // Send an event

struct s_rc_header
{
    unsigned int cmd_type;
    unsigned int data_size;
    unsigned char data[];
} __attribute__ ((packed));


#endif



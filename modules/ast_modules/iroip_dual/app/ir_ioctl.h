/*
 * Copyright (c) 2004
 * ASPEED Technology Inc. All Rights Reserved
 * Proprietary and Confidential
 *
 * By using this code you acknowledge that you have signed and accepted
 * the terms of the ASPEED SDK license agreement.
 */

#ifndef _IR_IOCTL_H_
#define _IR_IOCTL_H_

#define	IR_DOWNSTREAM_PORT 0x4952
#define IR_UPSTREAM_PORT 0x4953

#define IOCTL_IR_START_RX	0x495200
#define IOCTL_IR_START_TX	0x495201
#define IOCTL_IR_START_REMOTE	0x495202
#define IOCTL_IR_START_LOCAL	0x495203


typedef struct _IO_ACCESS_DATA {
	int data;
} IO_ACCESS_DATA, *PIO_ACCESS_DATA;

#endif /* _IR_IOCTL_H_ */

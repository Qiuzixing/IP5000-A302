/*
 * Copyright (c) 2004-2012
 * ASPEED Technology Inc. All Rights Reserved
 * Proprietary and Confidential
 *
 * By using this code you acknowledge that you have signed and accepted
 * the terms of the ASPEED SDK license agreement.
 */

#define	I2S_PORT 1234
#define	CTRL_PORT 1235

#define  IOCTL_IO_READ    0x1103
#define  IOCTL_IO_WRITE   0x1104
#define  IOCTL_REAL_IO_READ    0x1106
#define  IOCTL_REAL_IO_WRITE   0x1107
#define  IOCTL_BIT_STREAM_BASE 0x1108
#define  IOCTL_TX_BIT_STREAM   0x1109
#define  IOCTL_GET_SOCKET      0x1110
#define  IOCTL_AUTOMODE_TRIGGER         0x1111
#define  IOCTL_PASS3_TRIGGER   0x1112
#define  IOCTL_I2C_READ        0x1113
#define  IOCTL_I2C_WRITE       0x1114
#define  IOCTL_I2S_TX          0x1115
#define  IOCTL_I2S_RX          0x1116
#define  IOCTL_ADD_CLIENT  0x1117
#define  IOCTL_REMAP      0x1105
#define  RELOCATE_OFFSET  0x380

typedef struct _IO_ACCESS_DATA {
    int CtrlSocket;
    int I2sSocket;
    unsigned long Data;
	int	transport;//0:UDP; 1:TCP
} IO_ACCESS_DATA, *PIO_ACCESS_DATA;

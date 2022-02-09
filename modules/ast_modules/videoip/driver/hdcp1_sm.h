/*
 * Copyright (c) 2004-2012
 * ASPEED Technology Inc. All Rights Reserved
 * Proprietary and Confidential
 *
 * By using this code you acknowledge that you have signed and accepted
 * the terms of the ASPEED SDK license agreement.
 */

#ifndef _HDCP1_SM_H_
#define _HDCP1_SM_H_

#include <linux/timer.h>
#include <linux/workqueue.h>
#include <asm/semaphore.h>

#include <aspeed/hdcp1.h>
#ifdef CONFIG_AST1500_CAT6023
#include <../../hdmirx/hdmirx_export.h>
#else
#define KSV_FIFO_SIZE 40//KSV_LENGTH * KSV_FIFO_DEVICE_COUNT
#endif

#ifdef CONFIG_ARCH_AST1500_HOST
typedef struct _HDCP1_Repeater_Upstream_Port_Context_
{
	unsigned int	auth_requested;
	struct timer_list	timer;
	struct work_struct timeout;
	unsigned int	state;
#if !HDCP_AUTHENTICATE_LOOPBACK
	unsigned char	default_ksv_set;
	unsigned char	default_ksv[KSV_LENGTH];
#endif
	unsigned short	Bstatus;
	unsigned char	KSV_FIFO[KSV_FIFO_SIZE];
} HDCP1_Repeater_Upstream_Port_Context, *pHDCP1_Repeater_Upstream_Port_Context;
#endif

typedef struct _HDCP1_Repeater_Downstream_Port_Context_
{
	struct work_struct retry;
	struct semaphore lock;
	unsigned char	state;
	unsigned char	Bksv[KSV_LENGTH];
	unsigned char	Bcaps;
	unsigned short	Bstatus;
	unsigned char	KSV_list[150];//KSV_LENGTH * 30
} HDCP1_Repeater_Downstream_Port_Context, *pHDCP1_Repeater_Downstream_Port_Context;

void hdcp1_downstream_port_retry_work(void *data);
#ifdef CONFIG_ARCH_AST1500_HOST
void hdcp1_upstream_port_timeout(unsigned long data);
void hdcp1_upstream_port_timeout_work(void *data);
unsigned int assemble_Ksv_list(void);
#endif

#if HDCP1_STRICTLY_TRANSMIT_LOW_VALUE_CONTENT
void Hdcp1_transmit_low_value_content(void);
void Hdcp1_transmit_normal_content(void);
#else
#define Hdcp1_transmit_low_value_content() do{}while(0)
#define Hdcp1_transmit_normal_content() do{}while(0)
#endif

#endif

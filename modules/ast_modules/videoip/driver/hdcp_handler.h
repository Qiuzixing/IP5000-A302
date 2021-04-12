/*
 * Copyright (c) 2004-2012
 * ASPEED Technology Inc. All Rights Reserved
 * Proprietary and Confidential
 *
 * By using this code you acknowledge that you have signed and accepted
 * the terms of the ASPEED SDK license agreement.
 */
#ifndef _HDCP_HANDLER_H_
#define _HDCP_HANDLER_H_

#include <aspeed/features.h>

void Tx_Hotplug_Callback(unsigned int plugged);
void HDCP1_Downstream_Port_Auth_Status_Callback(unsigned char authenticated, unsigned char *Bksv, unsigned char Bcaps, unsigned short Bstatus, unsigned char *KSV_FIFO);

#ifdef CONFIG_ARCH_AST1500_HOST
#if SUPPORT_HDCP_REPEATER
void HDCP1_Upstream_Port_Auth_Request_Callback(unsigned int enable);
#endif /* #if SUPPORT_HDCP_REPEATER */
#endif /* #ifdef CONFIG_ARCH_AST1500_HOST */

#ifdef CONFIG_ARCH_AST1500_CLIENT
#if SUPPORT_HDCP_REPEATER
void Cmd_network_loop(struct videoip_task *ut);
void hdcp2_timeout(unsigned long data);
void hdcp2_task(struct videoip_task *ut);
void hdcp2_stop(void);
#endif /* #if SUPPORT_HDCP_REPEATER */
#endif /* #ifdef CONFIG_ARCH_AST1500_CLIENT */

#endif /* #ifndef _HDCP_HANDLER_H_ */


/* Copyright (c) 2017
 * ASPEED Technology Inc. All Rights Reserved
 * Proprietary and Confidential
 *
 * By using this code you acknowledge that you have signed and accepted
 * the terms of the ASPEED SDK license agreement.
 */

#ifndef __AST_SEND_EVENT_H__
#define __AST_SEND_EVENT_H__

#define MAX_AST_EVENT_PAYLOAD 1024  /* maximum payload size*/

void ast_send_event(unsigned int dest_pid, char *event_msg);

#endif /**/

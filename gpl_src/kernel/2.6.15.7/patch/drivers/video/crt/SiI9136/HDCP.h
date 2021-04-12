/*
 *****************************************************************************
 *
 * Copyright 2010, Silicon Image, Inc.  All rights reserved.
 * No part of this work may be reproduced, modified, distributed, transmitted,
 * transcribed, or translated into any language or computer format, in any form
 * or by any means without written permission of: Silicon Image, Inc., 1060
 * East Arques Avenue, Sunnyvale, California 94085
 *****************************************************************************
 */
/*
 *****************************************************************************
 * @file  HDCP.h
 *
 * @brief Function prototypes of the HDCP.
 *
 *****************************************************************************
*/
#ifndef __HDCP_H__
#define __HDCP_H__

void HDCP_Init (void);

void HDCP_CheckStatus (uint8_t InterruptStatusImage);
void HDCP_CheckStatus_One(void);

void HDCP_Off(void);
void HDCP_On(void);
void HDCP_Off_without_avmute(void);
u32 link_protection_level(void);
int sink_support_hdcp(void);

#endif //#ifndef __HDCP_H__


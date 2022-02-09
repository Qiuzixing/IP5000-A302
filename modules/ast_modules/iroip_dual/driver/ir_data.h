/*
 * Copyright (c) 2004-2012
 * ASPEED Technology Inc. All Rights Reserved
 * Proprietary and Confidential
 *
 * By using this code you acknowledge that you have signed and accepted
 * the terms of the ASPEED SDK license agreement.
 */

#ifndef _IR_DATA_
#define _IR_DATA_

typedef struct _IRData_Struct
{
	struct list_head	list;
	unsigned int	length;
	unsigned long rx_time;//the time this CMD was received (in jiffies)
	void *pclient_context;//used to identify the Tx client
} IRData_Struct, *PIRData_Struct;

#endif

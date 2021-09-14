/*
 * Copyright (c) 2004-2012
 * ASPEED Technology Inc. All Rights Reserved
 * Proprietary and Confidential
 *
 * By using this code you acknowledge that you have signed and accepted
 * the terms of the ASPEED SDK license agreement.
 */

#ifndef _NAME_SERVICE_
#define _NAME_SERVICE_

#define AST_NAME_SERVICE_QUERY_PORT 3333
#define AST_NAME_SERVICE_REPLY_PORT 3334

typedef enum _AST_Device_Type_
{
	Type_Any = 0,
	Type_Host,
	Type_Client,
	Type_Unknown,
} AST_Device_Type ;

typedef enum _AST_Device_Function_
{
	Function_Any = 0,
	Function_USB,
	Function_Digital,
	Function_Analog,
	Function_Unknown,
} AST_Device_Function ;

typedef enum _AST_Device_Status_
{
	Status_Any = 0,
	Status_Available,
	Status_Busy,
	Status_Idle,
	Status_Unknown,
} AST_Device_Status ;

typedef struct _query_struct_
{
	AST_Device_Type	device_type;
	AST_Device_Function	device_function;
//	AST_Device_Status	device_status;
}query_struct, *pquery_struct;

#define MAX_STATUS_LENGTH 32
#define MAX_NAME_LENGTH 64
#define MAX_MODEL_LENGTH 32
#define MAX_VER_LENGTH 16
#define MAX_CH_NUM_LENGTH 8
#define MAX_RESERVED_LENGTH 136
typedef struct _reply_struct_
{
	AST_Device_Type	device_type;
	AST_Device_Function	device_function;
	char device_status[MAX_STATUS_LENGTH];
	char device_name[MAX_NAME_LENGTH];
	char model_name[MAX_MODEL_LENGTH];
	char version[MAX_VER_LENGTH];
	char channel_number[MAX_CH_NUM_LENGTH];
	char reserved[MAX_RESERVED_LENGTH];
}reply_struct, *preply_struct;

//AST_Device_Status device_status = Status_Unknown;
#define AST_NAME_SERVICE_GROUP_ADDR "225.1.0.0";

#endif

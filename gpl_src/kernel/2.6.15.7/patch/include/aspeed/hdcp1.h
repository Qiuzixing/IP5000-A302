#ifndef _HDCP1_H_
#define _HDCP1_H_

#include <asm/arch/drivers/board_def.h>

#define HDCP1_MAX_DEVICE_COUNT	127
#define HDCP1_MAX_DEPTH	31
#define KSV_LENGTH	5

//Bcaps bit definitions
#define HDCP1_BCAPS_REPEATER_MASK	0x40

//Bstatus bit definitions
#define HDCP1_BSTATUS_DEVICE_COUNT_MASK	0x007F
#define HDCP1_BSTATUS_DEVICE_COUNT_SHIFT	0
#define HDCP1_BSTATUS_MAX_DEVS_EXCEEDED_MASK	0x0080	
#define HDCP1_BSTATUS_DEPTH_MASK	0x0700
#define HDCP1_BSTATUS_DEPTH_SHIFT	8
#define HDCP1_BSTATUS_MAX_CASCADE_EXCEEDED_MASK	0x0800	
#define HDCP1_BSTATUS_HDMI_MODE_MASK	0x1000	

#define HDCP1_Repeater_Upstream_Port_Timeout_Wait_for_Downstream	5000

//upstream port states
#define HDCP1_Repeater_Upstream_Port_State_Unauthenticated	0
#define HDCP1_Repeater_Upstream_Port_State_Wait_for_Downstream	1
#define HDCP1_Repeater_Upstream_Port_State_Assemble_KSV_List	2
#define HDCP1_Repeater_Upstream_Port_State_Authenticated	3
//downstream port states
#define HDCP1_Repeater_Downstream_Port_State_No_Rx_Attached	0
#define HDCP1_Repeater_Downstream_Port_State_Transmit_Low_Value	1
#define HDCP1_Repeater_Downstream_Port_State_Wait_for_Active_Rx	2
#define HDCP1_Repeater_Downstream_Port_State_Authenticating	3
#define HDCP1_Repeater_Downstream_Port_State_Authenticated	4

//interface between video driver and HDMI Rx driver
void vrxhal_HDCP1_set_upstream_port_mode(unsigned char repeater);
void vrxhal_HDCP1_set_upstream_port_auth_status(unsigned int state, unsigned short Bstatus, void *KSV_list);
typedef void (*pfn_HDCP1_Upstream_Port_Auth_Request_Callback)(unsigned int enable);
void vrxhal_HDCP1_reg_upstream_port_auth_request_callback(pfn_HDCP1_Upstream_Port_Auth_Request_Callback);
void vrxhal_HDCP1_dereg_upstream_port_auth_request_callback(void);

//>>>>>>>> To be removed?!
unsigned char Get_HDCP1_Upstream_Port_Encryption_Status(void);
typedef void (*pfn_HDCP1_Upstream_Port_Encryption_Status_Callback)(unsigned int);
void Reg_HDCP1_Upstream_Port_Encryption_Status_Callback(pfn_HDCP1_Upstream_Port_Encryption_Status_Callback);
void Dereg_HDCP1_Upstream_Port_Encryption_Status_Callback(void);
//<<<<<<<< To be removed?!

#endif

/*
 * Copyright (c) 2004-2012
 * ASPEED Technology Inc. All Rights Reserved
 * Proprietary and Confidential
 *
 * By using this code you acknowledge that you have signed and accepted
 * the terms of the ASPEED SDK license agreement.
 */

#ifndef _HDCP2_SM_H_
#define _HDCP2_SM_H_

#include <linux/list.h>
#include <linux/spinlock.h>
#include <linux/timer.h>
#include <linux/workqueue.h>
#include <aspeed/features.h>
#ifdef CONFIG_AST1500_CAT6023
#include <../../hdmirx/hdmirx_export.h>
#endif
#include "hdcp1_sm.h"

//HDCP 2.X

#define HDCP2_GLOBAL_CONSTANT_LENGTH	16
#define HDCP2_DCP_RSA_PUBLIC_KEY_MOD_LENGTH	384
#define HDCP2_DCP_RSA_PUBLIC_KEY_EXP_LENGTH	1
#define HDCP2_DCP_RSA_PUBLIC_KEY_LENGTH	385
#define HDCP2_RX_CERT_DATA_LENGTH	138
#define HDCP2_RX_CERT_SIG_LENGTH	384
#define HDCP2_RX_CERT_LENGTH	522
#define HDCP2_RX_RSA_PUBLIC_KEY_MOD_LENGTH	128
#define HDCP2_RX_RSA_PUBLIC_KEY_EXP_LENGTH	3
#define HDCP2_RECEIVER_ID_LENGTH	5
#define HDCP2_SESSION_KEY_LENGTH	16
#define HDCP2_IV_LENGTH	8
#define HDCP2_AES_INPUTCTR_LENGTH	8
#if 0
#define HDCP2_RX_RSA_PRIVATE_KEY_LENGTH	131
#else
#define HDCP2_RX_RSA_PRIVATE_KEY_CRT_LENGTH	320
#endif

//DCP RSA public key
typedef struct _DCP_RSA_Public_Key_
{
	unsigned char n[HDCP2_DCP_RSA_PUBLIC_KEY_MOD_LENGTH];
	unsigned char e[HDCP2_DCP_RSA_PUBLIC_KEY_EXP_LENGTH];
} __attribute__ ((packed)) DCP_RSA_Public_Key, *pDCP_RSA_Public_Key;

//Rx RSA public key
typedef struct _Rx_RSA_Public_Key_
{
	unsigned char n[HDCP2_RX_RSA_PUBLIC_KEY_MOD_LENGTH];
	unsigned char e[HDCP2_RX_RSA_PUBLIC_KEY_EXP_LENGTH];
} __attribute__ ((packed)) Rx_RSA_Public_Key, *pRx_RSA_Public_Key;

//Rx RSA private key
#if 0
typedef struct _Rx_RSA_Private_Key_
{
	unsigned char n[128];
	unsigned char d[3];
} __attribute__ ((packed)) Rx_RSA_Private_Key, *pRx_RSA_Private_Key;
#else
typedef struct _Rx_RSA_Private_Key_CRT_
{
	unsigned char p[64];
	unsigned char q[64];
	unsigned char dp[64];
	unsigned char dq[64];
	unsigned char qp[64];
} __attribute__ ((packed)) Rx_RSA_Private_Key_CRT, *pRx_RSA_Private_Key_CRT;
#endif

//HDCP Rx RSA Public Key Certificate
typedef struct _Rx_Public_Key_Certificate_
{
	unsigned char Rx_ID[HDCP2_RECEIVER_ID_LENGTH];
	Rx_RSA_Public_Key kpub_rx;
	unsigned char Reserved[2];
	unsigned char DCP_LLC_Signature[HDCP2_RX_CERT_SIG_LENGTH];
} __attribute__ ((packed)) Rx_Public_Key_Certificate, *pRx_Public_Key_Certificate;

#if 0
typedef struct _HDCP_Rx_
{
	unsigned char lp_128[16];
	Rx_Public_Key_Certificate cert_rx;
	unsigned char kpriv_rx[128];
} HDCP_Rx, *pHDCP_Rx;
#endif

#define BOOL_MASK	1
//msg_id
#define MSG_Null_message	1
#define MSG_AKE_Init	2
#define MSG_AKE_Send_Cert	3
#define MSG_AKE_No_Stored_km	4
#define MSG_AKE_Stored_km	5
#define MSG_AKE_Send_rrx	6
#define MSG_AKE_Send_H_prime	7
#define MSG_AKE_Send_Pairing_Info	8
#define MSG_LC_Init	9
#define MSG_LC_Send_L_prime	10
#define MSG_SKE_Send_Eks	11
#define MSG_RepeaterAuth_Send_ReceiverID_List	12
#define MSG_RTT_Ready	13
#define MSG_RTT_Challenge	14
#define MSG_RepeaterAuth_Send_Ack	15
#define MSG_RepeaterAuth_Stream_Manage	16
#define MSG_RepeaterAuth_Stream_Ready	17
#define MSG_Receiver_AuthStatus	18
#define MSG_AKE_Transmitter_Info	19
#define MSG_AKE_Receiver_Info	20

typedef struct _AKE_Init_
{
	unsigned char msg_id;
	unsigned char r_tx[8];
} __attribute__ ((packed)) AKE_Init, *pAKE_Init;

typedef struct _AKE_Send_Cert_
{
	unsigned char msg_id;
	unsigned char REPEATER;
	unsigned char cert_rx[HDCP2_RX_CERT_LENGTH];
} __attribute__ ((packed)) AKE_Send_Cert, *pAKE_Send_Cert;

typedef struct _AKE_No_Stored_km_
{
	unsigned char msg_id;
	unsigned char Ekpub_km[128];
} __attribute__ ((packed)) AKE_No_Stored_km, *pAKE_No_Stored_km;

typedef struct _AKE_Stored_km_
{
	unsigned char msg_id;
	unsigned char Ekh_km[16];
	unsigned char m[16];
} __attribute__ ((packed)) AKE_Stored_km, *pAKE_Stored_km;

typedef struct _AKE_Send_rrx_
{
	unsigned char msg_id;
	unsigned char r_rx[8];
} __attribute__ ((packed)) AKE_Send_rrx, *pAKE_Send_rrx;

typedef struct _AKE_Send_H_prime_
{
	unsigned char msg_id;
	unsigned char H_prime[32];
} __attribute__ ((packed)) AKE_Send_H_prime, *pAKE_Send_H_prime;

typedef struct _AKE_Send_Pairing_Info_
{
	unsigned char msg_id;
	unsigned char Ekh_km[16];
} __attribute__ ((packed)) AKE_Send_Pairing_Info, *pAKE_Send_Pairing_Info;

#define AKE_Transmitter_Info_LENGTH	6
#define AKE_Transmitter_Info_VERSION	1
#define TRANSMITTER_LOCALITY_PRECOMPUTE_SUPPORT	0x01
typedef struct _AKE_Transmitter_Info_
{
	unsigned char msg_id;
	unsigned char LENGTH[2];
	unsigned char VERSION;
	unsigned char TRANSMITTER_CAPABILITY_MASK[2];
} __attribute__ ((packed)) AKE_Transmitter_Info, *pAKE_Transmitter_Info;

#define AKE_Receiver_Info_LENGTH	6
#define AKE_Receiver_Info_VERSION	1
#define RECEIVER_LOCALITY_PRECOMPUTE_SUPPORT	0x01
typedef struct _AKE_Receiver_Info_
{
	unsigned char msg_id;
	unsigned char LENGTH[2];
	unsigned char VERSION;
	unsigned char RECEIVER_CAPABILITY_MASK[2];
} __attribute__ ((packed)) AKE_Receiver_Info, *pAKE_Receiver_Info;

typedef struct _LC_Init_
{
	unsigned char msg_id;
	unsigned char r_n[8];
} __attribute__ ((packed)) LC_Init, *pLC_Init;

typedef struct _LC_Send_L_prime_0_
{
	unsigned char msg_id;
	unsigned char L_prime[32];
} __attribute__ ((packed)) LC_Send_L_prime_0, *pLC_Send_L_prime_0;

typedef struct _LC_Send_L_prime_1_
{
	unsigned char msg_id;
	unsigned char L_prime[16];
} __attribute__ ((packed)) LC_Send_L_prime_1, *pLC_Send_L_prime_1;

typedef struct _RTT_Ready_
{
	unsigned char msg_id;
} __attribute__ ((packed)) RTT_Ready, *pRTT_Ready;

typedef struct _RTT_Challenge_
{
	unsigned char msg_id;
	unsigned char L[16];
} __attribute__ ((packed)) RTT_Challenge, *pRTT_Challenge;

typedef struct _SKE_Send_Eks_
{
	unsigned char msg_id;
	unsigned char Edkey_ks[16];
	unsigned char r_iv[HDCP2_IV_LENGTH];
} __attribute__ ((packed)) SKE_Send_Eks, *pSKE_Send_Eks;

#define HDCP2_MAX_DEVICE_COUNT	31
#define HDCP2_MAX_DEPTH	4
#define HDCP2_MAX_RECEIVER_ID_LIST_LENGTH	165//RECEIVER_ID_LENGTH * HDCP2_MAX_DEVICE_COUNT
typedef struct _RepeaterAuth_Send_ReceiverID_List_Header_
{
	unsigned char msg_id;
	unsigned char MAX_DEVS_EXCEEDED;
	unsigned char MAX_CASCADE_EXCEEDED;
} __attribute__ ((packed)) RepeaterAuth_Send_ReceiverID_List_Header, *pRepeaterAuth_Send_ReceiverID_List_Header;

typedef struct _RepeaterAuth_Send_ReceiverID_List_0_
{
	unsigned char DEVICE_COUNT;
	unsigned char DEPTH;
	unsigned char V_prime[32];
} __attribute__ ((packed)) RepeaterAuth_Send_ReceiverID_List_0, *pRepeaterAuth_Send_ReceiverID_List_0;

typedef struct _RepeaterAuth_Send_ReceiverID_List_1_
{
	unsigned char DEVICE_COUNT;
	unsigned char DEPTH;
	unsigned char HDCP2_0_REPEATER_DOWNSTREAM;
	unsigned char HDCP1_DEVICE_DOWNSTREAM;
	unsigned char seq_num_V[3];
	unsigned char V_prime[16];
} __attribute__ ((packed)) RepeaterAuth_Send_ReceiverID_List_1, *pRepeaterAuth_Send_ReceiverID_List_1;

typedef struct _RepeaterAuth_Send_Ack_
{
	unsigned char msg_id;
	unsigned char V[16];
} __attribute__ ((packed)) RepeaterAuth_Send_Ack, *pRepeaterAuth_Send_Ack;

typedef struct _Receiver_AuthStatus_
{
	unsigned char msg_id;
	unsigned char LENGTH[2];
	unsigned char REAUTH_REQ;
} __attribute__ ((packed)) Receiver_AuthStatus, *pReceiver_AuthStatus;

typedef struct _RepeaterAuth_Stream_Manage_Header_
{
	unsigned char msg_id;
	unsigned char seq_num_M[3];
	unsigned char k[2];
} __attribute__ ((packed)) RepeaterAuth_Stream_Manage_Header, *pRepeaterAuth_Stream_Manage_Header;

typedef struct _Stream_Manage_Entry_
{
	unsigned char streamCtr[4];
	unsigned char ContentStreamID[2];
	unsigned char Type;
} __attribute__ ((packed)) Stream_Manage_Entry, *pStream_Manage_Entry;

typedef struct _RepeaterAuth_Stream_Ready_
{
	unsigned char msg_id;
	unsigned char M_prime[32];
} __attribute__ ((packed)) RepeaterAuth_Stream_Ready, *pRepeaterAuth_Stream_Ready;

//HDCP repeater upstream authentication protocol states
typedef enum _HDCP2_Repeater_Upstream_State_
{
    Unauthenticated = 0,
    Compute_km,
	Compute_L_prime,
	Compute_ks,//transient
	Wait_for_Downstream,
	Assemble_Receiver_ID_List,//transient
	Authenticated,
	Verify_Receiver_ID_List_Ack,//HDCP 2.1
	Content_stream_management,//HDCP 2.1; transient
} HDCP2_Repeater_Upstream_State;

//HDCP repeater upstream authentication protocol events
typedef enum _HDCP2_Repeater_Upstream_Event_
{
    Reset = 0,
    AKE_Init_received,
	AKE_No_Stored_km_received,
	AKE_Stored_km_received,
	LC_Init_received,
	SKE_Send_Eks_received,
	Downstream_Port_Disconnected,
	Downstream_Port_Unauthenticated,
	Downstream_Port_Authenticated,
	Upstream_Timeouted,
	AKE_Transmitter_Info_received,//HDCP 2.1
	RTT_Challenge_received,//HDCP 2.1
	RepeaterAuth_Send_Ack_received,//HDCP 2.1
	RepeaterAuth_Stream_Manage_received,//HDCP 2.1
} HDCP2_Repeater_Upstream_Event;

//timeouts in ms
#define Timeout_AKE_Send_rrx_No_Stored_km (((videoip_debug_flag & videoip_debug_hdcp2_mask) >= HDCP2_DBG_LEVEL3)?10000:1000)
#define Timeout_AKE_Send_rrx_Stored_km (((videoip_debug_flag & videoip_debug_hdcp2_mask) >= HDCP2_DBG_LEVEL3)?2000:200)
#define Timeout_AKE_Send_Pairing_Info (((videoip_debug_flag & videoip_debug_hdcp2_mask) >= HDCP2_DBG_LEVEL3)?2000:200)
#define Timeout_RepeaterAuth_Send_ReceiverID_List (((videoip_debug_flag & videoip_debug_hdcp2_mask) >= HDCP2_DBG_LEVEL3)?30000:3000)
#if 0//mission impossible
#define Timeout_AKE_Send_Cert	100
#define Timeout_Locality_Check	7
#define Timeout_Receiver_ID_List_Ack	1000
#else
#define Timeout_AKE_Send_Cert	(((videoip_debug_flag & videoip_debug_hdcp2_mask) >= HDCP2_DBG_LEVEL3)?2000:200)
#define Timeout_Locality_Check	(((videoip_debug_flag & videoip_debug_hdcp2_mask) >= HDCP2_DBG_LEVEL3)?2000:200)
#define Timeout_Receiver_ID_List_Ack	(((videoip_debug_flag & videoip_debug_hdcp2_mask) >= HDCP2_DBG_LEVEL3)?15000:1500)
#endif
//HDCP repeater downstream authentication protocol states
typedef enum _HDCP2_Repeater_Downstream_State_
{
    No_Rx_Attached = 0,
    Transmit_Low_value_Content,
#if 0
	Determine_Rx_HDCP_Capable,//transient
#endif
	Exchange_km,
	Locality_Check,
	Exchange_ks,//transient
	Test_for_Repeater,//transient
	Rx_Authenticated,
	Wait_for_RepeaterAuth_Send_ReceiverID_List,
	Verify_Receiver_ID_List,//transient
	Send_Receiver_ID_List_Ack,//HDCP 2.1; transient
	Content_Stream_management,//HDCP 2.1
	//states define by Aspeed
	Wait_for_AKE_Send_Cert,
	Wait_for_AKE_Receiver_Info,//HDCP 2.1
	Wait_for_AKE_Send_rrx,
	Wait_for_AKE_Send_H_prime,
	Wait_for_AKE_Send_Pairing_Info,
	Wait_for_RTT_Ready,//HDCP 2.1
	Wait_for_LC_Send_L_prime,
} HDCP2_Repeater_Downstream_State;

//HDCP repeater downstream authentication protocol events
typedef enum _HDCP2_Repeater_Downstream_Event_
{
    Tx_Reset = 0,
    Receiver_Disconnected_Indication,
	Receiver_Connected_Indication,
	Upstream_Auth_Request,
	AKE_Send_Cert_received,
	AKE_Send_rrx_received,
	AKE_Send_H_prime_received,
	AKE_Send_Pairing_Info_received,
	LC_Send_L_prime_received,
	RepeaterAuth_Send_ReceiverID_List_received,
	Downstream_Timeouted,
	AKE_Receiver_Info_received,//HDCP 2.1
	RTT_Ready_received,//HDCP 2.1
	Receiver_AuthStatus_received,//HDCP 2.1
	RepeaterAuth_Stream_Ready_received,//HDCP 2.1
} HDCP2_Repeater_Downstream_Event;

typedef struct _Hdcp2_event_
{
	struct list_head	list;
	unsigned int	event;
	void	*pdata;
	unsigned int	len;
	unsigned int	free_data;
} Hdcp2_event, *pHdcp2_event;

#define CMD_Type_HDCP2_Message 0x0A
typedef struct _Cmd_packet_header_
{
	unsigned int type;
	unsigned int length;
} Cmd_packet_header, *pCmd_packet_header;

typedef struct _HDCP2_Repeater_Downstream_Port_Context_
{
	struct list_head	hdcp2_eventQ;
	spinlock_t			hdcp2_eventQ_lock;
	struct timer_list	timer;
	HDCP2_Repeater_Downstream_State	hdcp2_downstream_state;
	//Receiver ID list cache relevant
	unsigned char	MAX_DEVS_EXCEEDED;
	unsigned char	MAX_CASCADE_EXCEEDED;
	unsigned char	DEVICE_COUNT;
	unsigned char	DEPTH;
	unsigned char 	ReceiverID_List[HDCP2_MAX_RECEIVER_ID_LIST_LENGTH];
	//AKE relevant
	unsigned char	Km_Stored;//steven: I assume that host will not store Km in nonvolatile storage so that Km will only be stored for a connecting client.
	unsigned char	r_tx[8];
	unsigned char	ctr[8];
	unsigned char	r_rx[8];
	unsigned char	Repeater;
	Rx_Public_Key_Certificate	cert_rx;
	unsigned int	Rx_HDCP2_0_Compliant;
	unsigned char	RECEIVER_CAPABILITY_MASK[2];
	unsigned char	k_m[16];
	unsigned char	Ekpub_km[128];
	unsigned char	k_d[32];
	unsigned char	Ekh_km[16];
	unsigned char	m[16];
	unsigned char	H[32];
	//LC relevant
	unsigned char	r_n[8];
	unsigned char	L[32];
	//SKE relevant
#if 0//k_s and r_iv are shared among clients.
	unsigned char	k_s[16];
	unsigned char	r_iv[8];
#endif
	unsigned char	V[32];
} HDCP2_Repeater_Downstream_Port_Context, *pHDCP2_Repeater_Downstream_Port_Context;

typedef struct _HDCP2_Repeater_Upstream_Port_Context_
{
	unsigned int	auth_requested;
	struct list_head	hdcp2_eventQ;
	spinlock_t	hdcp2_eventQ_lock;
	struct timer_list	timer;
	HDCP2_Repeater_Upstream_State	hdcp2_upstream_state;
	unsigned char	lc_128[HDCP2_GLOBAL_CONSTANT_LENGTH];
	//Receiver ID list cache relevant
	unsigned char	MAX_DEVS_EXCEEDED;
	unsigned char	MAX_CASCADE_EXCEEDED;
	unsigned char	DEVICE_COUNT;
	unsigned char	DEPTH;
	unsigned char 	ReceiverID_List[HDCP2_MAX_RECEIVER_ID_LIST_LENGTH];
	//HDCP 2.x RSA public and private keys
	Rx_Public_Key_Certificate	cert_rx;
#if 0
	Rx_RSA_Private_Key	kpriv_rx;
#else
	Rx_RSA_Private_Key_CRT kpriv_rx_crt;
#endif
	//HDCP 2.x state machine relevant
	unsigned char	r_tx[8];
	unsigned char	ctr[8];
	unsigned char	r_rx[8];
	unsigned char	Repeater;
	unsigned char	k_m[16];
	unsigned char	k_d[32];
	unsigned int	Tx_HDCP2_0_Compliant;
	unsigned char	TRANSMITTER_CAPABILITY_MASK[2];
	unsigned char	r_n[8];
	unsigned char	L_prime[32];
	unsigned char	k_s[HDCP2_SESSION_KEY_LENGTH];
	unsigned char	r_iv[HDCP2_IV_LENGTH];
	unsigned int	seq_num_V;
	unsigned char	V_prime[32];
} HDCP2_Repeater_Upstream_Port_Context, *pHDCP2_Repeater_Upstream_Port_Context;

void add_hdcp2_timer(struct timer_list *timer, unsigned long timeout);
#if defined(CONFIG_ARCH_AST1500_HOST)
#if HDCP_AUTHENTICATE_LOOPBACK
void HDCP2_Repeater_Downstream_Port_State_Machine(pHDCP2_Repeater_Downstream_Port_Context pHdcp2_context, HDCP2_Repeater_Downstream_Event event, unsigned char *pdata, unsigned int len, pHDCP1_Repeater_Upstream_Port_Context pupstream_context, pHDCP1_Repeater_Downstream_Port_Context pHdcp1_context);
#else
void HDCP2_Repeater_Downstream_Port_State_Machine(pHDCP2_Repeater_Downstream_Port_Context pHdcp2_context, HDCP2_Repeater_Downstream_Event event, unsigned char *pdata, unsigned int len, pHDCP1_Repeater_Upstream_Port_Context pupstream_context);
#endif
#endif
void HDCP2_Repeater_Upstream_Port_State_Machine(pHDCP2_Repeater_Upstream_Port_Context pHdcp2_context, HDCP2_Repeater_Upstream_Event event, void *pdata, unsigned int len, pHDCP1_Repeater_Downstream_Port_Context pHdcp1_context);

#endif

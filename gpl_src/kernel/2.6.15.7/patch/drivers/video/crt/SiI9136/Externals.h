/*
 *****************************************************************************
 *
 * Copyright 2002-2009,2010,2011, Silicon Image, Inc.  All rights reserved.
 * No part of this work may be reproduced, modified, distributed, transmitted,
 * transcribed, or translated into any language or computer format, in any form
 * or by any means without written permission of: Silicon Image, Inc., 1060
 * East Arques Avenue, Sunnyvale, California 94085
 *****************************************************************************
 */
/*
 *****************************************************************************
 * @file  Externals.h
 *
 * @brief Defines and external definitions.
 *
 *****************************************************************************
*/

#ifndef __EXTERNALS_H_ //(
#define __EXTERNALS_H_

#include "videomodedesc.h"

#include "serialpacket.h"
extern VideoModeDescription_t VideoModeDescription;
//extern AMFVideoAudioModeDescription_t AMFVideoAudioModeDescription;
extern int VModeTblIndex;

//extern uint8_t CommModeState;
#define NOT_IN_COMM_MODE	0
#define IN_COMM_MODE		1

// Patches
//========
extern uint8_t EmbeddedSynPATCH;

#if !(AST_HDMITX)
//UART
//====
extern uint8_t TXBusy;
extern uint8_t IDX_InChar;
extern uint8_t NumOfArgs;
extern uint8_t MessageType;
#endif

extern uint8_t g_audio_Checksum;	// Audio checksum

// Communication
//extern FirmwareScratchPad_u g_ScratchPad;

extern uint16_t globalFlags;
#if (AST_HDMITX)
extern u8 global_sink_edid[256];
extern struct s_xmiter_info xmiter_info[];
extern u8 hdcp_retry_counter;
#endif


#ifdef MHL_CONNECTION_STATE_MACHINE
	// External
	extern unsigned char toggleCbus;
	extern void txPowerD3 (void);
	extern void InitForceUsbIdSwitchOpen (void);
	extern void InitReleaseUsbIdSwitchOpen (void);
	extern void ForceUsbIdSwitchOpen (void);
	extern void ReleaseUsbIdSwitchOpen (void);
	extern void CbusWakeUpGenerator (void);
#endif

typedef enum
{
      tmdsPoweredUpBit      = 0x0001
    , dsRxPoweredUpBit      = 0x0002
    , HDCP_StartedBit       = 0x0004
    , HDCP_AksvValidBit     = 0x0008
    , HDCP_TxSupportsBit    = 0x0010
    , HDCP_OverrideBit      = 0x0020
    , edidDataValidBit      = 0x0040
    , USRX_OutputChangeBit  = 0x0080
    , InTpiModeBit         = 0x0100
    , HDCPIsAuthenticatedBit     = 0x0200
#if (AST_HDMITX)
    , CableConnectedBit     = 0x8000
#endif
}globalFlags_e;

#define tmdsPoweredUp      ((globalFlags &  tmdsPoweredUpBit    )?1:0)
#define SetTmdsPoweredUp     globalFlags |= tmdsPoweredUpBit;
#define ClrTmdsPoweredUp     globalFlags &=~tmdsPoweredUpBit;

#define dsRxPoweredUp      ((globalFlags &  dsRxPoweredUpBit    )?1:0)
#define SetDsRxPoweredUp     globalFlags |= dsRxPoweredUpBit;
#define ClrDsRxPoweredUp     globalFlags &=~dsRxPoweredUpBit;
#define AssignDsRxPoweredUp(val)  globalFlags = (val)?(globalFlags | dsRxPoweredUpBit) : globalFlags & ~dsRxPoweredUpBit;

#define HDCP_Started       ((globalFlags &  HDCP_StartedBit     )?1:0)
#define SetHDCP_Started      globalFlags |= HDCP_StartedBit;
#define ClrHDCP_Started      globalFlags &=~HDCP_StartedBit;

#define HDCP_AksvValid     ((globalFlags &  HDCP_AksvValidBit    )?1:0)
#define SetHDCP_AksvValid    globalFlags |= HDCP_AksvValidBit;
#define ClrHDCP_AksvValid    globalFlags &=~HDCP_AksvValid;

#define HDCP_TxSupports    ((globalFlags &  HDCP_TxSupportsBit   )?1:0)
#define SetHDCP_TxSupports   globalFlags |= HDCP_TxSupportsBit;
#define ClrHDCP_TxSupports   globalFlags &=~HDCP_TxSupportsBit;

#define HDCP_Override      ((globalFlags &  HDCP_OverrideBit     )?1:0)
#define SetHDCP_Override     globalFlags |= HDCP_OverrideBit;
#define ClrHDCP_Override     globalFlags &=~HDCP_Override;

#define edidDataValid      ((globalFlags &  edidDataValidBit     )?1:0)
#define SetEdidDataValid     globalFlags |= edidDataValidBit;
#define ClrEdidDataValid     globalFlags &=~edidDataValidBit;

#define USRX_OutputChange  ((globalFlags &  USRX_OutputChangeBit )?1:0)
#define SetUSRX_OutputChange globalFlags |= USRX_OutputChangeBit;
#define ClrUSRX_OutputChange globalFlags &=~USRX_OutputChangeBit;

#define bInTpiMode         ((globalFlags &   InTpiModeBit)?1:0)
#define SetInTpiMode         globalFlags |=  InTpiModeBit;
#define ClrInTpiMode         globalFlags &= ~InTpiModeBit;

#define HDCPIsAuthenticated           ((globalFlags &  HDCPIsAuthenticatedBit    )?1:0)
#define SetHDCPIsAuthenticated         {globalFlags |= HDCPIsAuthenticatedBit; uinfo("SetHDCPIsAuthenticated :\n"); }
#define ClrHDCPIsAuthenticated         {globalFlags &=~HDCPIsAuthenticatedBit; uinfo("ClrHDCPIsAuthenticated\n"); }
#define AssignHDCPIsAuthenticated(val) {globalFlags = (val)?(globalFlags | HDCPIsAuthenticatedBit) : globalFlags & ~HDCPIsAuthenticatedBit;  uinfo("AssignHDCPIsAuthenticated:%s\n",val);}


#if (AST_HDMITX)
#define CableConnected     ((globalFlags &  CableConnectedBit     )?1:0)
#define SetCableConnected    globalFlags |= CableConnectedBit;
#define ClrCableConnected    globalFlags &=~CableConnectedBit;

#define HDCP_RETRY_COUNTER	hdcp_retry_counter
#define HDCP_RETRY_COUNTER_INC	{hdcp_retry_counter++; }
#define HDCP_RETRY_COUNTER_RESET {hdcp_retry_counter = 0; }

#endif

#define SetGlobalFlags(flags)  globalFlags |= flags;
#define ClrGlobalFlags(flags)  globalFlags &=~(flags);

extern uint8_t serialFlags;
typedef enum
{
      F_SBUF_DataReadyBit   = 0x01
    , F_CollectingDataBit   = 0x02
}serialFlags_e;

#define F_SBUF_DataReady      ((serialFlags &  F_SBUF_DataReadyBit    )?1:0)
#define SetF_SBUF_DataReady     serialFlags |= F_SBUF_DataReadyBit;
#define ClrF_SBUF_DataReady     serialFlags &=~F_SBUF_DataReadyBit;
#define AssignF_SBUF_DataReady(val)  serialFlags = (val)?(serialFlags | F_SBUF_DataReadyBit) : serialFlags & ~F_SBUF_DataReadyBit;


#define F_CollectingData      ((serialFlags &  F_CollectingDataBit    )?1:0)
#define SetF_CollectingData     serialFlags |= F_CollectingDataBit;
#define ClrF_CollectingData     serialFlags &=~F_CollectingDataBit;
#define AssignF_CollectingData(val)  serialFlags = (val)?(serialFlags | F_CollectingDataBit) : serialFlags & ~F_CollectingDataBit;

#ifdef MHL_CONNECTION_STATE_MACHINE //(

#define MHLCableConnected (((txPowerState == txpsD0_Connected                            ) \
                        || (txPowerState == txpsD0_ConnectedReadyToSampleRSEN           ) \
                        || (txPowerState == txpsD0_HDMICableConnected                   ) \
                        || (txPowerState == txpsD0_HDMICableConnectedReadyToSampleRSEN  ) \
                        || (txPowerState == txpsD0_HDCPAuthenticated                    ) \
                        || (txPowerState == txpsD0_HDCPAuthenticatedReadyToSampleRSEN   ) \
                        )?1:0)

//
// structure to hold command details from upper layer to CBUS module
//
typedef struct _mhlTx_config_t
{
    uint8_t status_0;
    uint8_t status_1;
    uint8_t linkMode;
    uint8_t connectedReady;
    uint8_t mhlHpdRSENflags;       // keep track of SET_HPD/CLR_HPD
    uint8_t cbusRevID;
    uint8_t mhlFlags;
    uint8_t mscRequesterAbortReason;    // keep track of downstream HPD, etc
    uint8_t     miscFlags;          // such as SCRATCHPAD_BUSY


	bool_t		mhlConnectionEvent;
	uint8_t		mhlConnected;

	// mscMsgArrived == true when a MSC MSG arrives, false when it has been picked up
	bool_t		mscMsgArrived;
	uint8_t		mscMsgSubCommand;
	uint8_t		mscMsgData;

	// Remember FEATURE FLAG of the peer to reject app commands if unsupported
	uint8_t		mscFeatureFlag;

    uint8_t cbusReferenceCount;         // keep track of CBUS requests
	// Mostly for READ_DEVCAP command and other non-MSC_MSG commands
	uint8_t		mscLastCommand;
	uint8_t		mscLastOffset;
	uint8_t		mscLastData;

	// Remember last MSC_MSG command (RCPE particularly)
	uint8_t		mscMsgLastCommand;
	uint8_t		mscMsgLastData;
	uint8_t		mscSaveRcpKeyCode;

    //  support WRITE_BURST
    uint8_t     localScratchPad[16];


}mhlTx_config_t;

#define hdmiCableConnected(x) \
    (MSC_REQUESTER_ABORT_REASON_DOWNSTREAM_HOT_PLUG_STATUS & x)

#define ResetDownStreamHotPlugDetectionStatus \
    mhlTxConfig.mscRequesterAbortReason &= ~MSC_REQUESTER_ABORT_REASON_DOWNSTREAM_HOT_PLUG_STATUS;

#define downStreamHotPlugDetectionStatusChanged(x) \
    (MSC_REQUESTER_ABORT_REASON_DOWNSTREAM_HOT_PLUG_STATUS \
    & (x ^ mhlTxConfig.mscRequesterAbortReason))
typedef enum
{
        tclkStableBit       = 0x01
    ,   checkTclkStableBit  = 0x02
    ,   mscCmdInProgressBit = 0x04
#ifdef SWWA_20005 //{
    ,   RxClockUnstableBit  = 0x80
#endif //}
}mhlFlags_e;

#define tclkStable     ((mhlTxConfig.mhlFlags &   tclkStableBit)?1:0)
#define SetTclkStable    mhlTxConfig.mhlFlags |=  tclkStableBit;
#define ClrTclkStable    mhlTxConfig.mhlFlags &= ~tclkStableBit;

#define checkTclkStable  ((mhlTxConfig.mhlFlags &   checkTclkStableBit)?1:0)
#define SetCheckTclkStable mhlTxConfig.mhlFlags |=  checkTclkStableBit;
#define ClrCheckTclkStable mhlTxConfig.mhlFlags &= ~checkTclkStableBit;

#define mscCmdInProgress  ((mhlTxConfig.mhlFlags &   mscCmdInProgressBit)?1:0)
#define SetMscCmdInProgress mhlTxConfig.mhlFlags |=  mscCmdInProgressBit;
#define ClrMscCmdInProgress mhlTxConfig.mhlFlags &= ~mscCmdInProgressBit;


#ifdef SWWA_20005 //(

#define RxClockUnstable    ((mhlTxConfig.mhlFlags &   RxClockUnstableBit)?1:0)
#define SetRxClockUnstable   mhlTxConfig.mhlFlags |=  RxClockUnstableBit;
#define ClrRxClockUnstable   mhlTxConfig.mhlFlags &= ~RxClockUnstableBit;

#else //)(

#define SetRxClockUnstable   /* do nothing */
#define ClrRxClockUnstable   /* do nothing */

#endif //)
// bits for mhlHpdRSENflags:
typedef enum
{
     MHL_HPD            = 0x01
   , MHL_RSEN           = 0x02
}MhlHpdRSEN_e;


typedef enum
{
      FLAGS_SCRATCHPAD_BUSY         = 0x01
    , FLAGS_REQ_WRT_PENDING         = 0x02
    , FLAGS_WRITE_BURST_PENDING     = 0x04
    , FLAGS_RCP_READY               = 0x08
    , FLAGS_HAVE_DEV_CATEGORY       = 0x10
    , FLAGS_HAVE_DEV_FEATURE_FLAGS  = 0x20
    , FLAGS_SENT_DCAP_RDY           = 0x40
    , FLAGS_SENT_PATH_EN            = 0x80
}MiscFlags_e;

#define IncrementCBusReferenceCount(func) {mhlTxConfig.cbusReferenceCount++; TPI_DEBUG_PRINT((TPI_DEBUG_CHANNEL,"MhlTx:%s cbusReferenceCount:%d\n",#func,(int)mhlTxConfig.cbusReferenceCount)); }
#define DecrementCBusReferenceCount(func) {mhlTxConfig.cbusReferenceCount--; TPI_DEBUG_PRINT((TPI_DEBUG_CHANNEL,"MhlTx:%s cbusReferenceCount:%d\n",#func,(int)mhlTxConfig.cbusReferenceCount)); }

#define SetMiscFlag(func,x) { mhlTxConfig.miscFlags |=  (x); TPI_DEBUG_PRINT((TPI_DEBUG_CHANNEL,"MhlTx:%s set %s\n",(int)__LINE__,#func,#x)); }
#define ClrMiscFlag(func,x) { mhlTxConfig.miscFlags &= ~(x); TPI_DEBUG_PRINT((TPI_DEBUG_CHANNEL,"MhlTx:%s clr %s\n",(int)__LINE__,#func,#x)); }

extern mhlTx_config_t mhlTxConfig;

#else //)(

#define ResetDownStreamHotPlugDetectionStatus /* do nothing */ //dd review...

#define SetTclkStable    /* do nothing */
#define ClrTclkStable    /* do nothing */

#define SetCheckTclkStable /* do nothing */
#define ClrCheckTclkStable /* do nothing */

#define SetRxClockUnstable   /* do nothing */
#define ClrRxClockUnstable   /* do nothing */

#define hdmiCableConnected(x) ( HOT_PLUG_STATE & x )
#define downStreamHotPlugDetectionStatusChanged(x) ( HOT_PLUG_STATE & (x ^ g_InterruptStatusImage))

#endif //)

#endif //)


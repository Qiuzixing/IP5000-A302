/*
 *****************************************************************************
 *
 * Copyright 2002-2009,2010, Silicon Image, Inc.  All rights reserved.
 * No part of this work may be reproduced, modified, distributed, transmitted,
 * transcribed, or translated into any language or computer format, in any form
 * or by any means without written permission of: Silicon Image, Inc., 1060
 * East Arques Avenue, Sunnyvale, California 94085
 *****************************************************************************
 */
/*
 *****************************************************************************
 * @file  HDCP.c
 *
 * @brief Implementation of the HDCP.
 *
 *****************************************************************************
*/
//#include <stdio.h>
#include "defs.h"
#include "ast_utilities.h"
#include "si_basetypes.h"
#include "si_datatypes.h"
//#include "at89c51xd2.h"
#include "constants.h"
//#include "gpio.h"

#include "videomodedesc.h"
//#include "AMF_Lib.h"
#include "serialpacket.h"
#include "TPI_Regs.h"
#include "Externals.h"
#include "Macros.h"
#include "TPI_Access.h"
#include "TPI.h"
#include "HDCP.h"
//#include "delay.h"
#include "AV_Config.h"
#include "edid.h"
#include "TPI_generic.h"
#include "tpidebug.h"
//#include "hal_timers.h"

#if AST_HDMITX
#if SUPPORT_HDCP_REPEATER
void CRT_HDCP1_Downstream_Port_Auth_Status_Callback(unsigned char authenticated, unsigned char *Bksv, unsigned char Bcaps, unsigned short Bstatus, unsigned char *KSV_FIFO);
#endif
#endif


#ifdef MHL_CONNECTION_STATE_MACHINE //(
#include "statetable.h"
#endif //)

#define AKSV_SIZE              5
#define NUM_OF_ONES_IN_KSV    20


uint8_t HDCP_LinkProtectionLevel;

static uint8_t AreAKSV_OK(void);
static uint8_t IsHDCP_Supported(void);

#ifdef CHECKREVOCATIONLIST
static uint8_t CheckRevocationList(void);
#endif

#ifdef READKSV
//////////////////////////////////////////////////////////////////////////////
//
// FUNCTION     :   GetKSV()
//
// PURPOSE      :   Collect all downstrean KSV for verification
//
// INPUT PARAMS :   None
//
// OUTPUT PARAMS:   None
//
// GLOBALS USED :   KSV_Array[]
//
// RETURNS      :   TRUE if KSVs collected successfully. False if not.
//
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// The buffer is limited to KSV_ARRAY_SIZE due to the 8051 implementation.
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//////////////////////////////////////////////////////////////////////////////
static uint8_t GetKSV(void)
{
	uint8_t i;
    uint16_t KeyCount;

	HDCP_TRACE_PRINT((HDCP_TRACE_CHANNEL,">>GetKSV()\n"));
    ReadBlockHDCP(DDC_BSTATUS_ADDR_L, 1, &i);
    KeyCount = (i & DEVICE_COUNT_MASK) * 5;
	if (KeyCount != 0)
	{
        ReadBlockHDCPAndDiscard(DDC_KSV_FIFO_ADDR, KeyCount);
	}

	 return TRUE;
}
#endif

//////////////////////////////////////////////////////////////////////////////
//
// FUNCTION     :   IsRepeater()
//
// PURPOSE      :   Test if sink is a repeater
//
// INPUT PARAMS :   None
//
// OUTPUT PARAMS:   None
//
// GLOBALS USED :   None
//
// RETURNS      :   TRUE if sink is a repeater. FALSE if not.
//
//////////////////////////////////////////////////////////////////////////////
uint8_t IsRepeater(void)
{
    uint8_t RegImage;

	HDCP_TRACE_PRINT((HDCP_TRACE_CHANNEL,">>IsRepeater()\n"));

    RegImage = ReadByteTPI(PAGE_0_TPI_COPP_DATA1_ADDR);

    if (RegImage & HDCP_REPEATER_MASK)
        return TRUE;

    return FALSE;           // not a repeater
}



//////////////////////////////////////////////////////////////////////////////
//
// FUNCTION     :   HDCP_Init()
//
// PURPOSE      :   Tests Tx and Rx support of HDCP. If found, checks if
//                  and attempts to set the security level accordingly.
//
// INPUT PARAMS :   None
//
// OUTPUT PARAMS:   None
//
// GLOBALS USED :	HDCP_TxSupports - initialized to FALSE, set to TRUE if supported by this device
//					HDCP_AksvValid - initialized to FALSE, set to TRUE if valid AKSVs are read from this device
//					HDCP_Started - initialized to FALSE
//					HDCP_LinkProtectionLevel - initialized to (EXTENDED_LINK_PROTECTION_NONE | LOCAL_LINK_PROTECTION_NONE)
//
// RETURNS      :   None
//
//////////////////////////////////////////////////////////////////////////////

void HDCP_Init (void)
{

	HDCP_TRACE_PRINT((HDCP_TRACE_CHANNEL,">>HDCP_Init()\n"));

#if AST_HDMITX
	/* we'll set VideoModeDescription.HDCPAuthenticated once HDCP is enabled, so disable following code */
#if 0
	/* for HDCP procedures in TPI_Poll, initialize to authenticated */
	VideoModeDescription.HDCPAuthenticated = VMD_HDCP_AUTHENTICATED;
#endif
#endif

    ClrHDCP_TxSupports;
    ClrHDCP_AksvValid;
    ClrHDCP_Started;

	HDCP_LinkProtectionLevel = EXTENDED_LINK_PROTECTION_NONE | LOCAL_LINK_PROTECTION_NONE;

	// This is TX-related... need only be done once.
    if (!IsHDCP_Supported())
    {
		// The TX does not support HDCP, so authentication will never be attempted.
		// Video will be shown as soon as TMDS is enabled.
		HDCP_DEBUG_PRINT((HDCP_DEBUG_CHANNEL,"HDCP -> TX does not support HDCP\n"));
		return;
	}
	SetHDCP_TxSupports;

	// This is TX-related... need only be done once.
    if (!AreAKSV_OK())
    {
		// The TX supports HDCP, but does not have valid AKSVs.
		// Video will not be shown.
        HDCP_DEBUG_PRINT((HDCP_DEBUG_CHANNEL,"HDCP -> Illegal AKSV\n"));
        return;
    }

	SetHDCP_AksvValid;

	HDCP_DEBUG_PRINT((HDCP_DEBUG_CHANNEL,"HDCP -> Supported by TX, AKSVs valid\n"));
}

#if AST_HDMITX && SUPPORT_HDCP_REPEATER
int sink_support_hdcp(void)
{
	if (ReadByteTPI(PAGE_0_TPI_COPP_DATA1_ADDR) & PROTECTION_TYPE_MASK)
		return 1;

	return 0;
}
#endif

void HDCP_CheckStatus (uint8_t InterruptStatusImage)
{
	if (HDCP_TxSupports)
	{
		if (HDCP_AksvValid)
		{
			if (HDCP_LinkProtectionLevel == (EXTENDED_LINK_PROTECTION_NONE | LOCAL_LINK_PROTECTION_NONE))
			{
				if (!HDCP_Started)
				{
					uint8_t QueryData;
					QueryData = ReadByteTPI(PAGE_0_TPI_COPP_DATA1_ADDR);
					HDCP_DEBUG_PRINT((HDCP_DEBUG_CHANNEL,"HDCP QueryData:%02X\n",(int)QueryData));

					if (QueryData & PROTECTION_TYPE_MASK)   // Is HDCP available
					{
						HDCP_DEBUG_PRINT((HDCP_DEBUG_CHANNEL,"Sink Support HDCP 1.x\n"));
						HDCP_On();
					}

					if (VideoModeDescription.HDCPAuthenticated == VMD_HDCP_AUTHENTICATED) {
						/*
						 * with Dell moniter (model:2408WFPb),
						 * sometimes no SECURITY_CHANGE_EVENT interrupt after enable HDCP
						 * let CRT retry later here is to try to fix this situation
						 */
						CRT_HDCP1_Downstream_Port_Auth_Status_Callback(2, NULL, 0, 0, NULL);
					}
				}
			}

			// Check if Link Status has changed:
			if (InterruptStatusImage & SECURITY_CHANGE_EVENT)
			{
				uint8_t LinkStatus;

				LinkStatus = ReadByteTPI(PAGE_0_TPI_COPP_DATA1_ADDR);
				LinkStatus &= LINK_STATUS_MASK;

				ClearInterrupt(SECURITY_CHANGE_EVENT);

				switch (LinkStatus)
				{
					case LINK_STATUS_NORMAL:
						HDCP_DEBUG_PRINT((HDCP_DEBUG_CHANNEL,"HDCP: Link = Normal\n"));
						break;

					case LINK_STATUS_LINK_LOST:
						HDCP_DEBUG_PRINT((HDCP_DEBUG_CHANNEL,"HDCP: Link = Lost\n"));
						RestartHDCP();
						break;

					case LINK_STATUS_RENEGOTIATION_REQ:
						HDCP_DEBUG_PRINT((HDCP_DEBUG_CHANNEL,"HDCP: Link = Renegotiation Required 0x%02X\n",(int)InterruptStatusImage));
						HDCP_Off();
						HDCP_On();
						break;

					case LINK_STATUS_LINK_SUSPENDED:
						HDCP_DEBUG_PRINT((HDCP_DEBUG_CHANNEL,"HDCP: Link = Suspended\n"));
						HDCP_On();
						break;
				}
			}

			// Check if HDCP state has changed:
			if (InterruptStatusImage & HDCP_CHANGE_EVENT)
			{
				uint8_t RegImage;
				uint8_t NewLinkProtectionLevel;
				HDCP_DEBUG_PRINT((HDCP_DEBUG_CHANNEL,"HDCP_CHANGE_EVENT\n"));
				RegImage = ReadByteTPI(PAGE_0_TPI_COPP_DATA1_ADDR);

				NewLinkProtectionLevel = RegImage & (EXTENDED_LINK_PROTECTION_MASK | LOCAL_LINK_PROTECTION_MASK);
				if (NewLinkProtectionLevel != HDCP_LinkProtectionLevel)
				{
#if (AST_HDMITX && SUPPORT_HDCP_REPEATER)
					u32 authendicated = 0;
					u8 link_status = RegImage & LINK_STATUS_MASK;
#endif
					HDCP_LinkProtectionLevel = NewLinkProtectionLevel;

					switch (HDCP_LinkProtectionLevel)
					{
						case (EXTENDED_LINK_PROTECTION_NONE | LOCAL_LINK_PROTECTION_NONE):
							HDCP_DEBUG_PRINT((HDCP_DEBUG_CHANNEL,"HDCP -> Protection = None\n"));
							RestartHDCP();
#if (AST_HDMITX && SUPPORT_HDCP_REPEATER)
							if (LINK_STATUS_NORMAL == link_status)
								authendicated = 0; /* HDCP fail */
							else
								authendicated = 2; /* retry */
#endif
							break;

						case LOCAL_LINK_PROTECTION_SECURE:

							if (IsHDMI_Sink())
							{
								ReadModifyWriteTPI(PAGE_0_TPI_CONFIG3_ADDR, AUDIO_MUTE_MASK, AUDIO_MUTE_NORMAL);
							}

#ifdef MHL_CONNECTION_STATE_MACHINE //(
							PutNextTxEvent(txpseHDCPAuthenticated);
#else //)(
							ReadModifyWriteTPI(PAGE_0_TPI_SC_ADDR, AV_MUTE_MASK, AV_MUTE_NORMAL);
							HDCP_DEBUG_PRINT((HDCP_DEBUG_CHANNEL,"HDCP -> Protection = Local, Video Unmuted\n"));
							DelayMS(100);
#endif //)
#if (AST_HDMITX && SUPPORT_HDCP_REPEATER)
							authendicated = 1;
#endif
							break;

						case (EXTENDED_LINK_PROTECTION_SECURE | LOCAL_LINK_PROTECTION_SECURE):
							HDCP_DEBUG_PRINT((HDCP_DEBUG_CHANNEL,"HDCP -> HDCP -> Protection = Extended\n"));
							if (IsRepeater())
							{
#ifdef READKSV
								uint8_t RiCnt;
								RiCnt = ReadTxPage0Register( TXL_PAGE_0_HDCP_I_CNT_ADDR);
								while (RiCnt > 0x70)  // Frame 112
								{
									RiCnt = ReadTxPage0Register( TXL_PAGE_0_HDCP_I_CNT_ADDR);
								}
								ReadModifyWriteTPI(PAGE_0_TPI_SC_ADDR
								    , DDC_BUS_REQUEST_MASK      | DDC_BUS_GRANT_MASK
								    , DDC_BUS_REQUEST_REQUESTED | DDC_BUS_GRANT_GRANTED);
								GetKSV();
								RiCnt = ReadByteTPI(PAGE_0_TPI_SC_ADDR);
#endif
#ifdef MHL_CONNECTION_STATE_MACHINE //(
								PutNextTxEvent(txpseHDCPAuthenticated);
#else //)(
								ReadModifyWriteTPI(PAGE_0_TPI_SC_ADDR, AV_MUTE_MASK, AV_MUTE_NORMAL);
#endif //)
							}

#if (AST_HDMITX && SUPPORT_HDCP_REPEATER)
							authendicated = 1;
#endif
							break;

						default:
							HDCP_DEBUG_PRINT((HDCP_DEBUG_CHANNEL,"HDCP -> Protection = Extended but not Local?\n"));
							RestartHDCP();
#if (AST_HDMITX && SUPPORT_HDCP_REPEATER)
							authendicated = 2; /* retry */
#endif
							break;
					}
#if (AST_HDMITX && SUPPORT_HDCP_REPEATER)
					/* TODO non-legacy mode */
					/*
					 * VideoModeDescription.HDCPAuthenticated is VMD_HDCP_AUTHENTICATED means that HDCP is enabled by video/crt
					 *
					 * we use VideoModeDescription.HDCPAuthenticated to determine whether calling HDCP callback
					 * because a race condition between interrupt handler and hdcp1_auth() if we use notify flag
					 */
					if (VideoModeDescription.HDCPAuthenticated == VMD_HDCP_AUTHENTICATED)
						CRT_HDCP1_Downstream_Port_Auth_Status_Callback(authendicated, NULL, 0, 0, NULL);
#endif
				}

#ifdef KSVFORWARD
				// Check if KSV FIFO is ready and forward - Bug# 17892
				// If interrupt never goes off:
			 	//   a) KSV formwarding is not enabled
				//   b) not a repeater
				//   c) a repeater with device count == 0
				// and therefore no KSV list to forward
				if ((ReadByteTPI(TPI_KSV_FIFO_READY_INT) & KSV_FIFO_READY_MASK) == KSV_FIFO_READY_YES)
				{
					uint8_t ksv, ksvSaved[10];
					uint8_t i=0;

					ReadModifyWriteTPI(TPI_KSV_FIFO_READY_INT, KSV_FIFO_READY_MASK, KSV_FIFO_READY_YES);
					HDCP_DEBUG_PRINT((HDCP_DEBUG_CHANNEL,"KSV Fwd: KSV FIFO has data...\n"));

					// While !(last byte has been read from KSV FIFO)
					// if (count = 0) then a byte is not in the KSV FIFO yet, do not read
					// else read a byte from the KSV FIFO and forward it or keep it for revocation check
					do
					{
						ksv = ReadByteTPI(PAGE_0_TPI_KSV_FIFO_STAT_ADDR);		   // TPI reg 0x41
						if (ksv & KSV_FIFO_COUNT_MASK)
						{
							HDCP_DEBUG_PRINT((HDCP_DEBUG_CHANNEL,"KSV Fwd: KSV FIFO Count = %d, ", (int)(ksv & KSV_FIFO_COUNT_MASK)));
							ksv = ReadByteTPI(PAGE_0_TPI_KSV_FIFO_ADDR);	// TPI reg 0x42
							HDCP_DEBUG_PRINT((HDCP_DEBUG_CHANNEL,"Value = %d\n", (int)ksv));
							ksvSaved[i++] = ksv;	   // ksv list saved for revocation check
						}
						ksv = ReadByteTPI(PAGE_0_TPI_KSV_FIFO_STAT_ADDR);	 // Reload of ksv for TPI reg 0x41.

					} while ((ksv & KSV_FIFO_LAST_MASK) != KSV_FIFO_LAST_NO);
					HDCP_DEBUG_PRINT((HDCP_DEBUG_CHANNEL,"KSV Fwd: Last KSV FIFO forward complete\n"));

					// Prints saved ksv list.
					//for (i = 0; i < sizeof(ksvSaved); i++)
					//{
					// 	HDCP_DEBUG_PRINT((HDCP_DEBUG_CHANNEL,"Saved ksv list[%d] = %d\n", (int)i, (int)ksvSaved[i]));
					//}
				}
#endif
				ClearInterrupt(HDCP_CHANGE_EVENT);
			}
		}
	}
}

#if AST_HDMITX
void HDCP_CheckStatus_One(void)
{
	HDCP_CheckStatus(ReadByteTPI(PAGE_0_TPI_INTR_ST_ADDR));
}
#else
void HDCP_CheckStatus_One(void) {}
#endif

//////////////////////////////////////////////////////////////////////////////
//
// FUNCTION     :   IsHDCP_Supported()
//
// PURPOSE      :   Check Tx revision number to find if this Tx supports HDCP
//                  by reading the HDCP revision number from TPI register 0x30.
//
// INPUT PARAMS :   None
//
// OUTPUT PARAMS:   None
//
// GLOBALS USED :   None
//
// RETURNS      :   TRUE if Tx supports HDCP. FALSE if not.
//
//////////////////////////////////////////////////////////////////////////////

uint8_t IsHDCP_Supported(void)
{
    uint8_t HDCP_Rev;
	uint8_t HDCP_Supported;

	HDCP_TRACE_PRINT((HDCP_TRACE_CHANNEL,">>IsHDCP_Supported()\n"));

#ifdef DBG_HDCP_DISABLE
	return FALSE;
#endif

	HDCP_Supported = TRUE;

	// Check Device ID
    HDCP_Rev = ReadByteTPI(PAGE_0_TPI_HDCP_REV_ADDR);

	HDCP_DEBUG_PRINT((HDCP_DEBUG_CHANNEL,"HDCP_Rev:%02X\n",(int)HDCP_Rev));
    if (HDCP_Rev != (HDCP_MAJOR_REVISION_VALUE | HDCP_MINOR_REVISION_VALUE))
	{
    	HDCP_Supported = FALSE;
	}

#ifdef SiI_9022AYBT_DEVICEID_CHECK
	// Even if HDCP is supported check for incorrect Device ID
	HDCP_Rev = ReadByteTPI(PAGE_0_TPI_AKSV_1_ADDR);
	if (HDCP_Rev == 0x90)
	{
		HDCP_Rev = ReadByteTPI(PAGE_0_TPI_AKSV_2_ADDR);
		if (HDCP_Rev == 0x22)
		{
			HDCP_Rev = ReadByteTPI(PAGE_0_TPI_AKSV_3_ADDR);
			if (HDCP_Rev == 0xA0)
			{
				HDCP_Rev = ReadByteTPI(PAGE_0_TPI_AKSV_4_ADDR);
				if (HDCP_Rev == 0x00)
				{
					HDCP_Rev = ReadByteTPI(PAGE_0_TPI_AKSV_5_ADDR);
					if (HDCP_Rev == 0x00)
					{
						HDCP_Supported = FALSE;
					}
				}
			}
		}
	}
#endif
	return HDCP_Supported;
}

//////////////////////////////////////////////////////////////////////////////
//
// FUNCTION     :   HDCP_On()
//
// PURPOSE      :   Switch hdcp on.
//
// INPUT PARAMS :   None
//
// OUTPUT PARAMS:   None
//
// GLOBALS USED :   HDCP_Started set to TRUE
//
// RETURNS      :   None
//
//////////////////////////////////////////////////////////////////////////////

void HDCP_On(void)
{
	if (!HDCP_Override)
	{
		HDCP_DEBUG_PRINT((HDCP_DEBUG_CHANNEL,"HDCP -> Started\n"));

    	ReadModifyWriteTPI(PAGE_0_TPI_COPP_DATA2_ADDR, PROTECTION_LEVEL_MASK, PROTECTION_LEVEL_MAX); //WriteByteTPI(PAGE_0_TPI_COPP_DATA2_ADDR, PROTECTION_LEVEL_MAX);
#ifdef MHL_CONNECTION_STATE_MACHINE //)
        HalTimerSet(TIMER_HDCP,HDCP_WORKAROUND_INTERVAL);
#endif //)
		SetHDCP_Started
	}
	else
	{
		ClrHDCP_Started
	}
}

//////////////////////////////////////////////////////////////////////////////
//
// FUNCTION     :   HDCP_Off()
//
// PURPOSE      :   Switch hdcp off.
//
// INPUT PARAMS :   None
//
// OUTPUT PARAMS:   None
//
// GLOBALS USED :   HDCP_Started set to FALSE
//
// RETURNS      :   None
//
//////////////////////////////////////////////////////////////////////////////

void HDCP_Off(void)
{
	//HDCP_TRACE_PRINT((HDCP_TRACE_CHANNEL,">>HDCP_Off()\n"));
	HDCP_DEBUG_PRINT((HDCP_DEBUG_CHANNEL, "HDCP -> Stop, AV Mute\n"));

	// AV MUTE
	ReadModifyWriteTPI(PAGE_0_TPI_SC_ADDR, AV_MUTE_MASK, AV_MUTE_MUTED);
	ReadModifyWriteTPI(PAGE_0_TPI_COPP_DATA2_ADDR, PROTECTION_LEVEL_MASK, PROTECTION_LEVEL_MIN); //WriteByteTPI(PAGE_0_TPI_COPP_DATA2_ADDR, PROTECTION_LEVEL_MIN);
#ifdef MHL_CONNECTION_STATE_MACHINE //(
    PutNextTxEvent(txpseHDCPDeAuthenticated);
#endif //)
	ClrHDCP_Started;
	HDCP_LinkProtectionLevel = EXTENDED_LINK_PROTECTION_NONE | LOCAL_LINK_PROTECTION_NONE;
}

void HDCP_Off_without_avmute(void)
{
	//HDCP_TRACE_PRINT((HDCP_TRACE_CHANNEL,">>HDCP_Off(), without AV Mute\n"));
	HDCP_DEBUG_PRINT((HDCP_DEBUG_CHANNEL, "HDCP -> Stop, no AV Mute\n"));
	ReadModifyWriteTPI(PAGE_0_TPI_COPP_DATA2_ADDR, PROTECTION_LEVEL_MASK, PROTECTION_LEVEL_MIN);
	ClrHDCP_Started;
	HDCP_LinkProtectionLevel = EXTENDED_LINK_PROTECTION_NONE | LOCAL_LINK_PROTECTION_NONE;
}

u32 link_protection_level(void)
{
	return TPI_PAGE_0_REG_COPP_LPROT_RD(ReadByteTPI(PAGE_0_TPI_COPP_DATA1_ADDR));
}

//////////////////////////////////////////////////////////////////////////////
//
// FUNCTION      :  AreAKSV_OK()
//
// PURPOSE       :  Check if AKSVs contain 20 '0' and 20 '1'
//
// INPUT PARAMS  :  None
//
// OUTPUT PARAMS :  None
//
// GLOBALS USED  :  TBD
//
// RETURNS       :  TRUE if 20 zeros and 20 ones found in AKSV. FALSE OTHERWISE
//
//////////////////////////////////////////////////////////////////////////////
static uint8_t AreAKSV_OK(void)
{
    uint8_t B_Data[AKSV_SIZE];
    uint8_t NumOfOnes = 0;

    uint8_t i;
    uint8_t j;

	HDCP_TRACE_PRINT((HDCP_TRACE_CHANNEL,">>AreAKSV_OK()\n"));

    ReadBlockTPI(PAGE_0_TPI_AKSV_1_ADDR, AKSV_SIZE, B_Data);

    for (i=0; i < AKSV_SIZE; i++)
    {
        for (j=0; j < BYTE_SIZE; j++)
        {
            if (B_Data[i] & 0x01)
            {
                NumOfOnes++;
            }
            B_Data[i] >>= 1;
        }
     }
     if (NumOfOnes != NUM_OF_ONES_IN_KSV)
        return FALSE;

    return TRUE;
}


//////////////////////////////////////////////////////////////////////////////
//
// FUNCTION      :  CheckRevocationList()
//
// PURPOSE       :  Compare KSVs to those included in a revocation list
//
// INPUT PARAMS  :  None
//
// OUTPUT PARAMS :  None
//
// GLOBALS USED  :  TBD
//
// RETURNS       :  TRUE if no illegal KSV found in BKSV list
//
// NOTE			 :	Currently this function is implemented as a place holder only
//
//////////////////////////////////////////////////////////////////////////////
#ifdef CHECKREVOCATIONLIST
static uint8_t CheckRevocationList(void)
{
	HDCP_TRACE_PRINT((HDCP_TRACE_CHANNEL,">>CheckRevocationList()\n"));
    return TRUE;
}
#endif


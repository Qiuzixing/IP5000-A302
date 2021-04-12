///*****************************************
//  Copyright (C) 2009-2019
//  ITE Tech. Inc. All Rights Reserved
//  Proprietary and Confidential
///*****************************************
//   @file   <iTE6805_CEC_SYS.c>
//   @author Kuro.Chung@ite.com.tw
//   @date   2019/03/05
//   @fileversion: iTE6805_MCUSRC_1.31
//******************************************/
#include "iTE6805_Global.h"
#include "iTE6805_CEC_SYS.h"
#include "iTE6805_CEC_DRV.h"
#include "iTE6805_CEC_FETURE.h"

#if defined(AST_HDMIRX)
#include "ast_it6805.h"
//#include <linux/platform_device.h>
#endif

#if (_ENABLE_IT6805_CEC_==TRUE)

#if defined(AST_HDMIRX)
void cec_topology_fsm(void);
#endif

#define iTE6805_CEC_Logical_Address CEC_LOG_ADDR_TV

extern _iTE6805_DATA	iTE6805_DATA;
extern iTE_u8			CEC_timeunit;

#define Max_TX_Retry_Count 5
iTE_u8 Current_TX_Retry_Count = 0;

// In 6805 only need is iTE6805_CECRX
//_iTE6805_CEC iTE6805_CEC_RX[1], iTE6805_CEC_TX[1];
_iTE6805_CEC iTE6805_CEC_RX[1];
_iTE6805_CEC *iTE6805_CEC;


/*
 * Specify minimum length and whether the message is directed, broadcast
 * or both. Messages that do not match the criteria are ignored as per
 * the CEC specification.
 */
_CODE iTE_u8 cec_msg_size[256] = {
0x84 ,0x00 ,0x00 ,0x00 ,0x82 ,0x82 ,0x82 ,0x87 ,0x83 ,0x83 ,0x83 ,0x82 ,0x00 ,0x82 ,0x00 ,0x82 ,
0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x83 ,0x83 ,0x00 ,0x00 ,0x00 ,0x00 ,
0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,
0x00 ,0x00 ,0x65 ,0x8D ,0x8D ,0x83 ,0xE2 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,
0x00 ,0x83 ,0x83 ,0x83 ,0x83 ,0x82 ,0x82 ,0x82 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,
0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,
0x00 ,0x00 ,0x00 ,0x00 ,0x83 ,0x00 ,0x00 ,0x82 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,
0x82 ,0x82 ,0xE3 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x83 ,0x00 ,0x00 ,0x82 ,0x83 ,0x00 ,
0x66 ,0x64 ,0x64 ,0x82 ,0x65 ,0x62 ,0x64 ,0x65 ,0x00 ,0x82 ,0xE2 ,0xE2 ,0x82 ,0x83 ,0x83 ,0x82 ,
0xA3 ,0x82 ,0x86 ,0x89 ,0x00 ,0x00 ,0x00 ,0x90 ,0x00 ,0x90 ,0x83 ,0x00 ,0x00 ,0x84 ,0x83 ,0x82 ,
0xE5 ,0x8D ,0x8D ,0x82 ,0x82 ,0x82 ,0x66 ,0x64 ,0x67 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,
0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,
0x82 ,0x82 ,0x82 ,0x82 ,0x82 ,0x82 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,
0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,
0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,
0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x62 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x82 ,

};

/*
saving space for global stack prevent stack overflow in MCU-like platefoom
keep this function for gen cec_msg_size by using print
void iTE6805_INIT_MSG_CHECK_TABLE(void)
{
	iTE_u16 i = 0;
	cec_msg_size[CEC_MSG_ACTIVE_SOURCE] = 4 | BCAST;
	cec_msg_size[CEC_MSG_IMAGE_VIEW_ON] = 2 | DIRECTED;
	cec_msg_size[CEC_MSG_TEXT_VIEW_ON] = 2 | DIRECTED;
	cec_msg_size[CEC_MSG_INACTIVE_SOURCE] = 4 | DIRECTED;
	cec_msg_size[CEC_MSG_REQUEST_ACTIVE_SOURCE] = 2 | BCAST;
	cec_msg_size[CEC_MSG_ROUTING_CHANGE] = 6 | BCAST;
	cec_msg_size[CEC_MSG_ROUTING_INFORMATION] = 4 | BCAST;
	cec_msg_size[CEC_MSG_SET_STREAM_PATH] = 4 | BCAST;
	cec_msg_size[CEC_MSG_STANDBY] = 2 | BOTH;
	cec_msg_size[CEC_MSG_RECORD_OFF] = 2 | DIRECTED;
	cec_msg_size[CEC_MSG_RECORD_ON] = 3 | DIRECTED;
	cec_msg_size[CEC_MSG_RECORD_STATUS] = 3 | DIRECTED;
	cec_msg_size[CEC_MSG_RECORD_TV_SCREEN] = 2 | DIRECTED;
	cec_msg_size[CEC_MSG_CLEAR_ANALOGUE_TIMER] = 13 | DIRECTED;
	cec_msg_size[CEC_MSG_CLEAR_DIGITAL_TIMER] = 16 | DIRECTED;
	cec_msg_size[CEC_MSG_CLEAR_EXT_TIMER] = 13 | DIRECTED;
	cec_msg_size[CEC_MSG_SET_ANALOGUE_TIMER] = 13 | DIRECTED;
	cec_msg_size[CEC_MSG_SET_DIGITAL_TIMER] = 16 | DIRECTED;
	cec_msg_size[CEC_MSG_SET_EXT_TIMER] = 13 | DIRECTED;
	cec_msg_size[CEC_MSG_SET_TIMER_PROGRAM_TITLE] = 2 | DIRECTED;
	cec_msg_size[CEC_MSG_TIMER_CLEARED_STATUS] = 3 | DIRECTED;
	cec_msg_size[CEC_MSG_TIMER_STATUS] = 3 | DIRECTED;
	cec_msg_size[CEC_MSG_CEC_VERSION] = 3 | DIRECTED;
	cec_msg_size[CEC_MSG_GET_CEC_VERSION] = 2 | DIRECTED;
	cec_msg_size[CEC_MSG_GIVE_PHYSICAL_ADDR] = 2 | DIRECTED;
	cec_msg_size[CEC_MSG_GET_MENU_LANGUAGE] = 2 | DIRECTED;
	cec_msg_size[CEC_MSG_REPORT_PHYSICAL_ADDR] = 5 | BCAST;
	cec_msg_size[CEC_MSG_SET_MENU_LANGUAGE] = 5 | BCAST;
	cec_msg_size[CEC_MSG_REPORT_FEATURES] = 6 | BCAST;
	cec_msg_size[CEC_MSG_GIVE_FEATURES] = 2 | DIRECTED;
	cec_msg_size[CEC_MSG_DECK_CONTROL] = 3 | DIRECTED;
	cec_msg_size[CEC_MSG_DECK_STATUS] = 3 | DIRECTED;
	cec_msg_size[CEC_MSG_GIVE_DECK_STATUS] = 3 | DIRECTED;
	cec_msg_size[CEC_MSG_PLAY] = 3 | DIRECTED;
	cec_msg_size[CEC_MSG_GIVE_TUNER_DEVICE_STATUS] = 3 | DIRECTED;
	cec_msg_size[CEC_MSG_SELECT_ANALOGUE_SERVICE] = 6 | DIRECTED;
	cec_msg_size[CEC_MSG_SELECT_DIGITAL_SERVICE] = 9 | DIRECTED;
	cec_msg_size[CEC_MSG_TUNER_DEVICE_STATUS] = 7 | DIRECTED;
	cec_msg_size[CEC_MSG_TUNER_STEP_DECREMENT] = 2 | DIRECTED;
	cec_msg_size[CEC_MSG_TUNER_STEP_INCREMENT] = 2 | DIRECTED;
	cec_msg_size[CEC_MSG_DEVICE_VENDOR_ID] = 5 | BCAST;
	cec_msg_size[CEC_MSG_GIVE_DEVICE_VENDOR_ID] = 2 | DIRECTED;
	cec_msg_size[CEC_MSG_VENDOR_COMMAND] = 2 | DIRECTED;
	cec_msg_size[CEC_MSG_VENDOR_COMMAND_WITH_ID] = 5 | BOTH;
	cec_msg_size[CEC_MSG_VENDOR_REMOTE_BUTTON_DOWN] = 2 | BOTH;
	cec_msg_size[CEC_MSG_VENDOR_REMOTE_BUTTON_UP] = 2 | BOTH;
	cec_msg_size[CEC_MSG_SET_OSD_STRING] = 3 | DIRECTED;
	cec_msg_size[CEC_MSG_GIVE_OSD_NAME] = 2 | DIRECTED;
	cec_msg_size[CEC_MSG_SET_OSD_NAME] = 2 | DIRECTED;
	cec_msg_size[CEC_MSG_MENU_REQUEST] = 3 | DIRECTED;
	cec_msg_size[CEC_MSG_MENU_STATUS] = 3 | DIRECTED;
	cec_msg_size[CEC_MSG_USER_CONTROL_PRESSED] = 3 | DIRECTED;
	cec_msg_size[CEC_MSG_USER_CONTROL_RELEASED] = 2 | DIRECTED;
	cec_msg_size[CEC_MSG_GIVE_DEVICE_POWER_STATUS] = 2 | DIRECTED;
	cec_msg_size[CEC_MSG_REPORT_POWER_STATUS] = 3 | DIRECTED | BCAST2_0;
	cec_msg_size[CEC_MSG_FEATURE_ABORT] = 4 | DIRECTED;
	cec_msg_size[CEC_MSG_ABORT] = 2 | DIRECTED;
	cec_msg_size[CEC_MSG_GIVE_AUDIO_STATUS] = 2 | DIRECTED;
	cec_msg_size[CEC_MSG_GIVE_SYSTEM_AUDIO_MODE_STATUS] = 2 | DIRECTED;
	cec_msg_size[CEC_MSG_REPORT_AUDIO_STATUS] = 3 | DIRECTED;
	cec_msg_size[CEC_MSG_REPORT_SHORT_AUDIO_DESCRIPTOR] = 2 | DIRECTED;
	cec_msg_size[CEC_MSG_REQUEST_SHORT_AUDIO_DESCRIPTOR] = 2 | DIRECTED;
	cec_msg_size[CEC_MSG_SET_SYSTEM_AUDIO_MODE] = 3 | BOTH;
	cec_msg_size[CEC_MSG_SYSTEM_AUDIO_MODE_REQUEST] = 2 | DIRECTED;
	cec_msg_size[CEC_MSG_SYSTEM_AUDIO_MODE_STATUS] = 3 | DIRECTED;
	cec_msg_size[CEC_MSG_SET_AUDIO_RATE] = 3 | DIRECTED;
	cec_msg_size[CEC_MSG_INITIATE_ARC] = 2 | DIRECTED;
	cec_msg_size[CEC_MSG_REPORT_ARC_INITIATED] = 2 | DIRECTED;
	cec_msg_size[CEC_MSG_REPORT_ARC_TERMINATED] = 2 | DIRECTED;
	cec_msg_size[CEC_MSG_REQUEST_ARC_INITIATION] = 2 | DIRECTED;
	cec_msg_size[CEC_MSG_REQUEST_ARC_TERMINATION] = 2 | DIRECTED;
	cec_msg_size[CEC_MSG_TERMINATE_ARC] = 2 | DIRECTED;
	cec_msg_size[CEC_MSG_REQUEST_CURRENT_LATENCY] = 4 | BCAST;
	cec_msg_size[CEC_MSG_REPORT_CURRENT_LATENCY] = 7 | BCAST;
	cec_msg_size[CEC_MSG_CDC_MESSAGE] = 2 | BCAST;
	printf("\n\n\n");
	for(i = 0 ; i <= 0xFF ; i++)
	{
		if(i%16 == 0){printf("\n");}
		printf("0x%02X ,", (int)cec_msg_size[i]);
	}
	printf("\n\n\n");
}
*/

#if defined(AST_HDMIRX)
u8 old_dest;
u8 ping_wait, polling_wait;

CEC_Device CECList[15];

#define TX_HP_Q_SIZE 8 /* must power of 2 */
#define TX_Q_SIZE 4 /* must power of 2 */

CEC_FRAME tx_hp_queue[TX_HP_Q_SIZE], tx_queue[TX_Q_SIZE];
static u32 tx_hp_head = 0, tx_hp_tail = 0; 
static u32 tx_head = 0, tx_tail = 0; 

static void tx_hp_q_init(void)
{
        tx_hp_head = 0; 
        tx_hp_tail = 0; 
}

static void tx_hp_q_enqueue(u8 *buf, u32 size)
{
	tx_hp_queue[tx_hp_tail].id.CMD_SIZE = size;
	memcpy(tx_hp_queue[tx_hp_tail].CEC_ARY + 1, buf, size);

	tx_hp_tail = (tx_hp_tail + 1) & (TX_HP_Q_SIZE - 1);
}

static pCEC_FRAME tx_hp_q_dequeue(void)
{
	u32 index;

	index = tx_hp_head;

	tx_hp_head = (tx_hp_head + 1) & (TX_HP_Q_SIZE - 1);

	return &tx_hp_queue[index];
}

static u32 tx_hp_q_full(void)
{
	if (tx_hp_head == ((tx_hp_tail + 1) & (TX_HP_Q_SIZE - 1)))
		return TRUE;

	return FALSE;
}

static u32 tx_hp_q_empty(void)
{
	if (tx_hp_head == tx_hp_tail)
		return TRUE;

	return FALSE;
}

static u32 tx_hp_q_addq(u8 *buf, u32 size)
{
	if (tx_hp_q_full() == TRUE)
		return FALSE;

	tx_hp_q_enqueue(buf, size);

	return TRUE;
}

static void tx_q_init(void)
{
	tx_head = 0;
	tx_tail = 0;
}

static void tx_q_enqueue(u8 *buf, u32 size)
{
	tx_queue[tx_tail].id.CMD_SIZE = size;
	memcpy(tx_queue[tx_tail].CEC_ARY + 1, buf, size);

	tx_tail = (tx_tail + 1) & (TX_Q_SIZE - 1);
}

static pCEC_FRAME tx_q_dequeue(void)
{
	u32 index;

	index = tx_head;

	tx_head = (tx_head + 1) & (TX_Q_SIZE - 1);

	return &tx_queue[index];
}

static u32 tx_q_full(void)
{
	if (tx_head == ((tx_tail + 1) & (TX_Q_SIZE - 1)))
		return TRUE;

	return FALSE;
}

static u32 tx_q_empty(void)
{
	if (tx_head == tx_tail)
		return TRUE;

	return FALSE;
}

static u32 tx_q_addq(u8 *buf, u32 size)
{
	if (tx_q_full() == TRUE)
		return FALSE;

	tx_q_enqueue(buf, size);

	return TRUE;
}

void it680x_cec_tx_en_q(u8 *buf, u32 size)
{
	tx_hp_q_addq(buf, size);
}

void it680x_cec_tx_en_q_lp(u8 *buf, u32 size)
{
        tx_q_addq(buf, size);
}
#endif /* #if defined(AST_HDMIRX) */

void	iTE6805_hdmirx_CEC_irq(void)
{
	//0x4C
	//[5] TX Fail
	//[4] RX Done
	//[3] TX Done
	//[2] RX Fail
	//[1] RX Int : CEC follower received byte
	//[0] TX Int : CEC initiator output byte

	iTE_u8 Reg4Ch;
	Reg4Ch = cecrd(0x4C); cecwr(0x4C, Reg4Ch);

#if defined(AST_HDMIRX)
	if ((Reg4Ch & 0xFC) == 0) {
		iTE6805_CEC_chg(STATECEC_None);
		goto cec_irq_exit;
	}
#endif

	if (Reg4Ch & BIT5) // TX INT
	{
		CEC_DEBUG_PRINTF(("TX Fail INT!\n"));
		iTE6805_CEC_chg(STATECEC_TXFail);
	}

	if (Reg4Ch & BIT3)
	{
		CEC_DEBUG_PRINTF(("TX Done INT!\n"));
		iTE6805_CEC_chg(STATECEC_TXDone);
	}

	if (Reg4Ch & (BIT4|BIT2)) // RX Done
	{
		CEC_DEBUG_PRINTF(("RX Done INT!\n"));
		iTE6805_CEC_chg(STATECEC_RXCheck);
	}

	if (Reg4Ch & 0xC0)
	{
		// DataOvFlw_Int or CmdOvFlw_Int
		CEC_DEBUG_PRINTF(("INT DataOvFlw_Int or CmdOvFlw_Int need to iTE6805_CEC_Reset_RX_FIFO \n"));
		iTE6805_CEC_Reset_RX_FIFO();
	}

#if defined(AST_HDMIRX)
cec_irq_exit:
	if (cec_tx_avaiable()) {
		CEC_FRAME *pframe = NULL;
		if (tx_hp_q_empty() != TRUE) {
			pframe = tx_hp_q_dequeue();

		} else if (tx_q_empty() != TRUE) {
			pframe = tx_q_dequeue();
		}

		if (pframe) {
			memcpy(&iTE6805_CEC->CEC_FRAME_TX, pframe, pframe->id.CMD_SIZE + 1);
			iTE6805_CEC_CMD_Ready_To_Fire();
		}
	}

	cec_topology_fsm();
#endif
}

void	iTE6805_CEC_fsm(void)
{
	switch(iTE6805_DATA.STATECEC)
	{
		case STATECEC_TXFail:
			break;
		case STATECEC_TXDone:
			break;
		case STATECEC_RXCheck:
			break;
		default:
			break;
	}
}

#if defined(AST_HDMIRX)
void ast_it6805_cec_rx_handle(_iTE6805_CEC *cec)
{
	switch (cec->CEC_FRAME_RX.id.OPCODE) {
	case CEC_MSG_REPORT_PHYSICAL_ADDR:
		if ((cec->CEC_FRAME_RX.id.HEADER & 0x0F) == 0x0F) {
			u8 uc = ((cec->CEC_FRAME_RX.id.HEADER & 0xf0) >> 4);

			CECList[uc].PhyicalAddr1 = cec->CEC_FRAME_RX.id.OPERAND1;
			CECList[uc].PhyicalAddr2 = cec->CEC_FRAME_RX.id.OPERAND2;
			CECList[uc].Active = TRUE;

			if (polling_wait == uc)
				polling_wait = 0xF;
		}
		break;
	default:
		break;
	}

	cecif_rx(cec->CEC_FRAME_RX.CEC_ARY + 1, cec->CEC_FRAME_RX.id.CMD_SIZE);
}
#endif

void	iTE6805_CEC_chg(STATECEC_Type NewState)
{
	iTE6805_DATA.STATECEC = NewState;
#if defined(AST_HDMIRX)
	u8 tx_retry = 0, list_idx;
#endif
	switch(NewState)
	{
		case STATECEC_TXFail:
			CEC_DEBUG_PRINTF(("CEC STATE change to STATECEC_TXFail \n"));
			if(!iTE6805_CEC_Check_IsPollingMsg())
			{
				// If is not polling, need to retry
				if(iTE6805_CEC_Get_TxFailStatus() == TX_FAIL_RECEIVE_NACK)
				{
					if( (Current_TX_Retry_Count++ < Max_TX_Retry_Count) && iTE6805_CEC_Check_Fire_Ready())
					{
#if defined(AST_HDMIRX)
						tx_retry = 1;
#else
						iTE6805_CEC_Fire_Tx();
#endif
					}
				}
#if defined(AST_HDMIRX)
			} else {
				list_idx = iTE6805_CEC->CEC_FRAME_TX.id.HEADER & 0xF;
				CECList[list_idx].Active = FALSE;
				if (ping_wait == list_idx)
					ping_wait = 0xF;
#endif
			}

#if defined(AST_HDMIRX)
			if (tx_retry) {
				iTE6805_CEC_Fire_Tx();
			} else {
				cec_tx_done();
			}
#endif
			break;
		case STATECEC_TXDone:
			CEC_DEBUG_PRINTF(("CEC STATE change to STATECEC_TXDone \n"));
			// TX Done only we need is check Polling msg
			if(iTE6805_CEC_Check_IsPollingMsg())
			{
				if(iTE6805_CEC_Get_TxFailStatus() == TX_FAIL_RECEIVE_ACK)
				{
					// Polling should not get ACK, need to do change MY_LA
					// but 6805 is sink (CEC_LOG_ADDR_TV), do not need to do anything
					// polling will trigger TX Fail, so in Reg4D, 4E, 4F will 100% get not-polling-message.
					// polling message size = 1 only header, no opCode, no Operand
				}
#if defined(AST_HDMIRX)
				list_idx = iTE6805_CEC->CEC_FRAME_TX.id.HEADER & 0xF;
				CECList[list_idx].Active = TRUE;
				if (ping_wait == list_idx)
					ping_wait = 0xF;
#endif
			}
#if defined(AST_HDMIRX)
			cec_tx_done();
#endif
			break;

		case STATECEC_RXCheck:
			CEC_DEBUG_PRINTF(("CEC STATE change to STATECEC_RXCheck \n"));

			// Push all Hardware get to SW Queue
			iTE6805_CEC_CMD_Push_To_Queue_Handler();

			while(iTE6805_CEC_RX_CMD_Pull()) // Get One Command
			{
				iTE6805_CEC_CMD_Print(&iTE6805_CEC->CEC_FRAME_RX);

#if defined(AST_HDMIRX)
				ast_it6805_cec_rx_handle(iTE6805_CEC);
#else
				// Check Command Valid or not
				if(!iTE6805_CEC_CMD_Check(&iTE6805_CEC->CEC_FRAME_RX))
				{
					continue;
				}

				// Command valid, start feature decode
				iTE6805_CEC_CMD_Feature_decode(&iTE6805_CEC->CEC_FRAME_RX);
#endif
			}

			// decode done, Reset CMD Buffer
			iTE6805_CEC->Rx_Queue.Wptr = iTE6805_CEC->Rx_Queue.Rptr = 0;

			break;

		default:
			break;
	}
}

void iTE6805_CEC_CMD_Ready_To_Fire(void)
{
	CEC_DEBUG_PRINTF(("CEC STATE change to STATECEC_ReadyToFire \n"));
	iTE6805_CEC_CMD_Print(&iTE6805_CEC->CEC_FRAME_TX);
	cecwr(0x23, iTE6805_CEC->CEC_FRAME_TX.id.CMD_SIZE); // write size to register
	cecbwr(0x10, iTE6805_CEC->CEC_FRAME_TX.id.CMD_SIZE, &iTE6805_CEC->CEC_FRAME_TX.CEC_ARY[1]); // write CMD to register
	Current_TX_Retry_Count=0;
	iTE6805_CEC_Fire_Tx(); // fire
}

void iTE6805_CEC_CMD_Push_To_Queue_Handler(void)
{
	iTE_u8	pu8Reg51[3], RxCMDStage;
	piTE_u8	pu8RxHeader = iTE6805_CEC->Rx_TmpHeader;

	cecbrd(0x51, 0x03, pu8Reg51);
	CEC_DEBUG_PRINTF(("0x51,0x52,0x53 [%02X, %02X, %02X]\n",(int) pu8Reg51[0],(int) pu8Reg51[1],(int) pu8Reg51[2]));
	RxCMDStage = pu8Reg51[1] & 0x0F;

	do{
		if(pu8Reg51[0] & 0xCC)
		{
			// t FIFO overflow
			CEC_DEBUG_PRINTF(("**CecRx_FIFORESET\n"));
			iTE6805_CEC_Reset_RX_FIFO();
			return ;
		}
		else
		{
			// It may have Header and opcode in
			if(pu8RxHeader[0])
			{
				if(FALSE == iTE6805_CEC_RX_CMD_Push(pu8RxHeader))
				{
					return ;
				}
			}

			// Get All CMD in stage
			while(RxCMDStage--)
			{
				cecbrd(0x4D, 0x03, pu8RxHeader);	// Read 4D --> Header, OpCode, In_Cnd
				CEC_DEBUG_PRINTF(("0x%02X, %02X, %02X\n",(int) pu8RxHeader[0],(int) pu8RxHeader[1],(int) pu8RxHeader[2]));
				if(FALSE == iTE6805_CEC_RX_CMD_Push(pu8RxHeader))
				{
					CEC_DEBUG_PRINTF(("iTE6805_CEC_RX_CMD_Push Fail!\n"));
					return ;
				}
			}

			// After get all command in stage, it may get CMD again
			cecbrd(0x51, 0x02, pu8Reg51);
			RxCMDStage = pu8Reg51[1] & 0x0F;
		}
	}while(RxCMDStage);

}

void	iTE6805_CEC_CMD_Print(pCEC_FRAME CEC_FRAME)
{
	CEC_DEBUG_PRINTF(("\n"));
	CEC_DEBUG_PRINTF(("CEC CMD CMD_SIZE		=0x%02X \n"	,(int) CEC_FRAME->id.CMD_SIZE));
	CEC_DEBUG_PRINTF(("CEC CMD HEADER	=0x%02X \n"	,(int) CEC_FRAME->id.HEADER));
	CEC_DEBUG_PRINTF(("CEC CMD OPCODE	=0x%02X \n"	,(int) CEC_FRAME->id.OPCODE));
	CEC_DEBUG_PRINTF(("CEC CMD OPERAND1 =0x%02X \n"	,(int) CEC_FRAME->id.OPERAND1));
	CEC_DEBUG_PRINTF(("CEC CMD OPERAND2 =0x%02X \n"	,(int) CEC_FRAME->id.OPERAND2));
	CEC_DEBUG_PRINTF(("CEC CMD OPERAND3 =0x%02X \n"	,(int) CEC_FRAME->id.OPERAND3));
	CEC_DEBUG_PRINTF(("CEC CMD OPERAND4 =0x%02X \n"	,(int) CEC_FRAME->id.OPERAND4));
	CEC_DEBUG_PRINTF(("CEC CMD OPERAND5 =0x%02X \n"	,(int) CEC_FRAME->id.OPERAND5));
	CEC_DEBUG_PRINTF(("\n"));
}

#define CEC_MAX_MSG_SIZE        16
iTE_u8	iTE6805_CEC_CMD_Check(pCEC_FRAME CEC_FRAME)
{
	iTE_u8 Flag_Return = TRUE;
	iTE_u8 Length_Min = cec_msg_size[CEC_FRAME->id.OPCODE] & 0x1F;
	iTE_u8 IS_BOARD_CAST = FALSE;

	if (CEC_FRAME->id.Follower == CEC_LOG_ADDR_BROADCAST)
	{
		IS_BOARD_CAST = TRUE;
	}
	// Check Size, CEC_FRAME->id.CMD_SIZE <= 1 is for polling msg
	if(CEC_FRAME->id.CMD_SIZE <= 1 || CEC_FRAME->id.CMD_SIZE > CEC_MAX_MSG_SIZE || CEC_FRAME->id.CMD_SIZE < Length_Min)
	{
		CEC_DEBUG_PRINTF(("CEC CMD Check Size Fail!\n"));
		Flag_Return = FALSE;
	}

	// I'm the Initiator but I get the command
	if(CEC_FRAME->id.Initiator != CEC_LOG_ADDR_UNREGISTERED && iTE6805_CEC->MY_LA == CEC_FRAME->id.Initiator)
	{
		CEC_DEBUG_PRINTF(("CEC I'm the Initiator but I get the command!\n"));
		Flag_Return = FALSE;
	}

	/* Check if this message was for us (directed or broadcast). */
	if(IS_BOARD_CAST != TRUE && iTE6805_CEC->MY_LA != CEC_FRAME->id.Follower)
	{
		CEC_DEBUG_PRINTF(("CEC Check if this message was for us (directed or broadcast) Fail!\n"));
		Flag_Return = FALSE;
	}

	/*
	* Check if the length is not too short or if the message is a
	* broadcast message where a directed message was expected or
	* vice versa. If so, then the message has to be ignored (according
	* to section CEC 7.3 and CEC 12.2).
	*/
	if(cec_msg_size[CEC_FRAME->id.OPCODE])
	{
		if(!IS_BOARD_CAST && !(cec_msg_size[CEC_FRAME->id.OPCODE] & DIRECTED))
		{
			CEC_DEBUG_PRINTF(("CEC Check Direction Fail (Need to be Boardcast)!\n"));
			Flag_Return = FALSE;
		}
		else if(IS_BOARD_CAST && !(cec_msg_size[CEC_FRAME->id.OPCODE] & BCAST))
		{
			CEC_DEBUG_PRINTF(("CEC Check Direction Fail! (Need to be Directed)!\n"));
			Flag_Return = FALSE;
		}
	}

	if(Flag_Return == FALSE)
	{
		CEC_DEBUG_PRINTF((" CEC iTE6805_CEC_CMD_Check Fail! \n"));
		// check fail need to ignore this CEC MSG
		return FALSE;
	}

	return TRUE;
}

void iTE6805_CEC_INIT(void)
{

	iTE6805_DATA.STATECEC = STATECEC_None;

	iTE6805_CEC_RX->MY_LA = iTE6805_CEC_Logical_Address;
	iTE6805_CEC_RX->MY_PA_HIGH = 0x00;
	iTE6805_CEC_RX->MY_PA_LOW = 0x00; // ROOT TV PA FROM HIGH TO LOW = 0.0.0.0
	iTE6805_CEC_RX->Rx_Queue.Rptr = 0;
	iTE6805_CEC_RX->Rx_Queue.Wptr = 0;
	iTE6805_CEC_RX->Tx_Queue.Rptr = 0;
	iTE6805_CEC_RX->Tx_Queue.Wptr = 0;
	iTE6805_CEC_RX->Rx_TmpHeader[0] = 0;
	iTE6805_CEC_RX->Tx_QueueFull = 0;
	iTE6805_CEC_RX->Tx_CECDone = 1;
	iTE6805_CEC_RX->Tx_CECFire = 0x00;
	iTE6805_CEC_RX->Tx_CECInitDone = 0x00;

	iTE6805_CEC = &iTE6805_CEC_RX[0]; // point to CEC_RX, if needed TX, need to implement change TX RX sel

	cecwr(0x08, 0x4C);
	cecwr(0x08, 0x48);

	cecwr(0x09 , 0x60);			// RD Setting
	cecwr(0x0B , 0x14);			// RD Setting

	cecwr(0x0C, CEC_timeunit);		//REG_Timer_unit[7:0]

	cecset(0x08, 0x04, 0x00);

	cecset(0x09, 0x02, 0x00);

#if defined(AST_HDMIRX)
	/* initial Queue for save CEC Tx command */
	tx_hp_q_init();
	tx_q_init();

#if (AST_IT6805_INTR_CEC == 1)
	cecwr(0x06, 0x07); /* just enable: Rx done, Tx done and Tx fail */
	cecset(0x08, 0x01, 0x01); /* enable CEC interrupt */
#endif

#else
	cecwr(0x06, 0x00);			// enable INT
	cecset(0x08, 0x01, 0x01);	// enable INT
#endif

	iTE6805_CEC_Reset_RX_FIFO();

	cecwr(0x22, iTE6805_CEC_Logical_Address);	// setting 6805 Logical Address

	cecwr(0x0A, 0x03);
	cecset(0x0A, 0x40, 0x40);			// Enable CEC
}

#if defined(AST_HDMIRX)

void cec_cmd_set(u8 follower, u8 opcode, u8 *operand, u32 operand_size)
{
	struct it680x_drv_data *d = (struct it680x_drv_data *) iTE6805_DATA.drv_data;
	u8 buf[32];

	buf[0] = ((iTE6805_CEC_RX->MY_LA << 4) | follower);
	buf[1] = opcode;

	if (operand_size) {
		if (operand_size > 16)
			operand_size = 16;

		memcpy(buf + 2, operand, operand_size);
	}

	it680x_cec_tx_en_q_lp(buf, operand_size + 2);
	queue_work(d->wq, &d->cec_work);
}

void cec_cmd_ping(u8 follower)
{
	struct it680x_drv_data *d = (struct it680x_drv_data *) iTE6805_DATA.drv_data;
	u8 buf[1];

	buf[0] = ((follower << 4) | follower);
	it680x_cec_tx_en_q_lp(buf, 1);
	queue_work(d->wq, &d->cec_work);
}

void cec_cmd_give_physical_address(u8 follower)
{
	cec_cmd_set(follower, CEC_MSG_GIVE_PHYSICAL_ADDR, NULL, 0);
}

static req_pa_state_type rpa_state = s_req_pa_idle;

void switch_req_pa_state(req_pa_state_type state)
{
	rpa_state = state;
}

void cec_topology_fsm(void)
{
	struct it680x_drv_data *d = (struct it680x_drv_data *) iTE6805_DATA.drv_data;
	static u8 polling_cnt, pa_polling_cnt;

	switch (rpa_state) {
	case s_req_pa_idle:
		break;
	case s_req_pa_ping:
		polling_cnt++;

		if (polling_cnt < 15) {
			ping_wait = polling_cnt;
			cec_cmd_ping(polling_cnt);
			switch_req_pa_state(s_req_pa_ping_wait);
		} else {
			switch_req_pa_state(s_req_pa_idle);
		}
		queue_work(d->wq, &d->cec_work);
		break;
	case s_req_pa_ping_wait:

		if (polling_cnt != ping_wait) {
			switch_req_pa_state(s_req_pa_polling);
			queue_work(d->wq, &d->cec_work);
		}
		break;
	case s_req_pa_polling:
		if ((polling_cnt < 15) && (CECList[polling_cnt].Active == TRUE)) {
			cec_cmd_give_physical_address(polling_cnt);
			cec_cmd_give_physical_address(polling_cnt);
			pa_polling_cnt = 0;
			polling_wait = polling_cnt;
			/* have to wait for respond of follower, switch to another waiting state */
			switch_req_pa_state(s_req_pa_polling_wait);
		} else {
			switch_req_pa_state(s_req_pa_ping);
		}

		queue_work(d->wq, &d->cec_work);
		break;
	case s_req_pa_polling_wait:
		pa_polling_cnt++;

		if ((polling_cnt != polling_wait) || (pa_polling_cnt > 4)) {
			switch_req_pa_state(s_req_pa_ping);
			queue_work(d->wq, &d->cec_work);
		} else {
			msleep(50);
		}
		queue_work(d->wq, &d->cec_work);
		break;
	case s_req_pa_init:
		polling_cnt = 0;
		switch_req_pa_state(s_req_pa_ping);
		queue_work(d->wq, &d->cec_work);
		break;
	default:
		break;
	}
}
#endif /* #if defined(AST_HDMIRX) */
#endif


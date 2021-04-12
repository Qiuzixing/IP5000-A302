///*****************************************
//  Copyright (C) 2009-2019
//  ITE Tech. Inc. All Rights Reserved
//  Proprietary and Confidential
///*****************************************
//   @file   <iTE6805_CEC_SYS.h>
//   @author Kuro.Chung@ite.com.tw
//   @date   2019/03/05
//   @fileversion: iTE6805_MCUSRC_1.31
//******************************************/
#ifndef _ITE6805_CEC_SYS_H_
#define _ITE6805_CEC_SYS_H_

#if (_ENABLE_IT6805_CEC_==TRUE)
void iTE6805_hdmirx_CEC_irq(void);

void	iTE6805_CEC_fsm(void);
void	iTE6805_CEC_chg(STATECEC_Type NewState);

iTE_u8	iTE6805_CEC_CMD_Check(pCEC_FRAME CEC_FRAME);
void	iTE6805_CEC_CMD_Print(pCEC_FRAME CEC_FRAME);
void	iTE6805_CEC_CMD_Push_To_Queue_Handler(void);
void	iTE6805_CEC_CMD_Ready_To_Fire(void);

void iTE6805_CEC_INIT(void);

#if defined(AST_HDMIRX)
typedef enum _req_pa_state_type {
	s_req_pa_idle,		/* 0 */
	s_req_pa_init,		/* 1 */
	s_req_pa_ping,		/* 2 */
	s_req_pa_ping_wait,	/* 3 */
	s_req_pa_polling,	/* 4 */
	s_req_pa_polling_wait,	/* 5 */
} req_pa_state_type;

void switch_req_pa_state(req_pa_state_type state);
void it680x_cec_send(u8 *buf, u32 size);
#endif /* #if defined(AST_HDMIRX) */
#endif
#endif


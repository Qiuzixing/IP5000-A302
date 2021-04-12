/*
 * Copyright (c) 2004-2012
 * ASPEED Technology Inc. All Rights Reserved
 * Proprietary and Confidential
 *
 * By using this code you acknowledge that you have signed and accepted
 * the terms of the ASPEED SDK license agreement.
 */

#ifndef _VIDEOIP_INTERRUPT_H_
#define _VIDEOIP_INTERRUPT_H_

#include "global.h"

irqreturn_t video_interrupt (int irq, void *dev_id, struct pt_regs *regs);

#if defined(CONFIG_ARCH_AST1500_HOST)
#if (CONFIG_AST1500_SOC_VER >= 3)
irqreturn_t video_interrupt_desc (int irq, void *dev_id, struct pt_regs *regs);
#endif /* #if (CONFIG_AST1500_SOC_VER >= 3) */
#endif /* #if defined(CONFIG_ARCH_AST1500_HOST) */

#ifdef CONFIG_ARCH_AST1500_CLIENT
#if SYNC_WITH_VSYNC
irqreturn_t vsync_interrupt (int irq, void *dev_id, struct pt_regs *regs);
#endif /* #if SYNC_WITH_VSYNC */
#if DELAY_DEQUEUE
irqreturn_t hw_timer_interrupt(int irq, void *dev_id, struct pt_regs *regs);
#endif /* #if DELAY_DEQUEUE */
#endif /* #ifdef CONFIG_ARCH_AST1500_CLIENT */


#endif /* #ifndef _VIDEOIP_INTERRUPT_H_ */


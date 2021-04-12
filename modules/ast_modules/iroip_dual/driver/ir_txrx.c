/*
 * Copyright (c) 2004-2012
 * ASPEED Technology Inc. All Rights Reserved
 * Proprietary and Confidential
 *
 * By using this code you acknowledge that you have signed and accepted
 * the terms of the ASPEED SDK license agreement.
 */

#if (CONFIG_AST1500_SOC_VER >= 2)

#include <asm/arch/gpio.h>
#include "ir_txrx.h"


#ifdef CONFIG_ARCH_AST1500_CLIENT
int check_IRRx_Int(void)
{
	volatile unsigned int status = read_register (IR_INT_STAT);
//#if NEW_DBG
//	printk("INT status=0x%08x\n", status);
//#endif
	write_register (IR_INT_STAT, status);
	status &= IR_RX_INT_MASK;

	if (status)
	{
		return 1;
	}
	else
		return 0;
}
#endif

#endif //#if (CONFIG_AST1500_SOC_VER >= 2)


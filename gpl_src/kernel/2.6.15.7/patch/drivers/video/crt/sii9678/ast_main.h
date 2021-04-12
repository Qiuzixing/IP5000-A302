/*
** Copyright (c) 2004-2014
** ASPEED Technology Inc. All Rights Reserved
** Proprietary and Confidential
**
** By using this code you acknowledge that you have signed and accepted
** the terms of the ASPEED SDK license agreement.
**
*/
#ifndef _AST_MAIN_H_
#define _AST_MAIN_H_

#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/workqueue.h>

#include <asm/arch/drivers/board_def.h>
#include <asm/arch/ast-scu.h>
#include <asm/arch/drivers/I2C.h>
#include <asm/arch/drivers/crt.h>
#include <asm/arch/drivers/video_hal.h>
#include "si_virtual_page.h"
#include "si_drv_adapter_tx.h"
#include "si_drv_adapter_internal.h"
#include "si_drv_adapter_flash.h"

struct sii_drv_data {
	DrvAdaptInst_t adapter;
#if defined(CONFIG_ARCH_AST1500_HOST)
	struct work_struct intr_work;
#endif
};
#endif /* #ifndef _AST_MAIN_H_ */

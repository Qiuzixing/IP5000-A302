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
#include <linux/types.h>

//#include "it6663_adapter.h"
typedef struct
{
    uint8_t                   instCra;   // CRA instance
   // ItDrvAdaptConfig_t       config;    // Driver configuration
    //ItDrvAdaptAccessStatus_t accessStatus; // FW update, SRAM and EDID access status
    //SiiLibEvtCbFunc_t         pNtfCallback;  // Pointer to the notification callback function
    //bool_t                    isBootingInProgress; // true during booting

    //bool_t                    upgradeMode;
    //bool_t                    isError;
} DrvAdaptInst_t;
struct gsv200x_drv_data {
		DrvAdaptInst_t adapter;
};



#endif /* #ifndef _AST_MAIN_H_ */


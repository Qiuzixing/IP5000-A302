/******************************************************************************
 *
 * Copyright 2013, Silicon Image, Inc.  All rights reserved.
 * No part of this work may be reproduced, modified, distributed, transmitted,
 * transcribed, or translated into any language or computer format, in any form
 * or by any means without written permission of
 * Silicon Image, Inc., 1140 East Arques Avenue, Sunnyvale, California 94085
 *
 *****************************************************************************/
/**
 * @file
 *
 * @brief HDCP 1.x/2.x to HDCP 2.x/1.x adapter.
 *
 *****************************************************************************/


#ifndef SI_DRV_ADAPTER_API_H
#define SI_DRV_ADAPTER_API_H

#ifdef __cplusplus
extern "C"{
#endif

/***** #include statements ***************************************************/
#include "si_drv_adapter.h"
#include "si_drv_adapter_version.h"
#include "si_drv_adapter_mhl.h"
#include "si_drv_adapter_rx.h"
#include "si_drv_adapter_tx.h"
#include "si_drv_adapter_platform_api.h"        //can be removed, check with sakita's branch code
#include "si_drv_adapter_flash.h"

/***** public macro definitions **********************************************/

/***** public type definitions ***********************************************/

/***** public functions ******************************************************/

#ifdef __cplusplus
}
#endif

#endif // SI_DRV_ADAPTER_API_H


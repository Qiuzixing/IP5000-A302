/*
 * Copyright (c) 2004-2012
 * ASPEED Technology Inc. All Rights Reserved
 * Proprietary and Confidential
 *
 * By using this code you acknowledge that you have signed and accepted
 * the terms of the ASPEED SDK license agreement.
 */

#ifndef _VE_H_
#define _VE_H_

#include "global.h"

#if (CONFIG_AST1500_SOC_VER == 1) && defined (CONFIG_ARCH_AST1500_HOST)
#define THIS_VE ((INPUT_SELECT_ANALOG == VideoDev.cap_info.inputSelect)? VIDEO2 : VIDEO1)
#define VE_REG_OFFSET ((INPUT_SELECT_ANALOG == VideoDev.cap_info.inputSelect)? 0x100 : 0)
#else
#define THIS_VE (VIDEO1)
#define VE_REG_OFFSET (0)
#endif


#endif /* #ifndef _VE_H_ */

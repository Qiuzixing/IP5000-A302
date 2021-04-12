///*****************************************
//  Copyright (C) 2009-2019
//  ITE Tech. Inc. All Rights Reserved
//  Proprietary and Confidential
///*****************************************
//   @file   <iTE6805_Global.h>
//   @author Kuro.Chung@ite.com.tw
//   @date   2019/03/05
//   @fileversion: iTE6805_MCUSRC_1.31
//******************************************/
#ifdef WIN32
#include <windows.h>
#include "..\\src\\USBI2C.h"
#endif

#include "typedef.h"
#include "config.h"
#include "debug.h"

#if (_ENABLE_6805_AS_HDCP_REPEATER_CODE_)
#include "iTE6805_HDCP_Repeater_DEFINE.h"
#endif

#include "iTE6805_DEV_DEFINE.h"
#if (_ENABLE_IT6805_CEC_==TRUE)
	#include "iTE6805_CEC_DEFINE.h"
#endif
#include "iTE6805_DRV.h"
#include "iTE6805_I2C_RDWR.h"

#if (_ENABLE_EDID_RAM_==TRUE)
#include "iTE6805_EDID.h"
#endif

#ifdef _MCU_8051_
// this file don't need add to PC Code
	#if _MCU_8051_EVB_
		#include "iTE6805_EVB_Debug.h"
	#endif

#include "IO.h"
#include "mcu.h"

#endif

#if !defined(AST_HDMIRX)
#include "Utility.h"
#endif
#include "iTE6805_SYS.h"

#if (_ENABLE_IT6805_CEC_==TRUE)
	#include "iTE6805_CEC_SYS.h"
#endif

#if (_ENABLE_AUTO_EQ_==TRUE)
	#include "iTE6805_EQ.h"
#endif

#if (_ENABLE_IT6805_MHL_FUNCTION_==TRUE)
	#include "iTE6805_MHL_DEF.h"
	#include "iTE6805_MHL_SYS.h"
	#include "iTE6805_MHL_DRV.h"
#endif


#if (_ENABLE_6805_AS_HDCP_REPEATER_CODE_)
	#include "iTE6805_HDCP_Repeater.h"
	#include "iTE6805_HDCP_Repeater_Callback.h"
#endif




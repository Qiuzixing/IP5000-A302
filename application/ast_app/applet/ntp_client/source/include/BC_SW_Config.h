/****************************************************************************
* 版权信息：（C）2013，深圳市拔超科技有限公司版权所有
* 系统名称： 
* 文件名称：BC_SW_Config.h 
* 文件说明：
* 作    者：Heyman 
* 版本信息：1.0 
* 设计日期：2021-09-16
* 修改记录：
* 日    期		版    本		修改人 		修改摘要  
****************************************************************************/

#ifndef __BC_SW_CONFIG_H__
#define __BC_SW_CONFIG_H__

#include <unistd.h>
#include <pthread.h>
#include <sys/syscall.h>

#include "BC_Data_Type.h"

#define SOFTWARE_VERSION         	            0x1000 //0x1000   -> V1.0.0.0

#define SOFTWARE_COMPLIER_DATE      			__DATE__
#define SOFTWARE_COMPLIER_TIME      			__TIME__

#ifndef APP_PID
#define APP_PID          						(syscall(SYS_getpid))
#endif

#ifndef THREAD_ID
#define THREAD_ID    	 						(syscall(SYS_gettid))    
#endif


#endif//#ifndef __BC_SW_CONFIG_H__

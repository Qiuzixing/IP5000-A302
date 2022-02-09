/****************************************************************************
* 版权信息：（C）2020，深圳市拔超科技有限公司版权所有
* 系统名称： 
* 文件名称：debug.h
* 文件说明：该文件对log4cpp的接口做了一层简单的封装
* 作    者：zp
* 版本信息：1.0 
* 设计日期：2020-9-4 
* 修改记录：
* 日    期                版    本                修改人                 修改摘要  
****************************************************************************/

#ifndef _DEBUG_H_
#define _DEBUG_H_

#include "log4cpp/log4cpp.hh"


#define BCLog_Init(/*const char* */filename) do \
{ \
    GBLog_Init(filename); \
} while (0)

//GBLog_Log(LL_EMERG, "printf LL_EMERG\n");


#define BC_INFO_LOG(format, args...) do\
{ \
	GBLog_Log(LL_INFO, format,##args); \
}while(0)


#define BC_WARN_LOG(format, args...) do\
{ \
	GBLog_Log(LL_WARN, format,##args); \
}while(0)


#define BC_ERROR_LOG(format, args...) do\
{ \
	GBLog_Log(LL_ERROR, format,##args); \
}while(0)


#define BCLog_Close() do \
{ \
    GBLog_Shutdown(); \
} while (0)






#endif


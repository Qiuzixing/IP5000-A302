/****************************************************************************
* ��Ȩ��Ϣ����C��2020�������аγ��Ƽ����޹�˾��Ȩ����
* ϵͳ���ƣ� 
* �ļ����ƣ�debug.h
* �ļ�˵�������ļ���log4cpp�Ľӿ�����һ��򵥵ķ�װ
* ��    �ߣ�zp
* �汾��Ϣ��1.0 
* ������ڣ�2020-9-4 
* �޸ļ�¼��
* ��    ��                ��    ��                �޸���                 �޸�ժҪ  
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


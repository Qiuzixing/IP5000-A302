/****************************************************************************
* ��Ȩ��Ϣ����C��2013�������аγ��Ƽ����޹�˾��Ȩ����
* ϵͳ���ƣ� 
* �ļ����ƣ�BC_SW_Config.h 
* �ļ�˵����
* ��    �ߣ�Heyman 
* �汾��Ϣ��1.0 
* ������ڣ�2021-09-16
* �޸ļ�¼��
* ��    ��		��    ��		�޸��� 		�޸�ժҪ  
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

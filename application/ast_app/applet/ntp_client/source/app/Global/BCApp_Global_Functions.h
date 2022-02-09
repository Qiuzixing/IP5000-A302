/****************************************************************************
* ��Ȩ��Ϣ����C��2013�������аγ��Ƽ����޹�˾��Ȩ����
* ϵͳ���ƣ� 
* �ļ����ƣ�BCApp_Global_Functions.h
* �ļ�˵����
* ��    �ߣ�Heyman 
* �汾��Ϣ��1.0 
* ������ڣ�2021-09-16 
* �޸ļ�¼��
* ��    ��		��    ��		�޸��� 		�޸�ժҪ  
****************************************************************************/
#ifndef __BCAPP_GLOBAL_FUNCTIONS_H__
#define __BCAPP_GLOBAL_FUNCTIONS_H__
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>

#define YEAR_START			  1900

typedef struct _T_Local_Time
{
	int sec;
	int min;
	int hour;
	int day;
	int mon;
	int year;
}T_Local_Time;

#ifdef __cplusplus
extern "C" {
#endif
int BCApp_Excute_Shell_Cmd(const char* i_cmd,bool i_bNoWait = false);
int BCApp_Is_IP_Addr_Str(char* i_pBuf);
int BCApp_Is_Valid_Hour(int *i_pdata);
void Get_System_Time(T_Local_Time *o_tTime);

#ifdef __cplusplus
}
#endif

#endif
 

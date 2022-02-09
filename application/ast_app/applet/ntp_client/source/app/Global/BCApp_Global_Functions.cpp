/****************************************************************************
* 版权信息：（C）2013，深圳市拔超科技有限公司版权所有
* 系统名称： 
* 文件名称：BCApp_Global_Functions.cpp
* 文件说明：
* 作    者：Heyman 
* 版本信息：1.0 
* 设计日期：2021-09-16
* 修改记录：
* 日    期		版    本		修改人 		修改摘要  
****************************************************************************/
#include "BC_SW_Config.h"
#include "BCApp_Global_Functions.h"

#include "BC_Common_Log.h"

#define BCLOG_TAG    		  "BCApp_Global_Functions"

int BCApp_Excute_Shell_Cmd(const char* i_cmd,bool i_bNoWait)
{
	if(i_cmd == NULL)
	{
		return FAILURE;
	}
	if(i_bNoWait)
	{
		FILE *fp = NULL;
		fp = popen(i_cmd,"r");
        if(fp == NULL)
        {
			return -1;
        }
		pclose(fp);
	}
	else
	{
		return system(i_cmd);
	}
	return 0;
}

int BCApp_Is_IP_Addr_Str(char* i_pBuf)
{
	// check ip
	int rt = 0;
    int s32Point = 0;
	int nNumCount = 0;
    int s32StrLen = strlen(i_pBuf);

	if(i_pBuf == NULL )
	{
		BC_LOGE(BCLOG_TAG,"input pBuf is NULL\n");
		return FAILURE;
	}
	
	if(s32StrLen == 0)
	{
		BC_LOGE(BCLOG_TAG,"input pBuf length is 0\n");
		return FAILURE;
	}
	
    for(int s32ForLoop = 0; s32StrLen > s32ForLoop; s32ForLoop++)
    {   
        if (('0' <= i_pBuf[s32ForLoop]) && ('9' >= i_pBuf[s32ForLoop]))
        {
        	nNumCount++;
			rt++;
        }
        else if ('.' == i_pBuf[s32ForLoop])
        {
        	if(nNumCount <= 0|| nNumCount >= 4)
        	{
                break;
        	}
            s32Point++;
			nNumCount = 0;
			rt++;
        }
        else
        {
            rt = -1;
			break;
        }
    }
	
	
    if (3 != s32Point)
    {
        rt = -1;
    }
	
	if((nNumCount <= 0)||(nNumCount >= 4))
    {
        rt = -1;
    }
	
	int aBuffer[4];
	if(sscanf(i_pBuf, "%d.%d.%d.%d", &aBuffer[0], &aBuffer[1], &aBuffer[2], &aBuffer[3]) == 4)
	{
		for(int iLoop = 0; iLoop < 4; iLoop++)
		{
			if(aBuffer[iLoop] < 0 || aBuffer[iLoop] > 255)
			{
				rt = -1;
				break;
			}
		}
	}
	else
	{
		rt = -1;
	}
	return rt;
}

int BCApp_Is_Valid_Hour(int *i_pdata)
{
    if(i_pdata == NULL)
    {
        return -1;
    }
    if(*i_pdata < 0 || *i_pdata > 23)
    {
        return -1;
    }
    return 0;
}

void Get_System_Time(T_Local_Time *o_tTime)
{
	if(o_tTime == NULL)
	{
		return;
	}
	time_t timep;
	struct tm *ptm;
	
	time(&timep);
    ptm =localtime(&timep); 
	
    o_tTime->sec = ptm->tm_sec;
	o_tTime->min = ptm->tm_min;
	o_tTime->hour = ptm->tm_hour;
	o_tTime->day = ptm->tm_mday;
	o_tTime->mon = ptm->tm_mon + 1;
	o_tTime->year = ptm->tm_year + YEAR_START;
}

/****************************************************************************
* 版权信息：（C）2013，深圳市拔超科技有限公司版权所有
* 系统名称： 
* 文件名称：main.cpp 
* 文件说明：
* 作    者：Heyman 
* 版本信息：1.0 
* 设计日期：2021-09-16
* 修改记录：
* 日    期		版    本		修改人 		修改摘要  
****************************************************************************/
#include "BC_SW_Config.h"
#include "BC_Common_Log.h"

#include "BCApp_Global_Functions.h"

#define BCLOG_TAG    						"ntp_client"

#define NTP_CLIENT_PROCESS					"ntpdate"

void BCApp_Excute_Ntp_Date(char *p_Ipaddr)
{
	if(-1 == BCApp_Is_IP_Addr_Str(p_Ipaddr))
	{
		return;
	}
	char strcmd[256];
	memset(strcmd,0x00,sizeof(strcmd));
	snprintf(strcmd,sizeof(strcmd),"%s -u %s",NTP_CLIENT_PROCESS,p_Ipaddr);
	BC_LOGD(BCLOG_TAG,"strcmd=[%s]\n",strcmd);
	BCApp_Excute_Shell_Cmd(strcmd,true);
}

int main(int argc, char **argv)
{
	int ihour = 0;
	if(argc < 3)
	{
		printf("useage:%s server_ipaddr hour\n",argv[0]);
		printf("For example:%s ntp_client 192.168.0.96 8\r\n",argv[0]);
		exit(1);
	}
	else
	{
		ihour = atoi(argv[2]);
		if((-1 == BCApp_Is_IP_Addr_Str(argv[1])) || (-1 == BCApp_Is_Valid_Hour(&ihour)))
		{
			printf("error param\n");
			exit(1);
		}
	}
	BC_LOGD(BCLOG_TAG,"%s V%d.%d.%d %s %s\n",argv[0],CHV2(SOFTWARE_VERSION),SOFTWARE_COMPLIER_DATE,SOFTWARE_COMPLIER_TIME);
	BC_LOGD(BCLOG_TAG,"server_ipaddr=[%s] hour=[%d]\n",argv[1],ihour);
	BCApp_Excute_Ntp_Date(argv[1]);
	T_Local_Time tLocalTime;
	memset(&tLocalTime,0x00,sizeof(T_Local_Time));
	while(1)
	{
		Get_System_Time(&tLocalTime);
		if(tLocalTime.hour == ihour && (0 == tLocalTime.min % 12))
		{
			BC_LOGD(BCLOG_TAG,"tLocalTime.hour=[%d] hour=[%d]\n",tLocalTime.hour,ihour);
			BCApp_Excute_Ntp_Date(argv[1]);
		}
		usleep(60*1000*1000);//60s
	}
    return 0;
}


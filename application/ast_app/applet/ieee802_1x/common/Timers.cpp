
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <string>

#include "Timers.h"
#include "MsgQueue.h"
#include "wpa_ctrl.h"
#include "log.h"

#define		BUF_SIZE		1024
//#define 	WPA_CTRL_FILE	"/var/run/wpa_supplicant/eth0"

typedef enum 
{
	IEEE_802_1X_SUCCESS 		= 1,
	IEEE_802_1X_FAIL			,
	IEEE_802_1X_ERROR_TIMEOUT	,
	IEEE_802_1X_ERROR_INIT		,
	IEEE_802_1X_ERROR_INTERNAL	,
}ieee802_1x_Status;

void Timeout(union sigval i_pArg)
{
	Timers *pTimer = (Timers *)i_pArg.sival_ptr;
	
	if (pTimer->WpaCtrlCmd("LOGOFF") != 0)
	{
		return;
	}

	if (pTimer->WpaCtrlCmd("DISCONNECT") != 0)
	{
		return;
	}
	
	if (pTimer->m_objMsgQueue->MsgSend(IEEE_802_1X_ERROR_TIMEOUT) != 0)
	{ 
		return;
	}
	
}

int Timers::TimerInit(MessageQueue *i_objMsgQueue, int i_s32Msec)
{
	int err = -1;

	m_pWpaCtrlFd = NULL;
	m_objMsgQueue = i_objMsgQueue;
	
	struct sigevent sevp;
	sevp.sigev_notify = SIGEV_THREAD;
	sevp.sigev_value.sival_ptr = this;
	
	sevp.sigev_notify_function = Timeout;
	sevp.sigev_notify_attributes = NULL;
	
	err = timer_create(CLOCK_REALTIME, &sevp, &m_Timer);
	if (err != 0)
	{
		GBLog_Log(LL_INFO, "timer_create fail");
		return -1;
	}

	m_tIntervalTime.it_interval.tv_sec = 0;
	m_tIntervalTime.it_interval.tv_nsec = 0;
	
	m_tIntervalTime.it_value.tv_sec = i_s32Msec / 1000;
	m_tIntervalTime.it_value.tv_nsec = i_s32Msec % 1000 * 1000000L;

	return err;
}


void Timers::TimerDeinit()
{
	if (timer_delete(m_Timer))
	GBLog_Log(LL_INFO, "timer deinit fail=%s", strerror(errno));
}

int Timers::TimerStart(std::string i_File)
{
	GBLog_Log(LL_INFO, "Timers::TimerStart enter!");
	int err;
	
	m_strWpaCtrlFile = i_File;
	err =  timer_settime(m_Timer, 0, &m_tIntervalTime, NULL);
	if (err != 0)
	{
		GBLog_Log(LL_INFO, "timer start fail=%s", strerror(errno));
		return -1;
	}
	return 0;
}

int Timers::TimerStop()
{
	GBLog_Log(LL_INFO, "Timers::TimerStop enter!");
	int err;
	struct itimerspec tIntervalTime;
	
	tIntervalTime.it_interval.tv_sec = 0;
	tIntervalTime.it_interval.tv_nsec = 0;
	tIntervalTime.it_value.tv_sec = 0;
	tIntervalTime.it_value.tv_nsec = 0;

	if (m_Timer != (timer_t)-1)
	{
		err = timer_settime(m_Timer, 0, &tIntervalTime, NULL);
		if (err != 0)
		{
			GBLog_Log(LL_INFO, "timer stop fail=%s", strerror(errno));
			if (errno ==  EINVAL)
			{
				GBLog_Log(LL_INFO, "EINVAL");
			}
			return -1;
		}
	}
	
	return 0;
}

void _MsgCB(char * i_pMsg, size_t i_Len)
{
	GBLog_Log(LL_INFO, "MsgCB: %s", i_pMsg);
}

int Timers::WpaCtrlCmd(std::string i_strCtrlCmd)
{
	GBLog_Log(LL_INFO, "Timers::WpaCtrlCmd enter");
	int err = 0;
	char Reply[BUF_SIZE] = {0};

	if (NULL == m_pWpaCtrlFd)

	{
		m_pWpaCtrlFd = wpa_ctrl_open(m_strWpaCtrlFile.c_str());
		if (m_pWpaCtrlFd == NULL)
		{
			GBLog_Log(LL_INFO, "wpa_ctrl_open fail");
			return -1;
		}
	}
	
	size_t s32Size = BUF_SIZE -1;
	err = wpa_ctrl_request(m_pWpaCtrlFd, i_strCtrlCmd.c_str(), i_strCtrlCmd.length(), Reply, &s32Size, _MsgCB);
	do {
		if (err == -1)
		{
			GBLog_Log(LL_INFO, "wpa_ctrl_request fail");
			break;
		}
		
		else if (err == -2)
		{
			GBLog_Log(LL_INFO, "wpa_ctrl_request timeout");
			break;
		}
		
	}while(0);
		
	Reply[s32Size] = '\0';
	GBLog_Log(LL_INFO, "%s :ctrl response = %s", i_strCtrlCmd.c_str(), Reply);
	usleep(5 * 1000);
	
	return err;
}



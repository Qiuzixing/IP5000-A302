
#ifndef __TIMERS_H__
#define __TIMERS_H__

#include <time.h>
#include <signal.h>
#include <string>
#include "MsgQueue.h"

class Timers
{
public:
	int TimerInit(MessageQueue *i_objMsgQueue, int i_s32Msec);
	void TimerDeinit();
	int TimerStart(std::string i_File);
	int TimerStop();

	friend void Timeout(union sigval i_pArg);
private:
	std::string m_strWpaCtrlFile;
	timer_t m_Timer; //定时器id
	struct wpa_ctrl * m_pWpaCtrlFd;
	struct itimerspec m_tIntervalTime; //定时器时间
	MessageQueue *m_objMsgQueue; //消息队列
	
	int WpaCtrlCmd(std::string i_strCtrlCmd);
	
};

#endif

